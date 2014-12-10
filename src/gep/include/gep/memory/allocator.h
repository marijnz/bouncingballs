#pragma once

#include "gep/threading/mutex.h"
#include <vector>

#ifdef _DEBUG
#define TRACK_MEMORY_LEAKS
#endif

namespace gep
{
    /// \brief generic allocator interface
    class IAllocator
    {
    public:
        virtual void* allocateMemory(size_t size) = 0;
        virtual void freeMemory(void* mem) = 0;
    };

    /// \brief interface for a allocator which keeps statistics
    class IAllocatorStatistics : public IAllocator
    {
    public:
        /// \brief returns the number of allocations this allocator did so far
        virtual size_t getNumAllocations() const = 0;

        /// \brief returns the number of free operations this allocator did so far
        virtual size_t getNumFrees() const = 0;

        /// \brief returns the amount of memory that is reserved by this allocator
        virtual size_t getNumBytesReserved() const = 0;

        /// \brief returns the amount of memory that is actually used by alive allocations
        virtual size_t getNumBytesUsed() const = 0;

        /// \brief returns the allocator this allocator takes the memory from,
        ///  nullptr if it does not obtain the memory from another allocator
        virtual IAllocator* getParentAllocator() const = 0;
    };

    /// \brief standard allocator
    class GEP_API StdAllocator : public IAllocatorStatistics
    {
    private:
        #ifdef TRACK_MEMORY_LEAKS
        static volatile IAllocatorStatistics* s_globalInstance;
        #else
        static volatile StdAllocator* s_globalInstance;
        #endif

        static Mutex s_creationMutex;

        size_t m_numAllocations;
        size_t m_numFrees;
        size_t m_bytesAllocated;
        size_t m_peakBytesAllocated;

        Mutex m_allocationLock;

        StdAllocator(){}
        ~StdAllocator(){}
    public:
        // IAllocator interface
        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;

        // IAllocatorStatistics Interface
        virtual size_t getNumAllocations() const override;
        virtual size_t getNumFrees() const override;
        virtual size_t getNumBytesReserved() const override;
        virtual size_t getNumBytesUsed() const override;
        virtual IAllocator* getParentAllocator() const override;

        #ifdef TRACK_MEMORY_LEAKS
        /// \brief returns the only instance of this class
        static IAllocatorStatistics& globalInstance();
        #else
        /// \brief returns the only instance of this class
        static StdAllocator& globalInstance(); //not using DoubleLockingSingelton because of cyclic dependency
        #endif
        static void destroyInstance();
    };

    /// \brief standard allocation policy
    struct StdAllocatorPolicy
    {
        GEP_API static IAllocatorStatistics* getAllocator();
    };

    struct MallocAllocatorPolicy
    {
        GEP_API static IAllocator* getAllocator();
        GEP_API static IAllocatorStatistics* getAllocatorStatistics();
    };

    struct MallocAllocatorStatisticsPolicy
    {
        GEP_API static IAllocatorStatistics* getAllocator();
    };
}

#define g_stdAllocator (::gep::StdAllocator::globalInstance())

#include "gep/ReferenceCounting.h"
#include "gep/ArrayPtr.h"

namespace gep
{
    template <class T, class A>
    inline void deleteHelper(T* ptr, A& allocator)
    {
        ptr->~T();
        allocator.freeMemory(ptr);
    }

    template <class T, class A>
    inline void deleteHelper(T* ptr, A* allocator)
    {
        ptr->~T();
        allocator->freeMemory(ptr);
    }

    template <class T, class A>
    inline void deleteArrayHelper(ArrayPtr<T> array, A& allocator)
    {
        MemoryUtils::destroy(array.getPtr(), array.length());
        allocator.freeMemory(array.getPtr());
    }

    template <class T, class A>
    inline void deleteArrayHelper(ArrayPtr<T> array, A* allocator)
    {
        MemoryUtils::destroy(array.getPtr(), array.length());
        allocator->freeMemory(array.getPtr());
    }

    template <class T, class A, bool isRefCounted>
    struct NewHelper
    {
        static void* newHelper(A* pAllocator)
        {
            GEP_ASSERT(pAllocator != nullptr);
            return pAllocator->allocateMemory(sizeof(T));
        }

        static void* newHelper(A& pAllocator)
        {
            return pAllocator.allocateMemory(sizeof(T));
        }

        static ArrayPtr<T> newArray(A* pAllocator, size_t length)
        {
            GEP_ASSERT(pAllocator != nullptr);
            if(length == 0)
                return ArrayPtr<T>();
            ArrayPtr<T> result(static_cast<T*>(pAllocator->allocateMemory(sizeof(T) * length)), length);
            MemoryUtils::uninitializedConstruct(result.getPtr(), result.length());
            return result;
        }

        static ArrayPtr<T> newArray(A& pAllocator, size_t length)
        {
            if(length == 0)
                return ArrayPtr<T>();
            ArrayPtr<T> result(static_cast<T*>(pAllocator.allocateMemory(sizeof(T) * length)), length);
            MemoryUtils::uninitializedConstruct(result.getPtr(), result.length());
            return result;
        }
    };

    template <class T, class A>
    struct NewHelper<T, A, true>
    {
        static void* newHelper(A* pAllocator)
        {
            GEP_ASSERT(pAllocator != nullptr);
            return newHelper(*pAllocator);
        }

        static void* newHelper(A& allocator)
        {
            auto pMem = allocator.allocateMemory(sizeof(T));
#ifdef _DEBUG
            memset(pMem, 0, sizeof(T));
#endif // _DEBUG
            ReferenceCounted* pObj = static_cast<ReferenceCounted*>(static_cast<T*>(pMem));
            pObj->setAllocator(&allocator);
            return pObj;
        }

        static ArrayPtr<T> newArray(A* pAllocator, size_t length)
        {
            GEP_ASSERT(pAllocator != nullptr);
            ArrayPtr<T> result(static_cast<T*>(pAllocator->allocateMemory(sizeof(T) * length)), length);
            MemoryUtils::uninitializedConstruct(result.getPtr(), result.length());
            for(auto e : result)
                result->setAllocator(pAllocator);
            return result;
        }

        static ArrayPtr<T> newArray(A& pAllocator, size_t length)
        {
            ArrayPtr<T> result(static_cast<T*>(pAllocator.allocateMemory(sizeof(T) * length)), length);
            MemoryUtils::uninitializedConstruct(result.getPtr(), result.length());
            for(auto e : result)
                result->setAllocator(&pAllocator);
            return result;
        }
    };
}

#define GEP_NEW(allocator, T) new (gep::NewHelper<T, typename std::remove_reference<typename std::remove_pointer<decltype(allocator)>::type>::type, std::is_convertible<T*, gep::ReferenceCounted*>::value>::newHelper(allocator)) T
#define GEP_NEW_ARRAY(allocator, T, length) gep::NewHelper<T, typename std::remove_reference<typename std::remove_pointer<decltype(allocator)>::type>::type, std::is_convertible<T*, gep::ReferenceCounted*>::value>::newArray(allocator, length)
#define GEP_DELETE(allocator, ptr) { gep::deleteHelper(ptr, allocator); ptr = nullptr; }
#define GEP_DELETE_ARRAY(allocator, array) { gep::deleteArrayHelper(array, allocator); array = nullptr; }
namespace gep
{
    template<typename T>
    inline void deleteAndNull(T*& ptr){ delete ptr; ptr = nullptr; }
}
#define DELETE_AND_NULL(ptr) ::gep::deleteAndNull((ptr));

// Overwrite global new delete
#include <new>

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* mem);
void operator delete[](void* mem);
