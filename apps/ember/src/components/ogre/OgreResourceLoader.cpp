//
// C++ Implementation: OgreResourceLoader
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2006
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
#include "OgreResourceLoader.h"
#include "services/EmberServices.h"
#include "framework/Tokeniser.h"
#include "services/server/ServerService.h"
#include "services/config/ConfigService.h"
#include "model/ModelDefinitionManager.h"
#include "sound/XMLSoundDefParser.h"

#include "EmberOgreFileSystem.h"

#include "framework/TimedLog.h"
#include <OgreArchiveManager.h>
#include <Ogre.h>
#include <boost/algorithm/string.hpp>
#include <framework/FileSystemObserver.h>
#include <framework/MainLoopController.h>
#include <components/ogre/model/XMLModelDefinitionSerializer.h>
#include <squall/core/Repository.h>
#include "SquallArchive.h"


/**
 * This method is licensed under the CC BY-SA 3.0 as it's taken from this StackOverflow answer: https://stackoverflow.com/a/29221546
 * Copyright 2017 Robert Massaioli, https://stackoverflow.com/users/83446/robert-massaioli
 *
 * It's provided since boost::filesystem::relative is only available in boost 1.60, and we want to support
 * distros that don't yet have that version.
 *
 * Provides a relative path.
 */
static boost::filesystem::path relativeTo(const boost::filesystem::path& from, const boost::filesystem::path& to) {
	// Start at the root path and while they are the same then do nothing then when they first
	// diverge take the entire from path, swap it with '..' segments, and then append the remainder of the to path.
	boost::filesystem::path::const_iterator fromIter = from.begin();
	boost::filesystem::path::const_iterator toIter = to.begin();

	// Loop through both while they are the same to find nearest common directory
	while (fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter)) {
		++toIter;
		++fromIter;
	}

	// Replace from path segments with '..' (from => nearest common directory)
	boost::filesystem::path finalPath;
	while (fromIter != from.end()) {
		finalPath /= "..";
		++fromIter;
	}

	// Append the remainder of the to path (nearest common directory => to)
	while (toIter != to.end()) {
		finalPath /= *toIter;
		++toIter;
	}

	return finalPath;
}

namespace Ember::OgreView {

struct EmberResourceLoadingListener : public Ogre::ResourceLoadingListener {
	Ogre::DataStreamPtr resourceLoading(const Ogre::String& name, const Ogre::String& group, Ogre::Resource* resource) override {
		return {};
	}

	void resourceStreamOpened(const Ogre::String& name, const Ogre::String& group, Ogre::Resource* resource, Ogre::DataStreamPtr& dataStream) override {

	}

	bool resourceCollision(Ogre::Resource* resource, Ogre::ResourceManager* resourceManager) override {
		logger->debug("Resource '{}' already exists in group '{}' for type '{}'.", resource->getName(), resource->getGroup(), resourceManager->getResourceType());
		if (resourceManager->getResourceType() == "Material") {

			//If a material, update the old version once the new one has been compiled (hence the need for "runOnMainThread".
			//Note that this only works the first time a material is updated.
			Ogre::MaterialPtr existingMaterial = Ogre::MaterialManager::getSingleton().getByName(resource->getName(), resource->getGroup());

			MainLoopController::getSingleton().getEventService().runOnMainThread([=]() {
				Ogre::MaterialPtr oldMat = existingMaterial;
				Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(existingMaterial->getName(), existingMaterial->getGroup());
				if (mat) {
					mat->copyDetailsTo(oldMat);
					oldMat->load();
				} else {
					logger->warn("Material '{}' does not exist anymore in group '{}'.", existingMaterial->getName(), existingMaterial->getGroup());
				}
			});
		}

		resourceManager->remove(resource->getName(), resource->getGroup());
		return true;
	}
};

OgreResourceLoader::OgreResourceLoader(Squall::Repository repository) :
		UnloadUnusedResources("unloadunusedresources", this, "Unloads any unused resources."),
		mFileSystemArchiveFactory(std::make_unique<FileSystemArchiveFactory>()),
		mSquallArchiveFactory(std::make_unique<SquallArchiveFactory>(repository)),
		mLoadingListener(std::make_unique<EmberResourceLoadingListener>()) {
}

OgreResourceLoader::~OgreResourceLoader() {
	//Don't deregister mLoadingListener, since this destructor needs to be called after OGRE has been shut down, and there won't be any ResourceGroupManager
	//available by then.
	for (auto& path: mResourceRootPaths) {
		Ember::FileSystemObserver::getSingleton().remove_directory(path);
	}
}

void OgreResourceLoader::initialize() {

	//We can only add an archive factory, there's no method for removing it. Thus an instance of this needs to survive longer than Ogre itself.
	Ogre::ArchiveManager::getSingleton().addArchiveFactory(mFileSystemArchiveFactory.get());
	Ogre::ArchiveManager::getSingleton().addArchiveFactory(mSquallArchiveFactory.get());

	Ogre::ResourceGroupManager::getSingleton().setLoadingListener(mLoadingListener.get());

	auto& configSrv = ConfigService::getSingleton();

	if (configSrv.itemExists("media", "extraresourcelocations")) {
		varconf::Variable const resourceConfigFilesVar = configSrv.getValue("media", "extraresourcelocations");
		std::string const resourceConfigFiles = resourceConfigFilesVar.as_string();
		mExtraResourceLocations = Tokeniser::split(resourceConfigFiles, ";");
	}
}

void OgreResourceLoader::runCommand(const std::string& command, const std::string& args) {
	if (UnloadUnusedResources == command) {
		unloadUnusedResources();
	}
}

void OgreResourceLoader::unloadUnusedResources() {
	TimedLog const l("Unload unused resources.");
	Ogre::ResourceGroupManager& resourceGroupManager(Ogre::ResourceGroupManager::getSingleton());

	auto resourceGroups = resourceGroupManager.getResourceGroups();
	for (const auto& resourceGroup: resourceGroups) {
		resourceGroupManager.unloadUnreferencedResourcesInGroup(resourceGroup, false);
	}
}

bool OgreResourceLoader::addSharedMedia(const std::string& path, const std::string& type, const std::string& section) {
	auto sharedMediaPath = ConfigService::getSingleton().getSharedDataDirectory();
	return addResourceDirectory(sharedMediaPath / path, type, section, OnFailure::Throw);
}

bool OgreResourceLoader::addUserMedia(const std::string& path, const std::string& type, const std::string& section) {
	auto userMediaPath = ConfigService::getSingleton().getUserMediaDirectory();

	return addResourceDirectory(userMediaPath / path, type, section, OnFailure::Ignore);
}

bool OgreResourceLoader::addResourceDirectory(const boost::filesystem::path& path,
											  const std::string& type,
											  const std::string& section,
											  OnFailure onFailure) {
	if (boost::filesystem::is_directory(path)) {
		logger->debug("Adding dir {}", path.string());
		try {
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(path.string(), type, section, true);
			mResourceRootPaths.emplace_back(path.string());
			observeDirectory(path);

			return true;
		} catch (const std::exception&) {
			switch (onFailure) {
				case OnFailure::Ignore:
					break;
				case OnFailure::Report:
					logger->error("Couldn't load {}. Continuing as if nothing happened.", path.string());
					break;
				case OnFailure::Throw:
					throw Ember::Exception(std::string("Could not load from required directory '") + path.string() +
										   "'. This is fatal and Ember will shut down. The probable cause for this error is that you haven't properly installed all required media.");
			}
		}
	} else {
		switch (onFailure) {
			case OnFailure::Ignore:
				break;
			case OnFailure::Report:
				logger->error("Couldn't find resource directory {}", path.string());
				break;
			case OnFailure::Throw:
				throw Ember::Exception(fmt::format(
						"Could not find required directory '{}'. This is fatal and Ember will shut down. The probable cause for this error is that you haven't properly installed all required media.",
						path.string()));
		}
	}
	return false;
}

void OgreResourceLoader::loadBootstrap() {
	addSharedMedia("data/ui", "EmberFileSystem", "UI");

	addSharedMedia("data/assets", "EmberFileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	addSharedMedia("OGRE/Media/Main", "EmberFileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	addSharedMedia("OGRE/Media/RTShaderLib/GLSL", "EmberFileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	addUserMedia("media/assets", "EmberFileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void OgreResourceLoader::loadGui() {
	addSharedMedia("gui", "EmberFileSystem", "UI");
	addUserMedia("gui", "EmberFileSystem", "UI");
}

void OgreResourceLoader::loadGeneral() {


	//Lua scripts
	addSharedMedia("scripting", "EmberFileSystem", "Scripting");
	addUserMedia("scripting", "EmberFileSystem", "Scripting");

	//Model definitions, terrain definitions, sound definitions and entity mappings
	//TODO: remove
	addSharedMedia("data/dural", "EmberFileSystem", "Data");
	addUserMedia("data", "EmberFileSystem", "Data");

	//The Caelum component
	addSharedMedia("data/caelum", "EmberFileSystem", "Caelum");

	//Entity recipes
	addSharedMedia("data/entityrecipes", "EmberFileSystem", "EntityRecipes");
	addUserMedia("entityrecipes", "EmberFileSystem", "EntityRecipes");

	//End with adding any extra defined locations.
	for (auto& location: mExtraResourceLocations) {
		addResourceDirectory(location, "EmberFileSystem", "Extra", OnFailure::Report);
	}


}

void OgreResourceLoader::preloadMedia() {
	// resource groups to be loaded
	const char* resourceGroup[] = {"General", "Data"};

	for (auto& group: resourceGroup) {
		try {
			Ogre::ResourceGroupManager::getSingleton().loadResourceGroup(group);
		} catch (const std::exception& ex) {
			logger->error("An error occurred when preloading media: {}", ex.what());
		}
	}
}

void OgreResourceLoader::observeDirectory(const boost::filesystem::path& path) {
	try {
		FileSystemObserver::getSingleton().add_directory(path, [](const FileSystemObserver::FileSystemEvent& event) {
			auto& ev = event.ev;
			//Skip if it's not a file. This also means that we won't catch deletion of files. That's ok for now; but perhaps we need to revisit this.
			if (!boost::filesystem::is_regular_file(ev.path)) {
				return;
			}
			logger->debug("Resource changed {} {}", ev.path.string(), ev.type_cstr());

			if (ev.type == boost::asio::dir_monitor_event::modified) {
				try {
					if (boost::filesystem::file_size(ev.path) == 0) {
						return;
					}
				} catch (...) {
					//Could not find file, just return
					return;
				}
			}


			auto resourceGroups = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
			for (auto& group: resourceGroups) {

				auto reloadResource = [&](Ogre::ResourceManager& resourceManager, const std::string& resourceName) {
					if (resourceManager.resourceExists(resourceName, group)) {
						auto resource = resourceManager.getResourceByName(resourceName, group);
						if (resource->isLoaded() || resource->isPrepared()) {
							try {
								resource->reload();
							} catch (const std::exception& e) {
								logger->error("Could not reload resource '{}' of type '{}': {}", resourceName, resourceManager.getResourceType(), e.what());
							}
						}
					} else {
						//Add resource
						Ogre::ResourceGroupManager::getSingleton().declareResource(resourceName, resourceManager.getResourceType(), group);
						resourceManager.createResource(resourceName, group);
					}

				};

				auto startsWith = [](const boost::filesystem::path& root, boost::filesystem::path aPath) {
					while (!aPath.empty()) {
						if (aPath == root) {
							return true;
						}
						aPath = aPath.parent_path();
					}
					return false;
				};

				auto locations = Ogre::ResourceGroupManager::getSingleton().listResourceLocations(group);
				for (auto& location: *locations) {
					boost::filesystem::path locationDirectory(location);
					if (startsWith(locationDirectory, ev.path)) {
						auto relative = relativeTo(locationDirectory, ev.path);
						auto extension = ev.path.extension();

						if (extension == ".modeldef") {
							if (event.ev.type == boost::asio::dir_monitor_event::event_type::added ||
								event.ev.type == boost::asio::dir_monitor_event::event_type::modified ||
								event.ev.type == boost::asio::dir_monitor_event::event_type::renamed_old_name ||
								event.ev.type == boost::asio::dir_monitor_event::event_type::renamed_new_name) {
								refreshModelDefinition(ev.path, relative);
							}
						} else if (extension == ".material") {
							try {
								auto materialMgr = Ogre::MaterialManager::getSingletonPtr();
								if (materialMgr) {
									std::ifstream stream(ev.path.string());
									Ogre::SharedPtr<Ogre::DataStream> fileStream(new Ogre::FileStreamDataStream(&stream, false));
									materialMgr->parseScript(fileStream, group);
								}
							} catch (const std::exception& ex) {
								logger->error("Error when parsing changed file '{}': {}", ev.path.string(), ex.what());
							}
						} else if (extension == ".dds" || extension == ".png" || extension == ".jpg") {
							reloadResource(Ogre::TextureManager::getSingleton(), relative.string());
						} else if (extension == ".mesh") {
							reloadResource(Ogre::MeshManager::getSingleton(), relative.string());
						} else if (extension == ".skeleton") {
							reloadResource(Ogre::SkeletonManager::getSingleton(), relative.string());
						} else if (extension == ".glsl" || extension == ".frag" || extension == ".vert") {
							//Reloading GLSL shaders in Render System GL doesn't seem to work. Perhaps we'll have more luck with GL3+?


//						{
//							Ogre::SharedPtr<Ogre::DataStream> stream(new Ogre::MemoryDataStream(0));
//							Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache(stream);
//
//							Ogre::HighLevelGpuProgramManager::getSingleton().reloadAll(true);
//							Ogre::GpuProgramManager::getSingleton().reloadAll(true);
//							Ogre::MaterialManager::getSingleton().reloadAll(true);
//						}



//						auto iterator = Ogre::HighLevelGpuProgramManager::getSingleton().getResourceIterator();
//						while (iterator.hasMoreElements()) {
//							auto resource = Ogre::static_pointer_cast<Ogre::HighLevelGpuProgram>(iterator.getNext());
//							if (resource->getSourceFile() == relative) {
//								logger->debug("Reloading GLSL script " << resource->getName());
//								Ogre::SharedPtr<Ogre::DataStream> stream(new Ogre::MemoryDataStream(0));
//								Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache(stream);
//								//Ogre::GpuProgramManager::getSingleton().cac
//								resource->reload();
//
////								auto matIterator = Ogre::MaterialManager::getSingleton().getResourceIterator();
////								while (matIterator.hasMoreElements()) {
////									bool needReload = false;
////									auto material = Ogre::static_pointer_cast<Ogre::Material>(matIterator.getNext());
////									for (auto* tech : material->getTechniques()) {
////										for (auto* pass : tech->getPasses()) {
////											if (pass->getFragmentProgramName() == resource->getName()) {
////												pass->setFragmentProgram("");
////												pass->setFragmentProgram(resource->getName());
////												needReload = true;
////											}
////											if (pass->getVertexProgramName() == resource->getName()) {
////												pass->setVertexProgram("");
////												pass->setVertexProgram(resource->getName());
////												needReload = true;
////											}
////										}
////									}
////									if (needReload) {
////										material->reload();
////									}
////								}
//							}
//						}
						}
					}
				}
			}
		});
	} catch (...) {
		//Ignore errors
	}

}

bool OgreResourceLoader::addMedia(const std::string& path, const std::string& resourceGroup) {
	return addSharedMedia("media/" + path, "EmberFileSystem", resourceGroup);
}

void OgreResourceLoader::refreshModelDefinition(const boost::filesystem::path& fullPath, const boost::filesystem::path& relativePath) {
	std::ifstream stream(fullPath.string());
	if (stream) {
		auto modelDefMgr = Model::ModelDefinitionManager::getSingletonPtr();
		if (modelDefMgr) {
			Model::XMLModelDefinitionSerializer serializer;

			auto modelDef = serializer.parseScript(stream, relativePath);
			if (modelDef) {
				auto existingDef = modelDefMgr->getByName(relativePath.string());
				//Model definition doesn't exist, just add it.
				if (!existingDef) {
					modelDefMgr->addDefinition(relativePath.string(), modelDef);
				} else {
					//otherwise update existing
					existingDef->moveFrom(std::move(*modelDef));
					existingDef->reloadAllInstances();
				}
			}
		}
	}
}

bool OgreResourceLoader::addSquallMedia(Squall::Signature signature, const std::string& resourceGroup) {

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(signature.str(),
																   "Squall",
																   resourceGroup, true);

	return true;
}

}
