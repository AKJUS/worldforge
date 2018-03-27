//
// C++ Implementation: XMLHelper
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "XMLHelper.h"
#include <OgreVector3.h>

namespace Ember {
namespace OgreView {

bool XMLHelper::Load(TiXmlDocument& xmlDoc, Ogre::DataStreamPtr stream) {
	size_t length(stream->size());

	if (length) {
		// If we have a file, assume it is all one big XML file, and read it in.
		// The document parser may decide the document ends sooner than the entire file, however.
		std::string data(stream->getAsString());

		xmlDoc.Parse(data.c_str());

		if (xmlDoc.Error()) {
			std::string errorDesc = xmlDoc.ErrorDesc();
			int errorLine = xmlDoc.ErrorRow();
			int errorColumn = xmlDoc.ErrorCol();
			std::stringstream ss;
			ss << "Failed to load xml file '" << stream->getName() << "'! Error at column: " << errorColumn << " line: " << errorLine << ". Error message: " << errorDesc;
			S_LOG_FAILURE(ss.str());
			return false;
		} else {
			return true;
		}
	}
	return false;
}

bool XMLHelper::Load(TiXmlDocument& xmlDoc, std::istream& stream, const boost::filesystem::path& path) {
	stream >> xmlDoc;

	if (xmlDoc.Error()) {
		std::string errorDesc = xmlDoc.ErrorDesc();
		int errorLine = xmlDoc.ErrorRow();
		int errorColumn = xmlDoc.ErrorCol();
		std::stringstream ss;
		ss << "Failed to load xml file '" << path << "'! Error at column: " << errorColumn << " line: " << errorLine << ". Error message: " << errorDesc;
		S_LOG_FAILURE(ss.str());
		return false;
	} else {
		return true;
	}
}

Ogre::Vector3 XMLHelper::fillVector3FromElement(TiXmlElement* elem) {
	Ogre::Real x = 0.0f, y = 0.0f, z = 0.0f;
	if (elem->Attribute("x")) {
		x = std::stof(elem->Attribute("x"));
	}
	if (elem->Attribute("y")) {
		y = std::stof(elem->Attribute("y"));
	}
	if (elem->Attribute("z")) {
		z = std::stof(elem->Attribute("z"));
	}

	return Ogre::Vector3(x, y, z);
}


void XMLHelper::fillElementFromVector3(TiXmlElement& elem, const Ogre::Vector3& vector) {
	elem.SetDoubleAttribute("x", vector.x);
	elem.SetDoubleAttribute("y", vector.y);
	elem.SetDoubleAttribute("z", vector.z);
}

Ogre::Quaternion XMLHelper::fillQuaternionFromElement(TiXmlElement* elem) {
	Ogre::Vector3 vector = fillVector3FromElement(elem);
	Ogre::Degree degrees;
	//first check if degrees is specified, but also allow for radians to be specified
	if (elem->Attribute("degrees")) {
		degrees = std::stof(elem->Attribute("degrees"));
	} else if (elem->Attribute("radians")) {
		degrees = Ogre::Radian(std::stof(elem->Attribute("radians")));
	}
	Ogre::Quaternion q;
	q.FromAngleAxis(degrees, vector);
	return q;

}

void XMLHelper::fillElementFromQuaternion(TiXmlElement& elem, const Ogre::Quaternion& quaternion) {
	//split the quaternion into an axis and a degree (our format allows us to store the angle element as a radian too, but I prefer degrees)
	Ogre::Degree degrees;
	Ogre::Vector3 axis;
	quaternion.ToAngleAxis(degrees, axis);
	fillElementFromVector3(elem, axis);
	elem.SetDoubleAttribute("degrees", degrees.valueDegrees());
}

}
}
