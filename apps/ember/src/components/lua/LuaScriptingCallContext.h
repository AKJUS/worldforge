//
// C++ Interface: LuaScriptingCallContext
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
#ifndef EMBEROGRELUASCRIPTINGCALLCONTEXT_H
#define EMBEROGRELUASCRIPTINGCALLCONTEXT_H

#include "framework/IScriptingProvider.h"
#include "sol2/sol.hpp"


namespace Ember::Lua {

/**
 * @brief A scripting call context for lua scripts.
 *
 * Whenever you want to be able to inspect return values from calling lua scripts, you should use an instance of this class and submit it when calling executeCode(...).
 * @author Erik Ogenvik <erik@ogenvik.org>
 */
struct LuaScriptingCallContext : public IScriptingCallContext {
	sol::object ReturnValue;
};


}


#endif
