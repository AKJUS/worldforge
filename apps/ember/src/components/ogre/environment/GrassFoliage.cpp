//
// C++ Implementation: GrassFoliage
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
#include "GrassFoliage.h"

#include "FoliageLayer.h"

#include "../Scene.h"
#include "../Convert.h"
#include "../terrain/TerrainInfo.h"
#include "../terrain/TerrainManager.h"
#include "../terrain/TerrainLayerDefinition.h"

#include <utility>


namespace Ember::OgreView::Environment {

GrassFoliage::GrassFoliage(Terrain::TerrainManager& terrainManager,
						   Terrain::TerrainLayer terrainLayer,
						   Terrain::TerrainFoliageDefinition foliageDefinition)
		: FoliageBase(terrainManager, std::move(terrainLayer), std::move(foliageDefinition)),
		  mMinHeight(1.0f),
		  mMaxHeight(1.5f),
		  mMinWidth(1.0f),
		  mMaxWidth(1.5f) {
	if (mFoliageDefinition.hasParameter("minHeight")) {
		mMinHeight = std::stof(mFoliageDefinition.getParameter("minHeight"));
	}
	if (mFoliageDefinition.hasParameter("maxHeight")) {
		mMaxHeight = std::stof(mFoliageDefinition.getParameter("maxHeight"));
	}
	if (mFoliageDefinition.hasParameter("minWidth")) {
		mMinWidth = std::stof(mFoliageDefinition.getParameter("minWidth"));
	}
	if (mFoliageDefinition.hasParameter("maxWidth")) {
		mMaxWidth = std::stof(mFoliageDefinition.getParameter("maxWidth"));
	}
}

GrassFoliage::~GrassFoliage() = default;

void GrassFoliage::initialize() {
	mPagedGeometry = std::make_unique<::Forests::PagedGeometry>(&mTerrainManager.getScene().getMainCamera(), mTerrainManager.getFoliageBatchSize());
	const WFMath::AxisBox<2>& worldSize = mTerrainManager.getTerrainInfo().getWorldSizeInIndices();

	mPagedGeometry->setInfinite();

	mPagedGeometry->addDetailLevel<Forests::GrassPage>(96);
	//We'll provide our own shaders for the grass materials.
	mPagedGeometry->setShadersEnabled(false);

	//Create a GrassLoader object
	mGrassLoader = std::make_unique<::Forests::GrassLoader<FoliageLayer>>(mPagedGeometry.get());
	mPagedGeometry->setPageLoader(mGrassLoader.get());    //Assign the "treeLoader" to be used to load
	mGrassLoader->setHeightFunction(&getTerrainHeight, static_cast<IHeightProvider*>(&mTerrainManager));

	//Add some grass to the scene with GrassLoader::addLayer()
	FoliageLayer* l = mGrassLoader->addLayer(mFoliageDefinition.getParameter("material"));

	l->configure(&mTerrainManager, &mTerrainLayer, &mFoliageDefinition);
	//Configure the grass layer properties (size, density, animation properties, fade settings, etc.)
	l->setMinimumSize(mMinWidth, mMinHeight);
	l->setMaximumSize(mMaxWidth, mMaxHeight);
	l->setAnimationEnabled(true);        //Enable animations
	if (mFoliageDefinition.hasParameter("swayDistribution")) {
		l->setSwayDistribution(std::stof(mFoliageDefinition.getParameter("swayDistribution")));
	} else {
		l->setSwayDistribution(10.0f);        //Sway fairly unsynchronized
	}
	if (mFoliageDefinition.hasParameter("swayLength")) {
		l->setSwayLength(std::stof(mFoliageDefinition.getParameter("swayLength")));
	} else {
		l->setSwayLength(0.5f);                //Sway back and forth 0.5 units in length
	}

	if (mFoliageDefinition.hasParameter("swaySpeed")) {
		l->setSwaySpeed(std::stof(mFoliageDefinition.getParameter("swaySpeed")));
	} else {
		l->setSwaySpeed(0.5f);                //Sway 1/2 a cycle every second
	}

	if (mFoliageDefinition.hasParameter("fadeTech")) {
		const std::string& fadeTech(mFoliageDefinition.getParameter("fadeTech"));
		if (fadeTech == "alphagrow") {
			l->setFadeTechnique(::Forests::FADETECH_ALPHAGROW);    //Distant grass should slowly fade in
		} else if (fadeTech == "grow") {
			l->setFadeTechnique(::Forests::FADETECH_GROW);    //Distant grass should slowly fade in
		} else {
			l->setFadeTechnique(::Forests::FADETECH_ALPHA);    //Distant grass should slowly fade in
		}
	} else {
		l->setFadeTechnique(::Forests::FADETECH_ALPHA);    //Distant grass should slowly fade in
	}
// 	l->setDensity(1.5f);				//Relatively dense grass
	if (mFoliageDefinition.hasParameter("renderTech")) {
		const std::string& renderTech(mFoliageDefinition.getParameter("renderTech"));
		if (renderTech == "quad") {
			l->setRenderTechnique(::Forests::GRASSTECH_QUAD);
		} else if (renderTech == "sprite") {
			l->setRenderTechnique(::Forests::GRASSTECH_SPRITE);
		} else {
			l->setRenderTechnique(::Forests::GRASSTECH_CROSSQUADS);    //Draw grass as scattered quads
		}
	} else {
		l->setRenderTechnique(::Forests::GRASSTECH_CROSSQUADS);    //Draw grass as scattered quads
	}

	l->setMapBounds(Convert::toOgre(worldSize));
	l->setMaxSlope(Ogre::Degree(40.0f));

	auto& detailLevels = mPagedGeometry->getDetailLevels();
	for (auto& detailLevel: detailLevels) {
		DistanceStore tempDistance = {detailLevel->getFarRange(), detailLevel->getNearRange(), detailLevel->getTransition()};
		mDistanceStore.push_back(tempDistance);
	}

}

void GrassFoliage::frameStarted() {
	if (mPagedGeometry) {
		mPagedGeometry->update();
	}
}

void GrassFoliage::setDensity(float newGrassDensity) {
	mGrassLoader->setDensityFactor(newGrassDensity);
	mPagedGeometry->reloadGeometry();
}

void GrassFoliage::setFarDistance(float factor) {
	std::list<Forests::GeometryPageManager*> detailLevels = mPagedGeometry->getDetailLevels();

	auto J = mDistanceStore.begin();
	for (auto I = detailLevels.begin(); I != detailLevels.end() && J != mDistanceStore.end(); ++I) {
		(*I)->setFarRange(factor * J->farDistance);
		(*I)->setNearRange(factor * J->nearDistance);
		(*I)->setTransition(factor * J->transition);
		++J;
	}
	mPagedGeometry->reloadGeometry();
}

}



