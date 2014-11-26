#pragma once

// You should not use this macro directly!
#define _LUA_BIND_TYPE_BEGIN public:                                                        \
    template <typename __T>                                                                 \
    static void Lua_Bind(lua_State* L, const char* className)                               \
    {                                                                                       \
        auto& scriptInfo = gep::getScriptTypeInfo<__T>();                                   \
        if(scriptInfo.getNumBindings() > 0) { return; }                                     \
        scriptInfo.addNumBindings();                                                        \
        lua::utils::StackChecker checker(L, 0);                                             \
        /* remember the className */                                                        \
        scriptInfo.setClassName(className);                                                 \
        /* create new metatable and store it in the registry */                             \
        luaL_newmetatable(L, scriptInfo.getMetaTableName());                                \
        auto metaTableIndex = lua_gettop(L);                                                \
                                                                                            \
        /* TODO inheritance */                                                              \
        /* Class_Sub -> setmetatable(Class_Sub_Meta, Class_Meta) */                         \
                                                                                            \
        /* add __index to metatable*/                                                       \
        lua_pushvalue(L, metaTableIndex);                                                   \
        lua_setfield(L, metaTableIndex, "__index");                                         \
        lua::structs::DecreaseReferenceCount<__T*, std::is_convertible<__T*, gep::ReferenceCounted*>::value>::addGcEntry(L);

#define LUA_BIND_FUNCTION_PTR(pFunction, funcName) \
    lua::bind<__T>(pFunction).bind<pFunction>(L, funcName);
#define LUA_BIND_FUNCTION_NAMED(function, name) LUA_BIND_FUNCTION_PTR(&function, name)
#define LUA_BIND_FUNCTION(function) LUA_BIND_FUNCTION_NAMED(function, #function)

#define LUA_BIND_REFERENCE_TYPE_BEGIN public:          \
    typedef lua::structs::ReferenceTypeMarker LuaType; \
    _LUA_BIND_TYPE_BEGIN

#define LUA_BIND_REFERENCE_TYPE_END \
        lua_pop(L, 1);              \
    }

#define LUA_BIND_VALUE_TYPE_BEGIN public:          \
    typedef lua::structs::ValueTypeMarker LuaType; \
    _LUA_BIND_TYPE_BEGIN

#define LUA_BIND_VALUE_TYPE_MEMBERS                                                   \
        /* add "create" function to metatable */                                      \
        lua_pushcfunction(L, Lua_Create<__T>);                                        \
        /* local t = T(params) OR local t = T({params}) */                            \
        lua_setglobal(L, gep::getScriptTypeInfo<__T>().getClassName());        \
        lua_pop(L, 1);                                                                \
    }                                                                                 \
    template <typename __T>                                                           \
    static int Lua_Create(lua_State* L)                                               \
    {                                                                                 \
        __T t;                                                                        \
        /* get constructor parameters from stack */                                   \
        if (lua_gettop(L) >= 1)                                                       \
            lua::popValueType(L, t, 1);                                               \
        lua::pushValueType(L, t);                                                     \
        return 1;                                                                     \
    }                                                                                 \
    template <typename __T>                                                           \
    void Lua_TableValueType(lua_State* L, bool push, int idx, bool popParams = false) \
    {

#define LUA_BIND_MEMBER_NAMED(memberVariable, luaName)                         \
        if (push) {                                                            \
            lua::pushTableEntry<const char*, decltype(memberVariable)>(L,      \
                luaName, memberVariable);                                      \
        }                                                                      \
        else {                                                                 \
            if (popParams) {                                                   \
                memberVariable = lua::pop<decltype(memberVariable)>(L, idx++); \
            }                                                                  \
            else {                                                             \
                lua_getfield(L, idx, luaName);                                 \
                memberVariable = lua::pop<decltype(memberVariable)>(L, -1);    \
                lua_pop(L, 1);                                                 \
            }                                                                  \
        }

#define LUA_BIND_MEMBER(memberVariable) LUA_BIND_MEMBER_NAMED(memberVariable, #memberVariable)

#define LUA_BIND_VALUE_TYPE_END                                                                  \
        if (push) luaL_setmetatable(L, gep::getScriptTypeInfo<__T>().getMetaTableName()); \
    }
