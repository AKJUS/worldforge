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

#include "ContainerView.h"

#include "components/ogre/EmberOgre.h"
#include "components/ogre/GUIManager.h"
#include "domain/EmberEntity.h"
#include "EntityIconSlot.h"
#include "EntityIcon.h"
#include "icons/IconManager.h"
#include "EntityTooltip.h"
#include <CEGUI/widgets/DragContainer.h>


namespace Ember::OgreView::Gui {

ContainerView::ContainerView(EntityIconManager& entityIconManager,
							 Icons::IconManager& iconManager,
							 CEGUI::Tooltip& tooltip,
							 CEGUI::Window& iconContainer,
							 int slotSize)
		:
		mEntityIconManager(entityIconManager),
		mIconManager(iconManager),
		mTooltip(tooltip),
		mIconContainer(iconContainer),
		mSlotSize(slotSize),
		mObservedEntity(nullptr) {
	mResizedConnection = mIconContainer.subscribeEvent(CEGUI::Window::EventSized, [&]() { layoutSlots(); });

	//Handle the case where there's an automatically created child which will receive all drop events.
	auto containerDropWindow = mIconContainer.getChild("__auto_container__");
	if (!containerDropWindow) {
		containerDropWindow = &mIconContainer;
	}

	mContainerDropTarget = std::make_unique<EntityIconDragDropTarget>(containerDropWindow);
	mContainerDropTarget->EventIconDropped.connect([this](EntityIcon* icon) {
		EventIconDropped.emit(icon, nullptr);
	});
}

ContainerView::~ContainerView() {
	mResizedConnection->disconnect();
	mActionConnection.disconnect();
	mChildRemovedConnection.disconnect();
	mChildAddedConnection.disconnect();
	mBeingDeletedConnection.disconnect();
	//Needs to remove icons and slots before we remove the widget.
	mIcons.clear();
	mSlots.clear();

}

void ContainerView::showEntityContents(EmberEntity* entity) {
	if (entity == mObservedEntity) {
		return;
	}
	mChildAddedConnection.disconnect();
	mChildRemovedConnection.disconnect();
	mBeingDeletedConnection.disconnect();
	mObservedEntity = entity;
	clearShownContent();
	if (entity) {
		for (size_t i = 0; i < entity->numContained(); ++i) {
			auto child = entity->getEmberContained(i);
			auto entityIcon = createEntityIcon(child);
			if (entityIcon) {
				getFreeSlot()->addEntityIcon(entityIcon);
				EventIconAdded.emit(entityIcon);
			}
		}
		layoutSlots();
		mChildAddedConnection = entity->ChildAdded.connect([&](Eris::Entity* child) {
			auto entityIcon = createEntityIcon(dynamic_cast<EmberEntity*>(child));
			if (entityIcon) {
				getFreeSlot()->addEntityIcon(entityIcon);
				EventIconAdded.emit(entityIcon);
				layoutSlots();
			}
		});
		mChildRemovedConnection = entity->ChildRemoved.connect([&](Eris::Entity* child) {
			auto I = std::find_if(mIcons.begin(), mIcons.end(), [child](const std::unique_ptr<EntityIcon>& entry) { return dynamic_cast<Eris::Entity*>(entry->getEntity()) == child; });
			if (I != mIcons.end()) {
				mIcons.erase(I);
				layoutSlots();
			}
		});
		mBeingDeletedConnection = entity->BeingDeleted.connect([&]() {
			clearShownContent();
			mObservedEntity = nullptr;
			mChildAddedConnection.disconnect();
			mChildRemovedConnection.disconnect();
		});
	}

}

EntityIconSlot* ContainerView::addSlot() {

	UniqueWindowPtr<CEGUI::Window> container(CEGUI::WindowManager::getSingleton().createWindow("EmberLook/StaticImage"));
	container->setSize(CEGUI::USize(CEGUI::UDim(0, 32), CEGUI::UDim(0, 32)));
	mIconContainer.addChild(container.get());
	layoutSlots();
	auto iconSlot = std::make_unique<EntityIconSlot>(std::move(container));

	mSlots.emplace_back(std::move(iconSlot));

	auto ptr = mSlots.back().get();

	ptr->EventIconDropped.connect([this, ptr](EntityIcon* entityIcon) {
		//If it's an icon that's already in the container, just add it. Otherwise emit a signal and let other code handle it.
		if (std::find_if(mIcons.begin(), mIcons.end(), [entityIcon](const std::unique_ptr<EntityIcon>& entry) { return entry.get() == entityIcon; }) != mIcons.end()) {
			auto oldSlot = entityIcon->getSlot();
			ptr->addEntityIcon(entityIcon);
			if (oldSlot) {
				oldSlot->notifyIconDraggedOff(entityIcon);
			}
		} else {
			EventIconDropped.emit(entityIcon, ptr);
		}
	});

	return ptr;
}

EntityIconSlot* ContainerView::getFreeSlot() {
	for (auto& slot: mSlots) {
		if (slot->getEntityIcon() == nullptr) {
			return slot.get();
		}
	}
	return addSlot();
}

EntityIcon* ContainerView::createEntityIcon(EmberEntity* entity) {
	auto icon = mIconManager.getIcon(32, entity);
	if (icon) {
		auto entityIcon = mEntityIconManager.createIconInstance(icon, entity, 32);
		entityIcon->getImage().setTooltip(&mTooltip);
		entityIcon->getImage().setTooltipText(entity->getId());
		auto& image = entityIcon->getImage();
		auto connection1 = entityIcon->getDragContainer()->subscribeEvent(CEGUI::DragContainer::EventMouseEntersSurface, [&image]() {
			image.setProperty("FrameEnabled", "true");
		});
		auto connection2 = entityIcon->getDragContainer()->subscribeEvent(CEGUI::DragContainer::EventMouseLeavesSurface, [&image]() {
			image.setProperty("FrameEnabled", "false");
		});
		auto connection3 = entityIcon->getDragContainer()->subscribeEvent(CEGUI::DragContainer::EventMouseClick, [this, entity]() {
			EventEntityPicked(entity);
		});

		entityIcon->getDragContainer()->subscribeEvent(CEGUI::Window::EventDestructionStarted, [connection1, connection2, connection3]() mutable {
			connection1->disconnect();
			connection2->disconnect();
			connection3->disconnect();
		});
		mIcons.emplace_back(std::move(entityIcon));
		return mIcons.back().get();
	}
	return nullptr;
}

void ContainerView::clearShownContent() {
	mIcons.clear();
}

void ContainerView::layoutSlots() {
	auto columns = (size_t) (std::floor(mIconContainer.getPixelSize().d_width / (float) mSlotSize));

	for (size_t i = 0; i < mSlots.size(); ++i) {
		auto& slot = mSlots[i];
		auto yPosition = (int) std::floor(i / columns);
		auto xPosition = i % columns;
		slot->getWindow()->setPosition({{0, (float) (xPosition * 32)},
										{0, (float) (yPosition * 32)}});

	}
}

EntityIcon* ContainerView::getEntityIcon(const std::string& entityId) {
	auto I = std::find_if(mIcons.begin(), mIcons.end(), [&entityId](const std::unique_ptr<EntityIcon>& entry) { return entry->getEntity() && entry->getEntity()->getId() == entityId; });
	if (I != mIcons.end()) {
		return I->get();
	}
	return nullptr;
}

void ContainerView::addEntityIcon(EntityIcon* entityIcon) {
	auto oldSlot = entityIcon->getSlot();
	getFreeSlot()->addEntityIcon(entityIcon);
	if (oldSlot) {
		oldSlot->notifyIconDraggedOff(entityIcon);
	}
	EventIconAdded.emit(entityIcon);
}


}

