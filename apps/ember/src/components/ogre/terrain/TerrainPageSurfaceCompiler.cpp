//
// C++ Implementation: TerrainPageSurfaceCompiler
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2007
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.//
//
#include "TerrainPageSurfaceCompiler.h"
#include "TerrainPageSurfaceLayer.h"
#include "ICompilerTechniqueProvider.h"
#include "TerrainPageGeometry.h"

#include <OgreTextureManager.h>

namespace Ember {
namespace OgreView {
namespace Terrain {

TerrainPageSurfaceCompiler::TerrainPageSurfaceCompiler(ICompilerTechniqueProvider& compilerTechniqueProvider) :
		mCompilerTechniqueProvider(compilerTechniqueProvider) {
}

TerrainPageSurfaceCompiler::~TerrainPageSurfaceCompiler() {
	//Clean up any textures that were created for the specific page.
	for (auto& textureName: mManagedTextures) {
		Ogre::TextureManager::getSingleton().remove(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
	}
}

std::unique_ptr<TerrainPageSurfaceCompilationInstance> TerrainPageSurfaceCompiler::createCompilationInstance(
		const TerrainPageGeometryPtr& geometry,
		const SurfaceLayerStore& terrainPageSurfaces) {
	return std::make_unique<TerrainPageSurfaceCompilationInstance>(
			mCompilerTechniqueProvider.createTechnique(geometry, terrainPageSurfaces),
			mManagedTextures);

}

TerrainPageSurfaceCompilationInstance::TerrainPageSurfaceCompilationInstance(
		std::unique_ptr<TerrainPageSurfaceCompilerTechnique> technique,
		std::set<std::string>& managedTextures) :
		mTechnique(std::move(technique)),
		mManagedTextures(managedTextures) {

}

TerrainPageSurfaceCompilationInstance::~TerrainPageSurfaceCompilationInstance() = default;

bool TerrainPageSurfaceCompilationInstance::prepare() {
	return mTechnique->prepareMaterial();
}

bool TerrainPageSurfaceCompilationInstance::compile(const Ogre::MaterialPtr& material) {
	try {
		bool result = mTechnique->compileMaterial(material, mManagedTextures);
		if (!result) {
			material->removeAllTechniques();
		}
		return result;
	} catch (const std::exception& ex) {
		logger->warn("Error when compiling material '{}'. It's probably in an invalid state and will be reset (with blank terrain pages as a probable result): {}",
					 material->getName(),
					 ex.what());
		material->removeAllTechniques();
		return false;
	}
}

bool TerrainPageSurfaceCompilationInstance::compileCompositeMap(const Ogre::MaterialPtr& material) {
	try {
		bool result = mTechnique->compileCompositeMapMaterial(material, mManagedTextures);
		if (!result) {
			material->removeAllTechniques();
		}
		return result;
	} catch (const std::exception& ex) {
		logger->warn("Error when compiling material '{}'. It's probably in an invalid state and will be reset (with blank terrain pages as a probable result): {}",
					 material->getName(),
					 ex.what());
		material->removeAllTechniques();
		return false;
	}
}

}
}
}
