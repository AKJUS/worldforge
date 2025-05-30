//
// C++ Implementation: SnapToMovement
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
#include "SnapToMovement.h"
#include "domain/EmberEntity.h"
#include "components/ogre/Convert.h"

#include <wfmath/rotbox.h>
#include <wfmath/ball.h>

#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <components/ogre/MousePicker.h>

#include <memory>
#include <framework/Log.h>


namespace Ember::OgreView::Authoring {

SnapToMovement::SnapToMovement(Eris::Entity& entity, Ogre::Node& node, float snapThreshold, Ogre::SceneManager& sceneManager, bool showDebugOverlay) :
		mEntity(entity),
		mNode(node),
		mSnapThreshold(snapThreshold),
		mSceneManager(sceneManager) {
	if (showDebugOverlay) {
		for (int i = 0; i < 30; ++i) {
			Ogre::SceneNode* newNode = mSceneManager.getRootSceneNode()->createChildSceneNode();
			Ogre::Entity* sphereEntity = mSceneManager.createEntity("common/primitives/model/sphere.mesh");
			//start out with a normal material
			sphereEntity->setMaterialName("/common/base/authoring/point");
			sphereEntity->setRenderingDistance(300);
			// 		entity.setQueryFlags(MousePicker::CM_UNDEFINED);
			newNode->setScale(0.25, 0.25, 0.25);
			newNode->attachObject(sphereEntity);
			newNode->setVisible(false);
			mDebugNodes.push_back(newNode);
		}
	}
}

SnapToMovement::~SnapToMovement() {
	for (auto node: mDebugNodes) {
		node->removeAndDestroyAllChildren();
		mSceneManager.destroySceneNode(node);
	}

}

bool SnapToMovement::testSnapTo(const WFMath::Point<3>& position, const WFMath::Quaternion& orientation, WFMath::Vector<3>& adjustment, EmberEntity** snappedToEntity) {
	try {
		for (auto node: mDebugNodes) {
			node->setVisible(false);
			auto* sphereEntity = dynamic_cast<Ogre::Entity*>(node->getAttachedObject(0));
			sphereEntity->setMaterialName("/common/base/authoring/point");
		}
	} catch (const std::exception& ex) {
		logger->warn("Error when setting up debug nodes for snapping: {}", ex.what());
	}

	auto nodeIterator = mDebugNodes.begin();

	//Use an auto pointer to allow both for undefined values and automatic cleanup when exiting the method.
	std::unique_ptr<SnapPointCandidate> closestSnapping(nullptr);

	WFMath::AxisBox<3> currentBbox = mEntity.getBBox();
	//Translate the bbox into a rotbox
	WFMath::RotBox<3> currentRotbox;
	currentRotbox.size() = currentBbox.highCorner() - currentBbox.lowCorner();
	currentRotbox.corner0() = currentBbox.lowCorner();
	currentRotbox.orientation().identity();
	currentRotbox.rotatePoint(orientation, WFMath::Point<3>(0, 0, 0));
	currentRotbox.shift(WFMath::Vector<3>(position));

	//See if we should visualize debug nodes for the moved entity
	for (size_t j = 0; j < currentRotbox.numCorners(); ++j) {
		WFMath::Point<3> currentPoint = currentRotbox.getCorner(j);
		if (currentPoint.isValid() && nodeIterator != mDebugNodes.end()) {

			Ogre::SceneNode* node = *nodeIterator;
			node->setPosition(Convert::toOgre(currentPoint));
			node->setVisible(true);
			nodeIterator++;
		}
	}

	//First find all entities which are close enough
	//Then try to do a snap movement based on the points of the eris bounding boxes. I.e. we only provide support for snapping one corner of a bounding box to another corner (for now).
	auto parentLocation = mEntity.getLocation();
	if (parentLocation) {
		auto entitySphere = mEntity.getBBox().boundingSphere().toParentCoords(position);
		for (auto& child: parentLocation->getContent()) {
			if (child != &mEntity) {
				if (child->hasBBox()) {
					WFMath::AxisBox<3> bbox = child->getBBox();
					auto childSphere = bbox.boundingSphereSloppy().toParentCoords(child->getPosition());
					if (WFMath::Intersect(childSphere, entitySphere, false)) {
						//Ok, we have an entity which is close to our entity. Now check if any of the points of the bounding box is close.
						WFMath::RotBox<3> rotbox;
						rotbox.size() = bbox.highCorner() - bbox.lowCorner();
						rotbox.corner0() = bbox.lowCorner();
						rotbox.orientation().identity();
						rotbox.rotatePoint(child->getOrientation(), WFMath::Point<3>(0, 0, 0));
						rotbox.shift(WFMath::Vector<3>(child->getPosition()));

						for (size_t i = 0; i < rotbox.numCorners(); ++i) {
							WFMath::Point<3> point = rotbox.getCorner(i);
							Ogre::SceneNode* currentNode(nullptr);
							//If there is any unclaimed debug node left we'll use it to visualize the corner
							if (nodeIterator != mDebugNodes.end()) {
								currentNode = *nodeIterator;
								currentNode->setPosition(Convert::toOgre(point));
								currentNode->setVisible(true);
								nodeIterator++;
							}
							point.y() = 0;
							for (size_t j = 0; j < currentRotbox.numCorners(); ++j) {
								WFMath::Point<3> currentPoint = currentRotbox.getCorner(j);
								currentPoint.y() = 0;
								WFMath::CoordType distance = WFMath::Distance(currentPoint, point);
								if (distance <= mSnapThreshold) {
									if (currentNode) {
										auto* sphereEntity = dynamic_cast<Ogre::Entity*> (currentNode->getAttachedObject(0));
										if (sphereEntity) {
											try {
												sphereEntity->setMaterialName("/common/base/authoring/point/moved");
											} catch (const std::exception& ex) {
												logger->warn("Error when setting material for point: {}", ex.what());
											}
										}
									}
									if (!closestSnapping) {
										closestSnapping = std::make_unique<SnapPointCandidate>();
										closestSnapping->entity = dynamic_cast<EmberEntity*>(child);
										closestSnapping->distance = distance;
										closestSnapping->adjustment = point - currentPoint;
									} else if (distance < closestSnapping->distance) {
										closestSnapping->entity = dynamic_cast<EmberEntity*>(child);
										closestSnapping->distance = distance;
										closestSnapping->adjustment = point - currentPoint;
									}
								}
							}
						}
					}
				}
			}
		}
	}


//	WFMath::Ball<3> boundingSphere = mEntity.getBBox().boundingSphere();
//	Ogre::Sphere sphere(mNode._getDerivedPosition(), boundingSphere.radius() * 2);
//	Ogre::SphereSceneQuery* query = mSceneManager.createSphereQuery(sphere);
//	query->setQueryMask(MousePicker::CM_ENTITY);
//	Ogre::SceneQueryResult& result = query->execute();
//	for (auto& movable : result.movables) {
//		if (movable->getUserObjectBindings().getUserAny().type() == typeid(EmberEntityUserObject::SharedPtr)) {
//			EmberEntityUserObject* anUserObject = Ogre::any_cast<EmberEntityUserObject::SharedPtr>(movable->getUserObjectBindings().getUserAny()).get();
//			EmberEntity& entity = anUserObject->mEmberEntity;
//			if (&entity != &mEntity && entity.hasBBox()) {
//
//			}
//		}
//	}
	//mSceneManager.destroyQuery(query);
	if (closestSnapping) {
		adjustment = closestSnapping->adjustment;
		*snappedToEntity = closestSnapping->entity;
		return true;
	}
	return false;
}

}



