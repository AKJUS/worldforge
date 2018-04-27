// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2004 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#ifndef COMMON_PROPERTY_H
#define COMMON_PROPERTY_H

#include "OperationRouter.h"

#include <Atlas/Message/Element.h>

class LocatedEntity;
class TypeNode;

/// \brief Interface for Entity properties
///
/// \ingroup PropertyClasses
class PropertyBase {
  protected:
    /// \brief Flags indicating how this Property should be handled
    std::uint32_t m_flags;
    explicit PropertyBase(std::uint32_t flags = 0);
    PropertyBase(const PropertyBase &) = default;
  public:
    virtual ~PropertyBase() = default;

    /// \brief Accessor for Property flags
    std::uint32_t flags() const { return m_flags; }
    /// \brief Accessor for Property flags
    std::uint32_t & flags() { return m_flags; }

    void addFlags(std::uint32_t flags) { m_flags |= flags; }

    void removeFlags(std::uint32_t flags) { m_flags &= ~flags; }

    bool hasFlags(std::uint32_t flags) const { return (m_flags & flags) != 0; }

    /// \brief Install this property on an entity
    ///
    /// Called whenever an Entity gains this property for the first time
    virtual void install(LocatedEntity *, const std::string &);
    /// \brief Install this property on a type
    ///
    /// Called whenever a TypeNode gains this property for the first time
    virtual void install(TypeNode *, const std::string &);
    /// \brief Remove this property from an entity.
    ///
    /// Called whenever the property is removed or the entity is shutting down.
    virtual void remove(LocatedEntity *, const std::string & name);
    /// \brief Apply whatever effect this property has on an Entity
    ///
    /// Called whenever the value of this property should affect an Entity
    virtual void apply(LocatedEntity *);


    /// \brief Copy the value of the property into an Atlas Message
    virtual int get(Atlas::Message::Element & val) const = 0;
    /// \brief Read the value of the property from an Atlas Message
    virtual void set(const Atlas::Message::Element & val) = 0;
    /// \brief Add the value as an attribute to an Atlas map
    virtual void add(const std::string & key, Atlas::Message::MapType & map) const;
    /// \brief Add the value as an attribute to an Atlas entity
    virtual void add(const std::string & key, const Atlas::Objects::Entity::RootEntity & ent) const;
    /// \brief Handle an operation
    virtual HandlerResult operation(LocatedEntity *,
                                    const Operation &,
                                    OpVector &);
    /// \brief Create a copy of this instance
    ///
    /// The copy should have exactly the same type, and the same value
    virtual PropertyBase * copy() const = 0;
};

/// \brief Flag indicating data has been written to permanent store
/// \ingroup PropertyFlags
static const std::uint32_t per_clean = 1u << 0u;
/// \brief Flag indicating data should never be persisted
/// \ingroup PropertyFlags
static const std::uint32_t per_ephem = 1u << 1u;
/// \brief Flag indicating data has been stored initially
/// \ingroup PropertyFlags
static const std::uint32_t per_seen = 1u << 2u;

/// \brief Flag mask indicating data should not be written to store
/// \ingroup PropertyFlags
static const std::uint32_t per_mask = per_clean | per_ephem;

/// \brief Flag indicating data is not visible
/// \ingroup PropertyFlags
static const std::uint32_t vis_hidden = 1u << 3u;
/// \brief Flag indicating data is server internal
/// \ingroup PropertyFlags
static const std::uint32_t vis_internal = 1u << 4u;

/// \brief Flag mask indicating data should be be perceptable
/// \ingroup PropertyFlags
static const std::uint32_t vis_mask = vis_hidden | vis_internal;

/// \brief Flag set to indicate this is a class property, and has no instance
/// \ingroup PropertyFlags
static const std::uint32_t flag_class = 1u << 5u;

/// \brief Flag used for boolean properties
/// \ingroup PropertyFlags
static const std::uint32_t flag_bool = 1u << 6u;

/// \brief Flag used to mark properties whose state has not been broadcast
/// \ingroup PropertyFlags
static const std::uint32_t flag_unsent = 1u << 7u;

/// \brief Flag used to mark properties which must be instance properties
/// \ingroup PropertyFlags
/// Typically this will be because they have per-entity state which cannot
/// be handled on a class property.
static const std::uint32_t flag_instance = 1u << 8u;

/// \brief Entity property template for properties with single data values
/// \ingroup PropertyClasses
template <typename T>
class Property : public PropertyBase {
  protected:
    /// \brief Reference to variable holding the value of this Property
    T m_data;
    Property(const Property<T> &) = default;
  public:
    static const std::string property_atlastype;

    explicit Property(unsigned int flags = 0);

    const T & data() const { return this->m_data; }
    T & data() { return this->m_data; }

    int get(Atlas::Message::Element & val) const override;

    void set(const Atlas::Message::Element &) override;

    void add(const std::string & key, Atlas::Message::MapType & map) const override;
    void add(const std::string & key, const Atlas::Objects::Entity::RootEntity & ent) const override;
    Property<T> * copy() const override;
};

/// \brief Entity property that can store any Atlas value
/// \ingroup PropertyClasses
class SoftProperty : public PropertyBase {
  protected:
    Atlas::Message::Element m_data;
  public:
    SoftProperty() = default;
    explicit SoftProperty(const Atlas::Message::Element & data);

    int get(Atlas::Message::Element & val) const override ;
    void set(const Atlas::Message::Element & val) override ;
    SoftProperty * copy() const override;
};

class BoolProperty : public PropertyBase {
public:
    static constexpr const char* property_atlastype = "int";

    explicit BoolProperty() = default;

    int get(Atlas::Message::Element & val) const override;
    void set(const Atlas::Message::Element & val) override;
    BoolProperty * copy() const override;

    bool isTrue() const;

};

#endif // COMMON_PROPERTY_H
