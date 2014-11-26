#pragma once

#include "gep/interfaces/subsystem.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    enum class LogChannel
    {
        message,
        warning,
        error
    };

    class ILogSink
    {
    public:
        virtual ~ILogSink() {}
        virtual void take(LogChannel channel, const char* msg) = 0;
    };

    class ILogging
    {
    public:
        virtual ~ILogging() {}
        virtual void logMessage(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) = 0;
        virtual void logWarning(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) = 0;
        virtual void logError(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) = 0;

        virtual void registerSink(ILogSink* pSink) = 0;
        virtual void deregisterSink(ILogSink* pSink) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(logMessagePreFormatted, "logMessage")
            LUA_BIND_FUNCTION_NAMED(logWarningPreFormatted, "logWarning")
            LUA_BIND_FUNCTION_NAMED(logErrorPreFormatted, "logError")
        LUA_BIND_REFERENCE_TYPE_END

    private:
        inline void logMessagePreFormatted(const char* message) { logMessage(message); }
        inline void logWarningPreFormatted(const char* message) { logWarning(message); }
        inline void logErrorPreFormatted(const char* message) { logError(message); }
    };
}
