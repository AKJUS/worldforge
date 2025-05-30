#ifndef ERIS_AVATAR_H
#define ERIS_AVATAR_H

#include "Types.h"
#include "EntityRef.h"
#include "World.h"

#include <Atlas/Objects/ObjectsFwd.h>
#include <Atlas/Message/Element.h>

#include <wfmath/point.h>
#include <wfmath/vector.h>
#include <wfmath/quaternion.h>
#include <wfmath/timestamp.h>

#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include <optional>
#include <chrono>
#include <vector>

namespace Eris {

// Forward Declerations
class Account;

class View;

class Connection;

class TransferInfo;

class TimedEvent;

/** The player's avatar representation. **/
class Avatar : virtual public sigc::trackable {
public:

	/** Create a new Avatar object.
	@param pl The player that owns the Avatar
	*/
	Avatar(Account& pl, std::string mindId, std::string entityId);

	virtual ~Avatar();


	/// Get the Mind id of this Avatar. All interaction with the entity goes through the Mind.
	const std::string& getId() const;

	// Get the Entity if of this Avatar. This can't be directly interacted with; all interaction must go through the Mind.
	const std::string& getEntityId() const;


	/// Get the Entity this Avatar refers to
	Entity* getEntity() const;

	View& getView() const;

	Connection& getConnection() const;

	Account& getAccount() const;

	const World& getWorld() const;

	/** get the current local approximation of world time. */
	std::chrono::milliseconds getWorldTime();

	/// Touch an entity
	void touch(Entity*, const WFMath::Point<3>& pos);

	/*
	 * Wields an entity at an attach point, or removes the currently attached entity from
	 * the attach point.
	 *
	 * @param entity An entity, or null.
	 * @param attachPoint A named attach point.
	 */
	void wield(Eris::Entity* entity, std::string attachPoint) const;

	/// Say something (in-game)
	void say(const std::string&);

	/// Say something (in-game), addressing one or many entities
	/// @param message The message, i.e. what is being said.
	/// @param entities A list of ids of entities being addressed.
	void sayTo(const std::string& message, const std::vector<std::string>& entities);

	/// Emote something (in-game)
	void emote(const std::string&);

	/// Have the character move towards a position. Any non-valid data will not be sent.
	void moveToPoint(const WFMath::Point<3>&, const WFMath::Quaternion& orient);

	/// Set the character's velocity and orientation. Any non-valid data will not be sent.
	void moveInDirection(const WFMath::Vector<3>&, const WFMath::Quaternion&);

	/**
	 * @brief Place an entity inside another one.
	 *
	 * @note Use this method both when you want to move an entity from one
	 * container to another, or just want to update the position or
	 * orientation of an entity without changing its container.
	 *
	 * @param entity The entity to place.
	 * @param container The container for the entity.
	 * @param pos The position of the entity within the container.
	 * @param orientation An optional orientation of the entity.
	 */
	void place(const Entity* entity,
			   const Entity* container,
			   const WFMath::Point<3>& pos = WFMath::Point<3>(),
			   const WFMath::Quaternion& orientation = WFMath::Quaternion(),
			   std::optional<float> offset = {},
			   int amount = 1);


	/**
	@brief Stop the current task, if one is in progress.
	*/
	void useStop();

	void deactivate();

	/**
	 * @brief Sets whether the current avatar is an admin character.
	 *
	 * As an "admin" character the avatar has greater ability to alter the state of
	 * the server. This is often done by sending Atlas ops to the entity itself, thus
	 * bypassing the normal routing rules on the server.
	 *
	 * It's up to the client to determine which avatars are admin, and set this flag
	 * as soon as possible after the Avatar has been created.
	 */
	void setIsAdmin(bool isAdmin);

	/**
	 * @brief Gets whether the current avatar is an admin character.
	 *
	 * As an "admin" character the avatar has greater ability to alter the state of
	 * the server. This is often done by sending Atlas ops to the entity itself, thus
	 * bypassing the normal routing rules on the server.
	 *
	 * It's up to the client to determine which avatars are admin, and call setIsAdmin
	 * as soon as possible after the Avatar has been created.
	 */
	bool getIsAdmin() const;

	/**
	 * @brief Sends an operation from this Avatar.
	 *
	 * This will set the "to" property to be from this avatar's mind.
	 * @param op
	 */
	void send(const Atlas::Objects::Operation::RootOperation& op);

	/**
	 * Gets the current list of opened containers.
	 * EntityRefs that aren't valid yet will issue the ContainerOpened signal when they are resolved.
	 * @return
	 */
	const std::map<std::string, std::unique_ptr<EntityRef>>& getActiveContainers() const;

	/**
 * @brief Called when a logout of the avatar has been requested by the
 *  server.
 */
	void logoutRequested();

	/**
	 * @brief Called when a logout and server transfer of the avatar has been
	 *  requested by the server.
	 * @param transferInfo The transfer info which contains information about
	 *  the server to transfer to.
	 */
	void logoutRequested(const TransferInfo& transferInfo);

	/**
	Emitted when the character entity of this Avatar is valid (and presumably,
	visible). This will happen some time after the InGame signal is emitted.
	A client might wish to show some kind of 'busy' animation, eg an hour-glass,
	while waiting for this signal.
	*/
	sigc::signal<void(Entity*)> GotCharacterEntity;

	/**
	 * Emitted when the avatar entity for whatever reason is deleted.
	 */
	sigc::signal<void()> CharacterEntityDeleted;

	sigc::signal<void(Entity&)> ContainerOpened;
	sigc::signal<void(Entity&)> ContainerClosed;

	/**
	Emitted when a character transfer authentication is requested. Clients
	should use the hostname, port number, possess key and entity ID to claim
	the character on a remote host
	*/
	sigc::signal<void(const TransferInfo&)> TransferRequested;


protected:
	friend class Account;

	friend class AccountRouter;

	friend class IGRouter;

	void onEntityAppear(Entity* ent);

	/**
	 * @brief Called when the avatar entity is deleted.
	 */
	void onAvatarEntityDeleted();

	virtual void onTransferRequested(const TransferInfo& transfer);

	void logoutResponse(const Atlas::Objects::Operation::RootOperation&);


	void containerActiveChanged(const Atlas::Message::Element& element);

	Account& m_account;

	std::string m_mindId;
	std::string m_entityId;
	Entity* m_entity;

	std::unique_ptr<View> m_view;
	std::unique_ptr<World> m_world;

	sigc::connection m_entityAppearanceCon;
	sigc::connection m_avatarEntityDeletedConnection;

	bool m_isAdmin;

	std::unique_ptr<TimedEvent> m_logoutTimer;

	std::map<std::string, std::unique_ptr<EntityRef>> m_activeContainers;

	sigc::connection m_entityParentDeletedConnection;
};

inline const std::string& Avatar::getId() const {
	return m_mindId;
}

inline const std::string& Avatar::getEntityId() const {
	return m_entityId;
}


inline Entity* Avatar::getEntity() const {
	return m_entity;
}

inline View& Avatar::getView() const {
	return *m_view;
}

inline Account& Avatar::getAccount() const {
	return m_account;
}

inline const std::map<std::string, std::unique_ptr<EntityRef>>& Avatar::getActiveContainers() const {
	return m_activeContainers;
}

inline const World& Avatar::getWorld() const {
	return *m_world;
}


} // of namespace Eris

#endif
