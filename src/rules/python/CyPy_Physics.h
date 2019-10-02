/*
 Copyright (C) 2018 Erik Ogenvik

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

#ifndef CYPHESIS_CYPY_PHYSICS_H
#define CYPHESIS_CYPY_PHYSICS_H

#include "pycxx/CXX/Objects.hxx"
#include "pycxx/CXX/Extensions.hxx"

class CyPy_Physics : public Py::ExtensionModule<CyPy_Physics>
{

    public:
        CyPy_Physics();

        Py::Object square_horizontal_edge_distance(const Py::Tuple& args);

        Py::Object square_horizontal_distance(const Py::Tuple& args);

        Py::Object square_distance(const Py::Tuple& args);

        Py::Object distance_to(const Py::Tuple& args);

        Py::Object distance_between(const Py::Tuple& args);

        static std::string init();
};


#endif //CYPHESIS_CYPY_PHYSICS_H
