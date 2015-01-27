#include "stdafx.h"
#include "gep/memory/allocator.h"
#include "gep/threading/mutex.h"
#include "gep/exit.h"
#include "gep/memory/leakDetection.h"
#include <fstream>

#include "gep/memory/newdelete.inl"

#ifdef TRACK_MEMORY_LEAKS
volatile gep::IAllocatorStatistics* gep::StdAllocator::s_globalInstance = nullptr;
#else
volatile gep::StdAllocator* gep::StdAllocator::s_globalInstance = nullptr;
#endif

gep::Mutex gep::StdAllocator::s_creationMutex;

void* gep::StdAllocator::allocateMemory(size_t size)
{
    ScopedLock<Mutex> lock(m_allocationLock);
    m_bytesAllocated += size;
    if(m_bytesAllocated > m_peakBytesAllocated)
        m_peakBytesAllocated = m_bytesAllocated;
    m_numAllocations++;
    return malloc(size);
}

void gep::StdAllocator::freeMemory(void* mem)
{
    ScopedLock<Mutex> lock(m_allocationLock);
    if(mem != nullptr)
    {
        ++m_numFrees;
        m_bytesAllocated -= _msize(mem);
        free(mem);
    }
}

size_t gep::StdAllocator::getNumAllocations() const
{
    return m_numAllocations;
}

size_t gep::StdAllocator::getNumFrees() const
{
    return m_numFrees;
}

size_t gep::StdAllocator::getNumBytesReserved() const
{
    return m_peakBytesAllocated;
}

size_t gep::StdAllocator::getNumBytesUsed() const
{
    return m_bytesAllocated;
}

gep::IAllocator* gep::StdAllocator::getParentAllocator() const
{
    return nullptr;
}

#ifdef TRACK_MEMORY_LEAKS
gep::IAllocatorStatistics& gep::StdAllocator::globalInstance()
#else
gep::StdAllocator& gep::StdAllocator::globalInstance()
#endif
{
    // double locking pattern
    // this is NOT an anti pattern if you know what the pitfalls are
    if(s_globalInstance == nullptr)
    {
        ScopedLock<Mutex> lock(s_creationMutex);
        if(s_globalInstance == nullptr)
        {
            static char allocatorInstanceMemory[sizeof(StdAllocator)];
            StdAllocator* stdAllocator = new(allocatorInstanceMemory) StdAllocator();

            #ifdef TRACK_MEMORY_LEAKS
            static char leakDetectorMemory[sizeof(LeakDetectorAllocatorStatistics)];
            s_globalInstance = new(leakDetectorMemory) LeakDetectorAllocatorStatistics(stdAllocator);
            #else
            s_globalInstance = stdAllocator;
            #endif

            auto result = gep::atexit(&destroyInstance);
            GEP_ASSERT(result == SUCCESS, "registering exit function failed");
        }
    }
    GEP_ASSERT(s_globalInstance != nullptr);
    return (StdAllocator&)(*s_globalInstance);
}

void gep::StdAllocator::destroyInstance()
{
    ScopedLock<Mutex> lock(s_creationMutex);
    if(s_globalInstance != nullptr)
    {
        #ifdef TRACK_MEMORY_LEAKS
        auto temp = (LeakDetectorAllocatorStatistics*)s_globalInstance;
        auto wrapped = (StdAllocator*)temp->getWrapped();
        {
            // Will empty the file if it already existed.
            std::ofstream leakfile("memoryLeaks.log", std::ios_base::trunc);
            if(temp->hasLeaks())
            {
                temp->findLeaks(leakfile);
                GEP_ASSERT(false, "you have memory leaks, check 'memoryLeaks.log' for details");
            }
            else
            {
                leakfile << "No leaks detected." << std::endl;
            }
        }
        temp->~LeakDetectorAllocatorStatistics();
        wrapped->~StdAllocator();
        #else
        auto temp = s_globalInstance;
        s_globalInstance = nullptr;
        temp->~StdAllocator();
        #endif
    }
}

gep::IAllocatorStatistics* gep::StdAllocatorPolicy::getAllocator()
{
    return &StdAllocator::globalInstance();
}

gep::IAllocator* gep::MallocAllocatorPolicy::getAllocator()
{
    static MallocAllocator allocator;
    return &allocator;
}

gep::IAllocatorStatistics* gep::MallocAllocatorStatisticsPolicy::getAllocator()
{
    static MallocAllocatorStatistics allocator;
    return &allocator;
}
