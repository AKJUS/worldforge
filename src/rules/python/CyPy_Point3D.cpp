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

#include <physics/Vector3D.h>
#include "CyPy_Point3D.h"
#include "CyPy_Vector3D.h"
#include "CoordHelper.h"

CyPy_Point3D::CyPy_Point3D(Py::PythonClassInstance* self, Py::Tuple& args, Py::Dict& kwds)
        : WrapperBase(self, args, kwds)
{
    CoordHelper::init(m_value, args);
}

CyPy_Point3D::CyPy_Point3D(Py::PythonClassInstance* self, WFMath::Point<3> value)
        : WrapperBase(self, std::move(value))
{

}

void CyPy_Point3D::init_type()
{
    behaviors().name("Point3D");
    behaviors().doc("");

    behaviors().supportRepr();
    behaviors().supportRichCompare();

    behaviors().supportSequenceType(Py::PythonType::support_sequence_item
                                    | Py::PythonType::support_sequence_length
                                    | Py::PythonType::support_sequence_ass_item);

    behaviors().supportNumberType(Py::PythonType::support_number_add
                                  | Py::PythonType::support_number_subtract);

    PYCXX_ADD_VARARGS_METHOD(unit_vector_to, unit_vector_to, "");
    PYCXX_ADD_VARARGS_METHOD(distance, distance, "");

    PYCXX_ADD_NOARGS_METHOD(mag, mag, "");
    PYCXX_ADD_NOARGS_METHOD(is_valid, is_valid, "");

    behaviors().readyType();
}

WFMath::Point<3> CyPy_Point3D::parse(const Py::Object& object)
{
    return CoordHelper::parse<WFMath::Point<3>, CyPy_Point3D>(object);
}

Py::Object CyPy_Point3D::getattro(const Py::String& name)
{
    return CoordHelper::getattro(*this, name);
}

int CyPy_Point3D::setattro(const Py::String& name, const Py::Object& attr)
{
    return CoordHelper::setattro(m_value, name, attr);
}

Py::Object CyPy_Point3D::repr()
{
    return CoordHelper::repr(m_value);
}

Py::Object CyPy_Point3D::rich_compare(const Py::Object& other, int type)
{
    return CoordHelper::rich_compare<decltype(m_value), CyPy_Point3D>(m_value, other, type);
}

PyCxx_ssize_t CyPy_Point3D::sequence_length()
{
    return 3;
}

Py::Object CyPy_Point3D::sequence_item(Py_ssize_t pos)
{
    return CoordHelper::sequence_item(m_value, pos);
}

int CyPy_Point3D::sequence_ass_item(Py_ssize_t pos, const Py::Object& other)
{
    return CoordHelper::sequence_ass_item(m_value, pos, other);
}

Py::Object CyPy_Point3D::number_add(const Py::Object& other)
{
    if (CyPy_Vector3D::check(other)) {
        return CyPy_Point3D::wrap(m_value + CyPy_Vector3D::value(other));
    }
    if (CyPy_Point3D::check(other)) {
        return CyPy_Point3D::wrap(m_value + WFMath::Vector<3>(CyPy_Point3D::value(other)));
    }

    throw Py::TypeError("Can only add Vector3D or Point3D to Point3D");

}

Py::Object CyPy_Point3D::number_subtract(const Py::Object& other)
{
    if (CyPy_Vector3D::check(other)) {
        return CyPy_Point3D::wrap(m_value - CyPy_Vector3D::value(other));
    }
    if (CyPy_Point3D::check(other)) {
        return CyPy_Vector3D::wrap(m_value - CyPy_Point3D::value(other));
    }
    throw Py::TypeError("Can only subtract Vector3D or Point3D from Point3D");
}

Py::Object CyPy_Point3D::mag()
{
    return Py::Float(std::sqrt(sqrMag(m_value)));
}

Py::Object CyPy_Point3D::unit_vector_to(const Py::Tuple& args)
{
    args.verify_length(1, 1);
    auto other = args.front();
    if (!CyPy_Point3D::check(other)) {
        throw Py::TypeError("Can get unit vector to Point3D");
    }
    auto vector = CyPy_Point3D::value(other) - m_value;
    vector.normalize();
    return CyPy_Vector3D::wrap(std::move(vector));
}

Py::Object CyPy_Point3D::distance(const Py::Tuple& args)
{
    args.verify_length(1, 1);
    auto other = args.front();
    if (!CyPy_Point3D::check(other)) {
        throw Py::TypeError("Can get distance to other Point3D");
    }
    return Py::Float(::distance(m_value, CyPy_Point3D::value(other)));
}

Py::Object CyPy_Point3D::is_valid()
{
    return CoordHelper::is_valid(m_value);
}
