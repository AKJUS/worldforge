//
// C++ Interface: TerrainPageSurfaceCompiler
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
#ifndef EMBEROGRETERRAINPAGESURFACECOMPILER_H
#define EMBEROGRETERRAINPAGESURFACECOMPILER_H

#include "../EmberOgrePrerequisites.h"
#include "Types.h"

#include <memory>
#include <set>


namespace Ember::OgreView::Terrain {

class TerrainPageSurfaceLayer;

class TerrainPage;

class TerrainPageGeometry;

struct ICompilerTechniqueProvider;

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief A technique for rendering the terrain.
 *
 * Since there are many different ways to perform terrain rendering (depending on hardware, features etc.) we
 * provide multiple implementations of this interface, and let the terrain surface compiler choose which one to use.
 * An implementation of this interface should be able to generate a complete Ogre::Material for any terrain geometry.
 *
 * Surface generation happens in two steps, where the first preparation step occurs in a background thread, and
 * the final compilation step occurs in the main thread.
 */
struct TerrainPageSurfaceCompilerTechnique {

	/**
	 * @brief Dtor.
	 */
	virtual ~TerrainPageSurfaceCompilerTechnique() = default;

	/**
	 * @brief Prepares the material.
	 *
	 * This is called in a background thread, so any implementation must be sure to perform this in a thread safe way.
	 * In most instances that means not touching Ogre (and if it needs to be touched, be very careful when doing so).
	 * @return False is something went wrong during preparation. The surface generation process will then be halted,
	 * and compileMaterial() not called.
	 */
	virtual bool prepareMaterial() = 0;

	/**
	 * @brief Compiles a previously prepared material.
	 *
	 * This is called in the main thread, and thus has full access to the Ogre system. It's expected that all heavy lifting
	 * has occurred in prepareMaterial, and any code here merely handled the setup of the Ogre structures needed.
	 * @param material The material which will be used for the terrain geometry.
	 * @param managedTextures A set of textures created in the process. These will be destroyed when the page is destroyed.
	 * @return False if something went wrong during compilation.
	 */
	virtual bool compileMaterial(Ogre::MaterialPtr material, std::set<std::string>& managedTextures) const = 0;

	/**
	 * @brief Compiles a previously prepared material for the terrain composite map. May not be implemented.
	 *
	 * This is called in the main thread, and thus has full access to the Ogre system. It's expected that all heavy lifting
	 * has occurred in prepareMaterial, and any code here merely handled the setup of the Ogre structures needed.
	 * @param material The material which will be used for rendering of the terrain composite map.
     * @param managedTextures A set of textures created in the process. These will be destroyed when the page is destroyed.
	 * @return False if something went wrong during compilation or if the technique does not support generating composite maps.
	 */
	virtual bool compileCompositeMapMaterial(Ogre::MaterialPtr material, std::set<std::string>& managedTextures) const = 0;

};

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 *
 * @brief Represents one unit of work for generating a terrain surface.
 *
 * An instance of this is created by the TerrainPageSurfaceCompiler.
 */
class TerrainPageSurfaceCompilationInstance {
public:

	/**
	 * @brief Ctor.
	 * @param technique The technique to use. Ownership is passed to this instance.
     * @param managedTextures A set of textures created in the process. These will be destroyed when the page is destroyed.
	 */
	TerrainPageSurfaceCompilationInstance(std::unique_ptr<TerrainPageSurfaceCompilerTechnique> technique, std::set<std::string>& managedTextures);

	/**
	 * @brief Dtor.
	 */
	virtual ~TerrainPageSurfaceCompilationInstance();

	/**
	 * @brief Prepares the surface. This is called in a background thread.
	 * @return False is something went wrong during preparation.
	 * 	The surface generation process will then be halted, and compileMaterial() not called.
	 */
	bool prepare();

	/**
	 * @brief Compiles the surface. This is called in the main thread.
     * @param material The material which will be used for the terrain geometry.
     * @return False if something went wrong during compilation.
	 */
	bool compile(const Ogre::MaterialPtr& material);

	/**
	 * @brief Compiles the surface composite map. This is called in the main thread.
	 * @param material The material which will be used for the terrain geometry.
	 * @return False if something went wrong during compilation or composite maps are not supported by the used technique.
	 */
	bool compileCompositeMap(const Ogre::MaterialPtr& material);

private:

	/**
	 * @brief The technique to use. Owned by this instance.
	 */
	std::unique_ptr<TerrainPageSurfaceCompilerTechnique> mTechnique;

	/**
     * A set of textures created in the process. These will be destroyed when the page is destroyed.
	 */
	std::set<std::string>& mManagedTextures;
};

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 *
 * @brief Compiles Ogre::Materials for terrain geometries, to be used in the Ogre terrain manager.
 *
 * The actual preparation and compilation occurs in instances of TerrainPageSurfaceCompilationInstance
 * and TerrainPageSurfaceCompilerTechnique which are created by the compiler.
 */
class TerrainPageSurfaceCompiler {
public:

	/**
	 * @brief Ctor.
	 * @param compilerTechniqueProvider Provider for terrain surface compilation techniques.
	 */
	explicit TerrainPageSurfaceCompiler(ICompilerTechniqueProvider& compilerTechniqueProvider);

	/**
	 * @brief Dtor.
	 */
	virtual ~TerrainPageSurfaceCompiler();

	/**
	 * @brief Creates a new compilation instance.
	 * @param geometry The geometry to operate on.
	 * @param terrainPageSurfaces The surfaces to generate a rendering technique for.
	 * @param terrainPageShadow An optional shadow.
	 * @return A compilation instance.
	 */
	std::unique_ptr<TerrainPageSurfaceCompilationInstance> createCompilationInstance(const TerrainPageGeometryPtr& geometry,
																					 const SurfaceLayerStore& terrainPageSurfaces);

private:

	/**
	 * @brief The compiler technique provider, which creates new techniques.
	 */
	ICompilerTechniqueProvider& mCompilerTechniqueProvider;

	/**
	 * A set of textures created in the process. These will be destroyed when this instance is destroyed.
	 */
	std::set<std::string> mManagedTextures;

};

}


#endif
