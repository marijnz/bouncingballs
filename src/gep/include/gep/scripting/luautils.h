#pragma once

namespace lua
{
    namespace utils
    {
        struct TraversalInfo
        {
            lua_State* L;
            int index;
            size_t level;
            const size_t maxLevel;
            std::ostringstream& output;

            bool indent;

            TraversalInfo(lua_State* L, int index, size_t level, const size_t maxLevel, std::ostringstream& output) :
                L(L),
                index(index),
                level(level),
                maxLevel(maxLevel),
                output(output),
                indent(true)
            {
            }
        };

        GEP_API std::string dumpStack (lua_State *L);
        GEP_API void printType(TraversalInfo input);
        GEP_API void dumpTable(TraversalInfo input);
        GEP_API std::string traceback(lua_State* L);

        // Expects two objects on the stack, each containing a "__ptr" key.
        GEP_API int ptrCompare(lua_State* L);

        GEP_API int nullCheck(lua_State* L);

        /// \brief Checks the type of the item on the stack with the given \a expectedLuaType
        /// \remark may raise a lua error
        inline void typeCheck(lua_State* L, int expectedLuaType, int index, const char* expectedName = nullptr)
        {
            auto typeId = lua_type(L, index);
            if (typeId != expectedLuaType)
            {
                auto expectedTypeName = expectedName ? expectedName : lua_typename(L, expectedLuaType);
                auto typeName = lua_typename(L, typeId);
                luaL_error(L, "Type check failed. Expected \"%s\", got \"%s\"", expectedTypeName, typeName);
            }
        }

#ifdef _DEBUG
        struct StackChecker
        {
            StackChecker(lua_State* L, int numReturnValues) :
                L(L),
                stackSize(0),
                numReturnValues(numReturnValues)
            {
                stackSize = lua_gettop(L);
            }

            ~StackChecker()
            {
                int top = lua_gettop(L);
                GEP_ASSERT(top == stackSize + numReturnValues, "You didn't clean up the stack after yourself!");
            }

        private:
            lua_State* L;
            int stackSize;
            int numReturnValues;
        };
#else
        struct StackChecker
        {
            StackChecker(lua_State*, int){}
        };
#endif // _DEBUG


        struct StackCleaner
        {
            StackCleaner(lua_State* L, int numReturnValues) :
                L(L),
                stackSize(0),
                numReturnValues(numReturnValues)
            {
                stackSize = lua_gettop(L);
            }

            ~StackCleaner()
            {
                lua_settop(L, stackSize + numReturnValues);
            }
        private:
            lua_State* L;
            int stackSize;
            int numReturnValues;
        };
    }
}
