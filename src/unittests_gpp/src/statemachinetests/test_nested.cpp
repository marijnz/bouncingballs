#include "stdafx.h"
#include "Test_StateMachine.h"
#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/stateMachineFactory.h"
#include "eventTestingUtils.h"
#include "testLog.h"
#include "gpp/dummyLogging.h"

using namespace gpp;
using namespace gpp::sm;

GEP_UNITTEST_TEST(StateMachine, Nested)
{
    GEP_UNITTEST_SETUP_EVENT_GLOBALS;
    auto& logging =
        TestLogging::instance();
    //DummyLogging::instance();

    // set up update framework
    _updateFramework.setElapsedTime(10.0f);

    // Add at least one callback so we know that we are still updating
    size_t frameCount = 0;
    _updateFramework.registerUpdateCallback([&](float){ logging.logWarning("frame %u:", frameCount); });

    StateMachineFactory factory(&g_stdAllocator);

    StateMachine* pMainFsm = nullptr;

    const size_t numTestStages = 12U;
    gep::Hashmap<std::string, size_t, gep::StringHashPolicy> testStages;

    // actual test
    {
        auto pFsm_A = factory.create("fsm_A");
        pFsm_A->setLogging(&logging);

        pMainFsm = pFsm_A;

        // Adding states
        auto pState_A = pFsm_A->create<State>("A");
        auto pFsm_B = pFsm_A->create<StateMachine>("fsm_B");
        auto pState_B_a = pFsm_B->create<State>("a");

        // Adding state transitions
        pFsm_A->addTransition("__enter", "A");
        pFsm_A->addTransition("A", "fsm_B", [&](){ return frameCount == 3; });
        pFsm_A->addTransition("fsm_B", "A");
        pFsm_A->addTransition("fsm_B", "__leave", [&](){ return frameCount == 9; });

        pFsm_B->addTransition("__enter", "a");
        pFsm_B->addTransition("a", "__leave", [&](){ return frameCount == 6; });

        // Set some listeners
        pFsm_A->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages["fsm_A.enter"]++;
            return gep::EventResult::Handled;
        });
        pFsm_A->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages["fsm_A.leave"]++;
            return gep::EventResult::Handled;
        });
        pFsm_A->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages["fsm_A.update"]++;
            return gep::EventResult::Handled;
        });

        pState_A->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages["state_A.enter"]++;
            return gep::EventResult::Handled;
        });
        pState_A->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages["state_A.leave"]++;
            return gep::EventResult::Handled;
        });
        pState_A->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages["state_A.update"]++;
            return gep::EventResult::Handled;
        });

        pFsm_B->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages["fsm_B.enter"]++;
            return gep::EventResult::Handled;
        });
        pFsm_B->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages["fsm_B.leave"]++;
            GEP_ASSERT(testStages["state_B_a.leave"] != 0, "Leaving fsm_B before the inner states!");
            return gep::EventResult::Handled;
        });
        pFsm_B->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages["fsm_B.update"]++;
            return gep::EventResult::Handled;
        });

        pState_B_a->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages["state_B_a.enter"]++;
            return gep::EventResult::Handled;
        });
        pState_B_a->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages["state_B_a.leave"]++;
            return gep::EventResult::Handled;
        });
        pState_B_a->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages["state_B_a.update"]++;
            return gep::EventResult::Handled;
        });
    }

    GEP_ASSERT(pMainFsm, "pFsm may not be null! There must exist 1 state machine as entry point.");
    pMainFsm->setLogging(&logging);

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 0; frameCount < 10; ++frameCount)
    {
        _updateFramework.run();
    }
}
