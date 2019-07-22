/*
 Copyright (C) 2010 Erik Ogenvik <erik@ogenvik.org>

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "EnteredWorldState.h"
#include "ServerServiceSignals.h"

#include "framework/Tokeniser.h"
#include "framework/ConsoleBackend.h"
#include "framework/LoggingInstance.h"

#include <Eris/Entity.h>
#include <Eris/Avatar.h>
#include <Eris/Account.h>
#include <Eris/Connection.h>
#include <Eris/View.h>
#include <Eris/TypeInfo.h>

namespace Ember
{

EnteredWorldState::EnteredWorldState(IState& parentState, Eris::Avatar& avatar, Eris::Account& account) :
		StateBase<void>::StateBase(parentState), Say("say", this, "Say something."),
		SayTo("sayto", this, "Say something address to one or many entities. Format: '/sayto entityid,entityid,... message"),
		Emote("me", this, "Emotes something."),
		Delete("delete", this, "Deletes an entity."),
		AdminTell("admin_tell", this, "Uses admin mode to directly tell a NPC something. Usage: /admin_tell <entityid> <key> <value>"),
		mAvatar(avatar),
		mAccount(account),
		mAdapter(account, avatar)
{
	getSignals().GotAvatar.emit(&mAvatar);
	getSignals().GotView.emit(&getView());

}

EnteredWorldState::~EnteredWorldState()
{
	getSignals().DestroyedView.emit();
	getSignals().DestroyedAvatar.emit();
}

bool EnteredWorldState::logout()
{
	mAvatar.deactivate();
	return true;
}

void EnteredWorldState::runCommand(const std::string &command, const std::string &args)
{
	if (Say == command) {
		mAdapter.say(args);
	} else if (SayTo == command) {
		Tokeniser tokeniser(args);
		std::string entityIdsString = tokeniser.nextToken();
		std::vector<std::string> entityIds = Tokeniser::split(entityIdsString, ",");
		std::string message = tokeniser.remainingTokens();

		mAdapter.sayTo(message, entityIds);
	} else if (Emote == command) {
		mAdapter.emote(args);
	} else if (Delete == command) {
		Tokeniser tokeniser(args);
		std::string entityId = tokeniser.nextToken();
		if (!entityId.empty()) {
			Eris::Entity* entity = getView().getEntity(entityId);
			if (entity) {
				mAdapter.deleteEntity(entity);
			}
		}

		/*		// Touch Command
		 } else if (command==TOUCH) {
		 // TODO: make this switch call the touch method
		 // TODO: polish this rough check
		 S_LOG_VERBOSE("Touching");
		 if(!mAvatar) {
		 S_LOG_WARNING("No avatar.");
		 return;
		 }

		 Atlas::Objects::Operation::Touch touch;
		 Atlas::Message::MapType opargs;

		 opargs["id"] = args;
		 touch->setFrom(mAvatar->getId());
		 touch->setArgsAsList(Atlas::Message::ListType(1, opargs));

		 mConn->send(touch);*/
	} else if (AdminTell == command) {
		Tokeniser tokeniser(args);
		std::string entityId = tokeniser.nextToken();
		if (!entityId.empty()) {
			std::string key = tokeniser.nextToken();
			if (!key.empty()) {
				std::string value = tokeniser.nextToken();
				if (!value.empty()) {
					mAdapter.adminTell(entityId, key, value);
				}
			}
		}
	}
}

IServerAdapter& EnteredWorldState::getServerAdapter()
{
	return mAdapter;
}

Eris::Connection& EnteredWorldState::getConnection() const
{
	return *mAvatar.getConnection();
}

Eris::View& EnteredWorldState::getView() const
{
	return *mAvatar.getView();
}

}
