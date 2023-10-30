//
// C++ Implementation: QuickHelpCursor
//
// Description:
//
//
// Author: Tiberiu Paunescu <tpa12@sfu.ca>, (C) 2010
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
#include "QuickHelpCursor.h"
#include "QuickHelp.h"

namespace Ember
{
namespace OgreView
{

namespace Gui
{

QuickHelpCursor::QuickHelpCursor() : mQuickHelp(Gui::QuickHelp::getSingleton())
{
	mQuickHelp.EventHelpMessageAdded.connect(sigc::mem_fun(*this, &QuickHelpCursor::getLatestHelpMessage));
	mQuickHelp.EventHelpMessageLocationChanged.connect(sigc::mem_fun(*this, &QuickHelpCursor::setCursorLocation));

	mCursorLocation = getSize();
	mCurrentPosition = mQuickHelp.getEnd();
	if (mCursorLocation > 0) {
		--mCurrentPosition;
		updateMessage();
	}
}

QuickHelpCursor::~QuickHelpCursor() = default;

void QuickHelpCursor::getLatestHelpMessage(std::list<HelpMessage>::const_iterator position)
{
	mCurrentPosition = position;
	updateMessage();
}

void QuickHelpCursor::updateMessage()
{
	if (mCurrentPosition != mQuickHelp.getEnd()) {
		EventUpdateText.emit(*mCurrentPosition);
	}
}

void QuickHelpCursor::nextMessage()
{
	if (getSize()) {
		//list.end() returns an iterator that is one past the front of the list, so we have a bit of extra work
		++mCurrentPosition;
		if (mCurrentPosition != mQuickHelp.getEnd())
		{
			++mCursorLocation;
			updateMessage();
		} else {
			--mCurrentPosition;
		}
	}
}

void QuickHelpCursor::previousMessage()
{
	if (mCurrentPosition != mQuickHelp.getBeginning())
	{
		--mCursorLocation;
		--mCurrentPosition;
		updateMessage();
	}
}

size_t QuickHelpCursor::getSize() const
{
	return mQuickHelp.getSize();
}

size_t QuickHelpCursor::getCursorLocation() const
{
	return mCursorLocation;
}

void QuickHelpCursor::setCursorLocation(size_t cursorLocation)
{
	mCursorLocation = cursorLocation;
}


}

}
}
