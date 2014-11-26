#pragma once

#include "gep/gepmodule.h"

namespace gep
{
    void __cdecl threadEntryFunction(void* pThread);

    class GEP_API Thread
    {
    private:
        void* m_handle;

        friend void __cdecl threadEntryFunction(void* pThread);

    public:
        Thread();
        virtual ~Thread();

        /// \brief starts the thread
        void start();

        /// \brief waits for the thread to finish
        void join();

        /// \brief function which will be called from the new thread
        virtual void run() = 0;
    };
}
