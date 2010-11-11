//
// C++ Implementation: TerrainMod
//
// Description:
//
//
// Author: Tamas Bates <rhymer@gmail.com>, (C) 2008
// Author: Erik Hjortsberg <erik@worldforge.org>, (C) 2008
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
#include "config.h"
#endif

#include <Eris/TerrainMod.h>

#include <Mercator/TerrainMod.h>
#include <Eris/Log.h>

#include <Atlas/Message/Element.h>

#include <wfmath/atlasconv.h>

#include <cassert>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Message::FloatType;

namespace Eris {

/**
 * @brief Ctor.
 * This is protected to prevent any other class than subclasses of this to call it.
 * @param terrainMod The TerrainMod instance to which this instance belongs to.
 * @param typemod The type of terrainmod this handles, such as "cratermod" or "slopemod. This will be stored in mTypeName.
 */
InnerTerrainMod::InnerTerrainMod() : m_mod(0)
{
}

/// @brief Dtor.
InnerTerrainMod::~InnerTerrainMod()
{
}

/**
 * @brief Gets the type of terrain mod handled by this.
 * This corresponds to the "type" attribute of the "terrainmod" atlas attribute, for example "cratermod" or "slopemod".
 * Internally, it's stored in the mTypeName field, as set through the constructor.
 * @return The type of mod handled by any instance of this.
 */
const std::string& InnerTerrainMod::getTypename() const
{
    return mTypeName;
}

template <template <int> class Shape>
bool InnerTerrainMod::parseStuff(const WFMath::Point<3> & pos,
                                 const WFMath::Quaternion & orientation,
                                 const MapType& modElement,
                                 Shape<2> & shape,
                                 const Element & shapeMap)
{
    if (!parseShape(shapeMap, pos, orientation, shape)) {
        return false;
    }
    if (mTypeName == "slopemod") {
        return createInstance<Mercator::SlopeTerrainMod>(shape, pos, modElement, 0, 0);
    } else if (mTypeName == "levelmod") {
        return createInstance<Mercator::LevelTerrainMod>(shape, pos, modElement);
    } else if (mTypeName == "adjustmod") {
        return createInstance<Mercator::AdjustTerrainMod>(shape, pos, modElement);
    } else if (mTypeName == "cratermod") {
        return createInstance<Mercator::CraterTerrainMod>(shape, pos, modElement);
    }
    return false;
}

bool InnerTerrainMod::parseData(const WFMath::Point<3> & pos,
                                const WFMath::Quaternion & orientation,
                                const MapType& modElement)
{
    MapType::const_iterator I = modElement.find("type");
    if (I == modElement.end() || !I->second.isString()) {
        return false;
    }
    const std::string& modType = I->second.String();

    I = modElement.find("shape");
    if (I == modElement.end() || !I->second.isMap()) {
        return false;
    }
    const MapType& shapeMap = I->second.Map();

    // Get shape's type
    I = shapeMap.find("type");
    if (I == shapeMap.end() || !I->second.isString()) {
        return false;
    }
    const std::string& shapeType = I->second.String();
    if (m_mod != 0) {
        assert(!mTypeName.empty());
        assert(!mShapeName.empty());
        if (mTypeName != modType || mShapeName != shapeType) {
            m_mod = 0;
        }
    }
    mTypeName = modType;
    mShapeName = shapeType;
    if (shapeType == "ball") {
        WFMath::Ball<2> shape;
        return parseStuff(pos, orientation, modElement, shape, shapeMap);
    } else if (shapeType == "rotbox") {
        WFMath::RotBox<2> shape;
        return parseStuff(pos, orientation, modElement, shape, shapeMap);
    } else if (shapeType == "polygon") {
        WFMath::Polygon<2> shape;
        return parseStuff(pos, orientation, modElement, shape, shapeMap);
    }
    return false;
}


Mercator::TerrainMod* InnerTerrainMod::getModifier()
{
    return m_mod;
}

/**
 * @brief Parses the position of the mod.
 * If no height data is given the height of the entity the mod belongs to will be used.
 * If however a "height" value is set, that will be used instead.
 * If no "height" value is set, but a "heightoffset" is present, that value will be added to the height set by the position of the entity the mod belongs to.
 * @param modElement The top mod element.
 * @return The position of the mod, where the height has been adjusted.
 */
float InnerTerrainMod::parsePosition(const WFMath::Point<3> & pos, const MapType& modElement)
{
    ///If the height is specified use that, else check for a height offset. If none is found, use the default height of the entity position
    MapType::const_iterator I = modElement.find("height");
    if (I != modElement.end()) {
        const Element& modHeightElem = I->second;
        if (modHeightElem.isNum()) {
            return modHeightElem.asNum();
        }
    } else {
        I = modElement.find("heightoffset");
        if (I != modElement.end()) {
            const Element& modHeightElem = I->second;
            if (modHeightElem.isNum()) {
                float heightoffset = modHeightElem.asNum();
                return pos.z() + heightoffset;
            }
        }
    }
    return pos.z();
}

/**
 * @brief Common method for parsing shape data from Atlas.
 * Since each different shape expects different Atlas data this is a
 * templated method with specialized implemtations for each available shape.
 * If you call this and get error regarding missing implementations it
 * probably means that there's no implementation for the type of shape you're
 * calling it with. Note that a new shape instance will be put on the heap if
 * the parsing is successful, and it's up to the calling code to properly
 * delete it when done.
 * @param shapeElement The atlas map element which contains the shape data.
 * Often this is found with the key "shape" in the atlas data.
 * @param pos The original position of the entity to which this shape will
 * belong. The shape will be positioned according to this.
 * @param shape The resulting shape is meant to be put here, if successfully
 * created. That means that a new shape instance will be created, and it's
 * then up to the calling method to properly delete it, to avoid memory leaks.
 * @return True if the atlas data was successfully parsed and a shape was
 * created.
 */
template<template <int> class Shape>
bool InnerTerrainMod::parseShape(const Element& shapeElement,
                                 const WFMath::Point<3>& pos,
                                 const WFMath::Quaternion& orientation,
                                 Shape <2> & shape)
{
    try {
        shape.fromAtlas(shapeElement);
    } catch (...) {
        ///Just log an error and return false, this isn't fatal.
        warning() << "Error when parsing shape from atlas.";
        return false;
    }

    if (!shape.isValid()) {
        return false;
    }

    if (orientation.isValid()) {
        /// rotation about Z axis
        WFMath::Vector<3> xVec = WFMath::Vector<3>(1.0, 0.0, 0.0).rotate(orientation);
        double theta = atan2(xVec.y(), xVec.x());
        WFMath::RotMatrix<2> rm;
        shape.rotatePoint(rm.rotation(theta), WFMath::Point<2>(0, 0));
    }

    shape.shift(WFMath::Vector<2>(pos.x(), pos.y()));
    return true;
}

/**
 * @brief Tries to create a new instance from the passes in atlas data.
 * @param shapeElement The atlas data containing shape information.
 * @param pos The position where the mod should be created, in world space.
 * @param height The height where the level should be created.
 * @return True if the atlas data could be successfully parsed an a mod cre
 */
template <template <template <int> class Shape> class Mod,
          template <int> class Shape>
bool InnerTerrainMod::createInstance(
      Shape <2> & shape,
      const WFMath::Point<3>& pos,
      const MapType& modElement,
      float ,
      float )
{
    float level = parsePosition(pos, modElement);
    MapType::const_iterator I = modElement.find("slopes");
    if (I == modElement.end()) {
        error() << "SlopeTerrainMod defined without slopes";
        return false;
    }
    const Element& modSlopeElem = I->second;
    if (!modSlopeElem.isList()) {
        error() << "SlopeTerrainMod defined with malformed slopes";
        return false;
    }
    const ListType & slopes = modSlopeElem.asList();
    if (slopes.size() < 2 || !slopes[0].isNum() || !slopes[1].isNum()) {
        error() << "SlopeTerrainMod defined without slopes";
        return false;
    }
    const float dx = slopes[0].asNum();
    const float dy = slopes[1].asNum();
    m_mod = new Mod<Shape>(level, dx, dy, shape);
    return true;
}

/**
 * @brief Tries to create a new instance from the passes in atlas data.
 * @param shapeElement The atlas data containing shape information.
 * @param pos The position where the mod should be created, in world space.
 * @param level The level where the slope should be created.
 * @param dx
 * @param dy
 * @return True if the atlas data could be successfully parsed an a mod cre
 */
template <template <template <int> class S> class Mod,
          template <int> class Shape>
bool InnerTerrainMod::createInstance(
      Shape <2> & shape,
      const WFMath::Point<3>& pos,
      const MapType& modElement)
{
    float level = parsePosition(pos, modElement);
    if (m_mod != 0) {
        ((Mod<Shape>*)m_mod)->setShape(level, shape);
        return true;
    }
    m_mod = new Mod<Shape>(level, shape);
    return true;
}

TerrainMod::TerrainMod(Entity* entity)
: mEntity(entity)
, mInnerMod(0)
{
}


TerrainMod::~TerrainMod()
{
}

bool TerrainMod::init(bool alwaysObserve)
{
    bool successfulParsing = parseMod();
    if (successfulParsing || alwaysObserve) {
        observeEntity();
    }
    return successfulParsing;
}

bool TerrainMod::parseMod()
{
    if (!mEntity->hasAttr("terrainmod")) {
        ///Don't log anything since it's expected that instances of this can be attached to entities where not terrainmod is present.
        return false;
    }

    const Element& modifier(mEntity->valueOfAttr("terrainmod"));

    if (!modifier.isMap()) {
        error() << "Terrain modifier is not a map";
        return false;
    }
    const MapType & modMap = modifier.asMap();

    mInnerMod = new InnerTerrainMod;
    if (mInnerMod) {
        if (mInnerMod->parseData(mEntity->getPosition(), mEntity->getOrientation(), modMap)) {
            return true;
        } else {
            delete mInnerMod;
            return false;
        }
    }


    return false;
}

void TerrainMod::reparseMod()
{
    InnerTerrainMod* oldMod = mInnerMod;
    mInnerMod = 0;
    if (parseMod()) {
        onModChanged();
    } else {
        ///If the parsing failed and there was an old mod, we need to temporarily set the inner mod to the old one while we emit the deleted event.
        if (oldMod) {
            mInnerMod = oldMod;
            onModDeleted();
            mInnerMod = 0;
        }
    }
    delete oldMod;
}

void TerrainMod::attributeChanged(const Element& attributeValue)
{
    reparseMod();
}

void TerrainMod::entity_Moved()
{
    reparseMod();
}

void TerrainMod::entity_Deleted()
{
    onModDeleted();
    delete mInnerMod;
}

void TerrainMod::observeEntity()
{
    mAttrChangedSlot.disconnect();
    if (mEntity) {
        mAttrChangedSlot = sigc::mem_fun(*this, &TerrainMod::attributeChanged);
        mEntity->observe("terrainmod", mAttrChangedSlot);
        mEntity->Moved.connect(sigc::mem_fun(*this, &TerrainMod::entity_Moved));
        mEntity->BeingDeleted.connect(sigc::mem_fun(*this, &TerrainMod::entity_Deleted));
    }
}

Entity* TerrainMod::getEntity() const
{
    return mEntity;
}

void TerrainMod::onModDeleted()
{
    ModDeleted.emit();
}

void TerrainMod::onModChanged()
{
    ModChanged.emit();
}
} // close Namespace Eris

