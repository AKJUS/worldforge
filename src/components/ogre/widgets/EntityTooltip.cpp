/*
 Copyright (C) 2010  Erik Ogenvik <erik@ogenvik.org>

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

#include "EntityTooltip.h"
#include "components/ogre/World.h"
#include "domain/EmberEntity.h"
#include "components/ogre/widgets/icons/IconManager.h"
#include "components/ogre/widgets/icons/Icon.h"

#include <Eris/TypeInfo.h>

#include <CEGUI/WindowManager.h>
#include <CEGUI/WindowFactoryManager.h>

namespace Ember
{
namespace OgreView
{
namespace Gui
{

EmberEntityTooltipWidget::EmberEntityTooltipWidget(const CEGUI::String& type, const CEGUI::String& name) :
	CEGUI::Tooltip(type, name)
{
}

CEGUI::Sizef EmberEntityTooltipWidget::getTextSize_impl() const
{
	return CEGUI::Sizef(180, 80);
}

const CEGUI::String EmberEntityTooltipWidget::WidgetTypeName("Ember/EntityTooltip");

void EntityTooltip::registerFactory()
{
	CEGUI::WindowFactoryManager::addFactory<CEGUI::TplWindowFactory<EmberEntityTooltipWidget>>();
}

EntityTooltip::EntityTooltip(World& world, EmberEntityTooltipWidget& tooltip, Icons::IconManager& iconManager) :
	mWorld(world), mTooltip(tooltip), mIconManager(iconManager), isUpdatingOurselves(false), mImageWindow(0), mTextWindow(0)
{
	mImageWindow = CEGUI::WindowManager::getSingleton().createWindow("EmberLook/StaticImage");
	mImageWindow->setSize(CEGUI::USize(CEGUI::UDim(0, 64), CEGUI::UDim(0, 64)));
	mImageWindow->setProperty("FrameEnabled", "false");
	mImageWindow->setProperty("BackgroundEnabled", "false");
	tooltip.addChild(mImageWindow);

	mTextWindow = CEGUI::WindowManager::getSingleton().createWindow("EmberLook/StaticText");
	mTextWindow->setSize(CEGUI::USize(CEGUI::UDim(1, -64), CEGUI::UDim(1, 0)));
	mTextWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 64), CEGUI::UDim(0, 0)));
	mTextWindow->setProperty("HorzFormatting", "WordWrapLeftAligned");
	mTextWindow->setProperty("VertFormatting", "TopAligned");
	mTextWindow->setProperty("FrameEnabled", "false");
	mTextWindow->setProperty("BackgroundEnabled", "false");
	tooltip.addChild(mTextWindow);

	//listen to the text being changed, since that indicates that the tooltip has been activated for a new window (alas there's no signal when the tooltip is attached to a target window, so this is the best we can do)
	tooltip.subscribeEvent(CEGUI::Window::EventTextChanged, CEGUI::Event::Subscriber(&EntityTooltip::tooltip_TextChanged, this));
}

EntityTooltip::~EntityTooltip()
{
	CEGUI::WindowManager::getSingleton().destroyWindow(&mTooltip);
}

CEGUI::Tooltip& EntityTooltip::getTooltipWindow() const
{
	return mTooltip;
}

bool EntityTooltip::tooltip_TextChanged(const CEGUI::EventArgs &e)
{
	//Check the guard so we don't end up in an infinite loop, as we'll be resetting the text of the tooltip.
	if (!isUpdatingOurselves) {
		EmberEntity* entity = getActiveEntity();
		if (entity) {
			isUpdatingOurselves = true;
			mTextWindow->setText(composeEntityInfoText(*entity));
			Icons::Icon* icon = mIconManager.getIcon(64, entity);
			if (icon) {
				mImageWindow->setProperty("Image", CEGUI::PropertyHelper<CEGUI::Image*>::toString(icon->getImage()));
			} else {
				mImageWindow->setProperty("Image", "");
			}
			mTooltip.setWidth(CEGUI::UDim(0, 120));
			mTooltip.setHeight(CEGUI::UDim(0, 80));
			mTooltip.positionSelf();
		}
		mTooltip.setText(""); //The text has contained the id of the entity and should now be removed.
	}
	isUpdatingOurselves = false;
	return true;
}

std::string EntityTooltip::composeEntityInfoText(EmberEntity& entity)
{
	std::stringstream ss;
	if (!entity.getName().empty()) {
		ss << entity.getName() << " (of type " << entity.getType()->getName() << ")";
	} else {
		ss << entity.getType()->getName();
	}
	if (entity.hasAttr("worn")) {
		const Atlas::Message::Element& element = entity.valueOfAttr("worn");
		if (element.isString()) {
			ss << std::endl << "Worn on the " << element.asString();
		}
	}
	for (auto& entry : entity.getUsages()) {
		ss << std::endl << "Can be used to " << entry.first;
	}

	return ss.str();
}

EmberEntity* EntityTooltip::getActiveEntity()
{
	if (mTooltip.getTargetWindow()) {
		return mWorld.getEmberEntity(mTooltip.getTargetWindow()->getTooltipText().c_str());
	}
	return nullptr;
}

}
}
}
