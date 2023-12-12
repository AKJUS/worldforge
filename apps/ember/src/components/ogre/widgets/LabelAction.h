/*
 Copyright (C) 2014 Erik Ogenvik

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

#ifndef LABELACTION_H_
#define LABELACTION_H_

#include "components/entitymapping/Actions/Action.h"
#include <functional>

namespace Ember {
class EmberEntity;

namespace OgreView::Gui {

class LabelAction : public EntityMapping::Actions::Action {
public:


	/**
	 * @brief Static function called when we want to enable labels for an entity.
	 * Be sure to check that the function is valid before calling it.
	 */
	static std::function<void(EmberEntity&)> sEnableForEntity;

	/**
	 * @brief Static function called when we want to disable labels for an entity.
	 * Be sure to check that the function is valid before calling it.
	 */
	static std::function<void(EmberEntity&)> sDisableForEntity;


	explicit LabelAction(EmberEntity& entity);

	~LabelAction() override;

	/**
	 * @copydoc EntityMapping::Actions::Action::activate()
	 */
	void activate(EntityMapping::ChangeContext& context) override;

	/**
	 * @copydoc EntityMapping::Actions::Action::deactivate()
	 */
	void deactivate(EntityMapping::ChangeContext& context) override;

private:

	EmberEntity& mEntity;
};

}

}
#endif /* LABELACTION_H_ */
