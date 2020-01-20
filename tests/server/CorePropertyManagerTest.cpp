// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2013 Alistair Riddoch
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


#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "../TestBase.h"

#include "server/CorePropertyManager.h"

#include "rules/Domain.h"
#include "rules/simulation/Entity.h"
#include "rules/simulation/ExternalMind.h"

#include "common/CommSocket.h"
#include "common/Inheritance.h"
#include "common/PropertyFactory.h"
#include "common/SystemTime.h"

#include "../TestWorld.h"

#include "../stubs/rules/simulation/stubPropelProperty.h"

#include <Atlas/Objects/SmartPtr.h>
#include <Atlas/Objects/Anonymous.h>
#include <Atlas/Objects/Operation.h>

#include <cassert>

using Atlas::Message::Element;
using Atlas::Message::ListType;
using Atlas::Message::MapType;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::RootEntity;

Atlas::Objects::Factories factories;

class MinimalProperty : public PropertyBase {
  public:
    MinimalProperty() { }
    virtual int get(Atlas::Message::Element & val) const { return 0; }
    virtual void set(const Atlas::Message::Element & val) { }
    virtual MinimalProperty * copy() const { return 0; }
};

class CorePropertyManagertest : public Cyphesis::TestBase
{
    CorePropertyManager * m_propertyManager;

  public:
    CorePropertyManagertest();

    void setup();
    void teardown();

    void test_addProperty_int();
    void test_addProperty_float();
    void test_addProperty_string();
    void test_addProperty_list();
    void test_addProperty_map();
    void test_addProperty_none();
    void test_addProperty_named();
    void test_installFactory();

        Inheritance* m_inheritance;
};

CorePropertyManagertest::CorePropertyManagertest()
{
    ADD_TEST(CorePropertyManagertest::test_addProperty_int);
    ADD_TEST(CorePropertyManagertest::test_addProperty_float);
    ADD_TEST(CorePropertyManagertest::test_addProperty_string);
    ADD_TEST(CorePropertyManagertest::test_addProperty_list);
    ADD_TEST(CorePropertyManagertest::test_addProperty_map);
    ADD_TEST(CorePropertyManagertest::test_addProperty_none);
    ADD_TEST(CorePropertyManagertest::test_addProperty_named);
    ADD_TEST(CorePropertyManagertest::test_installFactory);
}

void CorePropertyManagertest::setup()
{
    m_inheritance = new Inheritance(factories);
    m_propertyManager = new CorePropertyManager;
    m_propertyManager->m_propertyFactories.emplace(
        std::string("named_type"), std::make_unique<PropertyFactory<MinimalProperty>>()
    );
   
}

void CorePropertyManagertest::teardown()
{
    delete m_inheritance;
    delete m_propertyManager;
}

void CorePropertyManagertest::test_addProperty_int()
{
    auto p = m_propertyManager->addProperty("non_existant_type",
                                              Element::TYPE_INT);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<SoftProperty*>(p.get()));
}

void CorePropertyManagertest::test_addProperty_float()
{
    auto p = m_propertyManager->addProperty("non_existant_type",
                                              Element::TYPE_FLOAT);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<SoftProperty*>(p.get()));
}

void CorePropertyManagertest::test_addProperty_string()
{
    auto p = m_propertyManager->addProperty("non_existant_type",
                                              Element::TYPE_STRING);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<SoftProperty*>(p.get()));
}

void CorePropertyManagertest::test_addProperty_list()
{
    auto p = m_propertyManager->addProperty("non_existant_type",
                                              Element::TYPE_LIST);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<SoftProperty *>(p.get()));
}

void CorePropertyManagertest::test_addProperty_map()
{
    auto p = m_propertyManager->addProperty("non_existant_type",
                                              Element::TYPE_MAP);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<SoftProperty *>(p.get()));
}

void CorePropertyManagertest::test_addProperty_none()
{
    auto p = m_propertyManager->addProperty("non_existant_type",
                                              Element::TYPE_NONE);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<SoftProperty *>(p.get()));
}

void CorePropertyManagertest::test_addProperty_named()
{
    auto p = m_propertyManager->addProperty("named_type",
                                              Element::TYPE_NONE);
    ASSERT_TRUE(p);
    ASSERT_NOT_NULL(dynamic_cast<MinimalProperty *>(p.get()));
}

void CorePropertyManagertest::test_installFactory()
{
    int ret = m_propertyManager->installFactory(
          "new_named_type",
          Root(),
          std::make_unique<PropertyFactory<MinimalProperty>>()
    );

    ASSERT_EQUAL(ret, 0);
}

int main()
{
    CorePropertyManagertest t;

    return t.run();
}





#include "common/const.h"
#include "common/globals.h"
#include "common/log.h"
#include "common/Monitors.h"
#include "common/PropertyFactory.h"
#include "common/system.h"
#include "common/TypeNode.h"
#include "common/Variable.h"


// stubs


#include "server/ArithmeticBuilder.h"
#include "server/EntityFactory.h"
#include "server/Juncture.h"
#include "server/Persistence.h"
#include "server/Player.h"
#include "server/Ruleset.h"
#include "server/ServerRouting.h"
#include "server/TeleportProperty.h"

#include "rules/simulation/AreaProperty.h"
#include "rules/AtlasProperties.h"
#include "rules/BBoxProperty.h"
#include "rules/simulation/CalendarProperty.h"
#include "rules/simulation/EntityProperty.h"
#include "rules/simulation/InternalProperties.h"
#include "rules/simulation/LineProperty.h"
#include "rules/simulation/MindProperty.h"
#include "rules/SolidProperty.h"
#include "rules/simulation/SpawnProperty.h"
#include "rules/simulation/StatusProperty.h"
#include "rules/simulation/SuspendedProperty.h"
#include "rules/simulation/TasksProperty.h"
#include "rules/simulation/TerrainModProperty.h"
#include "rules/simulation/TerrainProperty.h"
#include "rules/simulation/TransientProperty.h"
#include "rules/simulation/VisibilityProperty.h"
#include "rules/simulation/SpawnerProperty.h"
#include "rules/simulation/DefaultLocationProperty.h"
#include "rules/simulation/LimboProperty.h"
#include "rules/simulation/DomainProperty.h"
#include "rules/python/ScriptsProperty.h"
#include "../stubs/rules/simulation/stubEntity.h"
#include "../stubs/rules/stubLocatedEntity.h"

#include "../stubs/common/stubProperty.h"
#include "../stubs/common/stubcustom.h"
#include "../stubs/common/stubLink.h"
#include "../stubs/modules/stubWeakEntityRef.h"
#include "../stubs/rules/stubBBoxProperty.h"
#include "../stubs/rules/simulation/stubSpawnerProperty.h"
#include "../stubs/rules/simulation/stubTerrainModProperty.h"
#include "../stubs/rules/simulation/stubTerrainProperty.h"
#include "../stubs/rules/simulation/stubServerBBoxProperty.h"
#include "../stubs/rules/simulation/stubStatusProperty.h"
#include "../stubs/rules/simulation/stubTasksProperty.h"
#include "../stubs/rules/simulation/stubSpawnProperty.h"
#include "../stubs/rules/simulation/stubDefaultLocationProperty.h"
#include "../stubs/rules/simulation/stubLimboProperty.h"
#include "../stubs/rules/simulation/stubDomainProperty.h"
#include "../stubs/rules/simulation/stubSuspendedProperty.h"
#include "../stubs/rules/simulation/stubModeProperty.h"
#include "../stubs/rules/stubQuaternionProperty.h"
#include "../stubs/rules/simulation/stubAngularFactorProperty.h"
#include "../stubs/rules/simulation/stubGeometryProperty.h"
#include "../stubs/rules/simulation/stubDensityProperty.h"
#include "../stubs/rules/stubVector3Property.h"
#include "../stubs/rules/simulation/stubThing.h"
#include "../stubs/rules/simulation/stubEntityProperty.h"
#include "../stubs/rules/simulation/stubInternalProperties.h"
#include "../stubs/rules/stubSolidProperty.h"
#include "../stubs/rules/simulation/stubPerceptionSightProperty.h"
#include "../stubs/rules/stubScaleProperty.h"
#include "../stubs/rules/python/stubScriptsProperty.h"
#include "../stubs/rules/simulation/stubMindsProperty.h"
#include "../stubs/server/stubAccount.h"
#include "../stubs/server/stubArithmeticBuilder.h"
#include "../stubs/server/stubConnectableRouter.h"
#include "../stubs/rules/simulation/stubAmountProperty.h"
#include "../stubs/rules/simulation/stubCalendarProperty.h"
#include "../stubs/rules/simulation/stubWorldTimeProperty.h"
#include "../stubs/rules/simulation/stubSimulationSpeedProperty.h"
#include "../stubs/rules/simulation/stubModeDataProperty.h"
#include "../stubs/rules/simulation/stubContainedVisibilityProperty.h"
#include "../stubs/rules/simulation/stubModifyProperty.h"
#include "../stubs/rules/simulation/stubModifiersProperty.h"


#define STUB_EntityFactory_newEntity
template <typename T>
Ref<LocatedEntity> EntityFactory<T>::newEntity(const std::string & id, long intId, const Atlas::Objects::Entity::RootEntity & attributes, LocatedEntity* location)
{
    return new Entity(id, intId);
}

class Stackable;
class World;

template <>
Ref<LocatedEntity> EntityFactory<World>::newEntity(const std::string & id, long intId,
        const Atlas::Objects::Entity::RootEntity & attributes, LocatedEntity* location)
{
    return 0;
}



template class EntityFactory<Thing>;
template class EntityFactory<Stackable>;
template class EntityFactory<World>;


#include "../stubs/server/stubPersistence.h"
#include "../stubs/server/stubPlayer.h"

#include "../stubs/server/stubRuleHandler.h"

#include "../stubs/server/stubEntityRuleHandler.h"
#include "../stubs/server/stubArchetypeRuleHandler.h"
#include "../stubs/server/stubOpRuleHandler.h"
#include "../stubs/server/stubPropertyRuleHandler.h"
#include "../stubs/server/stubRuleset.h"
#include "../stubs/server/stubServerRouting.h"
#include "../stubs/server/stubLobby.h"
#include "../stubs/rules/simulation/stubTask.h"
#include "../stubs/rules/simulation/stubUsagesProperty.h"
#include "../stubs/rules/entityfilter/stubFilter.h"
#include "../stubs/rules/stubAtlasProperties.h"
#include "../stubs/rules/simulation/stubCalendarProperty.h"
#include "../stubs/rules/simulation/stubAreaProperty.h"


template <class T>
std::unique_ptr<PropertyBase> PropertyFactory<T>::newProperty()
{
    return std::unique_ptr<PropertyBase>(new T());
}

template <class T>
PropertyFactory<T> * PropertyFactory<T>::duplicateFactory() const
{
    return new PropertyFactory<T>;
}

template class PropertyFactory<MinimalProperty>;


void TeleportProperty::install(LocatedEntity * owner, const std::string & name)
{
}

HandlerResult TeleportProperty::operation(LocatedEntity * ent,
                                          const Operation & op,
                                          OpVector & res)
{
    return OPERATION_IGNORED;
}
#include "../stubs/rules/simulation/stubLineProperty.h"
#include "../stubs/rules/simulation/stubMindProperty.h"
#include "../stubs/rules/simulation/stubAttachmentsProperty.h"
#include "../stubs/rules/simulation/stubModeDataProperty.h"
#include "../stubs/rules/simulation/stubAdminProperty.h"
#include "../stubs/rules/simulation/stubVisibilityProperty.h"

#include "../stubs/rules/simulation/stubTransientProperty.h"
#include "../stubs/rules/stubLocation.h"

#include "../stubs/rules/simulation/stubBaseWorld.h"

bool_config_register::bool_config_register(bool & var,
                                           const char * section,
                                           const char * setting,
                                           const char * help)
{
}


#include "../stubs/common/stubEntityKit.h"
#include "../stubs/server/stubEntityFactory.h"
#include "../stubs/server/stubJuncture.h"


Root atlasType(const std::string & name,
               const std::string & parent,
               bool abstract)
{
    return Atlas::Objects::Root();
}

#define STUB_Inheritance_getClass
const Atlas::Objects::Root& Inheritance::getClass(const std::string & parent, Visibility) const
{
    return noClass;
}

#define STUB_Inheritance_addChild
TypeNode * Inheritance::addChild(const Root & obj)
{
    return new TypeNode(obj->getId());
}


#include "../stubs/common/stubInheritance.h"



#include "../stubs/common/stubMonitors.h"
#include "../stubs/common/stubPropertyManager.h"
#include "../stubs/common/stubShaker.h"

#define STUB_Router_error
void Router::error(const Operation & op,
                   const std::string & errstring,
                   OpVector & res,
                   const std::string & to) const
{
    res.push_back(Atlas::Objects::Operation::Error());
}
#include "../stubs/common/stubRouter.h"

#include "../stubs/common/stubTypeNode.h"
#include "../stubs/common/stubVariable.h"
#include "../stubs/server/stubBuildid.h"

const char * const CYPHESIS = "cyphesis";

static const char * DEFAULT_INSTANCE = "cyphesis";

std::string instance(DEFAULT_INSTANCE);
int timeoffset = 0;
bool database_flag = false;


static long idGenerator = 0;

long newId(std::string & id)
{
    static char buf[32];
    long new_id = ++idGenerator;
    sprintf(buf, "%ld", new_id);
    id = buf;
    assert(!id.empty());
    return new_id;
}

#include "../stubs/common/stublog.h"


Root atlasClass(const std::string & name, const std::string & parent)
{
    return Root();
}

void hash_password(const std::string & pwd, const std::string & salt,
                   std::string & hash)
{
}

int check_password(const std::string & pwd, const std::string & hash)
{
    return -1;
}

void addToEntity(const Point3D & p, std::vector<double> & vd)
{
}
