//
// C++ Implementation: TerrainMod
//
// Description:
//
//
// Author: Tamas Bates <rhymer@gmail.com>, (C) 2008
// Author: Erik Ogenvik <erik@worldforge.org>, (C) 2008
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.//
//
#ifdef HAVE_CONFIG_H
#endif

#include "TerrainModTranslator.h"
#include "common/log.h"

#include <Mercator/TerrainMod.h>

#include <wfmath/atlasconv.h>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Message::FloatType;

/**
* Interface for inner translator.
*
* Concrete templated subclasses are used for both shapes and mod types.
*/
struct InnerTranslator {
	explicit InnerTranslator(const Atlas::Message::MapType& data) : mData(data) {}

	virtual ~InnerTranslator() = default;

	virtual std::unique_ptr<Mercator::TerrainMod> createInstance(const WFMath::Point<3>& pos, const WFMath::Quaternion& orientation) = 0;

	const Atlas::Message::MapType mData;
};


/**
 * @brief Templated translator which creates concrete mod instances.
 */
template<template<template<int> class ShapeT> class ModT,
		template<int> class ShapeT>
class InnerTranslatorImpl : public InnerTranslator {
public:
	InnerTranslatorImpl(const ShapeT<2>& shape, const Atlas::Message::MapType& data);

	~InnerTranslatorImpl() override = default;

	std::unique_ptr<Mercator::TerrainMod> createInstance(const WFMath::Point<3>& pos, const WFMath::Quaternion& orientation) override;

	const ShapeT<2> mShape;
};

/**
 * @brief Translator for Mercator::SlopeTerrainMod, since it has a different constructor than other mods.
 */
template<template<int> class ShapeT>
class InnerTranslatorSlope : public InnerTranslator {
public:
	InnerTranslatorSlope(const ShapeT<2>& shape, const Atlas::Message::MapType& data, float dx, float dy);

	~InnerTranslatorSlope() override = default;

	std::unique_ptr<Mercator::TerrainMod> createInstance(const WFMath::Point<3>& pos, const WFMath::Quaternion& orientation) override;

	const ShapeT<2> mShape;
	float mDx;
	float mDy;
};

template<template<template<int> class ShapeT> class ModT,
		template<int> class ShapeT>
InnerTranslatorImpl<ModT, ShapeT>::InnerTranslatorImpl(const ShapeT<2>& shape, const Atlas::Message::MapType& data) :
		InnerTranslator(data), mShape(shape) {
}

template<template<int> class ShapeT>
InnerTranslatorSlope<ShapeT>::InnerTranslatorSlope(const ShapeT<2>& shape, const Atlas::Message::MapType& data, float dx, float dy) :
		InnerTranslator(data), mShape(shape), mDx(dx), mDy(dy) {
}

template<template<template<int> class ShapeT> class ModT,
		template<int> class ShapeT>
std::unique_ptr<Mercator::TerrainMod> InnerTranslatorImpl<ModT, ShapeT>::createInstance(const WFMath::Point<3>& pos, const WFMath::Quaternion& orientation) {
	ShapeT<2> shape = this->mShape;

	if (!shape.isValid() || !pos.isValid()) {
		return nullptr;
	}

	if (orientation.isValid()) {
		/// rotation about Y axis
		WFMath::Vector<3> xVec = WFMath::Vector<3>(1.0, 0.0, 0.0).rotate(orientation);
		WFMath::CoordType theta = std::atan2(xVec.z(), xVec.x());
		WFMath::RotMatrix<2> rm;
		shape.rotatePoint(rm.rotation(theta), WFMath::Point<2>(0, 0));
	}

	shape.shift(WFMath::Vector<2>(pos.x(), pos.z()));
	float level = TerrainModTranslator::parsePosition(pos, this->mData);
	return std::unique_ptr<ModT<ShapeT>>(new ModT<ShapeT>(level, shape));
}

template<template<int> class ShapeT>
std::unique_ptr<Mercator::TerrainMod> InnerTranslatorSlope<ShapeT>::createInstance(const WFMath::Point<3>& pos, const WFMath::Quaternion& orientation) {
	ShapeT<2> shape = this->mShape;

	if (!shape.isValid() || !pos.isValid() || !orientation.isValid()) {
		return nullptr;
	}

	if (orientation.isValid()) {
		/// rotation about Y axis
		WFMath::Vector<3> xVec = WFMath::Vector<3>(1.0, 0.0, 0.0).rotate(orientation);
		WFMath::CoordType theta = std::atan2(xVec.z(), xVec.x());
		WFMath::RotMatrix<2> rm;
		shape.rotatePoint(rm.rotation(theta), WFMath::Point<2>(0, 0));
	}

	shape.shift(WFMath::Vector<2>(pos.x(), pos.z()));
	float level = TerrainModTranslator::parsePosition(pos, this->mData);
	return std::unique_ptr<Mercator::SlopeTerrainMod<ShapeT>>(new Mercator::SlopeTerrainMod<ShapeT>(level, mDx, mDy, shape));
}

/**
 * @brief Parses the changes to the position of the mod
 * If no height data is given the supplied height will
 * be used. If however a "height" value is set, that will be used instead.
 * If no "height" value is set, but a "heightoffset" is present, that value
 * will be added to the supplied height.
 * @param pos Position of the mod, without any adjustments.
 * @param modElement Atlas data describing the mod
 * @return The adjusted height of the mod
 */
float TerrainModTranslator::parsePosition(const WFMath::Point<3>& pos, const MapType& modElement) {
	///If the height is specified use that, else check for a height offset. If none is found, use the default height of the entity position
	auto I = modElement.find("height");
	if (I != modElement.end()) {
		const Element& modHeightElem = I->second;
		if (modHeightElem.isNum()) {
			return static_cast<float>(modHeightElem.asNum());
		}
	} else {
		I = modElement.find("heightoffset");
		if (I != modElement.end()) {
			const Element& modHeightElem = I->second;
			if (modHeightElem.isNum()) {
				auto heightoffset = static_cast<float>(modHeightElem.asNum());
				return pos.y() + heightoffset;
			}
		}
	}
	return pos.y();
}

/**
 * @brief Ctor.
 */
TerrainModTranslator::TerrainModTranslator(const Atlas::Message::MapType& data) :
		mInnerTranslator(nullptr) {

	auto I = data.find("type");
	if (I == data.end() || !I->second.isString()) {
		return;
	}
	const std::string& modType = I->second.String();

	I = data.find("shape");
	if (I == data.end() || !I->second.isMap()) {
		return;
	}
	const MapType& shapeMap = I->second.Map();

	// Get shape's type
	I = shapeMap.find("type");
	if (I == shapeMap.end() || !I->second.isString()) {
		return;
	}
	const std::string& shapeType = I->second.String();
	if (shapeType == "ball") {
		WFMath::Ball<2> shape;
		mInnerTranslator = buildTranslator(data, modType, shape, shapeMap);
	} else if (shapeType == "rotbox") {
		WFMath::RotBox<2> shape;
		mInnerTranslator = buildTranslator(data, modType, shape, shapeMap);
	} else if (shapeType == "polygon") {
		WFMath::Polygon<2> shape;
		mInnerTranslator = buildTranslator(data, modType, shape, shapeMap);
	}
}

void TerrainModTranslator::reset() {
	mInnerTranslator.reset();
}

template<template<int> class Shape>
std::unique_ptr<InnerTranslator>
TerrainModTranslator::buildTranslator(const Atlas::Message::MapType& modElement, const std::string& typeName, Shape<2>& shape, const Atlas::Message::Element& shapeElement) {
	try {
		shape.fromAtlas(shapeElement);
	} catch (...) {
		///Just log an error and return false, this isn't fatal.
		spdlog::warn("Error when parsing shape from atlas.");
		return nullptr;
	}

	if (!shape.isValid()) {
		return nullptr;
	}

	if (typeName == "slopemod") {

		auto I = modElement.find("slopes");
		if (I == modElement.end()) {
			spdlog::warn("SlopeTerrainMod defined without slopes");
			return nullptr;
		}
		const Element& modSlopeElem = I->second;
		if (!modSlopeElem.isList()) {
			spdlog::warn("SlopeTerrainMod defined with malformed slopes");
			return nullptr;
		}
		const ListType& slopes = modSlopeElem.asList();
		if (slopes.size() < 2 || !slopes[0].isNum() || !slopes[1].isNum()) {
			spdlog::warn("SlopeTerrainMod defined without slopes");
			return nullptr;
		}
		const auto dx = static_cast<float>(slopes[0].asNum());
		const auto dy = static_cast<float>(slopes[1].asNum());

		return std::unique_ptr<InnerTranslatorSlope<Shape>>(new InnerTranslatorSlope<Shape>(shape, modElement, dx, dy));
//		return createInstance<Mercator::SlopeTerrainMod>(shape, pos, modElement, 0, 0);
	} else if (typeName == "levelmod") {
		return std::unique_ptr<InnerTranslatorImpl<Mercator::LevelTerrainMod, Shape>>(new InnerTranslatorImpl<Mercator::LevelTerrainMod, Shape>(shape, modElement));
	} else if (typeName == "adjustmod") {
		return std::unique_ptr<InnerTranslatorImpl<Mercator::AdjustTerrainMod, Shape>>(new InnerTranslatorImpl<Mercator::AdjustTerrainMod, Shape>(shape, modElement));
	} else if (typeName == "cratermod") {
		return std::unique_ptr<InnerTranslatorImpl<Mercator::CraterTerrainMod, Shape>>(new InnerTranslatorImpl<Mercator::CraterTerrainMod, Shape>(shape, modElement));
	}
	return nullptr;
}

/** 
 * @brief Parse the Atlas data and create the terrain mod instance with it
 * @param pos Position of the mod
 * @param orientation Orientation of the mod
 * @param modElement Atlas data describing the mod
 * @return true if translation succeeds
 */
std::unique_ptr<Mercator::TerrainMod> TerrainModTranslator::parseData(const WFMath::Point<3>& pos, const WFMath::Quaternion& orientation) {
	if (mInnerTranslator) {
		return mInnerTranslator->createInstance(pos, orientation);
	}
	return nullptr;
}

bool TerrainModTranslator::isValid() const {
	return mInnerTranslator != nullptr;
}


