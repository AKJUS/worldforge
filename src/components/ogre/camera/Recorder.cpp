/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

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

#include "Recorder.h"
#include "components/ogre/EmberOgre.h"
#include "services/EmberServices.h"
#include "services/config/ConfigService.h"
#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <boost/filesystem/operations.hpp>

namespace Ember {
namespace OgreView {
namespace Camera {

Recorder::Recorder() :
		mSequence(0),
		mAccruedTime(0.0f),
		mFramesPerSecond(20.0f) {
}

void Recorder::startRecording() {
	Ogre::Root::getSingleton().addFrameListener(this);
}

void Recorder::stopRecording() {
	Ogre::Root::getSingleton().removeFrameListener(this);
}

bool Recorder::frameStarted(const Ogre::FrameEvent& event) {
	mAccruedTime += event.timeSinceLastFrame;
	if (mAccruedTime >= (1.0f / mFramesPerSecond)) {
		mAccruedTime = 0.0f;
		std::stringstream filename;
		filename << "screenshot_" << mSequence++ << ".tga";
		auto dir = ConfigService::getSingleton().getHomeDirectory(BaseDirType_DATA) / "recordings";
		try {
			//make sure the directory exists

			if (!boost::filesystem::exists(dir)) {
				boost::filesystem::create_directories(dir);
			}
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Error when creating directory for screenshots." << ex);
			stopRecording();
			return true;
		}
		try {
			// take screenshot
			EmberOgre::getSingleton().getRenderWindow()->writeContentsToFile((dir / filename.str()).string());
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Could not write screenshot to disc." << ex);
			stopRecording();
			return true;
		}
	}
	return true;
}

}
}
}

