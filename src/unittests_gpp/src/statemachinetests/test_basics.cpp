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

GEP_UNITTEST_TEST(StateMachine, Basics)
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

    // test data
    size_t testData_A = 0;
    size_t testData_B = 0;

    const size_t numTestStages = 9;
    size_t testStages[numTestStages] = { 0 };

    // actual test
    {
        auto pFsm = pMainFsm = factory.create("fsm");
        pFsm->setLogging(&logging);

        // Adding states
        auto pState_A = pFsm->create<State>("A");
        auto pState_B =  pFsm->create<State>("B");

        // Adding state transitions
        pFsm->addTransition("__enter", "A");
        pFsm->addTransition("A", "B", [&](){ return frameCount == 3; });
        pFsm->addTransition("B", "__leave", [&](){ return frameCount == 6; });

        // Set some listeners
        pFsm->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages[0]++;
            return gep::EventResult::Handled;
        });
        pFsm->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages[1]++;
            return gep::EventResult::Handled;
        });
        pFsm->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages[2]++;
            return gep::EventResult::Handled;
        });

        pState_A->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages[3]++;
            return gep::EventResult::Handled;
        });
        pState_A->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages[4]++;
            return gep::EventResult::Handled;
        });
        pState_A->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages[5]++;
            testData_A++;
            return gep::EventResult::Handled;
        });

        pState_B->getEnterEvent()->registerListener([&](EnterEventData*){
            testStages[6]++;
            return gep::EventResult::Handled;
        });
        pState_B->getLeaveEvent()->registerListener([&](LeaveEventData*){
            testStages[7]++;
            return gep::EventResult::Handled;
        });
        pState_B->getUpdateEvent()->registerListener([&](UpdateEventData*){
            testStages[8]++;
            testData_B++;
            return gep::EventResult::Handled;
        });
    }

    pMainFsm->setLogging(&logging);

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 1; frameCount < 10; ++frameCount)
    {
        _updateFramework.run();
    }

    GEP_ASSERT(testStages[0] == 1, "fsm was never entered");
    GEP_ASSERT(testStages[1] == 1, "fsm was never left");
    GEP_ASSERT(testStages[2] == 0,  "fsm was not supposed to be updated");

    GEP_ASSERT(testStages[3] == 1, "state A was never entered");
    GEP_ASSERT(testStages[4] == 1, "state A was never left");
    GEP_ASSERT(testStages[5] > 1,  "state A was never updated");

    GEP_ASSERT(testStages[6] == 1, "state B was never entered");
    GEP_ASSERT(testStages[7] == 1, "state B was never left");
    GEP_ASSERT(testStages[8] > 1,  "state B was never updated");

    GEP_ASSERT(testData_A == 3, "Invalid number of updates for state A");
    GEP_ASSERT(testData_B == 3, "Invalid number of updates for state B");
}


