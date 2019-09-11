#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif


#include "TestBase.h"

#include "rules/entityfilter/Filter.h"
#include "rules/entityfilter/ParserDefinitions.h"
#include "rules/entityfilter/Providers.h"

#include "rules/simulation/EntityProperty.h"
#include "rules/Domain.h"
#include "rules/AtlasProperties.h"
#include "rules/BBoxProperty.h"
#include "common/Property.h"
#include "rules/simulation/BaseWorld.h"
#include "common/log.h"
#include "common/Inheritance.h"

#include "rules/simulation/Entity.h"
#include "common/TypeNode.h"

#include <wfmath/point.h>
#include <Atlas/Objects/Anonymous.h>
#include <Atlas/Objects/Factories.h>

#include <cassert>

static std::map<std::string, TypeNode*> types;

using namespace EntityFilter;
using namespace boost::spirit;

Atlas::Objects::Factories factories;
///\These tests aim at verifying that entity filter parser builds
///correct predicates for given queries
class ParserTest : public Cyphesis::TestBase {
    private:
        //A helper function to build a predicate for a given query
        std::shared_ptr<Predicate> ConstructPredicate(const std::string &query);
    public:
        ParserTest();

        void setup();
        void teardown();

        void test_ComparisonOperators();
        void test_LogicalOperators();
        void test_Literals();

        Inheritance* m_inheritance;
};

ParserTest::ParserTest()
{
    ADD_TEST(ParserTest::test_ComparisonOperators);
    ADD_TEST(ParserTest::test_LogicalOperators);
    ADD_TEST(ParserTest::test_Literals);
}

void ParserTest::setup()
{
    m_inheritance = new Inheritance(factories);
}

void ParserTest::teardown()
{
    delete m_inheritance;
}

void ParserTest::test_ComparisonOperators()
{
    std::shared_ptr<Predicate> pred;

    pred = ConstructPredicate("1 = 2");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::EQUALS);

    pred = ConstructPredicate("1 != 2");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::NOT_EQUALS);

    pred = ConstructPredicate("1 > 2");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::GREATER);

    pred = ConstructPredicate("1 < 2");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::LESS);

    pred = ConstructPredicate("1 <= 2");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::LESS_EQUAL);

    pred = ConstructPredicate("1 >= 2");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::GREATER_EQUAL);

    pred = ConstructPredicate("entity.container includes 1");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::INCLUDES);

    pred = ConstructPredicate("1 in entity.container");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::IN);

    pred = ConstructPredicate("entity can_reach entity");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::CAN_REACH);

    pred = ConstructPredicate("entity can_reach entity with entity");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::CAN_REACH);

    //Instance_of can only be created for existing types
    TypeNode* thingType = new TypeNode("thing");
    types["thing"] = thingType;
    pred = ConstructPredicate(
            "types.thing = entity.type");
    assert(static_cast<ComparePredicate*>(pred.get())->m_comparator == ComparePredicate::Comparator::EQUALS);
    types["thing"] = nullptr;
    delete thingType;

     //Should not throw an exception for nonexisting type
     ConstructPredicate("types.nonexistant = entity.type");
}

void ParserTest::test_LogicalOperators()
{
    std::shared_ptr<Predicate> pred;

    pred = ConstructPredicate("1 = 2 or 3 = 4");
    assert(typeid(*pred) == typeid(OrPredicate));

    pred = ConstructPredicate("1 = 2 and 3 = 4");
    assert(typeid(*pred) == typeid(AndPredicate));

    pred = ConstructPredicate("!5 = 6");
    assert(typeid(*pred) == typeid(NotPredicate));

    pred = ConstructPredicate("not 7 = 8");
    assert(typeid(*pred) == typeid(NotPredicate));

    //Test precedence. not should be applied to 1 = 2, not the whole expression
    pred = ConstructPredicate("not 1 = 2 and 3 = 4");
    assert(typeid(*pred) == typeid(AndPredicate));
}

void ParserTest::test_Literals()
{
    std::shared_ptr<Predicate> pred;
    using Atlas::Message::Element;

    //Test int and single quote string
    pred = ConstructPredicate("1 = '1'");
    FixedElementProvider *lhs = (FixedElementProvider *)static_cast<ComparePredicate*>(pred.get())->m_lhs.get();
    FixedElementProvider *rhs = (FixedElementProvider *)static_cast<ComparePredicate*>(pred.get())->m_rhs.get();

    ASSERT_TRUE(lhs->m_element == Element(1));
    ASSERT_TRUE(rhs->m_element == Element("1"));

    //Test double and bool
    pred = ConstructPredicate("1.25 = true");
    lhs = (FixedElementProvider *)static_cast<ComparePredicate*>(pred.get())->m_lhs.get();
    rhs = (FixedElementProvider *)static_cast<ComparePredicate*>(pred.get())->m_rhs.get();

    ASSERT_TRUE(lhs->m_element == Element(1.25));
    ASSERT_TRUE(rhs->m_element == Element(true));

    //Test list and double quoted string
    pred = ConstructPredicate("[1, 2, 3] = '\"literal\"'");
    lhs = (FixedElementProvider *)static_cast<ComparePredicate*>(pred.get())->m_lhs.get();
    rhs = (FixedElementProvider *)static_cast<ComparePredicate*>(pred.get())->m_rhs.get();

    ASSERT_TRUE(lhs->m_element == Element(std::vector<Element> { 1, 2, 3 }));
    ASSERT_TRUE(rhs->m_element == Element("\"literal\""));

}

std::shared_ptr<Predicate> ParserTest::ConstructPredicate(const std::string &query)
{
    auto iter_begin = query.begin();
    auto iter_end = query.end();
    ProviderFactory factory{};
    parser::query_parser<std::string::const_iterator> grammar(factory);

    std::shared_ptr<Predicate> pred;

    bool parse_success = qi::phrase_parse(iter_begin, iter_end, grammar,
                                          boost::spirit::ascii::space, pred);

    if (!(parse_success && iter_begin == iter_end)) {
        throw std::invalid_argument(
                "Attempted creating entity filter with invalid query");
    }

    return pred;

}

int main(int argc, char ** argv)
{
    ParserTest t;

    return t.run();
}

//Stubs

#include "stubs/common/stubVariable.h"
#include "stubs/common/stubMonitors.h"
#include "stubs/common/stubLink.h"
#include "stubs/rules/simulation/stubDomainProperty.h"
#include "stubs/rules/simulation/stubDensityProperty.h"
#include "stubs/rules/stubScaleProperty.h"
#include "stubs/rules/stubAtlasProperties.h"
#include "stubs/common/stubcustom.h"
#include "stubs/common/stubRouter.h"
#include "stubs/rules/simulation/stubBaseWorld.h"
#include "stubs/rules/stubLocation.h"

#define STUB_Inheritance_getType
const TypeNode* Inheritance::getType(const std::string & parent) const
{
    auto I = types.find(parent);
    if (I == types.end()) {
        return nullptr;
    }
    return I->second;
}

#include "stubs/common/stubInheritance.h"


void log(LogLevel lvl, const std::string & msg)
{
}
