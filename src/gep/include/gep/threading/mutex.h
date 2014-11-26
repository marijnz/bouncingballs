#pragma once

#include "gep/gepmodule.h"
#include "gep/common.h"
#include <Windows.h>

namespace gep
{
    /// \brief a mutex for thread synchronisation
    class GEP_API Mutex
    {
    private:
        CRITICAL_SECTION m_criticalSection;
    public:
        Mutex();
        ~Mutex();

        /// \brief locks the mutex
        void lock();

        /// \brief unlocks the mutex
        void unlock();

        /// \brief tries to lock a critical section
        /// \return SUCCESS if locked, FAILURE otherwise
        Result tryLock();
    };

    /// \brief locks the given type during its lifetime (scope)
    template <class T>
    struct ScopedLock
    {
    private:
        T& m_lockable;
    public:
        ScopedLock(T& lockable) : m_lockable(lockable)
        {
            m_lockable.lock();
        }

        ~ScopedLock()
        {
            m_lockable.unlock();
        }
    };
}
