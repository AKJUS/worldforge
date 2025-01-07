//
// C++ Interface: CaelumSky
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2006
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
#ifndef EMBEROGRECAELUMSKY_H
#define EMBEROGRECAELUMSKY_H

#include "CaelumEnvironment.h"
#include "services/config/ConfigListenerContainer.h"


namespace Ember::OgreView::Environment {


/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class CaelumSky
		: CaelumEnvironmentComponent, public IFog, public ISky, ConfigListenerContainer {
public:
	explicit CaelumSky(CaelumEnvironment& environment);

	~CaelumSky() override;

	void setDensity(float density) override;

	float getDensity() const override;

	virtual bool frameEnded(const Ogre::FrameEvent& event);

protected:

	void Config_CloudSpeed(const std::string& section, const std::string& key, varconf::Variable& variable);

	void Config_CloudBlendTime(const std::string& section, const std::string& key, varconf::Variable& variable);

	void Config_CloudCover(const std::string& section, const std::string& key, varconf::Variable& variable);


};


}





#endif
