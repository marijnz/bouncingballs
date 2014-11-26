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

GEP_UNITTEST_TEST(StateMachine, ConditionEvaluationBeforeLeaveEvent)
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

    size_t conditionHitCounter = 0;

    // Actual tests. Don't put test data in here that has to be referenced
    // by the state machines, as they will get out of scope when pMainFsm
    // is actually run.
    {
        pMainFsm = factory.create("fsm");
        pMainFsm->setLogging(&logging);

        auto pState_A = pMainFsm->create<State>("A");
        auto pState_B = pMainFsm->create<State>("B");

        // Adding state transitions
        pMainFsm->addTransition("__enter", "A");
        pMainFsm->addTransition("A", "B", [&](){ conditionHitCounter++; return true; });
        pMainFsm->addTransition("A", "B", [&](){ conditionHitCounter++; return true; });
        pMainFsm->addTransition("A", "B", [&](){ conditionHitCounter++; return true; });
        pMainFsm->addTransition("B", "__leave");

        pState_A->getLeaveEvent()->registerListener([&](LeaveEventData*){
            GEP_ASSERT(conditionHitCounter == 3,
                       "Not all or too many transition conditions were checked before state A fired the leave event!");
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

