//
// C++ Implementation: PolygonPoint
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
#include "PolygonPoint.h"
#include "Polygon.h"
#include "IPolygonPositionProvider.h"

#include "MovementAdapter.h"

#include "../Convert.h"
#include "../MousePicker.h"
#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include <memory>

namespace Ember {
namespace OgreView {

namespace Authoring {

unsigned int PolygonPoint::sPointCounter = 0;

PolygonPoint::PolygonPoint(Ogre::SceneNode& baseNode, IPolygonPositionProvider* positionProvider, float scale, const WFMath::Point<2>& localPosition) :
		mBaseNode(baseNode),
		mPositionProvider(positionProvider),
		mUserObject(*this),
		mNode(nullptr),
		mEntity(nullptr) {
	Ogre::Vector3 nodePosition = Convert::toOgre<Ogre::Vector3>(localPosition);
	if (mPositionProvider) {
		nodePosition.y = (Ogre::Real)mPositionProvider->getHeightForPosition(localPosition);
	}
	mNode = mBaseNode.createChildSceneNode(nodePosition);
	mNode->setScale(scale, scale, scale);

	std::stringstream ss;
	ss << "PolygonPoint" << sPointCounter++;

	try {
		mEntity = mNode->getCreator()->createEntity(ss.str(), "common/primitives/model/sphere.mesh");
		//start out with a normal material
		mEntity->setMaterialName("/common/base/authoring/point");
		//The material is made to ignore depth checks, so if we put it in a later queue we're
		//making sure that the marker is drawn on top of everything else, making it easier to interact with.
		mEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_9);
		mEntity->setRenderingDistance(300);

	} catch (const std::exception& ex) {
		S_LOG_FAILURE("Error when creating polygon point marker entity." << ex);
		return;
	}

	if (!mEntity) {
		S_LOG_FAILURE("Unexpected error when creating polygon point marker entity.");
		return;
	}
	mNode->attachObject(mEntity);
}

PolygonPoint::~PolygonPoint() {
	try {
		if (mNode) {
			mBaseNode.removeAndDestroyChild(mNode);
		}
		if (mEntity) {
			mBaseNode.getCreator()->destroyEntity(mEntity);
		}
	} catch (const std::exception& ex) {
		S_LOG_WARNING("Error when deleting polygon point." << ex);
	}
}

Ogre::SceneNode* PolygonPoint::getNode() {
	return mNode;
}

Ogre::SceneNode* PolygonPoint::getNode() const {
	return mNode;
}

WFMath::Point<2> PolygonPoint::getLocalPosition() const {
	return WFMath::Point<2>(mNode->getPosition().x, mNode->getPosition().z);
}

void PolygonPoint::setLocalPosition(const WFMath::Point<2>& position) {
	mNode->setPosition((Ogre::Real) position.x(), mNode->getPosition().y, (Ogre::Real) position.y());
	if (mPositionProvider) {
		Ogre::Vector3 pos = getNode()->getPosition();
		pos.y = (Ogre::Real) mPositionProvider->getHeightForPosition(Convert::toWF<WFMath::Point<2>>(pos));
		getNode()->setPosition(pos);
	}
}

void PolygonPoint::setLocalPosition(const WFMath::Point<3>& position) {
	mNode->setPosition(Convert::toOgre(position));
}

// void PolygonPoint::startMovement()
// {
// 	delete mMover;
// 	delete mMoveAdapter;
// 	mMoveAdapter = new MovementAdapter();
// 	mMover = new PolygonPointMover(*this);
// 	mMoveAdapter->attachToBridge(mMover);
// }
//
// void PolygonPoint::endMovement()
// {
// 	delete mMover;
// 	mMover = 0;
// 	delete mMoveAdapter;
// 	mMoveAdapter = 0;
//
// }

void PolygonPoint::translate(const WFMath::Vector<2>& translation) {
	Ogre::Vector2 ogrePos = Convert::toOgre(translation);
	getNode()->translate(Ogre::Vector3(ogrePos.x, 0, ogrePos.y));
	if (mPositionProvider) {
		Ogre::Vector3 pos = getNode()->getPosition();
		pos.y = (Ogre::Real)mPositionProvider->getHeightForPosition(Convert::toWF<WFMath::Point<2>>(pos));
		getNode()->setPosition(pos);
	}
	if (mCollisionDetector) {
		mCollisionDetector->updateTransforms(Convert::toWF<WFMath::Point<3>>(getNode()->_getDerivedPosition()), WFMath::Quaternion::IDENTITY());
	}
}

void PolygonPoint::setVisible(bool visibility) {
	if (mEntity) {
		mEntity->setVisible(visibility);
	}
}

bool PolygonPoint::getVisible() const {
	if (mEntity) {
		return mEntity->getVisible();
	}
	return false;
}

void PolygonPoint::makeInteractive(BulletWorld* bulletWorld) {
	if (mEntity && bulletWorld) {
		mCollisionDetector = std::make_unique<BulletCollisionDetector>(*bulletWorld);
		mCollisionDetector->collisionInfo = &mUserObject;
		auto shape = std::make_shared<btSphereShape>(mEntity->getWorldBoundingSphere().getRadius());
		mCollisionDetector->addCollisionShape(std::move(shape));
		mCollisionDetector->updateTransforms(Convert::toWF<WFMath::Point<3>>(getNode()->_getDerivedPosition()), WFMath::Quaternion::IDENTITY());
	}
}

}

}
}
