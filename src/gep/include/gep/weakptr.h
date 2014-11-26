#pragma once

#include "gep/gepmodule.h"
#include "gep/memory/allocator.h"
#include "gep/exit.h"
#include <limits>
#include "gep/threading/mutex.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    // helper struct for weak ref index
    struct WeakRefIndex
    {
        union
        {
            struct {
                unsigned int index : 24;
                unsigned int hash : 8;
            };
            unsigned int both;
        };

        static WeakRefIndex invalidValue()
        {
            WeakRefIndex invalid = { 0xFFFFFF, 0xFF };
            return invalid;
        }

        inline bool operator == (const WeakRefIndex& rh)
        {
            return this->both == rh.both;
        }
    };
    static_assert(sizeof(WeakRefIndex) == 4, "WeakRefIndex should be 4 bytes big");

    template <class T>
    class WeakReferencedExport
    {
    protected:
        virtual ~WeakReferencedExport(){}

        typedef typename T WeakReferencedBaseType;

        GEP_API static T** s_weakTable;
        GEP_API static unsigned char* s_hashTable;
        GEP_API static unsigned int s_weakTableSize;
        GEP_API static unsigned int s_weakTableNumEntries;
        GEP_API static Mutex s_mutex;
    };

    template <class T>
    class WeakReferencedLocal
    {
    protected:
        virtual ~WeakReferencedLocal(){}

        typedef typename T WeakReferencedBaseType;

        static T** s_weakTable;
        static unsigned char* s_hashTable;
        static unsigned int s_weakTableSize;
        static unsigned int s_weakTableNumEntries;
        static Mutex s_mutex;
    };

    /// \brief base class for all weak referenced objects
    template <class T, template<class> class Base = WeakReferencedLocal>
    class WeakReferenced : public Base<T>
    {
        template <class, template<class> class> friend struct WeakPtr;
    protected:
        WeakRefIndex m_weakRefIndex;

        static void freeTable()
        {
            ScopedLock<Mutex> lock(s_mutex);
            delete[] s_hashTable;
            delete[] s_weakTable;
        }

        static WeakRefIndex findWeakRefIndex()
        {
            ScopedLock<Mutex> lock(s_mutex);
            if(s_weakTableNumEntries == s_weakTableSize)
            {
                unsigned int newSize = (s_weakTableSize == 0) ? 4 : s_weakTableSize * 2;
                T** oldWeakTable = s_weakTable;
                unsigned char* oldHashTable = s_hashTable;
                s_weakTable = new T*[newSize];
                s_hashTable = new unsigned char[newSize];
                memcpy(s_weakTable, oldWeakTable, sizeof(T*) * s_weakTableSize);
                memset(s_weakTable + s_weakTableSize, 0, (newSize - s_weakTableSize) * sizeof(T*));
                memcpy(s_hashTable, oldHashTable, sizeof(unsigned char) * s_weakTableSize);
                memset(s_hashTable + s_weakTableSize, 0, (newSize - s_weakTableSize));
                delete[] oldWeakTable;
                delete[] oldHashTable;
                if(s_weakTableSize == 0)
                {
                    gep::atexit(&freeTable);
                }
                s_weakTableSize = newSize;
            }
            unsigned int index = 0;
            for(; index < s_weakTableSize; index++)
            {
                if(s_weakTable[index] == nullptr)
                    break;
            }
            GEP_ASSERT(index < s_weakTableSize, "table is full");
            GEP_ASSERT(index <= 0x00FFFFFF, "index does not fit into 24 bits");
            WeakRefIndex result;
            s_weakTableNumEntries++;
            result.index = index;
            result.hash = ++s_hashTable[index];
            if(result.hash == 0xFF)
            {
                result.hash = 0;
                s_hashTable[index] = 0;
            }
            return result;
        }
    public:
        WeakReferenced()
        {
            ScopedLock<Mutex> lock(s_mutex);
            m_weakRefIndex = findWeakRefIndex();
            s_weakTable[m_weakRefIndex.index] = static_cast<T*>(this);
        }

        virtual ~WeakReferenced()
        {
            ScopedLock<Mutex> lock(s_mutex);
            s_weakTable[m_weakRefIndex.index] = nullptr;
            s_weakTableNumEntries--;
        }

        /// \brief gets the weak ref index for debugging purposes
        inline uint32 getWeakRefIndex()
        {
            return m_weakRefIndex.index;
        }

        /// \brief all weak references of this and another weak referenced object
        void swapPlaces(WeakReferenced<T, Base>& other)
        {
            ScopedLock<Mutex> lock(s_mutex);
            std::swap(s_weakTable[m_weakRefIndex.index], s_weakTable[other.m_weakRefIndex.index]);
            std::swap(m_weakRefIndex, other.m_weakRefIndex);
        }
    };

    #define DefineWeakRefStaticMembers(T) \
        T** gep::WeakReferencedLocal<T>::s_weakTable = nullptr; \
        unsigned char* gep::WeakReferencedLocal<T>::s_hashTable = nullptr; \
        unsigned int gep::WeakReferencedLocal<T>::s_weakTableSize = 0; \
        unsigned int gep::WeakReferencedLocal<T>::s_weakTableNumEntries = 0; \
        gep::Mutex gep::WeakReferencedLocal<T>::s_mutex;
    #define DefineWeakRefStaticMembersExport(T) \
        T** gep::WeakReferencedExport<T>::s_weakTable = nullptr; \
        unsigned char* gep::WeakReferencedExport<T>::s_hashTable = nullptr; \
        unsigned int gep::WeakReferencedExport<T>::s_weakTableSize = 0; \
        unsigned int gep::WeakReferencedExport<T>::s_weakTableNumEntries = 0; \
        gep::Mutex gep::WeakReferencedExport<T>::s_mutex;

    template <class T, template<class> class ExportType = WeakReferencedLocal>
    struct WeakPtr
    {
    protected:
        mutable WeakRefIndex m_weakRefIndex;
        #ifdef _DEBUG
        mutable typename T::WeakReferencedBaseType* m_pLastLookupResult;
        typename T::WeakReferencedBaseType*** m_pTable;
        uint8** m_pHashTable;
        #endif

    public:
        /// \brief default constructor
        inline WeakPtr()
        {
            m_weakRefIndex = WeakRefIndex::invalidValue();
            #ifdef _DEBUG
            m_pLastLookupResult = nullptr;
            m_pTable = &WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_weakTable;
            m_pHashTable = &WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_hashTable;
            #endif
        }

        /// \brief constructor from an object
        inline WeakPtr(T* ptr)
        {
            if(ptr != nullptr)
            {
                m_weakRefIndex = ptr->m_weakRefIndex;
            }
            else
            {
                m_weakRefIndex = WeakRefIndex::invalidValue();
            }
            #ifdef _DEBUG
            m_pLastLookupResult = ptr;
            m_pTable = &WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_weakTable;
            m_pHashTable = &WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_hashTable;
            #endif
        }

        /// \brief returns the pointer to the object, might be null
        inline T* get()
        {
            return const_cast<T*>(const_cast<const WeakPtr<T, ExportType>*>(this)->get());
        }

        /// \brief returns the pointer to the object, might be null
        inline const T* get() const
        {
            if(m_weakRefIndex == WeakRefIndex::invalidValue())
            {
                return nullptr;
            }
            else {
                ScopedLock<Mutex> lock(WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_mutex);
                unsigned int index = m_weakRefIndex.index;
                auto ptr = WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_weakTable[index];
                if(ptr != nullptr && WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_hashTable[index] == m_weakRefIndex.hash)
                {
                    #ifdef _DEBUG
                    m_pLastLookupResult = ptr;
                    #endif
                    return static_cast<T*>(ptr);
                }
                else
                {
                    // the weakRef is invalid, so invalidate it
                    m_weakRefIndex = WeakRefIndex::invalidValue();
                    #ifdef _DEBUG
                    m_pLastLookupResult = nullptr;
                    #endif
                    return nullptr;
                }
            }
        }

        WeakPtr<T, ExportType>& operator = (T* ptr)
        {
            if(ptr != nullptr)
            {
                m_weakRefIndex = ptr->m_weakRefIndex;
            }
            else
            {
                m_weakRefIndex = WeakRefIndex::invalidValue();
            }
            #ifdef _DEBUG
            m_pLastLookupResult = ptr;
            #endif
            return *this;
        }

        /// creates a new weak reference
        void setWithNewIndex(T* ptr)
        {
            ScopedLock<Mutex> lock(WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_mutex);
            m_weakRefIndex = WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::findWeakRefIndex();
            WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_weakTable[m_weakRefIndex.index] = ptr;
            #ifdef _DEBUG
            m_pLastLookupResult = ptr;
            #endif
        }

        /// \brief invalidates a weak reference which was previously created with setWithNewIndex
        ///   and replaces it with the given object
        void invalidateAndReplace(T* ptr)
        {
            ScopedLock<Mutex> lock(WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_mutex);
            GEP_ASSERT(ptr != nullptr);
            auto storedPtr = get();
            GEP_ASSERT(storedPtr != nullptr, "reference is already invalid");
            WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_weakTable[ptr->m_weakRefIndex.index] = nullptr;
            ptr->m_weakRefIndex = m_weakRefIndex;
            WeakReferenced<typename T::WeakReferencedBaseType, ExportType>::s_weakTable[m_weakRefIndex.index] = ptr;
            #ifdef _DEBUG
            m_pLastLookupResult = ptr;
            #endif
        }

        /// \brief gets the weak ref index for debugging purposes
        uint32 getWeakRefIndex()
        {
            return m_weakRefIndex.index;
        }
		
        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(luaGetter, "get")
        LUA_BIND_VALUE_TYPE_MEMBERS
        LUA_BIND_VALUE_TYPE_END

    private:
        T* luaGetter()
        {
            return get();
        }
    };
}
