#pragma once

#include "gep/interfaces/logging.h"

namespace gpp
{
    class DummyLogging : public gep::ILogging
    {
    public:
        static DummyLogging& instance()
        {
            static DummyLogging defaultInstance;
            return defaultInstance;
        }

        virtual void logMessage(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) override {}
        virtual void logWarning(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) override {}
        virtual void logError(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) override {}

        virtual void registerSink(gep::ILogSink* pSink) override {}
        virtual void deregisterSink(gep::ILogSink* pSink) override {}
    };
}
