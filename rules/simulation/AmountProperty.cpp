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

#include "AmountProperty.h"

#include "rules/LocatedEntity.h"
#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Entity.h>

AmountProperty* AmountProperty::copy() const
{
    return new AmountProperty(*this);
}

void AmountProperty::apply(LocatedEntity* entity)
{
    if (data() <= 0) {
        Atlas::Objects::Operation::Delete del;
        del->setTo(entity->getId());
        del->setFrom(entity->getId());
        Atlas::Objects::Entity::Anonymous ent;
        ent->setId(entity->getId());
        del->setArgs1(ent);
        //Apply the delete op immediately to avoid an interim time where there's too many entities.
        OpVector res;
        entity->operation(del, res);
        for (const auto& resOp : res) {
            entity->sendWorld(resOp);
        }
    }
}
