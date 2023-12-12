//
// C++ Interface: ParticleSystemBinding
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
#ifndef EMBEROGREPARTICLESYSTEMBINDING_H
#define EMBEROGREPARTICLESYSTEMBINDING_H

#include "components/ogre/EmberOgrePrerequisites.h"
#include "ModelDefinition.h"
#include <string>


namespace Ember::OgreView::Model {

class ParticleSystem;

/**
@author Erik Ogenvik
*/
struct ParticleSystemBinding {

	ModelDefinition::ParticleSystemSetting mEmitterVal;
	std::string mVariableName;
	ParticleSystem* mParticleSystem;
	Ogre::Real mOriginalValue;

	void scaleValue(Ogre::Real scaler) const;

	static void updateSettings(Ogre::ParticleSystem& particleSystem, ModelDefinition::ParticleSystemSetting setting, float value);

};


}


#endif
