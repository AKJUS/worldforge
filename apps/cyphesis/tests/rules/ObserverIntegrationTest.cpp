/*
 Copyright (C) 2020 Erik Ogenvik

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
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "../TestBaseWithContext.h"
#include "../DatabaseNull.h"
#include "../TestWorld.h"
#include "common/Monitors.h"
#include "common/custom.h"
#include "rules/simulation/Inheritance.h"
#include "rules/EntityLocation_impl.h"
#include "../NullEntityCreator.h"
#include "../TestWorldRouter.h"

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Entity.h>
#include <rules/simulation/CorePropertyManager.h>

#include <memory>
#include <utility>
#include <rules/simulation/PhysicalDomain.h>
#include <rules/simulation/VoidDomain.h>
#include <rules/simulation/WorldRouter.h>
#include <common/operations/Tick.h>
#include <wfmath/atlasconv.h>
#include <rules/simulation/AdminProperty.h>
#include <rules/simulation/ContainerAccessProperty.h>
#include <rules/simulation/ContainersActiveProperty.h>
#include <rules/BBoxProperty_impl.h>
#include <rules/simulation/LocatedEntity.h>

using Atlas::Objects::Operation::Set;
using Atlas::Objects::Operation::Wield;
using Atlas::Objects::Entity::Anonymous;
using Atlas::Message::MapType;
using Atlas::Message::ListType;

struct TestContext
{
    Atlas::Objects::Factories factories;
    DatabaseNull database;
    NullEntityCreator entityCreator;
    Ref<LocatedEntity> world;
    Inheritance inheritance;
    TestWorldRouter testWorld;
    CorePropertyManager propertyManager;

    TestContext()
            : world(new LocatedEntity(0)), inheritance(), testWorld(world, entityCreator), propertyManager(inheritance)
    {
    }

    ~TestContext()
    {
        testWorld.shutdown();
    }

    void clearUpdateOpsFlags()
    {
        for (auto entry : testWorld.getEntities()) {
            entry.second->removeFlags(entity_update_broadcast_queued);
        }
    }

    void clearQueues()
    {
        testWorld.getOperationsHandler().clearQueues();
        clearUpdateOpsFlags();
    }
};

namespace {
    std::vector<OpQueEntry<LocatedEntity>> collectQueue(std::priority_queue<OpQueEntry<LocatedEntity>, std::vector<OpQueEntry<LocatedEntity>>, std::greater<OpQueEntry<LocatedEntity>>>& queue)
    {
        std::vector<OpQueEntry<LocatedEntity>> list;
        list.reserve(queue.size());
        while (!queue.empty()) {
            list.emplace_back(queue.top());
            queue.pop();
        }
        return list;
    }

    auto moveFn = [](const Ref<LocatedEntity>& thing, WFMath::Point<3> pos, Ref<LocatedEntity> location = {}) -> OpVector {
        OpVector res;
        Atlas::Objects::Operation::Move move;

        Anonymous arg1;
        arg1->setId(thing->getIdAsString());
        Atlas::Message::Element posElement = pos.toAtlas();
        arg1->setPosAsList(posElement.List());
        if (location) {
            arg1->setLoc(location->getIdAsString());
        }
        move->setArgs1(arg1);
        thing->MoveOperation(move, res);
        return res;
    };

}

OpVector resIgnored;


struct Tested : public Cyphesis::TestBaseWithContext<TestContext>
{
    Tested()
    {
        ADD_TEST(test_handleNestedInventoryContainers)
        ADD_TEST(test_handleNestedContainers)
        ADD_TEST(test_handleContainers)
        ADD_TEST(test_sendAppearDisappearWithPrivateAndProtected);
        ADD_TEST(test_sendAppearDisappear);
    }

    static void sendSetOp(const Ref<LocatedEntity>& entity, const std::string& propertyName, Atlas::Message::Element value)
    {
        Set set;

        Anonymous arg1;
        arg1->setAttr(propertyName, std::move(value));
        set->setArgs1(arg1);

        entity->operation(set, resIgnored);
    }

    void test_handleNestedInventoryContainers(TestContext& context)
    {


        /**
         * Check that entities that are put in a nested inventory container, are handled correctly
         *
         * All entities are placed at origo
         * Hierarchy looks like this:
         * T1 has a physical domain
         * T2 has sight and inventory domain
         * T2, T4, T5 and T7 has a container domain
         *
         *              T1#
         *              T2**
         *              T3*
         *          T4*     T7*
         *          T5*     T8
         *          T6
         */
        {
            WFMath::AxisBox<3> bbox(WFMath::Point<3>(-1, -1, -1), WFMath::Point<3>(1, 1, 1));
            Ref<LocatedEntity> t1 = new LocatedEntity(1);
            t1->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-128, -128, -128},
                                                                    {128,  128,  128}};
            t1->setAttrValue("domain", "physical");
            t1->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            context.testWorld.addEntity(t1, context.world);
            Ref<LocatedEntity> t2 = new LocatedEntity(2);
            t2->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t2->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t2->setAttrValue("perception_sight", 1);
            t2->setAttrValue("reach", 1);
            t2->setAttrValue("domain", "inventory");
            context.testWorld.addEntity(t2, t1);
            Ref<LocatedEntity> t3 = new LocatedEntity(3);
            t3->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t3->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t3->setAttrValue("domain", "container");
            context.testWorld.addEntity(t3, t2);
            Ref<LocatedEntity> t4 = new LocatedEntity(4);
            t4->setAttrValue("domain", "container");
            context.testWorld.addEntity(t4, t3);
            Ref<LocatedEntity> t5 = new LocatedEntity(5);
            t5->setAttrValue("domain", "container");
            context.testWorld.addEntity(t5, t4);
            Ref<LocatedEntity> t6 = new LocatedEntity(6);
            context.testWorld.addEntity(t6, t5);
            Ref<LocatedEntity> t7 = new LocatedEntity(7);
            t7->setAttrValue("domain", "container");
            context.testWorld.addEntity(t7, t3);
            Ref<LocatedEntity> t8 = new LocatedEntity(8);
            context.testWorld.addEntity(t8, t7);

            //Initially only t3 is visible to t2, since it's in its inventory
            ASSERT_TRUE(t3->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t3}))
            ASSERT_FALSE(t4->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_FALSE(t5->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t6->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_FALSE(t8->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t8}))

            //Open containers
            t3->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t2->getIdAsString()});
            t4->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t2->getIdAsString()});
            t5->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t2->getIdAsString()});
            t7->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t2->getIdAsString()});
            ASSERT_TRUE(t4->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_TRUE(t5->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_TRUE(t6->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_TRUE(t8->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t8}))

            //Now move t4 to t1, which should sever the connection to t5 and t6
            moveFn(t4, {0, 0, 0}, t1);


            ASSERT_TRUE(t4->m_parent == t1.get())
            ASSERT_TRUE(t4->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_FALSE(t5->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t6->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_FALSE(t2->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t4->getIdAsString()))
            ASSERT_FALSE(t2->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))
            //Should not affect t8
            ASSERT_TRUE(t2->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t3->getIdAsString()))
            ASSERT_TRUE(t2->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t7->getIdAsString()))
            ASSERT_TRUE(t8->isVisibleForOtherEntity(*t2))
            ASSERT_TRUE(t2->canReach(EntityLocation<LocatedEntity>{t8}))

            //Now move t3 to t1, which should sever the connection to t7 and t8
            moveFn(t3, {0, 0, 0}, t1);
            ASSERT_FALSE(t2->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t3->getIdAsString()))
            ASSERT_FALSE(t2->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t7->getIdAsString()))
            ASSERT_FALSE(t7->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t7}))
            ASSERT_FALSE(t8->isVisibleForOtherEntity(*t2))
            ASSERT_FALSE(t2->canReach(EntityLocation<LocatedEntity>{t8}))
        }
    }


    void test_handleNestedContainers(TestContext& context)
    {


        /**
         * Check that entities only are allowed to look into containers if they are allowed by the __container_access, and that nested callbacks work.
         *
         * All entities are placed at origo
         * Hierarchy looks like this:
         * T1 has a physical domain
         * T2, T4 and T5 has a container domain
         * T3 has sight
         *
         *              T1#
         *         T2*       T3**
         *         T4*
         *         T5*
         *         T6
         */
        {
            WFMath::AxisBox<3> bbox(WFMath::Point<3>(-1, -1, -1), WFMath::Point<3>(1, 1, 1));
            Ref<LocatedEntity> t1 = new LocatedEntity(1);
            t1->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-128, -128, -128},
                                                                    {128,  128,  128}};
            t1->setAttrValue("domain", "physical");
            t1->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            context.testWorld.addEntity(t1, context.world);
            Ref<LocatedEntity> t2 = new LocatedEntity(2);
            t2->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t2->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t2->setAttrValue("domain", "container");
            context.testWorld.addEntity(t2, t1);
            Ref<LocatedEntity> t3 = new LocatedEntity(3);
            t3->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t3->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t3->setAttrValue("perception_sight", 1);
            t3->setAttrValue("reach", 1);
            context.testWorld.addEntity(t3, t1);
            Ref<LocatedEntity> t4 = new LocatedEntity(4);
            t4->setAttrValue("domain", "container");
            context.testWorld.addEntity(t4, t2);
            Ref<LocatedEntity> t5 = new LocatedEntity(5);
            t5->setAttrValue("domain", "container");
            context.testWorld.addEntity(t5, t4);
            Ref<LocatedEntity> t6 = new LocatedEntity(6);
            context.testWorld.addEntity(t6, t5);

            ASSERT_TRUE(t2->isVisibleForOtherEntity(*t3))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))

            //Add t3 as container observer to t2, t4 and t5
            t2->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            t4->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            t5->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t4->getIdAsString()))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))

            //Now move t3 away far enough that it can't reach or see t2 anymore.
            moveFn(t3, {510, 0, 500});
            //Make sure that it cascades, so we can't reach t6 anymore.
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t4->getIdAsString()))
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))

        }
    }

    void test_handleContainers(TestContext& context)
    {

        /**
         * Check that entities only are allowed to look into containers if they are allowed by the __container_access
         *
         * All entities are placed at origo
         * Hierarchy looks like this:
         * T1 has a physical domain
         * T2, T5 and T7 has a container domain
         * T3 has sight and an inventory domain
         *
         *              T1#
         *         T2*       T3**
         *      T4   T5*     T7*
         *           T6      T8
         */
        {
            auto& opsHandler = context.testWorld.getOperationsHandler();
            auto& queue = opsHandler.getQueue();

            WFMath::AxisBox<3> bbox(WFMath::Point<3>(-1, -1, -1), WFMath::Point<3>(1, 1, 1));
            Ref<LocatedEntity> t1 = new LocatedEntity(1);
            t1->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-128, -128, -128},
                                                                    {128,  128,  128}};
            t1->setAttrValue("domain", "physical");
            t1->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            context.testWorld.addEntity(t1, context.world);
            Ref<LocatedEntity> t2 = new LocatedEntity(2);
            t2->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t2->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t2->setAttrValue("domain", "container");
            context.testWorld.addEntity(t2, t1);
            Ref<LocatedEntity> t3 = new LocatedEntity(3);
            t3->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t3->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t3->setAttrValue("domain", "inventory");
            t3->setAttrValue("perception_sight", 1);
            t3->setAttrValue("reach", 1);
            context.testWorld.addEntity(t3, t1);
            Ref<LocatedEntity> t4 = new LocatedEntity(4);
            t4->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t4->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            context.testWorld.addEntity(t4, t2);
            Ref<LocatedEntity> t5 = new LocatedEntity(5);
            t5->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t5->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t5->setAttrValue("domain", "container");
            context.testWorld.addEntity(t5, t2);
            Ref<LocatedEntity> t6 = new LocatedEntity(6);
            t6->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t6->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            context.testWorld.addEntity(t6, t5);
            Ref<LocatedEntity> t7 = new LocatedEntity(7);
            t7->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t7->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            t7->setAttrValue("domain", "container");
            context.testWorld.addEntity(t7, t3);
            Ref<LocatedEntity> t8 = new LocatedEntity(8);
            t8->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t8->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            context.testWorld.addEntity(t8, t7);

            context.clearQueues();

            ASSERT_FALSE(t8->isVisibleForOtherEntity(*t3));
            //Add t3 as container observer to t7, which allows it to view its content (t8), even if it's an inventory domain, as t3 is both the observer and the owner of the inventory.
            t7->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t8->isVisibleForOtherEntity(*t3));

            context.clearQueues();

            ASSERT_TRUE(t2->isVisibleForOtherEntity(*t3))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            //Add t3 as container observer to t2, which allows it to view its content (t4 and t5)
            t2->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_EQUAL(2u, queue.size()) //One Update and one Appearance
            ASSERT_EQUAL(Atlas::Objects::Operation::UPDATE_NO, queue.top()->getClassNo())
            queue.pop();
            ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, queue.top()->getClassNo())
            ASSERT_EQUAL(t3->getIdAsString(), queue.top()->getTo())
            ASSERT_EQUAL(2u, queue.top()->getArgs().size())

            //Add t3 as container observer to t5, which allows it to view its content (t6)
            context.clearQueues();
            t5->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_EQUAL(2u, queue.size())
            queue.pop();
            ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, queue.top()->getClassNo())
            ASSERT_EQUAL(t3->getIdAsString(), queue.top()->getTo())
            ASSERT_EQUAL(1u, queue.top()->getArgs().size())
            //Remove t3 as observer from t5...
            context.clearQueues();
            t5->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{});
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            //Should not affect ability to reach t5 and t2
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_EQUAL(2u, queue.size())
            queue.pop();
            ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, queue.top()->getClassNo())
            ASSERT_EQUAL(t3->getIdAsString(), queue.top()->getTo())
            ASSERT_EQUAL(1u, queue.top()->getArgs().size())
            //...and add it back.
            t5->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            //Remove t3 as observer from t2, which should sever the connection to t5
            context.clearQueues();
            t2->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{});
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))
            //Should still be able to reach t2, even if we can't reach into it.
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_TRUE(t2->isVisibleForOtherEntity(*t3))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t5->isVisibleForOtherEntity(*t3))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_FALSE(t6->isVisibleForOtherEntity(*t3))
            ASSERT_EQUAL(3u, queue.size()) //Get Disappear from t4, t5 and t6
            queue.pop();
            ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, queue.top()->getClassNo())
            ASSERT_EQUAL(t3->getIdAsString(), queue.top()->getTo())
            ASSERT_EQUAL(2u, queue.top()->getArgs().size()) //From both t4 and t5

            //Add it back as observer to both t2 and t5
            t2->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            t5->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});

            //Shall reach all to t6
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))

            context.clearQueues();
            //Now move t3 away far enough that it can't reach or see t2 anymore.
            moveFn(t3, {510, 0, 500});
            //Make sure that it cascades, so we can't reach t6 anymore.
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t4}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t6}))
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t2->getIdAsString()))
            ASSERT_FALSE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t5->getIdAsString()))

            //The disappear ops are sent in random order (depending on memory layout) so we'll put them in a collection to check them.
            std::vector<Operation> disappearOps;
            ASSERT_EQUAL(4u, queue.size()) //One Sight, and then one Update + Disappearance from t2 and t5, as t5 is child of t2
            queue.pop();
            queue.pop();
            ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, queue.top()->getClassNo())
            disappearOps.push_back(queue.top().op);
            queue.pop();
            ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, queue.top()->getClassNo())
            disappearOps.push_back(queue.top().op);
            {
                auto firstOp = disappearOps[0]->getArgs().size() == 1 ? disappearOps[0] : disappearOps[1];
                auto secondOp = disappearOps[0]->getArgs().size() == 1 ? disappearOps[1] : disappearOps[0];

                ASSERT_EQUAL(t3->getIdAsString(), firstOp->getTo())
                ASSERT_EQUAL(1u, firstOp->getArgs().size()) //From t6
                ASSERT_EQUAL(t6->getIdAsString(), firstOp->getArgs()[0]->getId());


                ASSERT_EQUAL(t3->getIdAsString(), secondOp->getTo())
                ASSERT_EQUAL(2u, secondOp->getArgs().size()) //From both t4 and t5, could be in any order
                ASSERT_TRUE(t4->getIdAsString() == secondOp->getArgs()[0]->getId() || t5->getIdAsString() == secondOp->getArgs()[0]->getId());
                ASSERT_TRUE(t4->getIdAsString() == secondOp->getArgs()[1]->getId() || t5->getIdAsString() == secondOp->getArgs()[1]->getId());
            }

            //Move t3 closer again
            moveFn(t3, {0, 0, 0});
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            //t5 should not be reachable since we severed the "reach" connection when moving away
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))
            t2->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t5}))

            //And then once again move it away
            moveFn(t3, {510, 0, 500});

            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t2}))
            ASSERT_FALSE(t3->canReach(EntityLocation<LocatedEntity>{t5}))

            //Move it back, access the container, and then add a new entity and expect an Appearance op
            moveFn(t3, {0, 0, 0});
            t2->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            context.clearQueues();
            Ref<LocatedEntity> t9 = new LocatedEntity(9);
            t9->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = WFMath::Point<3>::ZERO();
            t9->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = bbox;
            context.testWorld.addEntity(t9, t2);

            ASSERT_EQUAL(1u, queue.size())
            ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, queue.top()->getClassNo())
            ASSERT_EQUAL(t3->getIdAsString(), queue.top()->getTo())
            ASSERT_EQUAL(1u, queue.top()->getArgs().size())
            ASSERT_EQUAL(t9->getIdAsString(), queue.top()->getArgs().front()->getId())


            t7->setAttrValue(ContainerAccessProperty::property_name, Atlas::Message::ListType{t3->getIdAsString()});
            ASSERT_TRUE(t3->canReach(EntityLocation<LocatedEntity>{t8}))
            ASSERT_TRUE(t3->getPropertyClassFixed<ContainersActiveProperty>()->hasContainer(t7->getIdAsString()))
        }
    }


    void test_sendAppearDisappearWithPrivateAndProtected(TestContext& context)
    {
        auto& opsHandler = context.testWorld.getOperationsHandler();
        auto& queue = opsHandler.getQueue();

        long counter = 1;
        Ref<LocatedEntity> domainPhysical(new LocatedEntity(counter++));
        context.testWorld.addEntity(domainPhysical, context.world);
        domainPhysical->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-512, -512, -512},
                                                                            {512,  512,  512}};
        domainPhysical->setDomain(std::make_unique<PhysicalDomain>(*domainPhysical));

        // Clear ops queue
        context.clearQueues();

        // Make an observer, which we'll add to the physical domain
        Ref<LocatedEntity> observer(new LocatedEntity(counter++));
        observer->setAttrValue("mode", "fixed");
        observer->setAttrValue("perception_sight", 1);
        observer->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {0, 0, 0};
        context.testWorld.addEntity(observer, domainPhysical);

        // Make an admin observer, which we'll add to the physical domain
        Ref<LocatedEntity> observerAdmin(new LocatedEntity(counter++));
        observerAdmin->setAttrValue("mode", "fixed");
        observerAdmin->setAttrValue("perception_sight", 1);
        observerAdmin->setAttrValue(AdminProperty::property_name, 1);
        observerAdmin->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {0, 0, 0};
        context.testWorld.addEntity(observerAdmin, domainPhysical);

        auto ops = collectQueue(queue);
        context.clearQueues();
        OpVector res;

        // Create a private entity, which should only be seen by observerAdmin
        Ref<LocatedEntity> objectPrivate1(new LocatedEntity(counter++));
        objectPrivate1->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-1, -1, -1},
                                                                            {1,  1,  1}};
        objectPrivate1->setAttrValue("mode", "fixed");
        objectPrivate1->setAttrValue("visibility", "private");
        objectPrivate1->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {0, 0, 0};
        context.testWorld.addEntity(objectPrivate1, domainPhysical);

        ops = collectQueue(queue);

        // We now expect to get an Appearance op sent to the admin observer (but not the regular observer)
        ASSERT_EQUAL(1u, ops.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, ops.front()->getClassNo())
        ASSERT_EQUAL(objectPrivate1->getIdAsString(), ops.front()->getArgs().front()->getId())
        ASSERT_EQUAL(observerAdmin->getIdAsString(), ops.front()->getTo())

        context.clearQueues();

        // Move objectPrivate1 a bit; only admin observer should see it
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(objectPrivate1->getIdAsString());
            arg1->setPos({510, 0, 500});
            move->setArgs1(arg1);
            objectPrivate1->MoveOperation(move, res);
        }
        ASSERT_EQUAL(1u, res.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, res.front()->getClassNo())
        ASSERT_EQUAL(Atlas::Objects::Operation::MOVE_NO, res.front()->getArgs().front()->getClassNo())
        ASSERT_EQUAL(objectPrivate1->getIdAsString(), Atlas::Objects::smart_dynamic_cast<Operation>(res.front()->getArgs().front())->getArgs().front()->getId())
        ASSERT_EQUAL(observerAdmin->getIdAsString(), res.front()->getTo())
    }

    void test_sendAppearDisappear(TestContext& context)
    {
        auto& opsHandler = context.testWorld.getOperationsHandler();
        auto& queue = opsHandler.getQueue();

        long counter = 1;
        Ref<LocatedEntity> domainPhysical(new LocatedEntity(counter++));
        context.testWorld.addEntity(domainPhysical, context.world);
        domainPhysical->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-512, -512, -512},
                                                                            {512,  512,  512}};
        domainPhysical->setDomain(std::make_unique<PhysicalDomain>(*domainPhysical));

        Ref<LocatedEntity> domainVoid(new LocatedEntity(counter++));
        context.testWorld.addEntity(domainVoid, context.world);
        domainVoid->setDomain(std::make_unique<VoidDomain>(*domainVoid));

        auto domainTickFn = [&]() -> OpVector {
            // We must send a Tick op to make the domain handle appear and disappear
            OpVector res;
            Atlas::Objects::Operation::Tick tick;
            Atlas::Objects::Entity::Anonymous arg1;
            arg1->setName("domain");
            tick->setArgs1(arg1);
            tick->setAttr("lastTick", 0.0f);
            tick->setStamp(2000);  // This should trigger a visibility.
            domainPhysical->getDomain()->operation(*domainPhysical, tick, res);
            return res;
        };

        // Clear ops queue
        context.clearQueues();

        // Make an observer, which we'll add to the physical domain
        Ref<LocatedEntity> observer(new LocatedEntity(counter++));
        observer->setAttrValue("mode", "fixed");
        observer->setAttrValue("perception_sight", 1);
        observer->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {0, 0, 0};
        context.testWorld.addEntity(observer, domainPhysical);

        auto ops = collectQueue(queue);
        // We now expect to get an Appearence op sent to the observer about the domain entity, as well as an Appearance sent to the observer itself.
        ASSERT_EQUAL(2u, ops.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, ops[0]->getClassNo())
        ASSERT_EQUAL(domainPhysical->getIdAsString(), ops[0]->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), ops[0]->getTo())

        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, ops[1]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), ops[1]->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), ops[1]->getTo())

        context.clearQueues();

        // Make another observer, which we'll add to the void domain
        Ref<LocatedEntity> observer_void(new LocatedEntity(counter++));
        observer_void->setAttrValue("mode", "fixed");
        observer_void->setAttrValue("perception_sight", 1);
        observer_void->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {0, 0, 0};
        context.testWorld.addEntity(observer_void, domainVoid);

        // Clear ops queue
        context.clearQueues();

        // Create something we can look at
        Ref<LocatedEntity> object1(new LocatedEntity(counter++));
        object1->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-1, -1, -1},
                                                                     {1,  1,  1}};
        object1->setAttrValue("mode", "fixed");
        object1->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {10, 0, 10};
        context.testWorld.addEntity(object1, domainPhysical);

        ops = collectQueue(queue);

        // We now expect to get an Appearance op sent to the observer (but not the observer in the void)
        ASSERT_EQUAL(1u, ops.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, ops.front()->getClassNo())
        ASSERT_EQUAL(object1->getIdAsString(), ops.front()->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), ops.front()->getTo())

        context.clearQueues();

        OpVector res;

        //Just doing a physics tick should not result in any extra ops.
        res = domainTickFn();
        ops = collectQueue(queue);
        ASSERT_EQUAL(0u, ops.size())
        ASSERT_EQUAL(1u, res.size())  // Should only contain Tick

        context.clearQueues();
        res.clear();


        // Move object1 just slightly, which should not result in any Appear or Disappear op.
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(object1->getIdAsString());
            arg1->setPosAsList({10.1, 0, 10});
            move->setArgs1(arg1);
            object1->MoveOperation(move, res);
        }
        ops = collectQueue(queue);
        ASSERT_EQUAL(1u, ops.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, ops[0]->getClassNo())
        ASSERT_EQUAL(1u, res.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, res[0]->getClassNo())

        context.clearQueues();
        res.clear();
        res = domainTickFn();
        ops = collectQueue(queue);
        context.clearQueues();
        ASSERT_EQUAL(0u, ops.size())
        ASSERT_EQUAL(1u, res.size())  // Should only contain Tick

        context.clearQueues();
        res.clear();


        // Move object1 to the void domain
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(object1->getIdAsString());
            arg1->setLoc(domainVoid->getIdAsString());
            move->setArgs1(arg1);
            object1->MoveOperation(move, res);
        }
        ops = collectQueue(queue);
        context.clearQueues();

        // We now expect to get a Disappearance op sent to the observer (but nothing sent to the observer in the void). The first op should be a Sight op, the second an Update op.
        ASSERT_EQUAL(0u, ops.size())
        ASSERT_EQUAL(3u, res.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, res[0]->getClassNo())
        ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, res[1]->getClassNo())
        ASSERT_EQUAL(Atlas::Objects::Operation::UPDATE_NO, res[2]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())
        ASSERT_EQUAL(object1->getIdAsString(), res[1]->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), res[1]->getTo())

        res.clear();

        // Move object1 back to the physical domain
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(object1->getIdAsString());
            arg1->setLoc(domainPhysical->getIdAsString());
            arg1->setPosAsList({0, 0, 0});
            move->setArgs1(arg1);
            object1->MoveOperation(move, res);
        }
        ops = collectQueue(queue);
        context.clearQueues();

        // We now expect to get an Appearance op sent to the observer (but nothing sent to the observer in the void). The first op should be an Update op
        ASSERT_EQUAL(0u, ops.size())
        ASSERT_EQUAL(2u, res.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, res[0]->getClassNo())
        ASSERT_EQUAL(Atlas::Objects::Operation::UPDATE_NO, res[1]->getClassNo())
        ASSERT_EQUAL(object1->getIdAsString(), res[0]->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())

        res.clear();

        // Delete object1
        {
            Atlas::Objects::Operation::Delete deleteOp;
            object1->DeleteOperation(deleteOp, res);
        }
        ops = collectQueue(queue);
        context.clearQueues();

        // A Sight is sent to the deleted entity, but it's sent directly without being put on the queue.
        ASSERT_EQUAL(0u, ops.size())
        ASSERT_EQUAL(2u, res.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, res[0]->getClassNo())
        ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, res[1]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())
        ASSERT_EQUAL(object1->getIdAsString(), res[1]->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), res[1]->getTo())

        // Create a new object, which we'll then move away beyond visible distance
        Ref<LocatedEntity> object2(new LocatedEntity(counter++));
        object2->requirePropertyClassFixed<BBoxProperty<LocatedEntity>>().data() = {{-0.1, -0.1, -0.1},
                                                                     {0.1,  0.1,  0.1}};
        object2->setAttrValue("mode", "fixed");
        object2->requirePropertyClassFixed<PositionProperty<LocatedEntity>>().data() = {5, 0, 5};
        context.testWorld.addEntity(object2, domainPhysical);

        ops = collectQueue(queue);

        // We now expect to get an Appearance op sent to the observer (but not the observer in the void)
        ASSERT_EQUAL(1u, ops.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, ops.front()->getClassNo())
        ASSERT_EQUAL(object2->getIdAsString(), ops.front()->getArgs().front()->getId())
        ASSERT_EQUAL(observer->getIdAsString(), ops.front()->getTo())

        domainTickFn();
        context.clearQueues();
        res.clear();
        // Move object2 away a great distance, outside of visible range
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(object2->getIdAsString());
            arg1->setPos({500, 0, 500});
            move->setArgs1(arg1);
            object2->MoveOperation(move, res);
        }
        ops = collectQueue(queue);
        // We now expect to get a Disappearance op sent to the observer. The first op should be a Sight op, the second an Update op.
        // The ops contains a Sight of a Set, which contains the new positional attributes as the entity moved within the PhysicalDomain.
        ASSERT_EQUAL(1u, ops.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, ops[0]->getClassNo())
        ASSERT_EQUAL(Atlas::Objects::Operation::SET_NO, ops[0]->getArgs().front()->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), ops[0]->getTo())
        ASSERT_EQUAL(1u, res.size())
        ASSERT_EQUAL(Atlas::Objects::Operation::SIGHT_NO, res[0]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())

        // We must send a Tick op to make the domain handle appear and disappear
        res = domainTickFn();
        ASSERT_EQUAL(2u, res.size())  // Second op is a Tick
        ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, res[0]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())
        ASSERT_EQUAL(object2->getIdAsString(), res[0]->getArgs().front()->getId())

        context.clearQueues();
        res.clear();
        // Move object2 a bit; nothing should see it
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(object2->getIdAsString());
            arg1->setPos({510, 0, 500});
            move->setArgs1(arg1);
            object2->MoveOperation(move, res);
        }
        ops = collectQueue(queue);
        ASSERT_TRUE(res.empty())
        ASSERT_TRUE(ops.empty())
        res = domainTickFn();
        ASSERT_EQUAL(1u, res.size())  // Should only contain Tick

        context.clearQueues();
        // Make object2 very much larger, so it should appear
        {
            object2->setAttrValue("bbox", WFMath::AxisBox<3>{{-500, -500, -500},
                                                             {500,  500,  500}}.toAtlas());
        }
        ops = collectQueue(queue);
        ASSERT_TRUE(ops.empty())
        res = domainTickFn();
        ASSERT_EQUAL(2u, res.size())  // Second op is a Tick

        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, res[0]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())
        ASSERT_EQUAL(object2->getIdAsString(), res[0]->getArgs().front()->getId())

        context.clearQueues();
        // Make object2 smaller again, so it should disappear
        {
            object2->setAttrValue("bbox", WFMath::AxisBox<3>{{-1, -1, -1},
                                                             {1,  1,  1}}.toAtlas());
        }
        res = domainTickFn();
        ASSERT_EQUAL(2u, res.size())  // Second op is a Tick

        ASSERT_EQUAL(Atlas::Objects::Operation::DISAPPEARANCE_NO, res[0]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())
        ASSERT_EQUAL(object2->getIdAsString(), res[0]->getArgs().front()->getId())


        context.clearQueues();
        res.clear();
        // Move object2 close to the observer so it appears
        {
            Atlas::Objects::Operation::Move move;

            Anonymous arg1;
            arg1->setId(object2->getIdAsString());
            arg1->setPos({0, 0, 0});
            move->setArgs1(arg1);
            object2->MoveOperation(move, res);
        }
        ops = collectQueue(queue);
        ASSERT_TRUE(res.empty())
        ASSERT_TRUE(ops.empty())
        res = domainTickFn();
        ASSERT_EQUAL(2u, res.size())  // Second op is a Tick

        ASSERT_EQUAL(Atlas::Objects::Operation::APPEARANCE_NO, res[0]->getClassNo())
        ASSERT_EQUAL(observer->getIdAsString(), res[0]->getTo())
        ASSERT_EQUAL(object2->getIdAsString(), res[0]->getArgs().front()->getId())
    }
};

int main()
{
    Monitors m;
    Tested t;

    return t.run();
}
