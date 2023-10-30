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

#ifndef SERVERSERVICECONNECTIONLISTENER_H_
#define SERVERSERVICECONNECTIONLISTENER_H_

#include "IConnectionListener.h"

namespace Ember
{

class ServerServiceSignals;

class ServerServiceConnectionListener: public IConnectionListener
{
public:
	explicit ServerServiceConnectionListener(ServerServiceSignals& signals);

	~ServerServiceConnectionListener() override = default;

	void sendingObject(const Atlas::Objects::Root& obj) override;

	void receivedObject(const Atlas::Objects::Root& obj) override;

protected:
	ServerServiceSignals& mSignals;
};
}
#endif /* SERVERSERVICECONNECTIONLISTENER_H_ */
