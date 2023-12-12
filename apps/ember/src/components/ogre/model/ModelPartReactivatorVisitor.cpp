/*
 Copyright (C) 2009 Erik Ogenvik

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

#include "ModelPartReactivatorVisitor.h"

#include "components/entitymapping/EntityMapping.h"
#include "components/ogre/EmberEntityPartAction.h"


namespace Ember::OgreView::Model {

void ModelPartReactivatorVisitor::visit(EntityMapping::Actions::Action& action) {
	auto* partAction = dynamic_cast<EmberEntityPartAction*> (&action);
	if (partAction) {
		if (partAction->getCase()->getIsActive()) {
			partAction->activate(mChangeContext);
		}
	}
}

void ModelPartReactivatorVisitor::visit(EntityMapping::Matches::MatchBase& match) {

}

void ModelPartReactivatorVisitor::visit(EntityMapping::Cases::CaseBase& caseBase) {

}

}


