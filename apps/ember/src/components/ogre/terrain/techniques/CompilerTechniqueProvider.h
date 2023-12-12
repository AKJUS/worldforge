/*
 Copyright (C) 2010 Erik Ogenvik <erik@ogenvik.org>

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

#ifndef COMPILERTECHNIQUEPROVIDER_H_
#define COMPILERTECHNIQUEPROVIDER_H_

#include "../ICompilerTechniqueProvider.h"
#include "components/ogre/OgreIncludes.h"
#include "components/ogre/terrain/Types.h"


namespace Ember::OgreView {
class ShaderManager;


namespace Terrain::Techniques {

class OnePixelMaterialGenerator;

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief A compiler technique provider which uses the base techniques found in the OgreView::Terrain::Techniques namespace.
 */
class CompilerTechniqueProvider : public ICompilerTechniqueProvider {
public:
	/**
	 * @brief Ctor.
	 * @param shaderManager The shader manager which handles shader settings.
	 * @param sceneManager The scene manager which handles the terrain.
	 */
	CompilerTechniqueProvider(ShaderManager& shaderManager, Ogre::SceneManager& sceneManager);

	~CompilerTechniqueProvider() override;

	std::unique_ptr<TerrainPageSurfaceCompilerTechnique> createTechnique(const TerrainPageGeometryPtr& geometry,
																		 const SurfaceLayerStore& terrainPageSurfaces) const override;

protected:

	/**
	 * @brief The shader manager which handles the shader settings.
	 * Used to see how complex shaders should be used.
	 */
	ShaderManager& mShaderManager;

	/**
	 * @brief The scene manager.
	 */
	Ogre::SceneManager& mSceneManager;

	/**
	 * @brief Handles generation of the one pixel texture used as dummy normal map in the shader technique.
	 */
	std::unique_ptr<OnePixelMaterialGenerator> mOnePixelMaterialGenerator;
};

}

}


#endif /* COMPILERTECHNIQUEPROVIDER_H_ */
