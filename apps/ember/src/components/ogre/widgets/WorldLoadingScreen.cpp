/*
 Copyright (C) 2013 Sean Ryan <sryan@evercrack.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "WorldLoadingScreen.h"
#include "../GUIManager.h"
#include "../EmberOgre.h"

#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>
#include <framework/Service.h>
#include <services/server/ServerService.h>
#include <Eris/Account.h>
#include <SDL3/SDL_events.h>


namespace Ember::OgreView::Gui {

WorldLoadingScreen::WorldLoadingScreen() :
		mLoadingWindow(CEGUI::WindowManager::getSingleton().createWindow("EmberLook/StaticText", "WorldLoadingScreen")) {

	/*
	 * Get Everything setup
	 */

	// Black background with white text
	mLoadingWindow->setProperty("BackgroundColours", "FFFFFF");
	mLoadingWindow->setProperty("TextColours", "FFFFFFFF");
	mLoadingWindow->setProperty("BackgroundEnabled", "true");
	mLoadingWindow->setHorizontalAlignment(CEGUI::HorizontalAlignment::HA_CENTRE);
	mLoadingWindow->setVerticalAlignment(CEGUI::VerticalAlignment::VA_CENTRE);
	mLoadingWindow->setAlwaysOnTop(true);
	mLoadingWindow->setEnabled(true);
	mLoadingWindow->setFont("DejaVuSans-14");
	mLoadingWindow->setProperty("HorzFormatting", "CentreAligned");
	mLoadingWindow->setText("Entering world, please wait...");

	EmberOgre::getSingleton().EventCreatedAvatarEntity.connect(sigc::hide(sigc::mem_fun(*this, &Ember::OgreView::Gui::WorldLoadingScreen::hideScreen)));
	//A failsafe if something went wrong and the avatar never was created.
	EmberOgre::getSingleton().EventWorldDestroyed.connect(sigc::mem_fun(*this, &Ember::OgreView::Gui::WorldLoadingScreen::hideScreen));

	EmberOgre::getSingleton().EventWorldCreated.connect(sigc::hide(sigc::mem_fun(*this, &Ember::OgreView::Gui::WorldLoadingScreen::showScreen)));

}

WorldLoadingScreen::~WorldLoadingScreen() = default;

CEGUI::Window& WorldLoadingScreen::getWindow() {
	return *mLoadingWindow;
}

void WorldLoadingScreen::showScreen() {
	//Allow ESC to remove the screen.
	Input::getSingleton().EventKeyReleased.connect([&](const SDL_KeyboardEvent& keysym, Input::InputMode) {
		if (keysym.key == SDLK_ESCAPE) {
			hideScreen();
		}
	});

	if (auto account = ServerService::getSingleton().getAccount()) {
		account->AvatarFailure.connect(sigc::hide(sigc::mem_fun(*this, &WorldLoadingScreen::hideScreen)));
	}
	if (!mLoadingWindow->getParent()) {
		/*
		 * Add to the main sheet.  This is "turning on" the load screen
		 */
		GUIManager::getSingleton().getMainSheet()->addChild(mLoadingWindow.get());
	}
}

void WorldLoadingScreen::hideScreen() {
	if (mLoadingWindow->getParent()) {
		/*
		 * Remove from the main sheet.  This is "turning off" the load screen
		 */
		GUIManager::getSingleton().getMainSheet()->removeChild(mLoadingWindow.get());
	}
}

} // end namespace Gui
// end namespace OgreView
// end namespace Ember

