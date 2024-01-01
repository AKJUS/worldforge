//
// C++ Implementation: EntityCreatorMovementBridge
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
#include "domain/EmberEntity.h"
#include "EntityCreatorMovementBridge.h"
#include "EntityCreatorCreationInstance.h"
#include "components/ogre/authoring/DetachedEntity.h"
#include "components/ogre/EntityCollisionInfo.h"
#include <OgreSceneNode.h>


namespace Ember::OgreView::Gui {

EntityCreatorMovementBridge::EntityCreatorMovementBridge(EntityCreatorCreationInstance& creationInstance,
														 Authoring::DetachedEntity* entity,
														 Ogre::SceneNode* node) :
		Authoring::EntityMoverBase(entity, node, *node->getCreator()),
		mCreationInstance(creationInstance) {
}


void EntityCreatorMovementBridge::finalizeMovement() {
	mCreationInstance.EventFinalizeRequested();
}

void EntityCreatorMovementBridge::cancelMovement() {
	mCreationInstance.EventAbortRequested();
}


void EntityCreatorMovementBridge::processPickResults(const std::vector<PickResult>& results) {
	if (mEntity) {
		for (auto& result: results) {
			if (result.collisionInfo.type() == typeid(EntityCollisionInfo)) {
				auto& entityCollisionInfo = std::any_cast<const EntityCollisionInfo&>(result.collisionInfo);
				//It's a valid entry if it's not transparent and not the entity which is being moved itself.
				if (!entityCollisionInfo.isTransparent && entityCollisionInfo.entity != mEntity.get()) {
					if (mFixedParentId && *mFixedParentId != entityCollisionInfo.entity->getId()) {
						continue;
					}
					mCollidedEntity = Eris::EntityRef(entityCollisionInfo.entity);
					setPosition(result.point);
					return;
				}
			}
		}
	}
	mCollidedEntity = {};
	setPosition({});
}


}



