#pragma once

#include "gep/interfaces/subsystem.h"

namespace gep
{
    //forward declarations
    class IAllocatorStatistics;

    class IMemoryManager : public ISubsystem
    {
    public:
        /// \brief registers a new allocator with the memory manager
        virtual void registerAllocator(const char* name, IAllocatorStatistics* pAllocator) = 0;

        /// \brief deregisters a allocator with the memory manager
        virtual void deregisterAllocator(IAllocatorStatistics* pAllocator) = 0;
    };
}
