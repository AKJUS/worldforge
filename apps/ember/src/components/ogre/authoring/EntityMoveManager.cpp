//
// C++ Implementation: EntityMoveManager
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2006
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

#include "EntityMoveManager.h"
#include "EntityMover.h"
#include "../GUIManager.h"
#include "../EmberOgre.h"
#include "components/ogre/World.h"
#include "components/ogre/NodeAttachment.h"
#include "components/ogre/Avatar.h"
#include "domain/EmberEntity.h"
#include "framework/Tokeniser.h"
#include "framework/ConsoleBackend.h"
#include "framework/MainLoopController.h"

#include <Eris/Avatar.h>




namespace Ember::OgreView::Authoring {

EntityMoveInstance::EntityMoveInstance(EmberEntity& entity,
									   MovementAdapter& moveAdapter,
									   sigc::signal<void()>& eventFinishedMoving,
									   sigc::signal<void()>& eventCancelledMoving) :
		EntityObserverBase(entity, true),
		mMoveAdapter(moveAdapter) {
	eventCancelledMoving.connect(sigc::mem_fun(*this, &EntityObserverBase::deleteOurselves));
	eventFinishedMoving.connect(sigc::mem_fun(*this, &EntityObserverBase::deleteOurselves));
}

void EntityMoveInstance::cleanup() {
	mMoveAdapter.detach();
}

EntityMoveManager::EntityMoveManager(World& world) :
		Move("move", this, "Moves an entity."),
		Place(ConsoleBackend::getSingleton(), "place", [&](const std::string& command, const std::string& args) { place(args); }, "Places an entity inside another."),
		mWorld(world),
		mMoveAdapter(world.getMainCamera()),
		mAdjuster(this, world.getEventService()) {
	GUIManager::getSingleton().EventEntityAction.connect(sigc::mem_fun(*this, &EntityMoveManager::GuiManager_EntityAction));
}

void EntityMoveManager::GuiManager_EntityAction(const std::string& action, EmberEntity* entity) {

	if (action == "move") {
		startMove(*entity);
	}
}

void EntityMoveManager::startMove(EmberEntity& entity) {
	//disallow moving of the root entity
	if (entity.getLocation() && entity.getAttachment()) {
		//Only provide movement for entities which have a node attachment.
		auto* attachment = dynamic_cast<NodeAttachment*> (entity.getAttachment().get());
		if (attachment) {
			auto mover = std::make_shared<EntityMover>(*attachment, *this);
			mMoveAdapter.attachToBridge(mover);
			//The EntityMoveInstance will delete itself when either movement is finished or the entity is deleted, so we don't need to hold a reference to it.
			//TODO: this feels unsafe; alter this so that the instance is owned by something
			new EntityMoveInstance(entity, mMoveAdapter, EventFinishedMoving, EventCancelledMoving);
			EventStartMoving.emit(entity, *mover);
		}
	}
}

void EntityMoveManager::runCommand(const std::string& command, const std::string& args) {
	if (Move == command) {
		//the first argument must be a valid entity id
		Tokeniser tokeniser;
		tokeniser.initTokens(args);
		std::string entityId = tokeniser.nextToken();
		if (!entityId.empty()) {
			EmberEntity* entity = mWorld.getEmberEntity(entityId);
			if (entity != nullptr) {
				startMove(*entity);
			}
		} else {
			ConsoleBackend::getSingletonPtr()->pushMessage("You must specify a valid entity id to move.", "error");
		}

	}
}

void EntityMoveManager::place(const std::string& args) {
	Tokeniser tokeniser;
	tokeniser.initTokens(args);
	std::string entityId = tokeniser.nextToken();
	std::string newContainerId = tokeniser.nextToken();
	if (!entityId.empty() && !newContainerId.empty()) {
		auto entity = mWorld.getEmberEntity(entityId);
		auto newContainer = mWorld.getEmberEntity(newContainerId);
		if (entity && newContainer) {
			mWorld.getAvatar()->getErisAvatar().place(entity, newContainer);
		}
	}
}


World& EntityMoveManager::getWorld() const {
	return mWorld;
}

void EntityMoveManager::delayedUpdatePositionForEntity(const std::string& entityId) {
	MainLoopController::getSingleton().getEventService().runOnMainThreadDelayed([this, entityId] {
		auto entity = mWorld.getEmberEntity(entityId);
		if (entity) {
			if (entity->getAttachment()) {
				entity->getAttachment()->updatePosition();
			}
		}
	}, std::chrono::seconds(1));
}


}



