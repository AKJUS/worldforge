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

#include "SegmentHolder.h"
#include "Segment.h"
#include "SegmentManager.h"

#include <cassert>
#include <Mercator/Segment.h>


namespace Ember::OgreView::Terrain {

SegmentHolder::SegmentHolder(std::unique_ptr<Segment> segment, SegmentManager& segmentManager) :
		mSegment(std::move(segment)),
		mSegmentManager(segmentManager),
		mRefCount(0) {

}

SegmentHolder::~SegmentHolder() = default;

std::shared_ptr<Segment> SegmentHolder::getReference() {
	mRefCount++;
	//If mRefCount is 1 we're guaranteed to be the only one interacting with the segment, so it's thread safe to call Mercator::Segment::isValid
	if (mRefCount == 1 && mSegment->getMercatorSegment().isValid()) {
		mSegmentManager.unmarkHolder(this);
	}

	//When the shared pointer is deleted we should just decrease our internal reference counter.
	auto deleter = [&](Segment* ptr) {
		returnReference();
	};
	return {mSegment.get(), deleter};
}

void SegmentHolder::returnReference() {
	assert(mRefCount > 0);
	mRefCount--;
	//If mRefCount is 0 we're guaranteed to be the only one interacting with the segment, so it's thread safe to call Mercator::Segment::isValid
	if (mRefCount == 0 && mSegment->getMercatorSegment().isValid()) {
		mSegmentManager.markHolderAsDirtyAndUnused(this);
		mSegmentManager.pruneUnusedSegments();
	}
}

bool SegmentHolder::isUnused() {
	return mRefCount == 0;
}

Segment& SegmentHolder::getSegment() {
	return *mSegment;
}

}



