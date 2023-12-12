/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef IENTITYCONTROLDELEGATE_H_
#define IENTITYCONTROLDELEGATE_H_

namespace WFMath {
template<int>
class Point;

template<int>
class Vector;

class Quaternion;
}

namespace Ember {

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief Allows control of an entity to be delegated.
 *
 * Under normal operations the position and orientation for an entity used for displaying ingame graphical representations would be obtained directly from the entity. However, by using an instance of this class it's possible to redirect this.
 *
 */
struct IEntityControlDelegate {

	/**
	 * @brief Dtor.
	 */
	virtual ~IEntityControlDelegate() = default;

	/**
	 * @brief Gets the position.
	 * @return The position, in world units and local space.
	 */
	virtual const WFMath::Point<3>& getPosition() const = 0;

	/**
	 * @brief Gets the orientation.
	 * @return The orientation, in local space.
	 */
	virtual const WFMath::Quaternion& getOrientation() const = 0;

	/**
	 * @brief Gets the velocity.
	 * @return The velocity, in world units.
	 */
	virtual const WFMath::Vector<3>& getVelocity() const = 0;
};

}

#endif /* IENTITYCONTROLDELEGATE_H_ */
