//
// C++ Implementation: ListAdapter
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
#include "ListAdapter.h"

using Atlas::Message::Element;
using Atlas::Message::ListType;
using Atlas::Message::MapType;


namespace Ember::OgreView::Gui::Adapters::Atlas {

ListAdapter::ListAdapter(const ::Atlas::Message::Element& element, CEGUI::Window* childContainer)
		: AdapterBase(element),
		  mChildContainer(childContainer),
		  mAttributes(element.asList()) {
}


ListAdapter::~ListAdapter() = default;

size_t ListAdapter::getSize() {
	return mAttributes.size();
}


const ::Atlas::Message::Element& ListAdapter::valueOfAttr(size_t index) const {
	static Element emptyElement;
	if (index > mAttributes.size()) {
		return emptyElement;
	} else {
		return mAttributes[index];
	}
}


void ListAdapter::updateGui(const ::Atlas::Message::Element& element) {
}


void ListAdapter::fillElementFromGui() {

}

bool ListAdapter::_hasChanges() {
	bool hasChanges = false;
	for (auto& wrapper: mAdapters) {
		if (!wrapper.Adapter) {
// 			logger->warn("The list of adapters contained a null reference. This should never happen.");
		} else {
			hasChanges = hasChanges || wrapper.Adapter->hasChanges();
		}
	}
	return hasChanges;
}


void ListAdapter::addAttributeAdapter(Adapters::Atlas::AdapterBase* adapter, CEGUI::Window* containerWindow) {
	if (adapter) {
		AdapterWrapper wrapper;
		wrapper.Adapter.reset(adapter);
		if (containerWindow) {
			containerWindow->setDestroyedByParent(false);
		}
		wrapper.ContainerWindow.reset(containerWindow);
		mAdapters.emplace_back(std::move(wrapper));
	} else {
// 		logger->warn("Tried to add a null adapter.");
	}
}

void ListAdapter::removeAdapters() {
	mAdapters.clear();
}

::Atlas::Message::Element ListAdapter::_getChangedElement() {
	//if one adapter has changes, we have to send all
	::Atlas::Message::ListType attributes;
	for (auto& wrapper: mAdapters) {
		auto& adapter = wrapper.Adapter;
		if (!adapter->isRemoved()) {
			attributes.emplace_back(adapter->getChangedElement());
		}
	}
	return {attributes};
}

}







