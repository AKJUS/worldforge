//
// C++ Interface: SimpleWater
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2008
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
#ifndef EMBEROGRE_ENVIRONMENTSIMPLEWATER_H
#define EMBEROGRE_ENVIRONMENTSIMPLEWATER_H

#include "components/ogre/EmberOgrePrerequisites.h"
#include "IEnvironmentProvider.h"

namespace Ember
{
namespace OgreView
{

namespace Environment
{

class WaterAdjustRenderTargetListener;

/**
 @brief A very simple water implementation which just shows a flat textured water plane.

 @author Erik Ogenvik <erik@ogenvik.org>
 */
class SimpleWater: public IWater
{
public:
	/**
	 * @brief Ctor.
	 * @param camera The main camera in the scene.
	 * @param sceneMgr The scene manager for the scene.
	 * @param mainRenderTarget The main render target for the scene.
	 */
	SimpleWater(Ogre::Camera& camera, Ogre::SceneManager& sceneMgr, Ogre::RenderTarget& mainRenderTarget);

	~SimpleWater() override;

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

	/**
	 * @brief Sets the level of the water.
	 * @param height The height of the water level, in world units.
	 */
	void setLevel(float height) override;

	bool setUserAny(const Ogre::Any &anything) override;

	float getLevel() const;

protected:

	/**
	 * @brief The camera used.
	 */
	Ogre::Camera& mCamera;

	/**
	 * @brief The scene manager to which the water will be added.
	 */
	Ogre::SceneManager& mSceneMgr;

	/**
	 * @brief The node to which the water bobbing node is attached to. Owned by this instance.
	 */
	Ogre::SceneNode *mWaterNode;

	/**
	 * @brief The node to which the water entity is attached to. Owned by this instance.
	 */
	Ogre::SceneNode *mWaterBobbingNode;

	/**
	 * @brief The entity representing the water plane. Owned by this instance.
	 */
	Ogre::Entity* mWaterEntity;

	/**
	 * @brief The main render target of the scene.
	 */
	Ogre::RenderTarget& mMainRenderTarget;

	/**
	 * @brief Render target listener which adjusts the water so that it's always placed on the camera.
	 *
	 * This makes sure that it appears that the water is infinite.
	 */
	std::unique_ptr<WaterAdjustRenderTargetListener> mRenderTargetListener;

	/**
	 * @brief Makes sure that the water bobs up and down a little.
	 */
	Ogre::Controller<Ogre::Real>* mWaterBobbingController;

};

}

}

}

#endif
