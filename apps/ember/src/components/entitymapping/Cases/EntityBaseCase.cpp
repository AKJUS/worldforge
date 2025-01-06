//
// C++ Implementation: EntityBaseCase
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2007
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
#include "EntityBaseCase.h"
#include <Eris/Entity.h>
#include <Eris/TypeInfo.h>

namespace Ember::EntityMapping::Cases {

bool EntityBaseCase::testMatch(Eris::Entity* entity) {
	//Check for entity types recursively for all of the supplied entity parents.
	if (entity) {
		Eris::TypeInfo* type = entity->getType();
		for (auto& entityType: mEntityTypes) {
			if (type->isA(entityType)) {
				_setState(true);
				return true;
			}
		}
	}
	_setState(false);
	return false;
}

void EntityBaseCase::addEntityType(Eris::TypeInfo* typeInfo) {
	mEntityTypes.push_back(typeInfo);
}


}




