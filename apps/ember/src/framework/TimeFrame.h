/*
 Copyright (C) 2011 Erik Ogenvik

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

#ifndef TIMEFRAME_H_
#define TIMEFRAME_H_

#include <chrono>

namespace Ember {

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief Represents a time frame of future time.
 *
 * An instance of this is to be used whenever you want to check whether a certain amount of time has elapsed.
 * The typical situation where it would be useful is when rendering a frame, and you don't want to spend too much time until the next frame.
 */
struct TimeFrame {

	/**
	 * @brief Ctor.
	 * @param timeSlice The slice of time for this time frame.
	 */
	explicit TimeFrame(std::chrono::steady_clock::duration timeSlice, std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now());

	/**
	 * @brief Checks whether there's any time left until the threshold has passed.
	 * @return True if there's any time left.
	 */
	bool isTimeLeft() const;

	/**
	 * @brief Returns the remaining.
	 * @return The remaining time.
	 */
	std::chrono::nanoseconds getRemainingTime() const;

	/**
	 * @brief Returns the elapsed time.
	 * @return The elapsed time.
	 */
	std::chrono::nanoseconds getElapsedTime() const;

	/**
	 * @brief Time when the task started.
	 */
	std::chrono::steady_clock::time_point mStartTime;

	std::chrono::steady_clock::time_point mEndTime;
};

}

#endif /* TIMEFRAME_H_ */
