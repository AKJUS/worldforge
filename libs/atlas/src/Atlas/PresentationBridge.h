/*
 Copyright (C) 2013 Erik Ogenvik

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
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef PRESENTATIONBRIDGE_H_
#define PRESENTATIONBRIDGE_H_

#include <sstream>
#include <string>
#include <stack>
#include <deque>
#include "Bridge.h"

namespace Atlas
{

/**
 * @brief A bridge which is meant to be used solely for presenting Element data.
 *
 */
class PresentationBridge: public Atlas::Bridge
{
public:
	explicit PresentationBridge(std::ostream& stream);
	~PresentationBridge() override = default;

	void streamBegin() override;
	void streamMessage() override;
	void streamEnd() override;

	void mapMapItem(std::string name) override;
	void mapListItem(std::string name) override;
	void mapIntItem(std::string name, std::int64_t) override;
	void mapFloatItem(std::string name, double) override;
	void mapStringItem(std::string name, std::string) override;
    void mapNoneItem(std::string name) override;
	void mapEnd() override;

	void listMapItem() override;
	void listListItem() override;
	void listIntItem(std::int64_t) override;
	void listFloatItem(double) override;
	void listStringItem(std::string) override;
    void listNoneItem() override;
	void listEnd() override;

	/**
	 * Sets the max number of items to print per level.
	 *
	 * This is useful to prevent too much output.
	 *
	 * @param maxItems The max number of items. 0 disables this feature (which is the default).
	 */
	void setMaxItemsPerLevel(size_t maxItems);

	/**
	 * Sets the level at which filtering, if setMaxItemsPerLevel() has been called, should occur.
	 * Default is 1 (i.e. print everything for the top level).
	 * @param startFilteringLevel At which level filtering should start.
	 */
	void setStartFilteringLevel(size_t startFilteringLevel);

private:

	void addPadding();

	void removePadding();

	/**
	 * Checks if the current item should be printed or not, depending on if mMaxItemsPerLevel is set.
	 * @return True if the current item should be printed.
	 */
	bool checkAndUpdateMaxItemCounter();

	std::string mPadding;

	std::ostream& mStream;

	/**
	 * @brief Keeps track of the number of maps in lists.
	 *
	 * This is used to determine if we should print a separator to make it easier to see where a new map starts.
	 */
	std::stack<int> mMapsInList;

	/**
	 * If set to > 0 denotes the max number of items to print (per level).
	 */
	size_t mMaxItemsPerLevel;

	/**
	 * Set to true when entries should be skipped because the max number of items for a level has been reached.
	 */
	bool mIsSkipEntry;

	/**
	 * Denotes the level at which filtering through the mMaxItemsPerLevel field should occur.
	 * Set by default to 1 (i.e. always print all entries on the first level).
	 */
	size_t mStartFilterLevel;

	/**
	 * Keeps track of the number of entries in each level. Used when mMaxItemsPerLevel is > 0.
	 */
	std::deque<size_t> mEntriesPerLevelCounter;
};

}
#endif /* PRESENTATIONBRIDGE_H_ */
