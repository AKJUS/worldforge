/*
 Copyright (C) 2004  Erik Ogenvik

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

#include "EmberEntityFactory.h"

#include <Atlas/Objects/RootEntity.h>

#include "domain/EmberEntity.h"

#include "services/config/ConfigService.h"

#include <Eris/View.h>
#include <Eris/TypeInfo.h>
#include <framework/Log.h>

namespace Ember::OgreView {

EmberEntityFactory::EmberEntityFactory(Eris::View& view, Scene& scene) :
		mView(view),
		mScene(scene) {
}

EmberEntityFactory::~EmberEntityFactory() = default;

// create whatever entity the client desires
std::unique_ptr<Eris::Entity> EmberEntityFactory::instantiate(const Atlas::Objects::Entity::RootEntity& ge, Eris::TypeInfo* type, Eris::View& w) {

	Eris::Entity::EntityContext context{
			.fetchEntity = [this](const std::string& id) {
				auto entity = mView.getEntity(id);
				if (!entity) {
					// we don't have the entity at all, so request it and skip
					// processing it here; everything will come right when it
					// arrives.
					mView.getEntityFromServer(id);
				}
				return entity;
			},
			.getEntity= [this](const std::string& id) {
				return mView.getEntity(id);
			},
			.taskUpdated = [this](Eris::Task& task) {
				mView.taskRateChanged(&task);
			}
	};

	auto entity = std::make_unique<EmberEntity>(ge->getId(), type, context);
	logger->debug("Entity {} ({}) added to game view.", entity->getId(), type->getName());
	return entity;
}


bool EmberEntityFactory::accept(const Atlas::Objects::Entity::RootEntity& ge, Eris::TypeInfo* type) {
	return true;
}

int EmberEntityFactory::priority() {
	return 10;
}

}


