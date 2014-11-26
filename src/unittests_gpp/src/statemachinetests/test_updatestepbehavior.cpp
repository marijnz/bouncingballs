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

GEP_UNITTEST_TEST(StateMachine, UpdateStepBehavior)
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

    // Test data.

    size_t conditionCheckCount = 0;
    size_t updateCount = 0;

    // Actual tests. Don't put test data in here that has to be referenced
    // by the state machines, as they will get out of scope when pMainFsm
    // is actually run.
    {
        pMainFsm = factory.create("fsm");
        pMainFsm->setLogging(&logging);

        auto pState_A = pMainFsm->create<State>("A");
        auto pState_B = pMainFsm->create<State>("B");
        auto pState_C = pMainFsm->create<State>("C");

        // Adding state transitions
        pMainFsm->addTransition("__enter", "A");
        pMainFsm->addTransition("A", "B", [&](){
            // This should be hit
            conditionCheckCount++;
            return true;
        });
        pMainFsm->addTransition("B", "C", [&](){
            // This should not be hit
            conditionCheckCount++;
            return true;
        });
        pMainFsm->addTransition("C", "__leave");

        pState_B->getLeaveEvent()->registerListener([&](LeaveEventData* pData){
            GEP_ASSERT(conditionCheckCount == 1, "Condition check 'A->B' not performed!");
            conditionCheckCount = 0;
            pData->setNextState(pState_C);
            return gep::EventResult::Handled;
        });

        pState_C->getLeaveEvent()->registerListener([&](LeaveEventData*){
            GEP_ASSERT(conditionCheckCount == 0, "Condition check 'B->C' should not have been performed!");
            conditionCheckCount = 0;
            return gep::EventResult::Handled;
        });

        // Set update event listeners. They should use the UpdateStepBehavior options.
        //////////////////////////////////////////////////////////////////////////

        pState_A->getUpdateEvent()->registerListener([&](UpdateEventData* pData){
            pData->setUpdateStepBehavior(UpdateStepBehavior::Leave);
            updateCount++;
            return gep::EventResult::Handled;
        });

        pState_B->getUpdateEvent()->registerListener([&](UpdateEventData* pData){
            pData->setUpdateStepBehavior(UpdateStepBehavior::LeaveWithNoConditionChecks);
            return gep::EventResult::Handled;
        });
    }

    pMainFsm->setLogging(&logging);

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 0; frameCount < 5; ++frameCount)
    {
        _updateFramework.run();
    }

    GEP_ASSERT(updateCount > 0, "State A should have at least be updated once!");
}
