// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright 2000-2001 Stefanus Du Toit and Aloril.
// Copyright 2001-2005 Alistair Riddoch.
// Copyright 2011-2019 Erik Ogenvik.
// Automatically generated using gen_cpp.py. Don't edit directly.

#include <Atlas/Objects/Operation.h>

using Atlas::Message::Element;
using Atlas::Message::MapType;

namespace Atlas { namespace Objects { namespace Operation { 

Allocator<ErrorData> ErrorData::allocator;
        


void ErrorData::free()
{
    allocator.free(this);
}



void ErrorData::reset()
{
    InfoData::reset();
}

ErrorData * ErrorData::copy() const
{
    return copyInstance<ErrorData>(*this);
}

bool ErrorData::instanceOf(int classNo) const
{
    if(ERROR_NO == classNo) return true;
    return InfoData::instanceOf(classNo);
}

void ErrorData::fillDefaultObjectInstance(ErrorData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<ChangeData> ChangeData::allocator;
        


void ChangeData::free()
{
    allocator.free(this);
}



void ChangeData::reset()
{
    InfoData::reset();
}

ChangeData * ChangeData::copy() const
{
    return copyInstance<ChangeData>(*this);
}

bool ChangeData::instanceOf(int classNo) const
{
    if(CHANGE_NO == classNo) return true;
    return InfoData::instanceOf(classNo);
}

void ChangeData::fillDefaultObjectInstance(ChangeData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

} } } // namespace Atlas::Objects::Operation
