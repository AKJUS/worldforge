// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2001 Alistair Riddoch
// Copyright (C) 2013 Erik Ogenvik
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#ifndef CLIENT_CLIENT_CONNECTION_H
#define CLIENT_CLIENT_CONNECTION_H

#include <common/AtlasStreamClient.h>

#include <Atlas/Objects/ObjectsFwd.h>
#include <Atlas/Objects/SmartPtr.h>
#include <Atlas/Objects/Root.h>

#include <deque>
#include "common/Link.h"

/// \brief Class to handle socket connection to a cyphesis server from an
/// an admin client
class ClientConnection : public Link {

  public:
    explicit ClientConnection(CommSocket & commSocket, const std::string & id, long iid);

    ~ClientConnection() override = default;

    friend class ClientConnectionintegration;
    friend class ClientConnectiontest;
};

#endif // CLIENT_CLIENT_CONNECTION_H
