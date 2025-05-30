/*
 Copyright (C) 2012 Erik Ogenvik

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

#include "EntityTextureManipulator.h"

#include "EntityCEGUITexture.h"
#include "../SimpleRenderContext.h"

#include <OgreSceneNode.h>

#include <CEGUI/Window.h>


namespace Ember::OgreView::Gui {

EntityTextureManipulator::EntityTextureManipulator(CEGUI::Window& window, EntityCEGUITexture& texture) :
		mWindow(window), mTexture(texture) {
	mButtonDownConnection = window.subscribeEvent(CEGUI::Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&EntityTextureManipulator::image_MouseButtonDown, this));
	mMoveConnection = window.subscribeEvent(CEGUI::Window::EventMouseWheel, CEGUI::Event::Subscriber(&EntityTextureManipulator::image_MouseWheel, this));

}

EntityTextureManipulator::~EntityTextureManipulator() {
	mButtonDownConnection->disconnect();
	mMoveConnection->disconnect();
	Input::getSingleton().removeAdapter(this);
}

bool EntityTextureManipulator::injectMouseButtonUp(Input::MouseButton button) {
	if (button == Input::MouseButtonLeft) {
		releaseInput();
	}
	return true;
}

bool EntityTextureManipulator::injectMouseButtonDown(Input::MouseButton button) {
	return true;
}

bool EntityTextureManipulator::injectChar(std::int32_t character) {
	return true;
}

bool EntityTextureManipulator::injectKeyDown(const SDL_Scancode&) {
	return true;
}

bool EntityTextureManipulator::injectKeyUp(const SDL_Scancode&) {
	return true;
}

void EntityTextureManipulator::catchInput() {
	Input::getSingleton().addAdapter(this);
	EventMovementStarted();
}

void EntityTextureManipulator::releaseInput() {
	Input::getSingleton().removeAdapter(this);
	EventMovementStopped();
}

bool EntityTextureManipulator::image_MouseWheel(const CEGUI::EventArgs& args) {
	const auto& mouseArgs = static_cast<const CEGUI::MouseEventArgs&>(args);

	if (mouseArgs.wheelChange != 0.0f) {
		float distance = mTexture.getRenderContext()->getCameraDistance();
		distance -= (mouseArgs.wheelChange * 0.1f);
		mTexture.getRenderContext()->setCameraDistance(mTexture.getRenderContext()->getDefaultCameraDistance() * distance);
	}

	return true;
}

bool EntityTextureManipulator::image_MouseButtonDown(const CEGUI::EventArgs& args) {
	const auto& mouseArgs = static_cast<const CEGUI::MouseEventArgs&>(args);
	handleMouseButtonDown(mouseArgs);
	return true;
}

void EntityTextureManipulator::handleMouseButtonDown(const CEGUI::MouseEventArgs& mouseArgs) {
	if (mouseArgs.button == CEGUI::LeftButton) {
		catchInput();
	}
}


DirectEntityTextureManipulator::DirectEntityTextureManipulator(CEGUI::Window& window, EntityCEGUITexture& texture) :
		EntityTextureManipulator(window, texture) {

}

DirectEntityTextureManipulator::~DirectEntityTextureManipulator() = default;

bool DirectEntityTextureManipulator::injectMouseMove(const MouseMotion& motion, bool& freezeMouse) {

	if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RSHIFT) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LSHIFT)) {
		//translate the modelnode
		Ogre::Vector3 translate;
		if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
			translate = Ogre::Vector3(-motion.xRelativeMovement, 0, -motion.yRelativeMovement);
		} else {
			translate = Ogre::Vector3(-motion.xRelativeMovement, motion.yRelativeMovement, 0);
		}
		translate = mTexture.getRenderContext()->getCameraOrientation() * translate;
		mTexture.getRenderContext()->getSceneNode()->translate(translate);
	} else {
		//rotate the modelnode
		Ogre::Quaternion rotate;
		//rotate the modelnode
		if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
			rotate.FromAngleAxis(Ogre::Degree(motion.xRelativeMovement * 180), mTexture.getRenderContext()->getCameraOrientation().zAxis());
		} else {
			rotate.FromAngleAxis(Ogre::Degree(motion.xRelativeMovement * 180), mTexture.getRenderContext()->getCameraOrientation().yAxis());
			Ogre::Quaternion q1;
			q1.FromAngleAxis(Ogre::Degree(motion.yRelativeMovement * 180), mTexture.getRenderContext()->getCameraOrientation().xAxis());
			rotate = rotate * q1;
		}
		mTexture.getRenderContext()->getSceneNode()->rotate(rotate);
	}
	//we don't want to move the cursor
	freezeMouse = true;
	return false;
}

CameraEntityTextureManipulator::CameraEntityTextureManipulator(CEGUI::Window& window, EntityCEGUITexture& texture) :
		EntityTextureManipulator(window, texture) {

}

CameraEntityTextureManipulator::~CameraEntityTextureManipulator() = default;

bool CameraEntityTextureManipulator::injectMouseMove(const MouseMotion& motion, bool& freezeMouse) {

	if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RSHIFT) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LSHIFT)) {
		//translate the modelnode
		Ogre::Vector3 translate;
		if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
			translate = Ogre::Vector3(-motion.xRelativeMovement, 0, -motion.yRelativeMovement);
		} else {
			translate = Ogre::Vector3(-motion.xRelativeMovement, motion.yRelativeMovement, 0);
		}
		translate = mTexture.getRenderContext()->getCameraOrientation() * translate;
		mTexture.getRenderContext()->getCameraRootNode()->translate(translate);
	} else {
		//rotate the modelnode
		if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
			mTexture.getRenderContext()->rollCamera(Ogre::Degree(motion.xRelativeMovement * 180));
		} else {
			mTexture.getRenderContext()->yawCamera(Ogre::Degree(motion.xRelativeMovement * 180));
			mTexture.getRenderContext()->pitchCamera(Ogre::Degree(motion.yRelativeMovement * 180));
		}
	}
	//we don't want to move the cursor
	freezeMouse = true;
	return false;
}

CombinedEntityTextureManipulator::CombinedEntityTextureManipulator(CEGUI::Window& window, EntityCEGUITexture& texture) :
		EntityTextureManipulator(window, texture), mLeftMouseButtonDown(false) {

}

CombinedEntityTextureManipulator::~CombinedEntityTextureManipulator() = default;

bool CombinedEntityTextureManipulator::injectMouseMove(const MouseMotion& motion, bool& freezeMouse) {
	if (mLeftMouseButtonDown) {
		if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RSHIFT) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LSHIFT)) {
			//translate the modelnode
			Ogre::Vector3 translate;
			if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
				translate = Ogre::Vector3(-motion.xRelativeMovement, 0, -motion.yRelativeMovement);
			} else {
				translate = Ogre::Vector3(-motion.xRelativeMovement, motion.yRelativeMovement, 0);
			}
			translate = mTexture.getRenderContext()->getCameraOrientation() * translate;
			mTexture.getRenderContext()->getCameraRootNode()->translate(translate);
		} else {
			//rotate the modelnode
			if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
				mTexture.getRenderContext()->rollCamera(Ogre::Degree(motion.xRelativeMovement * 180));
			} else {
				mTexture.getRenderContext()->yawCamera(Ogre::Degree(motion.xRelativeMovement * 180));
				mTexture.getRenderContext()->pitchCamera(Ogre::Degree(motion.yRelativeMovement * 180));
			}
		}
	} else {
		if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RSHIFT) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LSHIFT)) {
			//translate the modelnode
			Ogre::Vector3 translate;
			if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
				translate = Ogre::Vector3(-motion.xRelativeMovement, 0, -motion.yRelativeMovement);
			} else {
				translate = Ogre::Vector3(-motion.xRelativeMovement, motion.yRelativeMovement, 0);
			}
			translate = mTexture.getRenderContext()->getCameraOrientation() * translate;
			mTexture.getRenderContext()->getSceneNode()->translate(translate);
		} else {
			//rotate the modelnode
			Ogre::Quaternion rotate;
			//rotate the modelnode
			if (Input::getSingleton().isKeyDown(SDL_SCANCODE_RCTRL) || Input::getSingleton().isKeyDown(SDL_SCANCODE_LCTRL)) {
				rotate.FromAngleAxis(Ogre::Degree(motion.xRelativeMovement * 180), mTexture.getRenderContext()->getCameraOrientation().zAxis());
			} else {
				rotate.FromAngleAxis(Ogre::Degree(-motion.xRelativeMovement * 180), mTexture.getRenderContext()->getCameraOrientation().yAxis());
				Ogre::Quaternion q1;
				q1.FromAngleAxis(Ogre::Degree(-motion.yRelativeMovement * 180), mTexture.getRenderContext()->getCameraOrientation().xAxis());
				rotate = rotate * q1;
			}
			mTexture.getRenderContext()->getSceneNode()->rotate(rotate);
		}
	}
	//we don't want to move the cursor
	freezeMouse = true;
	return false;
}

void CombinedEntityTextureManipulator::handleMouseButtonDown(const CEGUI::MouseEventArgs& mouseArgs) {
	if (mouseArgs.button == CEGUI::LeftButton || mouseArgs.button == CEGUI::MiddleButton) {
		if (mouseArgs.button == CEGUI::LeftButton) {
			mLeftMouseButtonDown = true;
		} else {
			mLeftMouseButtonDown = false;
			EventEditingStarted.emit();
		}
		//only catch input if it's allowed
		catchInput();
	}
}

bool CombinedEntityTextureManipulator::injectMouseButtonUp(Input::MouseButton button) {
	if (button == Input::MouseButtonLeft || button == Input::MouseButtonMiddle) {
		releaseInput();
		if (!mLeftMouseButtonDown) {
			EventEditingStopped.emit();
		}
	}
	return true;
}
}


