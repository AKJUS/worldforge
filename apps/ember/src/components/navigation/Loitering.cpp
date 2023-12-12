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

#include "Loitering.h"
#include "Awareness.h"

#include <Eris/Avatar.h>
#include <Eris/Entity.h>

#include <wfmath/rotbox.h>
#include <wfmath/intersect.h>


namespace Ember::Navigation {

Loitering::Loitering(Awareness& awareness, Eris::Avatar& avatar, const WFMath::Vector<2>& areaSize) :
		mAwareness(awareness), mAvatar(avatar) {
	mMovedConnection = avatar.getEntity()->Moving.connect(sigc::mem_fun(*this, &Loitering::entity_Moving));

	auto entityViewPosition = mAvatar.getEntity()->getPredictedPos();

	WFMath::Point<2> entityPosition2d(entityViewPosition.x(), entityViewPosition.z());

	WFMath::RotBox<2> area;
	area.size() = areaSize;
	area.corner0() = entityPosition2d - (areaSize / 2);
	area.orientation() = WFMath::RotMatrix<2>().identity();

	mLoiterArea = area.boundingBox();
	mAwareness.setAwarenessArea(area, WFMath::Segment<2>());

}

Loitering::~Loitering() {
	mMovedConnection.disconnect();
}

void Loitering::entity_Moving(bool isMoving) {

	auto entityViewPosition = mAvatar.getEntity()->getPredictedPos();

	WFMath::Point<2> entityPosition2d(entityViewPosition.x(), entityViewPosition.z());
	if (!WFMath::Contains(mLoiterArea, entityPosition2d, false)) {
		//The entity is outside of the loiter area; stop loitering
		mAwareness.setAwarenessArea(WFMath::RotBox<2>(), WFMath::Segment<2>());
		mMovedConnection.disconnect();
	}
}

}

