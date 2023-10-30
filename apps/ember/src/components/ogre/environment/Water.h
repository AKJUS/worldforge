//
// C++ Interface: Water
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2004
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
#ifndef EMBEROGRE_WATER_H
#define EMBEROGRE_WATER_H

#include "components/ogre/EmberOgrePrerequisites.h"
#include "IEnvironmentProvider.h"
#include <OgrePlane.h>

namespace Ember {
namespace OgreView {

namespace Environment {

class RefractionTextureListener;
class ReflectionTextureListener;


/**
@author Erik Ogenvik
*/
class Water : public IWater
{


public:
    Water(Ogre::Camera& camera, Ogre::SceneManager& sceneMgr);

    ~Water() override;

	/**
	 * @brief Performs checks to see whether this technique is supported on the current hardware.
	 * @return True if the water technique is supported.
	 */
	bool isSupported() const override;

	/**
	 * @brief Initializes the water. You must call this in order for the water to show up.
	 * @return True if the water technique could be setup, else false.
	 */
	bool initialize() override;


protected:

	Ogre::Plane mReflectionPlane;

	/**
	 * @brief The camera used.
	 */
	Ogre::Camera& mCamera;

	/**
	 * @brief The scene manager to which the water will be added.
	 */
	Ogre::SceneManager& mSceneMgr;

    std::unique_ptr<RefractionTextureListener> mRefractionListener;
    std::unique_ptr<ReflectionTextureListener> mReflectionListener;

	/**
	 * @brief The node to which the water entity is attached to. Owned by this instance-
	 */
	Ogre::SceneNode *mWaterNode;

	/**
	 * @brief The entity representing the water plane. Owned by this instance-
	 */
	Ogre::Entity* mWaterEntity;

};

}

}

}

#endif
