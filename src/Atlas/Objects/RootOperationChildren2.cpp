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

Allocator<AffectData> AffectData::allocator;
        


void AffectData::free()
{
    allocator.free(this);
}



void AffectData::reset()
{
    SetData::reset();
}

AffectData * AffectData::copy() const
{
    return copyInstance<AffectData>(*this);
}

bool AffectData::instanceOf(int classNo) const
{
    if(AFFECT_NO == classNo) return true;
    return SetData::instanceOf(classNo);
}

void AffectData::fillDefaultObjectInstance(AffectData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<HitData> HitData::allocator;
        


void HitData::free()
{
    allocator.free(this);
}



void HitData::reset()
{
    AffectData::reset();
}

HitData * HitData::copy() const
{
    return copyInstance<HitData>(*this);
}

bool HitData::instanceOf(int classNo) const
{
    if(HIT_NO == classNo) return true;
    return AffectData::instanceOf(classNo);
}

void HitData::fillDefaultObjectInstance(HitData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<MoveData> MoveData::allocator;
        


void MoveData::free()
{
    allocator.free(this);
}



void MoveData::reset()
{
    SetData::reset();
}

MoveData * MoveData::copy() const
{
    return copyInstance<MoveData>(*this);
}

bool MoveData::instanceOf(int classNo) const
{
    if(MOVE_NO == classNo) return true;
    return SetData::instanceOf(classNo);
}

void MoveData::fillDefaultObjectInstance(MoveData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<WieldData> WieldData::allocator;
        


void WieldData::free()
{
    allocator.free(this);
}



void WieldData::reset()
{
    SetData::reset();
}

WieldData * WieldData::copy() const
{
    return copyInstance<WieldData>(*this);
}

bool WieldData::instanceOf(int classNo) const
{
    if(WIELD_NO == classNo) return true;
    return SetData::instanceOf(classNo);
}

void WieldData::fillDefaultObjectInstance(WieldData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<GetData> GetData::allocator;
        


void GetData::free()
{
    allocator.free(this);
}



void GetData::reset()
{
    ActionData::reset();
}

GetData * GetData::copy() const
{
    return copyInstance<GetData>(*this);
}

bool GetData::instanceOf(int classNo) const
{
    if(GET_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void GetData::fillDefaultObjectInstance(GetData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<PerceiveData> PerceiveData::allocator;
        


void PerceiveData::free()
{
    allocator.free(this);
}



void PerceiveData::reset()
{
    GetData::reset();
}

PerceiveData * PerceiveData::copy() const
{
    return copyInstance<PerceiveData>(*this);
}

bool PerceiveData::instanceOf(int classNo) const
{
    if(PERCEIVE_NO == classNo) return true;
    return GetData::instanceOf(classNo);
}

void PerceiveData::fillDefaultObjectInstance(PerceiveData& data, std::map<std::string, uint32_t>& attr_data)
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
