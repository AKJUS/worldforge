//
// C++ Interface: MatchBase
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
#ifndef EMBEROGRE_MODEL_MAPPING_MATCHESMATCHBASE_H
#define EMBEROGRE_MODEL_MAPPING_MATCHESMATCHBASE_H

#include "../Cases/CaseBase.h"

namespace Eris {
class Entity;
}

namespace Ember::EntityMapping {

struct IVisitor;

namespace Matches {

/**
	@author Erik Ogenvik <erik@ogenvik.org>
*/

class MatchBase {
public:

	MatchBase() : mParentCase(nullptr) {}

	virtual ~MatchBase() = default;

	void setParentCase(Cases::CaseBase* aCase);

	virtual void setEntity(Eris::Entity* entity) = 0;

	virtual void evaluateChanges(ChangeContext& changeContext) = 0;

	/**
	 * @brief Accepts a visitor.
	 * After calling visit() on the visitor the child cases will be traversed.
	 * @param visitor The visitor instance.
	 */
	virtual void accept(IVisitor& visitor) = 0;

protected:
	Cases::CaseBase* mParentCase;
};

inline void MatchBase::setParentCase(Cases::CaseBase* aCase) {
	mParentCase = aCase;
}

}

}



#endif
