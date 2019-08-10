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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Steering.h"
#include "Awareness.h"
#include "Loitering.h"

#include <Eris/Avatar.h>
#include <Eris/Entity.h>

#include <wfmath/rotbox.h>
#include <wfmath/segment.h>
#include <Atlas/Message/Element.h>

namespace Ember
{
namespace Navigation
{

Steering::Steering(Awareness& awareness, Eris::Avatar& avatar) :
		mAwareness(awareness),
		mAvatar(avatar),
		mSteeringEnabled(false),
		mUpdateNeeded(false),
		mPadding(16),
		mSpeed(5),
		mExpectingServerMovement(false),
		mLoitering(nullptr)
{
	mAwareness.EventTileUpdated.connect(sigc::mem_fun(*this, &Steering::Awareness_TileUpdated));

	if (avatar.getEntity()->hasProperty("speed-ground")) {
		auto speedElement = avatar.getEntity()->valueOfProperty("speed-ground");
		if (speedElement.isNum()) {
			mSpeed = speedElement.asNum();
		}
	}
}

Steering::~Steering()
{
	delete mLoitering;
}

void Steering::setDestination(const WFMath::Point<3>& viewPosition)
{
	mViewDestination = viewPosition;
	mUpdateNeeded = true;

	setAwareness();
}

void Steering::setAwareness()
{
	//If we are loitering we should stop that now.
	delete mLoitering;
	mLoitering = nullptr;

	const auto entityViewPosition = mAvatar.getEntity()->getViewPosition();

	WFMath::Point<2> destination2d(mViewDestination.x(), mViewDestination.z());
	WFMath::Point<2> entityPosition2d(entityViewPosition.x(), entityViewPosition.z());

	WFMath::Vector<2> direction(destination2d - entityPosition2d);
	auto theta = std::atan2(direction.y(), direction.x()); // rotation about Y
	WFMath::RotMatrix<2> rm;
	rm.rotation(theta);

	WFMath::Point<2> start = entityPosition2d;
	start -= WFMath::Vector<2>(mPadding, mPadding);

	WFMath::Vector<2> size(direction.mag() + (mPadding * 2), mPadding * 2);

	WFMath::RotBox<2> area;
	area.size() = size;
	area.corner0() = start;
	area.orientation() = WFMath::RotMatrix<2>().identity();
	area.rotatePoint(rm, entityPosition2d);

	mAwareness.setAwarenessArea(area, WFMath::Segment<2>(entityPosition2d, destination2d));

}

bool Steering::updatePath()
{
	mPath.clear();

	int result = mAwareness.findPath(mAvatar.getEntity()->getViewPosition(), mViewDestination, mPath);
	EventPathUpdated();
	mUpdateNeeded = false;
	return result > 0;
}

void Steering::requestUpdate()
{
	mUpdateNeeded = true;
}

void Steering::startSteering()
{
	mSteeringEnabled = true;
	mExpectingServerMovement = false;
}

void Steering::stopSteering()
{
	if (!mSteeringEnabled) {
		return;
	}
	mSteeringEnabled = false;
	mExpectingServerMovement = false;
	mLastSentVelocity = WFMath::Vector<2>();

	//When we stopped steering we'll retain an awareness around the avatar. We'll do this by "loitering".
	delete mLoitering;
	mLoitering = new Loitering(mAwareness, mAvatar, WFMath::Vector<2>(mPadding * 2, mPadding * 2));

	//reset path
	mPath.clear();
	EventPathUpdated();

}

bool Steering::isEnabled() const
{
	return mSteeringEnabled;
}

const std::list<WFMath::Point<3>>& Steering::getPath() const
{
	return mPath;
}

void Steering::update()
{
	if (mSteeringEnabled) {
		if (mUpdateNeeded) {
			updatePath();
		}
		auto entity = mAvatar.getEntity();
		if (!mPath.empty()) {
			const auto& finalDestination = mPath.back();
			auto entity3dPosition = entity->getViewPosition();
			const WFMath::Point<2> entityPosition(entity3dPosition.x(), entity3dPosition.z());
			//First check if we've arrived at our actual destination.
			if (WFMath::Distance(WFMath::Point<2>(finalDestination.x(), finalDestination.z()), entityPosition) < 0.1f) {
				//We've arrived at our destination. If we're moving we should stop.
				if (mLastSentVelocity.isValid() && mLastSentVelocity != WFMath::Vector<2>::ZERO()) {
					moveInDirection(WFMath::Vector<2>::ZERO());
				}
				stopSteering();
			} else {
				//We should send a move op if we're either not moving, or we've reached a waypoint, or we need to divert a lot.

				WFMath::Point<2> nextWaypoint(mPath.front().x(), mPath.front().z());
				if (WFMath::Distance(nextWaypoint, entityPosition) < 0.1f) {
					mPath.pop_front();
					nextWaypoint = WFMath::Point<2>(mPath.front().x(), mPath.front().z());
				}

				WFMath::Vector<2> velocity = nextWaypoint - entityPosition;
				WFMath::Point<2> destination;
				velocity.normalize();

				if (mPath.size() == 1) {
					//if the next waypoint is the destination we should send a "move to position" update to the server, to make sure that we stop when we've arrived.
					//otherwise, if there's too much lag, we might end up overshooting our destination and will have to double back
					destination = nextWaypoint;
				}

				//Check if we need to divert in order to avoid colliding.
				WFMath::Vector<2> newVelocity;
				bool avoiding = mAwareness.avoidObstacles(entityPosition, velocity * mSpeed, newVelocity);
				if (avoiding) {
					auto newMag = newVelocity.mag();
					auto relativeMag = mSpeed / newMag;

					velocity = newVelocity;
					velocity.normalize();
					velocity *= relativeMag;
					mUpdateNeeded = true;
				}

				bool shouldSend = false;
				if (velocity.isValid()) {
					if (mLastSentVelocity.isValid()) {
						//If the entity has stopped, and we're not waiting for confirmation to a movement request we've made, we need to start moving.
						if (!entity->isMoving() && !mExpectingServerMovement) {
							shouldSend = true;
						} else {
							auto currentTheta = std::atan2(mLastSentVelocity.y(), mLastSentVelocity.x());
							auto newTheta = std::atan2(velocity.y(), velocity.x());

							//If we divert too much from where we need to go we must adjust.
							if (std::abs(currentTheta - newTheta) > WFMath::numeric_constants<double>::pi() / 20) {
								shouldSend = true;
							}
						}
					} else {
						//If we've never sent a movement update before we should do that now.
						shouldSend = true;
					}
				}
				if (shouldSend) {
					//If we're moving to a certain destination and aren't avoiding anything we should tell the server to move to the destination.
					if (destination.isValid() && !avoiding) {
						moveToPoint(WFMath::Point<3>(destination.x(), entity3dPosition.y(), destination.y()));
					} else {
						moveInDirection(velocity);
					}
				}
			}
		} else {
			//We are steering, but the path is empty, which means we can't find any path. If we're moving we should stop movement.
			//But we won't stop steering; perhaps we'll find a path later.
			if (mLastSentVelocity.isValid() && mLastSentVelocity != WFMath::Vector<2>::ZERO()) {
				moveInDirection(WFMath::Vector<2>::ZERO());
			}
		}
	}

}

void Steering::moveInDirection(const WFMath::Vector<2>& direction)
{
	WFMath::Vector<3> fullDirection(direction.x(), 0, direction.y());
	WFMath::Quaternion orientation;
	if (direction != WFMath::Vector<2>::ZERO()) {
		orientation.rotation(WFMath::Vector<3>(0, 0, 1), WFMath::Vector<3>(fullDirection).normalize(), WFMath::Vector<3>(0, 1, 0));
	}

	mAvatar.moveInDirection(fullDirection, orientation);
	mLastSentVelocity = direction;
	mExpectingServerMovement = true;
}

void Steering::moveToPoint(const WFMath::Point<3>& point)
{

	auto entity3dPosition = mAvatar.getEntity()->getViewPosition();
	WFMath::Vector<3> vel = point - entity3dPosition;

	WFMath::Quaternion orientation;
	if (vel != WFMath::Vector<3>::ZERO()) {
		orientation.rotation(WFMath::Vector<3>(0, 0, 1), WFMath::Vector<3>(vel).normalize(), WFMath::Vector<3>(0, 1, 0));
	}

	mAvatar.moveToPoint(point, orientation);

	mLastSentVelocity = WFMath::Vector<2>(vel.x(), vel.z());
	mExpectingServerMovement = true;
}

void Steering::Awareness_TileUpdated(int tx, int ty)
{
	mUpdateNeeded = true;
}

bool Steering::getIsExpectingServerMovement() const
{
	return mExpectingServerMovement;
}

void Steering::setIsExpectingServerMovement(bool expected)
{
	mExpectingServerMovement = expected;
}

}
}

