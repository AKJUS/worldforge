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

#ifndef PLANTQUERYTASK_H_
#define PLANTQUERYTASK_H_

#include "Types.h"
#include "framework/tasks/TemplateNamedTask.h"
#include "PlantAreaQueryResult.h"

#include <sigc++/slot.h>

namespace Ember {
namespace OgreView {

namespace Terrain {

class TerrainPage;

class TerrainPageGeometry;

namespace Foliage {
class PlantPopulator;
}

class PlantQueryTask : public Tasks::TemplateNamedTask<PlantQueryTask> {
public:
	PlantQueryTask(SegmentRefPtr segmentRef,
				   Foliage::PlantPopulator& plantPopulator,
				   PlantAreaQuery query,
				   sigc::slot<void, const PlantAreaQueryResult&> asyncCallback);

	~PlantQueryTask() override;

	void executeTaskInBackgroundThread(Tasks::TaskExecutionContext& context) override;

	bool executeTaskInMainThread() override;

private:
	SegmentRefPtr mSegmentRef;
	Foliage::PlantPopulator& mPlantPopulator;
	sigc::slot<void, const PlantAreaQueryResult&> mAsyncCallback;

	PlantAreaQueryResult mQueryResult;
};

}

}

}

#endif /* PLANTQUERYTASK_H_ */
