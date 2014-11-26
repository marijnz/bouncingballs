#pragma once

#include "gep/memory/MemoryUtils.h"
#include <limits>

namespace gep
{
    GEP_API unsigned int hashOf( const void* buf, size_t len, unsigned int seed = 0 );

    struct StdHashPolicy
    {
        template <class T>
        static unsigned int hash(const T& el)
        {
            return hashOf(&el, sizeof(T));
        }

        template<class T>
        static bool equals(const T& lhs, const T& rhs)
        {
            return lhs == rhs;
        }
    };

    struct StringHashPolicy
    {
        static unsigned int hash(const char* str)
        {
            return hashOf(str, strlen(str));
        }

        static unsigned int hash(const std::string& str)
        {
            return hashOf(str.c_str(), str.length());
        }

        static bool equals(const char* lhs, const char* rhs)
        {
            return strcmp(lhs, rhs) == 0;
        }

        static bool equals(const std::string& lhs, const std::string& rhs)
        {
            return lhs == rhs;
        }
    };

    struct HashMethodPolicy
    {
        template <class T>
        static unsigned int hash(const T& el)
        {
            return el.hash();
        }

        template <class T>
        static unsigned int equals(const T& lhs, const T& rhs)
        {
            return lhs == rhs;
        }
    };

    struct HashMethodPointerPolicy
    {
        template <class T>
        static unsigned hash(const T* el)
        {
            return el->hash();
        }

        template <class T>
        static unsigned equals(const T* lhs, const T* rhs)
        {
            return (*lhs) == (*rhs);
        }
    };

    struct PointerHashPolicy
    {
        inline static unsigned int hash(const void* ptr)
        {
            // We can savely devide the pointer by the architectures default alignment because almost all pointers will be aligned
            // Pointers that are not aligned will cause a hash collision
            #ifdef _M_X64 //x64
            return (reinterpret_cast<unsigned int>(ptr) / 8) % 0xFFFFFFFF;
            #else //X86
            return reinterpret_cast<unsigned int>(ptr) / 4;
            #endif
        }

        static bool equals(const void* lhs, const void* rhs)
        {
            return lhs == rhs;
        }
    };

    struct DontHashPolicy
    {
        template <typename T>
        inline static unsigned int hash(T value)
        {
            return value;
        }

        template<class T>
        static bool equals(const T& lhs, const T& rhs)
        {
            return lhs == rhs;
        }
    };

    template <class K, class V, class HashPolicy>
    class HashmapImpl
    {
      public:
        struct Pair {
            K key;
            V value;
            Pair(const K& key, const V& value) :
                key(key), value(value) {}
            Pair(K&& key, V&& value) :
                key(key), value(value) {}
        };

      private:
        enum class State : char {
          Free, // 0
          Deleted, // 1
          Data // 2
        };

        static_assert(sizeof(State) == 1, "Invalid size of State enum");

        Pair* m_data;
        State* m_states;
        size_t m_reserved;
        size_t m_fullCount;
        size_t m_numDeletedEntries;
        IAllocator* m_allocator;

        static const int INITIAL_SIZE = 4;

        size_t insertEntry(const K& key, const V& value)
        {
            size_t index = getFreeIndex(key);
            new (m_data+index) Pair(key, value);
            if (m_states[index] == State::Deleted)
            {
                m_numDeletedEntries--;
            }
            m_states[index] = State::Data;
            return index;
        }

        size_t moveEntry(K& key, V& value)
        {
            size_t index = getFreeIndex(key);
            new (m_data+index) Pair(std::move(key), std::move(value));
            m_states[index] = State::Data;
            return index;
        }

        void moveEntry(const Pair& data)
        {
            new (m_data + getFreeIndex(data.key)) Pair(std::move(data));
        }

        size_t getFreeIndex(const K& key) const
        {
            size_t index = HashPolicy::hash(key) % m_reserved;
            while(m_states[index] == State::Data)
            {
                index++;
                if(index == m_reserved)
                {
                    index = 0;
                }
            }
            return index;
        }

        size_t getIndex(const K& key) const
        {
            if(m_reserved > 0)
            {
                size_t index = HashPolicy::hash(key) % m_reserved;
                size_t searched = 0;
                while(m_states[index] != State::Free && searched < m_reserved)
                {
                    if(m_states[index] == State::Data && HashPolicy::equals(m_data[index].key, key))
                        return index;
                    index++;
                    if(index == m_reserved)
                    {
                        index = 0;
                    }
                    searched++;
                }
            }
            return std::numeric_limits<size_t>::max();
        }

        void doRemove(size_t index)
        {
            size_t nextIndex = (index + 1) % m_reserved;
            if(m_states[nextIndex] != State::Free)
            {
                m_states[index] = State::Deleted;
                m_numDeletedEntries++;
            }
            else
            {
                m_states[index] = State::Free;
            }

            m_data[index].~Pair();
            m_fullCount--;
        }

        void copy(const HashmapImpl<K, V, HashPolicy>& other)
        {
            m_allocator = other.m_allocator;
            m_data = (Pair*)m_allocator->allocateMemory(sizeof(Pair) * other.m_reserved);
            m_states = (State*)m_allocator->allocateMemory(other.m_reserved);
            m_reserved = other.m_reserved;
            m_fullCount = other.m_fullCount;
            m_numDeletedEntries = other.m_numDeletedEntries;
            memcpy(m_states, other.m_states, other.m_reserved);
            for(size_t i=0; i<m_reserved; i++)
            {
                if(m_states[i] == State::Data)
                {
                    new (m_data + i) Pair (other.m_data[i]);
                }
            }
        }

        void move(HashmapImpl<K, V, HashPolicy>& other)
        {
            m_allocator = other.m_allocator;
            m_data = other.m_data;
            other.m_data = nullptr;
            m_states = other.m_states;
            other.m_states = nullptr;
            m_fullCount = other.m_fullCount;
            other.m_fullCount = 0;
            m_numDeletedEntries = other.m_numDeletedEntries;
            other.m_numDeletedEntries = 0;
            m_reserved = other.m_reserved;
            other.m_reserved = 0;
        }

        void destroy()
        {
            if(m_data != nullptr)
            {
                for(size_t i=0; i<m_reserved; i++)
                {
                    if(m_states[i] == State::Data)
                    {
                        m_data[i].~Pair();
                    }
                }
                m_allocator->freeMemory(m_states);
                m_allocator->freeMemory(m_data);
            }
        }

      public:

        struct Iterator
        {
        private:
            size_t index;
            HashmapImpl<K, V, HashPolicy>* m_pBackptr;

        public:
            Iterator(size_t index, HashmapImpl<K, V, HashPolicy>* pBackptr)
                : index(index), m_pBackptr(pBackptr)
            {}

            inline bool operator == (const Iterator& rh) const
            {
                return m_pBackptr == rh.m_pBackptr && index == rh.index;
            }
            inline bool operator != (const Iterator& rh) const
            {
                return !operator == (rh);
            }
            Iterator& operator++()
            {
                do
                {
                    index++;
                    if(index > m_pBackptr->m_reserved)
                    {
                        index = m_pBackptr->m_reserved;
                        break;
                    }
                }
                while(m_pBackptr->m_states[index] != State::Data);
                return *this;
            }
            Pair* operator->() const
            {
                GEP_ASSERT(index < m_pBackptr->m_reserved, "iterator out of bounds");
                return &m_pBackptr->m_data[index];
            }
            Pair& operator*() const
            {
                GEP_ASSERT(index < m_pBackptr->m_reserved, "iterator out of bounds");
                return m_pBackptr->m_data[index];
            }
        };

        struct KeyIterator
        {
        private:
            Iterator it;
        public:
            KeyIterator(Iterator it)
                : it(it)
            {}

            inline bool operator != (const KeyIterator& rh) const
            {
                return it != rh.it;
            }
            inline bool operator == (const KeyIterator& rh) const
            {
                return it == rh.it;
            }
            KeyIterator& operator++()
            {
                ++it;
                return *this;
            }
            K* operator->() const
            {
                return &it->key;
            }
            K& operator*() const
            {
                return it->key;
            }
        };

        struct ValueIterator
        {
        private:
            Iterator it;
        public:
            ValueIterator(Iterator it)
                : it(it)
            {}

            inline bool operator != (const ValueIterator& rh) const
            {
                return it != rh.it;
            }
            inline bool operator == (const ValueIterator& rh) const
            {
                return it == rh.it;
            }
            ValueIterator& operator++()
            {
                ++it;
                return *this;
            }
            V* operator->() const
            {
                return &it->value;
            }
            V& operator*() const
            {
                return it->value;
            }
        };

        struct KeyRange
        {
        private:
            KeyIterator m_begin, m_end;
        public:
            KeyRange(KeyIterator begin, KeyIterator end) : m_begin(begin), m_end(end) {}
            KeyIterator begin() { return m_begin; }
            KeyIterator end() { return m_end; }
        };

        struct ValueRange
        {
        private:
            ValueIterator m_begin, m_end;
        public:
            ValueRange(ValueIterator begin, ValueIterator end) : m_begin(begin), m_end(end) {}
            ValueIterator begin() { return m_begin; }
            ValueIterator end() { return m_end; }
        };

        /// \brief constructor
        HashmapImpl(IAllocator* allocator)
        {
            m_allocator = allocator;
            m_data = (Pair*)allocator->allocateMemory(sizeof(Pair) * INITIAL_SIZE);
            m_states = (State*)allocator->allocateMemory(INITIAL_SIZE);
            m_reserved = INITIAL_SIZE;
            m_fullCount = 0;
            m_numDeletedEntries = 0;
            memset(m_states, 0, m_reserved);
        }

        /// \brief copy constructor
        HashmapImpl(const HashmapImpl<K, V, HashPolicy>& other)
        {
            copy(other);
        }

        /// \brief move constructor
        HashmapImpl(HashmapImpl<K, V, HashPolicy>&& other)
        {
            move(other);
        }

        ~HashmapImpl()
        {
            destroy();
        }

        /// \brief assignment operator
        HashmapImpl<K, V, HashPolicy>& operator = (const HashmapImpl<K, V, HashPolicy>& rh)
        {
            if(this == &rh) // handle self assignment
                return *this;
            destroy();
            copy(rh);
            return *this;
        }

        /// \brief move assignment operator
        HashmapImpl<K, V, HashPolicy>& operator = (HashmapImpl<K, V, HashPolicy>&& rh)
        {
            if(this == &rh) // handle self assignment
                return *this;
            destroy();
            move(rh);
            return *this;
        }

        /// \brief [] operator
        V& operator[](const K& key)
        {
          size_t index = getIndex(key);
          if(index == std::numeric_limits<size_t>::max()) //not in the HashmapImpl yet
          {
            m_fullCount++;
            auto pseudoFullCount = m_fullCount + m_numDeletedEntries;
            if(pseudoFullCount > ((m_reserved * 3) / 4) || pseudoFullCount >= m_reserved)
            {
              Pair* oldData = m_data;
              State* oldStates = m_states;
              size_t oldLength = m_reserved;
              m_reserved = (oldLength == 0) ? 4 : oldLength * 2;
              m_data = (Pair*)m_allocator->allocateMemory(m_reserved * sizeof(Pair));
              m_states = (State*)m_allocator->allocateMemory(m_reserved);
              memset(m_states, (int)State::Free, m_reserved);

              //rehash all values
              for(size_t i=0; i < oldLength; i++)
              {
                  if(oldStates[i] == State::Data)
                  {
                      //move from the old to the new array
                      moveEntry(oldData[i].key, oldData[i].value);
                      //destroy the element in the old array
                      oldData[i].~Pair();
                  }
              }
              m_allocator->freeMemory(oldData);
              m_allocator->freeMemory(oldStates);
            }
            index = insertEntry(key, V());
          }
          return m_data[index].value;
        }

        /// \brief const version of operator []
        const V& operator[](const K& key) const
        {
            size_t index = getIndex(key);
            if(index != std::numeric_limits<size_t>::max())
            {
                return m_data[index].value;
            }

            GEP_ASSERT(0,"not found");
            throw std::exception("key not found");
        }

        /// \brief checks if a element does exist within the HashmapImpl
        bool exists(const K& key) const
        {
            return getIndex(key) != std::numeric_limits<size_t>::max();
        }

        /// \brief tries to retrieve a element from the HashmapImpl. On success outValue will be filled and SUCCESS will be returned, otherwise it will return FAILURE
        Result tryGet(const K& key, V& outValue) const
        {
            size_t index = getIndex(key);
            if(index != std::numeric_limits<size_t>::max())
            {
                outValue = m_data[index].value;
                return SUCCESS;
            }
            return FAILURE;
        }

        /// \brief removes a entry from the HashmapImpl
        Result remove(const K& key)
        {
            size_t index = getIndex(key);
            if(index == std::numeric_limits<size_t>::max())
                return FAILURE;

            doRemove(index);
            return SUCCESS;
        }

        /// \brief removes all entries from the HashmapImpl
        void clear()
        {
            for(size_t i=0; i < m_reserved; i++)
            {
                if(m_states[i] == State::Data)
                {
                    m_data[i].~Pair();
                    m_states[i] = State::Free;
                }
            }
            m_fullCount = 0;
        }

        /// \brief returns a begin iterator
        inline Iterator begin()
        {
            return ++(Iterator(-1, this));
        }

        /// \brief returns a end iterator
        inline Iterator end()
        {
            return Iterator(m_reserved, this);
        }

        /// \brief returns a range for iterating the keys
        inline KeyRange keys()
        {
            return KeyRange(KeyIterator(begin()), KeyIterator(end()));
        }

        /// \brief returns a range for iterating the values
        inline ValueRange values()
        {
            return ValueRange(ValueIterator(begin()), ValueIterator(end()));
        }

        /// \brief returns how many elements are inside the HashmapImpl
        inline size_t count() const
        {
            return m_fullCount;
        }

        inline size_t removeWhere(std::function<bool(K&, V&)> condition)
        {
            size_t removed = 0;
            for (size_t index = 0; index < m_reserved; ++index)
            {
                State state = m_states[index];
                Pair& entry = m_data[index];
                if (m_states[index] == State::Data && condition(entry.key, entry.value))
                {
                    doRemove(index);
                    ++removed;
                }
            }
            return removed;
        }

        void ifExists(const K& key, std::function<void(V&)> doIfTrue, std::function<void()> doIfFalse = nullptr)
        {
            auto index = getIndex(key);
            if(index != std::numeric_limits<size_t>::max())
                doIfTrue(m_data[index].value);
            else if(doIfFalse)
                doIfFalse();
        }

        // For testing
        bool isPseudoFull()
        {
            for(size_t i = 0; i < m_reserved; ++i)
            {
                if (m_states[i] == State::Free)
                {
                    return false;
                }
            }
            return true;

        }
    };

    /// \brief Hashmap indirection to deal with allocator policies and avoid code bloat
    // note: this is already fully implemented, you don't need to change anything here
    template <class K, class V, class HashPolicy = StdHashPolicy, class AllocatorPolicy = StdAllocatorPolicy>
    struct Hashmap : public HashmapImpl<K, V, HashPolicy>
    {
    public:
        inline Hashmap() : HashmapImpl(AllocatorPolicy::getAllocator())
        {
        }

        inline Hashmap(IAllocator* allocator) : HashmapImpl(allocator)
        {
        }

        inline Hashmap(const Hashmap<K, V, HashPolicy, AllocatorPolicy>& rh) : HashmapImpl(rh)
        {
        }

        inline Hashmap(const HashmapImpl<K, V, HashPolicy>& rh) : HashmapImpl(rh)
        {
        }

        inline Hashmap(Hashmap<K, V, HashPolicy, AllocatorPolicy>&& rh) : HashmapImpl(std::move(rh))
        {
        }

        inline Hashmap(HashmapImpl<K, V, HashPolicy>&& rh) : HashmapImpl(std::move(rh))
        {
        }

        inline Hashmap<K, V, HashPolicy, AllocatorPolicy>& operator = (const Hashmap<K, V, HashPolicy, AllocatorPolicy>& rh)
        {
            return static_cast<Hashmap<K, V, HashPolicy, AllocatorPolicy>&>(HashmapImpl<K, V, HashPolicy>::operator=(rh));
        }

        inline Hashmap<K, V, HashPolicy, AllocatorPolicy>& operator = (const HashmapImpl<K, V, HashPolicy>& rh)
        {
            return static_cast<Hashmap<K, V, HashPolicy, AllocatorPolicy>&>(HashmapImpl<K, V, HashPolicy>::operator=(rh));
        }

        inline Hashmap<K, V, HashPolicy, AllocatorPolicy>& operator = (Hashmap<K, V, HashPolicy, AllocatorPolicy>&& rh)
        {
            return static_cast<Hashmap<K, V, HashPolicy, AllocatorPolicy>&>(HashmapImpl<K, V, HashPolicy>::operator=(std::move(rh)));
        }

        inline Hashmap<K, V, HashPolicy, AllocatorPolicy>& operator = (HashmapImpl<K, V, HashPolicy>&& rh)
        {
            return static_cast<Hashmap<K, V, HashPolicy, AllocatorPolicy>&>(HashmapImpl<K, V, HashPolicy>::operator=(std::move(rh)));
        }
    };
}
