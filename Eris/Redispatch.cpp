#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <Eris/Redispatch.h>
#include <Eris/Connection.h>
#include <Eris/LogStream.h>

namespace Eris
{

void Redispatch::post()
{
    m_con->postForDispatch(m_obj);
    m_con->cleanupRedispatch(this);
}

void Redispatch::postModified(const Atlas::Objects::Root& obj)
{
    m_con->postForDispatch(obj);
    m_con->cleanupRedispatch(this);
}

void Redispatch::fail()
{
    warning() << "redispatch failed for " << m_obj;
    m_con->cleanupRedispatch(this);
}

} // of namespace
