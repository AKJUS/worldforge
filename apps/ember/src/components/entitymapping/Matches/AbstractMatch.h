//
// C++ Interface: AbstractMatch
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
#ifndef EMBEROGRE_MODEL_MAPPING_MATCHESABSTRACTMATCH_H
#define EMBEROGRE_MODEL_MAPPING_MATCHESABSTRACTMATCH_H

#include "MatchBase.h"
#include "../ChangeContext.h"
#include "../IVisitor.h"
#include <vector>

namespace Eris {
class Entity;
}






namespace Ember::EntityMapping::Matches {


/**
	Base class for all matches which includes templated definitions of the kind of Case it will hold.
	@author Erik Ogenvik <erik@ogenvik.org>
*/
template<class TCase>
class AbstractMatch : public MatchBase {
public:

	AbstractMatch() = default;

	~AbstractMatch() override;

	/**
	* Adds a child case.
	* @param aCase
	*/
	void addCase(std::unique_ptr<TCase> aCase);

	/**
	Gets all child cases.
	*/
	std::vector<TCase*>& getCases();

	/**
	Sets the Eris::Entity.
	*/
	void setEntity(Eris::Entity* entity) override;

	/**
	Evaluate all changes starting from this Match.
	*/
	void evaluateChanges();

	/**
	Evaluate all changes and add changed Cases to the supplied ChangeContext.
	*/
	void evaluateChanges(ChangeContext& changeContext) override;

	/**
	 * @brief Accepts a visitor.
	 * @param visitor The visitor instance.
	 */
	void accept(IVisitor& visitor) override;

protected:
	std::vector<std::unique_ptr<TCase>> mCases;
};

template<class TCase>
AbstractMatch<TCase>::~AbstractMatch() = default;


template<class TCase>
void AbstractMatch<TCase>::setEntity(Eris::Entity* entity) {
	for (auto& aCase: mCases) {
		aCase->setEntity(entity);
	}
}

template<class TCase>
std::vector<TCase*>& AbstractMatch<TCase>::getCases() {
	return mCases;
}

template<class TCase>
void AbstractMatch<TCase>::addCase(std::unique_ptr<TCase> aCase) {
	aCase->setParentCase(mParentCase);
	mCases.emplace_back(std::move(aCase));
}

template<class TCase>
void AbstractMatch<TCase>::evaluateChanges(ChangeContext& changeContext) {

	///we want to make sure that we start with deactivating actions, and then after that activate those that should be activated
	for (auto& aCase: mCases) {
		aCase->evaluateChanges(changeContext);
	}
}

template<class TCase>
void AbstractMatch<TCase>::evaluateChanges() {

	///we want to make sure that we start with deactivating actions, and then after that activate those that should be activated

	ChangeContext changeContext{};

	evaluateChanges(changeContext);

	changeContext.performActions();
}

template<class TCase>
void AbstractMatch<TCase>::accept(IVisitor& visitor) {
	visitor.visit(*this);
	for (auto& aCase: mCases) {
		aCase->accept(visitor);
	}
}


}





#endif
