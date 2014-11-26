#pragma once

#include "gep/scripting/luaTypeHandling.h"

namespace lua
{
    namespace structs
    {
        template <typename T, bool isType>
        struct enum_or_typeHandling {};

        template <typename T>
        struct enum_or_typeHandling<T, false>
        {
            static int push(lua_State* L, T t)
            {
                return typeHandling<T>::push(L, t);
            }
            static T pop(lua_State* L, int idx)
            {
                return typeHandling<T>::pop(L, idx);
            }
        };

        template <typename T>
        struct enum_or_typeHandling<T, true>    // T is an enum
        {
            static int push(lua_State* L, T t)
            {
                return typeHandling<int>::push(L, t);
            }
            static T pop(lua_State* L, int idx)
            {
                return (T)typeHandling<int>::pop(L, idx);
            }
        };
    }
}
