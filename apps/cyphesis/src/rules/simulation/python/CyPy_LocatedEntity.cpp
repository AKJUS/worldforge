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

#include "CyPy_LocatedEntity.h"

#include <rules/python/CyPy_RootEntity.h>
#include <rules/simulation/Inheritance.h>

#include "rules/python/EntityHelper.h"
#include "rules/python/PythonWrapper_impl.h"
#include "rules/simulation/ActionsProperty.h"
#include "CyPy_Task.h"
#include "CyPy_EntityProps.h"
#include "rules/simulation/TasksProperty.h"
#include "rules/entityfilter/python/CyPy_EntityFilter_impl.h"
#include "rules/entityfilter/Providers.h"
#include "rules/simulation/BaseWorld.h"
#include "rules/python/CyPy_Operation.h"
#include "rules/python/CyPy_Oplist.h"
#include "common/id.h"
#include "CyPy_Domain.h"
#include "common/id.h"
#include "CyPy_LocatedEntity_impl.h"

template<>
Py::Object wrapPython(LocatedEntity* value) {
	return CyPy_LocatedEntity::wrap(value);
}

template<>
Py::Object wrapEntity(Ref<LocatedEntity> le) {
	return wrapLocatedEntity(le);
}

Py::Object wrapLocatedEntity(Ref<LocatedEntity> le) {
	//If there's already a script entity use that (as a cache mechanism)
	if (le->m_scriptEntity.has_value()) {
		if (const auto wrapper = std::any_cast<Py::Object>(le->m_scriptEntity); !wrapper.isNone()) {
			if (const auto object = PyWeakref_GetObject(wrapper.ptr())) {
				if (Py::Object pythonObj(object); !pythonObj.isNone()) {
					return pythonObj;
				}
			}
		}
	}
	auto wrapped =  WrapperBase<Ref<LocatedEntity>, CyPy_LocatedEntity, Py::PythonClassInstanceWeak>::wrap(le);

	auto weakPtr = PyWeakref_NewRef(wrapped.ptr(), nullptr);
	le->m_scriptEntity = std::any(Py::Object(weakPtr, true));

	return wrapped;
}


Py::Object CyPy_LocatedEntity::wrap(Ref<LocatedEntity> value) {
	return wrapLocatedEntity(std::move(value));
}

CyPy_LocatedEntity::CyPy_LocatedEntity(Py::PythonClassInstanceWeak* self, Py::Tuple& args, Py::Dict& kwds)
	: CyPy_LocatedEntityBase(self, args, kwds) {
	args.verify_length(1);

	auto arg = args.front();
	if (arg.isString()) {
		auto id = verifyString(args.front());

		long intId = integerId(id);
		if (intId == -1L) {
			throw Py::TypeError("Entity() requires string/int ID");
		}
		m_value = new LocatedEntity(RouterId{intId});
	} else if (CyPy_LocatedEntity::check(arg)) {
		m_value = CyPy_LocatedEntity::value(arg);
	} else {
		throw Py::TypeError("Entity() requires string ID or Entity");
	}
}


CyPy_LocatedEntity::CyPy_LocatedEntity(Py::PythonClassInstanceWeak* self, Ref<LocatedEntity> value)
	: CyPy_LocatedEntityBase(self, std::move(value)) {
}

CyPy_LocatedEntity::~CyPy_LocatedEntity() = default;


void CyPy_LocatedEntity::init_type() {
	behaviors().name("LocatedEntity");
	behaviors().doc("");

	behaviors().supportRichCompare();
	behaviors().supportStr();


	PYCXX_ADD_VARARGS_METHOD(get_child, get_child, "");
	PYCXX_ADD_NOARGS_METHOD(as_entity, as_entity, "");
	PYCXX_ADD_VARARGS_METHOD(can_reach, can_reach, "");
	PYCXX_ADD_NOARGS_METHOD(describe_entity, describe_entity, "");
	PYCXX_ADD_VARARGS_METHOD(client_error, client_error, "");
	PYCXX_ADD_VARARGS_METHOD(is_type, is_type, "");

	PYCXX_ADD_VARARGS_METHOD(get_prop_num, get_prop_num, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_num, has_prop_num, "");
	PYCXX_ADD_VARARGS_METHOD(get_prop_float, get_prop_float, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_float, has_prop_float, "");
	PYCXX_ADD_VARARGS_METHOD(get_prop_int, get_prop_int, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_int, has_prop_int, "");
	PYCXX_ADD_VARARGS_METHOD(get_prop_string, get_prop_string, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_string, has_prop_string, "");
	PYCXX_ADD_VARARGS_METHOD(get_prop_bool, get_prop_bool, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_bool, has_prop_bool, "");
	PYCXX_ADD_VARARGS_METHOD(get_prop_list, get_prop_list, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_list, has_prop_list, "");
	PYCXX_ADD_VARARGS_METHOD(get_prop_map, get_prop_map, "");
	PYCXX_ADD_VARARGS_METHOD(has_prop_map, has_prop_map, "");


	PYCXX_ADD_VARARGS_METHOD(send_world, send_world, "");
	PYCXX_ADD_VARARGS_METHOD(mod_property, mod_property, "");
	PYCXX_ADD_VARARGS_METHOD(start_task, start_task, "");
	PYCXX_ADD_VARARGS_METHOD(start_action,
		start_action,
		"Starts a new action. First parameter is the action name, and the second is the duration. Since it's not tied to a task a duration is required.");
	PYCXX_ADD_NOARGS_METHOD(update_task, update_task, "");
	PYCXX_ADD_VARARGS_METHOD(find_in_contains, find_in_contains, "Returns a list of all contained entities that matches the supplied Entity Filter.");
	PYCXX_ADD_NOARGS_METHOD(get_parent_domain, get_parent_domain, "Gets the parent domain, i.e. the domain to which this entity belongs.");

	PYCXX_ADD_VARARGS_METHOD(create_new_entity, create_new_entity, "Creates a new entity.");


	behaviors().readyType();

}


Py::Object CyPy_LocatedEntity::start_task(const Py::Tuple& args) {
	return CyPy_LocatedEntity::start_task(m_value, args);
}

Py::Object CyPy_LocatedEntity::start_action(const Py::Tuple& args) {
	return CyPy_LocatedEntity::start_action(m_value, args);
}

Py::Object CyPy_LocatedEntity::update_task() {
	return CyPy_LocatedEntity::update_task(m_value);
}

Py::Object CyPy_LocatedEntity::mod_property(const Py::Tuple& args) {
	return CyPy_LocatedEntity::mod_property(m_value, args);
}


Py::Object CyPy_LocatedEntity::send_world(const Py::Tuple& args) {
	return CyPy_LocatedEntity::send_world(m_value, args);
}


Py::Object CyPy_LocatedEntity::send_world(const Ref<LocatedEntity>& entity, const Py::Tuple& args) {
	args.verify_length(1);
	entity->sendWorld(verifyObject<CyPy_Operation>(args.front()));
	return Py::None();
}

Py::Object CyPy_LocatedEntity::start_task(const Ref<LocatedEntity>& entity, const Py::Tuple& args) {
	OpVector res;
	args.verify_length(2);

	auto& tp = entity->requirePropertyClassFixed<TasksProperty>();
	tp.startTask(verifyString(args[0]), verifyObject<CyPy_Task>(args[1]), *entity, res);

	return CyPy_Oplist::wrap(std::move(res));
}

Py::Object CyPy_LocatedEntity::update_task(const Ref<LocatedEntity>& entity) {
	OpVector res;

	auto& tp = entity->requirePropertyClassFixed<TasksProperty>();
	tp.updateTask(*entity, res);

	return CyPy_Oplist::wrap(std::move(res));
}


Py::Object CyPy_LocatedEntity::start_action(const Ref<LocatedEntity>& entity, const Py::Tuple& args) {
	OpVector res;
	args.verify_length(2);

	auto& actionsProp = entity->requirePropertyClassFixed<ActionsProperty>();
	//Use seconds in the Python code
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(verifyFloat(args[1])));
	auto startTime = BaseWorld::instance().getTimeAsMilliseconds();
	actionsProp.addAction(*entity, res, verifyString(args[0]), {startTime, startTime + duration});

	return CyPy_Oplist::wrap(std::move(res));
}


Py::Object CyPy_LocatedEntity::mod_property(const Ref<LocatedEntity>& entity, const Py::Tuple& args) {
	OpVector res;
	args.verify_length(1, 2);
	auto name = verifyString(args.front());

	PropertyBase* prop;
	if (args.length() == 2) {
		auto defaultElement = CyPy_Element::asElement(args[1]);
		prop = entity->modProperty(name, defaultElement);
	} else {
		prop = entity->modProperty(name);
	}
	if (!prop) {
		return Py::None();
	}
	Atlas::Message::Element value;
	if (prop->get(value) != 0) {
		throw Py::RuntimeError(fmt::format("Could not create property '{}'.", name));
	}
	return CyPy_Element::asPyObject(value, false);
}

Py::Object CyPy_LocatedEntity::getattro(const Py::String& name) {
	if (name.as_string() == "props") {
		return CyPy_EntityProps::wrap(this->m_value);
	}
	return CyPy_LocatedEntityBase::getattro(name);
}

int CyPy_LocatedEntity::setattro(const Py::String& name, const Py::Object& attr) {
	if (name.as_string() == "props") {
		throw Py::AttributeError("Can not set 'props'.");
	}
	return CyPy_LocatedEntityBase::setattro(name, attr);
}


Py::Object CyPy_LocatedEntity::find_in_contains(const Py::Tuple& args) {
	args.verify_length(1);
	auto& filter = verifyObject<CyPy_Filter<LocatedEntity>>(args[0]);

	Py::List list;

	if (this->m_value->m_contains) {
		for (auto& entry: *this->m_value->m_contains) {
			EntityFilter::QueryContext<LocatedEntity> queryContext{{*entry}};
			queryContext.entity_lookup_fn = [](const std::string& id) { return BaseWorld::instance().getEntity(id); };
			queryContext.type_lookup_fn = [](const std::string& id) { return Inheritance::instance().getType(id); };

			if (filter->match(queryContext)) {
				list.append(CyPy_LocatedEntity::wrap(entry.get()));
			}
		}
	}
	return list;
}

Py::Object CyPy_LocatedEntity::get_parent_domain() {
	auto parent = m_value->m_parent;
	while (parent) {
		auto domain = parent->getDomain();
		if (domain) {
			return CyPy_Domain::wrap(parent);
		}
		parent = parent->m_parent;
	}
	return Py::None();
}

Py::Object CyPy_LocatedEntity::create_new_entity(const Py::Tuple& args) {
	args.verify_length(1, 2);

	if (CyPy_RootEntity::check(args.front())) {
		auto newEntity = m_value->createNewEntity(CyPy_RootEntity::value(args.front()));
		return CyPy_LocatedEntity::wrap(std::move(newEntity));
	} else if (CyPy_Operation::check(args.front())) {
		args.verify_length(2);
		auto newEntity = m_value->createNewEntity(CyPy_Operation::value(args.front()), verifyObject<CyPy_Oplist>(args[1]));
		return CyPy_LocatedEntity::wrap(std::move(newEntity));
	} else {
		throw Py::TypeError("Parameters must be either one single RootEntity, or a RootOperation with an Oplist.");
	}

}
