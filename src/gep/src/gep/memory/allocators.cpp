#include "stdafx.h"
#include "gep/memory/allocators.h"

gep::SimpleLeakCheckingAllocator* volatile gep::DoubleLockingSingleton<gep::SimpleLeakCheckingAllocator>::s_instance = nullptr;
gep::Mutex gep::DoubleLockingSingleton<gep::SimpleLeakCheckingAllocator>::s_creationMutex;

gep::SimpleLeakCheckingAllocator::SimpleLeakCheckingAllocator()
{
    m_allocCount = 0;
    m_freeCount = 0;
}

gep::SimpleLeakCheckingAllocator::~SimpleLeakCheckingAllocator()
{
    GEP_ASSERT(m_allocCount == m_freeCount, "You have memory leaks", m_allocCount, m_freeCount);
}

void* gep::SimpleLeakCheckingAllocator::allocateMemory(size_t size)
{
    m_allocCount++;
    return StdAllocator::globalInstance().allocateMemory(size);
}

void gep::SimpleLeakCheckingAllocator::freeMemory(void* mem)
{
    if(mem != nullptr)
        m_freeCount++;
    return StdAllocator::globalInstance().freeMemory(mem);
}

gep::SimpleLeakCheckingAllocator* gep::SimpleLeakCheckingAllocatorPolicy::getAllocator()
{
    return &SimpleLeakCheckingAllocator::instance();
}

size_t gep::SimpleLeakCheckingAllocator::getNumAllocations() const
{
    return StdAllocator::globalInstance().getNumAllocations();
}

size_t gep::SimpleLeakCheckingAllocator::getNumFrees() const
{
    return StdAllocator::globalInstance().getNumFrees();
}

size_t gep::SimpleLeakCheckingAllocator::getNumBytesReserved() const
{
    return StdAllocator::globalInstance().getNumBytesReserved();
}

size_t gep::SimpleLeakCheckingAllocator::getNumBytesUsed() const
{
    return StdAllocator::globalInstance().getNumBytesUsed();
}

gep::IAllocator* gep::SimpleLeakCheckingAllocator::getParentAllocator() const
{
    return StdAllocator::globalInstance().getParentAllocator();
}

size_t gep::AlignmentHelper::__alignedSize(size_t s)
{
    return ((s + __ALIGNMENT-1) / __ALIGNMENT) * __ALIGNMENT;
}

bool gep::AlignmentHelper::__isAligned(char* p)
{
    return 0 == (reinterpret_cast<uintptr_t>(p) % __ALIGNMENT);
}

void* gep::PoolAllocator::allocateMemory(size_t size)
{
    GEP_ASSERT(size>0);
    GEP_ASSERT(size<=m_chunkSize);
    if (m_nextFreeIdx!=INVALID)
    {
        char* pBuffer = m_pBuffer + (m_nextFreeIdx * m_chunkSize);
        int nextFreeIdx = m_pFreeList[m_nextFreeIdx];
        m_pFreeList[m_nextFreeIdx] = INVALID;
        m_nextFreeIdx = nextFreeIdx;
        ++m_numAllocations;
        ++m_allocatedChunks;
        GEP_ASSERT(m_allocatedChunks<=m_numChunks);
        return pBuffer;
    }
    return nullptr;
}

void gep::PoolAllocator::freeMemory(void* mem)
{
    if (mem!=nullptr)
    {
        GEP_ASSERT(mem>=m_pBuffer && mem<=(m_pBuffer+(m_chunkSize*(m_numChunks-1))));
        GEP_ASSERT(m_allocatedChunks>0);
        int chunkIdx = static_cast<int>((reinterpret_cast<char*>(mem)-m_pBuffer) / m_chunkSize);
        GEP_ASSERT(m_pFreeList[chunkIdx]==INVALID);
        m_pFreeList[chunkIdx] = m_nextFreeIdx;
        m_nextFreeIdx = chunkIdx;
        ++m_numFrees;
        --m_allocatedChunks;
    }
}

size_t gep::PoolAllocator::getNumAllocations() const
{
    return m_numAllocations;
}

size_t gep::PoolAllocator::getNumFrees() const
{
    return m_numFrees;
}

size_t gep::PoolAllocator::getNumBytesReserved() const
{
    return m_size + m_numChunks*sizeof(int);
}

size_t gep::PoolAllocator::getNumBytesUsed() const
{
    return m_chunkSize * m_allocatedChunks;
}

gep::IAllocator* gep::PoolAllocator::getParentAllocator() const
{
    return m_pParentAllocator;
}

gep::PoolAllocator::PoolAllocator(size_t chunkSize, size_t numChunks, IAllocator* pParentAllocator)
{
    if (pParentAllocator==nullptr)
        pParentAllocator = &StdAllocator::globalInstance();
    m_pParentAllocator = pParentAllocator;

    GEP_ASSERT(chunkSize>0);
    m_chunkSize = alignedSize(chunkSize);

    GEP_ASSERT(numChunks>0 && numChunks <= (size_t)std::numeric_limits<int>::max());
    m_numChunks = numChunks;
    m_allocatedChunks = 0;

    m_size = m_chunkSize * m_numChunks;
    m_pBuffer = (char*)m_pParentAllocator->allocateMemory(m_size);
    GEP_ASSERT(m_pBuffer!=nullptr);
    GEP_ASSERT(isAligned(m_pBuffer));
#ifdef _DEBUG
    memset(m_pBuffer, 0, m_size);
#endif

    m_pFreeList = (int*)m_pParentAllocator->allocateMemory(m_numChunks*sizeof(int));
    for (size_t i=0; i < m_numChunks-1; ++i)
        m_pFreeList[i] = (int)(i+1);
    m_pFreeList[m_numChunks-1] = INVALID;
    m_nextFreeIdx = 0;

    m_numAllocations = 0;
    m_numFrees = 0;
}

gep::PoolAllocator::~PoolAllocator()
{
    m_pParentAllocator->freeMemory(m_pBuffer);
    m_pParentAllocator->freeMemory(m_pFreeList);
}

size_t gep::PoolAllocator::getFreeListSize() const
{
    return m_numChunks * sizeof(int);
}

void* gep::StackAllocator::allocateMemory(size_t size)
{
    GEP_ASSERT(size>0);
    size = alignedSize(size);
    if (m_front)
    {
        if (m_pStackPtr+size <= m_pBuffer+m_size)
        {
            char* pBuffer = m_pStackPtr;
            m_pStackPtr += size;
            m_allocations.append(size);
            ++m_numAllocations;
            return pBuffer;
        }
    }
    else
    {
        if (m_pStackPtr-size >= m_pBuffer)
        {
            m_pStackPtr -= size;
            char* pBuffer = m_pStackPtr;
            m_allocations.append(size);
            ++m_numAllocations;
            return pBuffer;
        }
    }
    return nullptr;
}

void gep::StackAllocator::freeMemory(void* mem)
{
    if (mem!=nullptr)
    {
        size_t size = m_allocations[m_allocations.length()-1];
        char* pBuffer = reinterpret_cast<char*>(mem);
        if (m_front)
        {
            GEP_ASSERT(m_pStackPtr-size == pBuffer);
            m_pStackPtr -= size;
            m_allocations.removeAtIndex(m_allocations.length()-1);
            ++m_numFrees;
        }
        else
        {
            GEP_ASSERT(m_pStackPtr == pBuffer);
            m_pStackPtr += size;
            m_allocations.removeAtIndex(m_allocations.length()-1);
            ++m_numFrees;
        }
    }
}

void gep::StackAllocator::freeToMarker(void* mem)
{
    if (mem!=nullptr)
    {
        // try to find the marker
        char* pMarker = (m_front ? m_pStackPtr : m_pStackPtr-m_allocations[m_allocations.length()-1]);
        size_t numFrees = 0;
        for (intptr_t a=m_allocations.length()-1; a>=0; --a)
        {
            pMarker = (m_front ? pMarker-m_allocations[a] : pMarker+m_allocations[a]);
            ++numFrees;
            if (pMarker==mem)
            {
                m_pStackPtr = (m_front ? pMarker : pMarker+m_allocations[a]);
                for (size_t f=0; f<numFrees; ++f)
                    m_allocations.removeAtIndex(m_allocations.length()-1);
                m_numFrees += numFrees;
                return;
            }
        }
        GEP_ASSERT(0, "marker not found");
    }
}

size_t gep::StackAllocator::getNumAllocations() const
{
    return m_numAllocations;
}

size_t gep::StackAllocator::getNumFrees() const
{
    return m_numFrees;
}

size_t gep::StackAllocator::getNumBytesReserved() const
{
    return m_size + m_allocations.reserved()*sizeof(size_t);
}

size_t gep::StackAllocator::getNumBytesUsed() const
{
    size_t used = 0;
    for (size_t i=0; i<m_allocations.length(); ++i)
        used += m_allocations[i];
    return used;
}

gep::IAllocator* gep::StackAllocator::getParentAllocator() const
{
    return m_pParentAllocator;
}

gep::StackAllocator::StackAllocator(bool front, size_t size, IAllocator* pParentAllocator)
{
    m_front = front;

    if (pParentAllocator==nullptr)
        pParentAllocator = &StdAllocator::globalInstance();
    m_pParentAllocator = pParentAllocator;

    GEP_ASSERT(size>0);
    m_size = alignedSize(size);

    m_pBuffer = (char*)m_pParentAllocator->allocateMemory(m_size);
    GEP_ASSERT(m_pBuffer!=nullptr);
    GEP_ASSERT(isAligned(m_pBuffer));
#ifdef _DEBUG
    memset(m_pBuffer, 0, m_size);
#endif

    if (m_front)
        m_pStackPtr = m_pBuffer;
    else
        m_pStackPtr = m_pBuffer + m_size;

    m_numAllocations = 0;
    m_numFrees = 0;
}

gep::StackAllocator::StackAllocator(bool front, size_t size, char* pBuffer)
{
    m_front = front;

    m_pParentAllocator = nullptr;

    GEP_ASSERT(size>0);
    GEP_ASSERT(size%ALIGNMENT==0);
    m_size = size;

    GEP_ASSERT(pBuffer!=nullptr)
    GEP_ASSERT(isAligned(pBuffer));
    m_pBuffer = pBuffer;

    if (m_front)
        m_pStackPtr = m_pBuffer;
    else
        m_pStackPtr = m_pBuffer + m_size;

    m_numAllocations = 0;
    m_numFrees = 0;
}

gep::StackAllocator::~StackAllocator()
{
    if (m_pParentAllocator!=nullptr)
        m_pParentAllocator->freeMemory(m_pBuffer);
}

size_t gep::StackAllocator::getDynamicArraySize() const
{
    return m_allocations.reserved()*sizeof(size_t);
}

gep::StackAllocatorProxy::StackAllocatorProxy()
    : m_StackAllocator(true, 0, (char*)nullptr)
{
}

gep::StackAllocatorProxy::StackAllocatorProxy(bool front, size_t size, IAllocator* pParentAllocator)
    : m_StackAllocator(front, size, pParentAllocator)
{
}

gep::StackAllocatorProxy::StackAllocatorProxy(bool front, size_t size, char* pBuffer)
    : m_StackAllocator(front, size, pBuffer)
{
}

void* gep::StackAllocatorProxy::allocateMemory(size_t size)
{
    if (m_pDoubleEndedStackAllocator->checkStackOverlapping(size))
        return nullptr;
    return m_StackAllocator.allocateMemory(size);
}

void gep::StackAllocatorProxy::freeMemory(void* mem)
{
    m_StackAllocator.freeMemory(mem);
}

bool gep::DoubleEndedStackAllocator::checkStackOverlapping(size_t allocSize) const
{
    return allocSize > static_cast<size_t>(m_Back.m_StackAllocator.m_pStackPtr - m_Front.m_StackAllocator.m_pStackPtr);
}

void* gep::DoubleEndedStackAllocator::allocateMemory(size_t size)
{
    return m_Front.allocateMemory(size);
}

void gep::DoubleEndedStackAllocator::freeMemory(void* mem)
{
    m_Front.freeMemory(mem);
}

size_t gep::DoubleEndedStackAllocator::getNumAllocations() const
{
    return m_Front.m_StackAllocator.getNumAllocations() + m_Back.m_StackAllocator.getNumAllocations();
}

size_t gep::DoubleEndedStackAllocator::getNumFrees() const
{
    return m_Front.m_StackAllocator.getNumFrees() + m_Back.m_StackAllocator.getNumFrees();
}

size_t gep::DoubleEndedStackAllocator::getNumBytesReserved() const
{
    return m_Front.m_StackAllocator.getNumBytesReserved() + m_Back.m_StackAllocator.getDynamicArraySize();
}

size_t gep::DoubleEndedStackAllocator::getNumBytesUsed() const
{
    return m_Front.m_StackAllocator.getNumBytesUsed() + m_Back.m_StackAllocator.getNumBytesUsed();
}

gep::IAllocator* gep::DoubleEndedStackAllocator::getParentAllocator() const
{
    return m_Front.m_StackAllocator.getParentAllocator();
}

gep::StackAllocatorProxy* gep::DoubleEndedStackAllocator::getFront()
{
    return &m_Front;
}

gep::StackAllocatorProxy* gep::DoubleEndedStackAllocator::getBack()
{
    return &m_Back;
}

gep::DoubleEndedStackAllocator::DoubleEndedStackAllocator(size_t size, IAllocator* pParentAllocator)
    : m_Front(true, size, pParentAllocator), m_Back(false, size, m_Front.m_StackAllocator.m_pBuffer)
{
    m_Front.m_pDoubleEndedStackAllocator = this;
    m_Back.m_pDoubleEndedStackAllocator = this;
}

gep::DoubleEndedStackAllocator::~DoubleEndedStackAllocator()
{
    // nothing to do here
}

size_t gep::DoubleEndedStackAllocator::getDynamicArraysSize() const
{
    return m_Front.m_StackAllocator.getDynamicArraySize() + m_Back.m_StackAllocator.getDynamicArraySize();
}
