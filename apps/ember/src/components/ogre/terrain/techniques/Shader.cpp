/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "Shader.h"
#include "ShaderPass.h"
#include "components/ogre/terrain/TerrainPageSurfaceLayer.h"
#include "components/ogre/terrain/TerrainPage.h"
#include "components/ogre/terrain/TerrainPageGeometry.h"
#include <OgreShadowCameraSetupPSSM.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>

namespace Ember::OgreView::Terrain::Techniques {
const std::string Shader::NORMAL_TEXTURE_ALIAS = "EmberTerrain/NormalTexture";
const std::string Shader::COMPOSITE_MAP_ALIAS = "EmberTerrain/CompositeMap";

Shader::Shader(bool includeShadows,
			   const TerrainPageGeometryPtr& geometry,
			   const SurfaceLayerStore& terrainPageSurfaces,
			   Ogre::SceneManager& sceneManager,
			   bool UseNormalMapping) :
		Base(geometry, terrainPageSurfaces),
		mIncludeShadows(includeShadows),
		mSceneManager(sceneManager),
		mUseNormalMapping(UseNormalMapping),
		mUseCompositeMap(false) {
}

Shader::~Shader() = default;

void Shader::reset() {
	mPasses.clear();
	mPassesNormalMapped.clear();
}

bool Shader::prepareMaterial() {
	reset();
	if (mUseNormalMapping) {
		buildPasses(true);
	}

	buildPasses(false);

	//We don't need the geometry any more, so we'll release it as soon as we can.
	mGeometry.reset();
	return true;
}

void Shader::buildPasses(bool normalMapped) {
	ShaderPass* shaderPass;
	if (normalMapped) {
		shaderPass = addPassNormalMapped();
	} else {
		shaderPass = addPass();
	}

	int activeLayersCount = 0;
	if (shaderPass) {
		for (auto mTerrainPageSurface: mTerrainPageSurfaces) {
			const TerrainPageSurfaceLayer* surfaceLayer = mTerrainPageSurface.second;

			if (surfaceLayer->intersects(*mGeometry)) {
				if (!shaderPass->hasRoomForLayer(surfaceLayer)) {
					if (normalMapped) {
						shaderPass = addPassNormalMapped();
					} else {
						shaderPass = addPass();
					}
				}
				shaderPass->addLayer(*mGeometry, surfaceLayer);
				activeLayersCount++;
			}
		}
	} else {
		S_LOG_WARNING("Could not create pass in Shader terrain technique.");
	}
	//If the number of active layers are more than 2 we should use a composite map.
	if (activeLayersCount > 2) {
		mUseCompositeMap = true;
	} else {
		mUseCompositeMap = false;
	}
}

bool Shader::compileMaterial(Ogre::MaterialPtr material, std::set<std::string>& managedTextures) const {
	S_LOG_VERBOSE("Compiling terrain page material " << material->getName());

	material->removeAllTechniques();
	Ogre::Material::LodValueList lodList;
	Ogre::MaterialPtr shadowCasterMaterial = Ogre::MaterialManager::getSingleton().getByName("/common/ShadowCaster/NoAlpha", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	unsigned short currentLodIndex = 0;

	if (mUseNormalMapping) {
		// Create separate normal mapped technique
		auto technique = material->createTechnique();
		technique->setLodIndex(currentLodIndex++);
		technique->setShadowCasterMaterial(shadowCasterMaterial);
		// Use normal mapping for everything nearer than 50 units
		lodList.push_back(50);

		for (auto& shaderPass: mPassesNormalMapped) {
			Ogre::Pass* pass = technique->createPass();
			if (!shaderPass->finalize(*pass, managedTextures, mIncludeShadows, true)) {
				return false;
			}
			//If we use multipasses we need to disable fog for all passes except the last one (else the fog will stack up).
			if (shaderPass != mPassesNormalMapped.back()) {
				pass->getFragmentProgramParameters()->setNamedConstant("disableFogColour", 1);
			}
		}
	}

	{
		// Create the default technique
		auto technique = material->createTechnique();
		technique->setLodIndex(currentLodIndex++);
		technique->setShadowCasterMaterial(shadowCasterMaterial);
		for (auto& shaderPass: mPasses) {
			Ogre::Pass* pass = technique->createPass();
			if (!shaderPass->finalize(*pass, managedTextures, mIncludeShadows, true)) {
				return false;
			}
			//If we use multipasses we need to disable fog for all passes except the last one (else the fog will stack up).
			if (shaderPass != mPasses.back()) {
				pass->getFragmentProgramParameters()->setNamedConstant("disableFogColour", 1);
			}
		}
	}

	if (mUseCompositeMap) {
		// Create a technique which renders using the pre-rendered composite map
		auto technique = material->createTechnique();
		technique->setShadowCasterMaterial(shadowCasterMaterial);
		technique->setLodIndex(currentLodIndex++);
		// Use it for everything farther away than this limit
		//TODO: calculate this based on the size of each page and the size of the texture instead of having it fixed
		lodList.push_back(200);
		// Pretty sure we can always fit this into one pass
		Ogre::Pass* pass = technique->createPass();
		std::string cmVertexProgramName = "Lighting/NormalTexture/";
		if (mIncludeShadows) {
			cmVertexProgramName += "ShadowVp";
		} else {
			cmVertexProgramName += "SimpleVp";
		}
		pass->setVertexProgram(cmVertexProgramName);
		auto fragProgram = ShaderPass::fetchOrCreateSplattingFragmentProgram(ShaderPass::SplattingFragmentConfig{
				.lightning = true,
				.shadows = true,
				.offsetMapping= false,
				.fog = true,
				.layers = 1, //Only one layer, which is the composite diffuse texture
		});
		pass->setGpuProgram(Ogre::GpuProgramType::GPT_FRAGMENT_PROGRAM, fragProgram);

		Ogre::TextureUnitState* normalMapTextureUnitState = pass->createTextureUnitState();

		// Set up an alias for the normal texture. This way the terrain implementation can generate the normal texture at a later time and link it to this material.
		// With the Ogre Terrain Component, this is set up in OgreTerrainMaterialGeneratorEmber.cpp.
		normalMapTextureUnitState->setName(NORMAL_TEXTURE_ALIAS);
		normalMapTextureUnitState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

		if (mIncludeShadows) {
			for (size_t i = 0; i < mSceneManager.getShadowTextureConfigList().size(); ++i) {
				Ogre::TextureUnitState* shadowMapTus = pass->createTextureUnitState();
				shadowMapTus->setContentType(Ogre::TextureUnitState::CONTENT_SHADOW);
				shadowMapTus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
				shadowMapTus->setTextureBorderColour(Ogre::ColourValue(1.0, 1.0, 1.0, 1.0));
			}
		}

		//Since we reuse the same fragment shader as the up close terrain we need a texture for the blending, even if we only use one layer.
		//Use the "onepixel" one, as it's full white.
		auto* blendMapTUS = pass->createTextureUnitState();
		blendMapTUS->setTextureScale(1, 1);
		blendMapTUS->setTextureName("dynamic/onepixel");
		blendMapTUS->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

		Ogre::TextureUnitState* compositeMapTus = pass->createTextureUnitState();
		compositeMapTus->setName(COMPOSITE_MAP_ALIAS);
		compositeMapTus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

		try {
			Ogre::GpuProgramParametersSharedPtr fpParams = pass->getFragmentProgramParameters();
			float scales = 1.0f;
			fpParams->setNamedConstant("scales", scales); // The composite map spreads over the entire terrain, no uv scaling needed
			if (mIncludeShadows) {
				auto* pssmSetup = dynamic_cast<Ogre::PSSMShadowCameraSetup*>(mSceneManager.getShadowCameraSetup().get());
				if (pssmSetup) {
					Ogre::Vector4 splitPoints;
					Ogre::PSSMShadowCameraSetup::SplitPointList splitPointList = pssmSetup->getSplitPoints();
					for (int i = 0; i < 3; ++i) {
						splitPoints[i] = splitPointList[i];
					}
					fpParams->setNamedConstant("pssmSplitPoints", splitPoints);
				}
			}
		} catch (const std::exception& ex) {
			S_LOG_WARNING("Error when setting fragment program parameters." << ex);
		}
	}

	{
		//Now also add a "Low" technique, for use in the compass etc.
		auto technique = material->createTechnique();
		technique->setLodIndex(currentLodIndex);
		technique->setSchemeName("Low");

		for (auto& shaderPass: mPasses) {
			Ogre::Pass* pass = technique->createPass();
			if (!shaderPass->finalize(*pass, managedTextures, false, false)) {
				return false;
			}
		}
	}

	// Apply the LOD levels
	material->setLodLevels(lodList);
	//we need to load it before we can see how many techniques are supported
	material->load();
	//Set texture filtering once all techniques have been added.
	//We'll always use anisotropic filtering, for all detail levels, since the ground often is viewed from an angle.
	material->setTextureAnisotropy(16);
	material->setTextureFiltering(Ogre::TFO_ANISOTROPIC);

	if (material->getSupportedTechniques().empty()) {
		S_LOG_WARNING("The material '" << material->getName() << "' has no supported techniques. The reason for this is: \n" << material->getUnsupportedTechniquesExplanation());
		return false;
	}
	return true;
}

bool Shader::compileCompositeMapMaterial(Ogre::MaterialPtr material, std::set<std::string>& managedTextures) const {
	material->removeAllTechniques();
	if (mUseCompositeMap) {

		Ogre::Technique* technique = material->createTechnique();

		for (auto& shaderPass: mPasses) {
			Ogre::Pass* pass = technique->createPass();
			if (!shaderPass->finalize(*pass, managedTextures, false, false)) {
				return false;
			}
		}
	}
	return true;
}

ShaderPass* Shader::addPass() {
	auto shaderPass = std::make_unique<ShaderPass>(mSceneManager,
												   mGeometry->mPageWidth - 1,
												   mGeometry->mPageIndex,
												   false);
	if (mIncludeShadows) {
		for (size_t i = 0; i < mSceneManager.getShadowTextureConfigList().size(); ++i) {
			shaderPass->addShadowLayer();
		}
	}
	mPasses.emplace_back(std::move(shaderPass));
	return mPasses.back().get();
}

ShaderPass* Shader::addPassNormalMapped() {
	auto shaderPass = std::make_unique<ShaderPass>(mSceneManager,
												   mGeometry->mPageWidth - 1,
												   mGeometry->mPageIndex,
												   true);
	if (mIncludeShadows) {
		for (size_t i = 0; i < mSceneManager.getShadowTextureConfigList().size(); ++i) {
			shaderPass->addShadowLayer();
		}
	}
	mPassesNormalMapped.emplace_back(std::move(shaderPass));
	return mPassesNormalMapped.back().get();
}

}
