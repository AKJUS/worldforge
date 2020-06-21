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

#include "NonConnectedState.h"
#include "ConnectingState.h"

#include "framework/Tokeniser.h"
#include "framework/ConsoleBackend.h"
#include "framework/LoggingInstance.h"

namespace Ember {

NonConnectedState::NonConnectedState(ServerServiceSignals& signals, Eris::Session& session) :
		Connect("connect", this, "Connect to a server."),
		mSignals(signals),
		mSession(session),
		mChildState(nullptr) {
}

void NonConnectedState::destroyChildState() {
	//Make sure to sever the connection, so that we don't end up in an infinite loop if something goes wrong when shutting down.
	mDisconnectedConnection.disconnect();
	if (mChildState) {
		mChildState->destroyChildState();
		delete mChildState;
		mChildState = nullptr;
	}
}

ServerServiceSignals& NonConnectedState::getSignals() const {
	return mSignals;
}

IState& NonConnectedState::getTopState() {
	if (mChildState) {
		return mChildState->getTopState();
	}
	return *this;
}

bool NonConnectedState::connect(const std::string& host, short port) {
	destroyChildState();
	mChildState = new ConnectingState(*this, mSession, host, port);
	if (!mChildState->connect()) {
		destroyChildState();
	} else {
		mDisconnectedConnection.disconnect();
		mDisconnectedConnection = mChildState->getConnection().Disconnected.connect(sigc::mem_fun(*this, &NonConnectedState::disconnected));
	}

	return mChildState != nullptr;
}

bool NonConnectedState::connectLocal(const std::string& socket) {
	destroyChildState();
	mChildState = new ConnectingState(*this, mSession, socket);
	if (!mChildState->connect()) {
		destroyChildState();
	} else {
		mDisconnectedConnection.disconnect();
		mDisconnectedConnection = mChildState->getConnection().Disconnected.connect(sigc::mem_fun(*this, &NonConnectedState::disconnected));
	}

	return mChildState != nullptr;
}


void NonConnectedState::disconnected() {
	S_LOG_INFO("Disconnected");

	ConsoleBackend::getSingleton().pushMessage("Disconnected from server.", "important");

	destroyChildState();
}

void NonConnectedState::runCommand(const std::string& command, const std::string& args) {
	// Connect command
	if (Connect == command) {
		// Split string into server / port pair
		Tokeniser tokeniser = Tokeniser();
		tokeniser.initTokens(args);
		std::string server = tokeniser.nextToken();
		std::string port = tokeniser.remainingTokens();
		std::string msg;
		msg = "Connecting to: [" + server + "]";
		ConsoleBackend::getSingleton().pushMessage(msg, "info");
		if (port.empty())
			connect(server);
		else
			connect(server, (short) std::stoi(port));

		// Disonnect command
	}
}

void NonConnectedState::disconnect() {
}

bool NonConnectedState::logout() {
	return false;
}

void NonConnectedState::takeTransferredCharacter(const Eris::TransferInfo& transferInfo) {
}

void NonConnectedState::transfer(const Eris::TransferInfo& transferInfo) {
	//Start by disconnecting from current server, and reconnecting to new server.
	destroyChildState();
	connect(transferInfo.getHost(), static_cast<short>(transferInfo.getPort()));
}

}
