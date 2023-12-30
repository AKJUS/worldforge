//
// C++ Implementation: ModelDefinitionAtlasComposer
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2008
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
#include "ModelDefinitionAtlasComposer.h"
#include "Model.h"
#include "SubModel.h"

#include "../Convert.h"
#include "services/config/ConfigService.h"

#include <Atlas/MultiLineListFormatter.h>
#include <Atlas/Codecs/XML.h>
#include <Atlas/Message/MEncoder.h>
#include <Atlas/Message/QueuedDecoder.h>
#include <Ogre.h>
#include <wfmath/atlasconv.h>
#include <wfmath/stream.h>
#include <filesystem>

#ifdef _WIN32
#include "platform/platform_windows.h"
#endif

using namespace Atlas::Message;


namespace Ember::OgreView::Model {

Atlas::Message::MapType ModelDefinitionAtlasComposer::compose(Model* model, const std::string& typeName, const std::string& parentTypeName, float scale, const std::string& collisionType) {
	MapType mainMap;
	if (!model) {
		return mainMap;
	}
	MapType attributesMap;

	attributesMap["present"] = MapType{{"default", model->getDefinition()->getOrigin()}};

	//Don't supply bbox if the geometry is a mesh, since we'll get the bbox from the mesh in that case.
	if (collisionType != "mesh" && collisionType != "asset") {
		MapType bboxMap;

		Ogre::AxisAlignedBox aabb;
		for (auto& submodel: model->getSubmodels()) {
			const Ogre::Entity* entity = submodel->getEntity();
			aabb.merge(entity->getMesh()->getBounds());
		}

		if (scale != .0f && scale != 1.0f) {
			aabb.scale(Ogre::Vector3(scale, scale, scale));
		}
		WFMath::AxisBox<3> wfmathAabb = Convert::toWF(aabb);

		attributesMap["bbox"] = MapType{{"default", wfmathAabb.toAtlas()}};
	}

	attributesMap["geometry"] = MapType{{"default", composeGeometry(model, collisionType)}};

	mainMap["attributes"] = attributesMap;

	mainMap["objtype"] = StringType("class");
	mainMap["id"] = StringType(typeName);

	mainMap["parent"] = parentTypeName;

	return mainMap;
}

Atlas::Message::Element ModelDefinitionAtlasComposer::composeGeometry(Model* model, const std::string& collisionType) const {
	Atlas::Message::MapType geometryMap;
	geometryMap["type"] = collisionType;
	if (collisionType == "mesh") {

		std::vector<Atlas::Message::Element> vertices;
		std::vector<Atlas::Message::Element> indices;

		for (auto& submodel: model->getSubmodels()) {
			auto mesh = submodel->getEntity()->getMesh();
			if (mesh->sharedVertexData) {
				copyVertexData(vertices, *mesh->sharedVertexData);
			}
			for (size_t i = 0; i < mesh->getNumSubMeshes(); ++i) {
				Ogre::SubMesh* submesh = mesh->getSubMesh(i);
				size_t offset = 0;
				if (!submesh->useSharedVertices) {
					offset = vertices.size() / 3;
					copyVertexData(vertices, *submesh->vertexData);
				}

				auto indexBuffer = submesh->indexData->indexBuffer;
				Ogre::HardwareIndexBuffer::IndexType idxType = submesh->indexData->indexBuffer->getType();


				if (idxType == Ogre::HardwareIndexBuffer::IT_16BIT) {
					auto pIndex = static_cast<Ogre::uint16*>(indexBuffer->lock(submesh->indexData->indexStart * sizeof(Ogre::uint16),
																			   submesh->indexData->indexCount * sizeof(Ogre::uint16),
																			   Ogre::HardwareBuffer::HBL_READ_ONLY));
					for (size_t j = 0; j < submesh->indexData->indexCount; ++j) {
						auto index = static_cast<IntType>(pIndex[j] + offset);
						indices.emplace_back(index);
					}
				} else {
					auto pIndex = static_cast<Ogre::uint32*>(indexBuffer->lock(submesh->indexData->indexStart * sizeof(Ogre::uint32),
																			   submesh->indexData->indexCount * sizeof(Ogre::uint32),
																			   Ogre::HardwareBuffer::HBL_READ_ONLY));
					for (size_t j = 0; j < submesh->indexData->indexCount; ++j) {
						auto index = static_cast<IntType>(pIndex[j] + offset);
						indices.emplace_back(index);
					}
				}

				indexBuffer->unlock();
			}
		}
		logger->debug("Created mesh data with {} vertex element and {} index elements.", vertices.size(), indices.size());
		geometryMap["vertices"] = std::move(vertices);
		geometryMap["indices"] = std::move(indices);
	} else if (collisionType == "asset") {
		geometryMap["path"] = model->getSubModel(0)->getEntity()->getMesh()->getName();
	}
	return geometryMap;
}


void ModelDefinitionAtlasComposer::copyVertexData(std::vector<Atlas::Message::Element>& vertices, Ogre::VertexData& vertexData) {


	// Locate position element and the buffer to go with it.
	const Ogre::VertexElement* posElem = vertexData.vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
	Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData.vertexBufferBinding->getBuffer(posElem->getSource());
	size_t vertexSize = vbuf->getVertexSize();

	// Lock the buffer for reading.
	auto pVertex = static_cast<Ogre::uint8*>(vbuf->lock(vertexData.vertexStart * vertexSize,
														vertexData.vertexCount * vertexSize,
														Ogre::HardwareBuffer::HBL_READ_ONLY));
	size_t numEntries = vertexData.vertexCount * 3;
	vertices.reserve(vertices.size() + numEntries);

	logger->debug("Copying {} vertex elements.", numEntries);
	for (size_t i = 0; i < vertexData.vertexCount; ++i) {
		float* pFloat;
		posElem->baseVertexPointerToElement(pVertex, &pFloat);
		vertices.emplace_back(pFloat[0]);
		vertices.emplace_back(pFloat[1]);
		vertices.emplace_back(pFloat[2]);
		pVertex += vertexSize;
	}
	vbuf->unlock();
}

void
ModelDefinitionAtlasComposer::composeToStream(std::iostream& outstream, Model* model, const std::string& typeName, const std::string& parentTypeName, float scale, const std::string& collisionType) {
	Atlas::Message::QueuedDecoder decoder;

	Atlas::Codecs::XML codec(outstream, outstream, decoder);
	Atlas::MultiLineListFormatter formatter(outstream, codec);
	Atlas::Message::Encoder encoder(formatter);
	formatter.streamBegin();
	encoder.streamMessageElement(compose(model, typeName, parentTypeName, scale, collisionType));

	formatter.streamEnd();
}

std::string ModelDefinitionAtlasComposer::composeToFile(Model* model, const std::string& typeName, const std::string& parentTypeName, float scale, const std::string& collisionType) {
	if (model) {
		try {

			auto cleanedTypename = std::filesystem::path(typeName).filename().stem();
			auto cleanedDir = std::filesystem::path(typeName).remove_filename();

			//make sure the directory exists
			auto filename = ConfigService::getSingleton().getHomeDirectory(BaseDirType_DATA) / "typeexport" / cleanedDir / (cleanedTypename.string() + ".xml");
			auto dir = filename;
			dir.remove_filename();

			if (!std::filesystem::exists(dir)) {
				logger->info("Creating directory {}", dir.string());
				std::filesystem::create_directories(dir);
			}


			std::fstream exportFile(filename.c_str(), std::fstream::out);

			logger->info("Creating atlas type {}", filename.string());
			composeToStream(exportFile, model, cleanedTypename.string(), parentTypeName, scale, collisionType);
			exportFile.close();
			return filename.string();
		} catch (const std::exception& e) {
			logger->warn("Error when exporting Model to Atlas data: {}", e.what());
		}
	}
	return "";

}

}



