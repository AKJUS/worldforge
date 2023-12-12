/*
 Copyright (C) 2020 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <framework/Tokeniser.h>
#include "services/input/InputCommandMapper.h"
#include "ConsoleInputBinder.h"

namespace Ember {

ConsoleInputBinder::ConsoleInputBinder(Ember::Input& input, ConsoleBackend& consoleBackend) :
		mInput(input),
		mBind(consoleBackend, "bind", [this](auto&& command, auto&& args) { bindCommand(std::forward<decltype(command)>(command), std::forward<decltype(args)>(args)); }),
		mUnbind(consoleBackend, "unbind", [this](auto&& command, auto&& args) { unbindCommand(std::forward<decltype(command)>(command), std::forward<decltype(args)>(args)); }),
		mToggleFullscreen(consoleBackend, "toggle_fullscreen",
						  [this](auto&& command, auto&& args) { toggleFullscreenCommand(std::forward<decltype(command)>(command), std::forward<decltype(args)>(args)); },
						  "Switch between windowed and full screen mode.") {

}

void ConsoleInputBinder::bindCommand(const std::string&, const std::string& args) {
	Tokeniser tokeniser;
	tokeniser.initTokens(args);
	std::string state("general");
	std::string key = tokeniser.nextToken();
	if (!key.empty()) {
		std::string commandToken = tokeniser.nextToken();
		if (!commandToken.empty()) {
			if (!tokeniser.nextToken().empty()) {
				state = tokeniser.nextToken();
			}
			auto mapper = mInput.getMapperForState(state);
			if (mapper) {
				mapper->bindCommand(key, commandToken);
			}
		}
	}
}

void ConsoleInputBinder::unbindCommand(const std::string&, const std::string& args) {
	Tokeniser tokeniser;
	tokeniser.initTokens(args);
	std::string state("general");
	std::string key(tokeniser.nextToken());
	if (!key.empty()) {
		if (!tokeniser.nextToken().empty()) {
			state = tokeniser.nextToken();
		}
		auto mapper = mInput.getMapperForState(state);
		if (mapper) {
			mapper->unbindCommand(key);
		}
	}
}

void ConsoleInputBinder::toggleFullscreenCommand(const std::string&, const std::string&) {
	mInput.toggleFullscreen();
}

}