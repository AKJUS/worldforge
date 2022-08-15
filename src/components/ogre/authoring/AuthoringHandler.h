/*
 Copyright (C) 2009 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef AUTHORINGHANDLER_H_
#define AUTHORINGHANDLER_H_

#include "domain/IEntityVisitor.h"
#include "components/ogre/EntityObserverBase.h"
#include <unordered_map>
#include <sigc++/trackable.h>
#include <memory>
namespace Eris
{
class View;
class Entity;
}

namespace Ember
{
class EmberEntity;
namespace OgreView
{
class World;

namespace Authoring
{
class AuthoringVisualization;
class AuthoringVisualizationMover;
class EntityMover;
class AuthoringHandler;

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief An instance of a move operation. An instance of this is created when a AuthoringVisualizationMover is used for displaying the movement of an entity.
 * Note that this inherits from EntityObserverBase which makes it react to the entity being deleted. However, this functionality is already taken care of in AuthoringHandler::view_EntityDeleted making it a bit superfluous.
 */
class AuthoringMoveInstance: public EntityObserverBase
{
public:
	AuthoringMoveInstance(EmberEntity& entity, AuthoringVisualization& visualization, EntityMover& mover, AuthoringHandler& moveHandler);

	~AuthoringMoveInstance() override;

	AuthoringVisualization& getVisualization();
private:
	void cleanup() override;

	std::unique_ptr<AuthoringVisualizationMover> mMover;
	AuthoringHandler& mMoveHandler;
	AuthoringVisualization& mVisualization;
};

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief Handles authoring visualizations.
 * These are visualizations of all entities, for authoring sake. The main use of this is for entities which doesn't have a graphical visualization (like an area) or entities for which the graphical representation is hard to graphically select.
 *
 */
class AuthoringHandler: public virtual sigc::trackable, public IEntityVisitor
{
public:
	/**
	 * @brief Ctor.
	 * @param view The world for which this handler should create visualizations.
	 */
	explicit AuthoringHandler(World& world);

	/**
	 * @brief Dtor.
	 * During destruction all visualizations are cleaned up.
	 */
	~AuthoringHandler() override;

	/**
	 * @brief IEntityVisitor implementation.
	 * Allows creating of visualizations for all existing entities.
	 */
	void visit(EmberEntity& entity) override;

	/**
	 * @brief Starts movement of the entity, through the supplied mover.
	 * Calling this when an entity movement operation is started allows the authoring visualization to be synced with the movement, though an instance of AuthoringMoveInstance.
	 * @param entity The entity being moved.
	 * @param mover The mover, taking care of the movement.
	 */
	void startMovement(EmberEntity& entity, EntityMover& mover);

	/**
	 * @brief Stops a previously started movement operation.
	 * Call this when the movement has stopped, to make sure that the previously created AuthoringMoveInstance instance is destroyed.
	 */
	void stopMovement();

protected:

	typedef std::unordered_map<EmberEntity*, std::unique_ptr<AuthoringVisualization>> VisualizationStore;

	/**
	 * @brief A store of all visualizations in the world.
	 */
	VisualizationStore mVisualizations;

	/**
	 * @brief If an entity is moved client side, this instance will make sure that the visualization is synced with the movement.
	 */
	std::unique_ptr<AuthoringMoveInstance> mMoveInstance;

	/**
	 * @brief The world for which we should create visualizations.
	 */
	World& mWorld;

	/**
	 * @brief When an entity first is seen we'll create a visualization for it.
	 * @param entity The newly seen entity.
	 */
	void view_EntityInitialSight(Eris::Entity* entity);

	/**
	 * @brief When an entity is deleted we'll remove the visualization for it.
	 * @param entity The deleted entity.
	 */
	void view_EntityDeleted(Eris::Entity* entity);

	/**
	 * @brief When an entity changes location we need to alter the attachment of the graphical visualization.
	 * @param oldLocation The old location.
	 */
	void view_EntityLocationChanged(Eris::Entity* oldLocation, EmberEntity* entity);

	/**
	 * @brief Creates a visualization for an entity.
	 * @param entity The entity which we'll create a visualization for.
	 */
	void createVisualizationForEntity(EmberEntity* entity);

	/**
	 * @brief Create visualizations for all existing entities.
	 * @param view The view for which we'll create visualizations for.
	 */
	void createVisualizationsForExistingEntities(Eris::View& view);
};

}

}

}

#endif /* AUTHORINGHANDLER_H_ */
