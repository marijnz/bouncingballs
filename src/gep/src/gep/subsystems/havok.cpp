#include "stdafx.h"
#include "gepimpl/subsystems/havok.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

int gep::HavokErrorHandler::message(hkError::Message msg, int id, const char* description, const char* file, int line)
{
    int result = 1;
    switch(msg)
    {
    case hkError::MESSAGE_REPORT:
        g_globalManager.getLogging()->logMessage("%s(%d): %s", file, line, description);
        result = 0;
        break;
    case hkError::MESSAGE_WARNING:
        g_globalManager.getLogging()->logWarning("[%d] %s(%d): %s", id, file, line, description);
        result = 0;
        break;
    case hkError::MESSAGE_ASSERT:
    case hkError::MESSAGE_ERROR:
        g_globalManager.getLogging()->logError("[%d] %s(%d): %s", id, file, line, description);
        break;
    default:
        break;
    }

    return result;
}

void gep::hk::initialize()
{
#if defined(HK_COMPILER_HAS_INTRINSICS_IA32) && HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
    // Flush all denormal/subnormal numbers (2^-1074 to 2^-1022) to zero.
    // Typically operations on denormals are very slow, up to 100 times slower than normal numbers.
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif

    hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault(
        hkMallocAllocator::m_defaultMallocAllocator,
        hkMemorySystem::FrameInfo(1024 * 1024));
    hkBaseSystem::init(memoryRouter, nullptr);
    hkError::replaceInstance(new HavokErrorHandler());
}

void gep::hk::shutdown()
{
    hkBaseSystem::quit();
    hkMemoryInitUtil::quit();
}
