/*
 Copyright (C) 2019 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef EMBER_PROJECTILERENDERINGTECHNIQUE_H
#define EMBER_PROJECTILERENDERINGTECHNIQUE_H

#include "components/ogre/ISceneRenderingTechnique.h"
#include "OgreIncludes.h"
#include <OgreFrameListener.h>


namespace Ember::OgreView {

class ProjectileRenderingTechnique : public ISceneRenderingTechnique, Ogre::FrameListener {
public:
	/**
	 * @brief Ctor.
	 */
	explicit ProjectileRenderingTechnique(Ogre::SceneManager& sceneManager);

	/**
	 * @brief Dtor.
	 */
	~ProjectileRenderingTechnique() override;

	void registerEntity(EmberEntity& entity) override;

	void deregisterEntity(EmberEntity& entity) override;

	bool frameStarted(const Ogre::FrameEvent& evt) override;

	void setSimulationSpeed(double speed) {
		mSimulationSpeed = speed;
	}

protected:

	struct ActiveEntityEntry {
		Ogre::SceneNode* particleNode;
		float inactiveTime;
		bool hasBeenAdded; //True if the node has been added to the trail. This is mainly false in the instances where the entity position wasn't valid when the entry was created.
		bool isActive;
	};

	std::vector<Ogre::SceneNode*> mAvailableNodes;
	std::map<EmberEntity*, ActiveEntityEntry> mActiveNodes;

	Ogre::SceneManager& mSceneManager;

	Ogre::RibbonTrail* mTrail;

	std::set<EmberEntity*> mEntities;

	double mSimulationSpeed;
};

}


#endif //EMBER_PROJECTILERENDERINGTECHNIQUE_H
