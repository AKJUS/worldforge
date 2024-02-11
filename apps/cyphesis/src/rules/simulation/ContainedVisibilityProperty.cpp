/*
 Copyright (C) 2019 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ContainedVisibilityProperty.h"
#include "rules/simulation/LocatedEntity.h"
#include "common/Property_impl.h"

ContainedVisibilityProperty* ContainedVisibilityProperty::copy() const {
	return new ContainedVisibilityProperty(*this);
}

void ContainedVisibilityProperty::apply(LocatedEntity& ent) {
	if (isTrue()) {
		ent.addFlags(entity_contained_visible);
	} else {
		ent.removeFlags(entity_contained_visible);
	}
}

void ContainedVisibilityProperty::remove(LocatedEntity& ent, const std::string& name) {
	ent.removeFlags(entity_contained_visible);
}

