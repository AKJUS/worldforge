//
// C++ Interface: EntityCreationObserver
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
#ifndef EMBEROGRE_MODEL_MAPPING_MATCHES_OBSERVERSENTITYCREATIONOBSERVER_H
#define EMBEROGRE_MODEL_MAPPING_MATCHES_OBSERVERSENTITYCREATIONOBSERVER_H

#include <sigc++/trackable.h>
#include <Eris/View.h>

namespace Ember::EntityMapping::Matches {
class EntityRefMatch;
}

namespace Ember::EntityMapping::Matches::Observers {

/**
	Observes for the creation of a specific entity in the world and automatically trigger the Match the observer is attached to when that entity is created.
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class EntityCreationObserver : public virtual sigc::trackable {
public:

	explicit EntityCreationObserver(EntityRefMatch& entityRefMatch);

	~EntityCreationObserver();

	void observeCreation(Eris::View* view, const std::string& entityId);

protected:

	EntityRefMatch& mEntityRefMatch;

	Eris::View::EntitySightSlot mSlot;

	void entitySeen(Eris::Entity* entity);

};

}







#endif
