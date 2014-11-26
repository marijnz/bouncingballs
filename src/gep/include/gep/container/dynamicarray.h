#pragma once

#include "gep/memory/MemoryUtils.h"
#include "gep/memory/allocator.h"
#include "gep/ArrayPtr.h"

namespace gep
{

    /// \brief a resizeable array
    template <class T>
    struct DynamicArrayImpl
    {
    private:
        T* m_data;
        size_t m_length;
        size_t m_reserved;
        IAllocator* m_pAllocator;

        inline void copy(const DynamicArrayImpl<T>& other)
        {
            m_data = (T*)m_pAllocator->allocateMemory(sizeof(T) * other.m_length);
            m_length = other.m_length;
            MemoryUtils::uninitializedCopy(m_data, other.m_data, other.m_length);
            m_reserved = m_length;
        }

        inline void move(DynamicArrayImpl<T>& other)
        {
            m_data = other.m_data;
            m_length = other.m_length;
            m_reserved = other.m_reserved;
            m_pAllocator = other.m_pAllocator;
            other.m_data = nullptr;
            other.m_length = 0;
            other.m_reserved = 0;
        }

        inline void destroy()
        {
            MemoryUtils::destroy(m_data, m_length);
            m_pAllocator->freeMemory(m_data);
            m_data = nullptr;
            m_length = 0;
            m_reserved = 0;
        }


    public:
        /// \brief constructor
        ///
        /// \param allocator
        ///   the allocator to be used. May not be null
        DynamicArrayImpl(IAllocator* pAllocator)
            : m_data(nullptr),
            m_length(0),
            m_reserved(0),
            m_pAllocator(pAllocator)
        {
            GEP_ASSERT(m_pAllocator != nullptr, "pAllocator can not be null!");
        }

        /// \brief copy constructor
        DynamicArrayImpl(const DynamicArrayImpl<T>& other)
        {
            m_pAllocator = other.m_pAllocator;
            copy(other);
        }

        /// \brief move constructor
        DynamicArrayImpl(DynamicArrayImpl<T>&& other)
        {
            move(other);
        }

        /// \brief constructor with inital data
        DynamicArrayImpl(IAllocator* pAllocator, const ArrayPtr<T>& data)
            : m_data(nullptr),
            m_length(0),
            m_reserved(0),
            m_pAllocator(pAllocator)
        {
            reserve(data.length());
            MemoryUtils::uninitializedCopy(m_data, data.getPtr(), data.length());
            m_length = data.length();
        }

        /// \brief destructor
        ~DynamicArrayImpl()
        {
            destroy();
        }

        /// \brief copy assignment
        DynamicArrayImpl<T>& operator = (const DynamicArrayImpl<T>& rh)
        {
            if(this == &rh) //avoid self assignment
                return *this;
            destroy();
            copy(rh);
            return *this;
        }

        /// \brief move assignment
        DynamicArrayImpl<T>& operator = (DynamicArrayImpl<T>&& rh)
        {
            if(this == &rh) //avoid self assignnment
                return *this;
            destroy();
            move(rh);
            return *this;
        }

        /// \brief [] operator
        T& operator[] (size_t index)
        {
            GEP_ASSERT(index < m_length, "out of bounds access", index, m_length);
            return m_data[index];
        }

        /// \brief [] operator const
        const T& operator[] (size_t index) const
        {
            GEP_ASSERT(index < m_length, "out of bounds access", index, m_length);
            return m_data[index];
        }

        /// \brief reserves at least the given number of elements
        void reserve(size_t numElements)
        {
            if(numElements > m_reserved)
            {
                if(numElements < m_reserved * 2)
                    numElements = m_reserved * 2;
                T* newData = (T*)(m_pAllocator->allocateMemory(sizeof(T) * numElements));
                MemoryUtils::uninitializedMove(newData, m_data, m_length);
                MemoryUtils::destroy(m_data, m_length);
                m_pAllocator->freeMemory(m_data);
                m_data = newData;
                m_reserved = numElements;
            }
        }

        /// \brief resizes the array to the given number of elements
        void resize(size_t numElements)
        {
            if(m_reserved < numElements)
                reserve(numElements);
            ptrdiff_t numElementsToConstruct = numElements - m_length;
            if(numElementsToConstruct > 0)
            {
                MemoryUtils::uninitializedConstruct(m_data + m_length, numElementsToConstruct);
            }
            else
            {
                MemoryUtils::destroy(m_data + m_length + numElementsToConstruct, -numElementsToConstruct);
            }
            m_length = numElements;
        }

		/// \brief destroys all elements in the array and sets its length to 0
        void clear()
        {
            MemoryUtils::destroy(m_data, m_length);
            m_length = 0;
        }

        /// \brief appends a element to the end of the array
        void append(const T& el)
        {
            if(m_reserved == m_length)
            {
                if(m_reserved == 0)
                    reserve(4);
                else
                    reserve(m_reserved * 2); //amortized doubling
            }
            new (m_data + m_length) T(el);
            m_length++;
        }

        /// \brief removes the element at the given index shifting all elements behind it one index forth
        void removeAtIndex(size_t index)
        {
            GEP_ASSERT(index < m_length, "out of bounds removeAt", index, m_length);
            MemoryUtils::move(m_data + index, m_data + index + 1, m_length - index - 1);
            m_length--;
            m_data[m_length].~T();
        }

        /// \brief inserts a element at the given index
        void insertAtIndex(size_t index, const T& value)
        {
            GEP_ASSERT(index <= m_length, "out of bounds access");
            resize(m_length + 1);
            if(m_length > 1)
            {
              for(size_t i = m_length-1; i > index; i--)
              {
                m_data[i] = m_data[i-1]; //TODO use memmove
              }
            }
            m_data[index] = value;
        }

        /// \brief appends an array
        void append(const ArrayPtr<T>& array)
        {
            size_t currentLength = length();
            size_t newLength = currentLength + array.length();
            reserve(newLength);
            MemoryUtils::uninitializedCopy(m_data + currentLength, array.getPtr(), array.length());
            m_length = newLength;
        }

        /// \brief creates a begin iterator
        T* begin()
        {
            return m_data;
        }

        /// \brief creates a constant begin iterator
        const T* begin() const
        {
            return m_data;
        }

        /// \brief creates a end iterator
        T* end()
        {
            return m_data + m_length;
        }

        /// \brief creates a constant end iterator
        const T* end() const
        {
            return m_data + m_length;
        }

        /// \brief creates a array ptr point to the container data
        ArrayPtr<T> toArray()
        {
            return ArrayPtr<T>(m_data, m_length);
        }

        const ArrayPtr<T> toArray() const
        {
            return ArrayPtr<T>(m_data, m_length);
        }

        /// \brief returns the length of the dynamic array
        size_t length() const
        {
            return m_length;
        }

        /// \brief returns the reserved number of elements
        size_t reserved() const
        {
            return m_reserved;
        }

        /// \brief removes a element without keeping the order of elements
        void removeAtIndexUnordered(size_t index)
        {
            GEP_ASSERT(index < m_length, "out of bounds access");
            if(index < m_length - 1)
            {
                std::swap(m_data[index], m_data[m_length-1]);
            }
            MemoryUtils::destroy(m_data + m_length - 1, 1);
            m_length--;
        }

        /// \brief returns the last element in the array
        T& lastElement()
        {
            GEP_ASSERT(m_length > 0, "array is empty");
            return m_data[m_length-1];
        }

        /// \brief returns the last element in the array
        const T& lastElement() const
        {
            GEP_ASSERT(m_length > 0, "array is empty");
            return m_data[m_length-1];
        }

        /// \brief removes the last element in the array
        void removeLastElement()
        {
            GEP_ASSERT(m_length > 0, "array is empty");
            MemoryUtils::destroy(m_data + m_length - 1, 1);
            m_length--;
        }
    };

    /// \brief DynamicArray indirection to deal with allocator policies and avoid code bloat
    // note: this is already fully implemented, you don't need to change anything here
    template <class T, class AllocatorPolicy = StdAllocatorPolicy>
    struct DynamicArray : public DynamicArrayImpl<T>
    {
    public:
        inline DynamicArray() : DynamicArrayImpl(AllocatorPolicy::getAllocator())
        {
        }

        inline DynamicArray(IAllocator* allocator) : DynamicArrayImpl(allocator)
        {
        }

        inline DynamicArray(const ArrayPtr<T>& data) : DynamicArrayImpl(AllocatorPolicy::getAllocator(), data)
        {
        }

        inline DynamicArray(const DynamicArray<T, AllocatorPolicy>& rh) : DynamicArrayImpl(rh)
        {
        }

        inline DynamicArray(const DynamicArrayImpl<T>& rh) : DynamicArrayImpl(rh)
        {
        }

        inline DynamicArray(DynamicArray<T, AllocatorPolicy>&& rh) : DynamicArrayImpl(std::move(rh))
        {
        }

        inline DynamicArray(DynamicArrayImpl<T>&& rh) : DynamicArrayImpl(std::move(rh))
        {
        }

        inline DynamicArray<T, AllocatorPolicy>& operator = (const DynamicArray<T, AllocatorPolicy>& rh)
        {
            return static_cast<DynamicArray<T, AllocatorPolicy>&>(DynamicArrayImpl<T>::operator=(rh));
        }

        inline DynamicArray<T, AllocatorPolicy>& operator = (const DynamicArrayImpl<T>& rh)
        {
            return static_cast<DynamicArray<T, AllocatorPolicy>&>(DynamicArrayImpl<T>::operator=(rh));
        }

        inline DynamicArray<T, AllocatorPolicy>& operator = (DynamicArray<T, AllocatorPolicy>&& rh)
        {
            return static_cast<DynamicArray<T, AllocatorPolicy>&>(DynamicArrayImpl<T>::operator=(std::move(rh)));
        }

        inline DynamicArray<T, AllocatorPolicy>& operator = (DynamicArrayImpl<T>&& rh)
        {
            return static_cast<DynamicArray<T, AllocatorPolicy>&>(DynamicArrayImpl<T>::operator=(std::move(rh)));
        }
    };
}
