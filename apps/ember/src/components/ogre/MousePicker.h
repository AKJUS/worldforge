//
// C++ Interface: MousePicker
//
// Description: 
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2004
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
#ifndef EMBEROGRE_MOUSEPICKER_H
#define EMBEROGRE_MOUSEPICKER_H

#include <sigc++/signal.h>


#include "IWorldPickListener.h"
#include "EmberOgrePrerequisites.h"


namespace Ember::OgreView {


/**
Class used for picking stuff in the world.
Since we sometimes want different picking behaviour (sometimes we want to pick building blocks, sometimes we want to pick entities) it's possible to create derived classes and register them with the GUIManager.

@author Erik Ogenvik
*/
struct MousePicker {
	enum ClickMasks {
		CM_AVATAR = 1u << 9u,
		CM_ENTITY = 1u << 10u,
		CM_NATURE = 1u << 11u,
		CM_UNDEFINED = 1u << 12u,
		CM_NONPICKABLE = 1u << 13u
	};
};

}



#endif
