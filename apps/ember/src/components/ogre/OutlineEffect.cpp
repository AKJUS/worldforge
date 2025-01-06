/*
 Copyright (C) 2020 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "OutlineEffect.h"
#include "Scene.h"

#include "components/ogre/model/ModelRepresentation.h"
#include "components/ogre/model/Model.h"
#include "components/ogre/model/SubModel.h"
#include "components/ogre/OgreInfo.h"

#include <Ogre.h>

// render queues
#define RENDER_QUEUE_OUTLINE_OBJECT        (Ogre::RENDER_QUEUE_MAIN + 1)
#define RENDER_QUEUE_OUTLINE_BORDER        (Ogre::RENDER_QUEUE_8 + 2)

// stencil values
#define STENCIL_VALUE_FOR_OUTLINE_GLOW 1


namespace Ember::OgreView {
struct StencilOpQueueListener : Ogre::RenderQueueListener {
	~StencilOpQueueListener() override {
		Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
		static Ogre::StencilState stencilState;
		stencilState.enabled = false;
		renderSystem->setStencilState(stencilState);
	}

	void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation,
	                        bool& skipThisInvocation) override {
		if (queueGroupId == RENDER_QUEUE_OUTLINE_OBJECT) {
			Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();

			renderSystem->clearFrameBuffer(Ogre::FBT_STENCIL);
			static Ogre::StencilState stencilState;
			stencilState.compareOp = Ogre::CMPF_ALWAYS_PASS;
			stencilState.referenceValue = STENCIL_VALUE_FOR_OUTLINE_GLOW;
			stencilState.depthStencilPassOp = Ogre::SOP_REPLACE;
			stencilState.enabled = true;
			renderSystem->setStencilState(stencilState);
		}
		if (queueGroupId == RENDER_QUEUE_OUTLINE_BORDER) {
			Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();

			static Ogre::StencilState stencilState;
			stencilState.compareOp = Ogre::CMPF_NOT_EQUAL;
			stencilState.referenceValue = STENCIL_VALUE_FOR_OUTLINE_GLOW;
			stencilState.enabled = true;
			renderSystem->setStencilState(stencilState);
		}
	}

	void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation,
	                      bool& repeatThisInvocation) override {
		if (queueGroupId == RENDER_QUEUE_OUTLINE_OBJECT || queueGroupId == RENDER_QUEUE_OUTLINE_BORDER) {
			Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
			static Ogre::StencilState stencilState;
			stencilState.enabled = false;
			renderSystem->setStencilState(stencilState);
		}
	}
};

OutlineEffect::OutlineEffect(Scene& scene, EmberEntityRef entity)
	: mScene(scene),
	  mSelectedEntity(entity),
	  mStencilOpQueueListener(std::make_unique<StencilOpQueueListener>()) {
	scene.getSceneManager().addRenderQueueListener(mStencilOpQueueListener.get());
	auto* modelRep = dynamic_cast<Model::ModelRepresentation*>(entity->getGraphicalRepresentation());
	if (modelRep && modelRep->getModel().getNodeProvider()) {
		if (modelRep->getModel().useInstancing()) {
			modelRep->getModel().doWithMovables([](Ogre::MovableObject* movable, int index) {
				if (movable->getMovableType() == "InstancedEntity") {
					movable->setVisible(false);
				}
			});
		}

		for (auto& submodels = modelRep->getModel().getSubmodels(); auto& submodel: submodels) {
			if (const auto ogreEntity = submodel->getEntity()) {
				mOutline.originalRenderQueueGroups.push_back(ogreEntity->getRenderQueueGroup());

				if (ogreEntity->isVisible()) {
					ogreEntity->setRenderQueueGroup(RENDER_QUEUE_OUTLINE_OBJECT);

					if (!ogreEntity->getParentNode()) {
						modelRep->getModel().getNodeProvider()->attachObject(ogreEntity);
					}

					auto outlineEntity = ogreEntity->clone(OgreInfo::createUniqueResourceName("outline"));
					outlineEntity->setCastShadows(false);

					outlineEntity->setRenderQueueGroup(RENDER_QUEUE_OUTLINE_BORDER);
					if (outlineEntity->hasSkeleton()) {
						outlineEntity->shareSkeletonInstanceWith(ogreEntity);
					}
					for (size_t i = 0; i < ogreEntity->getNumSubEntities(); ++i) {
						const auto outlineSubEntity = outlineEntity->getSubEntity(i);
						const auto subEntity = ogreEntity->getSubEntity(i);

						outlineSubEntity->setVisible(subEntity->isVisible());
						if (subEntity->isVisible()) {
							Ogre::TexturePtr texture;
							auto& material = subEntity->getMaterial();
							if (auto tech = material->getBestTechnique(); tech && tech->getNumPasses() > 0) {
								if (auto pass = tech->getPass(0); pass->getNumTextureUnitStates() > 0) {
									texture = pass->getTextureUnitState(0)->_getTexturePtr();
								}
							}
							if (!texture) {
								return;
							}

							auto outlineMaterial = Ogre::MaterialManager::getSingleton().
									getByName("/common/base/outline/nonculled")->clone(
										OgreInfo::createUniqueResourceName("outlineMaterial"));
							outlineMaterial->load();
							outlineMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(texture);
							outlineSubEntity->setMaterial(outlineMaterial);
							mOutline.generatedMaterials.emplace_back(outlineMaterial);
						}
					}
					modelRep->getModel().getNodeProvider()->attachObject(outlineEntity);
					mOutline.generatedEntities.push_back(outlineEntity);
				}
			}
		}
	}
}

OutlineEffect::~OutlineEffect() {
	mScene.getSceneManager().removeRenderQueueListener(mStencilOpQueueListener.get());

	if (mSelectedEntity) {
		const auto& oldEmberEntity = *mSelectedEntity;

		if (auto* modelRep = dynamic_cast<const Model::ModelRepresentation*>(oldEmberEntity.
			getGraphicalRepresentation())) {
			auto& model = modelRep->getModel();
			for (const auto& entity: mOutline.generatedEntities) {
				if (model.getNodeProvider()) {
					model.getNodeProvider()->detachObject(entity);
				}
			}

			if (model.useInstancing()) {
				model.doWithMovables([](Ogre::MovableObject* movable, int index) {
					if (movable->getMovableType() == "InstancedEntity") {
						movable->setVisible(true);
					}
				});
			}

			auto submodelI = model.getSubmodels().begin();
			for (size_t i = 0; i < model.getSubmodels().size(); ++i) {
				//It could be that the entity has been reloaded in the interim, so we need to check that originalRenderQueueGroups size matches.
				if (i < mOutline.originalRenderQueueGroups.size()) {
					(*submodelI)->getEntity()->setRenderQueueGroup(mOutline.originalRenderQueueGroups[i]);
				}
				//If instancing is used we've temporarily attached the Ogre::Entity to the nodes; need to detach it.
				if (model.useInstancing() && model.getNodeProvider()) {
					model.getNodeProvider()->detachObject((*submodelI)->getEntity());
				}

				++submodelI;
			}
		}
	}
	for (const auto entity: mOutline.generatedEntities) {
		mScene.getSceneManager().destroyMovableObject(entity);
	}
	for (auto material: mOutline.generatedMaterials) {
		material->getCreator()->remove(material);
	}
}
}
