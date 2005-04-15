#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "controller.h"

#include <cassert>

#include <Atlas/Net/Stream.h>
#include <Atlas/Codec.h>
#include <Atlas/Objects/Encoder.h>
#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Entity.h>
#include <Eris/Exceptions.h>
#include <Eris/Avatar.h>
#include <wfmath/atlasconv.h>

using std::endl;
using std::cout;

using namespace Atlas::Objects::Operation;
using Atlas::Objects::Entity::RootEntity;
using Atlas::Objects::Root;

Controller::Controller(int fd) :
    m_stream(fd)
{
    assert(m_stream.is_open());
    
// force synchrous negotation now
    Atlas::Bridge* br = this;
    Atlas::Net::StreamConnect sc("eristest_oob", m_stream, *br);
    
    // spin (and block) while we negotiate
    do { sc.poll(); } while (sc.getState() == Atlas::Net::StreamConnect::IN_PROGRESS);
    
    if (sc.getState() == Atlas::Net::StreamConnect::FAILED)
        throw Eris::InvalidOperation("controller negotation failed");
            
    assert(sc.getState() == Atlas::Net::StreamConnect::SUCCEEDED);
    
    m_codec = sc.getCodec();
    m_encode = new Atlas::Objects::ObjectsEncoder(*m_codec);
    m_codec->streamBegin();
}

void Controller::objectArrived(const Atlas::Objects::Root&)
{
    cout << "controller recieved op!" << endl;
}

void Controller::send(const Atlas::Objects::Root &obj)
{
    m_encode->streamObjectsMessage(obj);
    m_stream << std::flush;
}

#pragma mark -

void Controller::setEntityVisibleToAvatar(const std::string& eid, Eris::Avatar* av)
{
    setEntityVisibleToAvatar(eid, av->getId());
}

void Controller::setEntityVisibleToAvatar(const std::string& eid, const std::string& charId)
{
    Appearance app;
    app->setTo(eid);
    app->setAttr("for", charId);
    
    send(app);
}

void Controller::setEntityInvisibleToAvatar(const std::string& eid, Eris::Avatar* av)
{
    setEntityInvisibleToAvatar(eid, av->getId());
}

void Controller::setEntityInvisibleToAvatar(const std::string& eid, const std::string& charId)
{
    Disappearance dap;
    dap->setTo(eid);
    dap->setAttr("for", charId);
    send(dap);
}

void Controller::setAttr(const std::string& eid, const std::string& attr, const Atlas::Message::Element v)
{
    Set s;
    
    Atlas::Objects::Root obj;
    obj->setId(eid);
    obj->setAttr(attr, v);
    s->setArgs1(obj);
    
    send(s);
}

void Controller::create(const Atlas::Objects::Entity::GameEntity& ent)
{
    Create c;
    c->setArgs1(ent);
    send(c);
}

void Controller::moveLocation(const std::string& eid, const std::string& loc, const WFMath::Point<3>& pos)
{
    Move mv;
    RootEntity arg;
    arg->setLoc(loc);
    arg->setAttr("pos", pos.toAtlas());
    mv->setTo(eid);
    mv->setFrom(eid);
    mv->setArgs1(arg);
    send(mv);
}

void Controller::movePos(const std::string& eid, const WFMath::Point<3>& pos)
{
    Move mv;
    Root arg;
    arg->setAttr("pos", pos.toAtlas());
    mv->setTo(eid);
    mv->setFrom(eid);
    mv->setArgs1(arg);
    send(mv);
}

 void Controller::broadcastSoundFrom(const std::string& e, const Atlas::Objects::Operation::RootOperation& op)
 {
    Sound snd;
    snd->setFrom(e);
    snd->setAttr("broadcast", 1);
    snd->setArgs1(op);
    send(snd);
 }

 void Controller::broadcastSightFrom(const std::string& e, const Atlas::Objects::Operation::RootOperation& op)
 {
    Sight st;
    st->setFrom(e);
    st->setAttr("broadcast", 1);
    st->setArgs1(op);
    send(st);
 }
