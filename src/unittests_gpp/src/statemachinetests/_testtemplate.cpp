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

GEP_UNITTEST_TEST(StateMachine, INSERT_TEST_NAME_HERE)
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

    /// TODO Insert test data here ///////////////////////////////////////////

    // Actual tests. Don't put test data in here that has to be referenced
    // by the state machines, as they will get out of scope when pMainFsm
    // is actually run.
    {
        pMainFsm = factory.create("fsm");
        /// TODO Insert test code here ///////////////////////////////////////////
    }

    pMainFsm->setLogging(&logging);

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 0; frameCount < 20; ++frameCount)
    {
        _updateFramework.run();
    }
}
