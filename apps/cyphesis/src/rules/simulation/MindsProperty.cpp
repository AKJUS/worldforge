/*
 Copyright (C) 2018 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "MindsProperty.h"
#include "TransientProperty.h"
#include "UsagesProperty.h"
#include "FilterProperty.h"

#include "rules/simulation/BaseWorld.h"
#include "rules/BBoxProperty_impl.h"
#include "rules/simulation/LocatedEntity.h"
#include "rules/ScaleProperty_impl.h"
#include "rules/entityfilter/Filter_impl.h"

#include "Inheritance.h"
#include "common/Router.h"
#include "common/debug.h"
#include "common/custom.h"

#include "common/operations/Tick.h"
#include "common/operations/Thought.h"

#include <Atlas/Objects/RootEntity.h>
#include <Atlas/Objects/Anonymous.h>
#include <Atlas/Objects/Operation.h>
#include <wfmath/atlasconv.h>
#include <iostream>
#include <algorithm>


using Atlas::Message::Element;
using Atlas::Message::ListType;
using Atlas::Message::MapType;
using Atlas::Objects::Root;
using Atlas::Objects::Operation::Info;
using Atlas::Objects::Operation::Set;
using Atlas::Objects::Operation::Sight;
using Atlas::Objects::Operation::Sound;
using Atlas::Objects::Operation::Tick;
using Atlas::Objects::Operation::Move;
using Atlas::Objects::Operation::Action;
using Atlas::Objects::Operation::Unseen;
using Atlas::Objects::Operation::Wield;
using Atlas::Objects::Entity::Anonymous;
using Atlas::Objects::Entity::RootEntity;

using Atlas::Objects::smart_dynamic_cast;

static constexpr bool debug_flag = false;

MindsProperty::MindsProperty()
	: PropertyBase(prop_flag_persistence_ephem | prop_flag_instance) {
}

MindsProperty::~MindsProperty() = default;

void MindsProperty::set(const Element& val) {
	//Don't allow setting.
}

int MindsProperty::get(Element& val) const {
	ListType list;
	for (auto& mind: m_data) {
		list.emplace_back(mind->getIdAsString());
	}
	val = list;
	return 0;
}

void MindsProperty::add(const std::string& val, MapType& map) const {
	ListType list;
	for (auto& mind: m_data) {
		list.emplace_back(mind->getIdAsString());
	}
	map[val] = list;
}

void MindsProperty::add(const std::string& val, const RootEntity& ent) const {
	ListType list;
	for (auto& mind: m_data) {
		list.emplace_back(mind->getIdAsString());
	}
	ent->setAttr(val, list);
}

MindsProperty* MindsProperty::copy() const {
	//Don't copy any values
	return new MindsProperty();
}

void MindsProperty::install(LocatedEntity& entity, const std::string& name) {
	entity.addListener(this);
}

void MindsProperty::remove(LocatedEntity& entity, const std::string& name) {
	entity.removeListener(this);
}


HandlerResult MindsProperty::operation(LocatedEntity& ent, const Operation& op, OpVector& res) {
	if (op->getClassNo() == Atlas::Objects::Operation::THOUGHT_NO) {
		return ThoughtOperation(ent, op, res);
	} else if (op->getClassNo() == Atlas::Objects::Operation::RELAY_NO) {
		return RelayOperation(ent, op, res);
	} else {
		if (world2mind(op)) {
			cy_debug_print("MindsProperty::operation(" << op->getParent() << ") passed to mind")
			OpVector mres;
			sendToMinds(op, mres);
			for (auto& resOp: mres) {
				//Wrap any returning ops in thoughts and send them to our entity
				Atlas::Objects::Operation::Thought thought;
				thought->setArgs1(resOp);
				thought->setTo(ent.getIdAsString());
				ent.sendWorld(thought);
			}
		}
	}

	return OPERATION_HANDLED;
}

void MindsProperty::sendToMinds(const Operation& op, OpVector& res) const {
	for (auto& mind: m_data) {
		mind->operation(op, res);
	}
}


HandlerResult MindsProperty::RelayOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) {
	if (op->isDefaultTo()) {
		ent.error(op, "A relay op must have a 'to'.", res, ent.getIdAsString());
		return OPERATION_BLOCKED;
	}
	if (op->isDefaultFrom()) {
		ent.error(op, "A relay op must have a 'from'.", res, ent.getIdAsString());
		return OPERATION_BLOCKED;
	}
	if (op->isDefaultId()) {
		ent.error(op, "A relay op must have an 'id'.", res, ent.getIdAsString());
		return OPERATION_BLOCKED;
	}

	//Get the mind to which the relay is directed.
	auto mindId = op->getId();

	for (auto& mind: m_data) {
		if (mind->getIdAsString() == mindId) {
			OpVector mres;
			mind->operation(op, res);
			for (auto& resOp: mres) {
				//Wrap any returning ops in thoughts and send them to our entity
				Atlas::Objects::Operation::Thought thought;
				thought->setArgs1(resOp);
				thought->setTo(ent.getIdAsString());
				res.push_back(std::move(resOp));
			}
			return OPERATION_BLOCKED;
		}
	}

	return OPERATION_BLOCKED;
}

HandlerResult MindsProperty::ThoughtOperation(LocatedEntity& ent, const Operation& op, OpVector& res) const {
	for (auto& arg: op->getArgs()) {
		if (auto innerOp = smart_dynamic_cast<Operation>(arg)) {
			OpVector mres;
			mind2body(ent, innerOp, mres);

			// If the original op had a serial no, we assume the first consequence
			// of that is effectively the same operation.
			// FIXME Can this be guaranteed by the mind2body phase?
			if (!op->isDefaultSerialno()) {
				if (!mres.empty() && mres.front()->isDefaultSerialno()) {
					mres.front()->setSerialno(op->getSerialno());
				}
			}

			for (auto& resOp: mres) {
				resOp->setFrom(ent.getIdAsString());
				res.push_back(std::move(resOp));
			}
		}
	}
	return OPERATION_BLOCKED;
}

/// \brief Filter a Use operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindUseOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	cy_debug_print("Got Use op from mind")

	//Make sure that the first contained arg is another Use operation,
	// which is then sent to the actual tool.

	auto& args = op->getArgs();
	if (args.empty()) {
		spdlog::error("mindUseOperation: use op has no arguments. " + ent.describeEntity());
		return;
	}

	Atlas::Objects::Operation::Use useOp;
	useOp->setFrom(ent.getIdAsString());
	auto firstArg = args.front();
	if (firstArg->getObjtype() == "op") {
		//TODO: should we perhaps check that this only can be Action ops?
		auto innerOp = smart_dynamic_cast<Atlas::Objects::Operation::RootOperation>(args.front());
		if (!innerOp) {
			spdlog::error("mindUseOperation: Second arg is not an operation. " + ent.describeEntity());
			return;
		}

		auto& innerArgs = innerOp->getArgs();
		if (innerArgs.empty()) {
			spdlog::error("mindUseOperation: inner use op has no arguments. " + ent.describeEntity());
			return;
		}

		auto toolEnt = smart_dynamic_cast<Atlas::Objects::Entity::RootEntity>(innerArgs.front());
		if (!toolEnt) {
			spdlog::error("mindUseOperation: First inner arg is not an entity. " + ent.describeEntity());
			return;
		}
		if (!toolEnt->hasAttrFlag(Atlas::Objects::ID_FLAG)) {
			spdlog::error("mindMoveOperation: First inner arg has no ID. " + ent.describeEntity());
			return;
		}

		useOp->setTo(toolEnt->getId());
		useOp->setArgs1(innerOp);
	} else if (firstArg->getObjtype() == "task") {
		useOp->setTo(ent.getIdAsString());
		useOp->setArgs(op->getArgs());
	}

	res.push_back(useOp);
}

/// \brief Filter a Wield operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindWieldOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	cy_debug_print("Got Wield op from mind")
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Move operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindMoveOperation(LocatedEntity& ent, const Operation& op, OpVector& res) const {
	cy_debug_print("MindsProperty::mind_move_op")
	const std::vector<Root>& args = op->getArgs();
	if (args.empty()) {
		spdlog::error("mindMoveOperation: move op has no argument. " + ent.describeEntity());
		return;
	}
	const auto arg = smart_dynamic_cast<RootEntity>(args.front());
	if (!arg.isValid()) {
		spdlog::error("mindMoveOperation: Arg is not an entity. " + ent.describeEntity());
		return;
	}
	if (!arg->hasAttrFlag(Atlas::Objects::ID_FLAG)) {
		spdlog::error("mindMoveOperation: Arg has no ID. " + ent.describeEntity());
		return;
	}
	const std::string& other_id = arg->getId();
	if (other_id != ent.getIdAsString()) {
		moveOtherEntity(ent, op, res, arg, other_id);
	} else {
		ent.clientError(op, "You can't move yourself.", res, ent.getIdAsString());
	}
}


void MindsProperty::moveOtherEntity(LocatedEntity& ent, const Operation& op, OpVector& res, const RootEntity& arg, const std::string& other_id) const {
	cy_debug_print("Moving something else. " << other_id)
	auto other = BaseWorld::instance().getEntity(other_id);
	if (!other) {
		Unseen u;

		Anonymous unseen_arg;
		unseen_arg->setId(other_id);
		u->setArgs1(unseen_arg);

		u->setTo(ent.getIdAsString());
		res.push_back(u);
		return;
	}

	auto moverConstraint = ent.getPropertyClass<FilterProperty>("mover_constraint");
	if (moverConstraint && moverConstraint->getData()) {
		std::vector<std::string> errorMessages;
		EntityFilter::QueryContext queryContext{.entityLoc = EntityFilter::QueryEntityLocation{*other}, .actor = &ent};
		queryContext.entity_lookup_fn = [](const std::string& id) { return BaseWorld::instance().getEntity(id); };
		queryContext.type_lookup_fn = [](const std::string& id) { return Inheritance::instance().getType(id); };
		queryContext.report_error_fn = [&errorMessages](const std::string& message) { errorMessages.emplace_back(message); };
		if (!moverConstraint->getData()->match(queryContext)) {
			auto message = errorMessages.empty() ? "You can't move this entity." : errorMessages.front();
			ent.clientError(op, message, res, ent.getIdAsString());
			return;
		}
	}

	auto moveConstraint = other->getPropertyClass<FilterProperty>("move_constraint");
	if (moveConstraint && moveConstraint->getData()) {
		std::vector<std::string> errorMessages;
		EntityFilter::QueryContext queryContext{.entityLoc = EntityFilter::QueryEntityLocation{*other}, .actor = &ent};
		queryContext.entity_lookup_fn = [](const std::string& id) { return BaseWorld::instance().getEntity(id); };
		queryContext.type_lookup_fn = [](const std::string& id) { return Inheritance::instance().getType(id); };
		queryContext.report_error_fn = [&errorMessages](const std::string& message) { errorMessages.emplace_back(message); };
		if (!moveConstraint->getData()->match(queryContext)) {
			auto message = errorMessages.empty() ? "You can't move this entity." : errorMessages.front();
			ent.clientError(op, message, res, ent.getIdAsString());
			return;
		}
	}

	if (other->m_parent) {
		auto containConstraint = other->m_parent->getPropertyClass<FilterProperty>("contain_constraint");
		if (containConstraint && containConstraint->getData()) {
			std::vector<std::string> errorMessages;
			EntityFilter::QueryContext queryContext{.entityLoc = EntityFilter::QueryEntityLocation{*other}, .actor = &ent, .tool = other->m_parent};
			queryContext.entity_lookup_fn = [](const std::string& id) { return BaseWorld::instance().getEntity(id); };
			queryContext.type_lookup_fn = [](const std::string& id) { return Inheritance::instance().getType(id); };
			queryContext.report_error_fn = [&errorMessages](const std::string& message) { errorMessages.emplace_back(message); };
			if (!containConstraint->getData()->match(queryContext)) {
				auto message = errorMessages.empty() ? "You can't move this entity." : errorMessages.front();
				ent.clientError(op, message, res, ent.getIdAsString());
				return;
			}
		}
	}

	//Check that we actually can reach the other entity.
	if (ent.canReach({other, {}})) {
		//Now also check that we can reach wherever we're trying to move the entity.

		Ref<LocatedEntity> targetLoc = other->m_parent;

		//Only allow some things to be set when moving another entity.
		RootEntity newArgs1;
		//We've already checked that the id exists
		newArgs1->setId(arg->getId());

		if (!arg->isDefaultLoc()) {
			newArgs1->setLoc(arg->getLoc());
			targetLoc = BaseWorld::instance().getEntity(arg->getLoc());
		}
		if (!targetLoc) {
			ent.clientError(op, "Target parent entity doesn't exist.", res, op->getFrom());
			return;
		}

		auto destinationConstraint = targetLoc->getPropertyClass<FilterProperty>("destination_constraint");
		if (destinationConstraint && destinationConstraint->getData()) {
			std::vector<std::string> errorMessages;
			EntityFilter::QueryContext queryContext{.entityLoc = EntityFilter::QueryEntityLocation{*other}, .actor = &ent, .tool = targetLoc.get()};
			queryContext.entity_lookup_fn = [](const std::string& id) { return BaseWorld::instance().getEntity(id); };
			queryContext.type_lookup_fn = [](const std::string& id) { return Inheritance::instance().getType(id); };
			queryContext.report_error_fn = [&errorMessages](const std::string& message) { errorMessages.emplace_back(message); };
			if (!destinationConstraint->getData()->match(queryContext)) {
				auto message = errorMessages.empty() ? "You can't move this entity." : errorMessages.front();
				ent.clientError(op, message, res, ent.getIdAsString());
				return;
			}
		}


		WFMath::Point<3> targetPos;
		if (!arg->isDefaultPos()) {
			newArgs1->setPos(arg->getPos());
			targetPos.fromAtlas(arg->getPosAsList());
		}
		auto bbox = ScaleProperty<LocatedEntity>::scaledBbox(*other);
		auto radius = bbox.isValid() ? bbox.boundingSphere().radius() : 0;
		//Check that we can reach the edge of the entity if it's placed in its new location.
		if (!ent.canReach({targetLoc, targetPos}, (float)radius)) {
			ent.clientError(op, "Target is too far away.", res, op->getFrom());
			return;
		}
		if (arg->hasAttr("orientation")) {
			newArgs1->setAttr("orientation", arg->getAttr("orientation"));
		}
		if (arg->hasAttr("amount")) {
			newArgs1->setAttr("amount", arg->getAttr("amount"));
		}
		//Replace first arg with our sanitized arg.
		op->setArgs1(std::move(newArgs1));
		op->setFrom(ent.getIdAsString());
		//Send the op to the current location of the entity being moved
		op->setTo(other->m_parent->getIdAsString());

		res.push_back(op);

	} else {
		ent.clientError(op, "Entity is too far away.", res, op->getFrom());
	}
}

/// \brief Filter a Set operation coming from the mind
///
/// Currently any Set op is permitted. In the future this will be locked
/// down to only allow mutable things to be changed. For example, for
/// inventory items with no name can have their name set from the client.
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindSetOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	const std::vector<Root>& args = op->getArgs();
	if (args.empty()) {
		spdlog::error("mindSetOperation: set op has no argument. " + ent.describeEntity());
		return;
	}

	const Root& arg = args.front();
	//Clean op
	Set setOp;
	setOp->setTo(ent.getIdAsString());
	setOp->setFrom(ent.getIdAsString());
	if (!op->isDefaultSerialno()) {
		setOp->setSerialno(op->getSerialno());
	}
	Anonymous cleanedArg;
	cleanedArg->setId(ent.getIdAsString());
	for (auto attrs = arg->asMessage(); auto entry: attrs) {
		if (entry.first == "_propel") {
			try {
				Vector3D new_propel;
				new_propel.fromAtlas(entry.second);
				if (new_propel.isValid()) {
					auto mag = new_propel.mag();

					//We don't allow the mind to set any speed greater than a normalized value.
					if (mag > 1.0) {
						new_propel.normalize();
					}
					cleanedArg->setAttr(entry.first, new_propel.toAtlas());
				}
			} catch (...) {
				//just ignore malformed data
			}

		} else if (entry.first == "_direction") {
			cleanedArg->setAttr(entry.first, std::move(entry.second));
		} else if (entry.first == "_destination") {
			cleanedArg->setAttr(entry.first, std::move(entry.second));
		} else if (entry.first == "id") {
			//no-op
		} else {
			spdlog::error("mindSetOperation: set op tried to set non-allowed property '{}' on entity {}. ",
				entry.first,
				ent.describeEntity());
		}
	}
	setOp->setArgs1(std::move(cleanedArg));

	res.push_back(std::move(setOp));
}

/// \brief Filter a Create operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindCreateOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Delete operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindDeleteOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Imaginary operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindImaginaryOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Talk operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindTalkOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	cy_debug_print("MindsProperty::mindTalkOperation")
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Look operation coming from the mind
///
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindLookOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	cy_debug_print("Got look up from mind from [" << op->getFrom() << "] to [" << op->getTo() << "]")

	const std::vector<Root>& args = op->getArgs();
	if (args.empty()) {
		//If nothing is specified, send to parent, if available.
		if (ent.m_parent) {
			op->setTo(ent.m_parent->getIdAsString());
		} else {
			return;
		}
	} else {
		//TODO: handle multiple entities being looked at in one op, up to some limit.
		const Root& arg = args.front();
		if (arg->isDefaultId()) {
			spdlog::error(ent.describeEntity() + " mindLookOperation: Op has no ID");
			return;
		}
		op->setTo(arg->getId());
	}
	cy_debug_print("  now to [" << op->getTo() << "]")
	res.push_back(op);
}

/// \brief Filter a GoalInfo operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindGoalInfoOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Touch operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindTouchOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	// Work out what is being touched.
	const std::vector<Root>& args = op->getArgs();
	if (args.empty()) {
		spdlog::error("mindTouchOperation: Op has no ARGS");
		return;
	}
	auto arg = smart_dynamic_cast<Atlas::Objects::Entity::Anonymous>(args.front());
	if (!arg->hasAttrFlag(Atlas::Objects::ID_FLAG)) {
		spdlog::error(ent.describeEntity() + " mindTouchOperation: Op has no ID");
		return;
	}

	WFMath::Point<3> pos;
	if (arg->hasAttrFlag(Atlas::Objects::Entity::POS_FLAG)) {
		pos.fromAtlas(arg->getPosAsList());
	}

	auto other = BaseWorld::instance().getEntity(arg->getId());

	//Check that we actually can reach the other entity.
	if (ent.canReach({std::move(other), pos})) {
		// Pass the modified touch operation on to target.
		op->setTo(arg->getId());
		res.push_back(op);
		// Send sight of touch
		Sight s;
		s->setArgs1(op);
		res.push_back(s);
	} else {
		ent.clientError(op, "Entity is too far away.", res, op->getFrom());
	}


}

/// \brief Filter any other operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindOtherOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	spdlog::warn("Passing '{}' op from mind through to world. {}", op->getParent(), ent.describeEntity());
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Thought operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindThoughtOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter a Think operation coming from the mind
///
/// @param ent Active entity.
/// @param op The operation to be filtered.
/// @param res The filtered result is returned here.
void MindsProperty::mindThinkOperation(const LocatedEntity& ent, const Operation& op, OpVector& res) const {
	op->setTo(ent.getIdAsString());
	res.push_back(op);
}

/// \brief Filter operations from the mind destined for the body.
///
/// Operations from the character's mind which is either an NPC mind,
/// or a remote client are passed in here for pre-processing and filtering
/// before they are valid to be processed as internal ops. The operation
/// may be modified and re-used so operations passed to this function have
/// their ownership passed in, and caller should not modify the operation,
/// make assumptions that it has not been modified after calling mind2body.
/// @param ent Active entity.
/// @param op The operation to be processed.
/// @param res The result of the operation is returned here.
void MindsProperty::mind2body(LocatedEntity& ent, const Operation& op, OpVector& res) const {
	cy_debug_print("MindsProperty::mind2body(" << op->getParent() << ") " << ent.describeEntity())

	if (!op->isDefaultTo()) {
		spdlog::error("Operation \"{}\" from mind with TO set. {}", op->getParent(), ent.describeEntity());
		return;
	}
	if (!op->isDefaultFutureMilliseconds() && op->getClassNo() != Atlas::Objects::Operation::TICK_NO) {
		spdlog::error("Operation \"{}\" from mind with "
			"FUTURE_MILLISECONDS set. {}",
			op->getParent(),
			ent.describeEntity());
	}
	switch (auto op_no = op->getClassNo()) {
	case Atlas::Objects::Operation::CREATE_NO:
		mindCreateOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::DELETE_NO:
		mindDeleteOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::IMAGINARY_NO:
		mindImaginaryOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::LOOK_NO:
		mindLookOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::MOVE_NO:
		mindMoveOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::SET_NO:
		mindSetOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::TALK_NO:
		mindTalkOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::TOUCH_NO:
		mindTouchOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::USE_NO:
		mindUseOperation(ent, op, res);
		break;
	case Atlas::Objects::Operation::WIELD_NO:
		mindWieldOperation(ent, op, res);
		break;
	default:
		if (op_no == Atlas::Objects::Operation::THOUGHT_NO) {
			mindThoughtOperation(ent, op, res);
		} else if (op_no == Atlas::Objects::Operation::GOAL_INFO_NO) {
			mindGoalInfoOperation(ent, op, res);
		} else if (op_no == Atlas::Objects::Operation::THINK_NO) {
			mindThinkOperation(ent, op, res);
		} else {
			mindOtherOperation(ent, op, res);
		}
		break;
	}
}


/// \brief Filter operations from the world to the mind
///
/// Operations from the world are checked here to see if they are suitable
/// to send to the mind. Some operations should not go to the mind as they
/// leak information. Others are just not necessary as they provide no
/// useful information.
bool MindsProperty::world2mind(const Operation& op) const {
	return !op->instanceOf(Atlas::Objects::Operation::RELAY_NO) && !op->instanceOf(Atlas::Objects::Operation::GET_NO);
}

void MindsProperty::addMind(Router* mind) {
	m_data.push_back(mind);
}

void MindsProperty::removeMind(const Router* mind, LocatedEntity& entity) {
	auto I = std::find(m_data.begin(), m_data.end(), mind);
	if (I != m_data.end()) {
		m_data.erase(I);
	}

	//If there are no more minds controlling we should stop all propelling movement.
	if (m_data.empty()) {
		// Send a move op stopping the current movement
		Anonymous move_arg;
		move_arg->setId(entity.getIdAsString());
		move_arg->setAttr("_propel", Vector3D::ZERO().toAtlas());

		Set setOp;
		setOp->setFrom(entity.getIdAsString());
		setOp->setTo(entity.getIdAsString());
		setOp->setArgs1(std::move(move_arg));
		entity.sendWorld(std::move(setOp));
	}
}

const std::vector<Router*>& MindsProperty::getMinds() const {
	return m_data;
}
