//
// C++ Interface: TerrainPageShadow
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
#ifndef EMBEROGRETERRAINPAGESHADOW_H
#define EMBEROGRETERRAINPAGESHADOW_H

#include "../EmberOgrePrerequisites.h"

#include <memory>
#include <wfmath/vector.h>
#include <OgreMath.h>
#include "components/ogre/OgreIncludes.h"


namespace Ember {
namespace OgreView {
namespace Terrain {

class TerrainPage;

class TerrainPageGeometry;

class OgreImage;

/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class TerrainPageShadow {
public:
	explicit TerrainPageShadow(const TerrainPage& terrainPage);

	virtual ~TerrainPageShadow();

	void setLightDirection(const WFMath::Vector<3>& lightDirection);

	void updateShadow(const TerrainPageGeometry& geometry);

	void loadIntoImage(Ogre::Image& ogreImage) const;

	/**
	 * @brief Sets an optional shadow texture name.
	 *
	 * This refers to a precomputed shadow texture in Ogre. Note that this only is of use when using
	 * the fixed function pipeline which requires precomputed shadows.
	 *
	 * @param shadowTextureName The name of the shadow texture.
	 */
	void setShadowTextureName(const std::string& shadowTextureName);

	/**
	 * @brief Gets an optional shadow texture name.
	 *
	 * This refers to a precomputed shadow texture in Ogre. Note that this only is of use when using
	 * the fixed function pipeline which requires precomputed shadows.
	 *
	 * @return A name of a texture in Ogre, or an empty string if there's no shadow texture.
	 */
	const std::string& getShadowTextureName() const;


protected:
	const TerrainPage& mTerrainPage;
	WFMath::Vector<3> mLightDirection;

	std::unique_ptr<OgreImage> mImage;

	/**
	 * @brief An optional shadow texture name.
	 *
	 * This refers to a precomputed shadow texture in Ogre. Note that this only is of use when using
	 * the fixed function pipeline which requires precomputed shadows.
	 */
	std::string mShadowTextureName;

};

}
}

}

#endif
