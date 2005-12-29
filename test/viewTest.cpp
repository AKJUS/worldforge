#include "viewTest.h"
#include "testUtils.h"
#include "controller.h"
#include "signalHelpers.h"
#include "setupHelpers.h"

#include <Eris/Account.h>
#include <Eris/Avatar.h>
#include <Eris/Entity.h>
#include <Eris/View.h>
#include <Eris/PollDefault.h>
#include <sigc++/object_slot.h>
#include <sigc++/object.h>

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Anonymous.h>
#include <Atlas/Objects/Entity.h>
#include <wfmath/timestamp.h>

using std::cout;
using std::endl;

using WFMath::TimeStamp;
using WFMath::TimeDiff;

class ViewObserver : public SigC::Object
{
public:
    ViewObserver(Eris::View* v)
    {
        v->Appearance.connect(SigC::slot(*this, &ViewObserver::onAppear));
        v->Disappearance.connect(SigC::slot(*this, &ViewObserver::onDisappear));
    }

    int getAppearCount(Eris::Entity* e)
    {
        return m_appears[e];
    }
    
    int getDisappearCount(Eris::Entity* e)
    {
        return m_disappears[e];
    }
private:
    void onAppear(Eris::Entity* e)
    {
        m_appears[e]++;
    }
    
    void onDisappear(Eris::Entity* e)
    {
        m_disappears[e]++;
    }
    
    std::map<Eris::Entity*, int> m_appears, 
        m_disappears;
};

class CharacterGetter : public SigC::Object
{
public:
    CharacterGetter(Eris::Avatar* av) :
        m_done(false)
    {
        av->GotCharacterEntity.connect(SigC::slot(*this, &CharacterGetter::onGetCharacter));
    }
    
    void run()
    {
        while (!m_done) Eris::PollDefault::poll();
    }
private:
    void onGetCharacter(Eris::Entity*)
    {
        m_done = true;
    }
    
    bool m_done;
};


WaitForAppearance::WaitForAppearance(Eris::View* v, const std::string& eid) : 
    m_view(v)
{
    waitFor(eid);
    v->Appearance.connect(SigC::slot(*this, &WaitForAppearance::onAppear));
}

void WaitForAppearance::waitFor(const std::string& eid)
{
    Eris::Entity* e = m_view->getEntity(eid);
    if (e && e->isVisible()) {
        return;
    }
    
    m_waiting.insert(eid);
}

void WaitForAppearance::run()
{
    TimeStamp end = TimeStamp::now() + TimeDiff(5 * 1000);
    
    while (!m_waiting.empty() && (TimeStamp::now() < end)) {
        Eris::PollDefault::poll();
    }
    
    if (!m_waiting.empty()) {
        cout << "timed out waiting for:" << endl;
        std::set<std::string>::const_iterator it;
        for (it = m_waiting.begin(); it != m_waiting.end(); ++it) {
            cout << "\t" << *it << endl;
        }
    }
}

void WaitForAppearance::onAppear(Eris::Entity* e)
{
    m_waiting.erase(e->getId());
}


WaitForDisappearance::WaitForDisappearance(Eris::View* v, const std::string& eid) : 
    m_view(v)
{
    waitFor(eid);
    v->Disappearance.connect(SigC::slot(*this, &WaitForDisappearance::onDisappear));
}

void WaitForDisappearance::waitFor(const std::string& eid)
{
    Eris::Entity* e = m_view->getEntity(eid);
    if (!e || !e->isVisible()) {
        std::cerr << "wait on invisible entity " << eid << std::endl;
        return;
    }
    
    m_waiting.insert(eid);
}

void WaitForDisappearance::run()
{
    while (!m_waiting.empty()) Eris::PollDefault::poll();
}

void WaitForDisappearance::onDisappear(Eris::Entity* e)
{
    m_waiting.erase(e->getId());
}

#pragma mark -

void testCharacterInitialVis(Controller& ctl)
{
    AutoConnection con = stdConnect();
    AutoAccount player = stdLogin("account_B", "sweede", con.get());
    ctl.setEntityVisibleToAvatar("_hut_01", "acc_b_character");
    
    AvatarGetter ag(player.get());
    ag.returnOnSuccess();
    AutoAvatar av = ag.take("acc_b_character");
    
    Eris::View* v = av->getView();
    ViewObserver vob(v);
    
    CharacterGetter cg(av.get());
    cg.run();
    
    assert(vob.getAppearCount(av->getEntity()) == 1);
    assert(vob.getDisappearCount(av->getEntity()) == 0);
}

void testAppearance(Controller& ctl)
{
    AutoConnection con = stdConnect();
    AutoAccount acc = stdLogin("account_B", "sweede", con.get());
    
    ctl.setEntityVisibleToAvatar("_hut_01", "acc_b_character");
    AutoAvatar av = AvatarGetter(acc.get()).take("acc_b_character");
    
    Eris::View* v = av->getView();

    ctl.setEntityVisibleToAvatar("_potato_1", av.get());    
    ctl.setEntityVisibleToAvatar("_potato_2", av.get());
    ctl.setEntityVisibleToAvatar("_pig_01", av.get());
    ctl.setEntityVisibleToAvatar("_field_01", av.get());
        
    {
        WaitForAppearance wf(v, "_field_01");
        wf.waitFor("_pig_01");
        wf.waitFor("_potato_2");
        wf.run();
    }
    
    ctl.setEntityInvisibleToAvatar("_field_01", av.get());
    WaitForDisappearance df(v, "_field_01");
    df.waitFor("_pig_01");
    df.waitFor("_potato_2");
    
    df.run();
    
    ctl.setEntityVisibleToAvatar("_potato_1", av.get());
    //
    ctl.setEntityVisibleToAvatar("_pig_02", av.get());
    ctl.setEntityVisibleToAvatar("_field_01", av.get());
    
    {
        WaitForAppearance af2(v, "_field_01");
        af2.waitFor("_potato_1");
        af2.run();
    }
}

void testLookQueue(Controller& ctl)
{
    AutoConnection con = stdConnect();
    AutoAccount acc = stdLogin("account_B", "sweede", con.get());
    ctl.setEntityVisibleToAvatar("_hut_01", "acc_b_character");
        
    ctl.command("add-many-objects", "acc_b_character");
    AutoAvatar av = AvatarGetter(acc.get()).take("acc_b_character");
    
    Eris::View* v = av->getView();
    
    {
        WaitForAppearance wf(v, "_oak300");
        wf.run();
    }
}

/*
void testSightCreate(Controller& ctl)
{
    AutoConnection con = stdConnect();
    AutoAccount acc = stdLogin("account_B", "sweede", con.get());
    
    ctl.setEntityVisibleToAvatar("_hut_01", "acc_b_character");
    ctl.setEntityVisibleToAvatar("_table_1", "acc_b_character");
    ctl.setEntityVisibleToAvatar("_vase_1", "acc_b_character");
    
    AutoAvatar av = AvatarGetter(acc.get()).take("acc_b_character");
    {
        WaitForAppearance wf(av->getView(), "_vase_1");
        wf.run();
    }

    SignalRecorder1<Eris::Entity*> created;
    av->getView()->EntityCreated.connect(SigC::slot(created, 
        &SignalRecorder1<Eris::Entity*>::fired));
    
    RootEntity gent;
    Atlas::Message::ListType prs;
    prs.push_back("book");
    gent->setParentsAsList(prs);
    gent->setName("The Lord of the Blings");
    gent->setAttr("foob", 42);
    gent->setLoc("_table_1");
    ctl.create(gent);
    
    while (!created.fireCount()) {
        Eris::PollDefault::poll();
    }
    
    assert(created.lastArg0()->getName() == "The Lord of the Blings");
    assert(created.lastArg0()->valueOfAttr("foob") == (long int) 42);
}
*/

void testEntityCreation(Controller& ctl)
{
    AutoConnection con = stdConnect();
    AutoAccount acc = stdLogin("account_B", "sweede", con.get());
    
    ctl.setEntityVisibleToAvatar("_hut_01", "acc_b_character");
    AutoAvatar av = AvatarGetter(acc.get()).take("acc_b_character");
    Eris::View* v = av->getView();
    
    Eris::TestInjector i(con.get());

    SignalRecorder1<Eris::Entity*> created, appeared;
    v->EntityCreated.connect(SigC::slot(created, &SignalRecorder1<Eris::Entity*>::fired));
    v->Appearance.connect(SigC::slot(appeared, &SignalRecorder1<Eris::Entity*>::fired));
    
    {
        Atlas::Objects::Operation::Appearance app;
        app->setTo("acc_b_character");
        Atlas::Objects::Entity::Anonymous arg;
        arg->setId("magic_id_1"); 
        app->setArgs1(arg);
        
        i.inject(app);
        assert(created.fireCount() == 0);
        assert(appeared.fireCount() == 0);
    }
    
    {
        Atlas::Objects::Operation::Create cr;
        Atlas::Objects::Entity::GameEntity arg;
        arg->setId("magic_id_1"); 
        arg->setName("foom");
        arg->setLoc("_hut_01");
        cr->setArgs1(arg);
        
        Atlas::Objects::Operation::Sight st;
        st->setTo("acc_b_character");
        st->setArgs1(cr);
        
        i.inject(st);
        assert(created.fireCount() == 1);
        assert(appeared.fireCount() == 1);
    }
    
    created.reset();
    appeared.reset();
    
    {
        Atlas::Objects::Operation::Create cr;
        Atlas::Objects::Entity::GameEntity arg;
        arg->setId("magic_id_2"); 
        arg->setName("wibble");
        arg->setLoc("_hut_01");
        cr->setArgs1(arg);
        
        Atlas::Objects::Operation::Sight st;
        st->setTo("acc_b_character");
        st->setArgs1(cr);
        
        i.inject(st);
        assert(created.fireCount() == 0);
        assert(appeared.fireCount() == 0);
    }

    /*
    {
        Atlas::Objects::Operation::Appearance app;
        app->setTo("acc_b_character");
        Atlas::Objects::Entity::Anonymous arg;
        arg->setId("magic_id_2"); 
        app->setArgs1(arg);
        
        i.inject(app);
        assert(created.fireCount() == 1);
        assert(appeared.fireCount() == 1);
    }

    */
}


