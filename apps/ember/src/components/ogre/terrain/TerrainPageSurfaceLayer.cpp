//
// C++ Implementation: TerrainPageSurfaceLayer
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
#include "TerrainPage.h"
#include "TerrainPageSurfaceLayer.h"
#include "TerrainPageSurface.h"
#include "TerrainLayerDefinition.h"
#include "TerrainPageGeometry.h"
#include "WFImage.h"
#include <Mercator/Surface.h>
#include <Mercator/Shader.h>


namespace Ember::OgreView::Terrain {

TerrainPageSurfaceLayer::TerrainPageSurfaceLayer(TerrainPageSurface& terrainPageSurface,
												 const TerrainLayerDefinition& definition,
												 int surfaceIndex,
												 const Mercator::Shader& shader) :
		mTerrainPageSurface(terrainPageSurface),
		mShader(shader),
		mSurfaceIndex(surfaceIndex),
		mScale(1.0f),
		mDefinition(definition) {

}

TerrainPageSurfaceLayer::~TerrainPageSurfaceLayer() = default;

bool TerrainPageSurfaceLayer::intersects(const TerrainPageGeometry& geometry) const {
	const SegmentVector validSegments = geometry.getValidSegments();
	//check if at least one surface intersects
	for (const auto& validSegment: validSegments) {
		if (mShader.checkIntersect(*validSegment.segment)) {
			return true;
		}
	}
	return false;
}

void TerrainPageSurfaceLayer::fillImage(const TerrainPageGeometry& geometry, Image& image, unsigned int channel) const {
	int segmentsPerAxis = geometry.getPage()->getNumberOfSegmentsPerAxis();
	SegmentVector validSegments = geometry.getValidSegments();
	for (const auto& validSegment: validSegments) {
		Mercator::Segment* segment = validSegment.segment;
		if (mShader.checkIntersect(*segment)) {
			Mercator::Surface* surface = getSurfaceForSegment(segment);
			if (surface && surface->isValid()) {
				WFImage sourceImage(std::make_unique<Image::ImageBuffer>(segment->getResolution(), 1));
				auto srcPtr = surface->getData();
				auto dataPtr = sourceImage.getData();
				auto segmentSize = segment->getSize();
				for (int i = 0; i < segment->getResolution(); ++i) {
					for (int j = 0; j < segment->getResolution(); ++j) {
						//interpolate four samples to get the fragment coverage
						*dataPtr = (unsigned char) (
								(srcPtr[(i * segmentSize) + j] + srcPtr[(i * segmentSize) + j + 1] + srcPtr[((i + 1) * segmentSize) + j] + srcPtr[((i + 1) * segmentSize) + j + 1]) / 4);
						dataPtr++;
					}
				}

				image.blit(sourceImage, channel, ((int) validSegment.index.x() * segment->getResolution()), ((segmentsPerAxis - (int) validSegment.index.y() - 1) * segment->getResolution()));
			}
		}
	}
}

float TerrainPageSurfaceLayer::getScale() const {
	return mScale;
}

void TerrainPageSurfaceLayer::setScale(float scale) {
	mScale = scale;
}


Mercator::Surface* TerrainPageSurfaceLayer::getSurfaceForSegment(const Mercator::Segment* segment) const {
	auto I = segment->getSurfaces().find(mSurfaceIndex);
	if (I == segment->getSurfaces().end()) {
		return nullptr;
	}
	return I->second.get();
}


const std::string& TerrainPageSurfaceLayer::getDiffuseTextureName() const {
	return mDiffuseTextureName;
}

void TerrainPageSurfaceLayer::setDiffuseTextureName(const std::string& textureName) {
	mDiffuseTextureName = textureName;
}

const std::string& TerrainPageSurfaceLayer::getSpecularTextureName() const {
	return mSpecularTextureName;
}

void TerrainPageSurfaceLayer::setSpecularTextureName(const std::string& textureName) {
	mSpecularTextureName = textureName;
}

const std::string& TerrainPageSurfaceLayer::getNormalTextureName() const {
	return mNormalTextureName;
}

void TerrainPageSurfaceLayer::setNormalTextureName(const std::string& textureName) {
	mNormalTextureName = textureName;
}

int TerrainPageSurfaceLayer::getSurfaceIndex() const {
	return mSurfaceIndex;
}

const TerrainLayerDefinition& TerrainPageSurfaceLayer::getDefinition() const {
	return mDefinition;
}

void TerrainPageSurfaceLayer::populate(const TerrainPageGeometry& geometry) {
	const SegmentVector validSegments = geometry.getValidSegments();
	for (const auto& validSegment: validSegments) {
#if 0
		//the current Mercator code works such that whenever an Area is added to Terrain, _all_ surfaces for the affected segments are invalidated, thus requiering a total repopulation of the segment
		//If however that code was changed to only invalidate the affected surface the code below would be very much handy
		Mercator::Surface* surface(getSurfaceForSegment(I->segment));
		if (surface) {
			surface->populate();
		}
#else

		Mercator::Segment* segment(validSegment.segment);
		if (!segment->isValid()) {
			segment->populate();
		}

		auto I2(segment->getSurfaces().find(mSurfaceIndex));
		if (I2 == segment->getSurfaces().end()) {
			//the segment doesn't contain this surface yet, lets add it
			if (mShader.checkIntersect(*segment)) {
				logger->debug("Adding new surface with id {} to segment at x: {} z: {}", mSurfaceIndex, segment->getXRef(), segment->getZRef());
				Mercator::Segment::Surfacestore& sss = segment->getSurfaces();
				sss[mSurfaceIndex] = mShader.newSurface(*segment);
			}
		}
		//NOTE: we have to repopulate all surfaces mainly to get the foliage to work.
		segment->populateSurfaces();
#endif
	}
}

}



