#pragma once

namespace lua
{
    class GEP_API TableWrapper
    {
    public:
        static const int s_invalidReference = -1;
        static TableWrapper invalidValue()
        {
            return TableWrapper();
        }

        inline explicit TableWrapper(lua_State* L = nullptr, int index = 0) :
            m_L(L),
            m_tableReference(s_invalidReference)
        {
            if(index == 0 || lua_isnil(m_L, index)) { return; } // invalid index

            if (lua_istable(m_L, index))
            {
                injectRefCountEntry(index);
            }
            else
            {
                auto luaType = lua_type(m_L, index);
                auto luaTypeName = lua_typename(m_L, luaType);
                GEP_ASSERT(false, "Expected a table on the stack at 'index' but got something else!", index, luaType, luaTypeName);
            }
        }

        inline TableWrapper(const TableWrapper& other) :
            m_L(other.m_L),
            m_tableReference(other.m_tableReference)
        {
            if (isValid())
            {
                addReference();
            }
        }

        inline void operator = (TableWrapper rhs)
        {
            if (isValid())
            {
                removeReference();
            }
            std::swap(m_L, rhs.m_L);
            std::swap(m_tableReference, rhs.m_tableReference);
        }

        inline ~TableWrapper()
        {
            if (isValid())
            {
                removeReference();
                m_tableReference = s_invalidReference;
            }
        }

        void push();

        inline bool isValid() { return m_tableReference != s_invalidReference; }

        template<typename T_Key, typename T_Value>
        inline void get(const T_Key& key, T_Value& out_value)
        {
            if(!tryGet(key, out_value))
            {
                GEP_ASSERT(false, "Failed to get value from table.", key);
            }
        }

        template<typename T_Key, typename T_Value>
        inline bool tryGet(const T_Key& key, T_Value& out_value)
        {
            utils::StackCleaner cleaner(m_L, 0);
            this->push();
            auto tableIndex = lua_gettop(m_L);
            lua::push(m_L, key);
            lua_gettable(m_L, tableIndex);
            if (lua_isnil(m_L, -1)) return false;
            auto valueIndex = lua_gettop(m_L);
            out_value = lua::pop<T_Value>(m_L, valueIndex);
            return true;
        }

        inline lua_State* getState() { return m_L; }

    private:
        static const char* refCountIndex() { return "__refCount"; }

        lua_State* m_L;

        // A reference to the actual table, also containing a ref count
        int m_tableReference;

        void addReference();
        void removeReference();

        void injectRefCountEntry(int functionIndex);
    };
}
