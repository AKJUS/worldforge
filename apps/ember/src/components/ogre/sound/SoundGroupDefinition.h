//
// C++ Interface: SoundGroupDefinition
//
// Description: 
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2008
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
#ifndef EMBEROGRESOUNDGROUPDEFINITION_H
#define EMBEROGRESOUNDGROUPDEFINITION_H

#include "services/sound/SoundGeneral.h"
#include <map>
#include <list>


namespace Ember::OgreView {
struct SoundDefinition;

/**
 * @brief Describes a set of sound samples (SoundDefinitions).
 */
struct SoundGroupDefinition {
	/**
	 * @brief The sounds defined for this group.
	 */
	std::list<SoundDefinition> mSamples;
};

}


#endif
