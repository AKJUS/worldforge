//
// C++ Interface: PolygonPointMover
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2009
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
#ifndef EMBEROGRE_MANIPULATIONPOLYGONPOINTMOVER_H
#define EMBEROGRE_MANIPULATIONPOLYGONPOINTMOVER_H

#include "IMovementBridge.h"
#include "services/input/Input.h"
#include <wfmath/point.h>
#include <sigc++/trackable.h>

namespace Ember
{
namespace OgreView
{

namespace Authoring
{

class PolygonPoint;
class Polygon;

/**
 @brief A listener interface which will receive updates when the movement of the point changes.
 @author Erik Ogenvik <erik@ogenvik.org>
 */
struct IMovementListener
{

	/**
	 * @brief Dtor.
	 */
	virtual ~IMovementListener() = default;

	/**
	 * @brief The movement has ended successfully.
	 */
	virtual void endMovement() = 0;

	/**
	 * @brief The movement was cancelled.
	 */
	virtual void cancelMovement() = 0;
};

/**
 @brief Responsible for handling movement of a PolygonPoint instance.
 The movement happens through a graphical interface in Ogre. Basically a sphere is added to the world, which can then moved with the mouse.
 If a point is moved and ctrl is pressed a new point will be created.
 If a point is moved and alt is pressed the point will be removed from the polygon.

 @author Erik Ogenvik <erik@ogenvik.org>
 */
class PolygonPointMover: public IMovementBridge, public virtual sigc::trackable
{
public:

	/**
	 * @brief Ctor.
	 * @param polygon The polygon to which the poing belongs.
	 * @param point The point for which movement should occur.
	 */
	PolygonPointMover(Polygon& polygon, PolygonPoint& point, IMovementListener* listener = nullptr);

	/**
	 * @brief Dtor.
	 */
	~PolygonPointMover() override;

	const WFMath::Quaternion& getOrientation() const override;
	void setOffset(boost::optional<float> offset) override;
	boost::optional<float> getOffset() const override;
	const WFMath::Point<3>& getPosition() const override;
	void setPosition(const WFMath::Point<3>& position) override;
	void move(const WFMath::Vector<3>& directionVector) override;
	void setRotation(int axis, WFMath::CoordType angle) override;
	void setOrientation(const WFMath::Quaternion& rotation) override;
	void yaw(WFMath::CoordType angle) override;

	void finalizeMovement() override;
	void cancelMovement() override;

private:

	/**
	 * @brief The polygon to which the point belongs.
	 */
	Polygon& mPolygon;

	/**
	 * @brief The polygon point to which this movement belongs.
	 */
	PolygonPoint& mPoint;

	/**
	 * @brief The current position of the polygon point.
	 */
	mutable WFMath::Point<3> mPosition;

	/**
	 * @brief Stores a newly created point.
	 */
	PolygonPoint* mNewPoint;

	/**
	 * @brief If true, the point has been deleted.
	 */
	bool mDeleted;

	/**
	 * @brief We need to keep track of the point preceding our own point, in the case that a delete operation is aborted (since we then need to return the point to its correct place in the polygon).
	 */
	PolygonPoint* mPointAfterDeleted;

	/**
	 * @brief Gets the active point, which is either the existing point or a new one if any such exists.
	 * @see mNewPoint
	 * @return Either the existing point, or a new point if any such exists.
	 */
	PolygonPoint* getActivePoint() const;

	/**
	 * @brief The initial position of the point, when the movement started.
	 */
	WFMath::Point<2> mInitialPosition;

	void input_KeyPressed(const SDL_Keysym& key, Input::InputMode mode);
	void input_KeyReleased(const SDL_Keysym& key, Input::InputMode mode);

	/**
	 * @brief Switch to new point mode.
	 * This involves creating a new point, positioning that at the current position of the moved point, and reverting the existing point back to the original position.
	 * If new point mode already is selected nothing will happen.
	 */
	void switchToNewPointMode();

	/**
	 * @brief Switches to existing point mode. This is the default.
	 * This involves removing the new point and placing the existing point on the mouse position.
	 * If existing point mode already is selected nothing will happen.
	 */
	void switchToExistingPointMode();

	/**
	 * @brief Switches to deleted mode.
	 * In deleted mode the point is removed from the polygon.
	 */
	void switchToDeleteMode();

	/**
	 * @brief An optional listener which will receive movement updates.
	 */
	IMovementListener* mListener;

	void processPickResults(const std::vector<PickResult>& results);
};

}

}

}

#endif
