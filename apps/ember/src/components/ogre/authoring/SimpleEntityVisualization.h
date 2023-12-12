//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2010
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

#ifndef SIMPLEENTITYVISUALIZATION_H_
#define SIMPLEENTITYVISUALIZATION_H_

#include <sigc++/trackable.h>
#include <sigc++/connection.h>

namespace Ogre {
class OOBBWireBoundingBox;

class SceneNode;

class Entity;
}


namespace Atlas::Message {
class Element;
}



namespace Ember {
class EmberEntity;

namespace OgreView::Authoring {

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief Provides a simple visualization of an Entity using a wire bounding box.
 *
 * This is useful when you want to visualize the position, size and orientation of a raw entity. Normally a more complex system using Models and attachments is used for visualization. This however doesn't always represent the actual Eris data, for which this class is very much suited.
 */
class SimpleEntityVisualization : public virtual sigc::trackable {
public:
	/**
	 * @brief Ctor.
	 * @param entity The entity this visualization is attached to.
	 * @param sceneNode A precreated scene node to which we'll attach our visualization. Ownership is transferred to this instance.
	 */
	SimpleEntityVisualization(EmberEntity& entity, Ogre::SceneNode* sceneNode);

	/**
	 * @brief Ctor.
	 */
	virtual ~SimpleEntityVisualization();

	/**
	 * @brief Updates the position of the visualization to fit with the entity.
	 */
	void updatePositionAndOrientation();

	/**
	 * @brief Updates the size of the bounding box representing the entity.
	 */
	void updateBbox();

protected:

	/**
	 * @brief The entity which this visualization represents.
	 */
	EmberEntity& mEntity;

	/**
	 * @brief The scene node used by this visualization.
	 */
	Ogre::SceneNode* mSceneNode;

	/**
	 * @brief Shows an arrow indicating current velocity.
	 */
	Ogre::SceneNode* mVelocitySceneNode;

	/**
	 * @brief For debugging purposes. This holds a bounding box of how the entity appears in the eris/atlas world.
	 * This is often different from the Ogre bounding box.
	 */
	Ogre::OOBBWireBoundingBox* mErisEntityBoundingBox;

	/**
	 * @brief The connection for the "bbox" attribute observer.
	 */
	sigc::connection mBboxConnection;
	sigc::connection mScaleConnection;

	/**
	 * @brief Shows the velocity using a scaled arrow.
	 */
	Ogre::Entity* mVelocityArrowEntity;


	/**
	 * @brief When the entity moves, update the position and orientation of the scene node.
	 */
	void entity_Moved();


	/**
	 * @brief Called when the bbox attribute changes.
	 * @param attributeValue The new value of the bbox attribute.
	 */
	void entity_BboxChanged(const Atlas::Message::Element& attributeValue);
};

}


}

#endif /* SIMPLEENTITYVISUALIZATION_H_ */
