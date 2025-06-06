/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

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

#ifndef EMBEROGRETERRAINTERRAINMATERIALCOMPILATIONTASK_H_
#define EMBEROGRETERRAINTERRAINMATERIALCOMPILATIONTASK_H_

#include "framework/tasks/TemplateNamedTask.h"
#include "Types.h"
#include <sigc++/signal.h>
#include <vector>


namespace Ember::OgreView::Terrain {

class TerrainPageSurfaceCompilationInstance;

class TerrainPage;

/**
 * @brief Recompiles the material for a terrain page.
 * @author Erik Ogenvik <erik@ogenvik.org>
 */
class TerrainMaterialCompilationTask : public Tasks::TemplateNamedTask<TerrainMaterialCompilationTask> {
public:

	/**
	 * @brief Ctor.
	 * @param pages The pages which needs to have their material recompiled.
	 * @param signal The signal to emit once the compilation is finished.
	 * @param lightDirection The main light direction.
	 */
	TerrainMaterialCompilationTask(GeometryPtrVector geometry,
								   sigc::signal<void(TerrainPage&)>& signal);

	/**
	 * @brief Ctor.
	 * @param page The page which needs to have its material recompiled.
	 * @param lightDirection The main light direction.
	 */
	TerrainMaterialCompilationTask(TerrainPageGeometryPtr pageGeometry,
								   sigc::signal<void(TerrainPage&)>& signal);

	/**
	 * @brief Dtor.
	 */
	~TerrainMaterialCompilationTask() override = default;

	void executeTaskInBackgroundThread(Tasks::TaskExecutionContext& context) override;

	bool executeTaskInMainThread() override;

private:

	/**
	 * @brief A store of terrain page surface compilation instances, and the page to which they belong.
	 */
	typedef std::vector<std::pair<std::unique_ptr<TerrainPageSurfaceCompilationInstance>, std::shared_ptr<Terrain::TerrainPage>>> CompilationInstanceStore;

	/**
	 * @brief The pages which needs recompilation.
	 */
	GeometryPtrVector mGeometry;

	/**
	 * @brief The compilation instances and their corresponding pages.
	 */
	CompilationInstanceStore mMaterialRecompilations;


	/**
	 * @brief A signal to emit once the update is done.
	 */
	sigc::signal<void(TerrainPage&)>& mSignal;

	/**
	 * @brief This needs to be called after materials have changed to make sure that Ogre flushes it's material caches.
	 * Failure to do so will result in assert errors during Ogre's rendering.
	 */
	void updateSceneManagersAfterMaterialsChange();


};

}


#endif /* EMBEROGRETERRAINTERRAINMATERIALCOMPILATIONTASK_H_ */
