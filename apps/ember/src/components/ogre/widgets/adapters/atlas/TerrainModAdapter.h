//
// C++ Interface: TerrainModAdapter
//
// Description: 
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2009
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
#ifndef EMBEROGRE_GUI_ADAPTERS_ATLASTERRAINMODADAPTER_H
#define EMBEROGRE_GUI_ADAPTERS_ATLASTERRAINMODADAPTER_H

#include "AdapterBase.h"

#include <memory>
#include "../ListBinder.h"

namespace Ember {
class EmberEntity;


namespace OgreView::Gui::Adapters::Atlas {

class PolygonAdapter;


class TerrainModBase {
public:

	const std::string& getType() const;

protected:
	explicit TerrainModBase(std::string type);

	std::string mType;
};

class LevelTerrainMod : public TerrainModBase {
public:
	LevelTerrainMod();
};

class AdjustTerrainMod : public TerrainModBase {
public:
	AdjustTerrainMod();
};

class PositioningBase {
public:

	const std::string& getType() const;

protected:
	explicit PositioningBase(std::string type);

	std::string mType;

};

class FixedPositioning : public PositioningBase {
public:
	FixedPositioning();
};

class RelativePositioning : public PositioningBase {
public:
	RelativePositioning();
};

/**
@brief An adapter for terrain mods.
This adapter will allow the user to edit the mods graphically through the use of the Polygon classes.
The polygon editing functionality is handled by an instance of PolygonAdapter, which this class wraps.
@author Erik Ogenvik <erik@ogenvik.org>
*/
class TerrainModAdapter : public AdapterBase {
public:

	/**
	 * @brief Ctor.
	 * @param element The existing element.
	 * @param showButton A buttons which toggles the display of the polygon on and off.
	 * @param entity The entity to which this belongs.
	 * @param posTypeCombobox A combobox for the positioning types.
	 * @param modTypeCombobox A combobox for the terrain mod types.
	 * @param heightTextbox An editbox for the height.
	 */
	TerrainModAdapter(const ::Atlas::Message::Element& element, CEGUI::PushButton* showButton, EmberEntity* entity, CEGUI::Combobox* posTypeCombobox, CEGUI::Combobox* modTypeCombobox,
					  CEGUI::Editbox* heightTextbox);

	/**
	 * @brief Dtor.
	 */
	~TerrainModAdapter() override;

	/**
	 * @brief Updates the gui with new values.
	 */
	void updateGui(const ::Atlas::Message::Element& element) override;

protected:

	/**
	 * @brief An optional entity to which the area belongs.
	 * Mainly used for height lookups, so that the polygon snaps to the ground.
	 */
	EmberEntity* mEntity;


	/**
	 * @brief The polygon adapter, which handles the actual polygon editing.
	 */
	std::unique_ptr<PolygonAdapter> mPolygonAdapter;

	CEGUI::Editbox* mHeightTextbox;

	ListBinder<TerrainModBase, CEGUI::Combobox> mTerrainModsBinder;
	ListBinder<PositioningBase, CEGUI::Combobox> mPositioningsBinder;


	bool heightTextbox_TextChanged(const CEGUI::EventArgs& e);

	void fillElementFromGui() override;

	bool _hasChanges() override;
};

}


}

#endif
