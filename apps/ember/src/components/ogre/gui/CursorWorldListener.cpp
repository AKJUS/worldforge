#include <memory>

/*
 Copyright (C) 2011 Erik Ogenvik

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

#include "CursorWorldListener.h"

#include "framework/Log.h"
#include "framework/MainLoopController.h"
#include "framework/TimeFrame.h"
#include "components/ogre/camera/MainCamera.h"
#include "components/ogre/World.h"
#include "components/ogre/Avatar.h"
#include "components/ogre/EntityWorldPickListener.h"
#include "components/ogre/model/ModelRepresentation.h"
#include "domain/EmberEntity.h"

#include <CEGUI/Window.h>


namespace Ember::OgreView::Gui {


CursorWorldListener::CursorWorldListener(MainLoopController& mainLoopController, CEGUI::Window& mainWindow, World& world) :
		mMainWindow(mainWindow),
		mWorld(world),
		mHoverEventSent(false),
		mCursorLingerStart(0),
		mClickThresholdMilliseconds(50),
		mMousePressedTimeFrame(nullptr),
		mConfigListenerContainer(std::make_unique<ConfigListenerContainer>()),
		mIsCursorInWorld(false) {

	mainLoopController.EventAfterInputProcessing.connect(sigc::mem_fun(*this, &CursorWorldListener::afterEventProcessing));
	Ember::Input::getSingleton().EventMouseButtonReleased.connect(sigc::mem_fun(*this, &CursorWorldListener::input_MouseButtonReleased));

	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventMouseEntersSurface, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseEnters, this)));
	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventMouseLeavesSurface, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseLeaves, this)));
	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventDragDropItemEnters, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseEnters, this)));
	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventDragDropItemLeaves, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseLeaves, this)));

	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseButtonDown, this)));
	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventMouseButtonUp, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseButtonUp, this)));
	mConnections.push_back(mMainWindow.subscribeEvent(CEGUI::Window::EventMouseDoubleClick, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseDoubleClick, this)));

	mConfigListenerContainer->registerConfigListenerWithDefaults("input", "clickthreshold", sigc::mem_fun(*this, &CursorWorldListener::Config_ClickThreshold), 200);


}

CursorWorldListener::~CursorWorldListener() {

	mConfigListenerContainer.reset();
	for (auto& connection: mConnections) {
		connection->disconnect();
	}
	if (mMouseMovesConnection.isValid()) {
		mMouseMovesConnection->disconnect();
	}
}

void CursorWorldListener::afterEventProcessing(float timeslice) {
	if (isInGUIMode()) {
		if (mIsCursorInWorld) {

			const auto& pixelPosition = mMainWindow.getGUIContext().getMouseCursor().getPosition();
			sendWorldClick(MPT_SELECT, pixelPosition, 50);

			if (!mHoverEventSent) {
				mCursorLingerStart += static_cast<long long>(timeslice * 1000);

				if (mCursorLingerStart > 500) {
					sendHoverEvent();
				}
			}

			if (mMousePressedTimeFrame) {
				if (!mMousePressedTimeFrame->isTimeLeft()) {
					mMousePressedTimeFrame.reset();
					sendWorldClick(MPT_PRESSED, pixelPosition, 50);
				}
			}
		}
	} else {

		if (mWorld.getAvatar()) {
			mWorld.getEntityPickListener().mFilter = [&](const EmberEntity& pickedEntity) {
				return &pickedEntity != &mWorld.getAvatar()->getEmberEntity();
			};
		}

		MousePickerArgs pickerArgs{};
		pickerArgs.pickType = MousePickType::MPT_SELECT;
		pickerArgs.distance = 50;
		mWorld.getMainCamera().pickInWorld(0.5, 0.5, pickerArgs);
		mWorld.getEntityPickListener().mFilter = nullptr;

	}
}

bool CursorWorldListener::windowMouseEnters(const CEGUI::EventArgs& args) {
	if (!mIsCursorInWorld) {
		mMouseMovesConnection = mMainWindow.subscribeEvent(CEGUI::Window::EventMouseMove, CEGUI::Event::Subscriber(&CursorWorldListener::windowMouseMoves, this));
		mHoverEventSent = false;
		mIsCursorInWorld = true;
	}
	return true;
}

bool CursorWorldListener::windowMouseLeaves(const CEGUI::EventArgs& args) {
	if (mMouseMovesConnection.isValid()) {
		mMouseMovesConnection->disconnect();
		mMouseMovesConnection = CEGUI::Event::Connection();
	}
	mHoverEventSent = true;
	mIsCursorInWorld = false;
	return true;
}

bool CursorWorldListener::windowMouseMoves(const CEGUI::EventArgs& args) {
	mCursorLingerStart = 0;
	mHoverEventSent = false;
	return true;
}

void CursorWorldListener::sendHoverEvent() {
	const auto& pixelPosition = mMainWindow.getGUIContext().getMouseCursor().getPosition();
	sendWorldClick(MPT_HOVER, pixelPosition, 30);
	mHoverEventSent = true;
}

void CursorWorldListener::input_MouseButtonReleased(Input::MouseButton button, Input::InputMode inputMode) {
	mMousePressedTimeFrame.reset();
}

void CursorWorldListener::sendWorldClick(MousePickType pickType, const CEGUI::Vector2f& pixelPosition, float distance) {

	const auto& position = mMainWindow.getGUIContext().getMouseCursor().getDisplayIndependantPosition();
	MousePickerArgs pickerArgs{};
	pickerArgs.windowX = pixelPosition.d_x;
	pickerArgs.windowY = pixelPosition.d_y;
	pickerArgs.pickType = pickType;
	pickerArgs.distance = distance;
	mWorld.getMainCamera().pickInWorld(position.d_x, position.d_y, pickerArgs);

}

bool CursorWorldListener::windowMouseButtonDown(const CEGUI::EventArgs& args) {
	if (isInGUIMode()) {
		logger->debug("Main sheet is capturing input");
		CEGUI::Window* aWindow = mMainWindow.getCaptureWindow();
		if (aWindow) {
			aWindow->releaseInput();
			aWindow->deactivate();
		}

		mMousePressedTimeFrame = std::make_unique<TimeFrame>(std::chrono::milliseconds(mClickThresholdMilliseconds));
		sendWorldClick(MPT_PRESS, mMainWindow.getGUIContext().getMouseCursor().getPosition(), 300);
	}

	return true;
}

bool CursorWorldListener::windowMouseButtonUp(const CEGUI::EventArgs& args) {
	if (isInGUIMode()) {
		if (mMousePressedTimeFrame) {
			if (mMousePressedTimeFrame->isTimeLeft()) {
				mMousePressedTimeFrame.reset();
				sendWorldClick(MPT_CLICK, static_cast<const CEGUI::MouseEventArgs&>(args).position, 300);
			}
		}
	}
	return true;
}

bool CursorWorldListener::windowMouseDoubleClick(const CEGUI::EventArgs& args) {
	auto& mouseArgs = static_cast<const CEGUI::MouseEventArgs&>(args);
	sendWorldClick(MPT_DOUBLECLICK, mouseArgs.position, 300);

	return true;
}

bool CursorWorldListener::isInGUIMode() const {
	return Input::getSingleton().getInputMode() == Input::IM_GUI;
}

void CursorWorldListener::Config_ClickThreshold(const std::string& section, const std::string& key, varconf::Variable& variable) {
	if (variable.is_int()) {
		mClickThresholdMilliseconds = static_cast<int>(variable);
	}
}

}


