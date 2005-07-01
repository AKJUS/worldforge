#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <Eris/Account.h>
#include <Eris/Connection.h>
#include <Eris/LogStream.h>
#include <Eris/Exceptions.h>
#include <Eris/Avatar.h>
#include <Eris/Router.h>
#include <Eris/Response.h>
#include <Eris/DeleteLater.h>

#include <Atlas/Objects/Entity.h>
#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Anonymous.h>

#include <sigc++/object_slot.h>

#include <algorithm>
#include <cassert>

using Atlas::Objects::Root;
using namespace Atlas::Objects::Operation;
using Atlas::Objects::Entity::GameEntity;
using Atlas::Objects::Entity::Anonymous;
typedef Atlas::Objects::Entity::Account AtlasAccount;
using Atlas::Objects::smart_dynamic_cast;

namespace Eris {

class AccountRouter : public Router
{
public:
    AccountRouter(Account* pl) :
        m_account(pl)
    {
        m_account->getConnection()->setDefaultRouter(this);
    }

    virtual ~AccountRouter()
    {
        m_account->getConnection()->clearDefaultRouter();
    }

    virtual RouterResult handleOperation(const RootOperation& op)
    {
        // logout
        if (op->instanceOf(LOGOUT_NO)) {
            debug() << "Account reciev forced logout from server";
            m_account->internalLogout(false);
            return HANDLED;
        }
  
        return IGNORED;
    }

private:
    Account* m_account;
};

#pragma mark -

Account::Account(Connection *con) :
    m_con(con),
    m_status(DISCONNECTED),
    m_doingCharacterRefresh(false)
{
    if (!m_con)
        throw InvalidOperation("invalid Connection passed to Account");

    m_router = new AccountRouter(this);

    m_con->Connected.connect(SigC::slot(*this, &Account::netConnected));
    m_con->Failure.connect(SigC::slot(*this, &Account::netFailure));
}	

Account::~Account()
{
    delete m_router;
}

Result Account::login(const std::string &uname, const std::string &password)
{
    if (!m_con->isConnected()) {
        error() << "called login on unconnected Connection";
        return NOT_CONNECTED;
    }
    
    if (m_status != DISCONNECTED) {
        error() << "called login, but state is not currently disconnected";
        return ALREADY_LOGGED_IN;
    }
        	
    return internalLogin(uname, password);
}

Result Account::createAccount(const std::string &uname, 
	const std::string &fullName,
	const std::string &pwd)
{
    if (!m_con->isConnected()) return NOT_CONNECTED;
    if (m_status != DISCONNECTED) return ALREADY_LOGGED_IN;

    m_status = LOGGING_IN;

// okay, build and send the create(account) op
    AtlasAccount account;
    account->setPassword(pwd);
    account->setName(fullName);
    account->setUsername(uname);
    
    Create c;
    c->setSerialno(getNewSerialno());
    c->setArgs1(account);
    
    m_con->getResponder()->await(c->getSerialno(), this, &Account::loginResponse);
    m_con->send(c);
	
// store for re-logins
    m_username = uname;
    m_pass = pwd;
    
    m_timeout.reset(new Timeout("login", this, 5000));
    m_timeout->Expired.connect(SigC::slot(*this, &Account::handleLoginTimeout));
    
    return NO_ERR;
}

Result Account::logout()
{
    if (!m_con->isConnected()) {
        error() << "called logout on bad connection ignoring";
        return NOT_CONNECTED;
    }
    
    if (m_status != LOGGED_IN) {
        error() << "called logout on non-logged-in Account";
        return NOT_LOGGED_IN;
    }
    
    m_status = LOGGING_OUT;
    
    Logout l;
    l->setId(m_accountId);
    l->setSerialno(getNewSerialno());
    l->setFrom(m_accountId);
    
    m_con->getResponder()->await(l->getSerialno(), this, &Account::logoutResponse);
    m_con->send(l);
	
    m_timeout.reset(new Timeout("logout", this, 5000));
    m_timeout->Expired.connect(SigC::slot(*this, &Account::handleLogoutTimeout));
    
    return NO_ERR;
}

const std::vector< std::string > & Account::getCharacterTypes(void) const
{
	return m_characterTypes;
}

const CharacterMap& Account::getCharacters()
{
    if (m_status != LOGGED_IN)
        error() << "Not logged into an account : getCharacter returning empty dictionary";
    
    if (m_doingCharacterRefresh)
        warning() << "client retrieving partial / incomplete character dictionary";
    
    return _characters;
}

Result Account::refreshCharacterInfo()
{
    if (!m_con->isConnected()) return NOT_CONNECTED;
    if (m_status != LOGGED_IN) return NOT_LOGGED_IN;
    
    // silently ignore overlapping refreshes
    if (m_doingCharacterRefresh) return NO_ERR;
        
    _characters.clear();

    if (m_characterIds.empty())
    {
        GotAllCharacters.emit(); // we must emit the done signal
        return NO_ERR;
    }
    
// okay, now we know we have at least one character to lookup, set the flag
    m_doingCharacterRefresh = true;
    
    Look lk;
    Anonymous obj;
    lk->setFrom(m_accountId);
        
    for (StringSet::iterator I=m_characterIds.begin(); I!=m_characterIds.end(); ++I)
    {
        obj->setId(*I);
        lk->setArgs1(obj);
        lk->setSerialno(getNewSerialno());
        m_con->getResponder()->await(lk->getSerialno(), this, &Account::sightCharacter);
        m_con->send(lk);
    }
    
    return NO_ERR;
}

Result Account::createCharacter(const Atlas::Objects::Entity::GameEntity &ent)
{
    if (!m_con->isConnected()) return NOT_CONNECTED;
    if (m_status != LOGGED_IN) {
        if ((m_status == CREATING_CHAR) || (m_status == TAKING_CHAR)) {
            error() << "duplicate char creation / take";
            return DUPLICATE_CHAR_ACTIVE;
        } else {
            error() << "called createCharacter on unconnected Account, ignoring";
            return NOT_LOGGED_IN;
        }
    }    

    Create c;    
    c->setArgs1(ent);
    c->setFrom(m_accountId);
    c->setSerialno(getNewSerialno());
    m_con->send(c);
    
    m_con->getResponder()->await(c->getSerialno(), this, &Account::avatarResponse);
    m_status = CREATING_CHAR;
    return NO_ERR;
}

/*
void Account::createCharacter()
{
	if (!_lobby || _lobby->getAccountID().empty())
		throw InvalidOperation("no account exists!");

	if (!_con->isConnected())
		throw InvalidOperation("Not connected to server");

	throw InvalidOperation("No UserInterface handler defined");

	// FIXME look up the dialog, create the instance,
	// hook in a slot to feed the serialno of any Create op
	// the dialog passes back to createCharacterHandler()
}

void Account::createCharacterHandler(long serialno)
{
    if (serialno)
        NewCharacter((new World(this, _con))->createAvatar(serialno));
}
*/

Result Account::takeCharacter(const std::string &id)
{
    if (m_characterIds.count(id) == 0) {
        error() << "Character '" << id << "' not owned by Account " << m_username;
        return BAD_CHARACTER_ID;
    }
	
    if (!m_con->isConnected()) return NOT_CONNECTED;
    if (m_status != LOGGED_IN) {
        if ((m_status == CREATING_CHAR) || (m_status == TAKING_CHAR)) {
            error() << "duplicate char creation / take";
            return DUPLICATE_CHAR_ACTIVE;
        } else {
            error() << "called createCharacter on unconnected Account, ignoring";
            return NOT_LOGGED_IN;
        }
    } 
        
    Root what;
    what->setId(id);
    
    Look l;
    l->setFrom(id);  // should this be m_accountId?
    l->setArgs1(what);
    l->setSerialno(getNewSerialno());
    m_con->send(l);

    m_con->getResponder()->await(l->getSerialno(), this, &Account::avatarResponse);
    m_status = TAKING_CHAR;
    return NO_ERR;
}

bool Account::isLoggedIn() const
{
    return ((m_status == LOGGED_IN) || 
        (m_status == TAKING_CHAR) || (m_status == CREATING_CHAR));
}
    
#pragma mark -

Result Account::internalLogin(const std::string &uname, const std::string &pwd)
{
    assert(m_status == DISCONNECTED);
        
    m_status = LOGGING_IN;
    m_username = uname; // store for posterity

    AtlasAccount account;
    account->setPassword(pwd);
    account->setUsername(uname);

    Login l;
    l->setArgs1(account);
    l->setSerialno(getNewSerialno());
    m_con->getResponder()->await(l->getSerialno(), this, &Account::loginResponse);
    m_con->send(l);
    
    m_timeout.reset(new Timeout("login", this, 5000));
    m_timeout->Expired.connect(SigC::slot(*this, &Account::handleLoginTimeout));
    
    return NO_ERR;
}

void Account::logoutResponse(const RootOperation& op)
{
    if (!op->instanceOf(INFO_NO))
        warning() << "received a logout response that is not an INFO";
        
    internalLogout(true);
}

void Account::internalLogout(bool clean)
{
    if (clean) {
        if (m_status != LOGGING_OUT)
            error() << "got clean logout, but not logging out already";
    } else {
        if ((m_status != LOGGED_IN) && (m_status != TAKING_CHAR) && (m_status != CREATING_CHAR))
            error() << "got forced logout, but not currently logged in";
    }
    
    m_status = DISCONNECTED;
    m_timeout.reset();
    
    if (m_con->getStatus() == BaseConnection::DISCONNECTING) {
        m_con->unlock();
    } else {
        // note this will cause netDisconnect to run, which is why we set
        // status to DISCONNECT already
        m_con->disconnect(); 
        LogoutComplete.emit(clean);
    }
}

void Account::loginResponse(const RootOperation& op)
{
    if (op->instanceOf(ERROR_NO)) {
        loginError(smart_dynamic_cast<Error>(op));
    } else if (op->instanceOf(INFO_NO)) {
        const std::vector<Root>& args = op->getArgs();
        loginComplete(smart_dynamic_cast<AtlasAccount>(args.front()));
    } else
        warning() << "received malformed login response";
}

void Account::loginComplete(const AtlasAccount &p)
{
    if (m_status != LOGGING_IN)
        error() << "got loginComplete, but not currently logging in!";
        
    if (p->getUsername()  != m_username)
        error() << "missing or incorrect username on login INFO";
        
    m_status = LOGGED_IN;
    m_accountId = p->getId();
    
	if(p->hasAttr("character_types") == true)
	{
		Atlas::Message::Element CharacterTypes(p->getAttr("character_types"));
		
		if(CharacterTypes.isList() == true)
		{
			const Atlas::Message::ListType & CharacterTypesList(CharacterTypes.asList());
			Atlas::Message::ListType::const_iterator iCharacterType(CharacterTypesList.begin());
			Atlas::Message::ListType::const_iterator iEnd(CharacterTypesList.end());
			
			m_characterTypes.reserve(CharacterTypesList.size());
			while(iCharacterType != iEnd)
			{
				if(iCharacterType->isString() == true)
				{
					m_characterTypes.push_back(iCharacterType->asString());
				}
				else
				{
					error() << "An element of the \"character_types\" list is not a String.";
				}
				++iCharacterType;
			}
		}
		else
		{
			error() << "Account has attribute \"character_types\" which is not of type List.";
		}
	}
    m_characterIds = StringSet(p->getCharacters().begin(), p->getCharacters().end());
    // notify an people watching us 
    LoginSuccess.emit();
      
    m_con->Disconnecting.connect(SigC::slot(*this, &Account::netDisconnecting));
    m_timeout.reset();
}

void Account::loginError(const Error& err)
{
    assert(err.isValid());
    if (m_status != LOGGING_IN)
        error() << "got loginError while not logging in";
        
    const std::vector<Root>& args = err->getArgs();
    std::string msg = args[0]->getAttr("message").asString();
    
    // update state before emitting signal
    m_status = DISCONNECTED;
    m_timeout.reset();
    
    warning() << "got login error: " << msg;
    LoginFailure.emit(msg);
}

void Account::handleLoginTimeout()
{
    warning() << "login / account creation timed out waiting for response";
    
    m_status = DISCONNECTED;
    deleteLater(m_timeout.release());
    
    LoginFailure.emit("timed out waiting for server response");
}

void Account::avatarResponse(const RootOperation& op)
{
    if (op->instanceOf(ERROR_NO)) {
        const std::vector<Root>& args = op->getArgs();
        std::string msg = args[0]->getAttr("message").asString();
        
        // creating or taking a character failed for some reason
        AvatarFailure(msg);
        m_status = Account::LOGGED_IN;
    } else if (op->instanceOf(INFO_NO)) {
        const std::vector<Root>& args = op->getArgs();
   
        GameEntity ent = smart_dynamic_cast<GameEntity>(args.front());
        if (!ent.isValid()) {
            warning() << "malformed character create/take response";
            return;
        }

        Avatar* av = new Avatar(this, ent->getId());
        AvatarSuccess.emit(av);
        m_status = Account::LOGGED_IN;
    } else 
        warning() << "received malformed login response";
}

void Account::sightCharacter(const RootOperation& op)
{
    if (!m_doingCharacterRefresh) {
        error() << "got sight of character outside a refresh, ignoring";
        return;
    }
    
    const std::vector<Root>& args = op->getArgs();
    assert(!args.empty());
    GameEntity ge = smart_dynamic_cast<GameEntity>(args.front());
    assert(ge.isValid());

    CharacterMap::iterator C = _characters.find(ge->getId());
    if (C != _characters.end()) {
        error() << "duplicate sight of character " << ge->getId();
        return;
    }
    
    // okay, we can now add it to our map
    _characters.insert(C, CharacterMap::value_type(ge->getId(), ge));
    GotCharacterInfo.emit(ge);
    
    // check if we're done
    if (_characters.size() == m_characterIds.size()) {
        m_doingCharacterRefresh = false;
        GotAllCharacters.emit();
    }
}

/* this will only ever get encountered after the connection is initally up;
thus we use it to trigger a reconnection. Note that some actions by
Lobby and World are also required to get everything back into the correct
state */

void Account::netConnected()
{
    // re-connection
    if (!m_username.empty() && !m_pass.empty() && (m_status == DISCONNECTED)) {
        debug() << "Account " << m_username << " got netConnected, doing reconnect";
        internalLogin(m_username, m_pass);
    }
}

bool Account::netDisconnecting()
{
    if (m_status == LOGGED_IN) {
        m_con->lock();
        logout();
        return false;
    } else
        return true;
}

void Account::netFailure(const std::string& /*msg*/)
{
  
}

void Account::handleLogoutTimeout()
{
    error() << "LOGOUT timed out waiting for response";
    
    m_status = DISCONNECTED;
    deleteLater(m_timeout.release());
    
    LogoutComplete.emit(false);
}

} // of namespace Eris
