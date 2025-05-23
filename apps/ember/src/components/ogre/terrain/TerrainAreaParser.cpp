//
// C++ Implementation: TerrainAreaParser
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
#include "TerrainAreaParser.h"
#include "TerrainArea.h"
#include "framework/Log.h"
#include <wfmath/atlasconv.h>


namespace Ember::OgreView::Terrain {

bool TerrainAreaParser::parseArea(const Atlas::Message::MapType& areaData, WFMath::Polygon<2>& poly, int& layer, bool& isScaled) {

	//Earlier versions of Cyphesis had all of the shape data directly in the area map attribute,
	//whereas newer versions have it all stored in a "shape" subentity. We therefore need to check for both
	//possibilities.
	const Atlas::Message::MapType* shapeMap(&areaData);

	auto shapeI = areaData.find("shape");
	if (shapeI != areaData.end()) {
		//If we enter here we know we're dealing with the newer format.
		const Atlas::Message::Element& shapeElement = shapeI->second;
		if (shapeElement.isMap()) {
			shapeMap = &shapeElement.Map();
			auto shapeTypeI = shapeMap->find("type");
			if (shapeTypeI == shapeMap->end() || !shapeTypeI->second.isString() || shapeTypeI->second != "polygon") {
				logger->error("TerrainArea 'shape' element must be of type 'polygon', since Ember currently doesn't support any other shape type.");
				return false;
			}
		}
	}

	try {
		WFMath::Polygon<2> newPoly(*shapeMap);
		poly = newPoly;
	} catch (const WFMath::_AtlasBadParse& ex) {
		logger->warn("Error when parsing polygon data from atlas: {}", ex.what());
		return false;
	}

	auto it = areaData.find("layer");
	if ((it != areaData.end()) && it->second.isInt()) {
		layer = (int) it->second.Int();
	} else {
		layer = 0;
	}

	it = areaData.find("scaled");
	if ((it != areaData.end()) && it->second.isInt()) {
		isScaled = it->second.Int() == 1;
	} else {
		isScaled = false;
	}

	return true;
}

Atlas::Message::Element TerrainAreaParser::createElement(const WFMath::Polygon<2>& poly, int layer, bool isScaled) {
	Atlas::Message::MapType map;
	if (poly.isValid()) {
		Atlas::Message::Element shapeElement = poly.toAtlas();
		if (shapeElement.isMap()) {
			shapeElement.Map()["type"] = "polygon";
			map.insert(std::make_pair("shape", shapeElement));
		} else {
			logger->warn("A polygon should be serialized into a map.");
		}
	}
	map["layer"] = layer;
	if (isScaled) {
		map["scaled"] = 1;
	}
	return map;

}

}



