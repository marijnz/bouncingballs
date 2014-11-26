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

GEP_UNITTEST_TEST(StateMachine, LastTransitionConditionWins)
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

    // Actual tests. Don't put test data in here that has to be referenced
    // by the state machines, as they will get out of scope when pMainFsm
    // is actually run.
    {
        pMainFsm = factory.create("fsm");
        pMainFsm->setLogging(&logging);

        auto pState_A = pMainFsm->create<State>("A");
        auto pState_B = pMainFsm->create<State>("B");
        auto pState_C = pMainFsm->create<State>("C");
        auto pState_D = pMainFsm->create<State>("D");

        // Adding state transitions
        pMainFsm->addTransition("__enter", "A");
        pMainFsm->addTransition("A", "B");
        pMainFsm->addTransition("A", "C");
        pMainFsm->addTransition("A", "D");
        pMainFsm->addTransition("B", "__leave");
        pMainFsm->addTransition("C", "__leave");
        pMainFsm->addTransition("D", "__leave");

        // register leave event listeners for state "A"
        // Will be called first
        pState_A->getLeaveEvent()->registerListener(-100, [&](LeaveEventData* pData){
            pData->setNextState(pState_B);
            return gep::EventResult::Handled;
        });
        // Will be called second
        pState_A->getLeaveEvent()->registerListener(0, [&](LeaveEventData* pData){
            pData->setNextState(pState_C);
            return gep::EventResult::Handled;
        });
        // Will be called last. This one should "win".
        pState_A->getLeaveEvent()->registerListener(100, [&](LeaveEventData* pData){
            pData->setNextState(pState_D);
            return gep::EventResult::Handled;
        });

        // register enter event listeners for the states that we do not expect to be entered.
        pState_B->getEnterEvent()->registerListener([](EnterEventData*){
            GEP_ASSERT(false, "Unexpected state change!");
            return gep::EventResult::Handled;
        });
        pState_C->getEnterEvent()->registerListener([](EnterEventData*){
            GEP_ASSERT(false, "Unexpected state change!");
            return gep::EventResult::Handled;
        });
    }

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 0; frameCount < 3; ++frameCount)
    {
        _updateFramework.run();
    }
}
