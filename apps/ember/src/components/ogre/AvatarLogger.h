//
// C++ Interface: AvatarLogger
//
// Description:
//
//
// Author: Sean Ryan <sryan@evercrack.com>, (C) 2008
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
#ifndef EMBEROGREAVATARLOGGER_H
#define EMBEROGREAVATARLOGGER_H

#include <sigc++/trackable.h>
#include <fstream>
#include <string>
#include <memory>


namespace Ember {
class EmberEntity;

struct EntityTalk;
namespace OgreView {

class Avatar;

/**
	@brief Handles logging of ingame events such as chat to a file.

	The file will be created when an instance of this is created and the connection will be severed at its destruction.
	The log file will be located in the directory returned by a call to ConfigService::getHomeDirectory(BaseDirType_DATA)

	@author Sean Ryan <sryan@evercrack.com>
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class AvatarLogger : public sigc::trackable {
public:
	explicit AvatarLogger(EmberEntity& avatarEntity);

	virtual ~AvatarLogger();

protected:

	/**
	 * Holds the file to which messages will be written.
	 */
	std::unique_ptr<std::ofstream> mChatLogger;


	/**
	 * Listens to the GUIManager::AppendIGChatLine event and write the contents of chat messages to the log.
	 * @param message
	 * @param entity
	 */
	void GUIManager_AppendIGChatLine(const EntityTalk& entityTalk, EmberEntity* entity);


};

}

}

#endif
