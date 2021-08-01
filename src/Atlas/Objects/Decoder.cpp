// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright 2000-2001 Stefanus Du Toit and Aloril.
// Copyright 2001-2005 Alistair Riddoch.
// Copyright 2019 Erik Ogenvik.

#include <Atlas/Objects/Decoder.h>

#include <Atlas/Objects/Factories.h>

namespace Atlas {
namespace Objects {

ObjectsDecoder::ObjectsDecoder(const Factories& f)
		: m_factories(f) {
}

ObjectsDecoder::~ObjectsDecoder() = default;

void ObjectsDecoder::messageArrived(Atlas::Message::MapType o) {
	objectArrived(m_factories.createObject(std::move(o)));
}

}
} // namespace Atlas::Objects
