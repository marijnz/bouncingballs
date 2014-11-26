#pragma once

#include <tuple>
#include <string>
#include "gep/utils.h"

#include "gep/scripting/luaUtils.h"
#include "gep/scripting/luaFunctionWrapper.h"
#include "gep/scripting/luaTableWrapper.h"
#include "gep/scripting/luaTypeHandling.h"
#include "gep/scripting/luaEnumOrTypeHandling.h"
#include "gep/scripting/luaReferenceCounting.h"

#define __RM_P(T)      std::remove_pointer<T>::type
#define __RM_C(T)      std::remove_const<T>::type
#define __RM_C_R(T)    std::remove_const<std::remove_reference<T>::type>::type

/// \source
///  http://stackoverflow.com/questions/16387354/template-tuple-calling-a-function-on-each-element
///
#if _MSC_VER >= 1800
namespace tuple
{
    template<int... Is>
    struct seq { };

    template<int N, int... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

    template<int... Is>
    struct gen_seq<0, Is...> : seq<Is...> { };

    template<typename T, typename F, int... Is>
    void for_each(T&& t, F f, seq<Is...>)
    {
        auto l = { (f(std::get<Is>(t)), 0)... };
    }

    template<typename... Ts, typename F>
    void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
    {
        for_each(t, f, gen_seq<sizeof...(Ts)>());
    }

    struct push_functor
    {
        template <typename T>
        void operator () (T t)
        {
            lua::push<T>(t);
        }
    };
}
#endif

namespace lua
{
    // typemarkers
    namespace structs
    {
        struct ReferenceTypeMarker { };
        struct ValueTypeMarker { };
        struct FunctionRefTypeMarker { };
    }

    namespace structs
    {
        template <typename T>
        struct objectHandling { };

        template <>
        struct objectHandling<ReferenceTypeMarker>
        {
            template <typename U>
            static U pop(lua_State* L, int idx)
            {
                lua_getfield(L, idx, "__ptr");
                void* __ptr = typeHandling<void*>::pop(L, -1);
                lua_pop(L, 1);

                return reinterpret_cast<U>(__ptr);
            }

            template <typename T_Ptr>
            static int push(lua_State* L, T_Ptr pObject)
            {
                typedef __RM_P(T_Ptr) T;

                lua_newtable(L);
                auto tableIndex = lua_gettop(L);
                pushTableEntry<const char*, void*>(L, "__ptr", pObject);

                lua_pushcfunction(L, lua::utils::ptrCompare);
                lua_setfield(L, tableIndex, "equals");

                lua_pushcfunction(L, lua::utils::nullCheck);
                lua_setfield(L, tableIndex, "isNull");

                auto& scriptTypeInfo = gep::getScriptTypeInfo<T>();

                luaL_setmetatable(L, scriptTypeInfo.getMetaTableName());
                IncreaseReferenceCount<std::is_convertible<T_Ptr, gep::ReferenceCounted*>::value>::addRef(pObject);
                return 1;
            }
        };

        template <>
        struct objectHandling<ValueTypeMarker>
        {
            template <typename U>
            static U pop(lua_State* L, int idx)
            {
                U object;
                popValueType(L, object, idx);
                return object;
            }

            template <typename U>
            static int push(lua_State* L, U& object)
            {
                pushValueType(L, object);
                return 1;
            }
        };

        template <typename T, bool is_object>
        struct object_or_typeHandling { };

        template <typename T>
        struct object_or_typeHandling<T, false>
        {
            static int push(lua_State* L, T t)
            {
                return enum_or_typeHandling<T, std::is_enum<T>::value>::push(L, t);
            }
            static T pop(lua_State* L, int idx)
            {
                return enum_or_typeHandling<T, std::is_enum<T>::value>::pop(L, idx);
            }
        };

        template <typename T>
        struct object_or_typeHandling<T, true>
        {
            static int push(lua_State* L, T t)
            {
                return objectHandling<typename __RM_P(T)::LuaType>::push<T>(L, t);
            }
            static T pop(lua_State* L, int idx)
            {
                return objectHandling<typename __RM_P(T)::LuaType>::pop<T>(L, idx);
            }
        };

        template <>
        struct object_or_typeHandling<void, false>
        {
            static int push(lua_State*)
            {
                return 0;
            }
            static void pop(lua_State*, int)
            {
            }
        };

        template <>
        struct object_or_typeHandling<FunctionWrapper, true>
        {
            static int push(lua_State* L, FunctionWrapper func)
            {
                func.push();
                return 1;
            }

            static FunctionWrapper pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TFUNCTION, idx);
                return FunctionWrapper(L, idx);
            }
        };

        template <>
        struct object_or_typeHandling<TableWrapper, true>
        {
            static int push(lua_State* L, TableWrapper table)
            {
                table.push();
                return 1;
            }

            static TableWrapper pop(lua_State* L, int idx)
            {
                utils::typeCheck(L, LUA_TTABLE, idx);
                return TableWrapper(L, idx);
            }
        };

        template <typename T>
        struct specializedTypeHandling { };

        template <>
        struct specializedTypeHandling<ReferenceTypeMarker>
        {
            template <typename U>
            static U& pop(lua_State* L, int idx)
            {
                return *objectHandling<ReferenceTypeMarker>::pop<U*>(L, idx);
            }
        };

        template <>
        struct specializedTypeHandling<ValueTypeMarker>
        {
            template <typename U>
            static U pop(lua_State* L, int idx)
            {
                return objectHandling<ValueTypeMarker>::pop<U>(L, idx);
            }
        };

#if _MSC_VER >= 1800
        template <typename T, typename R, typename... Args>
        struct bindHelper { };
#else
        template <typename BT, typename T, typename R, typename A1 = void, typename A2 = void, typename A3 = void, typename A4 = void, typename A5 = void, typename A6 = void>
        struct bindHelper { };
#endif

        template <typename BT, typename T>
        struct bindHelper<BT, T, void>
        {
            template<void(T::*F)()>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)()>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(); return 0; }
        };

        template <typename BT, typename T, typename R>
        struct bindHelper<BT, T, R>
        {
            template<R(T::*F)()>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)()>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)()); return 1; }
        };

        template <typename BT, typename T, typename A1>
        struct bindHelper<BT, T, void, A1>
        {
            template<void(T::*F)(A1)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1)>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1>
        struct bindHelper<BT, T, R, A1>
        {
            template<R(T::*F)(A1)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1)>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2>
        struct bindHelper<BT, T, void, A1, A2>
        {
            template<void(T::*F)(A1, A2)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2)>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2>
        struct bindHelper<BT, T, R, A1, A2>
        {
            template<R(T::*F)(A1, A2)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2)>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2, typename A3>
        struct bindHelper<BT, T, void, A1, A2, A3>
        {
            template<void(T::*F)(A1, A2, A3)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3)>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2, typename A3>
        struct bindHelper<BT, T, R, A1, A2, A3>
        {
            template<R(T::*F)(A1, A2, A3)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3)>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2, typename A3, typename A4>
        struct bindHelper<BT, T, void, A1, A2, A3, A4>
        {
            template<void(T::*F)(A1, A2, A3, A4)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3, A4)>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4), pop<__RM_C_R(A4)>(L, 5)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4>
        struct bindHelper<BT, T, R, A1, A2, A3, A4>
        {
            template<R(T::*F)(A1, A2, A3, A4)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3, A4)>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4), pop<__RM_C_R(A4)>(L, 5))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
        struct bindHelper<BT, T, void, A1, A2, A3, A4, A5>
        {
            template<void(T::*F)(A1, A2, A3, A4, A5)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3, A4, A5)>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4), pop<__RM_C_R(A4)>(L, 5), pop<__RM_C_R(A5)>(L, 6)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
        struct bindHelper<BT, T, R, A1, A2, A3, A4, A5>
        {
            template<R(T::*F)(A1, A2, A3, A4, A5)>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3, A4, A5)>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4), pop<__RM_C_R(A4)>(L, 5), pop<__RM_C_R(A5)>(L, 6))); return 1; }
        };

#if _MSC_VER >= 1800
        template <typename T, typename R, typename... Args>
        struct bindHelperConst { };
#else
        template <typename BT, typename T, typename R, typename A1 = void, typename A2 = void, typename A3 = void, typename A4 = void, typename A5 = void, typename A6 = void>
        struct bindHelperConst { };
#endif

        template <typename BT, typename T>
        struct bindHelperConst<BT, T, void>
        {
            template<void(T::*F)() const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)() const>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(); return 0; }
        };

        template <typename BT, typename T, typename R>
        struct bindHelperConst<BT, T, R>
        {
            template<R(T::*F)() const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)() const>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)()); return 1; }
        };

        template <typename BT, typename T, typename A1>
        struct bindHelperConst<BT, T, void, A1>
        {
            template<void(T::*F)(A1) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1) const>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1>
        struct bindHelperConst<BT, T, R, A1>
        {
            template<R(T::*F)(A1) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1) const>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2>
        struct bindHelperConst<BT, T, void, A1, A2>
        {
            template<void(T::*F)(A1, A2) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2) const>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2>
        struct bindHelperConst<BT, T, R, A1, A2>
        {
            template<R(T::*F)(A1, A2) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2) const>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2, typename A3>
        struct bindHelperConst<BT, T, void, A1, A2, A3>
        {
            template<void(T::*F)(A1, A2, A3) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3) const>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2, typename A3>
        struct bindHelperConst<BT, T, R, A1, A2, A3>
        {
            template<R(T::*F)(A1, A2, A3) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3) const>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4))); return 1; }
        };

        template <typename BT, typename T, typename A1, typename A2, typename A3, typename A4>
        struct bindHelperConst<BT, T, void, A1, A2, A3, A4>
        {
            template<void(T::*F)(A1, A2, A3, A4) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<void(T::*F)(A1, A2, A3, A4) const>
            static int lua_CFunction(lua_State* L) { (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4), pop<__RM_C_R(A4)>(L, 5)); return 0; }
        };

        template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4>
        struct bindHelperConst<BT, T, R, A1, A2, A3, A4>
        {
            template<R(T::*F)(A1, A2, A3, A4) const>
            void bind(lua_State* L, const char* name) { lua_pushcfunction(L, lua_CFunction<F>); lua_setfield(L, -2, name); }
            template<R(T::*F)(A1, A2, A3, A4) const>
            static int lua_CFunction(lua_State* L) { push<R>(L, (specializedTypeHandling<typename BT::LuaType>::pop<T>(L, 1).*F)(pop<__RM_C_R(A1)>(L, 2), pop<__RM_C_R(A2)>(L, 3), pop<__RM_C_R(A3)>(L, 4), pop<__RM_C_R(A4)>(L, 5))); return 1; }
        };

    } // structs

    template <typename T>
    int push(lua_State* L, T t)
    {
        return structs::object_or_typeHandling<__RM_C(T), std::is_class<__RM_P(T)>::value && !std::is_same<__RM_C_R(T), std::string>::value>::push(L, (__RM_C(T))t);
    }

    template <typename T>
    T pop(lua_State* L, int idx)
    {
        return structs::object_or_typeHandling<T, std::is_class<__RM_P(T)>::value && !std::is_same<__RM_C_R(T), std::string>::value>::pop(L, idx);
    }

    template <typename T>
    int countReturnValues()
    {
        return std::is_void<T>::value ? 0 : 1;
    }

    template <typename K, typename V>
    void pushTableEntry(lua_State* L, K k, V v)
    {
        push<K>(L, k);
        push<V>(L, v);
        lua_settable(L, -3);
    }

    template <typename T>
    int pushValueType(lua_State* L, T& t)
    {
        lua_newtable(L);
        t.Lua_TableValueType<T>(L, true, 0);
        return 1;
    }

    template <typename T>
    void popValueType(lua_State* L, T& t, int idx)
    {
        bool isTable = lua_istable(L, idx);
        t.Lua_TableValueType<T>(L, false, idx, !isTable);
    }

    // Non-Const versions
    //////////////////////////////////////////////////////////////////////////
    template <typename BT, typename T, typename R>
    inline structs::bindHelper<BT, T, R> bind(R(T::*func)()) { return structs::bindHelper<BT, T, R>(); }

    template <typename BT, typename T, typename R, typename A1>
    inline structs::bindHelper<BT, T, R, A1> bind(R(T::*func)(A1)) { return structs::bindHelper<BT, T, R, A1>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2>
    inline structs::bindHelper<BT, T, R, A1, A2> bind(R(T::*func)(A1, A2)) { return structs::bindHelper<BT, T, R, A1, A2>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2, typename A3>
    inline structs::bindHelper<BT, T, R, A1, A2, A3> bind(R(T::*func)(A1, A2, A3)) { return structs::bindHelper<BT, T, R, A1, A2, A3>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4>
    inline structs::bindHelper<BT, T, R, A1, A2, A3, A4> bind(R(T::*func)(A1, A2, A3, A4)) { return structs::bindHelper<BT, T, R, A1, A2, A3, A4>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    inline structs::bindHelper<BT, T, R, A1, A2, A3, A4, A5> bind(R(T::*func)(A1, A2, A3, A4, A5)) { return structs::bindHelper<BT, T, R, A1, A2, A3, A4, A5>(); }

    // Const versions
    //////////////////////////////////////////////////////////////////////////
    template <typename BT, typename T, typename R>
    inline structs::bindHelperConst<BT, T, R> bind(R(T::*func)() const) { return structs::bindHelperConst<BT, T, R>(); }

    template <typename BT, typename T, typename R, typename A1>
    inline structs::bindHelperConst<BT, T, R, A1> bind(R(T::*func)(A1) const) { return structs::bindHelperConst<BT, T, R, A1>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2>
    inline structs::bindHelperConst<BT, T, R, A1, A2> bind(R(T::*func)(A1, A2) const) { return structs::bindHelperConst<BT, T, R, A1, A2>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2, typename A3>
    inline structs::bindHelperConst<BT, T, R, A1, A2, A3> bind(R(T::*func)(A1, A2, A3) const) { return structs::bindHelperConst<BT, T, R, A1, A2, A3>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4>
    inline structs::bindHelperConst<BT, T, R, A1, A2, A3, A4> bind(R(T::*func)(A1, A2, A3, A4) const) { return structs::bindHelperConst<BT, T, R, A1, A2, A3, A4>(); }

    template <typename BT, typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    inline structs::bindHelperConst<BT, T, R, A1, A2, A3, A4, A5> bind(R(T::*func)(A1, A2, A3, A4, A5) const) { return structs::bindHelperConst<BT, T, R, A1, A2, A3, A4, A5>(); }
}

#include "gep/scripting/luaMacros.h"
