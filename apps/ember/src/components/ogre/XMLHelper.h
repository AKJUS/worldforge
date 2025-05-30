//
// C++ Interface: XMLHelper
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2007
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
#ifndef EMBEROGREXMLHELPER_H
#define EMBEROGREXMLHELPER_H

#include "tinyxml/tinyxml.h"
#include <OgreDataStream.h>
#include <filesystem>


namespace Ember::OgreView {

/**
@brief A simple helper class for loading tinyxml documents through the ogre resource system.
Also contains a couple of static method for converting common Ogre structures to and from xml.

@author Erik Ogenvik <erik@worldforge.org>
*/
class XMLHelper {
public:

	/**
	 * Ctor.
	 */
	XMLHelper() = default;

	/**
	 * Dtor.
	 */
	~XMLHelper() = default;

	/**
	 Attempts to load the supplied stream into the document. Failures will be logged.
	 @param An empty xml document.
	 @param An opened and valid data stream
	 @returns true if successful, else false
	 */
	static bool Load(TiXmlDocument& xmlDoc, const Ogre::DataStreamPtr& stream);


	static bool Load(TiXmlDocument& xmlDoc, std::istream& stream, const std::filesystem::path& path);

	/**
	 * @brief Utility method for filling an Ogre Vector3 with data from an xml element.
	 * @param elem The xml element from which data will be taken. The xml element must have three numerical properties, "x", "y" and "z".
	 * @return An Ogre::Vector3 instance.
	 */
	static Ogre::Vector3 fillVector3FromElement(TiXmlElement* elem);

	/**
	 * @brief Utility method for filling an xml element with data from an Ogre::Vector3 instance.
	 * @param elem The element which should be filled. The data will be represented through three numerical properties, "x", "y" and "z".
	 * @param vector The Ogre Vector3 instance from which the data will be taken.
	 */
	static void fillElementFromVector3(TiXmlElement& elem, const Ogre::Vector3& vector);

	/**
	 * @brief Utility method for filling an Ogre Quaternion with data from an xml element.
	 * @param elem The xml element from which data will be taken. The xml element must have four numerical properties, "w", "x", "y" and "z".
	 * @return An Ogre::Quaternion instance.
	 */
	static Ogre::Quaternion fillQuaternionFromElement(TiXmlElement* elem);

	/**
	 * @brief Utility method for filling an xml element with data from an Ogre::Quaternion instance.
	 * @param elem The element which should be filled. The data will be represented through four numerical properties, "w", "x", "y" and "z".
	 * @param vector The Ogre Quaternion instance from which the data will be taken.
	 */
	static void fillElementFromQuaternion(TiXmlElement& elem, const Ogre::Quaternion& quaternion);

};

}



#endif
