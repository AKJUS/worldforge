/*
 Copyright (C) 2013 Erik Ogenvik

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

#ifdef HAVE_CONFIG_H
#endif

#include "AgentCreationTask.h"

#include "common/serialno.h"
#include "common/log.h"
#include "common/compose.hpp"

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Entity.h>

#include <iostream>

#include <common/operations/Possess.h>

AgentCreationTask::AgentCreationTask(std::string account_id,
                                     std::string agent_type) :
    m_account_id(std::move(account_id)),
    m_agent_type(std::move(agent_type)),
    m_serial_no(0)
{
}

AgentCreationTask::~AgentCreationTask() = default;

void AgentCreationTask::setup(const std::string& arg, OpVector& res)
{
    Atlas::Objects::Operation::Create c;

    Atlas::Objects::Entity::Anonymous cmap;
    cmap->setParent("creator");
    cmap->setName("cyexport agent");
    cmap->setObjtype("obj");
    cmap->setAttr("possess", 1);
    c->setArgs1(cmap);
    m_serial_no = newSerialNo();
    c->setSerialno(m_serial_no);
    c->setFrom(m_account_id);
    res.push_back(c);

}

void AgentCreationTask::operation(const Operation& op, OpVector& res)
{
    if (op->getClassNo() == Atlas::Objects::Operation::ERROR_NO) {
        log(ERROR,
            String::compose(
                "Got error when creating agent. Message: %1",
                op->getArgs().front()->getAttr("message").asString()));
    } else if (!op->isDefaultRefno() && op->getRefno() == m_serial_no) {
        if (op->getClassNo() == Atlas::Objects::Operation::INFO_NO) {
            if (!op->getArgs().empty()) {
                auto arg = op->getArgs().front();
                m_mind_id = arg->getId();

                Atlas::Message::Element element;
                if (arg->copyAttr("entity", element) == 0 && element.isMap()) {
                    auto idElement = element.Map().find("id");
                    if (idElement != element.Map().end() && idElement->second.isString()) {
                        m_agent_id = idElement->second.String();
                    }
                }
            } else {
                log(ERROR, "No id received in response to creation.");
            }
            m_complete = true;

        }

    }
}

