//
// C++ Implementation: EntityCreatorTypeHelper
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2004
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
#include "EntityCreatorTypeHelper.h"
#include "ColouredListItem.h"
#include "ModelRenderer.h"
#include "EntityTextureManipulator.h"
#include "adapters/eris/RuleTreeAdapter.h"

#include "../Avatar.h"
#include "../model/Model.h"
#include "domain/EmberEntity.h"
#include "../World.h"
#include "components/ogre/mapping/EmberEntityMappingManager.h"

#include "services/EmberServices.h"
#include "services/server/ServerService.h"

#include <Eris/Connection.h>
#include <Eris/Avatar.h>
#include <Eris/TypeService.h>

#include <CEGUI/widgets/Listbox.h>
#include <CEGUI/widgets/Editbox.h>
#include <CEGUI/widgets/PushButton.h>
#include <CEGUI/widgets/Tree.h>
#include <CEGUI/widgets/ToggleButton.h>
#include "components/ogre/mapping/ModelActionCreator.h"


namespace Ember::OgreView::Gui {

EntityCreatorTypeHelper::EntityCreatorTypeHelper(Eris::Avatar& avatar,
												 CEGUI::Tree& typeTree,
												 CEGUI::Editbox& nameEditbox,
												 CEGUI::PushButton& pushButton,
												 CEGUI::Window& modelPreview,
												 CEGUI::Combobox& modeCombobox,
												 CEGUI::Window& defaultModeWindow,
												 CEGUI::ToggleButton& plantedOnGroundWindow,
												 CEGUI::Editbox& filterWindow) :
		mAvatar(avatar),
		mName(nameEditbox),
		mModelPreviewRenderer(nullptr),
		mModelPreviewManipulator(nullptr),
		mRuleTreeAdapter(nullptr),
		mCreateButton(nullptr),
		mModeCombobox(modeCombobox),
		mDefaultModeWindow(defaultModeWindow),
		mPlantedOnGroundWindow(plantedOnGroundWindow),
		mFilterWindow(filterWindow) {
	typeTree.setItemTooltipsEnabled(true);
	typeTree.setSortingEnabled(true);

	typeTree.subscribeEvent(CEGUI::Tree::EventSelectionChanged, CEGUI::Event::Subscriber(&EntityCreatorTypeHelper::typeTree_SelectionChanged, this));
	mCreateButton = &pushButton;
	mCreateButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&EntityCreatorTypeHelper::createButton_Click, this));

	mRuleTreeAdapter = std::make_unique<Adapters::Eris::RuleTreeAdapter>(mAvatar.getConnection(), mAvatar.getId(), typeTree);
	mRuleTreeAdapter->refreshRules({"game_entity", "archetype"});

	mModelPreviewRenderer = std::make_unique<ModelRenderer>(&modelPreview, "modelPreview");
	mModelPreviewManipulator = std::make_unique<CameraEntityTextureManipulator>(modelPreview, mModelPreviewRenderer->getEntityTexture());


	mAvatar.getConnection().getTypeService().BoundType.connect(sigc::mem_fun(*this, &EntityCreatorTypeHelper::typeService_BoundType));

	mModeCombobox.addItem(Gui::ColouredListItem::createColouredListItem("free"));
	mModeCombobox.addItem(Gui::ColouredListItem::createColouredListItem("planted"));
	mModeCombobox.addItem(Gui::ColouredListItem::createColouredListItem("fixed"));
	mModeCombobox.addItem(Gui::ColouredListItem::createColouredListItem("floating"));

	filterWindow.subscribeEvent(CEGUI::Editbox::EventTextChanged, [&]() { mRuleTreeAdapter->setFilter(filterWindow.getText().c_str()); });
}

EntityCreatorTypeHelper::~EntityCreatorTypeHelper() = default;

void EntityCreatorTypeHelper::updatePreview() {
	if (mModelPreviewRenderer && mRuleTreeAdapter) {
		auto typeData = mRuleTreeAdapter->getSelectedRule();
		if (typeData.isValid()) {
			//check if the type is bound
			mCurrentType = typeData->getId();
			auto type = mAvatar.getConnection().getTypeService().getTypeByName(typeData->getId());
			if (type && type->isBound()) {
				Authoring::DetachedEntity entity("0", type);
				showPreview(entity);

				mCreateButton->setEnabled(true);

				auto modeElement = type->getProperty("mode");
				if (modeElement && modeElement->isString()) {
					mDefaultModeWindow.setText("(" + modeElement->String() + ")");
				} else {
					mModeCombobox.setText("");
				}

			} else {
				mModelPreviewRenderer->showModel("");
				mCreateButton->setEnabled(false);
			}
		} else {
			mModelPreviewRenderer->showModel("");
			mCreateButton->setEnabled(false);
		}
	}
}

void EntityCreatorTypeHelper::showPreview(Ember::OgreView::Authoring::DetachedEntity& entity) {
	Mapping::ModelActionCreator actionCreator(entity, [&](const std::string& model) {
		mModelPreviewRenderer->showModel(model);
		mModelPreviewRenderer->showFull();
		//we want to zoom in a little
		mModelPreviewRenderer->setCameraDistance(0.7f);

	}, [&](const std::string& part) {
		if (mModelPreviewRenderer->getModel()) {
			mModelPreviewRenderer->getModel()->showPart(part);
		}
	});

	auto mapping = Mapping::EmberEntityMappingManager::getSingleton().getManager().createMapping(entity, actionCreator, mAvatar.getView().getTypeService(), &mAvatar.getView());
	if (mapping) {
		mapping->initialize();
	}
}

void EntityCreatorTypeHelper::typeService_BoundType(Eris::TypeInfo* typeInfo) {
	//If the type that's now bound is the one that's currently handled, update the preview
	if (mModelPreviewRenderer && mRuleTreeAdapter && typeInfo->getName() == mCurrentType) {
		auto typeData = mRuleTreeAdapter->getSelectedRule();
		if (typeData.isValid()) {
			Authoring::DetachedEntity entity("0", typeInfo);
			showPreview(entity);
			mCreateButton->setEnabled(true);
			auto modeElement = typeInfo->getProperty("mode");
			if (modeElement && modeElement->isString()) {
				mDefaultModeWindow.setText("(" + modeElement->String() + ")");
			} else {
				mModeCombobox.setText("");
			}
		}
	}
}

bool EntityCreatorTypeHelper::typeTree_SelectionChanged(const CEGUI::EventArgs& args) {
	updatePreview();
	return true;
}

bool EntityCreatorTypeHelper::createButton_Click(const CEGUI::EventArgs& args) {
	if (mRuleTreeAdapter) {

		auto typeData = mRuleTreeAdapter->getSelectedRule();
		if (typeData.isValid()) {
			try {
				std::string name;
				if (!mName.getText().empty()) {
					name = mName.getText().c_str();
				}

				auto typeInfo = mAvatar.getConnection().getTypeService().getTypeByName(typeData->getId());
				if (typeInfo) {
					Atlas::Message::MapType definition{{"parent", typeInfo->getName()}};

					if (!name.empty()) {
						definition["name"] = name;
					}

					if (!mModeCombobox.getText().empty()) {
						definition["mode"] = mModeCombobox.getText().c_str();
					}

					if (mPlantedOnGroundWindow.isSelected()) {
						definition["mode_data"] = Atlas::Message::MapType{{"mode", "planted"},
																		  {"$eid", mAvatar.getView().getTopLevel()->getId()}};
					}

					EventCreateFromType(definition);
				}
			} catch (const std::exception& ex) {
				logger->warn("Error when trying to create entity from type: {}", ex.what());
			}
		}
	}
	return true;
}

}



