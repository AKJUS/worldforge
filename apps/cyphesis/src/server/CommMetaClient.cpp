// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000,2001 Alistair Riddoch
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



/*
 * Imported MetaServer API implementation.
 */
#include "MetaServerAPI.hpp"

#include "CommMetaClient.h"

#include "common/debug.h"
#include "common/log.h"
#include "common/globals.h"
#include "common/Monitors.h"
#include "Remotery.h"
#include <varconf/config.h>

#include <boost/algorithm/string.hpp>


using namespace boost::asio;
static constexpr auto debug_flag = false;


/// \brief Constructor for metaserver communication socket object.
///
/// @param svr Reference to the object that manages all socket communication.
CommMetaClient::CommMetaClient(io_context& ioService) :
		mSocket(ioService, ip::udp::endpoint(ip::udp::v4(), 0)),
		mKeepaliveTimer(ioService),
		mResolver(ioService),
		mHasEndpoint(false),
		mReadBuffer{},
		m_heartbeatTime(300) {
}

CommMetaClient::~CommMetaClient() = default;

/// \brief Set the target address if the communication socket.
///
/// @param mserver String address of the metaserver,
int CommMetaClient::setup(const std::string& mserver) {
	m_server = mserver;

	/*
	 * Ensure attributes list is clear
	 */
	m_serverAttributes.clear();

	/*
	 * Get the heartbeat time
	 */
	readConfigItem("cyphesis", "msheartbeat", m_heartbeatTime);

	/*
	 * Update the attributes to include config + monitors
	 */
	updateAttributes();

	mResolver.async_resolve(mserver, "8453",
							[this, mserver](boost::system::error_code ec, ip::udp::resolver::results_type iterator) {
								if (!ec) {
									spdlog::debug("Connected to meta server at {}.", mserver);
									if (!iterator.empty()) {
										mHasEndpoint = true;
										mDestination = iterator.begin()->endpoint();
										this->metaserverKeepalive();
										this->do_receive();
										this->keepalive();
									}
								}
							});

	return 0;
}

void CommMetaClient::do_receive() {
	mSocket.async_receive_from(buffer(mReadBuffer.data(), MAX_PACKET_BYTES), mDestination,
							   [this](boost::system::error_code ec, std::size_t length) {
								   if (!ec) {
									   this->metaserverReply(length);
									   this->do_receive();
								   }
							   });
}

void CommMetaClient::keepalive() {

	mKeepaliveTimer.expires_after(std::chrono::seconds(m_heartbeatTime));
	mKeepaliveTimer.async_wait([this](boost::system::error_code ec) {
		if (!ec) {
			mResolver.async_resolve(m_server, "8453",
									[this](boost::system::error_code ecInner, ip::udp::resolver::results_type iterator) {
										if (!ecInner) {
											rmt_ScopedCPUSample(MetaClientKeepAlive, 0)
											if (!iterator.empty()) {
												mDestination = iterator.begin()->endpoint();
												mHasEndpoint = true;
											} else {
												mHasEndpoint = false;
											}
											this->metaserverKeepalive();
										}
									});
			this->keepalive();
		}
	});
}

/// \brief Send a keepalive packet to the metaserver.
///
/// This should be called periodically to send a packet notifying the
/// metaserver that this server is still alive.
void CommMetaClient::metaserverKeepalive() {

	auto keep = std::make_shared<MetaServerPacket>();

	keep->setPacketType(NMT_SERVERKEEPALIVE);

	spdlog::trace("Sending keep-alive to meta server.");
	mSocket.async_send_to(buffer(keep->getBuffer().data(), keep->getSize()),
						  mDestination,
						  [this, keep](boost::system::error_code ec, std::size_t length) {
							  if (!ec) {
								  updateAttributes();
								  sendAllAttributes();
							  }
						  });
}

/// \brief Read a reply from the metaserver.
///
/// Read the data sent by the metaserver. If the packet received is
/// a handshake, respond with the required response to verify that
/// we are alive.
void CommMetaClient::metaserverReply(size_t packet_size) {
	if (packet_size < sizeof(uint32_t)) {
		spdlog::warn("WARNING: Reply from metaserver too short");
		return;
	}

	MetaServerPacket shake(mReadBuffer, packet_size);

	if (shake.getPacketType() == NMT_HANDSHAKE) {
		uint32_t handshake = shake.getIntData(4); // we know 4 bytes for type, and 4 for shake
		spdlog::debug("MetaServer contacted successfully.");

		auto servershake = std::make_shared<MetaServerPacket>();
		servershake->setPacketType(NMT_SERVERSHAKE);
		servershake->addPacketData(handshake);


		mSocket.async_send_to(buffer(servershake->getBuffer().data(), servershake->getSize()), mDestination,
							  [servershake](boost::system::error_code ec, std::size_t length) {});


	}
}

/// \brief Send a terminate packet to the metaserver.
///
/// This should be called to indicate that this server is going down,
/// and should no longer be listed.
void CommMetaClient::metaserverTerminate() {

	mKeepaliveTimer.cancel();
	if (mHasEndpoint) {
		try {
			auto term = std::make_shared<MetaServerPacket>();

			term->setPacketType(NMT_TERMINATE);

			//Do a blocking send as we're calling this when we're shutting down.
			mSocket.send_to(buffer(term->getBuffer().data(), term->getSize()), mDestination);
		} catch (const std::exception& e) {
			//This isn't fatal
			spdlog::info("Got error when trying to send data to the meta server at shutdown: {}", e.what());
		}
	}
}

void CommMetaClient::metaserverAttribute(const std::string& k, const std::string& v) {
	auto m = std::make_shared<MetaServerPacket>();

	m->setPacketType(NMT_SERVERATTR);
	m->addPacketData(k.length());
	m->addPacketData(v.length());
	m->addPacketData(k);
	m->addPacketData(v);


	mSocket.async_send_to(buffer(m->getBuffer().data(), m->getSize()), mDestination,
						  [m](boost::system::error_code ec, std::size_t length) {});
}

/// \brief Read attributes from config and monitor
///
///
void CommMetaClient::updateAttributes() {
	std::string mstats;
	std::vector<std::string> fields;

	/*
	 * Get pipe separate list from the config about monitored variables
	 */
	readConfigItem("cyphesis", "metastats", mstats);

	/*
	 * Split on | character
	 */
	boost::algorithm::split(fields, mstats, boost::algorithm::is_any_of("|"));

	/*
	 * Iterate fields and snag from the monitor, if it exists;
	 */
	for (auto& v: fields) {
		std::stringstream ss;
		if (Monitors::instance().readVariable(v, ss) == 0) {
			std::string tmp = ss.str();
			boost::algorithm::trim(tmp);
			m_serverAttributes[v] = tmp;
		} else {
			ss << "WARNING: [cyphesis].metastats item [" << v << "] is not a monitored variable";
			spdlog::warn(ss.str());
		}
	}

	/*
	 * Get the configuration values to send
	 * In the event that a user has specified a variable that matches
	 * the name of a monitor, the configured item will have priority.
	 */
	auto c = global_conf->getSection("metaattributes");
	if (c) {
		for (auto& kv: *c) {
			std::string s = kv.second.as_string();
			boost::algorithm::trim(s);
			m_serverAttributes[kv.first] = s;
		}
	}


}

/// \brief Utility to send all attributes to the metaserver
///
void CommMetaClient::sendAllAttributes() {
	for (auto& kv: m_serverAttributes) {
		metaserverAttribute(kv.first, kv.second);
	}
}
