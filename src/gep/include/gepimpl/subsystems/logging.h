#pragma once

#include "gep/interfaces/logging.h"
#include "gep/container/DynamicArray.h"
#include "gep/threading/mutex.h"

namespace gep
{
    class Logging
        : public ILogging
    {
    private:
        DynamicArray<ILogSink*> m_sinks;
        Mutex m_sinkMutex;

    public:
        Logging();
        ~Logging();

        virtual void logMessage(const char* fmt, ...) override;
        virtual void logWarning(const char* fmt, ...) override;
        virtual void logError(const char* fmt, ...) override;

        virtual void registerSink(ILogSink* pSink) override;
        virtual void deregisterSink(ILogSink* pSink) override;

    };

    class ConsoleLogSink
         : public ILogSink
    {
    public:
        virtual void take(LogChannel channel, const char* msg) override;
    };

    class FileLogSink
         : public ILogSink
    {
    private:
        std::ofstream m_LogFile;
        bool m_autoFlush;

    public:
        FileLogSink(const char* filename, bool autoFlush = true);
        virtual ~FileLogSink();
        virtual void take(LogChannel channel, const char* msg) override;
    };
}
