/*
 Copyright (C) 2014 Erik Ogenvik

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

#ifndef STREAMSOCKET_H_
#define STREAMSOCKET_H_

#include <Atlas/Objects/ObjectsFwd.h>
#include <Atlas/Negotiate.h>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/noncopyable.hpp>

#include <memory>

namespace Atlas
{
class Bridge;
class Codec;
namespace Net
{
class StreamConnect;
}
namespace Objects
{
class ObjectsEncoder;
}
}

namespace Eris
{

/**
 * @brief Handles the internal socket instance, interacting with the asynchronous io_service calls.
 *
 * Since this will be used to make asynchronous calls it must be wrapped in a shared_ptr.
 * When the owner instance is destroyed it must call "detach" to make sure the connection is severed.
 */
class StreamSocket: public std::enable_shared_from_this<StreamSocket>, private boost::noncopyable
{
public:

    typedef enum
    {
        INVALID_STATUS = 0, ///< indicates an illegal state
        CONNECTING, ///< stream / socket connection in progress
        CONNECTING_TIMEOUT, ///< timeout when trying to establish a connection
        CONNECTING_FAILED, ///< failure when trying to establish a connection
        NEGOTIATE, ///< Atlas negotiation in progress
        NEGOTIATE_TIMEOUT, ///< timeout when negotiating
        NEGOTIATE_FAILED, ///< failure when negotiating
        CONNECTED, ///< connection fully established
        CONNECTION_FAILED, ///< connection failed
        DISCONNECTED, ///< finished disconnection
        DISCONNECTING ///< clean disconnection in progress
    } Status;

    /**
     * @brief Methods that are used as callbacks.
     */
    struct Callbacks
    {
        /**
         * @brief Called when operations have arrived and needs dispatching.
         */
        std::function<void()> dispatch;

        /**
         * @brief Called when the internal state has changed.
         */
        std::function<void(Status)> stateChanged;
    };

    StreamSocket(boost::asio::io_service& io_service,
            const std::string& client_name,
            Atlas::Bridge& bridge,
            Callbacks callbacks);
    virtual ~StreamSocket();

    /**
     * @brief Detaches the callbacks.
     *
     * Call this when the owner instance is destroyed, or you otherwise don't want any callbacks.
     */
    void detach();

    /**
     * @brief Gets the codec object.
     * @note Only call this after the socket has successfully negotiated.
     * @return
     */
    Atlas::Codec& getCodec();

    /**
     * @brief Gets the encoder object.
     * @note Only call this after the socket has successfully negotiated.
     * @return
     */
    Atlas::Objects::ObjectsEncoder& getEncoder();

    /**
     * @brief Send any unsent data.
     */
    virtual void write() = 0;
protected:
    enum
    {
        read_buffer_size = 2048
    };
    Atlas::Bridge& _bridge;
    Callbacks _callbacks;

    /**
     * Buffer used to write data to be sent.
     * Swapped with mSendBuffer once data is being sent.
     */
    std::unique_ptr<boost::asio::streambuf> mWriteBuffer;

    /**
     * Buffer of data which is being sent. This should not be touched until
     * the async_write call completes.
     */
    std::unique_ptr<boost::asio::streambuf> mSendBuffer;

    /**
     * Buffer for data being read from the socket.
     */
    boost::asio::streambuf mReadBuffer;

    /**
     * Stream for data being received.
     */
    std::istream mInStream;

    /**
     * Stream for data being sent out.
     */
    std::ostream mOutStream;

    /**
     * True if we should send again as soon as an ongoing async_write operation completes.
     */
    bool mShouldSend;

    /**
     * True if we're currently sending through an async_write (and thus shouldn't touch mSendBuffer).
     */
    bool mIsSending;

    std::unique_ptr<Atlas::Net::StreamConnect> _sc; ///< negotiation object (nullptr after connection!)
    boost::asio::steady_timer _negotiateTimer;
    boost::asio::steady_timer _connectTimer;
    std::unique_ptr<Atlas::Codec> m_codec;
    std::unique_ptr<Atlas::Objects::ObjectsEncoder> m_encoder;
    bool m_is_connected;

    virtual void do_read() = 0;
    virtual void negotiate_read() = 0;
    void startNegotiation();
    Atlas::Negotiate::State negotiate();

};

/**
 * @brief Template specialization which uses boost::asio sockets.
 */
template<typename ProtocolT>
class AsioStreamSocket: public StreamSocket
{
public:
    AsioStreamSocket(boost::asio::io_service& io_service,
            const std::string& client_name, Atlas::Bridge& bridge,
            StreamSocket::Callbacks callbacks);
    ~AsioStreamSocket() override;
    void connect(const typename ProtocolT::endpoint& endpoint);
    void write() override;
    typename ProtocolT::socket& getAsioSocket();
protected:
    typename ProtocolT::socket m_socket;
    void negotiate_read() override;
    void negotiate_write();
    void do_read() override;
};

/**
 * @brief Template specialization which uses boost::asio sockets with resolvers (i.e. TCP and UDP, but not domain sockets).
 */
template<typename ProtocolT>
class ResolvableAsioStreamSocket: public AsioStreamSocket<ProtocolT>
{
public:
    ResolvableAsioStreamSocket(boost::asio::io_service& io_service,
            const std::string& client_name, Atlas::Bridge& bridge,
            StreamSocket::Callbacks callbacks);
    void connectWithQuery(const typename ProtocolT::resolver::query& query);
protected:
    typename ProtocolT::resolver m_resolver;
};

}
#endif /* STREAMSOCKET_H_ */
