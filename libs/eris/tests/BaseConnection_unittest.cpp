// Eris Online RPG Protocol Library
// Copyright (C) 2007 Alistair Riddoch
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

// $Id$

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include <Eris/BaseConnection.h>
#include <Eris/Exceptions.h>
#include <Eris/Log.h>

#include "SignalFlagger.h"

#include <Atlas/Codecs/XML.h>
#include <Atlas/Net/Stream.h>
#include <Atlas/Message/QueuedDecoder.h>
#include <Atlas/Objects/Factories.h>
#include <Atlas/Objects/Encoder.h>

#include <iostream>

#include <cassert>

class TestStreamClientSocketBase : public Eris::StreamSocket {
public:
    TestStreamClientSocketBase(boost::asio::io_service& io_service, const std::string& client_name, Atlas::Bridge& bridge, Callbacks& callbacks)
: Eris::StreamSocket(io_service, client_name, bridge, callbacks)
{}
    virtual void write(){}
protected:
    virtual void do_read(){}
    virtual void negotiate_read(){}
};

class TestBaseConnection : public Eris::BaseConnection {
  public:
    bool failure;
    bool timeout;

    TestBaseConnection(boost::asio::io_service& io_service, Atlas::Bridge * b) : Eris::BaseConnection(io_service, "test", "1"), failure(false), timeout(false) {
    	_bridge = b;
    }

    virtual void handleFailure(const std::string & msg) {
        failure = true;
    }

    virtual void handleTimeout(const std::string & msg) {
        timeout = true;
    }

    void dispatch()
    {
    }

    void test_setStatus(Eris::BaseConnection::Status sc) {
        setStatus(sc);
    }

    void test_onConnect() {
        onConnect();
    }

    void test_hardDisconnect(bool flag) {
        hardDisconnect(flag);
    }

    void test_onConnectTimeout() {
        onConnectTimeout();
    }

    void test_onNegotiateTimeout() {
        onNegotiateTimeout();
    }

    void setup_socket() {
        Eris::StreamSocket::Callbacks callbacks;
        _socket.reset(new TestStreamClientSocketBase(_io_service, "", *_bridge, callbacks));
    }

};

static void writeLog(Eris::LogLevel, const std::string & msg)
{
    std::cerr << msg << std::endl;
}

int main()
{

    boost::asio::io_service io_service;
    Eris::Logged.connect(sigc::ptr_fun(writeLog));

    {
		Atlas::Message::QueuedDecoder bridge;
        TestBaseConnection tbc(io_service, &bridge);
    }


    // Test the other code path when a second connection is created.
    {
		Atlas::Message::QueuedDecoder bridge;
        TestBaseConnection tbc(io_service, &bridge);
    }

    // Test isConnected.
    {
		Atlas::Message::QueuedDecoder bridge;
        TestBaseConnection tbc(io_service, &bridge);

        assert(!tbc.isConnected());
    }

    // Test getStatus().
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        assert(tbc.getStatus() == Eris::BaseConnection::DISCONNECTED);
    }

    // Test setStatus().
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        assert(tbc.getStatus() == Eris::BaseConnection::DISCONNECTED);

        tbc.test_setStatus(Eris::BaseConnection::CONNECTED);

        assert(tbc.getStatus() == Eris::BaseConnection::CONNECTED);

        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTED);
    }

    // Test connect() and verify getStatus() changes.
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        assert(tbc.getStatus() == Eris::BaseConnection::DISCONNECTED);

        int ret = tbc.connectRemote("localhost", 6723);

        assert(ret == 0);

        assert(tbc.getStatus() == Eris::BaseConnection::CONNECTING);
    }

    // Test onConnect() does nothing.
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        tbc.test_onConnect();
    }

    // Test onConnect() emits the signal.
    {
        SignalFlagger onConnect_checker;

        assert(!onConnect_checker.flagged());

		Atlas::Message::QueuedDecoder bridge;
        TestBaseConnection tbc(io_service, &bridge);

        tbc.Connected.connect(sigc::mem_fun(onConnect_checker,
                                            &SignalFlagger::set));

        assert(!onConnect_checker.flagged());

        tbc.test_onConnect();

        assert(onConnect_checker.flagged());
    }

    // Test hardDisconnect() does nothing.
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        tbc.test_hardDisconnect(true);
    }

//    // Test hardDisconnect() throws in polldefault when connected
//    {
//        TestBaseConnection tbc(io_service, &bridge);
//
//        // Add members to be consistent with connected state.
//        tbc.setup_stream();
//        tbc.setup_codec();
//        tbc.setup_encode();
//        // Make the state different
//        tbc.test_setStatus(Eris::BaseConnection::CONNECTED);
//
//        try {
//            tbc.test_hardDisconnect(true);
//        }
//        catch (Eris::InvalidOperation & eio) {
//        }
//
//        // Make it disconnected again, or we crash on destructor
//        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTED);
//    }
//
//    // Test hardDisconnect() throws in polldefault when disconnecting
//    {
//        TestBaseConnection tbc(io_service, &bridge);
//
//        // Add members to be consistent with disconnecting state.
//        tbc.setup_stream();
//        tbc.setup_codec();
//        tbc.setup_encode();
//        // Make the state different
//        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTING);
//
//        try {
//            tbc.test_hardDisconnect(true);
//        }
//        catch (Eris::InvalidOperation & eio) {
//        }
//
//        // Make it disconnected again, or we crash on destructor
//        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTED);
//    }
//
//    // Test hardDisconnect() throws in polldefault when negotiating
//    {
//        TestBaseConnection tbc(io_service, &bridge);
//
//        // Add members to be consistent with negotiating state.
//        tbc.setup_stream();
//        tbc.setup_sc();
//        // Make the state different
//        tbc.test_setStatus(Eris::BaseConnection::NEGOTIATE);
//
//        try {
//            tbc.test_hardDisconnect(true);
//        }
//        catch (Eris::InvalidOperation & eio) {
//        }
//
//        // Make it disconnected again, or we crash on destructor
//        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTED);
//    }
//
//    // Test hardDisconnect() throws in polldefault when negotiating
//    {
//        TestBaseConnection tbc(io_service, &bridge);
//
//        // Add members to be consistent with connecting state.
//        tbc.setup_stream();
//        // Make the state different
//        tbc.test_setStatus(Eris::BaseConnection::CONNECTING);
//
//        try {
//            tbc.test_hardDisconnect(true);
//        }
//        catch (Eris::InvalidOperation & eio) {
//        }
//
//        // Make it disconnected again, or we crash on destructor
//        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTED);
//    }

    // Test hardDisconnect() throws in polldefault when negotiating
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        // Add members to be consistent with connecting state.
        tbc.setup_socket();
        // Make the state different
        tbc.test_setStatus(Eris::BaseConnection::INVALID_STATUS);

        try {
            tbc.test_hardDisconnect(true);
        }
        catch (Eris::InvalidOperation & eio) {
        }

        // Make it disconnected again, or we crash on destructor
        tbc.test_setStatus(Eris::BaseConnection::DISCONNECTED);
    }

    // Test onConnectTimeout()
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        tbc.test_onConnectTimeout();

        assert(tbc.timeout);
    }

    // Test onNegotiateTimeout()
    {
		Atlas::Message::QueuedDecoder bridge;
		TestBaseConnection tbc(io_service, &bridge);

        tbc.test_onNegotiateTimeout();

        assert(tbc.timeout);
    }

    return 0;
}
