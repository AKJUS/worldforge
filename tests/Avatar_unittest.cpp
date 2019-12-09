// Eris Online RPG Protocol Library
// Copyright (C) 2007 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

// $Id$

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "Eris/Avatar.h"

#include "Eris/Connection.h"
#include "Eris/Account.h"
#include "Eris/ViewEntity.h"
#include "Eris/View.h"
#include "Eris/Log.h"
#include "Eris/EventService.h"

#include "SignalFlagger.h"

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/SmartPtr.h>

#include <sigc++/adaptors/hide.h>

#include <iostream>

class TestConnection : public Eris::Connection {
  public:
    TestConnection(boost::asio::io_service& io_service, 
    		Eris::EventService& eventService,
            const std::string &cnm,
            const std::string& host,
            short port) :
                Eris::Connection(io_service, eventService, cnm, host, port) {
    }

    virtual void send(const Atlas::Objects::Root &obj) {
        std::cout << "Sending " << obj->getParent()
                  << std::endl << std::flush;
    }
};

class TestAccount : public Eris::Account {
  public:
    TestAccount(Eris::Connection & con) : Eris::Account(con) { }

    void setup_insertActiveCharacters(Eris::Avatar * ea) {
        m_activeAvatars.emplace(ea->getId(), std::unique_ptr<Eris::Avatar>(ea));
    }
};

class TestAvatar : public Eris::Avatar {
  public:
    TestAvatar(Eris::Account * ac, std::string mind_id, const std::string & ent_id) :
               Eris::Avatar(*ac, mind_id, ent_id) { }

    void setup_setEntity(Eris::Entity * ent) {
        m_entity = ent;
        m_entityId = ent->getId();
    }

    void test_onEntityAppear(Eris::Entity * ent) {
        onEntityAppear(ent);
    }

    void test_onCharacterChildAdded(Eris::Entity * ent) {
        onCharacterChildAdded(ent);
    }

    void test_onCharacterChildRemoved(Eris::Entity * ent) {
        onCharacterChildRemoved(ent);
    }

    void test_updateWorldTime(double seconds) {
        updateWorldTime(seconds);
    }

    void test_logoutResponse(const Atlas::Objects::Operation::RootOperation & op) {
        logoutResponse(op);
    }

    void test_logoutRequested()
    {
        logoutRequested();
    }

    void test_logoutRequested(const Eris::TransferInfo& info)
    {
        logoutRequested(info);
    }
};

class TestEntity : public Eris::ViewEntity {
  public:
    TestEntity(const std::string& id, Eris::TypeInfo* ty, Eris::View& vw) :
               Eris::ViewEntity(id, ty, vw) { }

    void setup_setLocation(Eris::Entity * e) {
        setLocation(e);
    }
};

static void writeLog(Eris::LogLevel, const std::string & msg)
{       
    std::cerr << msg << std::endl << std::flush;
}

int main()
{
	Atlas::Objects::Factories factories;
    Eris::Logged.connect(sigc::ptr_fun(writeLog));
    Eris::setLogLevel(Eris::LOG_DEBUG);

    // Test constructor
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        new TestAvatar(acc, fake_mind_id, fake_char_id);
    }

    // Test destructor
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        TestAccount * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar* ea = new TestAvatar(acc, fake_mind_id, fake_char_id);

        delete ea;
    }

    // Test deactivate()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        Eris::Avatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);

        ea->deactivate();
    }

    // Test drop() of something not in inventory
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);

        ea->drop(inv_ent, WFMath::Point<3>(0,0,0), WFMath::Quaternion(), "3");
    }
    
    // Test drop() of something in inventory by position
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        inv_ent->setup_setLocation(char_ent);

        ea->drop(inv_ent, WFMath::Point<3>(0,0,0), WFMath::Quaternion(), "3");
    }

    // Test drop() of something in inventory by position and orientation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        inv_ent->setup_setLocation(char_ent);

        ea->drop(inv_ent, WFMath::Point<3>(0,0,0), WFMath::Quaternion(1, 90), "3");
    }
    
    // Test drop() of something in inventory by offset
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        char_ent->setup_setLocation(wrld_ent);
        ea->setup_setEntity(char_ent);
        inv_ent->setup_setLocation(char_ent);

        ea->drop(inv_ent, WFMath::Vector<3>(0,0,0));
    }

    // Test drop() of something in inventory by offset and orientation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        char_ent->setup_setLocation(wrld_ent);
        ea->setup_setEntity(char_ent);
        inv_ent->setup_setLocation(char_ent);

        ea->drop(inv_ent, WFMath::Vector<3>(0,0,0), WFMath::Quaternion(1, 90));
    }

    // Test take() of something
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);
        inv_ent->setup_setLocation(wrld_ent);

        ea->take(inv_ent);
    }
    
    // Test touch() of something
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);
        inv_ent->setup_setLocation(wrld_ent);

        ea->touch(inv_ent, WFMath::Point<3>());
    }
    
    // Test say() of a message
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);

        ea->say("Hello world");
    }
    
    // Test sayTo() of a message, with one addressed
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1"), fake_npc_id("2");
        std::string fake_mind_id = "12";
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * npc_ent = new TestEntity(fake_npc_id, 0, ea->getView());
        std::vector<std::string> entityIds;
        entityIds.push_back(npc_ent->getId());

        ea->setup_setEntity(char_ent);

        ea->sayTo("Hello world", entityIds);
    }

    // Test sayTo() of a message, with two addressed
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1"), fake_npc_id("2"), fake_npc2_id("3");
        std::string fake_mind_id = "12";
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * npc_ent = new TestEntity(fake_npc_id, 0, ea->getView());
        TestEntity * npc2_ent = new TestEntity(fake_npc2_id, 0, ea->getView());
        std::vector<std::string> entityIds;
        entityIds.push_back(npc_ent->getId());
        entityIds.push_back(npc2_ent->getId());

        ea->setup_setEntity(char_ent);

        ea->sayTo("Hello world", entityIds);
    }

    // Test sayTo() of a message, with none addressed (still valid)
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        std::vector<std::string> entityIds;

        ea->setup_setEntity(char_ent);

        ea->sayTo("Hello world", entityIds);
    }

    // Test emote() of a message
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);

        ea->emote("greets the world.");
    }
    
    // Test moveToPoint()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);

        ea->moveToPoint(WFMath::Point<3>(3,4,5), WFMath::Quaternion());
    }
    
    // Test moveInDirection() with zero velocity
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);

        ea->moveInDirection(WFMath::Vector<3>(0,0,0), WFMath::Quaternion());
    }
    
    // Test moveInDirection() with vertical velocity
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);

        ea->moveInDirection(WFMath::Vector<3>(0,0,5), WFMath::Quaternion());
    }
    
    // Test moveInDirection() with high velocity
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);

        ea->moveInDirection(WFMath::Vector<3>(3,4,5), WFMath::Quaternion());
    }
    
    // FIXME test the conditionals in moveInDirection()

    // Test moveInDirection()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);

        ea->moveInDirection(WFMath::Vector<3>(3,4,5), WFMath::Quaternion());
    }
    
    // Test place() of something
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);
        inv_ent->setup_setLocation(wrld_ent);

        ea->place(inv_ent, wrld_ent, WFMath::Point<3>(1,2,3));
    }
    
    // Test place() of something as admin
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);
        inv_ent->setup_setLocation(wrld_ent);

        ea->setIsAdmin(true);
        ea->place(inv_ent, wrld_ent, WFMath::Point<3>(1,2,3));
    }
    
    // Test place() of something with no pos
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);
        inv_ent->setup_setLocation(wrld_ent);

        ea->place(inv_ent, wrld_ent, WFMath::Point<3>());
    }
    
    // Test place() of something with orientation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        TestEntity * inv_ent = new TestEntity("2", 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);
        inv_ent->setup_setLocation(wrld_ent);

        ea->place(inv_ent, wrld_ent, WFMath::Point<3>(),
                  WFMath::Quaternion(1.f, 0.f, 0.f, 0.f));
    }
    

    // Test useStop()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * wrld_ent = new TestEntity("0", 0, ea->getView());
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());

        ea->setup_setEntity(char_ent);
        char_ent->setup_setLocation(wrld_ent);

        ea->useStop();
    }
    
    // Test onEntityAppear() for avatar entity
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity(fake_char_id, 0, ea->getView());
        SignalFlagger gotCharacterEntity;

        ea->GotCharacterEntity.connect(sigc::hide(sigc::mem_fun(gotCharacterEntity, &SignalFlagger::set)));

        ea->test_onEntityAppear(char_ent);

        assert(ea->getEntity() == char_ent);
        assert(gotCharacterEntity.flagged());
    }
    
    // Test onEntityAppear() for a different entity
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_char_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_char_id);
        TestEntity * char_ent = new TestEntity("2", 0, ea->getView());
        SignalFlagger gotCharacterEntity;

        ea->GotCharacterEntity.connect(sigc::hide(sigc::mem_fun(gotCharacterEntity, &SignalFlagger::set)));

        ea->test_onEntityAppear(char_ent);

        assert(ea->getEntity() != char_ent);
        assert(!gotCharacterEntity.flagged());
    }
    
    // Test onCharacterChildAdded()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        TestEntity * ent = new TestEntity("2", 0, ea->getView());
        SignalFlagger invAdded;

        ea->InvAdded.connect(sigc::hide(sigc::mem_fun(invAdded, &SignalFlagger::set)));

        ea->test_onCharacterChildAdded(ent);

        assert(invAdded.flagged());
    }
    
    // Test onCharacterChildRemoved()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        TestEntity * ent = new TestEntity("2", 0, ea->getView());
        SignalFlagger invRemoved;

        ea->InvRemoved.connect(sigc::hide(sigc::mem_fun(invRemoved, &SignalFlagger::set)));

        ea->test_onCharacterChildRemoved(ent);

        assert(invRemoved.flagged());
    }

    // Test getConnection()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);

        assert(&ea->getConnection() == con);
    }

    // Test getWorldTime()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);

        ea->getWorldTime();
    }

    // Test updateWorldTime()
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);

        ea->test_updateWorldTime(100.0);
    }

    // Test logoutResponse() with a non-info operation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        Atlas::Objects::Operation::Get op;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));

        ea->test_logoutResponse(op);

        assert(!avatarDeactivated.flagged());
    }

    // Test logoutResponse() with an empty info operation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        Atlas::Objects::Operation::Info op;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));

        ea->test_logoutResponse(op);

        assert(!avatarDeactivated.flagged());
    }

    // Test logoutResponse() with an info operation with bad arg
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        Atlas::Objects::Operation::Info op;
        Atlas::Objects::Root bad_arg;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));
        op->setArgs1(bad_arg);

        ea->test_logoutResponse(op);

        assert(!avatarDeactivated.flagged());
    }

    // Test logoutResponse() with an empty info logout operation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        Atlas::Objects::Operation::Info op;
        Atlas::Objects::Operation::Logout logout;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));
        op->setArgs1(logout);

        ea->test_logoutResponse(op);

        assert(!avatarDeactivated.flagged());
    }

    // Test logoutResponse() with a non-empty info logout operation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        Eris::Account * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        Atlas::Objects::Operation::Info op;
        Atlas::Objects::Operation::Logout logout;
        Atlas::Objects::Root logout_arg;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));
        op->setArgs1(logout);
        logout->setArgs1(logout_arg);

        ea->test_logoutResponse(op);

        assert(!avatarDeactivated.flagged());
    }

    // Test logoutResponse() with a non-empty info logout operation
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        TestAccount * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        // The account must know about this Avatar, as avatar removes itself
        // from account on destruction.
        acc->setup_insertActiveCharacters(ea);
        Atlas::Objects::Operation::Info op;
        Atlas::Objects::Operation::Logout logout;
        Atlas::Objects::Root logout_arg;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));
        op->setArgs1(logout);
        logout->setArgs1(logout_arg);
        logout_arg->setId(fake_mind_id);

        ea->test_logoutResponse(op);

        assert(avatarDeactivated.flagged());
    }

    // Test logoutRequested() without any transfer info
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        TestAccount * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        acc->setup_insertActiveCharacters(ea);

        SignalFlagger avatarTransferRequested;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));
        ea->TransferRequested.connect(sigc::hide(sigc::mem_fun(avatarTransferRequested, &SignalFlagger::set)));

        ea->test_logoutRequested();

        assert(!avatarTransferRequested.flagged() && avatarDeactivated.flagged());
    }

    // Test logoutRequested() with a transfer info
    {
        boost::asio::io_service io_service;
        Eris::EventService event_service(io_service);
        Eris::Connection * con = new TestConnection(io_service, event_service, "name",
                "localhost", 6767);

        TestAccount * acc = new TestAccount(*con);
        std::string fake_id("1");
		std::string fake_mind_id("12");
        TestAvatar * ea = new TestAvatar(acc, fake_mind_id, fake_id);
        acc->setup_insertActiveCharacters(ea);

        SignalFlagger avatarTransferRequested;
        SignalFlagger avatarDeactivated;

        acc->AvatarDeactivated.connect(sigc::hide(sigc::mem_fun(avatarDeactivated, &SignalFlagger::set)));
        ea->TransferRequested.connect(sigc::hide(sigc::mem_fun(avatarTransferRequested, &SignalFlagger::set)));

        Eris::TransferInfo transfer("localhost", 6768, "key", "id");
        ea->test_logoutRequested(transfer);

        assert(avatarTransferRequested.flagged() && avatarDeactivated.flagged());
    }
    return 0;
}
