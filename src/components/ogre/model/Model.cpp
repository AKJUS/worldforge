/*
 Copyright (C) 2004  Erik Ogenvik
 Copyright (c) 2005 The Cataclysmos Team

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Model.h"
#include "SubModel.h"
#include "SubModelPart.h"
#include "ParticleSystemBinding.h"

#include "ModelDefinitionManager.h"


#include "framework/TimeFrame.h"
#include "framework/TimedLog.h"

#include <OgreTagPoint.h>
#include <OgreMeshManager.h>
#include <OgreMesh.h>
#include <OgreSceneManager.h>
#include <OgreSubEntity.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreSkeletonInstance.h>
#include <OgreMaterialManager.h>
#include <OgreInstancedEntity.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreInstanceBatch.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <components/ogre/OgreInfo.h>


namespace Ember {
namespace OgreView {
namespace Model {

std::map<Ogre::SceneManager*, std::map<Ogre::InstancedEntity*, Model*>> Model::sInstancedEntities;


Model::Model(Ogre::SceneManager& manager, const ModelDefinitionPtr& definition, const std::string& name) :
		mManager(manager),
		mDefinition(definition),
		mParentNodeProvider(nullptr),
		mName(name),
		mSkeletonOwnerEntity(nullptr),
		mSkeletonInstance(nullptr),
		mRotation(Ogre::Quaternion::IDENTITY),
		mAnimationStateSet(nullptr),
		mAttachPoints(nullptr),
		mVisible(true),
		mRenderingDistance(0),
		mQueryFlags(0),
		mLoaded(false),
		mUseInstancing(definition->mUseInstancing) {
	definition->addModelInstance(this);
}

Model::~Model() {

	mDefinition->removeModelInstance(this);
	for (auto& submodel : mSubmodels) {
		removeMovable(submodel->getEntity());
		delete submodel;
	}

	//Lights are not parts of mMovableObjects, so we need to destroy them ourselves here.
	resetLights();

	for (auto& movable : mMovableObjects) {
		mManager.destroyMovableObject(movable);
	}
	mDefinition->removeFromLoadingQueue(this);

	//Clean up any submodels not yet added.
	for (auto submodel : mAssetCreationContext.mSubmodels) {
		delete submodel;
	}


//	S_LOG_VERBOSE("Deleted "<< getName());
}

void Model::reset() {
//	S_LOG_VERBOSE("Resetting "<< getName());
	Resetting.emit();
	//	resetAnimations();
	resetSubmodels();
	resetParticles();
	resetLights();
	mRotation = Ogre::Quaternion::IDENTITY;
	mSkeletonInstance = nullptr;
	// , mAnimationStateSet(0)
	mSkeletonOwnerEntity = nullptr;
	mAttachPoints.reset();

}

const ModelDefinitionPtr& Model::getDefinition() const {
	return mDefinition;
}

bool Model::load() {
	return mDefinition->requestLoad(this);
}

bool Model::reload() {
	return load();
}

bool Model::loadAssets() {
	if (mAssetCreationContext.mCurrentlyLoadingSubModelIndex == 0) {
		reset();
	}
	bool result = createModelAssets();
	//if we are attached, we have to notify the new entities, else they won't appear in the scene
	//_notifyAttached(mParentNode, mParentIsTagPoint);

	if (result) {
		Reloaded.emit();
	}
	return result;
}


//void Model::loadingComplete(Ogre::Resource*) {
//	//This is called when the mesh is reloaded and it has a skeleton; we need to reset the actions since they now refer to invalid animation states
//
//	for (auto& subModel : mSubmodels) {
//		//We need to call _initialise in order for the animation states to get recreated;
//		//else this will happen lazily the next time the entity is rendered.
//		subModel->getEntity()->_initialise(true);
//	}
//
//	for (auto& actionDef : mDefinition->getActionDefinitions()) {
//		auto actionI = mActions.find(actionDef->getName());
//		if (actionI != mActions.end()) {
//			Action& action = actionI->second;
//			//Important these calls happen in this order, else we'll risk segfaults
//			action.getAnimations().getAnimations().clear();
//			action.getAnimations().reset();
//
//			for (auto& animationDef : actionDef->getAnimationDefinitions()) {
//				Animation animation(animationDef->getIterations(), getSkeleton()->getNumBones());
//				for (auto& animationPartDef : animationDef->getAnimationPartDefinitions()) {
//					if (getAllAnimationStates()->hasAnimationState(animationPartDef->Name)) {
//						AnimationPart animPart;
//						try {
//							Ogre::AnimationState* state = getAnimationState(animationPartDef->Name);
//							animPart.state = state;
//							for (auto& boneGroupDef : animationPartDef->BoneGroupRefs) {
//								auto I_boneGroup = mDefinition->getBoneGroupDefinitions().find(boneGroupDef.Name);
//								if (I_boneGroup != mDefinition->getBoneGroupDefinitions().end()) {
//									BoneGroupRef boneGroupRef;
//									boneGroupRef.boneGroupDefinition = I_boneGroup->second;
//									boneGroupRef.weight = boneGroupDef.Weight;
//									animPart.boneGroupRefs.push_back(boneGroupRef);
//								}
//							}
//							animation.addAnimationPart(animPart);
//						} catch (const std::exception& ex) {
//							S_LOG_FAILURE("Error when loading animation: " << animationPartDef->Name << "." << ex);
//						}
//					}
//				}
//				action.getAnimations().addAnimation(animation);
//			}
//		}
//	}
//}

bool Model::createModelAssets() {
	TimedLog timedLog("Model::createActualModel " + mDefinition->getOrigin());

	if (mAssetCreationContext.mCurrentlyLoadingSubModelIndex < mDefinition->getSubModelDefinitions().size()) {
		auto& submodelDef = mDefinition->getSubModelDefinitions()[mAssetCreationContext.mCurrentlyLoadingSubModelIndex];
		try {

			auto mesh = Ogre::MeshManager::getSingleton().getByName(submodelDef->getMeshName(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			if (mesh) {
				mesh->load();

				//Instancing (currently) doesn't support HW skeletons, so we'll have to disable for any meshes with skeletons.
				if (mesh->hasSkeleton()) {
					mUseInstancing = false;
				}

				Ogre::Entity* entity;
				if (!mName.empty()) {
					entity = mManager.createEntity(mName + "/" + submodelDef->getMeshName(), mesh);
				} else {
					entity = mManager.createEntity(mesh);
				}
				entity->setCastShadows(submodelDef->mShadowCaster);
				timedLog.report("Created entity '" + entity->getName() + "' of mesh '" + mesh->getName() + "'.");


				SubModel* submodel = new SubModel(*entity, *this);
				//Model::SubModelPartMapping* submodelPartMapping = new Model::SubModelPartMapping();


				if (!submodelDef->getPartDefinitions().empty()) {
					for (auto& partDef : submodelDef->getPartDefinitions()) {
						SubModelPart& part = submodel->createSubModelPart(partDef->getName());
						//std::string groupName("");

						if (!partDef->getSubEntityDefinitions().empty()) {
							for (auto& subEntityDef : partDef->getSubEntityDefinitions()) {
								try {
									Ogre::SubEntity* subEntity(nullptr);
									size_t subEntityIndex;
									//try with a submodelname first
									if (!subEntityDef->getSubEntityName().empty()) {
										subEntityIndex = entity->getMesh()->_getSubMeshIndex(subEntityDef->getSubEntityName());

										subEntity = entity->getSubEntity(subEntityIndex);
									} else {
										//no name specified, use the index instead
										if (entity->getNumSubEntities() > subEntityDef->getSubEntityIndex()) {
											subEntityIndex = subEntityDef->getSubEntityIndex();
											subEntity = entity->getSubEntity(subEntityIndex);
										} else {
											S_LOG_WARNING("Model definition " << mDefinition->getOrigin() << " has a reference to entity with index " << subEntityDef->getSubEntityIndex() << " which is out of bounds.");
										}
									}
									if (subEntity) {
										part.addSubEntity(subEntity, subEntityDef, subEntityIndex);

										if (!subEntityDef->getMaterialName().empty()) {
											subEntity->setMaterialName(subEntityDef->getMaterialName());
										}
									} else {
										S_LOG_WARNING("Could not add subentity.");
									}
								} catch (const std::exception& ex) {
									S_LOG_WARNING("Error when getting sub entities for model '" << mDefinition->getOrigin() << "'." << ex);
								}
							}
						} else {
							//if no subentities are defined, add all subentities
							size_t numSubEntities = entity->getNumSubEntities();
							for (size_t i = 0; i < numSubEntities; ++i) {
								part.addSubEntity(entity->getSubEntity(i), nullptr, i);
							}
						}
						if (!partDef->getGroup().empty()) {
							mAssetCreationContext.mGroupsToPartMap[partDef->getGroup()].push_back(partDef->getName());
							//mPartToGroupMap[partDef->getName()] = partDef->getGroup();
						}

						if (partDef->getShow()) {
							mAssetCreationContext.showPartVector.push_back(partDef->getName());
						}

						ModelPart& modelPart = mAssetCreationContext.mModelParts[partDef->getName()];
						modelPart.addSubModelPart(&part);
						modelPart.setGroupName(partDef->getGroup());
					}
				} else {
					//if no parts are defined, add a default "main" part and add all subentities to it. This ought to be a good default behaviour
					SubModelPart& part = submodel->createSubModelPart("main");
					for (size_t i = 0; i < entity->getNumSubEntities(); ++i) {

						Ogre::SubEntity* subentity = entity->getSubEntity(i);
						part.addSubEntity(subentity, nullptr, i);
					}
					mAssetCreationContext.showPartVector.push_back(part.getName());
					ModelPart& modelPart = mAssetCreationContext.mModelParts[part.getName()];
					modelPart.addSubModelPart(&part);
				}
				mAssetCreationContext.mSubmodels.insert(submodel);
				timedLog.report("Created submodel.");

			} else {
				S_LOG_FAILURE("Could not load mesh " << submodelDef->getMeshName() << " which belongs to model " << mDefinition->getOrigin() << ".");
			}


		} catch (const std::exception& e) {
			S_LOG_FAILURE("Submodel load error for mesh '" << submodelDef->getMeshName() << "'." << e);
		}
		mAssetCreationContext.mCurrentlyLoadingSubModelIndex++;
		return false;
	}


	setRenderingDistance(mDefinition->getRenderingDistance());

	for (auto submodel : mAssetCreationContext.mSubmodels) {
		addSubmodel(submodel);
	}
	mModelParts = mAssetCreationContext.mModelParts;
	mGroupsToPartMap = mAssetCreationContext.mGroupsToPartMap;

	createActions();
	timedLog.report("Created actions.");

	createParticles();
	timedLog.report("Created particles.");

	createLights();
	timedLog.report("Created lights.");

	mRotation = mDefinition->mRotation;
	for (auto& part : mAssetCreationContext.showPartVector) {
		showPart(part);
	}
	mLoaded = true;
	mAssetCreationContext = AssetCreationContext();
	return true;
}

void Model::createActions() {

//	//If the mesh has a skeleton we'll add a listener to the mesh, so that we can reload the animation states when
//	//the mesh or skeleton gets reloaded.
//	if (getSkeleton()) {
//		mSkeletonOwnerEntity->getMesh()->addListener(this);
//	}

	for (auto& actionDef : mDefinition->getActionDefinitions()) {
		Action action;
		action.mActivations = actionDef->getActivationDefinitions();
		action.setName(actionDef->getName());
		action.getAnimations().setSpeed(actionDef->getAnimationSpeed());

		if (getSkeleton() && getAllAnimationStates()) {

			if (!mDefinition->getBoneGroupDefinitions().empty()) {
				//If there are bone groups, we need to use a cumulative blend mode. Note that this will affect all animations in the model.
				getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
			}
			if (!mSubmodels.empty()) {
				for (auto& animationDef : actionDef->getAnimationDefinitions()) {
					Animation animation(animationDef->getIterations(), getSkeleton()->getNumBones());
					for (auto& animationPartDef : animationDef->getAnimationPartDefinitions()) {
						if (getAllAnimationStates()->hasAnimationState(animationPartDef->Name)) {
							AnimationPart animPart;
							try {
								Ogre::AnimationState* state = getAnimationState(animationPartDef->Name);
								animPart.state = state;
								for (auto& boneGroupDef : animationPartDef->BoneGroupRefs) {
									auto I_boneGroup = mDefinition->getBoneGroupDefinitions().find(boneGroupDef.Name);
									if (I_boneGroup != mDefinition->getBoneGroupDefinitions().end()) {
										BoneGroupRef boneGroupRef{};
										boneGroupRef.boneGroupDefinition = I_boneGroup->second;
										boneGroupRef.weight = boneGroupDef.Weight;
										animPart.boneGroupRefs.push_back(boneGroupRef);
									}
								}
								animation.addAnimationPart(animPart);
							} catch (const std::exception& ex) {
								S_LOG_FAILURE("Error when loading animation: " << animationPartDef->Name << "." << ex);
							}
						}
					}
					action.getAnimations().addAnimation(animation);
				}
			}
		}

		//TODO: add sounds too

		mActions[actionDef->getName()] = action;
	}
}

void Model::createParticles() {
	for (auto& particleSystemDef : mDefinition->mParticleSystems) {
		//first try to create the ogre particle system
		Ogre::ParticleSystem* ogreParticleSystem;

		try {
			if (!mName.empty()) {
				std::string name(mName + "/particle" + particleSystemDef.Script);
				ogreParticleSystem = mManager.createParticleSystem(name, particleSystemDef.Script);
			} else {
				ogreParticleSystem = mManager.createParticleSystem(OgreInfo::createUniqueResourceName("particleSystem"),
																   particleSystemDef.Script);
			}
			if (ogreParticleSystem) {
				//Try to trigger a load of any image resources used by affectors.
				//The reason we want to do this now is that otherwise it will happen during rendering. An exception will then be thrown
				//which will bubble all the way up to the main loop, thus aborting all frames.
				ogreParticleSystem->_update(0);

				//ogreParticleSystem->setDefaultDimensions(1, 1);
				auto particleSystem = new ParticleSystem(ogreParticleSystem, particleSystemDef.Direction);
				for (auto& bindingDef : particleSystemDef.Bindings) {
					ParticleSystemBinding* binding = particleSystem->addBinding(bindingDef.EmitterVar, bindingDef.AtlasAttribute);
					mAllParticleSystemBindings.push_back(binding);
				}
				mParticleSystems.push_back(particleSystem);


				//Check if the material used is transparent. If so, assign it a later render queue.
				//This is done to make transparent particle systems play better with the foliage and the water.
				//The foliage would be rendered at an earlier render queue (RENDER_QUEUE_6 normally) and the water at RENDER_QUEUE_8.
				//This of course means that there's still an issue when the camera is below the water
				//(as the water, being rendered first, will prevent the particles from being rendered). That will need to be solved.
				Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(ogreParticleSystem->getMaterialName());
				if (materialPtr) {
					if (materialPtr->isTransparent()) {
						ogreParticleSystem->setRenderQueueGroup(Ogre::RENDER_QUEUE_9);
					}
				}

				addMovable(ogreParticleSystem);
			}
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Could not create particle system: " << particleSystemDef.Script << "." << ex);
		}
	}
}

void Model::createLights() {
	int j = 0;
	for (auto& lightDef : mDefinition->mLights) {
		//first try to create the ogre lights
		LightInfo lightInfo{};
		Ogre::Light* ogreLight;
		try {
			if (!mName.empty()) {
				std::stringstream name;
				name << mName << "/light" << (j++);
				ogreLight = mManager.createLight(name.str());
			} else {
				ogreLight = mManager.createLight();
			}
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Could not create light." << ex);
			continue;
		}
		if (ogreLight) {
			ogreLight->setType(Ogre::Light::LT_POINT);
			ogreLight->setDiffuseColour(lightDef.diffuseColour);
			ogreLight->setSpecularColour(lightDef.specularColour);
			ogreLight->setAttenuation(lightDef.range, lightDef.constant, lightDef.linear, lightDef.quadratic);

			lightInfo.light = ogreLight;
			lightInfo.position = lightDef.position;
			addLight(std::move(lightInfo));
		}
	}
}

void Model::addLight(LightInfo lightInfo)
{

    if (mParentNodeProvider) {
        //If the light has a position we need to create a different node to attach it to.
        if (!lightInfo.position.isNaN() && !lightInfo.position.isZeroLength()) {
            lightInfo.nodeProvider = mParentNodeProvider->createChildProvider("");
            lightInfo.nodeProvider->setPositionAndOrientation(lightInfo.position, Ogre::Quaternion::IDENTITY);
            lightInfo.nodeProvider->attachObject(lightInfo.light);
        } else {
            mParentNodeProvider->attachObject(lightInfo.light);
        }
    }
    mLights.emplace_back(std::move(lightInfo));

}

bool Model::hasParticles() const {
	return !mParticleSystems.empty();
}

const ParticleSystemBindingsPtrSet& Model::getAllParticleSystemBindings() const {
	return mAllParticleSystemBindings;
}

ParticleSystemSet& Model::getParticleSystems() {
	return mParticleSystems;
}

std::vector<LightInfo>& Model::getLights() {
	return mLights;
}

bool Model::addSubmodel(SubModel* submodel) {
	auto entity = submodel->getEntity();

	entity->getUserObjectBindings().setUserAny("model", Ogre::Any(this));
	//if the submodel has a skeleton, check if it should be shared with existing models
	if (entity->hasSkeleton()) {
		if (mSkeletonOwnerEntity != nullptr) {
			entity->shareSkeletonInstanceWith(mSkeletonOwnerEntity);
		} else {
			mSkeletonOwnerEntity = entity;
			// 			mAnimationStateSet = submodel->getEntity()->getAllAnimationStates();
		}
	}
	auto result = mSubmodels.insert(submodel);
	if (result.second) {
		if (!mUseInstancing) {
			addMovable(entity);
		}
	}

	return true;
}


bool Model::removeSubmodel(SubModel* submodel) {
	auto numberRemoved = mSubmodels.erase(submodel);
	if (numberRemoved > 0) {
		removeMovable(submodel->getEntity());
	}
	return true;
}

SubModel* Model::getSubModel(size_t index) {
	size_t i = 0;
	auto result = std::find_if(mSubmodels.begin(), mSubmodels.end(),
							   [&i, &index](SubModel*) -> bool { return i++ == index; }
	);
	if (result != mSubmodels.end()) {
		return *result;
	}
	S_LOG_FAILURE("Could not find submodel with index " << index << " in model " << mName);
	return nullptr;

}

void Model::showPart(const std::string& partName, bool hideOtherParts) {
	auto I = mModelParts.find(partName);
	if (I != mModelParts.end()) {
		ModelPart& modelPart = I->second;
		if (hideOtherParts) {
			const std::string& groupName = modelPart.getGroupName();
			//make sure that all other parts in the same group are hidden
			auto partBucketI = mGroupsToPartMap.find(groupName);
			if (partBucketI != mGroupsToPartMap.end()) {
				for (auto& part : partBucketI->second) {
					if (part != partName) {
						hidePart(part, true);
					}
				}
			}
		}

		modelPart.show();
	}
}

void Model::hidePart(const std::string& partName, bool dontChangeVisibility) {
	auto I = mModelParts.find(partName);
	if (I != mModelParts.end()) {
		ModelPart& modelPart = I->second;
		modelPart.hide();
		if (!dontChangeVisibility) {
			modelPart.setVisible(false);
			const std::string& groupName = modelPart.getGroupName();
			//if some part that was hidden before now should be visible
			auto partBucketI = mGroupsToPartMap.find(groupName);
			if (partBucketI != mGroupsToPartMap.end()) {
				for (auto& part : partBucketI->second) {
					if (part != partName) {
						auto I_modelPart = mModelParts.find(partName);
						if (I_modelPart != mModelParts.end()) {
							if (I_modelPart->second.getVisible()) {
								I_modelPart->second.show();
								break;
							}
						}
					}
				}
			}
		}
	}
}

void Model::setVisible(bool visible) {
	mVisible = visible;
	for (auto& movable : mMovableObjects) {
		movable->setVisible(visible);
	}
	for (auto& lightEntry: mLights) {
	    lightEntry.light->setVisible(visible);
	}
}

bool Model::getVisible() const {
	return mVisible;
}

void Model::setDisplaySkeleton(bool display) {
	for (auto& submodel : mSubmodels) {
		submodel->getEntity()->setDisplaySkeleton(display);
	}
}

bool Model::getDisplaySkeleton() const {
	const auto I = mSubmodels.begin();
	if (I != mSubmodels.end()) {
		return (*I)->getEntity()->getDisplaySkeleton();
	}
	return false;
}

Ogre::Vector3 Model::getScale() const {
	if (mParentNodeProvider) {
		return mParentNodeProvider->getScale();
	}
	return Ogre::Vector3::UNIT_SCALE;
}

const Ogre::Quaternion& Model::getRotation() const {
	return mRotation;
}

ModelDefinition::UseScaleOf Model::getUseScaleOf() const {
	return mDefinition->getUseScaleOf();
}

Action* Model::getAction(const std::string& name) {
	const auto I = mActions.find(name);
	if (I == mActions.end()) {
		return nullptr;
	}
	return &(I->second);
}

Action* Model::getAction(ActivationDefinition::Type type, const std::string& trigger) {
	for (auto& entry : mActions) {
		Action& action = entry.second;
		for (auto& activationDefinition : action.mActivations) {
			if (type == activationDefinition.type && trigger == activationDefinition.trigger) {
				//FIXME: Should keep actions as pointers
				return &action;
			}
		}
	}
	return nullptr;
}

void Model::addMovable(Ogre::MovableObject* movable) {
	if (mParentNodeProvider) {
		mParentNodeProvider->attachObject(movable);
	}
	if (mUserObject) {
		movable->getUserObjectBindings().setUserAny(Ogre::Any(mUserObject));
	}

	movable->setQueryFlags(mQueryFlags);
	if (mRenderingDistance > 0) {
		movable->setRenderingDistance(mRenderingDistance);
	}

	mMovableObjects.push_back(movable);

}

void Model::removeMovable(Ogre::MovableObject* movable) {
	if (mParentNodeProvider) {
		mParentNodeProvider->detachObject(movable);
	}
	if (mUserObject) {
		movable->getUserObjectBindings().clear();
	}

	auto I = std::find(std::begin(mMovableObjects), std::end(mMovableObjects), movable);
	if (I != mMovableObjects.end()) {
		mMovableObjects.erase(I);
	}
}

void Model::resetSubmodels() {
	for (auto& submodel : mSubmodels) {
		removeMovable(submodel->getEntity());
		delete submodel;
	}
	mSubmodels.clear();
	mModelParts.clear();
}

void Model::resetParticles() {
	if (!mParticleSystems.empty()) {
		auto particleSystems = mParticleSystems;
		for (auto& system : particleSystems) {
			removeMovable(system->getOgreParticleSystem());
			delete system;
		}
		mParticleSystems.clear();
	}
	mAllParticleSystemBindings.clear();
}

void Model::resetLights() {
    for (auto& lightInfo : mLights) {
        Ogre::Light* light = lightInfo.light;
        delete lightInfo.nodeProvider;

        if (light) {
            mManager.destroyLight(light);
        }
    }
    mLights.clear();
}

void Model::attachToNode(INodeProvider* nodeProvider) {
	for (auto& movable : mMovableObjects) {

		if (mParentNodeProvider && mParentNodeProvider != nodeProvider) {
			mParentNodeProvider->detachObject(movable);
		}
		if (nodeProvider) {
			nodeProvider->attachObject(movable);
		}
	}
	for (auto& lightEntry : mLights) {
	    if (lightEntry.nodeProvider) {
	        lightEntry.nodeProvider->detachObject(lightEntry.light);
	        delete lightEntry.nodeProvider;
	        lightEntry.nodeProvider = nullptr;
	    }

	    if (nodeProvider) {
            if (!lightEntry.position.isNaN() && !lightEntry.position.isZeroLength()) {
                lightEntry.nodeProvider = nodeProvider->createChildProvider("");
                lightEntry.nodeProvider->setPositionAndOrientation(lightEntry.position, Ogre::Quaternion::IDENTITY);
                lightEntry.nodeProvider->attachObject(lightEntry.light);
            } else {
                nodeProvider->attachObject(lightEntry.light);
            }
        }
	}
	mParentNodeProvider = nodeProvider;
}

Ogre::TagPoint* Model::attachObject(const std::string& attachPoint, Ogre::MovableObject* movable) {
	if (mSkeletonOwnerEntity) {
		for (auto& attachPointDef : mDefinition->getAttachPointsDefinitions()) {
			if (attachPointDef.Name == attachPoint) {
				const std::string& boneName = attachPointDef.BoneName;
				//use the rotation in the attach point def
				Ogre::TagPoint* tagPoint = mSkeletonOwnerEntity->attachObjectToBone(boneName, movable);
				if (!mAttachPoints) {
					mAttachPoints.reset(new std::vector<AttachPointWrapper>());
				}

				AttachPointWrapper wrapper;
				wrapper.TagPoint = tagPoint;
				wrapper.Movable = movable;
				wrapper.Definition = attachPointDef;
				mAttachPoints->push_back(wrapper);
				return tagPoint;
			}
		}
	}
	return nullptr;
}

bool Model::hasAttachPoint(const std::string& attachPoint) const {
	return std::find_if(mDefinition->mAttachPoints.begin(), mDefinition->mAttachPoints.end(), [&attachPoint](const AttachPointDefinition& def) -> bool { return def.Name == attachPoint; }) != mDefinition->mAttachPoints.end();
}

void Model::detachObject(Ogre::MovableObject* movable) {
	if (mSkeletonOwnerEntity) {
		mSkeletonOwnerEntity->detachObjectFromBone(movable);
	}
	if (mAttachPoints) {
		std::vector<AttachPointWrapper>& attachPoints = *mAttachPoints;
		for (auto I = attachPoints.begin(); I != attachPoints.end(); ++I) {
			if (I->Movable == movable) {
				attachPoints.erase(I);
				break;
			}
		}
	}
}

Ogre::AnimationState* Model::getAnimationState(const Ogre::String& name) {
	if (!mSubmodels.empty() && mSkeletonOwnerEntity) {
		return mSkeletonOwnerEntity->getAnimationState(name);
	}

	return nullptr;

}

Ogre::AnimationStateSet* Model::getAllAnimationStates() {
	if (!mSubmodels.empty() && mSkeletonOwnerEntity) {
		return mSkeletonOwnerEntity->getAllAnimationStates();
	}

	return nullptr;

}

Ogre::SkeletonInstance* Model::getSkeleton() const {
	if (!mSubmodels.empty() && mSkeletonOwnerEntity) {
		return mSkeletonOwnerEntity->getSkeleton();
	}

	return nullptr;

}


void Model::setRenderingDistance(Ogre::Real dist) {
	mRenderingDistance = dist;
	if (dist > 0) {
		for (auto& movable : mMovableObjects) {
			movable->setRenderingDistance(dist);
		}
        for (auto& lightEntry: mLights) {
            lightEntry.light->setRenderingDistance(dist);
        }
    }
}

void Model::setQueryFlags(unsigned int flags) {
	mQueryFlags = flags;
	for (auto& movable : mMovableObjects) {
		movable->setQueryFlags(flags);
	}
    for (auto& lightEntry: mLights) {
        lightEntry.light->setQueryFlags(flags);
    }
}

/** Overridden from MovableObject */
//void Model::_notifyAttached(Ogre::Node* parent, bool isTagPoint) {


//	auto I = mParticleSystems.begin();
//	while (I != mParticleSystems.end()) {
//
//		(*I)->getOgreParticleSystem()->_notifyAttached(parent, isTagPoint);
//		try {
//			//Try to trigger a load of any image resources used by affectors.
//			//The reason we want to do this now is that otherwise it will happen during rendering. An exception will then be thrown
//			//which will bubble all the way up to the main loop, thus aborting all frames.
//			(*I)->getOgreParticleSystem()->_update(0);
//
//			//Check if the material used is transparent. If so, assign it a later render queue.
//			//This is done to make transparent particle systems play better with the foliage and the water.
//			//The foliage would be rendered at an earlier render queue (RENDER_QUEUE_6 normally) and the water at RENDER_QUEUE_8.
//			//This of course means that there's still an issue when the camera is below the water
//			//(as the water, being rendered first, will prevent the particles from being rendered). That will need to be solved.
//			std::pair<Ogre::ResourcePtr, bool> result = Ogre::MaterialManager::getSingleton().createOrRetrieve((*I)->getOgreParticleSystem()->getMaterialName(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
//			Ogre::MaterialPtr materialPtr = result.first.staticCast<Ogre::Material>();
//			if (materialPtr) {
//				if (materialPtr->isTransparent()) {
//					(*I)->getOgreParticleSystem()->setRenderQueueGroup(Ogre::RENDER_QUEUE_9);
//				}
//			}
//
//			++I;
//		} catch (const Ogre::Exception& ex) {
//			//An exception occurred when forcing an update of the particle system. Remove it.
//			S_LOG_FAILURE("Error when loading particle system " << (*I)->getOgreParticleSystem()->getName() << ". Removing it.");
//			mMovableObjects.erase(std::find(std::begin(mMovableObjects), std::end(mMovableObjects), (*I)->getOgreParticleSystem()));
//			delete *I;
//			I = mParticleSystems.erase(I);
//		}
//	}


//}

const std::unique_ptr<std::vector<Model::AttachPointWrapper>>& Model::getAttachedPoints() const {
	return mAttachPoints;
}

void Model::setUserObject(std::shared_ptr<EmberEntityUserObject> userObject) {
	for (auto& movable : mMovableObjects) {
		movable->getUserObjectBindings().setUserAny(Ogre::Any(userObject));
	}
    for (auto& lightEntry: mLights) {
        lightEntry.light->getUserObjectBindings().setUserAny(Ogre::Any(userObject));
    }


    mUserObject = userObject;
}

Ogre::SceneManager& Model::getManager() {
	return mManager;
}

float Model::getCombinedBoundingRadius() const {
	float radius = 0;
	for (auto& movable : mMovableObjects) {
		radius = std::max(movable->getBoundingRadius(), radius);
	}
	return radius;
}

float Model::getBoundingRadius() const {
	return getCombinedBoundingRadius();
}

Ogre::AxisAlignedBox Model::getCombinedBoundingBox() const {
	Ogre::AxisAlignedBox aabb;
	for (auto& movable : mMovableObjects) {
		aabb.merge(movable->getBoundingBox());
	}
	return aabb;
}

Ogre::AxisAlignedBox Model::getBoundingBox() const {
	return getCombinedBoundingBox();
}

const INodeProvider* Model::getNodeProvider() const {
	return mParentNodeProvider;
}

INodeProvider* Model::getNodeProvider() {
	return mParentNodeProvider;
}

bool Model::isLoaded() const {
	return mLoaded;
}

bool Model::useInstancing() const {
	return mUseInstancing;
}

void Model::doWithMovables(const std::function<void(Ogre::MovableObject*, int)>& callback) {
	int i = 0;
	for (auto movable : mMovableObjects) {
		callback(movable, i++);
	}
}


}
}
}
