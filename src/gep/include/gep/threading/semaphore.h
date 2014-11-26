#pragma once

#include "gep/gepmodule.h"
#include "gep/common.h"
#include <Windows.h>

namespace gep
{
    /// \brief a semaphore
    class GEP_API Semaphore
    {
    private:
        HANDLE m_handle;

        //non-copyable
        Semaphore(const Semaphore& rh);
        void operator = (const Semaphore& rh);

    public:
        /// \brief creates a new semaphore with a given start count
        Semaphore(uint8 startCount);
        ~Semaphore();

        void waitAndDecrement();
        Result waitAndDecrement(uint32 millisecondsToWait);
        void increment();
    };
}
