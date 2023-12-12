/*
 Copyright (C) 2009 Erik Ogenvik

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

#ifndef TASKEXECUTIONLISTENER_H_
#define TASKEXECUTIONLISTENER_H_

#include "framework/Exception.h"


namespace Ember::Tasks {

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief A listener for tasks being executed.
 */
struct ITaskExecutionListener {
public:
	/**
	 * @brief Dtor.
	 */
	virtual ~ITaskExecutionListener() = default;

	/**
	 * @brief Called when execution has started.
	 */
	virtual void executionStarted() = 0;

	/**
	 * @brief Called when execution has ended.
	 */
	virtual void executionEnded() = 0;

	/**
	 * @brief Called when an error occurred during execution.
	 * @param exception Describes the error.
	 */
	virtual void executionError(const Exception& exception) = 0;
};

}


#endif /* TASKEXECUTIONLISTENER_H_ */
