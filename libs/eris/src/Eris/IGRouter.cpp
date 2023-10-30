#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "IGRouter.h"
#include "Avatar.h"
#include "Connection.h"
#include "View.h"
#include "Entity.h"
#include "LogStream.h"
#include "TypeInfo.h"
#include "TypeBoundRedispatch.h"
#include "TransferInfo.h"
#include "TypeService.h"

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Entity.h>

using namespace Atlas::Objects::Operation;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::RootEntity;
using Atlas::Objects::smart_dynamic_cast;
using Atlas::Message::Element;

namespace Eris {

IGRouter::IGRouter(Avatar& av, View& view) :
    m_avatar(av),
    m_view(view)
{
    m_avatar.getConnection().registerRouterForTo(this, m_avatar.getEntityId());
    m_actionType = m_avatar.getConnection().getTypeService().getTypeByName("action");
}

IGRouter::~IGRouter()
{
    m_avatar.getConnection().unregisterRouterForTo(this, m_avatar.getEntityId());
}

Router::RouterResult IGRouter::handleOperation(const RootOperation& op)
{
    if (!op->isDefaultSeconds()) {
        // grab out world time
        m_avatar.updateWorldTime(op->getSeconds());
    }

    const std::vector<Root>& args = op->getArgs();

    if (op->getClassNo() == SIGHT_NO) {
        if (args.empty()) {
            warning() << "Avatar received sight with empty args";
            return IGNORED;
        }

		for (const auto& arg : args) {
			if (arg->instanceOf(ROOT_OPERATION_NO)) {
				handleSightOp(op, smart_dynamic_cast<RootOperation>(arg));
			} else {
				// initial sight of entities
				auto gent = smart_dynamic_cast<RootEntity>(arg);
				if (gent.isValid()) {
					// View needs a bound TypeInfo for the entity
					if (!gent->isDefaultId() && !gent->isDefaultParent()) {
						TypeInfo* ty = m_avatar.getConnection().getTypeService().getTypeForAtlas(gent);
						if (!ty->isBound()) {
							auto opCopy = op.copy();
							opCopy->setArgs1(arg);
							new TypeBoundRedispatch(m_avatar.getConnection(), opCopy, ty);
						} else {
							m_view.sight(gent);
						}
					}
				}
			}
		}

		return HANDLED;


    }

    if (op->getClassNo() == APPEARANCE_NO) {
        for (const auto& arg : args) {
            double stamp = -1;
            if (!arg->isDefaultStamp()) {
                stamp = arg->getStamp();
            }

			if (!arg->isDefaultId()) {
				m_view.appear(arg->getId(), stamp);
			}
        }

        return HANDLED;
    }

    if (op->getClassNo() == DISAPPEARANCE_NO) {
        for (const auto& arg : args) {
        	if (!arg->isDefaultId()) {
				m_view.disappear(arg->getId());
			}
        }

        return HANDLED;
    }

    if (op->getClassNo() == UNSEEN_NO)
    {
        if (args.empty()) {
            warning() << "Avatar received unseen with empty args";
            return IGNORED;
        }
		for (const auto& arg : args) {
			if (!arg->isDefaultId()) {
				m_view.unseen(arg->getId());
			}
		}
        return HANDLED;
    }

    // logout
    if (op->getClassNo() == LOGOUT_NO) {
        debug() << "Avatar received forced logout from server";

        if(args.size() >= 2) {
            bool gotArgs = true;
            // Teleport logout op. The second attribute is the payload for the teleport host data.
            const Root & arg = args[1];
            Element tp_host_attr;
            Element tp_port_attr;
            Element pkey_attr;
            Element pentity_id_attr;
            if(arg->copyAttr("teleport_host", tp_host_attr) != 0
                    || !tp_host_attr.isString()) {
                debug() << "No teleport host specified. Doing normal logout."
                        << std::endl << std::flush;
                gotArgs = false;
            } else if (arg->copyAttr("teleport_port", tp_port_attr) != 0
                    || !tp_port_attr.isInt()) {
                debug() << "No teleport port specified. Doing normal logout."
                        << std::endl << std::flush;
                gotArgs = false;
            } else if (arg->copyAttr("possess_key", pkey_attr) != 0
                    || !pkey_attr.isString()) {
                debug() << "No possess key specified. Doing normal logout."
                        << std::endl << std::flush;
                gotArgs = false;
            } else if (arg->copyAttr("possess_entity_id", pentity_id_attr) != 0
                    || !pentity_id_attr.isString()) {
                debug() << "No entity ID specified. Doing normal logout."
                        << std::endl << std::flush;
                gotArgs = false;
            }

            // Extract argument data and request transfer only if we
            // succeed in extracting them all
            if (gotArgs) {
                std::string teleport_host = tp_host_attr.String();
                int teleport_port = static_cast<int>(tp_port_attr.Int());
                std::string possess_key = pkey_attr.String();
                std::string possess_entity_id = pentity_id_attr.String();
                debug() << "Server transfer data: Host: " << teleport_host
                    << ", Port: " << teleport_port << ", "
                    << "Key: " << possess_key << ", "
                    << "ID: " << possess_entity_id << std::endl << std::flush;
                // Now do a transfer request
                TransferInfo transfer(teleport_host, teleport_port, possess_key
                        , possess_entity_id);
                m_avatar.logoutRequested(transfer);
            } else {
                m_avatar.logoutRequested();
            }

        } else {
            // Regular force logout op
            m_avatar.logoutRequested();
        }

        return HANDLED;
    }

    return IGNORED;
}

Router::RouterResult IGRouter::handleSightOp(const RootOperation& sightOp, const RootOperation& op)
{
    const auto& args = op->getArgs();

    // because a SET op can potentially (legally) update multiple entities,
    // we decode it here, not in the entity router
    if (op->getClassNo() == SET_NO) {
        for (const auto& arg : args) {
        	if (!arg->isDefaultId()) {
				auto ent = m_view.getEntity(arg->getId());
				if (!ent) {
					if (m_view.isPending(arg->getId())) {
						/* no-op, we'll get the state later */
					} else {
						m_view.sendLookAt(arg->getId());
					}

					continue; // we don't have it, ignore
				}

				//If we get a SET op for an entity that's not visible, it means that the entity has moved
				//within our field of vision without sending an Appear op first. We should treat this as a
				//regular Appear op and issue a Look op back, to get more info.
				if (!ent->isVisible()) {
//					float stamp = -1;
//					if (!arg->isDefaultStamp()) {
//						stamp = static_cast<float>(arg->getStamp());
//					}
//
//					m_view.appear(arg->getId(), stamp);
					m_view.getEntityFromServer(arg->getId());
				} else {
					ent->setFromRoot(arg, false);
				}
			}
        }
        return HANDLED;
    }

    if (!op->isDefaultParent()) {
		// we have to handle generic 'actions' late, to avoid trapping interesting
		// such as create or divide
		TypeInfo* ty = m_avatar.getConnection().getTypeService().getTypeForAtlas(op);
		if (!ty->isBound()) {
			new TypeBoundRedispatch(m_avatar.getConnection(), sightOp, ty);
			return HANDLED;
		}

		//For hits we want to check the "to" field rather than the "from" field. We're more interested in
		//the entity that was hit than the one which did the hitting.
		//Note that we'll let the op fall through, so that we later on handle the Hit action for the "from" entity.
		if (op->getClassNo() == HIT_NO) {
			if (!op->isDefaultTo()) {
				Entity* ent = m_view.getEntity(op->getTo());
				if (ent) {
					ent->onHit(smart_dynamic_cast<Hit>(op), *ty);
				}
			} else {
				warning() << "received hit with TO unset";
			}
		}


		if (ty->isA(m_actionType)) {
			if (op->isDefaultFrom()) {
				warning() << "received op " << ty->getName() << " with FROM unset";
				return HANDLED;
			}

			Entity* ent = m_view.getEntity(op->getFrom());
			if (ent) {
				ent->onAction(op, *ty);
			}

			return HANDLED;
		}
	}

    return IGNORED;
}

} // of namespace Eris
