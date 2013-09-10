/**
 Worldforge Next Generation MetaServer

 Copyright (C) 2011 Sean Ryan <sryan@evercrack.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "DataObject.hpp"

DataObject::DataObject()
{
	m_serverData.clear();
	m_clientData.clear();
	m_clientFilterData.clear();
	m_serverDataList.clear();
	m_handshakeQueue.clear();

	srand( (unsigned)time(0));

}


DataObject::~DataObject()
{

}

bool
DataObject::addServerAttribute(const std::string& sessionid, const std::string& name, const std::string& value )
{
	/**
	 * Can not have empty values for required keys, value *can* be an empty string
	 */
	if ( sessionid.size() > 0 && sessionid != "" && name.size() > 0 && name != "" )
	{
		m_serverData[sessionid][name] = value;
		return true;
	}
	return false;
}

void
DataObject::removeServerAttribute(const std::string& sessionid, const std::string& name )
{
	/**
	 * 	Some attributes are protected and must not be removed ... as they are
	 * 	considered essential to a "session"
	 * 	ip
	 * 	port
	 * 	expiry
	 */
	if ( m_serverData.find(sessionid) != m_serverData.end() )
	{
		if ( name != "ip" && name != "expiry" && name != "port" )
		{
			m_serverData[sessionid].erase(name);
		}

	}
}

std::string
DataObject::getServerAttribute( const std::string& sessionid, const std::string& key )
{
	if ( m_serverData.find(sessionid) != m_serverData.end() )
	{
		if ( m_serverData[sessionid].find(key) != m_serverData[sessionid].end() )
		{
			return m_serverData[sessionid][key];
		}
	}
	return "";
}

bool
DataObject::addClientAttribute( const std::string& sessionid, const std::string& name, const std::string& value )
{
	/**
	 * Can not have empty values for required keys, value *can* be an empty string
	 */
	if ( sessionid.size() > 0 && sessionid != "" && name.size() > 0 && name != "" )
	{
		m_clientData[sessionid][name] = value;
		return true;
	}
	return false;
}

void
DataObject::removeClientAttribute( const std::string& sessionid, const std::string& name )
{
	/**
	 * 	Some attributes are protected and must not be removed ... as they are
	 * 	considered essential to a "session"
	 * 	ip
	 * 	port
	 * 	expiry
	 */
	if ( m_clientData.find(sessionid) != m_clientData.end() )
	{
		if ( name != "ip" && name != "expiry" && name != "port" )
		{
			m_clientData[sessionid].erase(name);
		}

	}
}

std::string
DataObject::getClientAttribute( const std::string& sessionid, const std::string& key )
{
	if ( m_clientData.find(sessionid) != m_clientData.end() )
	{
		if ( m_clientData[sessionid].find(key) != m_clientData[sessionid].end() )
		{
			return m_clientData[sessionid][key];
		}
	}
	return "";
}

bool
DataObject::addClientFilter( const std::string& sessionid, const std::string& name, const std::string& value )
{
	/**
	 * Can not have empty values for required keys, value *can* be an empty string
	 */
	if ( sessionid.size() > 0 && sessionid != "" && name.size() > 0 && name != "" )
	{
		if ( keyExists<std::string>(m_clientData, sessionid) )
		{
			/**
			 * This serves as both a create and update.  In order to prevent DOS
			 * style attack on MS, make establishing a session is a requirement
			 * to add a filter.  Also, some cleanup processes will expire out
			 * filter data that does not have a corresponding client session
			 */
			m_clientFilterData[sessionid][name] = value;
			return true;
		}
	}
	return false;
}

std::map< std::string, std::string >
DataObject::getClientFilter( const std::string& sessionid )
{
	std::map<std::string,std::string> empty;
	empty.clear();

	if ( keyExists<std::string>(m_clientFilterData, sessionid ) &&
		 keyExists<std::string>(m_clientData, sessionid ) )
	{
		return m_clientFilterData[sessionid];
	}
	return empty;
}

std::string
DataObject::getClientFilter( const std::string& sessionid, const std::string& key )
{


	if ( m_clientFilterData.find(sessionid) != m_clientFilterData.end() &&
		 m_clientFilterData[sessionid].find(key) != m_clientFilterData[sessionid].end() &&
		 m_clientData.find(sessionid) != m_clientData.end() )
	{
		return m_clientFilterData[sessionid][key];
	}

	return "";
}

void
DataObject::removeClientFilter( const std::string& sessionid, const std::string& name )
{
	if ( m_clientFilterData.find(sessionid) != m_clientFilterData.end() )
	{
		m_clientFilterData[sessionid].erase(name);
	}
}

bool
DataObject::addServerSession( const std::string& sessionid )
{

	bool ret = false;
	bool found = false;
	/*
	 *  If the server session does not exist, create it, and add+uniq the listresp
	 */
	if ( ! keyExists<std::string>(m_serverData, sessionid) )
	{
		addServerAttribute(sessionid,"ip",sessionid);

		/*
		 * Check if the key already exists, so we know about it
		 */
		for ( auto &f: m_serverDataList )
		{
			if ( f == sessionid )
			{
				found = true;
				break;
			}
		}

		/*
		 *  If we haven't found our session, we need to add it
		 *  skip if we already have it.  This should in theory
		 *  ensure unique list (unless threading)
		 */
		if ( !found )
		{
			m_serverDataList.push_back(sessionid);
		}

		ret = true;
	}

	/*
	 *  If a new structure, this will create the expiry, if existing it will just
	 *  refresh the timeout
	 */
	addServerAttribute(sessionid,"expiry", getNowStr() );

	return ret;

}

void
DataObject::removeServerSession( const std::string& sessionid )
{

	for ( auto& k: m_serverDataList )
	{
		if ( k == sessionid )
		{
			/*
			 * Erase from main data
			 */
			m_serverData.erase(k);

			/*
			 * Erase from the ordered list
			 */
			m_serverDataList.erase(
					std::remove(m_serverDataList.begin(),m_serverDataList.end(),k),
					m_serverDataList.end()
					);
		}
	}
}

bool
DataObject::serverSessionExists( const std::string& sessionid )
{
	return keyExists<std::string>( m_serverData, sessionid );
}

std::map<std::string,std::string>
DataObject::getServerSession( const std::string& sessionid )
{
	if ( keyExists<std::string>(m_serverData, sessionid ))
		return m_serverData[sessionid];

	std::map<std::string, std::string> empty;
	empty.clear();
	return empty;
}

bool DataObject::addClientSession( const std::string& sessionid )
{
	bool ret = false;
	/*
	 *  If the client session does not exist, create it, and add+uniq the listresp
	 */
	if ( ! keyExists<std::string>( m_clientData, sessionid ) )
	{
		addClientAttribute(sessionid,"ip",sessionid);
		ret = true;
	}

	/*
	 *  If a new structure, this will create the expiry, if existing it will just
	 *  refresh the timeout
	 */
	addClientAttribute(sessionid,"expiry", getNowStr() );

	return ret;
}


void
DataObject::removeClientSession( const std::string& sessionid )
{
	m_clientFilterData.erase(sessionid);
	if(  m_clientData.erase(sessionid) == 1 )
	{
		// logging if any
	}
}

bool
DataObject::clientSessionExists( const std::string& sessionid )
{
	return keyExists<std::string>( m_clientData, sessionid );
}

std::list<std::string>
DataObject::getClientSessionList()
{
	std::list<std::string> cslist;
	cslist.clear();

	std::map<std::string, std::map<std::string,std::string> >::iterator it;

	for ( it = m_clientData.begin(); it != m_clientData.end(); it++ )
	{
		cslist.push_back( it->first );
	}

	return cslist;
}

std::list<std::string>
DataObject::getServerSessionList(uint32_t start_idx, uint32_t max_items )
{
	std::list<std::string> ss_slice;
	std::vector<std::string>::iterator ss_itr;

	ss_slice.clear();

	/*
	 *  Just gate it so that it won't breach the iterator bounds
	 */
	 if ( start_idx > m_serverDataList.size() )
	 {
		 start_idx = m_serverDataList.size();
	 }


	 /*
	  * It's a zero index list, ergo we want to move to n-1
	  */
	 for ( ss_itr=m_serverDataList.begin()+start_idx; ss_itr != m_serverDataList.end() ; ss_itr++ )
	 {

		 /*
		  * Early bail out
		  */
		 if ( ss_slice.size() >= max_items )
			 break;

	     ss_slice.push_back(*ss_itr);
	 }

	 ss_slice.unique();
	 return ss_slice;
}

/*
 * TODO: this is a copy of expireHandshakes ... refactor same as keyExists<T>(blah)
 */
std::vector<std::string>
DataObject::expireServerSessions( unsigned int expiry )
{
	std::vector<std::string> expiredSS;
	expiredSS.clear();

	boost::posix_time::ptime now = this->getNow();
	boost::posix_time::ptime etime;

	/*
	 * Option 1: etime = 0.  The end time becomes now + 0, thus guaranteed expiry.
	 * Option 2: etime = expiry(sec). End time becomes now + expiry.
	 *           a) expiry>0 : all entries that are older that m_handshakeExpirySeconds are removed
	 *           b) expiry<=0 : immediate expiry by making the etime less than now.
	 */
    std::map<std::string,std::map<std::string,std::string> >::iterator itr;
    for( itr = m_serverData.begin(); itr != m_serverData.end(); )
    {
    	std::string key = itr->first;

    	etime =  boost::posix_time::from_iso_string(itr->second["expiry"]) +
    			 boost::posix_time::seconds(expiry);

    	/**
    	 * We need to make a copy of the iterator if we modify the
    	 * underlying container because the iterator becomes invalid
    	 */
    	if ( now > etime )
    	{
    		std::map<std::string,std::map<std::string,std::string> >::iterator itr_copy = itr;
    		++itr_copy;
    		removeServerSession(key);
    		itr = itr_copy;
    		expiredSS.push_back(key);
    	}
    	else
    	{
    		/**
    		 * We are not modifying, just increment normally.
    		 */
    		++itr;
    	}

    }
    return expiredSS;


}

std::list<std::string>
DataObject::searchServerSessionByAttribute(std::string attr_name,std::string attr_val)
{
	std::list<std::string> matched;

	matched.clear();
    std::vector<std::string>::iterator itr;
    std::string vattr;

    /*
     * Loop through all sessions
     */
    for( auto& vv : m_serverDataList )
    {

    	/*
    	 * Returns an empty string
    	 */
    	std::string vattr = getServerAttribute(vv,attr_name);

    	/*
    	 * If the session has the attribute, and the value matches, push it on list
    	 */
    	if( boost::iequals(vattr,attr_val))
    		matched.push_back(vv);

    }
    return matched;
}

std::map<std::string,std::string>
DataObject::getClientSession( const std::string& sessionid )
{
	if ( keyExists<std::string>( m_clientData, sessionid ) )
		return m_clientData[sessionid];

	// @TODO: there has to be a way to do this without polluting the stack
	std::map<std::string, std::string> empty;
	empty.clear();
	return empty;
}

std::vector<std::string>
DataObject::expireClientSessions( unsigned int expiry )
{
	std::vector<std::string> expiredCS;
	expiredCS.clear();

	boost::posix_time::ptime now = this->getNow();
	boost::posix_time::ptime etime;

	/*
	 * Option 1: etime = 0.  The end time becomes now + 0, thus guaranteed expiry.
	 * Option 2: etime = expiry(sec). End time becomes now + expiry.
	 *           a) expiry>0 : all entries that are older that m_handshakeExpirySeconds are removed
	 *           b) expiry<=0 : immediate expiry by making the etime less than now.
	 */
    std::map<std::string,std::map<std::string,std::string> >::iterator itr;
    for( itr = m_clientData.begin(); itr != m_clientData.end(); )
    {
    	std::string key = itr->first;

    	etime =  boost::posix_time::from_iso_string(itr->second["expiry"]) +
    			 boost::posix_time::seconds(expiry);

    	/**
    	 * We need to make a copy of the iterator if we modify the
    	 * underlying container because the iterator becomes invalid
    	 */
    	if ( now > etime )
    	{
    		std::map<std::string,std::map<std::string,std::string> >::iterator itr_copy = itr;
    		++itr_copy;
    		removeClientSession(key);
    		itr = itr_copy;
    		expiredCS.push_back(key);
    	}
    	else
    	{
    		/**
    		 * We are not modifying, just increment normally.
    		 */
    		++itr;
    	}

    }
    return expiredCS;

}


uint32_t
DataObject::addHandshake(unsigned int def_hs )
{

	unsigned int handshake = def_hs;
	unsigned int ret = 0;

	/*
	 *  If we pass a value in ( other than 0 ), we set the handshake to that value
	 *  otherwise we generate a random one
	 */
	if ( handshake == 0 )
	{
		handshake = rand();
	}

	// set expiry in data structure, if it exists already it is updated
	m_handshakeQueue[handshake]["expiry"] = boost::posix_time::to_iso_string( getNow() );

	// if we find said element again, return handshake, otherwise 0
	if ( m_handshakeQueue[handshake].find("expiry") != m_handshakeQueue[handshake].end() )
	{
		ret = handshake;
	}

	return ret;

}

uint32_t
DataObject::removeHandshake( unsigned int hs )
{
	/*
	 * There is technically nothing wrong with deleting an element that doesn't exist.
	 * Thus the return code is semi-superfluous
	 */
	if ( m_handshakeQueue.erase(hs) == 1 )
	{
		return hs;
	}
	return 0;
}

bool
DataObject::handshakeExists( unsigned int hs )
{
	return keyExists<unsigned int>( m_handshakeQueue, hs );
}
std::vector<unsigned int>
DataObject::expireHandshakes( unsigned int expiry )
{
	/**
	 * Go over handshake queue ... expire any that are older than m_handshakeExpirySeconds
	 */
	std::vector<unsigned int> removedHS;
	removedHS.empty();

	boost::posix_time::ptime now = this->getNow();
	boost::posix_time::ptime etime;

	/*
	 * Option 1: etime = 0.  The end time becomes now + 0, thus guaranteed expiry.m_handshakeExpirySeconds
	 * Option 2: etime = expiry(sec). End time becomes now + expiry.
	 *           a) expiry>0 : all entries that are older that m_handshakeExpirySeconds are removed
	 *           b) expiry<=0 : immediate expiry by making the etime less than now.
	 */
    std::map<unsigned int,std::map<std::string,std::string> >::iterator itr;
    for( itr = m_handshakeQueue.begin(); itr != m_handshakeQueue.end(); )
    {
    	unsigned int key = itr->first;

    	etime =  boost::posix_time::from_iso_string(itr->second["expiry"]) +
    			 boost::posix_time::seconds(expiry);

    	/**
    	 * We need to make a copy of the iterator if we modify the
    	 * underlying container because the iterator becomes invalid
    	 */
    	if ( now > etime )
    	{
    		std::map<unsigned int,std::map<std::string,std::string> >::iterator itr_copy = itr;
    		++itr_copy;
    		removeHandshake(key);
    		itr = itr_copy;
    		removedHS.push_back(key);
    	}
    	else
    	{
    		/**
    		 * We are not modifying, just increment normally.
    		 */
    		++itr;
    	}

    }
    return removedHS;
}

uint32_t
DataObject::getHandshakeCount()
{
	return m_handshakeQueue.size();
}

uint32_t
DataObject::getServerSessionCount()
{
	return m_serverData.size();
}

uint32_t
DataObject::getClientSessionCount()
{
	return m_clientData.size();
}

boost::posix_time::ptime
DataObject::getNow()
{
	//boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	return boost::posix_time::microsec_clock::local_time();
}

std::string
DataObject::getNowStr()
{
	return boost::posix_time::to_iso_string( getNow() );
}
