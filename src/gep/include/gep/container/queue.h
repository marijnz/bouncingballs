#pragma once

#include "gep/container/DynamicArray.h"
#include "gep/container/policies.h"

namespace gep
{
    template <class T, class LockingPolicy = NoLockPolicy, class AllocatorPolicy = StdAllocatorPolicy>
    class Queue
    {
    private:
        DynamicArray<T, AllocatorPolicy> m_data;
        LockingPolicy m_lock;

        size_t m_insertIndex;
        size_t m_takeIndex;

    public:
        Queue() :
            m_insertIndex(0),
            m_takeIndex(0)
        {
            m_data.resize(4);
        }

        Queue(IAllocator* allocator) :
            m_data(allocator),
            m_lock(),
            m_insertIndex(0),
            m_takeIndex(0)
        {
            m_data.resize(4);
        }

        /// \brief takes a element from the beginning of the queue
        T take()
        {
            ScopedLock<LockingPolicy> slock(m_lock);
            GEP_ASSERT(count() > 0, "no more elements left in the queue");
            T result(m_data[m_takeIndex]);
            m_data[m_takeIndex] = T();
            m_takeIndex = (m_takeIndex + 1) % m_data.length();
            return result;
        }

        /// \brief appends a new element at the end of the queue
        void append(T& val)
        {
            ScopedLock<LockingPolicy> slock(m_lock);
            if(m_takeIndex > m_insertIndex)
            {
                GEP_ASSERT(m_takeIndex - m_insertIndex >= 1);
                if(m_takeIndex - m_insertIndex == 1)
                {
                    m_data.insertAtIndex(m_insertIndex, val);
                    m_insertIndex++;
                    m_takeIndex++;
                }
                else
                {
                    m_data[m_insertIndex] = val;
                    m_insertIndex++; // no overflow possible
                }
            }
            else
            {
                if(m_insertIndex == m_data.length() - 1 && m_takeIndex == 0)
                {
                    m_data.insertAtIndex(m_insertIndex, val);
                    m_insertIndex++;
                }
                else
                {
                    m_data[m_insertIndex] = val;
                    m_insertIndex = (m_insertIndex + 1) % m_data.length();
                }
            }
        }

        /// \brief returns the number of elements remaining in the queue
        size_t count()
        {
            ScopedLock<LockingPolicy> slock(m_lock);
            size_t insertIndex = m_insertIndex;
            if(insertIndex < m_takeIndex)
                insertIndex += m_data.length();
            GEP_ASSERT(insertIndex >= m_takeIndex);
            return insertIndex - m_takeIndex;
        }

    };
}
