//
// C++ Interface: GUICEGUIAdapter
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
#ifndef EMBEROGREGUICEGUIADAPTER_H
#define EMBEROGREGUICEGUIADAPTER_H

#include "EmberOgrePrerequisites.h"
#include <CEGUI/System.h>
#include <CEGUI/EventArgs.h>
#include <CEGUI/InputEvent.h>

#include "services/input/IInputAdapter.h"
#include "services/input/Input.h"

#include <map>

namespace CEGUI {
class System;

class OgreRenderer;

}

namespace Ember {
namespace OgreView {

typedef std::map<SDL_Scancode, CEGUI::Key::Scan> SDLKeyMap;

/**
 * @author Erik Ogenvik
 *
 * @brief Provides an adapter between the input system and CEGUI.
 */
class GUICEGUIAdapter : public IInputAdapter {
public:

	/**
	 * Creates a new instance.
	 * @param system A valid CEGUI::System
	 * @param renderer A valid CEGUI::OgreCEGUIRenderer
	 * @return
	 */
	GUICEGUIAdapter(CEGUI::System& system, CEGUI::OgreRenderer& renderer);

	~GUICEGUIAdapter() override;

	bool injectMouseMove(const MouseMotion& motion, bool& freezeMouse) override;

	bool injectMouseButtonUp(Input::MouseButton button) override;

	bool injectMouseButtonDown(Input::MouseButton button) override;

	bool injectChar(std::int32_t character) override;

	bool injectKeyDown(const SDL_Scancode& key) override;

	bool injectKeyUp(const SDL_Scancode& key) override;

private:
	CEGUI::System& mGuiSystem;
	CEGUI::OgreRenderer& mGuiRenderer;
	CEGUI::GUIContext& mGuiContext;

	/**
	mapping of SDL-keys to CEGUI keys
	*/
	SDLKeyMap mKeyMap;

};

}

}

#endif
