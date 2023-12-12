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

#include "OgrePluginLoader.h"


#include <OgreRoot.h>
#include <OgreBuildSettings.h>
#include <fstream>

#ifdef OGRE_STATIC_LIB

#ifdef OGRE_BUILD_PLUGIN_PFX

#include <Plugins/ParticleFX/OgreParticleFXPlugin.h>

#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS

#include <RenderSystems/GL3Plus/OgreGL3PlusPlugin.h>

#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES
#include <RenderSystems/GLES/OgreGLESPlugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#include <RenderSystems/GLES2/OgreGLES2Plugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
#include <RenderSystems/Direct3D9/OgreD3D9Plugin.h>
#endif
//#ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
//#  include <OgreD3D11Plugin.h>
//#endif

#endif //ifdef OGRE_STATIC_LIB


namespace Ember::OgreView {
OgrePluginLoader::OgrePluginLoader() {
#ifdef OGRE_STATIC_LIB
#ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
	mPlugins.emplace("RenderSystem_GL3Plus", OGRE_NEW Ogre::GL3PlusPlugin());
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES
	mPlugins.emplace("RenderSystem_GLES", OGRE_NEW Ogre::GLESPlugin());
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
	mPlugins.emplace("RenderSystem_GLES2", OGRE_NEW Ogre::GLES2Plugin());
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
	mPlugins.emplace("RenderSystem_Direct3D9", OGRE_NEW Ogre::D3D9Plugin());
#endif
#ifdef OGRE_BUILD_PLUGIN_PFX
	mPlugins.emplace("Plugin_ParticleFX", OGRE_NEW Ogre::ParticleFXPlugin());
#endif
#else // ifndef OGRE_STATIC_LIB
	auto& configSrv = ConfigService::getSingleton();
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	//on windows we'll bundle the dll files in the same directory as the executable
	mPluginDirs.push_back(".");
	mPluginExtension = ".dll";
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX

	//If any prefix is set (for example for AppImage builds), check for the plugins in directories relative to the prefix first.
	if (!configSrv.getPrefix().empty()) {
		mPluginDirs.push_back(configSrv.getPrefix() + "/lib64/OGRE");
		mPluginDirs.push_back(configSrv.getPrefix() + "/lib/OGRE");
	}

	mPluginDirs.emplace_back(OGRE_PLUGINDIR);

	mPluginExtension = ".so";
#ifdef ENABLE_BINRELOC
	//binreloc might be used
	char* br_libdir = br_find_lib_dir(br_strcat(PREFIX, "/lib"));
	std::string libDir(br_libdir);
	free(br_libdir);
	mPluginDirs.push_back(libDir + "/OGRE");
#endif
	//enter the usual locations if Ogre is installed system wide, with local installations taking precedence
	mPluginDirs.emplace_back("/usr/local/lib64/OGRE");
	mPluginDirs.emplace_back("/usr/local/lib/OGRE");
	mPluginDirs.emplace_back("/usr/lib64/OGRE");
	mPluginDirs.emplace_back("/usr/lib/OGRE");
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
	// On Mac, plugins are found in Resources in the Main (Application) bundle, then in the Ogre framework bundle
	std::string pluginDir = configSrv.getSharedDataDirectory();
	mPluginDirs.push_back(pluginDir);
	pluginDir += "/../Plugins";
	mPluginDirs.push_back(pluginDir);
	mPluginExtension = ".dylib";
#endif

#endif // ifndef OGRE_STATIC_LIB
}

void OgrePluginLoader::addPluginDir(const std::string& dir) {
	mPluginDirs.insert(mPluginDirs.begin(), dir);
}

bool OgrePluginLoader::loadPlugin(const std::string& pluginName) {
#ifndef OGRE_STATIC_LIB
	// If the dynamic lib is not yet loaded, try to find and load it.
	// Load the shared library.
	return loadDynPlugin(pluginName);
#else
	auto it = mPlugins.find(pluginName);
	if (it != mPlugins.end()) {
		Ogre::Root::getSingleton().installPlugin(it->second);
		return true;
	}
	logger->error("Could not find required plugin {}", pluginName);
	return false;
#endif
}

void OgrePluginLoader::unloadPlugins() {
	auto plugins = Ogre::Root::getSingleton().getInstalledPlugins();
	for (Ogre::Plugin* plugin: plugins) {
		plugin->uninstall();
	}
}

bool OgrePluginLoader::loadDynPlugin(const std::string& pluginName) {
#ifndef OGRE_STATIC_LIB

	for (const std::string& dir : mPluginDirs) {
		std::string pluginPath;
		pluginPath = dir + "/" + pluginName + mPluginExtension;
		if (std::ifstream(pluginPath).good()) {
			logger->info("Trying to load the plugin '{}'.", pluginPath);
			Ogre::Root::getSingleton().loadPlugin(pluginPath);
			return true;
		}
	}
	std::stringstream ss;
	ss << "Failed to load the plugin '" << pluginName << "'!";
	throw std::runtime_error(ss.str());
#else
	// Would work, but you should use static libs on static build to prevent strange bugs.
	assert(0);
#endif
	return false;
}

}

