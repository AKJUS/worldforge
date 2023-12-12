/*
 Copyright (C) 2014 Erik Ogenvik

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

#ifndef ONEPIXELMATERIALGENERATOR_H_
#define ONEPIXELMATERIALGENERATOR_H_

#include <OgreResource.h>


namespace Ember::OgreView::Terrain::Techniques {

/**
 * @brief Generates a one pixel texture used as normal map placeholder.
 *
 */
class OnePixelMaterialGenerator : public Ogre::ManualResourceLoader {
public:

	void loadResource(Ogre::Resource* resource) override;
};

}


#endif /* ONEPIXELMATERIALGENERATOR_H_ */
