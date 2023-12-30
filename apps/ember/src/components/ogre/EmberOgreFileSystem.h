/*
-----------------------------------------------------------------------------
This source file is based on source files that are part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Copyright (c) 2008 Erik Ogenvik <erik@ogenvik.org>
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA., or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef _EmberOgreFileSystem_H_
#define _EmberOgreFileSystem_H_

#include <OgrePrerequisites.h>

#include <OgreArchive.h>
#include <OgreArchiveFactory.h>

#include <filesystem>
#include <regex>


namespace Ember::OgreView {

/** Specialisation of the Archive class to allow reading of files from
	filesystem folders / directories.
	This has been modified from the original Ogre class to:
	1) not visit hidden directories (such as .svn)
	2) not recurse into directories if there's a file named "norecurse" in them
*/
class FileSystemArchive : public Ogre::Archive {
protected:

	std::filesystem::path mBaseName;

	/** Utility method to retrieve all files in a directory matching pattern.
	@param pattern File pattern
	@param recursive Whether to cascade down directories
	@param dirs Set to true if you want the directories to be listed
		instead of files
	@param simpleList Populated if retrieving a simple list
	@param detailList Populated if retrieving a detailed list
	*/
	void findFiles(Ogre::String pattern, bool recursive, bool dirs,
				   Ogre::StringVector* simpleList, Ogre::FileInfoList* detailList) const;

	void findFiles(const std::filesystem::path& directory, const std::unique_ptr<std::regex>& pattern, bool recursive, bool dirs,
				   Ogre::StringVector* simpleList, Ogre::FileInfoList* detailList) const;

public:
	FileSystemArchive(const Ogre::String& name, const Ogre::String& archType);

	~FileSystemArchive() override;

	/// @copydoc Archive::isCaseSensitive
	bool isCaseSensitive() const override;

	/// @copydoc Archive::load
	void load() override;

	/// @copydoc Archive::unload
	void unload() override;

	/// @copydoc Archive::open
	Ogre::DataStreamPtr open(const Ogre::String& filename, bool readOnly) const override;

	/// @copydoc Archive::list
	Ogre::StringVectorPtr list(bool recursive, bool dirs) const override;

	/// @copydoc Archive::listFileInfo
	Ogre::FileInfoListPtr listFileInfo(bool recursive, bool dirs) const override;

	/// @copydoc Archive::find
	Ogre::StringVectorPtr find(const Ogre::String& pattern, bool recursive,
							   bool dirs) const override;

	/// @copydoc Archive::findFileInfo
	Ogre::FileInfoListPtr findFileInfo(const Ogre::String& pattern, bool recursive,
									   bool dirs) const override;

	/// @copydoc Archive::exists
	bool exists(const Ogre::String& filename) const override;

	/**
	 * @copydoc Ogre::Archive::getModifiedTime
	 */
	time_t getModifiedTime(const Ogre::String& filename) const override;

};

/** Specialisation of ArchiveFactory for FileSystem files. */
class FileSystemArchiveFactory : public Ogre::ArchiveFactory {
public:
	~FileSystemArchiveFactory() override = default;

	/// @copydoc FactoryObj::getType
	const Ogre::String& getType() const override;

	/// @copydoc FactoryObj::createInstance
	Ogre::Archive* createInstance(const Ogre::String& name, bool readOnly) override {
		//FIXME: use the readOnly parameter
		return new OgreView::FileSystemArchive(name, "EmberFileSystem");
	}

	/// @copydoc FactoryObj::destroyInstance
	void destroyInstance(Ogre::Archive* arch) override { delete arch; }
};


}



#endif
