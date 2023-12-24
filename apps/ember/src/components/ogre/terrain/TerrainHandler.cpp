/*
 Copyright (C) 2010 erik

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

#include "TerrainHandler.h"

#include "TerrainDefPoint.h"
#include "TerrainShader.h"
#include "TerrainPage.h"
#include "TerrainPageSurface.h"
#include "TerrainPageGeometry.h"
#include "TerrainInfo.h"
#include "TerrainPageCreationTask.h"
#include "TerrainPageDeletionTask.h"
#include "TerrainShaderUpdateTask.h"
#include "TerrainAreaUpdateTask.h"
#include "TerrainModUpdateTask.h"
#include "TerrainUpdateTask.h"
#include "ITerrainAdapter.h"

#include "TerrainLayerDefinitionManager.h"
#include "TerrainLayerDefinition.h"

#include "GeometryUpdateTask.h"
#include "PlantQueryTask.h"
#include "HeightMap.h"
#include "HeightMapBufferProvider.h"
#include "PlantAreaQuery.h"
#include "SegmentManager.h"

#include "../Convert.h"
#include "../ILightning.h"

#include "framework/tasks/TaskQueue.h"
#include "framework/tasks/TaskExecutionContext.h"
#include "framework/TimedLog.h"

#include <Eris/EventService.h>

#include <Mercator/Segment.h>
#include <Mercator/Terrain.h>
#include <Mercator/AreaShader.h>

#include <sigc++/bind.h>

#include <memory>
#include <utility>


namespace Ember::OgreView::Terrain {

class BasePointRetrieveTask : public Tasks::TemplateNamedTask<BasePointRetrieveTask> {

private:
	Mercator::Terrain& mTerrain;
	sigc::slot<void(Mercator::Terrain::Pointstore&)> mAsyncCallback;
	Mercator::Terrain::Pointstore mPoints;

public:
	BasePointRetrieveTask(Mercator::Terrain& terrain, sigc::slot<void(Mercator::Terrain::Pointstore&)>& asyncCallback) :
			mTerrain(terrain), mAsyncCallback(asyncCallback) {
	}

	~BasePointRetrieveTask() override = default;

	void executeTaskInBackgroundThread(Tasks::TaskExecutionContext& context) override {
		mPoints = mTerrain.getPoints();
	}

	bool executeTaskInMainThread() override {
		mAsyncCallback(mPoints);
		return true;
	}

};

class TerrainPageReloadTask : public Tasks::TemplateNamedTask<TerrainPageReloadTask> {
private:
	TerrainHandler& mHandler;
	std::unique_ptr<TerrainPageGeometry> mGeometry;
	const std::vector<TerrainShader> mShaders;
	const WFMath::AxisBox<2> mArea;

public:
	TerrainPageReloadTask(TerrainHandler& handler,
						  std::unique_ptr<TerrainPageGeometry> geometry,
						  std::vector<TerrainShader> shaders,
						  const WFMath::AxisBox<2>& area) :
			mHandler(handler),
			mGeometry(std::move(geometry)),
			mShaders(std::move(shaders)),
			mArea(area) {
	}

	~TerrainPageReloadTask() override = default;

	void executeTaskInBackgroundThread(Tasks::TaskExecutionContext& context) override {
		mGeometry->repopulate();
		AreaStore areas;
		areas.push_back(mArea);
		//TODO: update terrain
//		if (mBridge) {
//			mBridge->updateTerrain(*mGeometry);
//		}
		std::vector<std::shared_ptr<TerrainPageGeometry>> geometries;
		geometries.emplace_back(std::move(mGeometry));
		context.executeTask(std::make_unique<TerrainShaderUpdateTask>(std::move(geometries),
																	  mShaders,
																	  areas,
																	  mHandler.EventLayerUpdated,
																	  mHandler.EventTerrainMaterialRecompiled));
	}

	bool executeTaskInMainThread() override {
		//TODO: update terrain
//		if (mBridge) {
//			mBridge->terrainPageReady();
//		}
		return true;
	}

};

TerrainHandler::TerrainHandler(ITerrainAdapter& terrainAdapter,
							   int pageIndexSize,
							   ICompilerTechniqueProvider& compilerTechniqueProvider,
							   Eris::EventService& eventService) :
		mTerrainAdapter(terrainAdapter),
		mPageIndexSize(pageIndexSize),
		mCompilerTechniqueProvider(compilerTechniqueProvider),
		mTerrainInfo(std::make_unique<TerrainInfo>(pageIndexSize)),
		mEventService(eventService),
		mTerrain(std::make_unique<Mercator::Terrain>(Mercator::Terrain::SHADED)),
		mSegmentManager(std::make_unique<SegmentManager>(*mTerrain, 64)),
		//The mercator buffers are one size larger than the resolution
		mHeightMapBufferProvider(std::make_unique<HeightMapBufferProvider>(mTerrain->getResolution() + 1)),
		mHeightMap(std::make_unique<HeightMap>(8.f, mTerrain->getResolution())),
		mTaskQueue(std::make_unique<Tasks::TaskQueue>(1, eventService)),
		mHeightMax(std::numeric_limits<Ogre::Real>::min()), mHeightMin(std::numeric_limits<Ogre::Real>::max()),
		mHasTerrainInfo(false),
		mTerrainEntity(nullptr),
		mAreaCounter(0),
		mModCounter(0) {
	mSegmentManager->setEndlessWorldEnabled(true);
	mSegmentManager->setDefaultHeight(getDefaultHeight());
	//TODO: get these values from the server if possible
	mSegmentManager->setDefaultHeightVariation(10);

	EventTerrainEnabled.connect(sigc::mem_fun(*this, &TerrainHandler::terrainEnabled));
	EventTerrainDisabled.connect(sigc::mem_fun(*this, &TerrainHandler::terrainDisabled));

}

TerrainHandler::~TerrainHandler() = default;

void TerrainHandler::shutdown() {
	mTaskQueue->deactivate();
}

void TerrainHandler::showTerrain(const WFMath::AxisBox<2>& interactingArea) {

	auto pageSizeInMeters = getPageMetersSize();

	auto xIndexMin = static_cast<int>(std::floor(interactingArea.lowCorner().x() / pageSizeInMeters));
	auto xIndexMax = static_cast<int>(std::floor(interactingArea.highCorner().x() / pageSizeInMeters));
	auto yIndexMin = -static_cast<int>(std::floor(interactingArea.highCorner().y() / pageSizeInMeters));
	auto yIndexMax = -static_cast<int>(std::floor(interactingArea.lowCorner().y() / pageSizeInMeters));


	std::set<TerrainIndex> visiblePages;

	assert(!(xIndexMax < xIndexMin));
	assert(!(yIndexMax < yIndexMin));

	for (auto xIndex = xIndexMin; xIndex <= xIndexMax; xIndex++) {
		for (auto yIndex = yIndexMin; yIndex <= yIndexMax; yIndex++) {
			TerrainIndex index{xIndex, yIndex};
			visiblePages.insert(index);
			auto I = mPages.find(index);
			if (I == mPages.end()) {
				setUpTerrainPageAtIndex(index);
			}
		}
	}

	//Don't unload pages as soon as they move out of range, since that might lead to a lot of churn when the camera is moved.
	auto center = interactingArea.getCenter();
	auto holdRadius = interactingArea.boundingSphereSloppy().radius() * 1.5;
	std::vector<TerrainIndex> pagesToDestroy;
	for (auto& entry: mPages) {
		if (visiblePages.count(entry.first) == 0) {
			auto distanceToPage = WFMath::Distance(entry.second->getWorldExtent().getCenter(), center) - (entry.second->getPageSize() * 0.5);
			if (distanceToPage > holdRadius) {
				pagesToDestroy.emplace_back(entry.first);
			}
		}
	}
//The Terrain component is unfortunately inherently non-thread safe, as it allows for both random deletion of Terrain instances while at the same time doing LOD calculations
//in a background thread. These calculations will access any Terrain isntances whenever they like, which will lead to spurious segfaults as the pages might be deleted.
//The solution for now is to never remove any pages while in the world, while we look for a better approach to rendering terrain.
//	for (auto& pageIndex: pagesToDestroy) {
//		destroyPage(pageIndex);
//	}
}


void TerrainHandler::setPageSize(int pageSize) {
	// Wait for all current tasks to finish
//	mTaskQueue->pollProcessedTasks(TimeFrame(boost::posix_time::seconds(60)));
	// Delete all page-related data
	mPageBridges.clear();
	mPages.clear();

	mTerrainInfo = std::make_unique<TerrainInfo>(pageSize + 1); //The number of vertices is always one more than the number of "units".
	mPageIndexSize = pageSize;
}

void TerrainHandler::getBasePoints(sigc::slot<void(Mercator::Terrain::Pointstore&)>& asyncCallback) {
	mTaskQueue->enqueueTask(std::make_unique<BasePointRetrieveTask>(*mTerrain, asyncCallback));
}

TerrainHandler::ShaderEntry* TerrainHandler::createShader(const TerrainLayerDefinition* layerDef, std::unique_ptr<Mercator::Shader> mercatorShader) {
	size_t index = mShaderMap.size();
	logger->debug("Creating new shader for shader {} with index {}", layerDef->mShaderName, index);

	ShaderEntry entry{
			TerrainLayer{*layerDef, (int) index},
			std::move(mercatorShader)
	};

	auto result = mShaderMap.emplace(index, std::move(entry));

	//TODO: should we instead do this in the task on the main thread? What are the consequences?
	if (result.second) {

		struct AddShaderTask : public Tasks::TemplateNamedTask<AddShaderTask> {

			TerrainHandler& mTerrainHandler;
			Mercator::Terrain& mTerrain;
			TerrainLayer mLayer;
			Mercator::Shader* mShader;

			AddShaderTask(TerrainHandler& terrainHandler, Mercator::Terrain& terrain, Mercator::Shader* shader, TerrainLayer layer) :
					mTerrainHandler(terrainHandler),
					mTerrain(terrain),
					mLayer(std::move(layer)),
					mShader(shader) {
			}

			void executeTaskInBackgroundThread(Tasks::TaskExecutionContext& context) override {
				mTerrain.addShader(mShader, mLayer.terrainIndex);
			}

			bool executeTaskInMainThread() override {
				mTerrainHandler.EventShaderCreated.emit(mLayer);
				return true;
			};

		};

		//mBaseShaders.emplace_back(&result.first->second.terrainShader);
		mTaskQueue->enqueueTask(std::make_unique<AddShaderTask>(*this, *mTerrain, result.first->second.shader.get(), result.first->second.layer));


		return &result.first->second;
	} else {
		return nullptr;
	}
}

void TerrainHandler::markShaderForUpdate(int layer, const WFMath::AxisBox<2>& affectedArea) {
	if (layer > 0) {
		auto I = mAreaShaders.find(layer);
		if (I == mAreaShaders.end()) {
			//Create new layer
			const TerrainLayerDefinition* layerDef = TerrainLayerDefinitionManager::getSingleton().getDefinitionForArea(layer);
			if (layerDef) {
				auto entry = createShader(layerDef, std::make_unique<Mercator::AreaShader>(layer));
				mAreaShaders[layer] = entry;
			}
		} else {
			ShaderUpdateRequest& updateRequest = mShadersToUpdate[I->second->layer.terrainIndex];
			updateRequest.Areas.push_back(affectedArea);
			mEventService.runOnMainThread([&]() {
				updateShaders();
			}, mActiveMarker);
		}
	}
}

const std::map<size_t, TerrainHandler::ShaderEntry>& TerrainHandler::getAllShaders() const {
	return mShaderMap;
}

void TerrainHandler::destroyPage(const TerrainIndex& index) {
	auto I = mPages.find(index);
	if (I != mPages.end()) {
		logger->debug("Destroying page at index [{},{}]", index.first, index.second);
		mTerrainAdapter.removePage(index);
		mPages.erase(I);
	}
}

void TerrainHandler::destroyPage(TerrainPage* page) {
	const auto& index = page->getWFIndex();
	auto I = mPages.find(index);
	if (I != mPages.end()) {
		mTerrainAdapter.removePage(index);
		mPages.erase(I);
	}
}

int TerrainHandler::getPageIndexSize() const {
	return mPageIndexSize;
}

int TerrainHandler::getPageMetersSize() const {
	return getPageIndexSize() - 1;
}

void TerrainHandler::getPlantsForArea(Foliage::PlantPopulator& populator, PlantAreaQuery& query, sigc::slot<void(const PlantAreaQueryResult&)> asyncCallback) {

	TerrainPosition wfPos(Convert::toWF(query.mCenter));

	TerrainIndex index(static_cast<int>(std::floor(query.mCenter.x / ((float) mPageIndexSize - 1.0f))),
					   static_cast<int>(-std::floor(query.mCenter.y / ((float) mPageIndexSize - 1.0f))));

	//If there's no terrain page created we shouldn't create any foliage at this moment.
	//Later on when the terrain page actually is shown, the TerrainManager::EventTerrainShown signal will be emitted
	//and the foliage reloaded.
	//The main reasons for this are twofold:
	//1) Calculating foliage occupies the task queue at the expense of creating terrain page data. We much rather would want the pages to be created before foliage is handled.
	//2) If foliage is shown before the page is shown it just looks strange, with foliage levitating in the empty air.

	auto pageI = mPages.find(index);
	if (pageI == mPages.end()) {
		return;
	}

	auto xIndex = static_cast<int>(std::floor(wfPos.x() / mTerrain->getResolution()));
	auto yIndex = static_cast<int>(std::floor(wfPos.y() / mTerrain->getResolution()));
	SegmentRefPtr segmentRef = mSegmentManager->getSegmentReference(xIndex, yIndex);
	if (segmentRef) {
		mTaskQueue->enqueueTask(std::make_unique<PlantQueryTask>(segmentRef, populator, query, std::move(asyncCallback)));
	}
}

ICompilerTechniqueProvider& TerrainHandler::getCompilerTechniqueProvider() {
	return mCompilerTechniqueProvider;
}

void TerrainHandler::updateShaders() {
	//update shaders that needs updating
	if (!mShadersToUpdate.empty()) {
		GeometryPtrVector geometry;
		for (const auto& page: mPages) {
			geometry.emplace_back(std::make_shared<TerrainPageGeometry>(page.second, *mSegmentManager, getDefaultHeight()));
		}
		//use a reverse iterator, since we need to update top most layers first, since lower layers might depend on them for their foliage positions
		for (auto& entry: mShadersToUpdate) {
			auto I = mShaderMap.find(entry.first);
			if (I != mShaderMap.end()) {
				std::vector<TerrainShader> shaders{TerrainShader{I->second.layer, *I->second.shader}};
				mTaskQueue->enqueueTask(std::make_unique<TerrainShaderUpdateTask>(geometry,
																				  std::move(shaders),
																				  entry.second.Areas,
																				  EventLayerUpdated,
																				  EventTerrainMaterialRecompiled)
				);
			}
		}
		mShadersToUpdate.clear();
	}
}

void TerrainHandler::updateAllPages() {
	GeometryPtrVector geometry;
	for (auto& page: mPages) {
		geometry.emplace_back(std::make_shared<TerrainPageGeometry>(page.second, *mSegmentManager, getDefaultHeight()));
	}

	//Update all pages
	AreaStore areas;
	areas.push_back(mTerrainInfo->getWorldSizeInIndices());

	//Update all shaders on all pages
	for (auto& entry: mShaderMap) {
		std::vector<TerrainShader> shaders{TerrainShader{entry.second.layer, *entry.second.shader}};
		mTaskQueue->enqueueTask(std::make_unique<TerrainShaderUpdateTask>(geometry,
																		  std::move(shaders),
																		  areas,
																		  EventLayerUpdated,
																		  EventTerrainMaterialRecompiled));
	}
}

void TerrainHandler::terrainEnabled(EmberEntity& entity) {
	mTerrainEntity = &entity;
}

void TerrainHandler::terrainDisabled() {
	mTerrainEntity = nullptr;
}

EmberEntity* TerrainHandler::getTerrainHoldingEntity() {
	return mTerrainEntity;
}

const TerrainInfo& TerrainHandler::getTerrainInfo() const {
	return *mTerrainInfo;
}

std::shared_ptr<Terrain::TerrainPage> TerrainHandler::getTerrainPageAtPosition(const TerrainPosition& worldPosition) const {

	int xRemainder = static_cast<int>(getMin().x()) % (getPageMetersSize());
	int yRemainder = static_cast<int>(getMin().y()) % (getPageMetersSize());

	int xIndex = static_cast<int>(floor((worldPosition.x() + xRemainder) / (getPageMetersSize())));
	int yIndex = static_cast<int>(floor((worldPosition.y() + yRemainder) / (getPageMetersSize())));

	return getTerrainPageAtIndex(TerrainIndex(xIndex, yIndex));
}

void TerrainHandler::setUpTerrainPageAtIndex(const TerrainIndex& index) {

	int x = index.first;
	int y = index.second;

	logger->info("Setting up TerrainPage at index [{},{}]", x, y);
	auto I = mPages.find(index);
	if (I == mPages.end()) {

		auto resultI = mPages.emplace(index, std::make_unique<TerrainPage>(index, getPageIndexSize(), getCompilerTechniqueProvider()));
		if (resultI.second) {
			auto page = resultI.first->second;

			logger->debug("Adding terrain page to TerrainHandler: [{}|{}]", index.first, index.second);

			mTaskQueue->enqueueTask(std::make_unique<TerrainPageCreationTask>(*this, page, *mHeightMapBufferProvider, *mHeightMap));
		} else {
			logger->warn("Could not insert terrain page at [{}|{}]", index.first, index.second);
		}
	}

}

std::shared_ptr<Terrain::TerrainPage> TerrainHandler::getTerrainPageAtIndex(const TerrainIndex& index) const {

	auto I = mPages.find(index);
	if (I != mPages.end()) {
		return I->second;
	}

	return nullptr;
}

bool TerrainHandler::getHeight(const TerrainPosition& point, float& height) const {
	WFMath::Vector<3> vector;

	return mHeightMap->getHeightAndNormal((float) point.x(), (float) point.y(), height, vector);
}

void TerrainHandler::blitHeights(int xMin, int xMax, int yMin, int yMax, std::vector<float>& heights) const {
	mHeightMap->blitHeights(xMin, xMax, yMin, yMax, heights);
}


float TerrainHandler::getDefaultHeight() const {
	return -12.f;
}

bool TerrainHandler::updateTerrain(const TerrainDefPointStore& terrainPoints) {
	mTaskQueue->enqueueTask(std::make_unique<TerrainUpdateTask>(*mTerrain, terrainPoints, *this, *mTerrainInfo, mHasTerrainInfo, *mSegmentManager));
	return true;
}

void TerrainHandler::reloadTerrain(const std::vector<TerrainPosition>& positions) {
	std::vector<WFMath::AxisBox<2>> areas;
	for (const auto& worldPosition: positions) {
		//Make an area which will cover the area affected by the basepoint
		int res = mTerrain->getResolution();
		areas.emplace_back(worldPosition - WFMath::Vector<2>(res, res), worldPosition + WFMath::Vector<2>(res, res));
	}
	reloadTerrain(areas);
}

void TerrainHandler::reloadTerrain(const std::vector<WFMath::AxisBox<2>>& areas) {
	if (mTaskQueue->isActive()) {
		std::set<std::shared_ptr<Terrain::TerrainPage>> pagesToUpdate;
		for (const auto& area: areas) {
			for (auto& entry: mPages) {
				auto page = entry.second;
				if (WFMath::Contains(page->getWorldExtent(), area, false) || WFMath::Intersect(page->getWorldExtent(), area, false) || WFMath::Contains(area, page->getWorldExtent(), false)) {
					pagesToUpdate.insert(page);
				}
			}
		}

		EventBeforeTerrainUpdate(areas, pagesToUpdate);
		//Spawn a separate task for each page to not bog down processing with all pages at once
		for (const auto& page: pagesToUpdate) {
			std::vector<TerrainPageGeometryPtr> geometryToUpdate;
			geometryToUpdate.emplace_back(std::make_shared<TerrainPageGeometry>(page, *mSegmentManager, getDefaultHeight()));
			std::vector<TerrainShader> shaders;
			shaders.reserve(mShaderMap.size());
			for (auto& entry: mShaderMap) {
				shaders.push_back(TerrainShader{entry.second.layer, *entry.second.shader});
			}
			mTaskQueue->enqueueTask(std::make_unique<GeometryUpdateTask>(geometryToUpdate,
																		 areas,
																		 *this,
																		 std::move(shaders),
																		 *mHeightMapBufferProvider,
																		 *mHeightMap));
		}
	}
}

TerrainPosition TerrainHandler::getMax() const {
	return mTerrainInfo->getWorldSizeInIndices().highCorner();
}

TerrainPosition TerrainHandler::getMin() const {
	return mTerrainInfo->getWorldSizeInIndices().lowCorner();
}

SegmentManager& TerrainHandler::getSegmentManager() {
	return *mSegmentManager;
}

void TerrainHandler::updateMod(const std::string& id,
							   WFMath::Point<3> pos,
							   WFMath::Quaternion orientation,
							   std::unique_ptr<Ember::Terrain::TerrainModTranslator> translator) {
	long modId = 0;
	auto I = mTerrainModsIndex.find(id);
	if (I == mTerrainModsIndex.end()) {
		//No existing mod, create one.
		if (translator) {
			modId = ++mModCounter;
			mTerrainModsIndex.emplace(id, modId);
		} else {
			return;
		}
	} else {
		modId = I->second;
	}


	if (pos.isValid() && orientation.isValid()) {
		mTaskQueue->enqueueTask(std::make_unique<TerrainModUpdateTask>(*mTerrain,
																	   std::move(translator),
																	   modId,
																	   pos,
																	   orientation,
																	   [this](const std::vector<WFMath::AxisBox<2>>& areas) {
																		   reloadTerrain(areas);
																	   }));
	}
}

const std::unordered_map<std::string, long>& TerrainHandler::getAreas() const {
	return mAreas;
}


void TerrainHandler::updateArea(const std::string& id, std::unique_ptr<Mercator::Area> terrainArea) {
	auto I = mAreas.find(id);
	if (I == mAreas.end()) {
		if (terrainArea && terrainArea->getLayer() != 0 && terrainArea->shape().isValid()) {
			//There's no existing area, we need to add a new one.
			auto areaId = mAreaCounter++;
			mAreas.emplace(id, areaId);
			mTaskQueue->enqueueTask(std::make_unique<TerrainAreaUpdateTask>(*mTerrain, areaId, std::move(terrainArea), sigc::mem_fun(*this, &TerrainHandler::markShaderForUpdate)));
		}
		//If there's no existing area, and no valid supplied one, just don't do anything.
	} else {
		auto existingAreaId = I->second;
		if (!terrainArea || terrainArea->getLayer() == 0 || !terrainArea->shape().isValid()) {
			//We should remove the area.
			mAreas.erase(I);
			mTaskQueue->enqueueTask(std::make_unique<TerrainAreaUpdateTask>(*mTerrain, existingAreaId, nullptr, sigc::mem_fun(*this, &TerrainHandler::markShaderForUpdate)));
		} else {
			auto existingArea = mTerrain->getArea(existingAreaId);
			//Check if we need to swap the area (if the layer has changed) or if we just can update the shape.
			if (existingArea && terrainArea->getLayer() != existingArea->getLayer()) {
				//Remove and add if we've switched layers.
				mTaskQueue->enqueueTask(std::make_unique<TerrainAreaUpdateTask>(*mTerrain, existingAreaId, nullptr, sigc::mem_fun(*this, &TerrainHandler::markShaderForUpdate)));
				mTaskQueue->enqueueTask(std::make_unique<TerrainAreaUpdateTask>(*mTerrain, existingAreaId, std::move(terrainArea), sigc::mem_fun(*this, &TerrainHandler::markShaderForUpdate)));
			} else {
				mTaskQueue->enqueueTask(std::make_unique<TerrainAreaUpdateTask>(*mTerrain, existingAreaId, std::move(terrainArea), sigc::mem_fun(*this, &TerrainHandler::markShaderForUpdate)));
			}
		}
	}
}

}




