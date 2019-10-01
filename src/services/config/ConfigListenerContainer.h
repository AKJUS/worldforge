//
// C++ Interface: ConfigListenerContainer
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

#ifndef CONFIGLISTENERCONTAINER_H_
#define CONFIGLISTENERCONTAINER_H_

#include "ConfigListener.h"
#include <sigc++/slot.h>
#include <vector>
#include <string>
#include <memory>
#include <varconf/variable.h>

namespace Ember
{

/**
 @author Erik Ogenvik <erik@ogenvik.org>
 All classes that wishes to use the ConfigListener class to listen for configuration changes must inherit from this class.
 Call registerConfigListener to register new listeners.

 All listeners will be automatically destroyed when this class is destroyed.
 */
class ConfigListenerContainer
{
public:

	virtual ~ConfigListenerContainer();

	/**
	 * @brief Registers a new listener. The listener instance will be owned by this
	 * class and automatically deleted when the destructor is called.
	 * @param section The config section to listen to.
	 * @param key The config key to listen to.
	 * @param slot The slot to execute when a change has occurred.
	 * @param evaluateNow If true, the listener will be evaluated instantly,
	 * possibly triggering a call to the signal. Defaults to true.
	 * @return A pointer to the newly created listener instance.
	 */
	ConfigListener* registerConfigListener(const std::string& section, const std::string& key, ConfigListener::SettingChangedSlot slot, bool evaluateNow = true);

	/**
	 * @brief Registers a new listener. The listener instance will be owned by this class and
	 * automatically deleted when the destructor is called. The setting will always be evaluated,
	 * and if no setting can be found the default value will be used to trigger a call to the listener method.
	 * @param section The config section to listen to.
	 * @param key The config key to listen to.
	 * @param slot The slot to execute when a change has occurred.
	 * @param defaultValue The default value, to use if no existing setting can be found.
	 * @return A pointer to the newly created listener instance.
	 */
	ConfigListener* registerConfigListenerWithDefaults(const std::string& section, const std::string& key, ConfigListener::SettingChangedSlot slot, varconf::Variable defaultValue);


private:
	/**
	 A collection of listeners.
	 */
	std::vector<std::unique_ptr<ConfigListener>> mConfigListeners;

};

}

#endif /* CONFIGLISTENERCONTAINER_H_ */
