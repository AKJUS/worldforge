//
// C++ Implementation: AssetsManager
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2007
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
#include "AssetsManager.h"
#include "../EmberOgrePrerequisites.h"
#include "../GUIManager.h"

#include "components/ogre/model/ModelDefinitionManager.h"
#include "components/ogre/model/ModelDefinition.h"

#include "framework/Tokeniser.h"

#include "CEGUIOgreRenderer/Renderer.h"
#include "CEGUIOgreRenderer/Texture.h"
#include <OgreMaterialSerializer.h>
#include <OgreTextureManager.h>
#include <OgreMeshSerializer.h>
#include <OgreMesh.h>

#include <CEGUI/Image.h>
#include <CEGUI/BasicImage.h>
#include <CEGUI/ImageManager.h>
#include <CEGUI/System.h>
#include <filesystem>
#include <MeshLodGenerator/OgreLodWorkQueueInjectorListener.h>
#include <MeshLodGenerator/OgreMeshLodGenerator.h>


namespace Ember::OgreView::Gui {

AssetsManager::AssetsManager() : mPMInjectorSignaler(std::make_unique<Lod::PMInjectorSignaler>()) {
	Ogre::MeshLodGenerator::getSingleton().setInjectorListener(mPMInjectorSignaler.get());
}

AssetsManager::~AssetsManager() {
	if (mPMInjectorSignaler.get() == Ogre::MeshLodGenerator::getSingleton().getInjectorListener()) {
		Ogre::MeshLodGenerator::getSingleton().setInjectorListener(nullptr);
	}
}

TexturePair AssetsManager::showTexture(const std::string& textureName) {
	// 	if (!mOgreCEGUITexture) {
	// 		logger->warn("You must first create a valid OgreCEGUITexture instance.");
	// 		return;
	// 	}
	if (Ogre::TextureManager::getSingleton().resourceExists(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)) {
		Ogre::TexturePtr texturePtr = static_cast<Ogre::TexturePtr>(Ogre::TextureManager::getSingleton().getByName(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME));
		if (texturePtr) {
			if (!texturePtr->isLoaded()) {
				try {
					texturePtr->load();
				} catch (...) {
					logger->warn("Error when loading {}. This texture will not be shown.", textureName);
					return {};
				}
			}
			std::string imageSetName(texturePtr->getName() + "_AssetsManager");

			return createTextureImage(texturePtr, imageSetName);
			// 			mOgreCEGUITexture->setOgreTexture(texturePtr);
		}
	}
	return {};

}

TexturePair AssetsManager::createTextureImage(Ogre::TexturePtr& texturePtr, const std::string& imageName) {
	// 	if (mOgreCEGUITexture) {
	// 		GUIManager::getSingleton().getGuiRenderer()->destroyTexture(mOgreCEGUITexture);
	// 		mOgreCEGUITexture = 0;
	// 	}

	auto renderer = CEGUI::System::getSingleton().getRenderer();

	CEGUI::Texture* ogreCEGUITexture;
	if (renderer->isTextureDefined(texturePtr->getName())) {
		ogreCEGUITexture = &renderer->getTexture(texturePtr->getName());
		dynamic_cast<CEGUI::OgreTexture*>(ogreCEGUITexture)->setOgreTexture(texturePtr);
	} else {
		//create a CEGUI texture from our Ogre texture
		logger->debug("Creating new CEGUI texture from Ogre texture.");
		ogreCEGUITexture = &GUIManager::getSingleton().createTexture(texturePtr);
	}

	//assign our image element to the StaticImage widget
	CEGUI::Image* textureImage;
	if (CEGUI::ImageManager::getSingleton().isDefined(imageName)) {
		textureImage = &CEGUI::ImageManager::getSingleton().get(imageName);
	} else {
		textureImage = &CEGUI::ImageManager::getSingleton().create("BasicImage", imageName);
	}

	auto* basicImage = dynamic_cast<CEGUI::BasicImage*>(textureImage);
	basicImage->setTexture(ogreCEGUITexture);
	auto area = CEGUI::Rectf(0, 0, ogreCEGUITexture->getSize().d_width, ogreCEGUITexture->getSize().d_height);
	basicImage->setArea(area);
	basicImage->setNativeResolution(area.getSize());
	basicImage->setAutoScaled(CEGUI::ASM_Both);

	return {texturePtr, textureImage};

}

std::string AssetsManager::materialAsText(const Ogre::MaterialPtr& material) {
	if (!material) {
		return "";
	}
	Ogre::MaterialSerializer serializer;
	material->compile();
	serializer.queueForExport(material, true, false);
	return serializer.getQueuedAsString();
}

std::string AssetsManager::resolveResourceNameFromFilePath(const std::string& filePath) {
	Ogre::ResourceGroupManager& manager = Ogre::ResourceGroupManager::getSingleton();

	auto groups = manager.getResourceGroups();
	for (auto& group: groups) {
		auto locations = manager.getResourceLocationList(group);
		for (auto& location: locations) {
			if (location.archive) {
				const std::string resourceLocation = location.archive->getName();
				if (Ogre::StringUtil::startsWith(filePath, resourceLocation, true)) {
					//Replace backwards slashes on windows with forwards slashes as that's what's used in the Ogre resource system.
					std::string localMeshPath = filePath.substr(resourceLocation.length(), std::string::npos);
					std::replace(localMeshPath.begin(), localMeshPath.end(), '\\', '/');
					return localMeshPath;
				}
			}
		}
	}

	return "";
}

std::string AssetsManager::resolveFilePathForMesh(const Ogre::MeshPtr& meshPtr) {
	Ogre::ResourceGroupManager& manager = Ogre::ResourceGroupManager::getSingleton();

	const auto& group = manager.findGroupContainingResource(meshPtr->getName());
	Ogre::FileInfoListPtr files = manager.findResourceFileInfo(group, meshPtr->getName(), false);
	if (files.get()) {
		for (const auto& fileInfo: *files) {
			if (fileInfo.archive && fileInfo.filename == meshPtr->getName()) {
				return fileInfo.archive->getName() + "/" + fileInfo.filename;
			}
		}
	}

	return "";
}

bool AssetsManager::exportMesh(const Ogre::MeshPtr& mesh, const std::filesystem::path& filePath) {
	if (!filePath.empty()) {
		Ogre::MeshSerializer serializer;
		try {
			if (!std::filesystem::exists(filePath.parent_path())) {
				std::filesystem::create_directories(filePath.parent_path());
			}
			serializer.exportMesh(mesh.get(), filePath.string());
			logger->info("Exported mesh {}", filePath.string());
		} catch (const Ogre::Exception& ex) {
			logger->error("Error when exporting mesh {}to path {}: {}", mesh->getName(), filePath.string(), ex.what());
			return false;
		}
		return true;
	}
	return false;
}

void AssetsManager::createModel(const Ogre::MeshPtr& mesh) {
	auto& modelDefinitionManager = Model::ModelDefinitionManager::getSingleton();
	std::string name = mesh->getName();
	if (name.empty()) {
		return;
	}
	//Extract the file name for the model. I.e. if the mesh has the full name "foo/bar.mesh" the model will be named "bar".
	auto tokens = Tokeniser::split(name, "/");
	std::string modelName = Tokeniser::split(tokens.back(), ".").front();
	auto modelDefinition = std::make_shared<Model::ModelDefinition>();
	Model::SubModelDefinition subModelDefinition{mesh->getName()};
	modelDefinition->addSubModelDefinition(subModelDefinition);
	modelDefinitionManager.exportScript(modelName, modelDefinition);

}


// bool AssetsManager::exportTexture(Ogre::TexturePtr texturePtr)
// {
//  getRenderTarget()->writeContentsToFile();
// }

}



