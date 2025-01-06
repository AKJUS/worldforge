/*
 Copyright (C) 2009 erik

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

#include "TerrainPageDataProvider.h"
#include "components/ogre/terrain/TerrainHandler.h"
#include "components/ogre/terrain/TerrainPage.h"


namespace Ember::OgreView {
TerrainPageData::TerrainPageData(std::shared_ptr<Terrain::TerrainPage> page) :
		mPage(page) {
}

Ogre::MaterialPtr TerrainPageData::getMaterial() {
	if (mPage) {
		return mPage->getMaterial();
	}
	return {};
}

Ogre::MaterialPtr TerrainPageData::getCompositeMapMaterial() {
	if (mPage) {
		return mPage->getCompositeMapMaterial();
	}
	return {};
}

TerrainPageDataProvider::TerrainPageDataProvider(Terrain::TerrainHandler& handler) :
		mHandler(handler) {
}


std::unique_ptr<IPageData> TerrainPageDataProvider::getPageData(const OgreIndex& ogreIndexPosition) {
	return std::make_unique<TerrainPageData>(mHandler.getTerrainPageAtIndex(convertToWFTerrainIndex(ogreIndexPosition)));
}


TerrainIndex TerrainPageDataProvider::convertToWFTerrainIndex(const OgreIndex& ogreIndexPosition) {
	return {ogreIndexPosition.first, ogreIndexPosition.second};
}

}

