#pragma once

#include "gep/threading/mutex.h"

namespace gep
{
    struct NoLockPolicy
    {
        inline void lock() {}
        inline void unlock() {}
    };

    struct MutexLockPolicy
    {
    private:
        Mutex m_mutex;
    public:
        inline void lock() { m_mutex.lock(); }
        inline void unlock() { m_mutex.unlock(); }
    };
}
