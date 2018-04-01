//
// C++ Implementation: ConfigListenerContainer
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ConfigListenerContainer.h"
#include <utility>
#include "ConfigListener.h"


namespace Ember
{

ConfigListenerContainer::~ConfigListenerContainer()
{
	for (auto& configListener : mConfigListeners) {
		delete configListener;
	}
}

ConfigListener* ConfigListenerContainer::registerConfigListener(const std::string& section, const std::string& key, SettingChangedSlot slot, bool evaluateNow)
{
	ConfigListener* listener = new ConfigListener(section, key, std::move(slot));
	mConfigListeners.push_back(listener);
	if (evaluateNow)
	{
		listener->evaluate();
	}
	return listener;
}

ConfigListener* ConfigListenerContainer::registerConfigListenerWithDefaults(const std::string& section, const std::string& key, SettingChangedSlot slot, varconf::Variable defaultValue)
{
	ConfigListener* listener = new ConfigListener(section, key, std::move(slot));
	mConfigListeners.push_back(listener);
	if (!listener->evaluate())
	{
		listener->mSlot(section, key, defaultValue);
	}
	return listener;
}
}
