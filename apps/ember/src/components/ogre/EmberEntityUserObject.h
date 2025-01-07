//
// C++ Interface: EmberEntityUserObject
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2005
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.//
//
#ifndef EMBEROGREEMBERENTITYUSEROBJECT_H
#define EMBEROGREEMBERENTITYUSEROBJECT_H

#include <memory>

namespace Ogre {
class Entity;
}


namespace Ember {
class EmberEntity;
namespace OgreView {

namespace Model {
class Model;
}

/**
 * @author Erik Ogenvik
 *
 * @brief An Ogre user object which connects to an EmberEntity.
 * Instances of this class can be attached to movable objects in the ogre system. They will allow for the Ember system to be accessed directly from Ogre, without having to do lookups.
 * This is generally mostly used for mouse picking and collision handling.
 */
struct EmberEntityUserObject {
	typedef std::shared_ptr<EmberEntityUserObject> SharedPtr;
	/**
	 * @brief The entity to which this user object belongs.
	 */
	EmberEntity& mEmberEntity;

};

}
}

#endif
