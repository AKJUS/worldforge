/*
    Copyright (C) 2002  Miguel Guzman Miranda [Aglanor], Martin Pollard (Xmp)
    Copyright (C) 2005	Erik Ogenvik
    Based on YUP::Metacmd code by Adam Wendt

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

#ifndef METASERVERSERVICE_H
#define METASERVERSERVICE_H

#include <framework/Service.h>
#include <framework/ConsoleCommandWrapper.h>

#include <Eris/Metaserver.h>
#include <Eris/ServerInfo.h>

#include <string>

namespace Ember {
struct Session;

class ConfigService;

/**
 * Ember Metaserver Service
 *
 * @author Miguel Guzman Miranda [Aglanor]
 *
 * @see Ember::Service
 * @see Ember::ServerService
 * @see Ember::ConsoleObject
 */
class MetaserverService : public Service,
						  public ConsoleObject {
private:

	Session& mSession;
	std::unique_ptr<Eris::Meta> mMetaserver;
	const ConsoleCommandWrapper MetaRefresh;
	const ConsoleCommandWrapper MetaAbort;


public:

	explicit MetaserverService(Session& session, ConfigService& configSrv);

	~MetaserverService() override;


	Eris::Meta& getMetaServer() const;

	void gotFailure(const std::string& msg);

	void receivedServerInfo(const Eris::ServerInfo& sInfo);

	void completedServerList(int count);

	/**
	 * This is the function that needs to be extended to use the console.
	 * command is a command that has been previously registered with the console
	 * args is the argument string that has been provided for the command
	 */
	void runCommand(const std::string& command, const std::string& args) override;

	/**
	 * @brief Compares two version strings.
	 *
	 * A version string should be in the format <major>.<minor>.<point>.
	 * @returns 0 if the versions are the same, or if it wasn't
	 * possible to correctly parse the version string.
	 * 1 if the first version was larger than the second.
	 * -1 if the second version was larger then the first.
	 * @param firstVersion The first version to compare.
	 * @param secondVersion The second version to compare.
	 */
	static int compareVersions(const std::string& firstVersion, const std::string& secondVersion);

}; //MetaserverService

} // namespace Ember

#endif


