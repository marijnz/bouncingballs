#pragma once

#include "gep/interfaces/logging.h"

namespace gpp
{
    // Always logs to the std out
    class TestLogging : public gep::ILogging
    {
    public:
        inline static TestLogging& instance()
        {
            static TestLogging defaultInstance;
            return defaultInstance;
        }

        virtual void logMessage(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) override
        {
            char buffer[2048];
            va_list argptr;
            va_start(argptr, fmt);
            vsprintf_s(buffer, fmt, argptr);
            va_end(argptr);

            logFormatted(gep::LogChannel::message, buffer);
        }
        virtual void logWarning(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) override
        {
            char buffer[2048];
            va_list argptr;
            va_start(argptr, fmt);
            vsprintf_s(buffer, fmt, argptr);
            va_end(argptr);

            logFormatted(gep::LogChannel::warning, buffer);
        }
        virtual void logError(GEP_PRINTF_FORMAT_STRING const char* fmt, ...) override
        {
            char buffer[2048];
            va_list argptr;
            va_start(argptr, fmt);
            vsprintf_s(buffer, fmt, argptr);
            va_end(argptr);

            logFormatted(gep::LogChannel::error, buffer);
        }
        virtual void registerSink(gep::ILogSink* pSink) override
        {
        }
        virtual void deregisterSink(gep::ILogSink* pSink) override
        {
        }

    private:

        void logFormatted(gep::LogChannel channel, const char* msg)
        {
            WORD color = 0x07; //white
            switch(channel)
            {
            case gep::LogChannel::message:
                break;
            case gep::LogChannel::warning:
                color = 0x0E; //yellow
                break;
            case gep::LogChannel::error:
                color = 0x0C; // red
                break;
            default:
                GEP_ASSERT(false, "unhandeled log channel value");
                break;
            }
            if(color != 0x07)
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
            printf("%s\n", msg);
            if(color != 0x07)
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07); //back to white
        }
    };
    
}
