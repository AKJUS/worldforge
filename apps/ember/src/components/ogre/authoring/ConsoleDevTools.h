/*
 * Copyright (C) 2014 Peter Szucs <peter.szucs.dev@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CONSOLEDEVTOOLS_H_
#define CONSOLEDEVTOOLS_H_

#include "framework/ConsoleObject.h"
#include "framework/ConsoleCommandWrapper.h"
#include <OgreFrameListener.h>

#include <string>


namespace Ember::OgreView {

/**
 * @brief Adds developer console commands.
 */
class ConsoleDevTools :
		protected ConsoleObject {
public:
	/**
	 * @brief Ctor.
	 * @param entity The entity being affected.
	 * @param mode The mode of the composition. Either "none", "composition" or "exclusive". This corresponds to EmberEntity::CompositionMode.
	 */
	ConsoleDevTools();

	~ConsoleDevTools() override;

	void showTexture(const std::string& textureName);

	void reloadMaterial(const std::string& materialName);

	void reloadTexture(const std::string& textureName);

	void performBenchmark();

protected:
	/**
	 * Reloads material at runtime.
	 */
	const ConsoleCommandWrapper mReloadMaterial;

	/**
	 * Shows a window of the RTT texture with runtime updating.
	 */
	const ConsoleCommandWrapper mShowTexture;

	const ConsoleCommandWrapper mBenchmark;

	std::unique_ptr<Ogre::FrameListener> mFrameListener;

	/**
	 * Reimplements the ConsoleObject::runCommand method
	 */
	void runCommand(const std::string& command, const std::string& args) override;

	static std::string genUniqueName();
};

}

#endif
