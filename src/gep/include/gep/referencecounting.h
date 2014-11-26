#pragma once

#include "gepmodule.h"
#include "common.h"
#include "gep/memory/allocator.h"

namespace gep
{
    // forward declarations
    class IAllocator;

    /// \brief base class for all reference counted classes
    class GEP_API ReferenceCounted
    {
    private:
        // reference counted objects can not be created using c++ new
        void* operator new(size_t size);

    protected:
        volatile unsigned int m_referenceCount;
        IAllocator* m_pAllocator;

    public:
        // placement new
        void* operator new(size_t size, void* pWhere);

        ReferenceCounted() :
            m_referenceCount(0)
        {
            GEP_ASSERT(m_pAllocator != nullptr,
                       "The allocator should have been set by GEP_NEW! "
                       "To ensure correct behavior, ReferenceCounted should "
                       "be the first thing your class/struct inherits from "
                       "(yes, the order of inheritance does matter!)");
        }

        virtual ~ReferenceCounted() { }

        inline void setAllocator(IAllocator* pAllocator)
        {
            m_pAllocator = pAllocator;
        }

        inline void addReference()
        {
            InterlockedIncrement(&m_referenceCount);
        }

        inline void removeReference()
        {
            auto newRefcount = InterlockedDecrement(&m_referenceCount);
            GEP_ASSERT(newRefcount < std::numeric_limits<size_t>::max(), "invalid refcount");
            if(newRefcount == 0)
            {
                this->~ReferenceCounted();
                m_pAllocator->freeMemory(this);
            }
        }
    };

    template <class T>
    struct SmartPtr
    {
    private:
        T* m_ptr;

        inline void copy(const SmartPtr<T>& other)
        {
            m_ptr = other.m_ptr;
            if(m_ptr != nullptr)
                m_ptr->addReference();
        }

        inline void move(SmartPtr<T>& other)
        {
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }

        inline void destroy()
        {
            if(m_ptr != nullptr)
            {
                m_ptr->removeReference();
                m_ptr = nullptr;
            }
        }
    public:
        /// \brief default constructor
        inline SmartPtr()
            : m_ptr(nullptr)
        {
        }

        /// \brief copy constructor
        inline SmartPtr(const SmartPtr<T>& other)
        {
            copy(other);
        }

        /// \brief move constructor
        inline SmartPtr(SmartPtr<T>&& other)
        {
            move(other);
        }

        /// \brief construct from an instance
        inline SmartPtr(T* ptr)
        {
            m_ptr = ptr;
            if(ptr != nullptr)
                ptr->addReference();
        }

        /// \brief destructor
        inline ~SmartPtr()
        {
            destroy();
        }

        /// \brief assignment operator
        inline SmartPtr<T>& operator = (const SmartPtr& rh)
        {
            if(this != &rh)
            {
                destroy();
                copy(rh);
            }
            return *this;
        }

        /// \brief move assignment operator
        inline SmartPtr<T>& operator = (SmartPtr&& rh)
        {
            if(this != &rh)
            {
                destroy();
                move(rh);
            }
            return *this;
        }

        /// \brief assign from an instance
        inline SmartPtr<T>& operator = (T* ptr)
        {
            destroy();
            m_ptr = ptr;
            if(m_ptr != nullptr)
                m_ptr->addReference();
            return *this;
        }

        /// \brief -> operator
        inline T* operator -> ()
        {
            return m_ptr;
        }

        /// \brief const -> operator
        inline const T* operator -> () const
        {
            return m_ptr;
        }

        /// \brief dereferencing operator
        inline T& operator * ()
        {
            GEP_ASSERT(m_ptr != nullptr, "dereferencing null-pointer");
            return *m_ptr;
        }

        /// \brief const dereferencing operator
        inline const T& operator * () const
        {
            GEP_ASSERT(m_ptr != nullptr, "dereferencing null-pointer");
            return *m_ptr;
        }

        /// \brief returns the stored pointer
        inline T* get()
        {
            return m_ptr;
        }

        /// \brief converts to a boolean (null check)
        inline operator bool () const
        {
            return m_ptr != nullptr;
        }
    };
};
