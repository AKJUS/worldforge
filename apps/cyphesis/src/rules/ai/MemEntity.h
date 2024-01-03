// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000,2001 Alistair Riddoch
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


#ifndef RULESETS_MEM_ENTITY_H
#define RULESETS_MEM_ENTITY_H

#include "rules/LocatedEntity.h"

/// \brief This class is used to represent entities inside MemMap used
/// by the mind of an AI.
///
/// It adds a flag to indicate if this entity is currently visible, and
/// a means of tracking when it was last seen, so garbage entities can
/// be cleaned up.
class MemEntity : public LocatedEntity {
protected:
	std::chrono::milliseconds m_lastSeen;

	std::unique_ptr<PropertyBase> createProperty(const std::string& propertyName) const override;

public:
	explicit MemEntity(RouterId id);

	~MemEntity() override = default;

	void setVisible(bool v = true) {
		if (v) {
			m_flags.addFlags(entity_visible);
		} else {
			m_flags.removeFlags(entity_visible);
		}
	}

	std::chrono::milliseconds lastSeen() const {
		return m_lastSeen;
	}

	void update(std::chrono::milliseconds d) {
		if (d >= m_lastSeen) {
			m_lastSeen = d;
		}
	}

	void externalOperation(const Operation& op, Link&) override;

	void operation(const Operation&, OpVector&) override;

	void destroy() override;

//        TransformData m_transform;
//        MovementData m_movement;
	//WFMath::AxisBox<3> m_bbox;
	std::chrono::milliseconds m_lastUpdated;
	//Location m_location;

};

#endif // RULESETS_MEM_ENTITY_H
