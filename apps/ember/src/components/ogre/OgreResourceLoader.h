//
// C++ Interface: OgreResourceLoader
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2006,
// Rômulo Fernandes <abra185@gmail.com>, (C) 2008
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.//
//
#ifndef EMBEROGREOGRERESOURCELOADER_H
#define EMBEROGREOGRERESOURCELOADER_H

#include "framework/ConsoleCommandWrapper.h"
#include "framework/Session.h"
#include <filesystem>
#include <squall/core/Repository.h>

namespace Ember::OgreView {

class FileSystemArchiveFactory;

struct EmberResourceLoadingListener;
struct SquallArchiveFactory;

/**
@author Erik Ogenvik
@author Rômulo Fernandes

@brief Loads resources into the Ogre resource system.

The main role of this class is to define and load the resources into the Ogre resource system.

If a directory contains a file named "norecurse" (it can be empty) Ember won't recurse further into it
*/
class OgreResourceLoader : public ConsoleObject, public boost::noncopyable {
public:
	enum class OnFailure {
		Ignore,
		Report,
		Throw
	};


	explicit OgreResourceLoader(Squall::Repository repository);

	~OgreResourceLoader() override;

	void initialize();

	void loadBootstrap();

	void loadGui();

	void loadGeneral();

	void preloadMedia();

	/**
	 * @brief Tells Ogre to unload all unused resources, thus freeing up memory.
	 * @note Calling this might stall the engine a little.
	 */
	void unloadUnusedResources();

	/**
	 * @copydoc ConsoleObject::runCommand
	 */
	void runCommand(const std::string& command, const std::string& args) override;

	/**
	 * Adds a media location in the filesystem.
	 *
	 * This will first look if there's any processed media under the build directory.
	 * Otherwise it will look if there's any raw media from the "mediarepo" directory under source.
	 * And lastly, if nothing is found, media from the install location will be added.
	 * @param path
	 * @param resourceGroup
	 * @return True if media was found.
	 */
	bool addMedia(const std::string& path, const std::string& resourceGroup);

	bool addSquallMedia(Squall::Signature signature);

	void replaceSquallMedia(Squall::Signature signature);

	std::optional<Squall::Signature> getSquallSignature() const {
		return mSquallSignature;
	}


private:

	/**
	 * @brief Allows setting of the right hand attachment's orientation. This is mainly for debugging purposes and should removed once we get a better editor in place.
	 */
	ConsoleCommandWrapper UnloadUnusedResources;

	/**
	 * @brief A store of extra locations, as specified in config or command line.
	 */
	std::vector<std::string> mExtraResourceLocations;

	std::unique_ptr<FileSystemArchiveFactory> mFileSystemArchiveFactory;

	std::unique_ptr<SquallArchiveFactory> mSquallArchiveFactory;

	std::unique_ptr<EmberResourceLoadingListener> mLoadingListener;

	std::vector<std::string> mResourceRootPaths;

	Squall::Repository mRepository;

	std::optional<Squall::Signature> mSquallSignature;


	bool addUserMedia(const std::string& path, const std::string& type, const std::string& section);

	bool addSharedMedia(const std::string& path, const std::string& type, const std::string& section);

	/**
	 * @brief Adds a resource directory to the Ogre resource system.
	 * @param path File system path.
	 * @param type The type of archive.
	 * @param section The resource group to add it to.
	 * @param onFailure What to do if we fail to find the directory.
	 * @return True if the path was successfully added.
	 */
	bool addResourceDirectory(const std::filesystem::path& path,
							  const std::string& type,
							  const std::string& section,
							  OnFailure onFailure);

	void observeDirectory(const std::filesystem::path& path, std::string group);


};

}

#endif
