//
// C++ Interface: PolygonPoint
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
#ifndef EMBEROGRE_MANIPULATIONPOLYGONPOINT_H
#define EMBEROGRE_MANIPULATIONPOLYGONPOINT_H

#include <wfmath/point.h>
#include <components/ogre/BulletWorld.h>
#include <components/ogre/BulletCollisionDetector.h>
#include "PolygonPointUserObject.h"

namespace Ogre
{
class SceneNode;
class Entity;
}

namespace Ember
{
namespace OgreView
{
class MovementAdapter;

namespace Authoring
{

class Polygon;
class PolygonPointMover;
struct IPolygonPositionProvider;
/**
 @brief A graphical representation of one point in a polygon.
 The point is represented by a sphere, which can be manipulated through dragging.

 @author Erik Ogenvik <erik@ogenvik.org>
 */
class PolygonPoint
{
public:

	/**
	 * @brief Ctor.
	 * @param polygon The polygon to which this point is a part of.
	 * @param positionProvider An optional position provider. Can be null.
	 * @param scale The scale applied to the ball entity.
	 * @param localPosition The local position of this point, within the polygon space.
	 */
	PolygonPoint(Ogre::SceneNode& baseNode, IPolygonPositionProvider* positionProvider, float scale, const WFMath::Point<2>& localPosition = WFMath::Point<2>::ZERO());

	/**
	 * @brief Dtor.
	 */
	~PolygonPoint();

	/**
	 * @brief Gets the scene node which represents this point.
	 * The scene node is owned by this instance and will be destroyed along with it.
	 * @return The scene node which represents this point.
	 */
	Ogre::SceneNode* getNode();

	/**
	 * @brief Gets the scene node which represents this point.
	 * The scene node is owned by this instance and will be destroyed along with it.
	 * @return The scene node which represents this point.
	 */
	Ogre::SceneNode* getNode() const;

	/**
	 * @brief Translates the position of the point.
	 * @param translation The translation, in local space (i.e. relative to the polygon to which the point belongs to).
	 */
	void translate(const WFMath::Vector<2>& translation);

	/**
	 * @brief Gets the local position of the point within the polygon space.
	 * @return The local position of the point within the polygon space.
	 */
	WFMath::Point<2> getLocalPosition() const;

	/**
	 * @brief Sets the local position.
	 *
	 * Vertical position will be calculated using the IPolygonPositionProvider, if such is set.
	 * @param position The new local position, within the polygon space.
	 */
	void setLocalPosition(const WFMath::Point<2>& position);

	/**
	 * @brief Sets the local position.
	 * @param position The new local position, within the polygon space.
	 */
	void setLocalPosition(const WFMath::Point<3>& position);

	/**
	 * @brief Sets the visibility of the point.
	 * @param visibility The visibility of the point.
	 */
	void setVisible(bool visibility);

	/**
	 * @brief Gets whether the point is shown or not.
	 * @return Whether the point is shown or not.
	 */
	bool getVisible() const;

	void makeInteractive(BulletWorld* bulletWorld);

protected:
	/**
	 * @brief Used for generating unique names for the entities used to represent the points in Ogre.
	 */
	static unsigned int sPointCounter;

	/**
	 * @brief The base node, onto which a new node will be created.
	 */
	Ogre::SceneNode& mBaseNode;

	/**
	 * @brief An optional position provider. Can be null.
	 */
	IPolygonPositionProvider* mPositionProvider;

	/**
	 * @brief The Ogre user object which is used for hooking into the picking system.
	 * This is owned by this instance.
	 */
	PolygonPointUserObject mUserObject;

	/**
	 * @brief The Ogre scene node which is used to represent the point graphically in Ogre.
	 * This is owned by this instance.
	 */
	Ogre::SceneNode* mNode;

	/**
	 * @brief The entity which shows the marker, i.e. the draggable ball.
	 */
	Ogre::Entity* mEntity;

	std::unique_ptr<BulletCollisionDetector> mCollisionDetector;

};

}

}

}

#endif
