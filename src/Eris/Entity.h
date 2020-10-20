#ifndef ERIS_ENTITY_H
#define ERIS_ENTITY_H

#include "Types.h"

#include <Atlas/Objects/ObjectsFwd.h>

#include <wfmath/point.h>
#include <wfmath/vector.h>
#include <wfmath/axisbox.h>
#include <wfmath/quaternion.h>
#include <wfmath/timestamp.h>

#include <sigc++/trackable.h>
#include <sigc++/slot.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include <map>
#include <vector>
#include <unordered_map>
#include <boost/optional.hpp>

namespace Atlas {
    namespace Message {
        class Element;
        typedef std::map<std::string, Element> MapType;
    }
}

namespace Eris {

// Forward Declarations
class Entity;
class TypeInfo;
class View;
class EntityRouter;
class Task;

typedef std::vector<Entity*> EntityArray;

/** 
@brief Entity is a concrete (instantiable) class representing one game entity

Entity encapsulates the state and tracking of one game entity; this includes
it's location in the containership tree (graph?), it's name and unique and id,
and so on.

This class may be sub-classed by users (and those sub-classes built via
a Factory), to allow specific functionality. This means there are two
integration strategies; either subclassing and over-riding virtual functions,
or creating peer classes and attaching them to the signals.
*/

class Entity : virtual public sigc::trackable
{
	friend class EntityRouter;
public:
    typedef std::map<std::string, Atlas::Message::Element> PropertyMap;
    
    explicit Entity(std::string id, TypeInfo* ty);
    virtual ~Entity();

// hierarchy interface
    /**
     * @brief Gets the number of contained entities, i.e. entities that are direct children of this.
     * The number returned is only for direct children, so the number of nested entities can be larger.
     * @return 
     */
    size_t numContained() const;
    
    /**
     * @brief Gets the child entity at the specified index.
     * @param index An index for the collection of child entities. This must be a valid index as no bounds checking will happen.
     * @return A pointer to a child entity.
     */
    Entity* getContained(size_t index) const;

    /**
     * @brief Gets the top level entity for this entity, i.e. the parent location which has no parent.
     * Will return null if any parent isn't resolved yet.
     * @return
     */
    Entity* getTopEntity();

    /**
     * Returns true if this entity is an ancestor to the supplied entity.
     * I.e. that it's either a direct or indirect parent.
     * Note that this might be incorrect if parents aren't bound yet.
     * @param entity
     * @return
     */
    bool isAncestorTo(Eris::Entity& entity) const;

    /**
     * @brief Gets the value of a named property.
     * If no property by the specified name can be found an InvalidOperation exception will be thrown. Therefore always first call hasProperty to make sure that the property exists.
     * @param name The property name.
     * @return A reference to the property by the specified name.
     * @throws InvalidOperation If no property by the specified name can be found.
     */
    const Atlas::Message::Element& valueOfProperty(const std::string& name) const;
        
    /**
     * @brief Checks whether an property exists.
     * @param p The name of the property.
     * @return True if the property exists.
     */
    bool hasProperty(const std::string &p) const;


    /**
     * @brief Gets the value of a named property, or null if none exists.
     * If no property by the specified name can be found null will be returned. This is thus a more efficient method than calling both "hasProperty(...)" and "valueOfProperty(...)" in sequence.
     * @param name The property name.
     * @return A pointer to the property by the specified name, or null if none could be found.
     */
    const Atlas::Message::Element* ptrOfProperty(const std::string& name) const;

    /**
     * @brief A slot which can be used for receiving property update signals.
     */
    typedef sigc::slot<void, const Atlas::Message::Element&> PropertyChangedSlot;

    /**
     * @brief Setup an observer so that the specified slot is fired when the named property's value changes
     * 
     * @param propertyName The name of the property to observe.
     * @param aslot The slot which will be fired when the property changes.
     * @param evaluateNow Whether the
     * @return The connection created.
     */
    sigc::connection observe(const std::string& propertyName, const PropertyChangedSlot& aslot, bool evaluateNow);

// accessors
    /**
     * @brief Retrieve the unique entity ID.
     * @return The unique id of the entity.
     */
    const std::string& getId() const;
    
    /**
     * @brief Gets the name of the entity.
     * In contrast to getId() this is not unique, and doesn't even have to be set.
     * @return The name of the entity.
     */
    const std::string& getName() const;
	
    /**
     * @brief Access the current time-stamp of the entity.
     * @return The current time stamp.
     */
    double getStamp() const;

    /**
     * @brief Gets the type of this entity.
     * @return The type of this entity. This can be null.
     */
    TypeInfo* getType() const;
    
    /**
     * @brief The containing entity, or null if this is a top-level visible entity.
     * @return The containing entity, or null.
     */
    Entity* getLocation() const;
	
    /**
     * @brief Returns the Entity's position inside it's parent in the parent's local system coordinates.
     * @return The position of the entity in parent relative coords.
     */
    const WFMath::Point<3>& getPosition() const;
    
    /**
     * @brief Gets all properties defined for this entity.
     * The collection of entities returned will include both local properties as well
     * as the defaults set in the TypeInfo (and all of its parents) of this entity.
     * @note This is a rather expensive operation since it needs to iterate over all parent
     * TypeInfo instances and build up a map, which is then returned by value.
     * If you only want to get a single property you should instead use the valueOfProperty method.
     * @see getInstanceProperties() for a similar method which only returns
     * those properties that are local to this entity.
     * @return A map of the combined properties of both this entity and all of it's TypeInfo parents.
     */
    PropertyMap getProperties() const;
    
    /**
     * @brief Gets all locally defined properties.
     * This will only return those properties that are locally defined for this entity.
     * In practice it will in most cases mean those properties that have been changed
     * by the defaults as defined in the TypeInfo instance.
     * @note This will only return a subset of all properties.
     * If you need to iterate over all properties you should instead use the getProperties() method.
     * If you only want the value of a specific property you should use the valueOfProperty method.
     * @see getProperties
     * @return The locally defined properties for the entity.
     */
    const PropertyMap& getInstanceProperties() const;
    
    /**
     * @brief Test if this entity has a non-zero velocity vector.
     * @return True if the entity has a non-zero velocity.
     */
    bool isMoving() const;

    /**
     * @brief Retrieve the predicted position of this entity, based on it's velocity and acceleration.
     * If the entity is not moving, this is the same as calling getPosition().
     * @return The predicted position of the entity.
     */
    const WFMath::Point<3>& getPredictedPos() const;
    
    /**
     * @brief Retrieve the current predicted velocity of an entity.
     * If the entity have no acceleration, this is the same as calling getVelocity().
     * @return The predicted velocity of the entity.
     */
    const WFMath::Vector<3>& getPredictedVelocity() const;
    
    /**
     * @brief Retrieve the current predicted orientation of an entity.
     * @return The predicted orientation of the entity.
     */
    const WFMath::Quaternion& getPredictedOrientation() const;

    /** Returns the entity's velocity as last set explicitly. **/
    const WFMath::Vector<3> & getVelocity() const;

    /** Returns the entity's angular velocity as last set explicitly. **/
    const WFMath::Vector<3> & getAngularVelocity() const;
    
    /** Returns the entity's orientation as last set explicitly. **/
    const WFMath::Quaternion & getOrientation() const;
    
    /** Returns the entity's bounding box in the entity's local system coordinates. **/
    const WFMath::AxisBox<3> & getBBox() const;

    /**
     * @brief Returns true if the entity has a bounding box.
     * Not all entities have bounding boxes, but those that are represented as physical objects in the world usually do.
     * @return True if the entity has a bounding box.
     */
    bool hasBBox() const;
    
    /**
     * @brief Gets the tasks associated with this entity.
     * @return The tasks associated with this entity.
     */
    const std::map<std::string, std::unique_ptr<Task>>& getTasks() const;
    
    bool hasChild(const std::string& eid) const;
    
    /** determine if this entity is visible. */
    bool isVisible() const;

    /**
     * @brief Extracts an entity id from the supplied element.
     *
     * There are two ways to express entity ids; either as a string,
     * or as a map with one entry named "$eid" (where the string is the value).
     * The function parses the element for these two cases, if found fills the
     * "id" parameter and then returns true.
     *
     * @param element The element which we want to extract from.
     * @param id The string where the id, if found, will be placed.
     * @return True if an id could be found.
     */
    static boost::optional<std::string> extractEntityId(const Atlas::Message::Element& element);

// coordinate transformations
    template<class C>
    C toLocationCoords(const C& c) const;
    
    template<class C>
    C fromLocationCoords(const C& c) const;


	const std::vector<Entity*>& getContent() const {
		return m_contents;
	}

    // A vector (e.g., the distance between two points, or
    // a velocity) gets rotated by a coordinate transformation,
    // but doesn't get shifted by the change in the position
    // of the origin, so we handle it separately. We also
    // need to copy the vector before rotating, because
    // Vector::rotate() rotates it in place.
    WFMath::Vector<3> toLocationCoords(const WFMath::Vector<3>& v) const;
    
    WFMath::Vector<3> fromLocationCoords(const WFMath::Vector<3>& v) const;
	
// Signals
    sigc::signal<void, Entity*> ChildAdded;
    sigc::signal<void, Entity*> ChildRemoved;
    
    /// Signal that the entity's container changed
    /** emitted when our location changes. First argument is the old location.
    The new location can be found via getLocation.
    Note either the old or new location might be nullptr.
    */
    sigc::signal<void, Entity*> LocationChanged;

    /** Emitted when one or more properties change. The arguments is a set
    of property IDs which were modified. */
    sigc::signal<void, const std::set<std::string>&> Changed;

    /** Emitted when then entity's position, orientation or velocity change.*/
    sigc::signal<void> Moved;

    /** Emitted when an entity starts or stops moving. The new movement status will be emitted. */
    sigc::signal<void, bool> Moving;

    /**
	 * @brief Emitted with the entity speaks.
	 * 
	 * The argument contains attributes that make up the content of the Say
	 * operation.
	 * - Attribute "say" which is of type string and contains the text that
	 *   this entity said.
	 * - Attrubute "responses" which is a list of strings. When conversing
	 *   with NPCs this list is used to give the client a clue about
	 *   possible answers the NPCs understand.
	 * - Attribute "address" which is optional, and is a list of strings,
	 *   which are entity id values specifying those entities that were
	 *   addressed. Note that all entities, even those not addressed, can
	 *   still receive such Say operations.
	 **/
    sigc::signal< void, const Atlas::Objects::Root & > Say;
	
    /**
    Emitted when this entity emits an imaginary operation (also known as
    an emote. This is used for debugging, but not much else. 
    */
    sigc::signal<void, const std::string&> Emote;
    
    /**
    Emitted when this entity performs an action. The argument to the
    action is passed as the signal argument. For examples of action
    arguments, see some documentation that probably isn't written yet.
    */
    sigc::signal<void, const Atlas::Objects::Operation::RootOperation&, const TypeInfo&> Acted;

	/**
	Emitted when this entity performs is hit by something.
	*/
	sigc::signal<void, const Atlas::Objects::Operation::Hit&, const TypeInfo&> Hit;

    /**
    Emitted when this entity performs an action which causes a noise. This
    may happen alongside the sight of the action, or not, depending on the
    distance to the entity and so on.
    */
    sigc::signal<void, const Atlas::Objects::Root&, const TypeInfo&> Noise;

    /**
    Emitted when the visibility of the entity changes. Often this happens
    because it has moved in or out of the sight range of the avatar.
    */
    sigc::signal<void, bool> VisibilityChanged;
    
    /**
    Emitted prior to deletion. Note that entity instances may be deleted for
    different reasons - passing out of the view, being deleted on the server,
    or during disconnection. This signal is emitted regardless.
    */
    sigc::signal<void> BeingDeleted;
    
    /**
    Emitted when a task has been added to the entity. Argument is the task.
    */
    sigc::signal<void, const std::string&, Task*> TaskAdded;
    /**
    Emitted when a task has been removed from the entity. Argument is the task.
    */
    sigc::signal<void, const std::string&, Task*> TaskRemoved;
protected:	        
    /** over-rideable initialisation helper. When subclassing, if you
    over-ride this method, take care to call the base implementation, or
    unfortunate things will happen. */
    virtual void init(const Atlas::Objects::Entity::RootEntity &ge, bool fromCreateOp);

    /**
     * Shuts down the entity. This is called by the destructor, but if you extend this class
     * you should call it in your subclass' destructor to make sure that vtables haven't been
     * cleared yet.
     */
    void shutdown();

    /** process TALK data - default implementation emits the Say signal.
    @param talk The TALK operation
    */
    virtual void onTalk(const Atlas::Objects::Operation::RootOperation& talk);

    virtual void onPropertyChanged(const std::string& propertyName, const Atlas::Message::Element &v);
	
    virtual void onLocationChanged(Entity* oldLoc);
    
    /** over-rideable hook method when then Entity position, orientation or
    velocity change. The default implementation emits the Moved signal. */
    virtual void onMoved(const WFMath::TimeStamp& timeStamp);
    
    /** over-rideable hook when the actual (computed) visiblity of this
    entity changed. The default implementation emits the VisiblityChanged
    signal. */
    virtual void onVisibilityChanged(bool vis);

    /**
    Over-rideable hook when this entity is seen to perform an action.
    Default implementation emits the Action signal.
    */
    virtual void onAction(const Atlas::Objects::Operation::RootOperation& act, const TypeInfo& typeInfo);

	/**
	Over-rideable hook when this entity is hit by something.
	Default implementation emits the Hit signal.
    */
	virtual void onHit(const Atlas::Objects::Operation::Hit& hit, const TypeInfo& typeInfo);

    /**
    Over-rideable hook when this entity is heard performing an action.
    Default implementation emits the Noise signal.
    */
    virtual void onSoundAction(const Atlas::Objects::Operation::RootOperation& op, const TypeInfo& typeInfo);

    /**
    Over-rideable hook when this entity is seen to emit an imginary op.
    Default implementation emits the Emote signal.
    */
    virtual void onImaginary(const Atlas::Objects::Root& act);

    /** over-rideable hook for when the entity changes from stationary to
    moving or vice-versa. This hook exists so a client can treat moving objects
    differently (eg, placing them in a different part of the scene graph).
    If you over-ride this, you <em>must</em> call the base version, or motion prediction
    will stop working for the entity. */
    virtual void setMoving(bool moving);
    
    /**
    Over-rideable hook when child entities are added. The default implementation
    emits the ChildAdded signal.
    */
    virtual void onChildAdded(Entity* child);
    
    /**
    Over-rideable hook when child entities are removed. The default implementation
    emits the Childremoved signal.
    */
    virtual void onChildRemoved(Entity* child);

    /**
     * @brief Over-rideable hook for when tasks are added.
     * @param id Id of the task.
     * @param task The new task.
     */
    virtual void onTaskAdded(const std::string& id, Task* task);

    friend class IGRouter;
    friend class View;
    friend class Task;
    friend class Avatar;

    /**
     * Fully initialise all entity state based on a RootEntity, including
     * location and contents.
     * This is only called once when the entity is first seen.
    */
    void firstSight(const Atlas::Objects::Entity::RootEntity& gent);
    

    /**
     * @brief Initialise all simple state from a Root. This excludes location and contents, and may optionally exclude all properties related to motion.
     * @param obj The atlas object containing the data.
     * @param includeTypeInfoProperties If true, the default properties of the type info will be used too. This is normally only desired when the entity is initially set up.
     */
    void setFromRoot(const Atlas::Objects::Root& obj, bool includeTypeInfoProperties = false);
    
    /** the View calls this to change local entity visibility. No one else
    should be calling it!*/
    void setVisible(bool vis);
    
    void setProperty(const std::string &p, const Atlas::Message::Element &v);
        
    /** 
    Map Atlas properties to natively stored properties. Should be changed to
    use an integer hash in the future, since this called frequently.
    */
    bool nativePropertyChanged(const std::string &p, const Atlas::Message::Element &v);
    
    /**
     * @brief Connected to the TypeInfo::PropertyeChanges event.
     * This will in turn call the propertyChangedFromTypeInfo, which is overridable in a subclass if so desired.
     * @param propertyName The name of the property which is to be changed.
     * @param element The new element data.
     */
    void typeInfo_PropertyChanges(const std::string& propertyName, const Atlas::Message::Element& element);
    
    /**
     * @brief Called when an property has been changed in the TypeInfo for this entity.
     * If the property doesn't have an instance value local to this entity the event will be processed
     * just like a call to setProperty but without the property being saved in the map of instance properties.
     * @param propertyName The name of the property which is to be changed.
     * @param element The new element data.
     */
    virtual void propertyChangedFromTypeInfo(const std::string& propertyName, const Atlas::Message::Element& element);
    
    
    /**
     * @brief Utility method for recursively filling a map of properties from a TypeInfo instance.
     * The method will recursively call itself to make sure that the topmost TypeInfo is used first. This makes sure that properties are overwritten by newer values, if duplicates exists.
     * @param properties The map of properties to fill.
     * @param typeInfo The type info from which we will copy values, as well as its parents.
     */
    void fillPropertiesFromType(Entity::PropertyMap& properties, const TypeInfo& typeInfo) const;
    
    void beginUpdate();
    void addToUpdate(const std::string& propertyName);
    void endUpdate();

    /** update the entity's location based on Atlas data. This is used by
    the MOVE handler to update the location information. */
    void setLocationFromAtlas(const std::string& locId);
    
    /** setLocation is the core of the entity hierarchy maintenance logic.
    We make setting location the 'fixup' action; addChild / removeChild are
    correspondingly simple. */
    void setLocation(Entity* newLocation);
    
    /// wrapper for setLocation with additional code the retrieve the
    /// location if it's not available right now
    void setContentsFromAtlas(const std::vector<std::string>& contents);

    typedef std::unordered_map<std::string, Entity*> IdEntityMap;
    void buildEntityDictFromContents(IdEntityMap& dict);
    
    void addChild(Entity* e);
    void removeChild(Entity* e);

    void addToLocation();
    void removeFromLocation();

    void updateTasks(const Atlas::Message::Element& e);

    /** recursively update the real visiblity of this entity, and fire
    appropriate signals. */
    void updateCalculatedVisibility(bool wasVisible);
        
    class DynamicState
    {
    public:
        WFMath::Point<3> position;
        WFMath::Vector<3> velocity;
        WFMath::Quaternion orientation;
    };
    
    void updatePredictedState(const WFMath::TimeStamp& t, double simulationSpeed);
    
    /**
     * @brief Gets an entity with the supplied id from the system.
     * @param id The id of the entity to get.
     */
    virtual Entity* getEntity(const std::string& id) = 0;


    PropertyMap m_properties;
    
    TypeInfo* m_type;
    
// primary state, in native form
    Entity* m_location;
    EntityArray m_contents;
    
    const std::string m_id;	///< the Atlas object ID
    std::string m_name;		///< a human readable name
    double m_stamp;		///< last modification time (in seconds)
    bool m_visible;
    bool m_waitingForParentBind;   ///< waiting for parent bind

    WFMath::Vector<3> m_scale;
    WFMath::AxisBox<3> m_bbox;
    WFMath::AxisBox<3> m_bboxUnscaled;
    WFMath::Point<3> m_position;
    WFMath::Vector<3> m_velocity;
    WFMath::Quaternion m_orientation;    
    WFMath::Vector<3> m_acc;
    /**
     * Angular velocity. The magnitude of the vector represents the angle. For performance reasons
     * a copy of the magnitude is stored in m_angularMag.
     */
    WFMath::Vector<3> m_angularVelocity;
    /**
     * The magnitude of the angular velocity. Kept separately for performance.
     */
    double m_angularMag;
    
    DynamicState m_predicted;
    
// extra state and state tracking things
    /** If greater than zero, we are doing a batched update. This suppresses emission
    of the Changed signal until endUpdate is called, so that a number of
    properties may be updated en-masse, generating just one signal. */
    int m_updateLevel;

    /** When a batched property update is in progress, the set tracks the names
    of each modified property. This set is passed as a parameter of the Changed
    callback when endUpdate is called, to allow clients to determine what
    was changed. */
	std::set<std::string> m_modifiedProperties;
        
    typedef sigc::signal<void, const Atlas::Message::Element&> PropertyChangedSignal;
        
    typedef std::unordered_map<std::string, PropertyChangedSignal> ObserverMap;
    ObserverMap m_observers;

    /** This flag should be set when the server notifies that this entity
    has a bounding box. If this flag is not true, the contents of the
    BBox property are undefined.  */
    bool m_hasBBox;
    
	WFMath::TimeStamp m_lastPosTime;
	WFMath::TimeStamp m_lastOrientationTime;
    bool m_moving; ///< flag recording if this entity is current considered in-motion
    
    bool m_recentlyCreated; ///< flag set if this entity was the subject of a sight(create)
    
    std::map<std::string, std::unique_ptr<Task>> m_tasks;
};

inline size_t Entity::numContained() const {
    return m_contents.size();
}

inline Entity* Entity::getContained(size_t index) const {
    return m_contents[index];
}

inline const std::string& Entity::getId() const
{
    return m_id;
}

inline const std::string& Entity::getName() const
{
    return m_name;
}

inline double Entity::getStamp() const
{
    return m_stamp;
}

inline TypeInfo* Entity::getType() const
{
    return m_type;
}

/** the containing entity, or null if this is a top-level visible entity. */
inline Entity* Entity::getLocation() const
{
    return m_location;
}

/** Returns the Entity's position inside its parent in the parent's local system coordinates. **/
inline const WFMath::Point<3>& Entity::getPosition() const
{
    return m_position;
}
/** Returns the entity's velocity as last set explicitly. **/
inline const WFMath::Vector< 3 > & Entity::getVelocity() const
{
    return m_velocity;
}

inline const WFMath::Vector< 3 > & Entity::getAngularVelocity() const
{
    return m_angularVelocity;
}

/** Returns the entity's orientation as last set explicitely. **/
inline const WFMath::Quaternion & Entity::getOrientation() const
{
    return m_orientation;
}

/** Returns the entity's bounding box in the entity's local system coordinates. **/
inline const WFMath::AxisBox< 3 > & Entity::getBBox() const
{
    return m_bbox;
}

inline bool Entity::hasBBox() const
{
    return m_hasBBox;
}

inline const std::map<std::string, std::unique_ptr<Task>>& Entity::getTasks() const
{
    return m_tasks; 
}

template<class C>
inline C Entity::toLocationCoords(const C& c) const
{
    return c.toParentCoords(getPredictedPos(), m_orientation);
}

template<class C>
inline C Entity::fromLocationCoords(const C& c) const
{
    return c.toLocalCoords(getPredictedPos(), m_orientation);
}

inline WFMath::Vector<3> Entity::toLocationCoords(const WFMath::Vector<3>& v) const
{
    return WFMath::Vector<3>(v).rotate(m_orientation);
}

inline WFMath::Vector<3> Entity::fromLocationCoords(const WFMath::Vector<3>& v) const
{
    return WFMath::Vector<3>(v).rotate(m_orientation.inverse());
}

} // of namespace

#endif
