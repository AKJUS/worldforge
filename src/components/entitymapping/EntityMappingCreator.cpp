//
// C++ Implementation: EntityMappingCreator
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "EntityMappingCreator.h"
#include "EntityMapping.h"
#include "EntityMappingManager.h"

#include "components/entitymapping/Cases/EntityRefCase.h"
#include "Cases/AttributeCase.h"

#include "Cases/AttributeComparers/HeightComparerWrapper.h"
#include "Cases/AttributeComparers/NumericComparer.h"
#include "Cases/AttributeComparers/NumericComparerWrapper.h"
#include "Cases/AttributeComparers/NumericEqualsComparer.h"
#include "Cases/AttributeComparers/NumericEqualsOrGreaterComparer.h"
#include "Cases/AttributeComparers/NumericEqualsOrLesserComparer.h"
#include "Cases/AttributeComparers/NumericGreaterComparer.h"
#include "Cases/AttributeComparers/NumericLesserComparer.h"
#include "Cases/AttributeComparers/NumericRangeComparer.h"
#include "Cases/AttributeComparers/StringComparer.h"
#include "Cases/AttributeComparers/StringComparerWrapper.h"

#include "Matches/Observers/EntityCreationObserver.h"
#include "Matches/VirtualAttributeMatch.h"
#include "Matches/SingleAttributeMatch.h"

#include "IActionCreator.h"

namespace Ember {


namespace EntityMapping {

using namespace Definitions;
using namespace Matches;
using namespace Observers;
using namespace Cases;
using namespace AttributeComparers;

static const CaseDefinition::ParameterEntry* findCaseParameter(const CaseDefinition::ParameterStore& parameters, const std::string& type) {
	for (auto& entry : parameters) {
		if (entry.first == type) {
			return &(entry);
		}
	}
	return nullptr;
}

EntityMappingCreator::EntityMappingCreator(EntityMappingDefinition& definition, Eris::Entity& entity, IActionCreator& actionCreator, Eris::TypeService& typeService, Eris::View* view)
		: mActionCreator(actionCreator),
		  mEntity(entity),
		  mEntityMapping(nullptr),
		  mDefinition(definition),
		  mTypeService(typeService),
		  mView(view) {
}


EntityMapping* EntityMappingCreator::create() {
	return createMapping();
}


EntityMapping* EntityMappingCreator::createMapping() {
	mEntityMapping = new EntityMapping(mEntity);

	mActionCreator.createActions(*mEntityMapping, &mEntityMapping->getBaseCase(), mDefinition.getRoot());

	for (auto& aMatch : mDefinition.getRoot().getMatches()) {
		addMatch(&mEntityMapping->getBaseCase(), aMatch);
	}

	//since we already have the entity, we can perform a check right away
	mEntityMapping->getBaseCase().setEntity(&mEntity);
	return mEntityMapping;
}

void EntityMappingCreator::addEntityTypeCases(EntityTypeMatch* entityTypeMatch, MatchDefinition& matchDefinition) {

	for (auto& aCase : matchDefinition.getCases()) {
		auto* entityCase = new EntityTypeCase();

		for (auto& paramEntry : aCase.getCaseParameters()) {
			if (paramEntry.first == "equals") {
				entityCase->addEntityType(mTypeService.getTypeByName(paramEntry.second));
			}
		}

		mActionCreator.createActions(*mEntityMapping, entityCase, aCase);

		for (auto& aMatch : aCase.getMatches()) {
			addMatch(entityCase, aMatch);
		}
		entityTypeMatch->addCase(entityCase);
		entityCase->setParentMatch(entityTypeMatch);
	}
}

void EntityMappingCreator::addEntityRefCases(EntityRefMatch* match, MatchDefinition& matchDefinition) {
	for (auto& aCase : matchDefinition.getCases()) {
		auto* entityRefCase = new EntityRefCase();

		for (auto& paramEntry : aCase.getCaseParameters()) {
			if (paramEntry.first == "equals") {
				entityRefCase->addEntityType(mTypeService.getTypeByName(paramEntry.second));
			}
		}
		mActionCreator.createActions(*mEntityMapping, entityRefCase, aCase);

		for (auto& aMatch : aCase.getMatches()) {
			addMatch(entityRefCase, aMatch);
		}
		match->addCase(entityRefCase);
		entityRefCase->setParentMatch(match);
	}
}


Cases::AttributeComparers::AttributeComparerWrapper* EntityMappingCreator::getAttributeCaseComparer(AttributeMatch* match, MatchDefinition& matchDefinition, CaseDefinition& caseDefinition) {
	const std::string& matchType = matchDefinition.getProperties()["type"];

	if ((matchType.empty()) || (matchType == "string")) {
		//default is string comparison
		if (auto param = findCaseParameter(caseDefinition.getCaseParameters(), "equals")) {
			return new AttributeComparers::StringComparerWrapper(new AttributeComparers::StringValueComparer(param->second));
		} else if (findCaseParameter(caseDefinition.getCaseParameters(), "notempty")) {
			return new AttributeComparers::StringComparerWrapper(new AttributeComparers::StringNotEmptyComparer());
		} else {
			return new AttributeComparers::StringComparerWrapper(new AttributeComparers::StringValueComparer(""));
		}
	} else if (matchType == "numeric") {
		return new AttributeComparers::NumericComparerWrapper(createNumericComparer(caseDefinition));
	} else if (matchType == "function") {
		if (match->getAttributeName() == "height") {
			return new AttributeComparers::HeightComparerWrapper(createNumericComparer(caseDefinition), mEntity);
		}
	}
	return nullptr;

}

AttributeComparers::NumericComparer* EntityMappingCreator::createNumericComparer(CaseDefinition& caseDefinition) {
	const CaseDefinition::ParameterEntry* param(nullptr);

	if ((param = findCaseParameter(caseDefinition.getCaseParameters(), "equals"))) {
		return new AttributeComparers::NumericEqualsComparer(std::stof(param->second));
	}

	//If both a min and max value is set, it's a range comparer
	AttributeComparers::NumericComparer* mMin(nullptr);
	AttributeComparers::NumericComparer* mMax(nullptr);
	if ((param = findCaseParameter(caseDefinition.getCaseParameters(), "lesser"))) {
		mMin = new AttributeComparers::NumericLesserComparer(std::stof(param->second));
	} else if ((param = findCaseParameter(caseDefinition.getCaseParameters(), "lesserequals"))) {
		mMin = new AttributeComparers::NumericEqualsOrLesserComparer(std::stof(param->second));
	}

	if ((param = findCaseParameter(caseDefinition.getCaseParameters(), "greater"))) {
		mMax = new AttributeComparers::NumericGreaterComparer(std::stof(param->second));
	} else if ((param = findCaseParameter(caseDefinition.getCaseParameters(), "greaterequals"))) {
		mMax = new AttributeComparers::NumericEqualsOrGreaterComparer(std::stof(param->second));
	}

	//check if we have both min and max set, and if so we should use a range comparer
	if (mMin && mMax) {
		return new AttributeComparers::NumericRangeComparer(mMin, mMax);
	} else if (!mMax && mMin) {
		return mMin;
	} else if (mMax) {
		return mMax;
	}
	//invalid, could not find anything to compare against
	return nullptr;
}


void EntityMappingCreator::addAttributeCases(AttributeMatch* match, MatchDefinition& matchDefinition) {
	for (auto& aCase : matchDefinition.getCases()) {
		Cases::AttributeComparers::AttributeComparerWrapper* wrapper = getAttributeCaseComparer(match, matchDefinition, aCase);
		if (wrapper) {
			auto* attrCase = new AttributeCase(wrapper);

			mActionCreator.createActions(*mEntityMapping, attrCase, aCase);

			for (auto& aMatch : aCase.getMatches()) {
				addMatch(attrCase, aMatch);
			}

			match->addCase(attrCase);
			attrCase->setParentMatch(match);
		}
	}

}

void EntityMappingCreator::addMatch(CaseBase* aCase, MatchDefinition& matchDefinition) {
	if (matchDefinition.getType() == "attribute") {
		addAttributeMatch(aCase, matchDefinition);
	} else if (matchDefinition.getType() == "entitytype") {
		addEntityTypeMatch(aCase, matchDefinition);
	} else if (matchDefinition.getType() == "entityref") {
		addEntityRefCase(aCase, matchDefinition);
	}
}

void EntityMappingCreator::addAttributeMatch(CaseBase* aCase, MatchDefinition& matchDefinition) {
	const std::string& attributeName = matchDefinition.getProperties()["attribute"];

	AttributeMatch* match = nullptr;
	std::string internalAttributeName;
	const std::string& matchType = matchDefinition.getProperties()["type"];
	//TODO: make this check better
	if (matchType == "function") {
		if (attributeName == "height") {

			auto virtualMatch = new VirtualAttributeMatch(attributeName, {"bbox", "scale"});
			virtualMatch->addMatchAttributeObserver(std::unique_ptr<MatchAttributeObserver>(new MatchAttributeObserver(virtualMatch, "bbox")));
			virtualMatch->addMatchAttributeObserver(std::unique_ptr<MatchAttributeObserver>(new MatchAttributeObserver(virtualMatch, "scale")));
			match = virtualMatch;
		}
	} else {
		auto singleMatch = new SingleAttributeMatch(attributeName);
		singleMatch->setMatchAttributeObserver(new MatchAttributeObserver(singleMatch, attributeName));
		match = singleMatch;
	}
	if (match) {
		aCase->addMatch(match);
		addAttributeCases(match, matchDefinition);

	}
}

void EntityMappingCreator::addEntityTypeMatch(CaseBase* aCase, MatchDefinition& matchDefinition) {
	auto* match = new EntityTypeMatch();
	aCase->addMatch(match);
	addEntityTypeCases(match, matchDefinition);

	//since we already have the entity, we can perform a check right away
// 	match->testEntity(mEntity);
}

void EntityMappingCreator::addEntityRefCase(CaseBase* aCase, MatchDefinition& matchDefinition) {
	if (mView) {
		const std::string& attributeName = matchDefinition.getProperties()["attribute"];
		auto* match = new EntityRefMatch(attributeName, mView);
		aCase->addMatch(match);

		addEntityRefCases(match, matchDefinition);


		//observe the attribute by the use of an MatchAttributeObserver
		auto* observer = new MatchAttributeObserver(match, attributeName);
		match->setMatchAttributeObserver(observer);

		auto* entityObserver = new EntityCreationObserver(*match);
		match->setEntityCreationObserver(entityObserver);
	}

}

}

}
