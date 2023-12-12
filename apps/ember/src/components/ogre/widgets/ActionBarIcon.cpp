//
// C++ Implementation: ActionBarIcon
//
// Description:
//
//	Author Tiberiu Paunescu <tpa12@sfu.ca>, (C) 2010
//	Based on the EntityIcon class by Erik Ogenvik <erik@ogenvik.org>, (C) 2007
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

#include "ActionBarIcon.h"
#include "CEGUIUtils.h"

#include <CEGUI/widgets/DragContainer.h>
#include <CEGUI/WindowManager.h>

using namespace CEGUI;


namespace Ember::OgreView::Gui {

ActionBarIcon::ActionBarIcon(ActionBarIconManager& manager, UniqueWindowPtr<CEGUI::DragContainer> dragContainer, UniqueWindowPtr<CEGUI::Window> image, Gui::Icons::Icon* icon)
		: ActionBarIconDragDropTarget(dragContainer.get()),
		  mManager(manager),
		  mDragContainer(std::move(dragContainer)),
		  mImage(std::move(image)),
		  mIcon(icon),
		  mUserData{*this},
		  mUserDataWrapper(mUserData),
		  mCurrentSlot(nullptr) {
	mDragContainer->setUserData(&mUserDataWrapper);
	mDragContainer->subscribeEvent(CEGUI::DragContainer::EventDragStarted, CEGUI::Event::Subscriber(&ActionBarIcon::dragContainer_DragStarted, this));
	mDragContainer->subscribeEvent(CEGUI::DragContainer::EventDragEnded, CEGUI::Event::Subscriber(&ActionBarIcon::dragContainer_DragStopped, this));
	icon->EventUpdated.connect(sigc::mem_fun(*this, &ActionBarIcon::icon_Updated));

}

ActionBarIcon::~ActionBarIcon() {
	if (mCurrentSlot) {
		mCurrentSlot->removeActionBarIcon();
	}
}

void ActionBarIcon::defaultAction() {
}

CEGUI::Window* ActionBarIcon::getImage() {
	return mImage.get();
}

Gui::Icons::Icon* ActionBarIcon::getIcon() {
	return mIcon;
}

CEGUI::DragContainer* ActionBarIcon::getDragContainer() {
	return mDragContainer.get();
}

void ActionBarIcon::setSlot(ActionBarIconSlot* slot) {
	if (mCurrentSlot) {
		mCurrentSlot->notifyIconRemoved();
	}
	mCurrentSlot = slot;
}

ActionBarIconSlot* ActionBarIcon::getSlot() {
	return mCurrentSlot;
}

void ActionBarIcon::setTooltipText(const std::string& text) {
	mDragContainer->setTooltipText(text);
}

bool ActionBarIcon::dragContainer_DragStarted(const CEGUI::EventArgs& args) {
	mManager.EventIconDragStart.emit(this);
	return true;
}

bool ActionBarIcon::dragContainer_DragStopped(const CEGUI::EventArgs& args) {
	mManager.EventIconDragStop.emit(this);
	return true;
}


void ActionBarIcon::icon_Updated() {
	//It seems that we're forced to invalidate the CEGUI Window to get it to update itself. This is perhaps a bug in CEGUI?
	mImage->invalidate();
}

bool ActionBarIcon::handleDragEnter(const CEGUI::EventArgs& args, ActionBarIcon* icon) {
	ActionBarIconDragDropTarget::handleDragEnter(args, icon);
	if (mCurrentSlot) {
		return mCurrentSlot->handleDragEnter(args, icon);
	}
	return true;
}

bool ActionBarIcon::handleDragLeave(const CEGUI::EventArgs& args, ActionBarIcon* icon) {
	ActionBarIconDragDropTarget::handleDragLeave(args, icon);
	if (mCurrentSlot) {
		return mCurrentSlot->handleDragLeave(args, icon);
	}
	return true;
}

bool ActionBarIcon::handleDragActionBarIconDropped(const CEGUI::EventArgs& args, ActionBarIcon* icon) {
	ActionBarIconDragDropTarget::handleDragActionBarIconDropped(args, icon);
	if (mCurrentSlot) {
		return mCurrentSlot->handleDragActionBarIconDropped(args, icon);
	}
	return true;

}

bool ActionBarIcon::handleDragEntityIconDropped(const CEGUI::EventArgs& args, EntityIcon* icon) {
	ActionBarIconDragDropTarget::handleDragEntityIconDropped(args, icon);
	if (mCurrentSlot) {
		return mCurrentSlot->handleDragEntityIconDropped(args, icon);
	}
	return true;
}

}


