/**
 Worldforge Next Generation MetaServer

 Copyright (C) 2011 Sean Ryan <sryan@evercrack.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */

#include "MetaServer.hpp"
#include <boost/asio/ip/udp.hpp>
#include <arpa/inet.h>

namespace {
std::string
IpNetToAscii(uint32_t address) {
	std::array<char, INET_ADDRSTRLEN> chars{};
	inet_ntop(AF_INET, &address, chars.data(), INET_ADDRSTRLEN);
	return {chars.data()};
}
}


typedef std::vector<std::string> attribute_list;

int main(int argc, char** argv) {

	/**
	 * Argument Wrangling
	 *
	 */
	boost::program_options::options_description desc("TestClient");
	boost::program_options::variables_map vm;
	boost::asio::io_context io_service;
	std::array<char, MAX_PACKET_BYTES> recvBuffer{};
	boost::asio::ip::udp::endpoint sender_endpoint;
	size_t bytes_recvd;


	/**
	 * Note: options inside the configuration file that are NOT listed here
	 *       become ignored and are not accessible.
	 */
	desc.add_options()
			("help,h", "Display help message")
			("server", boost::program_options::value<std::string>()->default_value("localhost"), "MetaServer host. \nDefault:localhost")
			("port", boost::program_options::value<int>()->default_value(8453), "MetaServer port. \nDefault:8453")
			("attribute", boost::program_options::value<attribute_list>(), "Set client attribute.\nDefault: none")
			("filter", boost::program_options::value<attribute_list>(), "Set client filters.\nDefault: none")
			("keepalives", boost::program_options::value<int>()->default_value(3), "Number of Keepalives. \nDefault:3");

	try {
		boost::program_options::store(
				boost::program_options::parse_command_line(argc, argv, desc),
				vm
		);
		boost::program_options::notify(vm);

		/**
		 * Special case for help
		 */
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}

		std::cout << "Server       : " << vm["server"].as<std::string>() << std::endl;
		std::cout << "Port         : " << vm["port"].as<int>() << std::endl;
		std::cout << "Keepalives   : " << vm["keepalives"].as<int>() << std::endl;
		std::cout << "---------------" << std::endl;

		for (auto & it : vm) {
			if (it.second.value().type() == typeid(int)) {
				std::cout << it.first.c_str() << "=" << it.second.as<int>() << std::endl;
			} else if (it.second.value().type() == typeid(std::string)) {
				std::cout << it.first.c_str() << "=" << it.second.as<std::string>().c_str() << std::endl;
			} else if (it.second.value().type() == typeid(attribute_list)) {
				std::cout << it.first.c_str() << "=Attribute List" << std::endl;
			}
		}

		std::cout << "-------------------------" << std::endl;


		boost::asio::ip::udp::socket s(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
		boost::asio::ip::udp::resolver resolver(io_service);
		auto resolver_result = resolver.resolve(boost::asio::ip::udp::v4(), vm["server"].as<std::string>(), std::to_string(vm["port"].as<int>()));

		if (!resolver_result.empty()) {
			/**
			 *  Step 1 : keepalive x3 w/ sleep
			 */
			auto resolved = *resolver_result.begin();

			/**
			 *    1.1 - send keepalive
			 */
			auto keepalives = vm["keepalives"].as<int>();
			for (int i = 0; i < keepalives; ++i) {
				MetaServerPacket keep;
				keep.setPacketType(NMT_CLIENTKEEPALIVE);

				std::cout << "Sending keepalive ... ";
				s.send_to(boost::asio::buffer(keep.getBuffer(), keep.getSize()), resolved);

				/**
				 *    1.2 - receive handshake
				 */

				bytes_recvd = s.receive_from(boost::asio::buffer(recvBuffer), sender_endpoint);

				MetaServerPacket shake(recvBuffer, bytes_recvd);
				shake.setAddress(sender_endpoint.address().to_string(), sender_endpoint.address().to_v4().to_uint());
				shake.setPort(sender_endpoint.port());
				std::cout << "Got handshake ... ";

				unsigned int shake_key = shake.getIntData(4);

				/**
				 *    1.3 - send clientshake
				 */
				MetaServerPacket clientshake;
				clientshake.setPacketType(NMT_CLIENTSHAKE);
				clientshake.addPacketData(shake_key);
				clientshake.setAddress(shake.getAddress(), shake.getAddressInt());
				s.send_to(boost::asio::buffer(clientshake.getBuffer(), clientshake.getSize()), resolved);
				std::cout << "Sending registration." << std::endl;
				//std::cout << "Sleeping between keepalives : 2s" << std::endl;
				//sleep(2);
			}

			/**
			 *  Step 2 : register attributes if any
			 */
			if (vm.count("attribute")) {
				std::cout << "Registering Client Attributes: " << std::endl;
				attribute_list v = vm["attribute"].as<attribute_list>();
				while (!v.empty()) {
					std::string ele = v.back();
					size_t pos = ele.find_first_of('=');
					if (pos != std::string::npos) {
						std::string n = ele.substr(0, pos);
						std::string value = ele.substr(pos + 1);
						std::cout << " register: " << n << std::endl;
						std::cout << "    value: " << value << std::endl;
						MetaServerPacket a;
						a.setPacketType(NMT_CLIENTATTR);
						a.addPacketData(n.length());
						a.addPacketData(value.length());
						a.addPacketData(n);
						a.addPacketData(value);
						s.send_to(boost::asio::buffer(a.getBuffer(), a.getSize()), resolved);
					} else {
						std::cout << " Attribute Ignored : " << ele << std::endl;
					}
					v.pop_back();
				}
			}

			/**
			 *  Step 3 : register filters
			 */
			if (vm.count("filter")) {
				std::cout << "Registering Client Filters: " << std::endl;
				attribute_list v = vm["filter"].as<attribute_list>();
				while (!v.empty()) {
					std::string ele = v.back();
					size_t pos = ele.find_first_of('=');
					if (pos != std::string::npos) {
						std::string n = ele.substr(0, pos);
						std::string value = ele.substr(pos + 1);
						std::cout << " register: " << n << std::endl;
						std::cout << "    value: " << value << std::endl;
						MetaServerPacket a;
						a.setPacketType(NMT_CLIENTFILTER);
						a.addPacketData(n.length());
						a.addPacketData(value.length());
						a.addPacketData(n);
						a.addPacketData(value);
						s.send_to(boost::asio::buffer(a.getBuffer(), a.getSize()), resolved);
					} else {
						std::cout << " Filter Ignored : " << ele << std::endl;
					}
					v.pop_back();
				}
			}

			/**
			 *  Step 4 : send listreq
			 */
			unsigned int total = 1;
			unsigned int from = 0;
			while (true) {

				if (from > total || total == 0)
					break;

				std::cout << "List Request: " << std::endl;
				MetaServerPacket req;

				req.setPacketType(NMT_LISTREQ);
				req.addPacketData(from);
				req.setAddress(sender_endpoint.address().to_string(), sender_endpoint.address().to_v4().to_uint());
				req.setPort(sender_endpoint.port());
				s.send_to(boost::asio::buffer(req.getBuffer(), req.getSize()), resolved);

				bytes_recvd = s.receive_from(boost::asio::buffer(recvBuffer), sender_endpoint);

				MetaServerPacket resp(recvBuffer, bytes_recvd);
				resp.setAddress(sender_endpoint.address().to_string(), sender_endpoint.address().to_v4().to_uint());
				resp.setPort(sender_endpoint.port());

				if (resp.getPacketType() != NMT_LISTRESP || resp.getPacketType() == NMT_PROTO_ERANGE)
					break;

				std::cout << "Received server list packet";
				total = resp.getIntData(sizeof(uint32_t) * 1); // 4
				auto packed = resp.getIntData(sizeof(uint32_t) * 2); // 8
				std::cout << "  Received " << packed << " / " << total << " servers." << std::endl;

				for (size_t count = 1; count <= packed; count++) {
					unsigned int offset = (sizeof(uint32_t) * 2) + (sizeof(uint32_t) * count);
					//std::cout << "     " << count << " / " << offset << " == ";
					uint32_t ip = resp.getIntData(offset);
					//std::cout << ip << std::endl;
					std::cout << "Server: " << IpNetToAscii(ip) << std::endl;
				}
				from += packed;
			}

			/**
			 *  Step 4: send terminate
			 */
			std::cout << "Sending Terminate: " << std::endl;
			MetaServerPacket term;
			term.setPacketType(NMT_TERMINATE);
			term.addPacketData(0); // increase size of term packet to indicate client
			term.setAddress(sender_endpoint.address().to_string(), sender_endpoint.address().to_v4().to_uint());
			term.setPort(sender_endpoint.port());
			s.send_to(boost::asio::buffer(term.getBuffer(), term.getSize()), resolved);

		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	std::cout << "All Done!" << std::endl;
	return 0;
}
