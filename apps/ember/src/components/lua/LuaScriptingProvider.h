//
// C++ Interface: LuaScriptingProvider
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
#ifndef EMBEROGRELUASCRIPTINGPROVIDER_H
#define EMBEROGRELUASCRIPTINGPROVIDER_H

#include "framework/IScriptingProvider.h"
#include "sol2/forward.hpp"

struct lua_State;

namespace Ember {
class ScriptingService;

/**
 * @brief Namespace for bindings to the Lua scripting environment.
 */
namespace Lua {

struct LuaScriptingCallContext;

/**
@brief A scripting provider for Lua.

This acts as a bridge between Ember and the Lua scripting environment. Opon creation and destruction it will take care of setting up and tearing down the lua virtual machine. Remember to call stop() before deleting an instance of this to make sure that everything is properly cleaned up.

If you want to inspect the return values from calls to lua scripts, pass a pointer to LuaScriptingCallContext to the executeScript methods.
@author Erik Ogenvik
*/
class LuaScriptingProvider : public IScriptingProvider {
public:
	LuaScriptingProvider();

	~LuaScriptingProvider() override;

	/**
	 * @brief Loads the script from the wrapper.
	 * @param resourceWrapper A resource wrapper pointing to a valid resource which can be loaded. This should contain a text file with the script contents.
	 */
	void loadScript(ResourceWrapper& resWrapper, IScriptingCallContext* callContext) override;

	/**
	 * @brief Executes the supplied string directly into the scripting environment.
	 * Optionally a pointer to a scripting call context can be submitted too, which will then be populated with return values and other scripting environment specific info.
	 * @param scriptCode The code to excute.
	 * @param callContext An optional pointer to a scripting call context. This will be populated with return values and other info. If you don't have any need for such info, leave this empty.
	 */
	void executeScript(const std::string& scriptCode, IScriptingCallContext* callContext) override;

	/**
	 * @brief Returns true if the provider will load the supplied script name. This is in most cases decided from the filename suffix.
	 * @param scriptName The name of the script.
	 * @return True if the script can be loaded, else false.
	 */
	bool willLoadScript(const std::string& scriptName) override;

	/**
	 * @brief Gets the unique name of the scripting provider.
	 * @return The name of the scripting provider.
	 */
	const std::string& getName() const override;

	/**
	 * @brief Forces a full garbage collection.
	 */
	void forceGC() override;

	/**
	 * @brief Stops the lua environment, which mainly means that all objects are destroyed.
	 * Call this before this object is destroyed to make sure that all held objects and references are properly released. If not, there's a risk of dangling pointers.
	 */
	void stop() override;


	/**
	 * @brief Gets the current lua state.
	 * This will always return a valid lua virtual machine, but note that if @see stop() already has been called it will porbably be in an invalid state.
	 * @return The current lua environment.
	 */
	sol::state& getLuaState();


private:

	/**
	 * @brief Executes the supplied script code.
	 * @param scriptCode The code to execute.
	 * @param luaCallContext An optional lua call context, which if present will contain any return values.
	 * @param scriptName The name of the script, mainly used for debugging purpose.
	 */
	void executeScriptImpl(const std::string& scriptCode, LuaScriptingCallContext* luaCallContext, const std::string& scriptName = std::string(""));

	/**
	The main lua state. This is the sole entry into the lua virtual machine.
	*/
	sol::state mLua;

};

}

}

#endif
