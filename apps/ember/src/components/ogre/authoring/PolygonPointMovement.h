//
// C++ Interface: PolygonPointMovement
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2009
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
#ifndef EMBEROGRE_MANIPULATIONPOLYGONPOINTMOVEMENT_H
#define EMBEROGRE_MANIPULATIONPOLYGONPOINTMOVEMENT_H

#include "PolygonPointMover.h"
#include "MovementAdapter.h"


namespace Ember::OgreView {

namespace Camera {
class MainCamera;
}
namespace Authoring {

class PolygonPoint;

class Polygon;

/**
 @brief Represents one movement action of a specific point. When the movement of the point is complete, either by it being cancelled or completed, the instance should be deleted.
 @author Erik Ogenvik <erik@ogenvik.org>
 */
class PolygonPointMovement {
public:
	/**
	 * @brief Ctor.
	 * @param polygon The polygon.
	 * @param point The point that should be moved.
	 * @param movementListener An optional listener which will receive movement updates.
	 * @param camera The main camera used in the world.
	 */
	PolygonPointMovement(Polygon& polygon, PolygonPoint& point, IMovementListener* movementListener, Camera::MainCamera& camera);

	/**
	 * @brief Dtor.
	 */
	~PolygonPointMovement() = default;

protected:

	/**
	 * @brief The point being moved.
	 */
	PolygonPoint& mPoint;

	/**
	 * @brief Main adapter which will intercept mouse and keyboard input to allow for movement of this point.
	 */
	MovementAdapter mMoveAdapter;

};

}

}



#endif
