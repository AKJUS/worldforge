/*
 Copyright (C) 2013 Samuel Kogler

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

#ifndef OGRETERRAINADAPTER_H_
#define OGRETERRAINADAPTER_H_

#include "../ITerrainAdapter.h"
#include "../../FrameListener.h"

#include <Eris/ActiveMarker.h>

#include <OgreTerrainMaterialGenerator.h>

#include <sigc++/signal.h>
#include <memory>
#include <boost/noncopyable.hpp>

namespace Ogre {
class TerrainGlobalOptions;

class TerrainGroup;
}

namespace Ember {
class EmberEntity;

namespace OgreView::Terrain {

class OgreTerrainMaterialGeneratorEmber;


/**
 * @brief A ITerrainAdapter implementation which connects to and sets up the Ogre Terrain component.
 */
class OgreTerrainAdapter : public ITerrainAdapter, public boost::noncopyable, public FrameListener {
public:
	OgreTerrainAdapter(Ogre::SceneManager& sceneManager, int terrainPageSize);

	~OgreTerrainAdapter() override;

	int getPageSize() override;

	void setPageSize(int pageSize) override;

	void setLoadRadius(Ogre::Real loadRadius) override;

	bool getHeightAt(Ogre::Real x, Ogre::Real z, float& height) override;

	void setCamera(Ogre::Camera* camera) override;

	void loadScene() override;

	void reset() override;

	void reloadAllPages() override;

	void reloadPage(const TerrainIndex& index) override;

	void showPage(std::shared_ptr<TerrainPageGeometry> geometry) override;

	bool isPageShown(const TerrainIndex& index) override;

	void removePage(const TerrainIndex& index) override;

	void reloadPageMaterial(const TerrainIndex& index) override;

	std::string getDebugInfo() override;

	/**
	 * @brief Currently setPageDataProvider MUST be called before this. Otherwise a nullptr will be returned.
	 */
	std::unique_ptr<ITerrainObserver> createObserver() override;

	std::pair<EmberEntity*, Ogre::Vector3> rayIntersects(const Ogre::Ray& ray) const override;

	void setPageDataProvider(std::unique_ptr<IPageDataProvider> pageDataProvider) override;

	sigc::connection bindTerrainShown(sigc::slot<void(const Ogre::TRect<Ogre::Real>)>& signal) override;

	void setTerrainEntity(EmberEntity* entity) override;

	bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;

private:

	Ogre::Real mLoadRadius;
	Ogre::Real mHoldRadius;

	Ogre::Camera* mCamera;

	/**
	 * @brief Signal emitted when a page has been shown for the first time.
	 * The argument is the area (in world coordinates) that was shown.
	 */
	sigc::signal<void(const Ogre::TRect<Ogre::Real>&)> mTerrainShownSignal;

	/**
	 * @brief Signal emitted when an area of the terrain has been updated.
	 * The argument is the area (in world coordinates) that was updated.
	 */
	sigc::signal<void(const Ogre::TRect<Ogre::Real>&)> mTerrainAreaUpdated;

	std::unique_ptr<Ogre::TerrainGlobalOptions> mTerrainGlobalOptions;
	std::unique_ptr<Ogre::TerrainGroup> mTerrainGroup;
	Ogre::TerrainMaterialGeneratorPtr mMaterialGenerator;


	std::unique_ptr<IPageDataProvider> mPageDataProvider;
	EmberEntity* mEntity;

	Eris::ActiveMarker mActiveMarker;


	void setOgrePageSize(int pageSize);

	/**
	 * Will try to update the terrain page, unless it's being worked on in a background thread.
	 * If that's the case it will schedule a future attempt.
	 * @param terrainPtr
	 * @param index
	 * @param heightData
	 */
	void updateExistingTerrain(const Ogre::Terrain* terrainPtr,
							   const TerrainIndex& index,
							   std::vector<float> heightData);

	void updateLods();

};

} // namespace OgreView::Terrain

} /* namespace Ember */
#endif /* OGRETERRAINADAPTER_H_ */
