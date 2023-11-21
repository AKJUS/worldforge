/*
 Copyright (C) 2002  Martin Pollard
	
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

#include "OOGChat.h"
#include "framework/Log.h"
#include "framework/ConsoleBackend.h"

#include <Eris/Person.h>
#include <sstream>

namespace Ember {

// List of OOGChat's console commands
const char* const OOGChat::CMD_TALK = "talk";
const char* const OOGChat::CMD_EMOTE = "emote";
const char* const OOGChat::CMD_ME = "me";
const char* const OOGChat::CMD_JOIN = "join";
const char* const OOGChat::CMD_PART = "part";
const char* const OOGChat::CMD_MSG = "msg";

OOGChat::OOGChat(Eris::Account* account) : myLobby(std::make_unique<Eris::Lobby>(*account)) {
	// Set up the lobby object

	// Specfic to lobby Callbacks setup
	myLobby->SightPerson.connect(sigc::mem_fun(*this, &OOGChat::sightPerson));
	myLobby->PrivateTalk.connect(sigc::mem_fun(*this, &OOGChat::privateTalk));
	//myLobby->LoggedIn.connect(SigC::slot(*this,&OOGChat::loggedIn));

	// Ordinary rooms callbacks
	myLobby->Entered.connect(sigc::mem_fun(*this, &OOGChat::entered));
	myLobby->Speech.connect(sigc::mem_fun(*this, &OOGChat::talk));
	myLobby->Emote.connect(sigc::mem_fun(*this, &OOGChat::emote));
	myLobby->Appearance.connect(sigc::mem_fun(*this, &OOGChat::appearance));
	myLobby->Disappearance.connect(sigc::mem_fun(*this, &OOGChat::disappearance));
//      myLobby->Changed.connect(SigC::bind(SigC::slot(*this,&OOGChat::changed),myLobby));

/*      ConsoleBackend::getSingletonPtr()->registerCommand( CMD_TALK, this );
      ConsoleBackend::getSingletonPtr()->registerCommand( CMD_EMOTE, this );
      ConsoleBackend::getSingletonPtr()->registerCommand( CMD_ME, this );
      ConsoleBackend::getSingletonPtr()->registerCommand( CMD_JOIN, this );
      ConsoleBackend::getSingletonPtr()->registerCommand( CMD_PART, this );
      ConsoleBackend::getSingletonPtr()->registerCommand( CMD_MSG, this );*/
}

OOGChat::~OOGChat() = default;


// Lobby Specific callbacks

void OOGChat::sightPerson(Eris::Person* person) {
	logger->info("Sighted Person name:\"{}\" id:{}", person->getName(), person->getAccount());
}

void OOGChat::privateTalk(Eris::Person* person, const std::string& msg) {
	std::ostringstream temp;

	temp << "PRIVMSG(" << person->getName() << ") says:" << msg;

	logger->info(temp.str());

#if 0 // not new sstream
	temp<<std::ends;
#endif
	ConsoleBackend::getSingletonPtr()->pushMessage(temp.str(), "info");
}

void OOGChat::loggedIn(const Atlas::Objects::Entity::Player& player) {
	// Xmp's Notes
	// Erm dunno what this function is for eris's doxygen doesn't explain
	ConsoleBackend::getSingletonPtr()->pushMessage("Logged In eris msg received", "important");
}

void OOGChat::runCommand(const std::string& command, const std::string& args) {
	// %FIXME xmp,4: Don't allow talk until we have logged in
	// %FIXME xmp,4: Stop just using myLobby to allow chat to multiple rooms

	if (command == CMD_TALK) {
		myLobby->say(args);
		return;
	} else if (command == CMD_EMOTE || command == CMD_ME) {
		myLobby->emote(args);
		return;
	} else if (command == CMD_JOIN) {
		return;
	} else if (command == CMD_PART) {
		return;
	} else if (command == CMD_MSG) {
		return;
	}
}

// All Eris::Room callbacks

void OOGChat::entered(Eris::Room* room) {
	std::ostringstream temp;

	temp << "Entry of " << room->getName() << " complete";
	logger->debug(temp.str());
#if 0 //not new stream
	temp<<std::ends;
#endif
	ConsoleBackend::getSingletonPtr()->pushMessage(temp.str(), "info");
}

void OOGChat::talk(Eris::Room* room, Eris::Person* person, const std::string& msg) {
	std::ostringstream temp;

	temp << "[" << room->getName() << "] " << person->getName() << " says: " << msg;
	logger->debug(temp.str());
	temp << std::ends;
	ConsoleBackend::getSingletonPtr()->pushMessage(temp.str(), "info");
}

void OOGChat::emote(Eris::Room* room, Eris::Person* person, const std::string& msg) {
	std::ostringstream temp;

	temp << "[" << room->getName() << "] " << person->getName() << " " << msg;
	logger->debug(temp.str());
#if 0 // not new sstream
	temp<<std::ends;
#endif
	ConsoleBackend::getSingletonPtr()->pushMessage(temp.str(), "info");
}

void OOGChat::appearance(Eris::Room* room, Eris::Person* person) {
	std::ostringstream temp;

	temp << person->getName() << " appears in " << room->getName();
	logger->debug(temp.str());
#if 0 // not new sstream
	temp<<std::ends;
#endif
	ConsoleBackend::getSingletonPtr()->pushMessage(temp.str(), "info");
}

void OOGChat::disappearance(Eris::Room* room, Eris::Person* person) {
	std::ostringstream temp;

	temp << person->getName() << " disappears from " << room->getName();
	logger->debug(temp.str());
#if 0 // if not new sstream
	temp<<std::ends;
#endif
	ConsoleBackend::getSingletonPtr()->pushMessage(temp.str(), "info");
}

void OOGChat::changed(const std::set<std::string>& sset, Eris::Room* room) {
}
}
