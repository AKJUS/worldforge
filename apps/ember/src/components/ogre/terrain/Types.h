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

#ifndef EMBEROGRETERRAINTYPES_H_
#define EMBEROGRETERRAINTYPES_H_

#include "domain/Types.h"

#include <vector>
#include <map>
#include <memory>

namespace Mercator {
class Area;

class Terrain;

class Shader;

class TerrainMod;
}

namespace Eris {
class TerrainModTranslator;
}

namespace WFMath {
template<int>
class AxisBox;
}


namespace Ember::OgreView::Terrain {

class TerrainPage;

class TerrainPageSurfaceLayer;

class TerrainPageGeometry;

class Segment;

class ITerrainPageBridge;

/**
 @brief Defines the height of a special "base point" in the terrain.
 These base points are then user by Mercator::Terrain for generating the actual terrain.
 */
struct TerrainDefPoint;

/**
 * @brief A type used for storing changes to areas. We use instances instead of pointers or references since this type will be used in delayed updating, where the originating instance might not any longer be around.
 */
typedef std::vector<WFMath::AxisBox<2>> AreaStore;

/**
 * @brief A type used for storing the terrain definition points.
 */
typedef std::vector<TerrainDefPoint> TerrainDefPointStore;


/**
 * @brief Encapsules a shader update request.
 */
struct ShaderUpdateRequest {
	/**
	 * @brief A list of areas that have been changed.
	 * Unless UpdateAll is true, this should be used for determining what geometry needs updating.
	 */
	std::vector<WFMath::AxisBox<2>> Areas;
};

typedef std::shared_ptr<Segment> SegmentRefPtr;

typedef std::map<int, SegmentRefPtr> SegmentRefColumn;

typedef std::map<int, SegmentRefColumn> SegmentRefStore;

typedef std::shared_ptr<TerrainPageGeometry> TerrainPageGeometryPtr;

typedef std::vector<TerrainPageGeometryPtr> GeometryPtrVector;

typedef std::map<int, const TerrainPageSurfaceLayer*> SurfaceLayerStore;
}


#endif /* EMBEROGRETERRAINTYPES_H_ */
