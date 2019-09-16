//
// C++ Interface: TerrainLayerDefinitionManager
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
#ifndef EMBEROGRE_TERRAINTERRAINLAYERDEFINITIONMANAGER_H
#define EMBEROGRE_TERRAINTERRAINLAYERDEFINITIONMANAGER_H

#include "components/ogre/EmberOgrePrerequisites.h"

#include "framework/Singleton.h"
#include <OgreResourceManager.h>

namespace Ember {
namespace OgreView {

namespace Terrain {

class TerrainLayerDefinition;
/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class TerrainLayerDefinitionManager : public Ogre::ResourceManager, public Singleton<TerrainLayerDefinitionManager>
{
public:
	typedef std::vector<std::unique_ptr<TerrainLayerDefinition>> DefinitionStore;
    TerrainLayerDefinitionManager();

	~TerrainLayerDefinitionManager() override;

	void parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &groupName) override;

    /**
    Adds a definition to the manager. This definition will be deleted by the manager upon destruction.
    @param definition A valid definition.
    */
    void addDefinition(TerrainLayerDefinition* definition);
    
    const DefinitionStore& getDefinitions() const;
    
    TerrainLayerDefinition* getDefinitionForArea(unsigned int areaIndex);
    TerrainLayerDefinition* getDefinitionForShader(const std::string& shaderType);

	/**
	 * Call this right after all resources have been setup.
	 * It will go through all textures and make sure that they refer to either .dds or .png versions, whatever is available.
	 */
	void resolveTextureReferences();
    
protected:
	DefinitionStore mDefinitions;
	
	Ogre::Resource* createImpl(const Ogre::String& name, Ogre::ResourceHandle handle, 
    	const Ogre::String& group, bool isManual, Ogre::ManualResourceLoader* loader,
    	const Ogre::NameValuePairList* createParams) override;
};

}

}

}

#endif
