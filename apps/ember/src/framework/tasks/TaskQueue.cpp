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

#include "TaskQueue.h"
#include "ITask.h"
#include "ITaskExecutionListener.h"
#include "TaskExecutor.h"
#include "TaskUnit.h"

#include "framework/Log.h"

#include <Eris/EventService.h>

namespace Ember::Tasks {

TaskQueue::TaskQueue(unsigned int numberOfExecutors, Eris::EventService& eventService) :
		mEventService(eventService), mActive(true), mIsQueuedOnMainThread(false) {
	logger->debug("Creating task queue with {} executors.", numberOfExecutors);
	for (unsigned int i = 0; i < numberOfExecutors; ++i) {
		mExecutors.push_back(std::make_unique<TaskExecutor>(*this));
	}
}

TaskQueue::~TaskQueue() {
	deactivate();
}

void TaskQueue::deactivate() {
	*mActiveMarker.getMarker() = false;
	if (mActive) {
		{
			std::unique_lock<std::mutex> l(mUnprocessedQueueMutex);
			mActive = false;
		}
		mUnprocessedQueueCond.notify_all();
		//Join all executors. Since the queue is shutting down they will all exit their main loop if there are no more tasks to process.
		for (auto& executor : mExecutors) {
			executor->join();
		}
		mExecutors.clear();

		//Finally we must process all of the tasks in our main loop. This of course requires that this instance is destroyed from the main loop.
		while (!mProcessedTaskUnits.empty()) {
			processCompletedTasks();
		}

		assert(mProcessedTaskUnits.empty());
		assert(mUnprocessedTaskUnits.empty());
	}
}

bool TaskQueue::enqueueTask(std::unique_ptr<ITask> task, ITaskExecutionListener* listener) {
	std::unique_lock<std::mutex> l(mUnprocessedQueueMutex);
	if (mActive) {
		mUnprocessedTaskUnits.emplace(std::make_unique<TaskUnit>(std::move(task), listener));
		mUnprocessedQueueCond.notify_one();
		return true;
	} else {
		logger->warn("Tried to enqueue the task {} on a task queue which isn't active (i.e. is shutting down).", task->getName());
		return false;
	}

}

std::unique_ptr<TaskUnit> TaskQueue::fetchNextTask() {
	//The semantics of this method is that if a null pointer is returned the task executor is
	// required to exit its main processing loop, since this indicates that the queue is shutting down.
	std::unique_ptr<TaskUnit> taskUnit;
	std::unique_lock<std::mutex> lock(mUnprocessedQueueMutex);
	if (mUnprocessedTaskUnits.empty()) {
		if (!mActive) {
			return {};
		}
		mUnprocessedQueueCond.wait(lock);
	}
	if (!mUnprocessedTaskUnits.empty()) {
		taskUnit = std::move(mUnprocessedTaskUnits.front());
		mUnprocessedTaskUnits.pop();
	}
	return taskUnit;
}

void TaskQueue::addProcessedTask(std::unique_ptr<TaskUnit> taskUnit) {
	std::unique_lock<std::mutex> lock(mProcessedQueueMutex);

	mProcessedTaskUnits.push(std::move(taskUnit));
	if (!mIsQueuedOnMainThread) {
		//Make sure that the task is handled on the main queue.
		mEventService.runOnMainThread([this] {
			processCompletedTasks();
		}, mActiveMarker);
	}

}

bool TaskQueue::isActive() const {
	return mActive;
}

void TaskQueue::processCompletedTasks() {
	if (!mProcessedTaskUnits.empty()) {
		TaskUnit* taskUnit;
		{
			std::unique_lock<std::mutex> lock(mProcessedQueueMutex);
			taskUnit = mProcessedTaskUnits.front().get();
		}
		try {
			bool result = taskUnit->executeInMainThread();
			if (result) {
				try {
					std::unique_lock<std::mutex> lock(mProcessedQueueMutex);
					mProcessedTaskUnits.pop();
				} catch (const std::exception& ex) {
					logger->error("Error when deleting task in main thread: {}", ex.what());
				} catch (...) {
					logger->error("Unknown error when deleting task in main thread.");
				}
			}

		} catch (const std::exception& ex) {
			logger->error("Error when executing task in main thread: {}", ex.what());
			//Task is broken; remove it
			std::unique_lock<std::mutex> lock(mProcessedQueueMutex);
			mProcessedTaskUnits.pop();
		} catch (...) {
			logger->error("Unknown error when executing task in main thread.");
			//Task is broken; remove it
			std::unique_lock<std::mutex> lock(mProcessedQueueMutex);
			mProcessedTaskUnits.pop();
		}

		{
			std::unique_lock<std::mutex> lock(mProcessedQueueMutex);
			if (!mProcessedTaskUnits.empty()) {
				mEventService.runOnMainThread([this] {
					processCompletedTasks();
				}, mActiveMarker);
			} else {
				mIsQueuedOnMainThread = false;
			}
		}

	}
}

}


