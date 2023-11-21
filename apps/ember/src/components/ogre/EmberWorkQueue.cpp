/*
 Copyright (C) 2017 Erik Ogenvik

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

#include <thread>
#include <Ogre.h>
#include "EmberWorkQueue.h"
#include "EmberOgrePrerequisites.h"

namespace Ember::OgreView {
EmberWorkQueue::EmberWorkQueue(Eris::EventService& eventService)
		: DefaultWorkQueue("EmberWorkQueue"),
		  mEventService(eventService) {
	//Set the number of Ogre worker threads to half the number of logical cores.
	unsigned int logicalCores = std::thread::hardware_concurrency();
	if (logicalCores == 0) {
		logger->info("Could not get information about logical cores in machine.");
	} else {
		logicalCores = std::max<unsigned int>(1, logicalCores / 2);
		logger->info("Setting Ogre to use {} worker threads.", logicalCores);
		setWorkerThreadCount(logicalCores);
	}
}

Ogre::uint16 EmberWorkQueue::getChannel(const Ogre::String& channelName) {
	auto channelId = Ogre::DefaultWorkQueue::getChannel(channelName);
	//The Ogre Terrain components rely on its handlers always being executed on the main thread directly when processResponses() is called.
	//We'll have to take this into account and keep track of the terrain channels.
	if (Ogre::StringUtil::startsWith(channelName, "Ogre/Terrain", false)) {
		mTerrainChannels.insert(channelId);
	}
	return channelId;
}

void EmberWorkQueue::processResponses() {
	auto msStart = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
	while (true) {
		Ogre::WorkQueue::Response* response;
		{
			OGRE_WQ_LOCK_MUTEX(mResponseMutex);

			if (mResponseQueue.empty()) {
				break;
			}

			response = mResponseQueue.front();
			mResponseQueue.pop_front();
		}

		if (response) {
			//The Ogre Terrain components rely on its handlers always being executed on the main thread directly when processResponses() is called.
			//We'll have to take this into account and process all terrain handlers directly.
			//The main cause is the Ogre::Terrain::waitForDerivedProcesses and similar methods.
			if (mTerrainChannels.find(response->getRequest()->getChannel()) != mTerrainChannels.end()) {
				processResponse(response);
				OGRE_DELETE response;
			} else {
				mEventService.runOnMainThread([this, response] {
					processResponse(response);
					OGRE_DELETE response;
				});
			}
		}

		if (mResposeTimeLimitMS) {
			auto msCurrent = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
			if (msCurrent - msStart > mResposeTimeLimitMS) {
				break;
			}
		}
	}
}
}
