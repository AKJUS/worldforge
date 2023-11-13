/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

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


#ifndef EMBEROGRETERRAINTECHNIQUESSHADERPASS_H_
#define EMBEROGRETERRAINTECHNIQUESSHADERPASS_H_

#include "components/ogre/OgreIncludes.h"
#include "domain/Types.h"
#include <wfmath/point.h>
#include <vector>
#include <string>
#include <set>


namespace Ember::OgreView::Terrain {
class TerrainPage;

class TerrainPageSurfaceLayer;

class TerrainPageGeometry;

namespace Techniques {

class ShaderPassBlendMapBatch;

typedef std::vector<const TerrainPageSurfaceLayer*> LayerStore;

class ShaderPass {
public:

	struct SplattingFragmentConfig {
		bool lightning;
		bool shadows;
		bool offsetMapping;
		bool fog;
		int layers;
	};

	friend class ShaderPassBlendMapBatch;

	ShaderPass(Ogre::SceneManager& sceneManager,
			   int blendMapPixelWidth,
			   TerrainIndex position,
			   bool useNormalMapping = false);

	~ShaderPass();

	void addLayer(const TerrainPageGeometry& geometry, const TerrainPageSurfaceLayer* layer);

	void addShadowLayer();

	bool hasRoomForLayer(const TerrainPageSurfaceLayer* layer);

	/**
	 * @brief Creates the combined final blend maps and sets the shader params. Be sure to call this before you load the material.
     * @param managedTextures A set of textures created in the process. These will be destroyed when the page is destroyed.
	 * @param useShadows Whether to use shadows or not in the pass.
	 * @param shaderSuffix A suffix to add to the shader name. This allows you to make it use a somewhat different shader depending on graphics level etc.
	 * @return True if the creation of the pass was successful.
	 */
	bool finalize(Ogre::Pass& pass, std::set<std::string>& managedTextures, bool useShadows, bool useLighting) const;

	static Ogre::GpuProgramPtr fetchOrCreateSplattingFragmentProgram(SplattingFragmentConfig config);

private:

	ShaderPassBlendMapBatch* getCurrentBatch();

	std::unique_ptr<ShaderPassBlendMapBatch> createNewBatch();

	unsigned int getBlendMapPixelWidth() const;

	Ogre::TexturePtr getCombinedBlendMapTexture(size_t passIndex, size_t batchIndex, std::set<std::string>& managedTextures) const;

	std::array<float, 16> mScales;
	std::vector<std::unique_ptr<ShaderPassBlendMapBatch>> mBlendMapBatches;
	LayerStore mLayers;
	Ogre::SceneManager& mSceneManager;
	int mBlendMapPixelWidth;
	TerrainIndex mPosition;

	int mShadowLayers;

	bool mUseNormalMapping;
};

//Since we support using both the raw media repository as well as the processed media we need to make sure we
//can load textures independent of whether they are .png or .dds.
Ogre::TexturePtr resolveTexture(const std::string& textureName);
}

}


#endif /* EMBEROGRETERRAINTECHNIQUESSHADERPASS_H_ */
