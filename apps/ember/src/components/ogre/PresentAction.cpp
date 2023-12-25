/*
 Copyright (C) 2018 Erik Ogenvik

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

#include <components/ogre/model/ModelDefinitionManager.h>
#include "EmberEntityMappingManager.h"
#include <boost/algorithm/string.hpp>
#include "PresentAction.h"
#include "components/ogre/model/Model.h"
#include "components/ogre/model/ModelRepresentation.h"
#include "components/entitymapping/ChangeContext.h"
#include "components/entitymapping/EntityMapping.h"
#include "components/ogre/Scene.h"

#include <OgreMeshManager.h>


namespace Ember::OgreView {

PresentAction::PresentAction(EmberEntity& entity,
							 Scene& scene,
							 EntityMapping::EntityMapping& mapping,
							 AttachmentFunction attachmentFunction)
		: ModelActionBase(entity, scene, mapping, std::move(attachmentFunction)) {
}

void PresentAction::activate(EntityMapping::ChangeContext& context) {

	auto element = mEntity.ptrOfProperty("present");
	if (element) {
		if (element->isString()) {
			auto& present = element->String();

			//If it's not an entity map it's either a mesh or a model.
			// Check if there's a model created already, if not we'll assume it's a mesh and create a model using that mesh
			if (!boost::ends_with(present, ".entitymap")) {
				if (!Model::ModelDefinitionManager::getSingleton().hasDefinition(present)) {
					//We'll automatically create a model which shows just the specified mesh.
					auto modelDef = std::make_shared<Model::ModelDefinition>();
					modelDef->setOrigin(present);
					//Create a single submodel definition using the mesh
					Model::SubModelDefinition subModelDefinition{present};
					modelDef->addSubModelDefinition(subModelDefinition);
					Model::ModelDefinitionManager::getSingleton().addDefinition(present, std::move(modelDef));
				}
				showModel(present);
			}
		}
	}

}

}
