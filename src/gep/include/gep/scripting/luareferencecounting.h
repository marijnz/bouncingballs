#pragma once

namespace lua
{
    namespace structs
    {
        template<bool isRefCounted>
        struct IncreaseReferenceCount;

        template<typename T, bool isRefCounted>
        struct DecreaseReferenceCount;

        //////////////////////////////////////////////////////////////////////////

        template<>
        struct IncreaseReferenceCount<false>
        {
            template<typename T>
            static void addRef(T obj)
            {
            }
        };

        template<>
        struct IncreaseReferenceCount<true>
        {
            template<typename T>
            static void addRef(T obj)
            {
                if(obj) obj->addReference();
            }
        };

        template<typename T>
        struct DecreaseReferenceCount<T, false>
        {
            static void addGcEntry(lua_State* L)
            {
            }
        };

        template<typename T>
        struct DecreaseReferenceCount<T, true>
        {
            static void addGcEntry(lua_State* L)
            {
                utils::StackChecker check(L, 0);

                lua_pushcfunction(L, &removeRef);
                lua_setfield(L, -2, "__gc");
            }

            static int removeRef(lua_State* L)
            {
                auto ptr = objectHandling<ReferenceTypeMarker>::pop<T>(L, 1);
                if(ptr) ptr->removeReference();
                return 0;
            }
        };
    }
}
