#include "ViewEntity.h"

#include "Connection.h"
#include "EntityRouter.h"
#include "View.h"
#include "Avatar.h"
#include "Task.h"

#include <sigc++/bind.h>



namespace Eris {
ViewEntity::ViewEntity(std::string id, TypeInfo* ty, View& view) :
	Entity(std::move(id), ty),
	m_view(view),
	m_router(new EntityRouter(*this, view.getTypeService())){
	m_view.getConnection().registerRouterForFrom(m_router.get(), getId());

}

ViewEntity::~ViewEntity() {
    if (m_moving) {
        m_view.removeFromPrediction(this);
    }
    m_view.getConnection().unregisterRouterForFrom(m_router.get(), m_id);
    m_view.entityDeleted(this); // remove ourselves from the View's content map. This will delete the entity.
}

TypeService& ViewEntity::getTypeService() const {
	return m_view.getAvatar().getConnection().getTypeService();
}

void ViewEntity::onTalk(const Atlas::Objects::Operation::RootOperation& talk) {
	Entity::onTalk(talk);
	m_view.getAvatar().Hear.emit(this, talk);
}

void ViewEntity::onSoundAction(
		const Atlas::Objects::Operation::RootOperation& op, const TypeInfo& typeInfo) {
	Entity::onSoundAction(op, typeInfo);
	m_view.getAvatar().Hear.emit(this, op);
}

void ViewEntity::onVisibilityChanged(bool vis)
{
    m_view.setEntityVisible(this, vis);
	Entity::onVisibilityChanged(vis);
}

void ViewEntity::removeFromMovementPrediction() {
	m_view.removeFromPrediction(this);
}

void ViewEntity::addToMovementPrediction() {
	m_view.addToPrediction(this);
}

Entity* ViewEntity::getEntity(const std::string& id) {
	auto child = m_view.getEntity(id);
	if (!child || !child->m_visible) {
		// we don't have the entity at all, so request it and skip
		// processing it here; everything will come right when it
		// arrives.
		m_view.getEntityFromServer(id);
	}
	return child;
}

void ViewEntity::onTaskAdded(const std::string& id, Task* task)
{
	task->ProgressRateChanged.connect(sigc::bind(sigc::mem_fun(*this, &ViewEntity::task_ProgressRateChanged), task));
	Entity::onTaskAdded(id, task);
	m_view.taskRateChanged(task);
}

void ViewEntity::task_ProgressRateChanged(Task* task)
{
	m_view.taskRateChanged(task);
}


}
