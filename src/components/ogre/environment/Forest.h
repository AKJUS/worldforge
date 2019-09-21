//
// C++ Interface: Forest
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2008
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
#ifndef EMBEROGRE_ENVIRONMENTFOREST_H
#define EMBEROGRE_ENVIRONMENTFOREST_H

#include <OgreMath.h>
#include <OgreFrameListener.h>
#include <sigc++/trackable.h>
#include "components/ogre/OgreIncludes.h"

namespace Forests {
class PagedGeometry;
class TreeLoader3D;
}

namespace Ember {
class EmberEntity;
namespace OgreView {

namespace Terrain
{
class TerrainManager;
}
namespace Model {
class ModelRepresentation;
}


namespace Environment {

class EmberEntityLoader;

/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class Forest : public Ogre::FrameListener, public virtual sigc::trackable
{
public:
    Forest(Terrain::TerrainManager& terrainManager);

    virtual ~Forest();

    void initialize();

	void addTree(Ogre::Entity *entity, const Ogre::Vector3 &position, Ogre::Degree yaw = Ogre::Degree(0), Ogre::Real scale = 1.0f);

    void addEmberEntity(Model::ModelRepresentation* modelRepresentation);
    void removeEmberEntity(EmberEntity* entity);


	bool frameStarted(const Ogre::FrameEvent & evt);

protected:

	Terrain::TerrainManager& mTerrainManager;
	Forests::PagedGeometry *mTrees;
	Forests::TreeLoader3D *mTreeLoader;
	EmberEntityLoader* mEntityLoader;

	/**
	 * @brief The max range for entities to be rendered in the forest.
	 */
	float mMaxRange;

	/**
	 * @brief Called when the size of the world has changed.
	 */
	void worldSizeChanged();

};

}

}

}

#endif
