#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "Room.h"
#include "Lobby.h"
#include "Connection.h"
#include "Person.h"
#include "Log.h"
#include "Exceptions.h"
#include "Account.h"

#include <sigc++/slot.h>

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Anonymous.h>

#include <cassert>

using namespace Atlas::Objects::Operation;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::RootEntity;
using Atlas::Objects::Entity::Anonymous;
using Atlas::Objects::smart_dynamic_cast;

namespace Eris
{
	
Room::Room(Lobby *l, const std::string& id) :
    m_roomId(id),
    m_entered(false),
    m_lobby(l)
{
    if (!id.empty()) {
        m_lobby->getConnection().registerRouterForFrom(this, id);
    }
}

Room::~Room()
{
    if (!m_roomId.empty()) {
        m_lobby->getConnection().unregisterRouterForFrom(m_roomId);
    }
}

// public command-issue wrappers

void Room::say(const std::string &tk)
{
    if (!m_lobby->getConnection().isConnected())
    {
        logger->error("talking in room {}, but connection is down", m_roomId);
        return;
    }
	
    Anonymous speech;
    speech->setAttr("say", tk);
    speech->setAttr("loc", m_roomId);
	
    Talk t;
    t->setArgs1(speech);
    t->setTo(m_roomId);
    t->setFrom(m_lobby->getAccount().getId());
    t->setSerialno(getNewSerialno());
	
    m_lobby->getConnection().send(t);
}

void Room::emote(const std::string &em)
{
    if (!m_lobby->getConnection().isConnected())
    {
        logger->error("emoting in room {}, but connection is down", m_roomId);
        return;
    }
	
    Imaginary im;
	
    Anonymous emote;
    emote->setId("emote");
    emote->setAttr("loc", m_roomId);
    emote->setAttr("description", em);
    
    im->setArgs1(emote);
    im->setTo(m_roomId);
    im->setFrom(m_lobby->getAccount().getId());
    im->setSerialno(getNewSerialno());
	
    m_lobby->getConnection().send(im);
}

void Room::leave()
{
    if (!m_lobby->getConnection().isConnected())
    {
        logger->error("leaving room {}, but connection is down", m_roomId);
        return;
    }

    Move part;
    part->setFrom(m_lobby->getAccount().getId());
    part->setSerialno(getNewSerialno());
    
    Anonymous args;
    args->setAttr("loc", m_roomId);
    args->setAttr("mode", "part");
    part->setArgs1(args);

    m_lobby->getConnection().send(part);
}

Room* Room::createRoom(const std::string &name)
{
    if (!m_lobby->getConnection().isConnected())
    {
        logger->error("creating room in room {}, but connection is down", m_roomId);
        return nullptr;
    }

    
    Create cr;
    cr->setFrom(m_lobby->getAccount().getId());
    cr->setTo(m_roomId);
    cr->setSerialno(getNewSerialno());
    
    RootEntity room;
    room->setName(name);
    room->setParent("room");
    
    cr->setArgs1(room);
    m_lobby->getConnection().send(cr);
    
    return nullptr;
}

Person* Room::getPersonByUID(const std::string& uid)
{
    return m_lobby->getPerson(uid);
}

std::vector<Person*> Room::getPeople() const
{
    std::vector<Person*> people;
    
    for (const auto & member : m_members)
    {    
        if (member.second) {
            people.push_back(member.second);
        }
    }
    
    return people;
}

Router::RouterResult Room::handleOperation(const RootOperation& op)
{
    if (op->getTo() != m_lobby->getAccount().getId()) {
        logger->error("Room received op TO account {}, not the account ID", op->getTo());
        return IGNORED;
    }

    const std::vector<Root>& args = op->getArgs();
    
    if (op->instanceOf(APPEARANCE_NO)) {
        for (const auto & arg : args) {
			appearance(arg->getId());
		}

        return HANDLED;
    }

    if (op->instanceOf(DISAPPEARANCE_NO)) {
        for (const auto & arg : args) {
			disappearance(arg->getId());
		}
        
        return HANDLED;
    }

    if (op->instanceOf(SIGHT_NO)) {
        assert(!args.empty());
        auto ent = smart_dynamic_cast<RootEntity>(args.front());
            
        if (ent.isValid() && (ent->getId() == m_roomId)) {
            sight(ent);
            return HANDLED;
        }
    }

    return IGNORED;
}

void Room::sight(const RootEntity &room)
{
    if (m_entered)
        logger->warn("got SIGHT of entered room {}", m_roomId);
        
    m_name = room->getName();
    if (room->hasAttr("topic"))
        m_topic = room->getAttr("topic").asString();
    
    m_lobby->SightPerson.connect(sigc::mem_fun(*this, &Room::notifyPersonSight));
    
    if (room->hasAttr("people"))
    {
        auto people = room->getAttr("people").asList();
        for (const auto & person : people) {
			appearance(person.asString());
        }
    }
    
    checkEntry();
    
    if (room->hasAttr("rooms"))
    {
        auto rooms = room->getAttr("rooms").asList();
        for (const auto & item : rooms)
        {
            m_subrooms.push_back(new Room(m_lobby, item.asString()));
        }
    }
}

void Room::handleSoundTalk(Person* p, const std::string& speech)
{
    assert(p);
    
    if (m_members.count(p->getAccount()) == 0) {
        logger->error("room {} got sound(talk) from non-member account", m_roomId);
        return;
    }
    
    Speech.emit(this, p, speech);
}

void Room::handleEmote(Person* p, const std::string& description)
{
    assert(p);
    
    if (m_members.count(p->getAccount()) == 0) {
        logger->error("room {} got sight(imaginary) from non-member account", m_roomId);
        return;
    }

    Emote.emit(this, p, description);
}

// room membership updates

void Room::appearance(const std::string& personId)
{
    auto P = m_members.find(personId);
    if (P != m_members.end()) {
        logger->error("duplicate appearance of person {} in room {}", m_roomId, personId);
        return;
    }
    
    Person* person = m_lobby->getPerson(personId);
    if (person)
    {
        m_members[personId] = person;
        if (m_entered)
            Appearance.emit(this, person);
    } else {
        m_members[personId] = nullptr; // we know the person is here, but that's all
        // we'll find out more when we get the SightPerson signal from Lobby
    }
}

void Room::disappearance(const std::string& personId)
{
    auto P = m_members.find(personId);
    if (P == m_members.end())
    {
        logger->error("during disappearance, person {} not found in room {}",personId, m_roomId);
        return;
    }
    
    if (P->second) // don't emit if never got sight
        Disappearance.emit(this, P->second);
    
    m_members.erase(P);
}

void Room::notifyPersonSight(Person *p)
{
    assert(p);
    auto P = m_members.find(p->getAccount());
    // for the moment, all rooms get spammed with sights of people, to avoid
    // the need for a counting / disconnect from SightPerson scheme
    if (P == m_members.end()) {
        return;
    }
    
    if (P->second == nullptr) {
        m_members[p->getAccount()] = p;

        if (m_entered) {
            Appearance.emit(this, p);
        } else {
            checkEntry();
        }
    } else {
        // fairly meaningless case, but I'm paranoid
        // could fire a 'changed' signal here, eg if they renamed?
        assert (P->second == p);
    }
}

void Room::checkEntry()
{
    assert(!m_entered);
    
    bool anyPending = false;
    for (auto& entry : m_members) {
        if (entry.second == nullptr) {
            anyPending = true;
        }
    }

    if (!anyPending)
    {
        Entered.emit(this);
        m_entered = true;
    }
}

} // of Eris namespace
