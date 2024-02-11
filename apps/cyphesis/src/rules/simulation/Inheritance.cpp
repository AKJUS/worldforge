// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000-2004 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#include "Inheritance.h"

#include "common/log.h"
#include "common/TypeNode_impl.h"
#include "common/AtlasFactories.h"
#include "common/custom_impl.h"

#include <Atlas/Objects/Entity.h>
#include <Atlas/Objects/RootOperation.h>
#include <Atlas/Objects/Operation.h>

using Atlas::Message::Element;
using Atlas::Message::ListType;

using Atlas::Objects::Root;

using Atlas::Objects::Operation::Login;
using Atlas::Objects::Operation::Logout;
using Atlas::Objects::Operation::Action;
using Atlas::Objects::Operation::Affect;
using Atlas::Objects::Operation::Create;
using Atlas::Objects::Operation::Delete;
using Atlas::Objects::Operation::Imaginary;
using Atlas::Objects::Operation::Set;
using Atlas::Objects::Operation::Get;
using Atlas::Objects::Operation::Sound;
using Atlas::Objects::Operation::Touch;
using Atlas::Objects::Operation::Talk;
using Atlas::Objects::Operation::Use;
using Atlas::Objects::Operation::Wield;
using Atlas::Objects::Operation::Look;
using Atlas::Objects::Operation::Error;
using Atlas::Objects::Operation::Use;
using Atlas::Objects::Operation::Wield;


Inheritance::Inheritance(Atlas::Objects::Factories& factories)
		: noClass(nullptr),
		  m_factories(factories) {
	Atlas::Objects::Entity::Anonymous root_desc;

	root_desc->setObjtype("meta");
	root_desc->setId(Atlas::Objects::RootData::default_parent);

	auto root = std::make_unique<TypeNode<LocatedEntity>>(root_desc->getId(), root_desc);

	atlasObjects.emplace("root", std::move(root));

	installStandardObjects(*this);
	installCustomOperations(*this);
	installCustomEntities(*this);
}


Inheritance::~Inheritance() {
	flush();
}

void Inheritance::flush() {
	atlasObjects.clear();
}

const Root& Inheritance::getClass(const std::string& parent, Visibility visibility) const {
	auto I = atlasObjects.find(parent);
	if (I == atlasObjects.end()) {
		return noClass;
	}
	return I->second->description(visibility);
}

int Inheritance::updateClass(const std::string& parent,
							 const Root& description) {
	auto I = atlasObjects.find(parent);
	if (I == atlasObjects.end()) {
		return -1;
	}
	auto& tn = I->second;
	if (tn->description(Visibility::PRIVATE)->getParent() != description->getParent()) {
		return -1;
	}
	tn->setDescription(description);
	return 0;
}

const TypeNode<LocatedEntity>* Inheritance::getType(const std::string& parent) const {
	auto I = atlasObjects.find(parent);
	if (I == atlasObjects.end()) {
		return nullptr;
	}
	return I->second.get();
}

bool Inheritance::hasClass(const std::string& parent) {
	auto I = atlasObjects.find(parent);
	return !(I == atlasObjects.end());
}

TypeNode<LocatedEntity>* Inheritance::addChild(const Root& obj) {
	assert(obj.isValid() && !obj->getParent().empty());
	const std::string& child = obj->getId();
	const std::string& parent = obj->getParent();
	auto I = atlasObjects.find(child);
	auto Iend = atlasObjects.end();
	if (I != Iend) {

		const TypeNode<LocatedEntity>* existingParent = I->second->parent();
		spdlog::error("Installing {} \"{}\"(parent \"{}\") "
					  "which was already installed as a {} with parent \"{}\"",
					  obj->getObjtype(), child, parent,
					  I->second->description(Visibility::PRIVATE)->getObjtype(),
					  existingParent ? existingParent->name() : "NON");
		return nullptr;
	}
	I = atlasObjects.find(parent);
	if (I == Iend) {
		spdlog::error("Installing {} \"{}\" "
					  "which has unknown parent \"{}\".",
					  obj->getObjtype(), child, parent);
		return nullptr;
	}
	Element children(ListType(1, child));

	auto description = I->second->description(Visibility::PRIVATE);

	if (description->copyAttr("children", children) == 0) {
		assert(children.isList());
		children.asList().emplace_back(child);
	}
	description->setAttr("children", children);
	I->second->setDescription(description);

	auto type = std::make_unique<TypeNode<LocatedEntity>>(child, obj);
	type->setParent(I->second.get());

	auto result = atlasObjects.emplace(child, std::move(type));

	return result.first->second.get();
}

bool Inheritance::isTypeOf(const std::string& instance,
						   const std::string& base_type) const {
	auto I = atlasObjects.find(instance);
	auto Iend = atlasObjects.end();
	if (I == Iend) {
		return false;
	}
	return I->second->isTypeOf(base_type);
}

bool Inheritance::isTypeOf(const TypeNode<LocatedEntity>* instance,
						   const std::string& base_type) const {
	return instance->isTypeOf(base_type);
}

bool Inheritance::isTypeOf(const TypeNode<LocatedEntity>* instance,
						   const TypeNode<LocatedEntity>* base_type) const {
	return instance->isTypeOf(base_type);
}

using Atlas::Objects::Operation::RootOperation;
using Atlas::Objects::Operation::Perception;
using Atlas::Objects::Operation::Communicate;
using Atlas::Objects::Operation::Perceive;
using Atlas::Objects::Operation::Smell;
using Atlas::Objects::Operation::Feel;
using Atlas::Objects::Operation::Listen;
using Atlas::Objects::Operation::Sniff;

using Atlas::Objects::Entity::RootEntity;
using Atlas::Objects::Entity::AdminEntity;
using Atlas::Objects::Entity::Game;
using Atlas::Objects::Entity::GameEntity;


const Atlas::Objects::Factories& Inheritance::getFactories() const {
	return m_factories;
}

Atlas::Objects::Factories& Inheritance::getFactories() {
	return m_factories;
}

