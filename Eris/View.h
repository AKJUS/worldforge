#ifndef ERIS_VIEW_H
#define ERIS_VIEW_H

#include <Eris/Types.h>
#include <Eris/Factory.h>
#include <Atlas/Objects/ObjectsFwd.h>

#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sigc++/slot.h>
#include <sigc++/connection.h>

#include <map>
#include <deque>

namespace Eris
{

class Avatar;
class Entity;
class Connection;

/** View encapsulates the set of entities currently visible to an Avatar,
 as well as those that have recently been seen. It recieves visibility-affecting
 ops from the IGRouter, and uses them to update its state and emit signals.
 */
class View : public sigc::trackable
{
public:
    View(Avatar* av);
    ~View();

    /**
    Retrieve an entity in the view by id. Returns NULL if no such entity exists
    in the view.
    */
    Entity* getEntity(const std::string& eid) const;

    Avatar* getAvatar() const
    {
        return m_owner;
    }

    /** return the current top-level entity. This will return NULL
    until the first emission of the TopLevelEntityChanged signal. */
    Entity* getTopLevel() const
    {
        return m_topLevel;
    }
    
    /** once-per-frame update of the View - clients should call this method
    once per game loop (or similar), to allow the View to update Entity
    state. This includes motion prediction for moving entities, and confidence
    levels for disappeared entities.
    */
    void update();

    /**
    Register an Entity Factory with this view
    */
    void registerFactory(Factory*);

    typedef sigc::slot<void, Entity*> EntitySightSlot;

    /**
    Conenct up a slot to be fired when an Entity with the specified ID is seen.
    If the entity is already visible, this is a no-op (and will log an error)
    */
    sigc::connection notifyWhenEntitySeen(const std::string& eid, const EntitySightSlot& slot);

    /** emitted whenever the View creates a new Entity instance. This signal
    is emitted once the entity has been fully bound into the View */
    SigC::Signal1<void, Entity*> EntitySeen;

    /** emitted when a SIGHT(CREATE) op is recieved for an entity */
    SigC::Signal1<void, Entity*> EntityCreated;
    
    /** emitted when a SIGHT(DELETE) op is recieved for an entity */
    sigc::signal<void, Entity*> EntityDeleted;
    
    sigc::signal<void, Entity*> Appearance;
    sigc::signal<void, Entity*> Disappearance;

    /// emitted when the TLVE changes
    sigc::signal<void> TopLevelEntityChanged;

    void dumpLookQueue();

    /**
    Retrieve the current look queue size, for debugging / statistics purposes.
    Eg, this could be displayed as a bar-chart on screen in a client (optionally)
    */
    unsigned int lookQueueSize() const
    {
        return m_lookQueue.size();
    }
protected:
    // the router passes various relevant things to us directly
    friend class IGRouter;
    friend class Entity;
    friend class Avatar;
    
    void appear(const std::string& eid, float stamp);
    void disappear(const std::string& eid);
    void sight(const Atlas::Objects::Entity::RootEntity& ge);
    void create(const Atlas::Objects::Entity::RootEntity& ge);
    void deleteEntity(const std::string& eid);
    void unseen(const std::string& eid);
    
    void setEntityVisible(Entity* ent, bool vis);

    /// test if the specified entity ID is pending initial sight on the View
    bool isPending(const std::string& eid) const;

    void addToPrediction(Entity* ent);
    void removeFromPrediction(Entity* ent);
    
    /** this is a hook that Entity's destructor calls to remove itself from
    the View's content map. The name is unfortantely similar to the public
    'EntityDeleted' signal - alternative naming suggestions appreciated. */
    void entityDeleted(Entity* ent);

private:
    Entity* initialSight(const Atlas::Objects::Entity::RootEntity& ge);

    Connection* getConnection() const;
    void getEntityFromServer(const std::string& eid);

    /** helper to update the top-level entity, fire signals, etc */
    void setTopLevelEntity(Entity* newTopLevel);

    Entity* createEntity(const Atlas::Objects::Entity::RootEntity&);

    /**
    Issue a LOOK operation for the specified entity ID. The id may be
    empty for an anonymous look. The pending sight map will be updated
    with the appropriate information.
    */
    void sendLookAt(const std::string& eid);
    
    /**
    If the look queue is not empty, pop the first item and send a request
    for it to the server.
    */
    void issueQueuedLook();

    void eraseFromLookQueue(const std::string& eid);

    typedef std::map<std::string, Entity*> IdEntityMap;

    Avatar* m_owner;
    IdEntityMap m_contents;
    Entity* m_topLevel; ///< the top-level visible entity for this view

    sigc::signal<void, Entity*> InitialSightEntity;

    /** enum describing what action to take when sight of an entity
    arrives. This allows us to handle intervening disappears or
    deletes cleanly. */
    typedef enum
    {
        SACTION_INVALID,
        SACTION_APPEAR,
        SACTION_HIDE,
        SACTION_DISCARD,
        SACTION_QUEUED
    } SightAction;

    typedef std::map<std::string, SightAction> PendingSightMap;
    PendingSightMap m_pending;
    
    /**
    A queue of entities to be looked at, which have not yet be requested
    from the server. The number of concurrent active LOOK requests is
    capped to avoid network failures.
    
    @sa m_maxPendingCount
    */
    std::deque<std::string> m_lookQueue;
          
    unsigned int m_maxPendingCount;
          
    typedef sigc::signal<void, Entity*> EntitySightSignal;
        
    typedef std::map<std::string, EntitySightSignal> NotifySightMap;
    NotifySightMap m_notifySights;
    
    typedef std::set<Entity*> EntitySet;
    
    /** all the entities in the view which are moving, so they can be
    motion predicted. */
    EntitySet m_moving;
    
    class FactoryOrdering
    {
    public:
        bool operator()(Factory* a, Factory* b) const
        {   // higher priority factories are placed nearer the start
            return a->priority() > b->priority();
        }
    };
    
    typedef std::multiset<Factory*, FactoryOrdering> FactoryStore;
    FactoryStore m_factories;
};

} // of namespace Eris

#endif // of ERIS_VIEW_H
