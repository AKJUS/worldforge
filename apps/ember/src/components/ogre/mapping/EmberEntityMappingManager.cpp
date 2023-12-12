//
// C++ Implementation: EmberEntityMappingManager
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
#include "EmberEntityMappingManager.h"

#include "components/ogre/XMLHelper.h"

using namespace Ember::EntityMapping;


namespace Ember::OgreView::Mapping {

EmberEntityMappingManager::EmberEntityMappingManager()
		: mEntityMappingManager(), mXmlSerializer(mEntityMappingManager) {

	Ogre::ResourceGroupManager::getSingleton()._registerScriptLoader(this);

}

EmberEntityMappingManager::~EmberEntityMappingManager() {
	Ogre::ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
}


const Ogre::StringVector& EmberEntityMappingManager::getScriptPatterns() const {
	static Ogre::StringVector patterns{"*.entitymap", "*.entitymap.xml"};
	return patterns;
}

Ogre::Real EmberEntityMappingManager::getLoadingOrder() const {
	return 300;
}


void EmberEntityMappingManager::parseScript(Ogre::DataStreamPtr& stream, const Ogre::String& groupName) {
	TiXmlDocument xmlDoc;
	XMLHelper xmlHelper;
	if (!xmlHelper.Load(xmlDoc, stream)) {
		return;
	}

	mXmlSerializer.parseScript(xmlDoc, stream->getName());
}


}



