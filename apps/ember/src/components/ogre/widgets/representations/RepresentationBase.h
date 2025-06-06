//
// C++ Interface: RepresentationBase
//
// Description: 
//
// Author: Martin Preisler <preisler.m@gmail.com>
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
#ifndef EMBEROGRE_GUI_REPRESENTATIONS_REPRESENTATIONBASE_H
#define EMBEROGRE_GUI_REPRESENTATIONS_REPRESENTATIONBASE_H

#include <CEGUI/Event.h>
#include <sigc++/signal.h>


namespace Ember::OgreView::Gui::Representations {

/**
 * @brief a visual (CEGUI widget) representation of data (using adapters)
 * 
 * Adapters simply connect widgets to native data but don't create the layouts
 * themselves. This class creates the layout and respective adapters so you
 * can use it standalone.
 */
template<typename ValueType>
class RepresentationBase {
public:

	/**
	 * @brief Dtor
	 */
	virtual ~RepresentationBase();

	/**
	 * @brief retrieves the GUI layout root
	 * 
	 * This is useful for adding the representation to your own GUI layouts
	 */
	virtual CEGUI::Window* getGuiRoot() = 0;

	/**
	 * @brief programatically sets the edited value
	 * 
	 * This can be used for "revert to default" for example
	 */
	virtual void setEditedValue(const ValueType& v) = 0;

	/**
	 * @brief retrieves the value user is editing
	 */
	virtual const ValueType& getEditedValue() const = 0;

	/**
	 * @brief retrieves a signal user can connect to, emitted when the value is edited
	 * 
	 * @see AdapterBase::EventValueChanged
	 */
	virtual sigc::signal<void()>& getEventValueChangedSignal() = 0;

	/**
	 * @brief retrieves the original value (at the time of representation construction)
	 * 
	 * @copydoc AdapterBase::getOriginalValue
	 */
	virtual const ValueType& getOriginalValue() const = 0;

	/**
	 * @brief notifies the representation to apply it's changes (to the original value)
	 * 
	 * @copydoc AdapterBase::applyChanges
	 */
	virtual void applyChanges() = 0;

	/**
	 * @brief checks whether this representation has changes
	 * 
	 * @copydoc AdapterBase::hasChanges
	 */
	virtual bool hasChanges() const = 0;

	/**
	 * @brief checks whether this represents something that has been removed
	 * 
	 * @copydoc AdapterBase::isRemoved
	 */
	virtual bool isRemoved() const = 0;

	/**
	 * @brief adds a suggested value
	 * 
	 * @copydoc AdapterBase::addSuggestion
	 */
	virtual void addSuggestion(const std::string& suggestion) = 0;
};

template<typename ValueType>
RepresentationBase<ValueType>::~RepresentationBase() = default;

}


#endif
