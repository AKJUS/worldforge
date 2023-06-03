#include <utility>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BaseConnection.h"

#include "Log.h"
#include "StreamSocket_impl.h"

#include <Atlas/Codec.h>
#include <Atlas/Net/Stream.h>
#include <Atlas/Objects/Encoder.h>

#include <sstream>
#include <Atlas/Objects/Factories.h>
#include "CustomEntities.h"

using namespace boost::asio;

namespace Eris
{

////////////////////////////////////////////////////////////////////////////////////////////////////////    

BaseConnection::BaseConnection(io_service& io_service,
							   std::string clientName,
							   std::string id) :
		_io_service(io_service),
		_factories(new Atlas::Objects::Factories()),
		_status(DISCONNECTED),
		_id(std::move(id)),
		_clientName(std::move(clientName)),
		_bridge(nullptr),
		_port(0) {
	if (!_factories->hasFactory("sys")) {
		Atlas::Objects::Entity::SYS_NO = _factories->addFactory("sys",
													   &Atlas::Objects::factory<Atlas::Objects::Entity::SysData>, &Atlas::Objects::defaultInstance<Atlas::Objects::Entity::SysData>);
	}
}

BaseConnection::~BaseConnection()
{
    if (_status != DISCONNECTED) {
        hardDisconnect(true);
    }
    if (_socket) {
        _socket->detach();
        _socket.reset();
    }
}

int BaseConnection::connectRemote(const std::string & host, short port)
{
    if (_socket) {
        _socket->detach();
        _socket.reset();
    }
    try {
        StreamSocket::Callbacks callbacks;
        callbacks.dispatch = [&] {this->dispatch();};
        callbacks.stateChanged =
                [&](StreamSocket::Status state) {
            if (state == StreamSocket::NEGOTIATE) {
                //Turn off Nagle's algorithm to increase responsiveness.
                ((ResolvableAsioStreamSocket<ip::tcp>*)_socket.get())->getAsioSocket().set_option(ip::tcp::no_delay(true));
            }
            this->stateChanged(state);};
        auto socket = new ResolvableAsioStreamSocket<ip::tcp>(_io_service, _clientName,
                *_bridge, callbacks);
        _socket.reset(socket);
        std::stringstream ss;
        ss << port;
        ip::tcp::resolver::query query(host, ss.str());
        setStatus(CONNECTING);
        socket->connectWithQuery(query);
    } catch (const std::exception& e) {
        error() << "Error when trying to connect to " << host << " on port "
                << port << ": " << e.what();
        hardDisconnect(true);
        return -1;
    }
    return 0;
}

int BaseConnection::connectLocal(const std::string & filename)
{
    if (_socket) {
        _socket->detach();
        _socket.reset();
    }
#ifdef _WIN32
	return 0;
#else
    try {
        StreamSocket::Callbacks callbacks;
        callbacks.dispatch = [&] {this->dispatch();};
        callbacks.stateChanged =
                [&](StreamSocket::Status state) {this->stateChanged(state);};
        auto socket = new AsioStreamSocket<local::stream_protocol>(
                _io_service, _clientName, *_bridge, callbacks);
        _socket.reset(socket);
        setStatus(CONNECTING);
        socket->connect(local::stream_protocol::endpoint(filename));
    } catch (const std::exception& e) {
        hardDisconnect(true);
        return -1;
    }
    return 0;
#endif
}

void BaseConnection::stateChanged(StreamSocket::Status status)
{
    switch (status) {
    case StreamSocket::CONNECTING:
        setStatus(CONNECTING);
        break;
    case StreamSocket::CONNECTING_TIMEOUT:
        onConnectTimeout();
        break;
    case StreamSocket::CONNECTING_FAILED:
        handleFailure("Failed to connect to " + _host);
        hardDisconnect(true);
        break;
    case StreamSocket::NEGOTIATE:
        setStatus(NEGOTIATE);
        break;
    case StreamSocket::NEGOTIATE_FAILED:
        hardDisconnect(true);
        break;
    case StreamSocket::NEGOTIATE_TIMEOUT:
        onNegotiateTimeout();
        break;
    case StreamSocket::CONNECTED:
        setStatus(CONNECTED);
        onConnect();
        break;
    case StreamSocket::CONNECTION_FAILED:
        hardDisconnect(true);
        break;
    case StreamSocket::DISCONNECTING:
        setStatus(DISCONNECTING);
        break;
    default:
        break;
    }
}

void BaseConnection::hardDisconnect(bool emit)
{
    if (_status == DISCONNECTED)
        return;

    if (_socket) {
        _socket->detach();
        _socket.reset();
    }

    setStatus(DISCONNECTED);
    if (emit) {
        Disconnected.emit();
    }
}

void BaseConnection::onConnect()
{
    // tell anyone who cares with a signal
    Connected.emit();
}

void BaseConnection::onConnectTimeout()
{
    std::ostringstream os;
    os << "Connect to " << _host << ':' << _port << " timed out";
    handleTimeout(os.str());
    hardDisconnect(true);
}

void BaseConnection::onNegotiateTimeout()
{
    handleTimeout("Atlas negotiation timed out");
    hardDisconnect(true);
}

void BaseConnection::setStatus(Status sc)
{
    _status = sc;
}

const std::string& BaseConnection::getHost() const
{
    return _host;
}

short BaseConnection::getPort() const
{
    return _port;
}

Atlas::Objects::Factories& BaseConnection::getFactories() {
	return *_factories;
}

const Atlas::Objects::Factories& BaseConnection::getFactories() const {
	return *_factories;
}

} // of namespace
