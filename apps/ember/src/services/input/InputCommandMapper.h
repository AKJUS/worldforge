//
// C++ Interface: InputCommandMapper
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2006
// Copyright (C) 2001 - 2005 Simon Goodall, University of Southampton
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
#ifndef EMBEROGREINPUTCOMMANDMAPPER_H
#define EMBEROGREINPUTCOMMANDMAPPER_H

#include "Input.h"
#include <SDL3/SDL_scancode.h>
#include <utility>
#include <vector>
#include <map>

namespace Ember {


/**

	@brief Listens for input (key presses, mouse movements etc) and maps this input to commands that are then executed (such as +move_forward etc.)

	It's expected that more than one instance of this should exists, one for each different game "state", such as "movement mode", "general" etc.

	@author Erik Ogenvik <erik@ogenvik.org>
*/
class InputCommandMapper : public virtual sigc::trackable {
public:
	typedef std::multimap<std::string, std::string> KeyCommandStore;
	typedef std::map<SDL_Scancode, std::string> KeyMapStore;
	typedef std::vector<std::string> StringStore;

	/**
	 * @brief Ctor.
	 * @param state The state which this instance should apply to, such as "general" or "gui"
	 */
	explicit InputCommandMapper(std::string state);

	/**
	 * @brief Constructor which also reads values from the config and sets up bindings.
	 * @param state The state which this instance should apply to, such as "general" or "gui"
	 * @param configSection The name of the config section to read values from.
	 */
	InputCommandMapper(std::string state, const std::string& configSection);

	/**
	 * @brief At destruction the instance is also deregistered from the Input instance.
	 */
	virtual ~InputCommandMapper();

	/**
	 * @brief Binds a command to a key.
	 * @param key The name of the key.
	 * @param command The full command to execute, such as "/show_inventory" or "+run"
	 */
	void bindCommand(const std::string& key, const std::string& command);

	/**
	 * @brief Removes all binding for a certain key.
	 * @param key The name of the key to remove bindings for.
	 */
	void unbindCommand(const std::string& key);

	/**
	 * @brief Removes a binding for a certain key.
	 * @param key The name of the key to remove bindings for.
	 * @param command The name of the command to remove.
	 */
	void unbindCommand(const std::string& key, const std::string& command);

	/**
	 * @brief The name of the state that this instance applies to.
	 * @return The name of the state.
	 */
	const std::string& getState() const;

	/**
	 * @brief Reads a section from the config and sets up bindings accordingly.
	 * @param sectionName The name of the section, such as "key_bindings_general".
	 */
	void readFromConfigSection(const std::string& sectionName);

	/**
	 * @brief Sets whether this instance is enabled or not.
	 * @param enabled If this mapper should be enabled or not.
	 */
	void setEnabled(bool enabled);

	/**
	 * @brief Returns whether this instance is enabled or not.
	 * @return True if enabled.
	 */
	bool getEnabled() const;

	/**
	 * @brief Binds the instance to a Input object. Call this to start recieving input.
	 * @param input
	 */
	void bindToInput(Input& input);

	/**
	 * @brief Sets which modes this instance should be restricted to. An empty list will make it listen for all modes.
	 * @param modes A collection of modes.
	 */
	void restrictToInputModes(std::vector<Input::InputMode> modes);

	/**
	 * @brief Sets which mode this instance should be restricted to.
	 * @param mode An input mode.
	 */
	void restrictToInputMode(Input::InputMode mode);

protected:

	/**
	 * @brief At keypress time, see what command to run.
	 * @param key
	 * @param inputMode
	 */
	void Input_EventKeyPressed(const SDL_KeyboardEvent& key, Input::InputMode inputMode);

	/**
	 * @brief At keyrelease time, see if there's a command prefixed with "+", such as "+run", which should have its "-" twin command sent out.
	 * @param key
	 * @param inputMode
	 */
	void Input_EventKeyReleased(const SDL_KeyboardEvent& key, Input::InputMode inputMode);

	/**
	 * @brief A store of the mapping between keys and commands.
	 */
	KeyCommandStore mKeyCommands;

	/**
	 * @brief The state which this instance applies to.
	 */
	std::string mState;

	/**
	 * @brief Gets the name of the commands registered to the key, or an empty string if no could be found.
	 * @param key
	 * @return
	 */
//	const std::string& getCommandForKey(SDLKey key);

	/**
	Mappings between SDLKeys and their names.
	*/
	KeyMapStore mKeymap;

	/**
	 *    Creates the mapping between SDLKeys and their names.
	 */
	void initKeyMap();

	/**
	Whether this is enabled or not.
	*/
	bool mEnabled;

	/**
	Stores the InputModes that this instance is restricted to listen to, if any.
	*/
	std::vector<Input::InputMode> mInputModesRestriction;

	/**
	 *    Returns true if this instance is active for the current input mode.
	 * @param mode
	 * @return
	 */
	bool isActiveForInputMode(Input::InputMode mode) const;

	/**
	We'll keep a reference to the Input object which we will use at dispose time.
	*/
	Input* mInput;
};

inline const std::string& InputCommandMapper::getState() const {
	return mState;
}

inline void InputCommandMapper::setEnabled(bool enabled) {
	mEnabled = enabled;
}

inline bool InputCommandMapper::getEnabled() const {
	return mEnabled;
}

inline void InputCommandMapper::restrictToInputModes(std::vector<Input::InputMode> modes) {
	mInputModesRestriction = std::move(modes);
}

inline void InputCommandMapper::restrictToInputMode(Input::InputMode mode) {
	std::vector<Input::InputMode> modes;
	modes.push_back(mode);
	restrictToInputModes(modes);
}


}

#endif
