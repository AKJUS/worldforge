//
// C++ Implementation: EntityCreationObserver
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
#include "EntityCreationObserver.h"
#include "components/entitymapping/Matches/EntityRefMatch.h"

namespace Ember::EntityMapping::Matches::Observers {

EntityCreationObserver::EntityCreationObserver(EntityRefMatch& entityRefMatch)
		: mEntityRefMatch(entityRefMatch) {
}

EntityCreationObserver::~EntityCreationObserver() {
	mSlot.disconnect();
}

void EntityCreationObserver::observeCreation(Eris::View* view, const std::string& entityId) {
	mSlot.disconnect();
	mSlot = sigc::mem_fun(*this, &EntityCreationObserver::entitySeen);
	view->notifyWhenEntitySeen(entityId, mSlot);
}

void EntityCreationObserver::entitySeen(Eris::Entity* entity) {
	mSlot.disconnect();
	mEntityRefMatch.testEntity(entity);
}


}






