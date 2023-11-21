/*
 -------------------------------------------------------------------------------
 This source file is part of Cataclysmos
 For the latest info, see http://www.cataclysmos.org/

 Copyright (c) 2005 The Cataclysmos Team
 Copyright (C) 2005  Erik Ogenvik

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 -------------------------------------------------------------------------------
 */
#include "SubModelPart.h"

#include "ModelDefinition.h"
#include "Model.h"
#include "SubModel.h"

#include <OgreSubEntity.h>
#include <OgreSubMesh.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include <OgreMesh.h>
#include <OgreSceneManager.h>
#include <OgreInstanceManager.h>
#include <OgreInstancedEntity.h>
#include <OgreTechnique.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreInstanceBatch.h>
#include <boost/algorithm/string.hpp>
#include <utility>

namespace Ember {
namespace OgreView {
namespace Model {

SubModelPart::SubModelPart(std::string name, SubModel& subModel) :
		mName(std::move(name)), mSubModel(subModel) {
}

//no need to try to delete the Ogre::Subentities in the mSubEntities store, since Ogre will take care of this
SubModelPart::~SubModelPart() = default;

void SubModelPart::addSubEntity(SubModelPartEntity subModelPartEntity) {
	mSubEntities.emplace_back(std::move(subModelPartEntity));
}

bool SubModelPart::addSubEntity(Ogre::SubEntity* subentity, SubEntityDefinition definition, unsigned short subEntityIndex) {
	mSubEntities.emplace_back(SubModelPartEntity{subentity, std::move(definition), subEntityIndex});
	return true;
}

bool SubModelPart::removeSubEntity(const Ogre::SubEntity* subentity) {
	for (auto I = mSubEntities.begin(); I != mSubEntities.end(); ++I) {
		if (I->SubEntity == subentity) {
			mSubEntities.erase(I);
			return true;
		}
	}
	return false;
}

const std::string& SubModelPart::getName() const {
	return mName;
}

void SubModelPart::show() {
	showSubEntities();
	if (mSubModel.mModel.mUseInstancing) {
		createInstancedEntities();
	}
}

void SubModelPart::showSubEntities() {
	for (auto& subModelPartEntity: mSubEntities) {
		std::string materialName;
		if (subModelPartEntity.Definition && !subModelPartEntity.Definition->materialName.empty()) {
			materialName = subModelPartEntity.Definition->materialName;
		} else {
			//if no material name is set in the ModelDefinition, use the default one from the mesh
			materialName = subModelPartEntity.SubEntity->getSubMesh()->getMaterialName();
		}

		if (!materialName.empty()) {
			if (mSubModel.mEntity.hasSkeleton()) {

				//We first need to check the number of bones to use
				const Ogre::VertexElement* blendWeightsData;
				if (subModelPartEntity.SubEntity->getSubMesh()->useSharedVertices) {
					blendWeightsData = subModelPartEntity.SubEntity->getSubMesh()->parent->sharedVertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_BLEND_WEIGHTS);
				} else {
					blendWeightsData = subModelPartEntity.SubEntity->getSubMesh()->vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_BLEND_WEIGHTS);
				}

				unsigned short numWeightsPerVertex = 0;
				if (blendWeightsData) {
					numWeightsPerVertex = Ogre::VertexElement::getTypeCount(blendWeightsData->getType());
				}

				//The number suffix denotes the number of bones to use.
				std::string skinningSuffix = "/Skinning/" + std::to_string(numWeightsPerVertex);
				//Check if we can use Hardware Skinning material.
				//This is done by checking if a material by the same name, but with the "/Skinning/*" suffix is available.
				//If not, we try to create such a material by cloning the original and replacing the vertex shader with
				//one with the same suffix (if available and supported).
				if (!boost::algorithm::ends_with(materialName, skinningSuffix)) {

					std::string newMaterialName = materialName + skinningSuffix;
					auto& materialMgr = Ogre::MaterialManager::getSingleton();
					if (!materialMgr.resourceExists(newMaterialName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)) {
						//Material does not exist; lets create it
						auto material = materialMgr.getByName(materialName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
						if (material) {
							material->load();
							auto newMaterial = material->clone(newMaterialName);
							for (auto tech: newMaterial->getTechniques()) {
								if (!tech->getPasses().empty()) {
									auto pass = tech->getPass(0);
									if (pass->hasVertexProgram()) {
										std::string newVertexProgramName = pass->getVertexProgramName() + skinningSuffix;
										auto program = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(newVertexProgramName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
										if (program) {
											program->load();
											if (program->isSupported()) {
												pass->setVertexProgram(newVertexProgramName);
											}
										}
									}

									auto shadowCasterMat = tech->getShadowCasterMaterial();
									if (shadowCasterMat && !boost::algorithm::ends_with(shadowCasterMat->getName(), skinningSuffix)) {
										std::string skinningShadowCasterMatName = shadowCasterMat->getName() + skinningSuffix;
										auto shadowCasterMatSkinning = materialMgr.getByName(skinningShadowCasterMatName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
										if (!shadowCasterMatSkinning) {
											shadowCasterMat->load();
											shadowCasterMatSkinning = shadowCasterMat->clone(skinningShadowCasterMatName);
											for (auto* shadowCasterTech: shadowCasterMatSkinning->getTechniques()) {
												auto shadowCasterPass = shadowCasterTech->getPass(0);
												if (shadowCasterPass->hasVertexProgram()) {
													std::string vertexProgramName = shadowCasterPass->getVertexProgram()->getName() + skinningSuffix;
													auto program = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(vertexProgramName,
																															  Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
													if (program) {
														program->load();
														if (program->isSupported()) {
															shadowCasterPass->setVertexProgram(vertexProgramName);
														}
													}
												}
											}
										}
										shadowCasterMatSkinning->load();
										tech->setShadowCasterMaterial(shadowCasterMatSkinning);
									}
								}
							}
						}
					}
					materialName = newMaterialName;
				}
			}


			if (materialName != subModelPartEntity.SubEntity->getMaterialName()) {
				//TODO: store the material ptr in the definition so we'll avoid a lookup in setMaterialName
				subModelPartEntity.SubEntity->setMaterialName(materialName);
			}
		}
		subModelPartEntity.SubEntity->setVisible(true);
	}
}

bool SubModelPart::createInstancedEntities() {

	try {
		std::vector<std::pair<Ogre::InstanceManager*, std::string>> managersAndMaterials;

		static std::string instancedSuffix = "/Instanced/HW";
		static Ogre::InstanceManager::InstancingTechnique instancedTechnique = Ogre::InstanceManager::HWInstancingBasic;
		auto resourceGroup = mSubModel.mEntity.getMesh()->getGroup();

		for (auto& entry: mSubEntities) {
			Ogre::SubEntity* subEntity = entry.SubEntity;
			Ogre::Entity* entity = subEntity->getParent();
			Ogre::SceneManager* sceneManager = entity->_getManager();
			std::string instanceName = entity->getMesh()->getName() + "/" + std::to_string(entry.subEntityIndex);


			auto subMeshInstanceSuffix = instancedSuffix;
			std::string materialName;
			if (entry.Definition && !entry.Definition->materialName.empty()) {
				materialName = entry.Definition->materialName;
			} else {
				//if no material name is set in the ModelDefinition, use the default one from the mesh
				materialName = entry.SubEntity->getSubMesh()->getMaterialName();
			}
			auto subMesh = subEntity->getSubMesh();
			if (subMesh->useSharedVertices) {
				subMeshInstanceSuffix += "/" + std::to_string(subMesh->parent->sharedVertexData->vertexDeclaration->getNextFreeTextureCoordinate());
			} else {
				subMeshInstanceSuffix += "/" + std::to_string(subMesh->vertexData->vertexDeclaration->getNextFreeTextureCoordinate());
			}

			std::string instancedMaterialName = materialName;

			//Check if the material is "instanced", i.e. has the suffix as specified in "subMeshInstanceSuffix".
			//If not, we'll create a new material by cloning the original and replacing the vertex shader with
			//one with the correct suffix, if such one is available and supported.
			if (!boost::algorithm::ends_with(materialName, subMeshInstanceSuffix)) {

				instancedMaterialName += subMeshInstanceSuffix;
				auto& materialMgr = Ogre::MaterialManager::getSingleton();


				if (!materialMgr.resourceExists(instancedMaterialName, resourceGroup)) {
					auto originalMaterial = materialMgr.getByName(materialName, resourceGroup);
					if (originalMaterial) {
						originalMaterial->load();
						auto material = originalMaterial->clone(instancedMaterialName);
						material->load();
						for (auto* tech: material->getTechniques()) {
							auto pass = tech->getPass(0);
							if (pass->hasVertexProgram()) {
								std::string vertexProgramName = pass->getVertexProgram()->getName() + subMeshInstanceSuffix;
								if (Ogre::HighLevelGpuProgramManager::getSingleton().resourceExists(vertexProgramName, resourceGroup)) {
									pass->setVertexProgram(vertexProgramName);
								} else {
									logger->warn("The model '{}' is set to use instancing, but the required vertex program '{}' couldn't be found.",
												 mSubModel.mModel.getName(),
												 vertexProgramName);
								}
							}

							auto shadowCasterMat = tech->getShadowCasterMaterial();
							if (shadowCasterMat && !boost::algorithm::ends_with(shadowCasterMat->getName(), subMeshInstanceSuffix)) {
								std::string instancedShadowCasterMatName = shadowCasterMat->getName() + subMeshInstanceSuffix;
								auto shadowCasterMatInstanced = materialMgr.getByName(instancedShadowCasterMatName, resourceGroup);
								if (!shadowCasterMatInstanced) {
									shadowCasterMat->load();
									shadowCasterMatInstanced = shadowCasterMat->clone(instancedShadowCasterMatName);
									for (auto* shadowCasterTech: shadowCasterMatInstanced->getTechniques()) {
										auto shadowCasterPass = shadowCasterTech->getPass(0);
										if (shadowCasterPass->hasVertexProgram()) {
											std::string vertexProgramName = shadowCasterPass->getVertexProgram()->getName() + subMeshInstanceSuffix;
											if (Ogre::HighLevelGpuProgramManager::getSingleton().resourceExists(vertexProgramName, resourceGroup)) {
												shadowCasterPass->setVertexProgram(vertexProgramName);
											} else {
												logger->warn("The model '{}' is set to use instancing, but the required shadow caster vertex program '{}' couldn't be found.",
															 mSubModel.mModel.getName(),
															 vertexProgramName);
											}
										}
									}
								}
								shadowCasterMatInstanced->load();
								tech->setShadowCasterMaterial(shadowCasterMatInstanced);
							}
						}
					} else {
						logger->warn("The material '{}' used by a submesh of the mesh '{}' could not be found. The submesh will be hidden.",
									 materialName,
									 entity->getMesh()->getName());
						continue;
					}
				}
			}

			if (sceneManager->hasInstanceManager(instanceName)) {
				managersAndMaterials.emplace_back(std::make_pair(sceneManager->getInstanceManager(instanceName), instancedMaterialName));
			} else {
				auto bestTech = subEntity->getMaterial()->getBestTechnique();
				if (!bestTech->getPasses().empty() && bestTech->getPass(0)->hasVertexProgram()) {
					auto& meshName = entity->getMesh()->getName();
					auto instancedMeshName = meshName + "/Instanced";
					//Use a copy of the original mesh, since the InstanceManager in its current iteration performs alterations to the original mesh.
					auto meshCopy = Ogre::MeshManager::getSingleton().getByName(instancedMeshName, resourceGroup);
					if (!meshCopy || !meshCopy->isLoaded()) {
						if (meshCopy) {
							//If the mesh existed but was unloaded we'll remove the unloaded version and just make a copy of the original again.
							//If we don't do this the instance manager will segfault later on since it's referring to submeshes that don't exist.
							Ogre::MeshManager::getSingleton().remove(meshCopy);
						}
						entity->getMesh()->clone(instancedMeshName);
					}

					try {
						Ogre::InstanceManager* instanceManager = sceneManager->createInstanceManager(instanceName,
																									 instancedMeshName,
																									 entity->getMesh()->getGroup(),
																									 instancedTechnique,
																									 50, Ogre::IM_USEALL, entry.subEntityIndex);
						instanceManager->setBatchesAsStaticAndUpdate(true);

						managersAndMaterials.emplace_back(std::make_pair(instanceManager, instancedMaterialName));
					} catch (const std::exception& e) {
						logger->error("Could not create instanced versions of mesh {} (as {}): {}", meshName, instancedMeshName, e.what());
						throw;
					}

				} else {
					//Could not make into instanced.
					logger->warn("Could not create instanced version of subentity with index {} of entity {}", entry.subEntityIndex, entity->getName());
				}
			}
		}

		for (auto& entry: managersAndMaterials) {
			try {
				auto instancedEntity = entry.first->createInstancedEntity(entry.second);
				if (instancedEntity) {
					mInstancedEntities.push_back(instancedEntity);
					mSubModel.mModel.addMovable(instancedEntity);
					::Ember::OgreView::Model::Model::sInstancedEntities[entry.first->getSceneManager()][instancedEntity] = &mSubModel.mModel;
				} else {
					logger->error("Could not create instanced entity {}", entry.first->getName());
				}
			} catch (const std::exception& ex) {
				logger->error("Could not create instanced entity {}: {}", entry.first->getName(), ex.what());
			}
		}
		return true;
	} catch (const std::exception& e) {
		logger->error("Error when trying to create instanced mesh for {}: {}", mName, e.what());
		return false;
	}
}


void SubModelPart::hide() {
	if (!mInstancedEntities.empty()) {
		for (auto& item: mInstancedEntities) {
			item->setVisible(false);
		}
	} else {
		for (auto& item: mSubEntities) {
			item.SubEntity->setVisible(false);
		}
	}

}

const std::vector<SubModelPartEntity>& SubModelPart::getSubentities() const {
	return mSubEntities;
}

void SubModelPart::destroy() {
	for (auto& item: mInstancedEntities) {
		//There's a bug where the InstancedEntity doesn't contain a pointer to it's scene manager; we need to go through the InstanceBatch instead
		::Ember::OgreView::Model::Model::sInstancedEntities[item->_getOwner()->_getManager()].erase(item);
		mSubModel.mModel.removeMovable(item);
		mSubModel.mEntity._getManager()->destroyInstancedEntity(item);
	}
}

}
}
}
