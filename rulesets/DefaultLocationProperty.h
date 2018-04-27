/*
 Copyright (C) 2014 Erik Ogenvik

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
#ifndef DEFAULTLOCATIONPROPERTY_H_
#define DEFAULTLOCATIONPROPERTY_H_

#include "common/Property.h"

/**
 * \brief The entity on which this is set is designated the "default location" when entities enter the world.
 * \ingroup PropertyClasses
 */
class DefaultLocationProperty : public BoolProperty
{
    public:
        static constexpr const char* property_name = "default_location";

        explicit DefaultLocationProperty() = default;

        DefaultLocationProperty* copy() const override;

        void install(LocatedEntity*, const std::string&) override;

        void remove(LocatedEntity*, const std::string&) override;
};

#endif /* DEFAULTLOCATIONPROPERTY_H_ */
