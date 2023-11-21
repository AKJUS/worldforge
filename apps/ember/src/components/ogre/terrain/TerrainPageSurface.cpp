//
// C++ Implementation: TerrainPageSurface
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
#include "TerrainPageSurface.h"
#include "TerrainPageSurfaceLayer.h"
#include "TerrainPageSurfaceCompiler.h"
#include "TerrainPageGeometry.h"
#include "TerrainLayerDefinition.h"
#include "../Convert.h"
#include <OgreMaterialManager.h>
#include <OgreRoot.h>

namespace Ember {
namespace OgreView {
namespace Terrain {

TerrainPageSurface::TerrainPageSurface(const TerrainPosition& terrainPosition,
									   ICompilerTechniqueProvider& compilerTechniqueProvider) :
		mSurfaceCompiler(std::make_unique<TerrainPageSurfaceCompiler>(compilerTechniqueProvider)) {
	//create a name for out material
	// 	logger->info("Creating a material for the terrain.");
	std::stringstream materialNameSS;
	materialNameSS << "EmberTerrain_Segment";
	materialNameSS << "_" << terrainPosition.x() << "_" << terrainPosition.y();
	mMaterialName = materialNameSS.str();

	mMaterial = Ogre::MaterialManager::getSingleton().create(mMaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	mMaterialComposite = Ogre::MaterialManager::getSingleton().create(mMaterialName + "/CompositeMap", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

}

TerrainPageSurface::~TerrainPageSurface() {
	mLayers.clear();
	Ogre::MaterialManager::getSingleton().remove(mMaterial);
	Ogre::MaterialManager::getSingleton().remove(mMaterialComposite);
}

const TerrainPageSurface::TerrainPageSurfaceLayerStore& TerrainPageSurface::getLayers() const {
	return mLayers;
}

void TerrainPageSurface::updateLayer(TerrainPageGeometry& geometry, int layerIndex, bool repopulate) {
	auto I = mLayers.find(layerIndex);
	if (I != mLayers.end()) {
		if (repopulate) {
			I->second.populate(geometry);
		}
		//		I->second->updateCoverageImage(geometry);
	}
}

Ogre::MaterialPtr TerrainPageSurface::getMaterial() const {
	return mMaterial;
}

Ogre::MaterialPtr TerrainPageSurface::getCompositeMapMaterial() const {
	return mMaterialComposite;
}

std::unique_ptr<TerrainPageSurfaceCompilationInstance> TerrainPageSurface::createSurfaceCompilationInstance(const TerrainPageGeometryPtr& geometry) const {
	//The compiler only works with const surfaces, so we need to create such a copy of our surface map.
	//TODO: perhaps store surfaces as shared_ptr, so they can be shared?
	SurfaceLayerStore constLayers;
	for (auto& entry: mLayers) {
		constLayers.emplace(entry.first, &entry.second);
	}
	return mSurfaceCompiler->createCompilationInstance(geometry, constLayers);
}

void TerrainPageSurface::createSurfaceLayer(const TerrainLayerDefinition& definition, int surfaceIndex, const Mercator::Shader& shader) {
	mLayers.emplace(surfaceIndex, TerrainPageSurfaceLayer(*this, definition, surfaceIndex, shader));
}


}

}
}
