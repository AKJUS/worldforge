//
// C++ Interface: CaelumSun
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
#ifndef EMBEROGRECAELUMSUN_H
#define EMBEROGRECAELUMSUN_H

#include "CaelumEnvironment.h"
#include "services/config/ConfigListenerContainer.h"

namespace Ember {
class ConfigListener;
}

namespace varconf {
class Variable;
}





namespace Ember::OgreView::Environment {

/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class CaelumSun : public CaelumEnvironmentComponent, public ISun, public ConfigListenerContainer {
public:
	CaelumSun(CaelumEnvironment& environment, Caelum::BaseSkyLight* sun);

	~CaelumSun() override;

	void setAmbientLight(const Ogre::ColourValue& colour) override;

	Ogre::Vector3 getSunDirection() const override;

	WFMath::Vector<3> getMainLightDirection() const override;

	Ogre::ColourValue getAmbientLightColour() const override;

private:

	Caelum::BaseSkyLight* mSun;


	/**
	 *    Listend for changes to the config.
	 * @param section
	 * @param key
	 * @param variable
	 */
	void Config_SunAmbientMultiplier(const std::string& section, const std::string& key, varconf::Variable& variable);

	/**
	 *    Listend for changes to the config.
	 * @param section
	 * @param key
	 * @param variable
	 */
	void Config_SunDiffuseMultiplier(const std::string& section, const std::string& key, varconf::Variable& variable);

	/**
	 *    Listend for changes to the config.
	 * @param section
	 * @param key
	 * @param variable
	 */
	void Config_SunSpecularMultiplier(const std::string& section, const std::string& key, varconf::Variable& variable);


	static bool parse(varconf::Variable& variable, Ogre::ColourValue& colour);
};

}





#endif
