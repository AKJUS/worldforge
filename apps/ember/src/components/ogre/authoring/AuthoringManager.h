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

#ifndef AUTHORINGMANAGER_H_
#define AUTHORINGMANAGER_H_

#include "framework/ConsoleCommandWrapper.h"
#include "framework/ConsoleObject.h"
#include "services/config/ConfigListenerContainer.h"
#include <sigc++/connection.h>
#include <unordered_map>


namespace varconf {
class Variable;
}

namespace Eris {
class View;

class Entity;
}

namespace Ember {
class ConfigListenerContainer;

class EmberEntity;
namespace OgreView {
class World;

/**
 * @brief Namespace for authoring classes and activities.
 * Any class or mechanism which deals with how to alter the world should go in here.
 */
namespace Authoring {

class AuthoringHandler;

class EntityMover;

class SimpleEntityVisualization;

class GeometryVisualization;

class EntityConsoleEditor;

/**
 * @author Erik Ogenvik <erik@ogenvik.org>
 * @brief Manages authoring, mainly visualizations of entities for authoring purposes.
 * The actual visualizations is handled by an instance of AuthoringHandler, which is held by this class.
 */
class AuthoringManager : public ConsoleObject, ConfigListenerContainer {
public:
	/**
	 * @brief Ctor.
	 * @param world The world which should be authored.
	 */
	explicit AuthoringManager(World& world);

	/**
	 * @brief Dtor.
	 */
	~AuthoringManager() override;

	/**
	 * @brief Displays authoring visualizations for entities.
	 * These are visualizations of each entity in the world, which is helpful when the user wants to perform authoring.
	 */
	void displayAuthoringVisualization();

	/**
	 * @brief Hides authoring visualizations for entities.
	 */
	void hideAuthoringVisualization();

	/**
	 * @brief Shows a simple entity visualization of the entity's bounding box.
	 * @param entity The entity to visualize.
	 */
	void displaySimpleEntityVisualization(EmberEntity& entity);

	/**
	 * @brief Hides a previously shown simple entity visualization of the entity's bounding box.
	 * It's safe to call this for an entity which hasn't previously been visualized.
	 * @param entity The entity to hide the visualization for.
	 */
	void hideSimpleEntityVisualization(EmberEntity& entity);

	/**
	 * @brief Checks whether a simple entity visualization of the entity's bounding box is active for the supplied entity.
	 * @param entity The entity to check visualization for.
	 * @returns True if the entity is being visualized.
	 */
	bool hasSimpleEntityVisualization(const EmberEntity& entity) const;

	void displayGeometryVisualization(EmberEntity& entity);

	void hideGeometryVisualization(EmberEntity& entity);

	bool hasGeometryVisualization(const EmberEntity& entity) const;

	/**
	 * @copydoc ConsoleObject::runCommand
	 */
	void runCommand(const std::string& command, const std::string& args) override;

	void startMovement(EmberEntity& entity, EntityMover& mover);

	void stopMovement();

	/**
	 * @brief Command for displaying authoring visualizations.
	 */
	const ConsoleCommandWrapper DisplayAuthoringVisualizations;

	/**
	 * @brief Command for hiding authoring visualizations.
	 */
	const ConsoleCommandWrapper HideAuthoringVisualizations;

protected:

	/**
	 * @brief Store for simple visualizations of entities.
	 * The sigc::connection is used for the Eris::Entity::BeingDeleted listening, which we want to remove when we're deleting the visualization.
	 */
	typedef std::unordered_map<const EmberEntity*, std::pair<std::unique_ptr<SimpleEntityVisualization>, sigc::connection>> SimpleEntityVisualizationStore;

	/**
	 * @brief The world to which this manager belongs.
	 */
	World& mWorld;

	/**
	 * @brief The handler, which will take care of the actual handling of visualizations.
	 */
	std::unique_ptr<AuthoringHandler> mHandler;

	/**
	 * @brief Keeps track of all simple visualizations of entities.
	 */
	SimpleEntityVisualizationStore mSimpleVisualizations;

	std::unordered_map<const EmberEntity*, std::pair<std::unique_ptr<GeometryVisualization>, sigc::connection>> mGeometryVisualizations;

	std::unique_ptr<EntityConsoleEditor> mEntityConsoleEditor;

	/**
	 * Determines whether visualizations should be shown or not.
	 * @param section
	 * @param key
	 * @param variable
	 */
	void config_AuthoringVisualizations(const std::string& section, const std::string& key, varconf::Variable& variable);

	/**
	 * @brief Called when the world has gotten the Avatar. Will evaluate the authoring visualization setting.
	 */
	void worldGotAvatar();

};
}
}

}

#endif /* AUTHORINGMANAGER_H_ */
