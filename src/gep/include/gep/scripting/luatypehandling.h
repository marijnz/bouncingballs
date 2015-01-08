#pragma once

namespace lua
{
    namespace structs
    {
        template<typename T>
        struct typeHandling {};

        template <>
        struct typeHandling<nullptr_t>
        {
            static int push(lua_State* L)
            {
                lua_pushnil(L);
                return 1;
            }
            static int push(lua_State* L, nullptr_t)
            {
                lua_pushnil(L);
                return 1;
            }
            static nullptr_t pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNIL, idx, "nil");
                return nullptr;
            }
        };

        template <>
        struct typeHandling<gep::int8>
        {
            static int push(lua_State* L, gep::int8 value)
            {
                lua_pushinteger(L, lua_Integer(value));
                return 1;
            }
            static gep::int8 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::int8");
                auto value = lua_tointeger(L, idx);
                return gep::int8(value);
            }
        };

        template <>
        struct typeHandling<gep::int16>
        {
            static int push(lua_State* L, gep::int16 value)
            {
                lua_pushinteger(L, lua_Integer(value));
                return 1;
            }
            static gep::int16 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::int16");
                auto value = lua_tointeger(L, idx);
                return gep::int16(value);
            }
        };

        template <>
        struct typeHandling<gep::int32>
        {
            static int push(lua_State* L, gep::int32 value)
            {
                lua_pushinteger(L, lua_Integer(value));
                return 1;
            }
            static gep::int32 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::int32");
                auto value = lua_tointeger(L, idx);
                return gep::int32(value);
            }
        };

        template <>
        struct typeHandling<gep::int64>
        {
            static int push(lua_State* L, gep::int64 value)
            {
                lua_pushinteger(L, lua_Integer(value));
                return 1;
            }
            static gep::int64 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::int64");
                auto value = lua_tointeger(L, idx);
                return gep::int64(value);
            }
        };

        template <>
        struct typeHandling<gep::uint8>
        {
            static int push(lua_State* L, gep::uint8 value)
            {
                lua_pushunsigned(L, lua_Unsigned(value));
                return 1;
            }
            static gep::uint8 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::uint8");
                auto value = lua_tounsigned(L, idx);
                return gep::uint8(value);
            }
        };

        template <>
        struct typeHandling<gep::uint16>
        {
            static int push(lua_State* L, gep::uint16 value)
            {
                lua_pushunsigned(L, lua_Unsigned(value));
                return 1;
            }
            static gep::uint16 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::uint16");
                auto value = lua_tounsigned(L, idx);
                return gep::uint16(value);
            }
        };

        template <>
        struct typeHandling<gep::uint32>
        {
            static int push(lua_State* L, gep::uint32 value)
            {
                lua_pushunsigned(L, lua_Unsigned(value));
                return 1;
            }
            static gep::uint32 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::uint32");
                auto value = lua_tounsigned(L, idx);
                return gep::uint32(value);
            }
        };

        template <>
        struct typeHandling<gep::uint64>
        {
            static int push(lua_State* L, gep::uint64 value)
            {
                lua_pushunsigned(L, lua_Unsigned(value));
                return 1;
            }
            static gep::uint64 pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "gep::uint64");
                auto value = lua_tounsigned(L, idx);
                return gep::uint64(value);
            }
        };

        template <>
        struct typeHandling<float>
        {
            static int push(lua_State* L, float value)
            {
                lua_pushnumber(L, lua_Number(value));
                return 1;
            }
            static float pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "float");
                auto value = lua_tonumber(L, idx);
                return float(value);
            }
        };

        template <>
        struct typeHandling<double>
        {
            static int push(lua_State* L, double value)
            {
                lua_pushnumber(L, lua_Number(value));
                return 1;
            }
            static double pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TNUMBER, idx, "double");
                auto value = lua_tonumber(L, idx);
                return double(value);
            }
        };

        template <>
        struct typeHandling<bool>
        {
            static int push(lua_State* L, bool value)
            {
                lua_pushboolean(L, value ? 1 : 0);
                return 1;
            }
            static bool pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TBOOLEAN, idx);
                auto value = lua_toboolean(L, idx) != 0;
                return value;
            }
        };

        template <>
        struct typeHandling<const char*>
        {
            static int push(lua_State* L, const char* value)
            {
                lua_pushstring(L, value);
                return 1;
            }
            static const char* pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TSTRING, idx);
                auto value = lua_tostring(L, idx);
                return value;
            }
        };

        template <>
        struct typeHandling<std::string>
        {
            static int push(lua_State* L, const std::string& value)
            {
                lua_pushstring(L, value.c_str());
                return 1;
            }
            static std::string pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TSTRING, idx);
                auto value = lua_tostring(L, idx);
                return value;
            }
        };

        template <>
        struct typeHandling<void*>
        {
            static int push(lua_State* L, void* pointer)
            {
                lua_pushlightuserdata(L, pointer);
                return 1;
            }
            static void* pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TLIGHTUSERDATA, idx);
                void* value = const_cast<void*>(lua_topointer(L, idx));
                return value;
            }
        };

        template <>
        struct typeHandling<void>
        {
            static int push(lua_State*)
            {
                return 0;
            }
            static void pop(lua_State*, int)
            {
                return;
            }
        };
    }
}
