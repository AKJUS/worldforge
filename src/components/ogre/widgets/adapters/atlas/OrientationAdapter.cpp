//
// C++ Implementation: OrientationAdapter
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "OrientationAdapter.h"
#include <wfmath/quaternion.h>
#include <wfmath/atlasconv.h>

namespace Ember {
namespace OgreView {

namespace Gui {

namespace Adapters {

namespace Atlas {

OrientationAdapter::OrientationAdapter(const ::Atlas::Message::Element& element, CEGUI::Window* xWindow, CEGUI::Window* yWindow, CEGUI::Window* zWindow, CEGUI::Window* scalarWindow)
: AdapterBase(element), mXWindow(xWindow), mYWindow(yWindow), mZWindow(zWindow), mScalarWindow(scalarWindow)
{
	if (mXWindow) {
		addGuiEventConnection(mXWindow->subscribeEvent(CEGUI::Window::EventTextChanged, CEGUI::Event::Subscriber(&OrientationAdapter::window_TextChanged, this))); 
	}
	if (mYWindow) {
		addGuiEventConnection(mYWindow->subscribeEvent(CEGUI::Window::EventTextChanged, CEGUI::Event::Subscriber(&OrientationAdapter::window_TextChanged, this))); 
	}
	if (mZWindow) {
		addGuiEventConnection(mZWindow->subscribeEvent(CEGUI::Window::EventTextChanged, CEGUI::Event::Subscriber(&OrientationAdapter::window_TextChanged, this))); 
	}
	if (mScalarWindow) {
		addGuiEventConnection(mScalarWindow->subscribeEvent(CEGUI::Window::EventTextChanged, CEGUI::Event::Subscriber(&OrientationAdapter::window_TextChanged, this))); 
	}

	
	updateGui(mOriginalValue);
}


OrientationAdapter::~OrientationAdapter() = default;

void OrientationAdapter::updateGui(const ::Atlas::Message::Element& element)
{
	SelfUpdateContext context(*this);
	
	WFMath::Quaternion orientation(element);
// 	axisBox.fromAtlas(element.asList());
	if (mXWindow) {
		mXWindow->setText(ValueTypeHelper<double, std::string>::convert(orientation.vector().x()));
	}
	if (mYWindow) {
		mYWindow->setText(ValueTypeHelper<double, std::string>::convert(orientation.vector().y()));
	}
	if (mZWindow) {
		mZWindow->setText(ValueTypeHelper<double, std::string>::convert(orientation.vector().z()));
	}
	if (mScalarWindow) {
		mScalarWindow->setText(ValueTypeHelper<double, std::string>::convert(orientation.scalar()));
	}

}

bool OrientationAdapter::window_TextChanged(const CEGUI::EventArgs& e)
{
	if (!mSelfUpdate) {
		EventValueChanged.emit();
	}
	return true;
}

void OrientationAdapter::fillElementFromGui()
{
	double x(0), y(0), z(0), scalar(0);
	if (mXWindow) {
		x = std::stod(mXWindow->getText().c_str());
	}
	if (mYWindow) {
		y = std::stod(mYWindow->getText().c_str());
	}
	if (mZWindow) {
		z = std::stod(mZWindow->getText().c_str());
	}
	if (mScalarWindow) {
		scalar = std::stod(mScalarWindow->getText().c_str());
	}
	WFMath::Quaternion orientation(scalar, x, y, z);
	mEditedValue = orientation.toAtlas();
}

bool OrientationAdapter::_hasChanges()
{
	WFMath::Quaternion originalOrientation(mOriginalValue);
	WFMath::Quaternion newOrientation(getValue());
	return originalOrientation != newOrientation;
}

}

}

}

}
}
