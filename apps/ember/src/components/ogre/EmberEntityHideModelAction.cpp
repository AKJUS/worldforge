//
// C++ Implementation: EmberEntityHideModelAction
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2008
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
#include "EmberEntityHideModelAction.h"
#include "domain/IGraphicalRepresentation.h"
#include "domain/IEntityAttachment.h"


namespace Ember::OgreView {

EmberEntityHideModelAction::EmberEntityHideModelAction(EmberEntity& entity)
		: mEntity(entity) {
}

EmberEntityHideModelAction::~EmberEntityHideModelAction() = default;

void EmberEntityHideModelAction::activate(EntityMapping::ChangeContext& context) {

	//TODO: is this the correct thing?
	mEntity.setAttachment({});
}

void EmberEntityHideModelAction::deactivate(EntityMapping::ChangeContext& context) {
}

}

