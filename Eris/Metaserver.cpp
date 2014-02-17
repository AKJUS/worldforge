#include <Eris/MetaQuery.h>

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <Eris/Metaserver.h>

#include <Eris/ServerInfo.h>
#include <Eris/Poll.h>
#include <Eris/Log.h>
#include <Eris/DeleteLater.h>
#include "Exceptions.h"

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/RootEntity.h>
#include <sigc++/slot.h>

#include <algorithm>

#include <cassert>
#include <cstdio>
#include <cstring>

#ifdef _WIN32

#ifndef snprintf
#define snprintf _snprintf
#endif

#endif // _WIN32

using namespace Atlas::Objects::Operation;
using Atlas::Objects::smart_dynamic_cast;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::RootEntity;

namespace Eris {

char* pack_uint32(uint32_t data, char* buffer, unsigned int &size);
char* unpack_uint32(uint32_t &dest, char* buffer);

const char* META_SERVER_PORT = "8453";

// meta-server protocol commands	
const uint32_t CKEEP_ALIVE = 2,
	HANDSHAKE = 3,
	CLIENTSHAKE = 5,
	LIST_REQ = 7,
	LIST_RESP = 8,
	PROTO_ERANGE = 9,
	LAST = 10;

// special  command value to track LIST_RESP processing
const uint32_t LIST_RESP2 = 999;		
	
#pragma mark -

Meta::Meta(boost::asio::io_service& io_service, const std::string& metaServer, unsigned int maxQueries) :
    m_io_service(io_service),
    m_status(INVALID),
    m_metaHost(metaServer),
    m_maxActiveQueries(maxQueries),
    m_nextQuery(0),
    m_resolver(io_service),
    m_socket(io_service),
    m_metaTimer(io_service),
    m_stream(&m_buffer)
{
    unsigned int max_half_open = FD_SETSIZE;
    if (m_maxActiveQueries > (max_half_open - 2)) {
        m_maxActiveQueries = max_half_open - 2;
    }
}

Meta::~Meta()
{
    disconnect();
      
    // delete any outstanding queries
    for (QuerySet::const_iterator Q = m_activeQueries.begin(); Q != m_activeQueries.end(); ++Q) {
        delete *Q;
    }
}

/*
void Meta::queryServer(const std::string &ip)
{
    m_status = QUERYING;
    
    if (m_activeQueries.size() < m_maxActiveQueries)
    {
        MetaQuery *q =  new MetaQuery(this, ip);
        if (q->isComplete())
        {
            // indicated a failure occurred, so we'll kill it now and say no more
            delete q;
        } else
            m_activeQueries.insert(q);
    }
}
*/

void Meta::queryServerByIndex(unsigned int index)
{
    if (m_status == INVALID) {
        error() << "called queryServerByIndex with invalid server list";
        return;
    }
    
    if (index >= m_gameServers.size()) {
        error() << "called queryServerByIndex with bad server index " << index;
        return;
    }
    
    if (m_gameServers[index].getStatus() == ServerInfo::QUERYING) {
        warning() << "called queryServerByIndex on server already being queried";
        return;
    }

    internalQuery(index);
}

void Meta::refresh()
{
    if (!m_activeQueries.empty()) {
        warning() << "called meta::refresh() while doing another query, ignoring";
        return;
    }

    if (m_status == VALID)
    {
    	// save the current list in case we fail
        m_lastValidList = m_gameServers;
    }

    m_gameServers.clear();
    m_nextQuery = 0;
    connect();
}

void Meta::cancel()
{
    for (QuerySet::iterator Q=m_activeQueries.begin(); Q!=m_activeQueries.end();++Q)
        delete *Q;
    m_activeQueries.clear();
	
    disconnect();

    // revert to the last valid list if possible	
    if (!m_lastValidList.empty())
    {
        m_gameServers = m_lastValidList;
        m_status = VALID;
    } else {
        m_status = INVALID;
        m_gameServers.clear();
    }
    m_nextQuery = m_gameServers.size();
}

const ServerInfo& Meta::getInfoForServer(unsigned int index) const
{
    if (index >= m_gameServers.size()) {
        error() << "passed out-of-range index " << index <<
            " to getInfoForServer";
        throw BaseException("Out of bounds exception when getting server info.");
    } else {
        return m_gameServers[index];
    }
}
                
unsigned int Meta::getGameServerCount() const
{
    return m_gameServers.size();
}

#pragma mark -

void Meta::connect()
{
    boost::asio::ip::udp::resolver::query query(m_metaHost, META_SERVER_PORT);
    m_resolver.async_resolve(query,
            [&](const boost::system::error_code& ec, boost::asio::ip::udp::resolver::iterator iterator) {
                if (!ec && iterator != boost::asio::ip::udp::resolver::iterator()) {
                    this->connect(*iterator);
                } else {
                    this->disconnect();
                }
            });
}

void Meta::connect(boost::asio::ip::udp::endpoint endpoint)
{
    disconnect();
    m_socket.async_connect(endpoint, [&](boost::system::error_code ec){
        if (!ec) {
            do_read();

            // build the initial 'ping' and send
            unsigned int dsz = 0;
            pack_uint32(CKEEP_ALIVE, _data, dsz);
            this->m_stream << std::string(_data, dsz) << std::flush;
            this->write();
            this->setupRecvCmd();

            this->m_status = GETTING_LIST;
            this->startTimeout();
        } else {
            this->doFailure("Couldn't open connection to metaserver " + this->m_metaHost);
        }
    });
}

void Meta::disconnect()
{
    m_socket.close();
    m_metaTimer.cancel();
}

void Meta::startTimeout()
{
    m_metaTimer.cancel();
    m_metaTimer.expires_from_now(boost::posix_time::seconds(8));
    m_metaTimer.async_wait([&](boost::system::error_code ec){
        if (!ec) {
            this->metaTimeout();
        }
    });
}


void Meta::do_read()
{
    query();
    m_socket.async_receive(m_buffer.prepare(DATA_BUFFER_SIZE),
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    m_buffer.commit(length);
                    if (length > 0) {
                        this->gotData();
                    }
                    this->write();
                    this->do_read();
                } else {
                    this->doFailure("Connection to the meta-server failed");
                }
            });
}

void Meta::write()
{
    if (m_buffer.size() != 0) {
        m_socket.async_send(m_buffer.data(),
                [&](boost::system::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        m_buffer.consume(length);
                    } else {
                        this->doFailure("Connection to the meta-server failed");
                    }
                });
    }
}

void Meta::gotData()
{    
    recv();
    
    std::vector<MetaQuery*> complete;
    
    for (QuerySet::iterator Q=m_activeQueries.begin(); Q != m_activeQueries.end(); ++Q)
    {
        if ((*Q)->isComplete()) complete.push_back(*Q);
    }

    for (unsigned int i=0; i < complete.size(); ++i)
        deleteQuery(complete[i]);
}

void Meta::deleteQuery(MetaQuery* query)
{
    assert(m_activeQueries.count(query));
    m_activeQueries.erase(query);
    deleteLater(query);
    
    if (m_activeQueries.empty() && m_nextQuery == m_gameServers.size())
    {
        m_status = VALID;
        // we're all done, emit the signal
        AllQueriesDone.emit();
    }
}

void Meta::recv()
{
    if (_bytesToRecv == 0) {
        error() << "No bytes to receive when calling recv().";
        return;
    }
	
    m_stream.peek();
    std::streambuf * iobuf = m_stream.rdbuf();
    std::streamsize len = std::min(_bytesToRecv, iobuf->in_avail());
    if (len > 0) {
    	iobuf->sgetn(_dataPtr, len);
    	_bytesToRecv -= len;
    	_dataPtr += len;
    }
//	do {
//		int d = m_stream.get();
//		*(_dataPtr++) = static_cast<char>(d);
//		_bytesToRecv--;
//	} while (iobuf->in_avail() && _bytesToRecv);
	
    if (_bytesToRecv > 0) {
        error() << "Fragment data received by Meta::recv";
        return; // can't do anything till we get more data
    }
    
    if (_recvCmd) {
        uint32_t op;
        unpack_uint32(op, _data);
        recvCmd(op);
    } else {
        processCmd();
    }
		
    // try and read more
    if (_bytesToRecv && m_stream.rdbuf()->in_avail())
        recv();
}

void Meta::recvCmd(uint32_t op)
{    
    switch (op) {
    case HANDSHAKE:
        setupRecvData(1, HANDSHAKE);
        break;
    
    case PROTO_ERANGE:
        doFailure("Got list range error from Metaserver");
        break;
    
    case LIST_RESP:
        setupRecvData(2, LIST_RESP);
        break;
    
    default:
        doFailure("Unknown Meta server command");
        break;
    }
}

void Meta::processCmd()
{
    if (m_status != GETTING_LIST) {
        error() << "Command received when not expecting any. It will be ignored. The command was: " << _gotCmd;
        return;
    }

    switch (_gotCmd)
    {
    case HANDSHAKE: {	
        uint32_t stamp;
        unpack_uint32(stamp, _data);
                
        unsigned int dsz = 0;
        _dataPtr = pack_uint32(CLIENTSHAKE, _data, dsz);
        pack_uint32(stamp, _dataPtr, dsz);
        
        m_stream << std::string(_data, dsz) << std::flush;
        write();
        
        m_metaTimer.cancel();
        // send the initial list request
        listReq(0);
    } break;
	
    case LIST_RESP:	{
        //uint32_t _totalServers, _packed;
        uint32_t total_servers;
        _dataPtr = unpack_uint32(total_servers, _data);
        if (!m_gameServers.empty()) {
        	if (total_servers != _totalServers) {
                warning() << "Server total in new packet has changed. " << total_servers << ":" << _totalServers;
        	}
        } else {
        	_totalServers = total_servers;
        }
        unpack_uint32(_packed, _dataPtr);
        // FIXME This assumes that the data received so far is all the servers, which
        // in the case of fragmented server list it is not. Currently this code is generally
        // of the size of packet receieved. As there should only ever be one packet incoming
        // we should be able to make assumptions based on the amount of data in the buffer.
        // The buffer should also contain a complete packet if it contains any, so retrieving
        // data one byte at a time is less efficient than it might be.
        setupRecvData(_packed, LIST_RESP2);
		
        // If this is the first response, allocate the space
        if (m_gameServers.empty()) {

            assert(m_nextQuery == 0);
            m_gameServers.reserve(_totalServers);
        }
    } break;
	
    case LIST_RESP2: {
        _dataPtr = _data;
        while (_packed--)
        {
            uint32_t ip;
            _dataPtr = unpack_uint32(ip, _dataPtr);
            
            char buf[32];
            snprintf(buf, 32, "%u.%u.%u.%u", 
                    (ip & 0x000000FF),
                    (ip & 0x0000FF00) >> 8,
                    (ip & 0x00FF0000) >> 16,
                    (ip & 0xFF000000) >> 24
            );
            
            // FIXME  - decide whether a reverse name lookup is necessary here or not
            m_gameServers.push_back(ServerInfo(buf));
        }
			
        if (m_gameServers.size() < _totalServers)
        {
            // request some more
            listReq(m_gameServers.size());
        } else {
            // allow progress bars to setup, etc, etc
            CompletedServerList.emit(_totalServers);
            m_status = QUERYING;
            // all done, clean up
            disconnect();
        }
		
    } break;
		
    default:
        std::stringstream ss;
        ss << "Unknown Meta server command: " << _gotCmd;
        doFailure(ss.str());
        break;
    }
}

void Meta::listReq(int base)
{
    unsigned int dsz=0;
    char* _dataPtr = pack_uint32(LIST_REQ, _data, dsz);
    pack_uint32(base, _dataPtr, dsz);
    
    m_stream << std::string(_data, dsz) << std::flush;
    write();
    setupRecvCmd();
    
    startTimeout();
}

void Meta::setupRecvCmd()
{
	_recvCmd = true;
	_bytesToRecv = sizeof(uint32_t);
	_dataPtr = (char*) _data;
}

void Meta::setupRecvData(int words, uint32_t got)
{
	_recvCmd = false;
	_bytesToRecv = words * sizeof(uint32_t);
	_dataPtr = (char*) _data;
	_gotCmd = got;
}

/* pack the data into the specified buffer, update the buffer size, and return
the new buffer insert pointer */

char* pack_uint32(uint32_t data, char* buffer, unsigned int &size)
{
	uint32_t netorder;
 
	netorder = htonl(data);
	memcpy(buffer, &netorder, sizeof(uint32_t));
	size += sizeof(uint32_t);
 	return buffer+sizeof(uint32_t);
}

/* unpack one data from the buffer, and return the next extract pointer */

char* unpack_uint32(uint32_t &dest, char* buffer)
{
	uint32_t netorder;
 
	memcpy(&netorder, buffer, sizeof(uint32_t));
	dest = ntohl(netorder);
	return buffer+sizeof(uint32_t);
} 

#pragma mark -

void Meta::internalQuery(unsigned int index)
{
    assert(index < m_gameServers.size());
    
    ServerInfo& sv = m_gameServers[index];
    MetaQuery *q =  new MetaQuery(m_io_service, *this, sv.getHostname(), index);
    if (q->getStatus() != BaseConnection::CONNECTING &&
        q->getStatus() != BaseConnection::NEGOTIATE) {
        // indicates a failure occurred, so we'll kill it now and say no more
        delete q;
        sv.m_status = ServerInfo::INVALID;
    } else {
        m_activeQueries.insert(q);
        sv.m_status = ServerInfo::QUERYING;
    }
}

void Meta::objectArrived(const Root& obj)
{	
    Info info = smart_dynamic_cast<Info>(obj);
    if (!info.isValid())
    {
        error() << "Meta::objectArrived, failed to convert object to INFO op";
        return;
    }
	
// work out which query this is
    long refno = info->getRefno();
    QuerySet::iterator Q;
	
    for (Q = m_activeQueries.begin(); Q != m_activeQueries.end(); ++Q)
        if ((*Q)->getQueryNo() == refno) break;
	
    if (Q == m_activeQueries.end()) {
        error() << "Couldn't locate query for meta-query reply";
        return;
    }
    
    (*Q)->setComplete();
    
    RootEntity svr = smart_dynamic_cast<RootEntity>(info->getArgs().front());	
    if (!svr.isValid()) {
        error() << "Query INFO argument object is broken";
        return;
    }
    
    if ((*Q)->getServerIndex() >= m_gameServers.size()) {
        error() << "Got server info with out of bounds index.";
    } else {
        ServerInfo& sv = m_gameServers[(*Q)->getServerIndex()];

        sv.processServer(svr);
        sv.setPing((*Q)->getElapsed());

        // emit the signal
        ReceivedServerInfo.emit(sv);
    }
}

void Meta::doFailure(const std::string &msg)
{
    Failure.emit(msg);
    cancel();
}

void Meta::dispatch()
{

}

void Meta::metaTimeout()
{
    // cancel calls disconnect, which will kill upfront without this
    m_metaTimer.cancel();
    
    // might want different behaviour in the future, I suppose
    doFailure("Connection to the meta-server timed out");
}

void Meta::queryFailure(MetaQuery *q, const std::string &msg)
{
    // we do NOT emit a failure signal here (becuase that would probably cause the 
    // host app to pop up a dialog or something) since query failures are likely to
    // be very frequent.
    m_gameServers[q->getServerIndex()].m_status = ServerInfo::INVALID;
    q->setComplete();
}

void Meta::query()
{
    if (m_activeQueries.size() >= m_maxActiveQueries)
    {
        return;
    }

    if (m_nextQuery >= m_gameServers.size())
    {
        assert(m_nextQuery == m_gameServers.size());
        return;
    }

    internalQuery(m_nextQuery++);
}

void Meta::queryTimeout(MetaQuery *q)
{
    m_gameServers[q->getServerIndex()].m_status = ServerInfo::TIMEOUT;
    deleteQuery(q);
}

} // of Eris namespace

