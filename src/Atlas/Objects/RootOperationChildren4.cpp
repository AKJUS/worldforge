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

Allocator<ImaginaryData> ImaginaryData::allocator;
        


void ImaginaryData::free()
{
    allocator.free(this);
}



void ImaginaryData::reset()
{
    ActionData::reset();
}

ImaginaryData * ImaginaryData::copy() const
{
    ImaginaryData * copied = allocator.alloc();
    *copied = *this;
    copied->m_refCount = 0;
    return copied;
}

bool ImaginaryData::instanceOf(int classNo) const
{
    if(IMAGINARY_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void ImaginaryData::fillDefaultObjectInstance(ImaginaryData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<UseData> UseData::allocator;
        


void UseData::free()
{
    allocator.free(this);
}



void UseData::reset()
{
    ActionData::reset();
}

UseData * UseData::copy() const
{
    UseData * copied = allocator.alloc();
    *copied = *this;
    copied->m_refCount = 0;
    return copied;
}

bool UseData::instanceOf(int classNo) const
{
    if(USE_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void UseData::fillDefaultObjectInstance(UseData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<ActivityData> ActivityData::allocator;
        


void ActivityData::free()
{
    allocator.free(this);
}



void ActivityData::reset()
{
    ActionData::reset();
}

ActivityData * ActivityData::copy() const
{
    ActivityData * copied = allocator.alloc();
    *copied = *this;
    copied->m_refCount = 0;
    return copied;
}

bool ActivityData::instanceOf(int classNo) const
{
    if(ACTIVITY_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void ActivityData::fillDefaultObjectInstance(ActivityData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<InfoData> InfoData::allocator;
        


void InfoData::free()
{
    allocator.free(this);
}



void InfoData::reset()
{
    RootOperationData::reset();
}

InfoData * InfoData::copy() const
{
    InfoData * copied = allocator.alloc();
    *copied = *this;
    copied->m_refCount = 0;
    return copied;
}

bool InfoData::instanceOf(int classNo) const
{
    if(INFO_NO == classNo) return true;
    return RootOperationData::instanceOf(classNo);
}

void InfoData::fillDefaultObjectInstance(InfoData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<PerceptionData> PerceptionData::allocator;
        


void PerceptionData::free()
{
    allocator.free(this);
}



void PerceptionData::reset()
{
    InfoData::reset();
}

PerceptionData * PerceptionData::copy() const
{
    PerceptionData * copied = allocator.alloc();
    *copied = *this;
    copied->m_refCount = 0;
    return copied;
}

bool PerceptionData::instanceOf(int classNo) const
{
    if(PERCEPTION_NO == classNo) return true;
    return InfoData::instanceOf(classNo);
}

void PerceptionData::fillDefaultObjectInstance(PerceptionData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<UnseenData> UnseenData::allocator;
        


void UnseenData::free()
{
    allocator.free(this);
}



void UnseenData::reset()
{
    PerceptionData::reset();
}

UnseenData * UnseenData::copy() const
{
    UnseenData * copied = allocator.alloc();
    *copied = *this;
    copied->m_refCount = 0;
    return copied;
}

bool UnseenData::instanceOf(int classNo) const
{
    if(UNSEEN_NO == classNo) return true;
    return PerceptionData::instanceOf(classNo);
}

void UnseenData::fillDefaultObjectInstance(UnseenData& data, std::map<std::string, uint32_t>& attr_data)
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
