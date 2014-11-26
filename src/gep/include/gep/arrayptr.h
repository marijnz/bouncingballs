#pragma once

#include "gep/memory/MemoryUtils.h"

namespace gep {

    /// \brief This class encapsulates an array and it's length. It is recommended to use this class instead of plain c arrays.
    template <typename T>
    struct ArrayPtr
    {
    public:
        typedef T element_t;

        /// \brief default constructor
        inline ArrayPtr() :
        m_ptr(nullptr), m_length(0)
        {
        }

        /// \brief constructor
        inline ArrayPtr(T* ptr, size_t length)
        {
            if (ptr != nullptr && length != 0)
            {
                m_ptr = ptr;
                m_length = length;
            }
            else
            {
                m_ptr = nullptr;
                m_length = 0;
            }
        }

        /// \brief creates a ArrayPtr from a static array
        template <size_t N>
        inline ArrayPtr(T (&staticArray)[N]) :
        m_ptr(staticArray), m_length(N)
        {
        }

        inline ArrayPtr(const ArrayPtr<typename std::remove_const<T>::type>& other) :
        m_ptr(other.getPtr()), m_length(other.length())
        {
        }

        inline ArrayPtr& operator=(const ArrayPtr<typename std::remove_const<T>::type>& other)
        {
            m_ptr = other.getPtr();
            m_length = other.length();
            return *this;
        }

        inline ArrayPtr& operator = (std::nullptr_t null)
        {
            m_ptr = nullptr;
            m_length = 0;
            return *this;
        }

        inline T* getPtr() const
        {
            return m_ptr;
        }

        inline size_t length() const
        {
            return m_length;
        }

        /// creates a sub-array from this array
        inline ArrayPtr<T> operator()(size_t start, size_t end)
        {
            GEP_ASSERT(start < end, "Start has to be smaller than end", start, end);
            GEP_ASSERT(end <= m_length, "End has to be smaller or equal than the count", end, m_length);
            return ArrayPtr<T>(m_ptr + start, end - start);
        }

        /// \brief creates a sub-array from this array
        inline const ArrayPtr<T> operator()(size_t start, size_t end) const
        {
            GEP_ASSERT(start < end, "Start has to be smaller than end", start, end);
            GEP_ASSERT(end <= m_length, "End has to be smaller or equal than the length", end, m_length);
            return ArrayPtr<T>(m_ptr + start, end - start);
        }

        /// \brief [] operator
        inline T& operator[](size_t index) const
        {
            GEP_ASSERT(index < m_length, "out of bounds access", index, m_length);
            return m_ptr[index];
        }

        /// \brief == operator
        inline bool operator==(const ArrayPtr<T>& other) const
        {
            if (m_length != other.m_length)
                return false;

            if (m_ptr == other.m_ptr)
                return true;

            return MemoryUtils::equals(m_ptr, other.m_ptr, m_length);
        }

        /// \brief != operator
        inline bool operator!=(const ArrayPtr<T>& other) const
        {
            return !operator==(other);
        }

        /// \brief copy data from another array
        inline void copyFrom(const ArrayPtr<T>& other)
        {
            GEP_ASSERT(m_length == other.m_length, "Array lengths for don't match.", m_length, other.m_length);
            MemoryUtils::copy(m_ptr, other.m_ptr, m_length);
        }

        /// \brief returns a begin iterator
        inline T* begin()
        {
            return m_ptr;
        }

        /// \brief retruns a begin iterator
        inline const T* begin() const
        {
            return m_ptr;
        }

        /// \brief returns a end iterator
        inline T* end()
        {
            return m_ptr + m_length;
        }

        /// \brief returns a end iterator
        inline const T* end() const
        {
            return m_ptr + m_length;
        }

    private:
        T* m_ptr;
        size_t m_length;
    };

}
