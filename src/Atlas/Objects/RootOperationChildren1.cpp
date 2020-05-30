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

Allocator<ActionData> ActionData::allocator;
        


void ActionData::free()
{
    allocator.free(this);
}



void ActionData::reset()
{
    RootOperationData::reset();
}

ActionData * ActionData::copy() const
{
    return copyInstance<ActionData>(*this);
}

bool ActionData::instanceOf(int classNo) const
{
    if(ACTION_NO == classNo) return true;
    return RootOperationData::instanceOf(classNo);
}

void ActionData::fillDefaultObjectInstance(ActionData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<CreateData> CreateData::allocator;
        


void CreateData::free()
{
    allocator.free(this);
}



void CreateData::reset()
{
    ActionData::reset();
}

CreateData * CreateData::copy() const
{
    return copyInstance<CreateData>(*this);
}

bool CreateData::instanceOf(int classNo) const
{
    if(CREATE_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void CreateData::fillDefaultObjectInstance(CreateData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<CommunicateData> CommunicateData::allocator;
        


void CommunicateData::free()
{
    allocator.free(this);
}



void CommunicateData::reset()
{
    CreateData::reset();
}

CommunicateData * CommunicateData::copy() const
{
    return copyInstance<CommunicateData>(*this);
}

bool CommunicateData::instanceOf(int classNo) const
{
    if(COMMUNICATE_NO == classNo) return true;
    return CreateData::instanceOf(classNo);
}

void CommunicateData::fillDefaultObjectInstance(CommunicateData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<TalkData> TalkData::allocator;
        


void TalkData::free()
{
    allocator.free(this);
}



void TalkData::reset()
{
    CommunicateData::reset();
}

TalkData * TalkData::copy() const
{
    return copyInstance<TalkData>(*this);
}

bool TalkData::instanceOf(int classNo) const
{
    if(TALK_NO == classNo) return true;
    return CommunicateData::instanceOf(classNo);
}

void TalkData::fillDefaultObjectInstance(TalkData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<DeleteData> DeleteData::allocator;
        


void DeleteData::free()
{
    allocator.free(this);
}



void DeleteData::reset()
{
    ActionData::reset();
}

DeleteData * DeleteData::copy() const
{
    return copyInstance<DeleteData>(*this);
}

bool DeleteData::instanceOf(int classNo) const
{
    if(DELETE_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void DeleteData::fillDefaultObjectInstance(DeleteData& data, std::map<std::string, uint32_t>& attr_data)
{
        data.attr_objtype = default_objtype;
        data.attr_serialno = 0;
        data.attr_refno = 0;
        data.attr_seconds = 0.0;
        data.attr_future_seconds = 0.0;
        data.attr_stamp = 0.0;
        data.attr_parent = default_parent;
}

Allocator<SetData> SetData::allocator;
        


void SetData::free()
{
    allocator.free(this);
}



void SetData::reset()
{
    ActionData::reset();
}

SetData * SetData::copy() const
{
    return copyInstance<SetData>(*this);
}

bool SetData::instanceOf(int classNo) const
{
    if(SET_NO == classNo) return true;
    return ActionData::instanceOf(classNo);
}

void SetData::fillDefaultObjectInstance(SetData& data, std::map<std::string, uint32_t>& attr_data)
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
