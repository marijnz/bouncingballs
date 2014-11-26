#pragma once

namespace lua
{
    class GEP_API FunctionWrapper
    {
    public:
        static const int s_invalidReference = -1;
        static FunctionWrapper invalidValue()
        {
            return FunctionWrapper();
        }

        inline explicit FunctionWrapper(lua_State* L = nullptr, int index = 0) :
            m_L(L),
            m_tableReference(s_invalidReference)
        {
            if(index == 0 || lua_isnil(m_L, index)) { return; }

            if (lua_isfunction(m_L, index))
            {
                initializeTable(index);
            }
            else
            {
                auto luaType = lua_type(m_L, index);
                auto luaTypeName = lua_typename(m_L, luaType);
                GEP_ASSERT(false, "Expected a function on the stack at 'index' but got something else!", index, luaType, luaTypeName);
            }
        }

        inline FunctionWrapper(const FunctionWrapper& other) :
            m_L(other.m_L),
            m_tableReference(other.m_tableReference)
        {
            if (isValid())
            {
                addReference();
            }
        }

        inline void operator = (FunctionWrapper rhs)
        {
            if (isValid())
            {
                removeReference();
            }
            std::swap(m_L, rhs.m_L);
            std::swap(m_tableReference, rhs.m_tableReference);
        }

        inline ~FunctionWrapper()
        {
            if (isValid())
            {
                removeReference();
                m_tableReference = s_invalidReference;
            }
        }

        void push();

        inline bool isValid() { return m_tableReference != s_invalidReference; }

    private:
        static const int s_refCountIndex = 1;
        static const int s_functionIndex = 2;

        lua_State* m_L;

        // A reference to a helper table, containing a ref count and the actual function
        int m_tableReference;

        void addReference();
        void removeReference();

        void initializeTable(int functionIndex);

        void pushTable();
    };
}
