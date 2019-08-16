//
// C++ Interface: ListAdapter
//
// Description: 
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2007
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
#ifndef EMBEROGRE_GUI_ADAPTERS_ATLASLISTADAPTER_H
#define EMBEROGRE_GUI_ADAPTERS_ATLASLISTADAPTER_H

#include "AdapterBase.h"
// #include "components/ogre/widgets/StackableContainer.h"


namespace CEGUI {
class Window;
}

namespace Ember {
namespace OgreView {

namespace Gui {

namespace Adapters {

namespace Atlas {

/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/
class ListAdapter : public AdapterBase {
public:
	ListAdapter(const ::Atlas::Message::Element& element, CEGUI::Window* childContainer);

	~ListAdapter() override;


	size_t getSize();

	const ::Atlas::Message::Element& valueOfAttr(size_t index) const;

	/**
	Updates the gui with new values.
	*/
	void updateGui(const ::Atlas::Message::Element& element) override;

//     void addAttributeAdapter(int index, Adapters::Atlas::AdapterBase* adapter, CEGUI::Window* containerWindow);
	void addAttributeAdapter(Adapters::Atlas::AdapterBase* adapter, CEGUI::Window* containerWindow);

	void removeAdapters();

protected:
	typedef std::vector<AdapterWrapper> AdapterStore;

	CEGUI::Window* mChildContainer;
// 	StackableContainer mStackableContainer;
	const ::Atlas::Message::ListType& mAttributes;

	AdapterStore mAdapters;

	void fillElementFromGui() override;

	bool _hasChanges() override;

	::Atlas::Message::Element _getChangedElement() override;

};

}

}

}

}

}

#endif
