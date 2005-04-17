#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <Eris/Connection.h>

#include <Eris/Timeout.h>
#include <Eris/TypeInfo.h>
#include <Eris/Poll.h>
#include <Eris/Log.h>
#include <Eris/Exceptions.h>
#include <Eris/Router.h>
#include <Eris/Redispatch.h>
#include <Eris/Response.h>

#include <skstream/skstream.h>
#include <Atlas/Objects/Encoder.h>
#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Entity.h>
#include <sigc++/bind.h>
#include <sigc++/object_slot.h>

#include <Atlas/Codecs/Bach.h>

#include <cassert>
#include <algorithm>

// #define ATLAS_LOG 1

using namespace Atlas::Objects::Operation;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::RootEntity;
using Atlas::Objects::smart_dynamic_cast;

namespace Eris {

Connection::Connection(const std::string &cnm, const std::string& host, short port, bool dbg) :
    BaseConnection(cnm, "game_", this),
    _host(host),
    _port(port),
    m_typeService(new TypeService(this)),
    m_defaultRouter(NULL),
    m_lock(0),
    m_info(""),
    m_responder(new ResponseTracker)
{	
    Poll::instance().Ready.connect(SigC::slot(*this, &Connection::gotData));
}
	
Connection::~Connection()
{
    // ensure we emit this before our vtable goes down, since we are the
    // Bridge on the underlying Atlas codec, and otherwise we might get
    // a pure virtual method call
    hardDisconnect(true);
}

int Connection::connect()
{
    return BaseConnection::connect(_host, _port);
}

int Connection::disconnect()
{
    if (_status == DISCONNECTING) {
        warning() << "duplicate disconnect on Connection that's already disconnecting";
        return -1;
    }

    if (_status == DISCONNECTED) {
        warning() << "called disconnect on already disconnected Connection";
        return -1;
    }
    
    assert(m_lock == 0);
	
    // this is a soft disconnect; it will give people a chance to do tear down and so on
    // in response, people who need to hold the disconnect will lock() the
    // connection, and unlock when their work is done. A timeout stops
    // locks from preventing disconnection
    setStatus(DISCONNECTING);
    Disconnecting.emit();

    if (m_lock == 0) {
        hardDisconnect(true);
        return 0;
    }
    
    // fell through, so someone has locked =>
    // start a disconnect timeout
    _timeout = new Eris::Timeout("disconnect_" + _host, this, 5000);
    _timeout->Expired.connect(SigC::slot(*this, &Connection::onDisconnectTimeout));
    return 0;
}
   
void Connection::gotData(PollData &data)
{
    if (!_stream || !data.isReady(_stream))
        return;
	
    if (_status == DISCONNECTED) {
        error() << "Got data on a disconnected stream";
        return;
    }
   
    BaseConnection::recv();
    
// now dispatch recieved ops
    while (!m_opDeque.empty()) {
        dispatchOp(m_opDeque.front());
        m_opDeque.pop_front();
    }
    
// finally, clean up any redispatches that fired (aka 'deleteLater')
    for (unsigned int D=0; D < m_finishedRedispatches.size(); ++D)
        delete m_finishedRedispatches[D];
        
    m_finishedRedispatches.clear();
}		

void Connection::send(const Atlas::Objects::Root &obj)
{
    if ((_status != CONNECTED) && (_status != DISCONNECTING))
    {
        error() << "called send on closed connection";
        return;
    }

#ifdef ATLAS_LOG	
    std::stringstream debugStream;
    
    Atlas::Codecs::Bach debugCodec(debugStream, *this /*dummy*/);
    Atlas::Objects::ObjectsEncoder debugEncoder(debugCodec);
    debugEncoder.streamObjectsMessage(obj);
    debugStream << std::flush;

    std::cout << "sending:" << debugStream.str() << std::endl;
#endif
        
    _encode->streamObjectsMessage(obj);
    (*_stream) << std::flush;
}

void Connection::registerRouterForTo(Router* router, const std::string toId)
{
    m_toRouters[toId] = router;
}

void Connection::unregisterRouterForTo(Router* router, const std::string toId)
{
    assert(m_toRouters[toId] == router);
    m_toRouters.erase(toId);
}
        
void Connection::registerRouterForFrom(Router* router, const std::string fromId)
{
    m_fromRouters[fromId] = router;
}

void Connection::unregisterRouterForFrom(Router* router, const std::string fromId)
{
    assert(m_fromRouters[fromId] == router);
    m_fromRouters.erase(fromId);
}
        
void Connection::setDefaultRouter(Router* router)
{
    if (m_defaultRouter || !router) {
        error() << "setDefaultRouter duplicate set or null argument";
        return;
    }
    
    m_defaultRouter = router;
}

void Connection::clearDefaultRouter()
{
    m_defaultRouter = NULL;
}
    
void Connection::lock()
{
    ++m_lock;
}

void Connection::unlock()
{
    if (m_lock < 1)
        throw InvalidOperation("Imbalanced lock/unlock calls on Connection");
	
    if (--m_lock == 0) {
        switch (_status)
        {
        case DISCONNECTING:	
            debug() << "Connection unlocked in DISCONNECTING, closing socket";
            hardDisconnect(true);
            break;

        default:
            warning() << "Connection unlocked in spurious state : this may cause a failure later";
        }
    }
}

void Connection::getServerInfo(ServerInfo& si) const
{
    si = m_info;
}

void Connection::refreshServerInfo()
{
    if (_status != CONNECTED) {
        warning() << "called refreshServerInfo while not connected, ignoring";
        return;
    }
    
    m_info.setStatus(ServerInfo::QUERYING);
    Get gt;
    gt->setSerialno(getNewSerialno());
    send(gt);
}

#pragma mark -

void Connection::objectArrived(const Root& obj)
{
#ifdef ATLAS_LOG
    std::stringstream debugStream;
    Atlas::Codecs::Bach debugCodec(debugStream, *this /* dummy */);
    Atlas::Objects::ObjectsEncoder debugEncoder(debugCodec);
    debugEncoder.streamObjectsMessage(obj);
    debugStream << std::flush;

    std::cout << "recieved:" << debugStream.str() << std::endl;
#endif
    if (!m_typeService->verifyObjectTypes(obj)) return;
    
    RootOperation op = smart_dynamic_cast<RootOperation>(obj);
    if (op.isValid()) {
        m_opDeque.push_back(op);
    } else
        error() << "Con::objectArrived got non-op";
}

void Connection::dispatchOp(const RootOperation& op)
{
    Router::RouterResult rr;
    bool anonymous = op->getTo().empty();
    
    if (m_responder->handleOp(op)) return;
    
// locate a router based on from
    IdRouterMap::const_iterator R = m_fromRouters.find(op->getFrom());
    if (R != m_fromRouters.end()) {
        rr = R->second->handleOperation(op);
        if ((rr == Router::HANDLED) || (rr == Router::WILL_REDISPATCH))
            return;
    }
    
// locate a router based on the op's TO value
    R = m_toRouters.find(op->getTo());
    if (R != m_toRouters.end()) {
        rr = R->second->handleOperation(op);
        if ((rr == Router::HANDLED) || (rr == Router::WILL_REDISPATCH))
            return;
    } else if (!anonymous && !m_toRouters.empty())
        warning() << "recived op with TO=" << op->getTo() << ", but no router is registered for that id";
            
// go to the default router
    rr = m_defaultRouter->handleOperation(op);
    if (rr != Router::HANDLED)
        warning() << "no-one handled op:" << op;
}


void Connection::setStatus(Status ns)
{
	if (_status != ns) {
		StatusChanged.emit(ns);
	}
	_status = ns;
}

void Connection::handleFailure(const std::string &msg)
{
	Failure.emit(msg);
	// FIXME - reset I think, but ensure this is safe
    m_lock= 0;
}

void Connection::handleTimeout(const std::string& msg)
{
    handleFailure(msg); // all the same in the end
}

void Connection::handleServerInfo(const RootOperation& op)
{
    RootEntity svr = smart_dynamic_cast<RootEntity>(op->getArgs().front());	
    if (!svr.isValid()) {
        error() << "server INFO argument object is broken";
        return;
    }
    
    m_info.processServer(svr);
    GotServerInfo.emit();
}

void Connection::onConnect()
{
    BaseConnection::onConnect();
    m_typeService->init();
    m_info = ServerInfo(_host);
}

void Connection::onDisconnectTimeout()
{
    handleTimeout("timed out waiting for disconnection");
    hardDisconnect(true);
}

void Connection::postForDispatch(const Root& obj)
{
    if (!m_typeService->verifyObjectTypes(obj)) {
        debug() << "amazingly, re-dispatched object failed to verify!";
        return;
    }
    
    RootOperation op = smart_dynamic_cast<RootOperation>(obj);
    m_opDeque.push_back(op);
}

void Connection::cleanupRedispatch(Redispatch* r)
{
    m_finishedRedispatches.push_back(r);
}

#pragma mark -

long getNewSerialno()
{
	static long _nextSerial = 1001;
	// note this will eventually loop (in theorey), but that's okay
	// FIXME - using the same intial starting offset is problematic
	// if the client dies, and quickly reconnects
	return _nextSerial++;
}

} // of namespace
