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

#include "ShaderPassBlendMapBatch.h"
#include "ShaderPass.h"
#include "components/ogre/terrain/TerrainPageSurfaceLayer.h"

#include "framework/TimedLog.h"

#include <OgreHardwarePixelBuffer.h>
#include <OgreTextureUnitState.h>
#include <OgrePass.h>
#include <framework/Log.h>

namespace Ember::OgreView::Terrain::Techniques {

ShaderPassBlendMapBatch::ShaderPassBlendMapBatch(ShaderPass& shaderPass, unsigned int imageSize) :
		mCombinedBlendMapImage(std::make_unique<Image::ImageBuffer>(imageSize, 4)) {
	//reset the blendMap image
	mCombinedBlendMapImage.reset();
}


void ShaderPassBlendMapBatch::addLayer(const TerrainPageGeometry& geometry, const TerrainPageSurfaceLayer* layer) {
	addBlendMap(geometry, layer, (unsigned int) mLayers.size());
	mLayers.push_back(layer);
}

void ShaderPassBlendMapBatch::addBlendMap(const TerrainPageGeometry& geometry, const TerrainPageSurfaceLayer* layer, unsigned int channel) {
	layer->fillImage(geometry, mCombinedBlendMapImage, channel);
	mSyncedTextures.clear();
}

std::vector<const TerrainPageSurfaceLayer*>& ShaderPassBlendMapBatch::getLayers() {
	return mLayers;
}

void ShaderPassBlendMapBatch::assignCombinedBlendMapTexture(const Ogre::TexturePtr& texture) {
	if (std::find(mSyncedTextures.begin(), mSyncedTextures.end(), texture->getName()) == mSyncedTextures.end()) {
		TimedLog log("ShaderPassBlendMapBatch::assignCombinedBlendMapTexture", true);

		//blit the whole image to the hardware buffer
		Ogre::PixelBox sourceBox(mCombinedBlendMapImage.getResolution(), mCombinedBlendMapImage.getResolution(), 1, Ogre::PF_B8G8R8A8, mCombinedBlendMapImage.getData());

		if ((texture->getUsage() & Ogre::TU_AUTOMIPMAP) && texture->getMipmapsHardwareGenerated()) {
			//No need to blit for all mipmaps as they will be generated.
			Ogre::HardwarePixelBufferSharedPtr hardwareBuffer(texture->getBuffer(0, 0));
			hardwareBuffer->blitFromMemory(sourceBox);
		} else {
			for (size_t i = 0; i <= texture->getNumMipmaps(); ++i) {
				Ogre::HardwarePixelBufferSharedPtr hardwareBuffer(texture->getBuffer(0, i));
				hardwareBuffer->blitFromMemory(sourceBox);
			}
		}

		mSyncedTextures.push_back(texture->getName());
	}
}

void ShaderPassBlendMapBatch::finalize(Ogre::Pass& pass, const Ogre::TexturePtr& texture, bool useNormalMapping) {
	//add our blend map textures first
	assignCombinedBlendMapTexture(texture);
	auto* blendMapTUS = pass.createTextureUnitState();
	blendMapTUS->setTextureScale(1, 1);
	blendMapTUS->setTextureName(texture->getName());
	blendMapTUS->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

	for (auto& layer: mLayers) {
		auto baseTexture = resolveTexture(layer->getDiffuseTextureName());
		if (baseTexture) {
			//add the layer textures
			logger->debug("Adding new layer with diffuse texture {}", baseTexture->getName());
			auto* diffuseTUS = pass.createTextureUnitState();
			//textureUnitState->setTextureScale(0.025, 0.025);
			diffuseTUS->setTexture(baseTexture);
			diffuseTUS->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);

			if (useNormalMapping) {
				Ogre::TextureUnitState* normalMapTextureUnitState = pass.createTextureUnitState();
				auto normalTexture = resolveTexture(layer->getNormalTextureName());
				if (!normalTexture) {
					//Since the shader always expects a normal texture we need to supply a dummy one if no specific one exists.
					normalTexture = Ogre::TextureManager::getSingleton().getByName("dynamic/onepixel", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
				}
				normalMapTextureUnitState->setTexture(normalTexture);
				normalMapTextureUnitState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
			}
		} else {
			logger->warn("Not adding layer to material '{}' since the diffuse texture ('{}') is missing.",
						 pass.getParent()->getParent()->getName(),
						 layer->getDiffuseTextureName());
		}
	}
}

}
