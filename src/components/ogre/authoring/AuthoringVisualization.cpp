/*
 Copyright (C) 2009 Erik Ogenvik

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

#include "AuthoringVisualization.h"
#include "domain/EmberEntity.h"
#include "components/ogre/Convert.h"
#include "components/ogre/OgreInfo.h"
#include "components/ogre/EmberEntityUserObject.h"
#include "components/ogre/MousePicker.h"
#include "components/ogre/EmberOgre.h"
#include "components/ogre/World.h"
#include "components/ogre/Scene.h"
#include "domain/IEntityControlDelegate.h"

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <components/ogre/EntityCollisionInfo.h>

namespace Ember
{
namespace OgreView
{

namespace Authoring
{
AuthoringVisualization::AuthoringVisualization(EmberEntity& entity, Ogre::SceneNode* sceneNode) :
	mEntity(entity),
	mSceneNode(sceneNode),
	mGraphicalRepresentation(nullptr),
	mControlDelegate(nullptr)
{
	createGraphicalRepresentation();
	mEntity.Moved.connect(sigc::mem_fun(*this, &AuthoringVisualization::entity_Moved));
	updatePositionAndOrientation();
}

AuthoringVisualization::~AuthoringVisualization()
{
	removeGraphicalRepresentation();
}

Ogre::SceneNode* AuthoringVisualization::getSceneNode() const
{
	return mSceneNode;
}

EmberEntity& AuthoringVisualization::getEntity()
{
	return mEntity;
}

void AuthoringVisualization::setControlDelegate(const IEntityControlDelegate* controlDelegate)
{
	mControlDelegate = controlDelegate;
}

void AuthoringVisualization::entity_Moved()
{
	updatePositionAndOrientation();
}

void AuthoringVisualization::updatePositionAndOrientation()
{
	if (mControlDelegate) {
		mSceneNode->setPosition(Convert::toOgre(mControlDelegate->getPosition()));
		mSceneNode->setOrientation(Convert::toOgre(mControlDelegate->getOrientation()));
		if (mCollisionDetector) {
			mCollisionDetector->updateTransforms(mControlDelegate->getPosition(), mControlDelegate->getOrientation());
		}
	} else {
		if (mEntity.getPredictedPos().isValid()) {
			mSceneNode->setPosition(Convert::toOgre(mEntity.getPredictedPos()));
		}
		if (mEntity.getOrientation().isValid()) {
			mSceneNode->setOrientation(Convert::toOgre(mEntity.getPredictedOrientation()));
		}
		if (mCollisionDetector) {
			mCollisionDetector->updateTransforms(mEntity.getPredictedPos(), mEntity.getPredictedOrientation());
		}
	}
}

void AuthoringVisualization::createGraphicalRepresentation()
{
	try {
		mGraphicalRepresentation = mSceneNode->getCreator()->createEntity(OgreInfo::createUniqueResourceName("authoring_visualization_" + mEntity.getId()), "common/primitives/model/axes.mesh");
		if (mGraphicalRepresentation) {
			try {
				mSceneNode->attachObject(mGraphicalRepresentation);
				mGraphicalRepresentation->setRenderingDistance(100);

				auto& bulletWorld = EmberOgre::getSingleton().getWorld()->getScene().getBulletWorld();
				mCollisionDetector.reset(new BulletCollisionDetector(bulletWorld));
				//These should only be pickable, not occluding.
				mCollisionDetector->setMask(COLLISION_MASK_PICKABLE);
				auto shape = bulletWorld.createMeshShape(mGraphicalRepresentation->getMesh());
				if (shape) {
					mCollisionDetector->addCollisionShape(shape);
				}
				mCollisionDetector->collisionInfo = EntityCollisionInfo{&mEntity, true};
			} catch (const std::exception& ex) {
				S_LOG_WARNING("Error when attaching axes mesh."<< ex);
				mSceneNode->getCreator()->destroyMovableObject(mGraphicalRepresentation);
			}
		}
	} catch (const std::exception& ex) {
		S_LOG_WARNING("Error when loading axes mesh."<< ex);
	}
}

void AuthoringVisualization::removeGraphicalRepresentation()
{
	mSceneNode->detachAllObjects();
	if (mGraphicalRepresentation) {
		mSceneNode->getCreator()->destroyEntity(mGraphicalRepresentation);
		mGraphicalRepresentation = nullptr;
	}
	mSceneNode->getCreator()->destroySceneNode(mSceneNode);
	mSceneNode = nullptr;
	mCollisionDetector.reset();
}

}
}
}
