#pragma once
#pragma warning( disable : 4251 )

#include "gep/memory/allocator.h"
#include "gep/container/DynamicArray.h"
#include "gep/singleton.h"

namespace gep
{
    /// \brief a simple leak checking allocator
    class GEP_API SimpleLeakCheckingAllocator : public IAllocatorStatistics,
        public DoubleLockingSingleton<SimpleLeakCheckingAllocator>
    {
        // the singelton template needs to be friend so it can create this class
        friend class DoubleLockingSingleton<SimpleLeakCheckingAllocator>;
    private:
        size_t m_allocCount;
        size_t m_freeCount;

        SimpleLeakCheckingAllocator();
        ~SimpleLeakCheckingAllocator();

    public:
        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;

        // IAllocatorStatistics Interface
        virtual size_t getNumAllocations() const override;
        virtual size_t getNumFrees() const override;
        virtual size_t getNumBytesReserved() const override;
        virtual size_t getNumBytesUsed() const override;
        virtual IAllocator* getParentAllocator() const override;

        inline size_t getAllocCount() const { return m_allocCount; }
        inline size_t getFreeCount() const { return m_freeCount; }
    };

    /// \brief allocator policy for simple leak checking allocator
    struct SimpleLeakCheckingAllocatorPolicy
    {
        GEP_API static SimpleLeakCheckingAllocator* getAllocator();
    };

    /// \brief alignemt helper
    struct AlignmentHelper
    {
        static const size_t __ALIGNMENT = sizeof(void*);
        static size_t __alignedSize(size_t s);
        static bool __isAligned(char* p);
    };

    /// \brief pool allocator
    // TODO Add locking policy
    class GEP_API PoolAllocator : public IAllocatorStatistics
    {
    private:
        char*            m_pBuffer;
        size_t            m_size;

        size_t            m_chunkSize;
        size_t            m_numChunks;
        size_t            m_allocatedChunks;

        int*            m_pFreeList;
        int                m_nextFreeIdx;

        size_t            m_numAllocations;
        size_t            m_numFrees;

        IAllocator*        m_pParentAllocator;

        static const int    INVALID        = -1;

        // not accessible
        PoolAllocator(){}
        PoolAllocator(const PoolAllocator& other){}
        PoolAllocator(PoolAllocator&& other){}

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

        PoolAllocator(size_t chunkSize, size_t numChuks, IAllocator* pParentAllocator = nullptr);
        ~PoolAllocator();

        // returns the size of the free list in bytes
        size_t getFreeListSize() const;
    };

    /// \brief stack allocator
    // TODO Add locking policy
    class GEP_API StackAllocator : public IAllocatorStatistics
    {
        friend class DoubleEndedStackAllocator;
    private:
        char*            m_pBuffer;
        size_t            m_size;

        bool            m_front;

        DynamicArray<size_t>    m_allocations;
        char*                    m_pStackPtr;

        size_t            m_numAllocations;
        size_t            m_numFrees;

        IAllocator*        m_pParentAllocator;

        // not accessible
        StackAllocator(){}
        StackAllocator(const StackAllocator& other){}
        StackAllocator(StackAllocator&& other){}

    public:
        // IAllocator interface
        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;

        // frees multiple allocations on the stack at once
        void freeToMarker(void* mem);
        inline void* getMarker()
        {
            return m_pStackPtr;
        }

        // IAllocatorStatistics Interface
        virtual size_t getNumAllocations() const override;
        virtual size_t getNumFrees() const override;
        virtual size_t getNumBytesReserved() const override;
        virtual size_t getNumBytesUsed() const override;
        virtual IAllocator* getParentAllocator() const override;

        StackAllocator(bool front, size_t size, IAllocator* pParentAllocator = nullptr);
        StackAllocator(bool front, size_t size, char* pBuffer);
        ~StackAllocator();

        // returns the memory reserved by the internally used dynamic array
        size_t getDynamicArraySize() const;
    };

    /// \brief stack allocator proxy used by double ended stack allocator
    class GEP_API StackAllocatorProxy : IAllocator
    {
        friend class DoubleEndedStackAllocator;
    private:
        DoubleEndedStackAllocator* m_pDoubleEndedStackAllocator;
        StackAllocator m_StackAllocator;

        StackAllocatorProxy();
        StackAllocatorProxy(bool front, size_t size, IAllocator* pParentAllocator);
        StackAllocatorProxy(bool front, size_t size, char* pBuffer);
    public:
        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;
    };

    /// \brief double ended stack allocator
    class GEP_API DoubleEndedStackAllocator : public IAllocatorStatistics
    {
        friend class StackAllocatorProxy;
    private:
        StackAllocatorProxy m_Front;
        StackAllocatorProxy m_Back;

        bool checkStackOverlapping(size_t allocSize) const;

        // not accessible
        DoubleEndedStackAllocator(){}
        DoubleEndedStackAllocator(const DoubleEndedStackAllocator& other){}
        DoubleEndedStackAllocator(DoubleEndedStackAllocator&& other){}

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

        // stack allocator proxies
        StackAllocatorProxy* getFront();
        StackAllocatorProxy* getBack();

        DoubleEndedStackAllocator(size_t size, IAllocator* pParentAllocator = nullptr);
        ~DoubleEndedStackAllocator();

        // returns the memory reserved by the internally used dynamic arrays of both stack allocators
        size_t getDynamicArraysSize() const;
    };
}

#define ALIGNMENT AlignmentHelper::__ALIGNMENT
#define alignedSize AlignmentHelper::__alignedSize
#define isAligned AlignmentHelper::__isAligned

#define g_simpleLeakCheckingAllocator gep::SimpleLeakCheckingAllocator::instance()
