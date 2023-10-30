/*
 Copyright (C) 2009 Erik Ogenvik

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

#include "FreeFlyingCameraMotionHandler.h"
#include "components/ogre/Convert.h"
#include "framework/ConsoleBackend.h"
#include "framework/Tokeniser.h"
#include <OgreSceneNode.h>

namespace Ember {
namespace OgreView {

FreeFlyingCameraMotionHandler::FreeFlyingCameraMotionHandler(Ogre::SceneNode& freeFlyingNode)
		: mFreeFlyingNode(freeFlyingNode),
		  mSpeed(50) {
	ConsoleBackend::getSingleton().registerCommand("move_camera", [&](const std::string& command, const std::string& args) {
		auto tokens = Tokeniser::split(args, " ");
		if (tokens.size() == 3) {
			mFreeFlyingNode.setPosition(std::stof(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]));
		}
	});
}

FreeFlyingCameraMotionHandler::~FreeFlyingCameraMotionHandler() {
	ConsoleBackend::getSingleton().deregisterCommand("move_camera");
}

void FreeFlyingCameraMotionHandler::move(const WFMath::Quaternion& orientation, const WFMath::Vector<3>& movement, float timeslice) {
	mFreeFlyingNode.translate(Convert::toOgre((movement * timeslice * mSpeed).rotate(orientation)));
}

float FreeFlyingCameraMotionHandler::getSpeed() const {
	return mSpeed;
}

void FreeFlyingCameraMotionHandler::setSpeed(float speed) {
	mSpeed = speed;
}

}
}
