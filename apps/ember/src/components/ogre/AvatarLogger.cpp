//
// C++ Implementation: AvatarLogger
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
#include "AvatarLogger.h"
#include "Avatar.h"
#include "domain/EmberEntity.h"
#include "GUIManager.h"
#include "services/config/ConfigService.h"
#include "framework/TimeHelper.h"
#include "domain/EntityTalk.h"
#include <memory>
#include <filesystem>

#ifdef _WIN32
#include "platform/platform_windows.h"
#endif


namespace Ember::OgreView {

AvatarLogger::AvatarLogger(EmberEntity& avatarEntity)
		: mChatLogger(nullptr) {
	//Put log files in a "logs" subdirectory of the home directory.
	auto dir = ConfigService::getSingleton().getHomeDirectory(BaseDirType_DATA) / "logs";
	try {
		//make sure the directory exists


		if (!std::filesystem::exists(dir)) {
			std::filesystem::create_directories(dir);
		}
		//perform setup of the stream
		std::stringstream logFileSS;
		logFileSS << dir << "/" << avatarEntity.getName() << "_chatlog.log";
		mChatLogger = std::make_unique<std::ofstream>(logFileSS.str().c_str(), std::ios::app);
		logger->debug("Chat Logging set to write in [ {} ]", logFileSS.str());

		*mChatLogger << "-------------------------------------------------------" << std::endl;
		*mChatLogger << "Chat Logging Initialized at " << TimeHelper::getLocalTimeStr() << std::endl;
		*mChatLogger << "-------------------------------------------------------" << std::endl;

		//wait with connecting until everything has been properly set up
		GUIManager::getSingleton().AppendIGChatLine.connect(sigc::mem_fun(*this, &AvatarLogger::GUIManager_AppendIGChatLine));

	} catch (const std::exception& ex) {
		logger->error("Error when creating directory for logs: {}", ex.what());
	}
}


AvatarLogger::~AvatarLogger() {
	*mChatLogger << "-------------------------------------------------------" << std::endl;
	*mChatLogger << "Chat Logging Ended at " << TimeHelper::getLocalTimeStr() << std::endl;
	*mChatLogger << "-------------------------------------------------------" << std::endl;
}

void AvatarLogger::GUIManager_AppendIGChatLine(const EntityTalk& entityTalk, EmberEntity* entity) {
	if (!entityTalk.message.empty()) {
		*mChatLogger << "[" << TimeHelper::getLocalTimeStr() << "] <" << entity->getName() << "> says: " << entityTalk.message << std::endl;
	}
}

}

