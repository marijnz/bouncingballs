#pragma once

#include "gep/interfaces/scripting.h"

namespace gep
{
    struct EventId
    {
        static EventId invalidValue()
        {
            static EventId invalid(std::numeric_limits<uint16>::max());
            return invalid;
        }

        static EventId generate()
        {
            static uint16 counter(0);
            return EventId(++counter);
        }

        uint16 value;

        EventId()
        {
            *this = invalidValue();
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(value)
        LUA_BIND_VALUE_TYPE_END

    private:
        EventId(uint16 value) : value(value) {}
    };

    inline bool operator == (EventId lhs, EventId rhs)
    {
        return lhs.value == rhs.value;
    }

    inline bool operator != (EventId lhs, EventId rhs)
    {
        return lhs.value == rhs.value;
    }

    template<typename T_Event>
    struct EventListenerId
    {
        static EventListenerId invalidValue()
        {
            static EventListenerId invalid(std::numeric_limits<uint16>::max());
            return invalid;
        }

        static EventListenerId generate()
        {
            static uint16 counter(0);
            return EventListenerId(++counter);
        }

        uint16 value;

        EventListenerId()
        {
            *this = invalidValue();
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(value)
        LUA_BIND_VALUE_TYPE_END

    private:
        EventListenerId(uint16 value) :
            value(value)
        {
        }
    };

    template<typename T_Event>
    inline bool operator == (const EventListenerId<T_Event>& lhs, const EventListenerId<T_Event>& rhs)
    {
        return lhs.value == rhs.value;
    }

    template<typename T_Event>
    inline bool operator != (const EventListenerId<T_Event>& lhs, const EventListenerId<T_Event>& rhs)
    {
        return lhs.value != rhs.value;
    }

    template<typename T_Event>
    struct DelayedEventId
    {
        static DelayedEventId invalidValue()
        {
            static DelayedEventId invalid(std::numeric_limits<uint16>::max());
            return invalid;
        }

        static DelayedEventId generate()
        {
            static uint16 counter(0);
            return DelayedEventId(++counter);
        }

        uint16 value;

        DelayedEventId()
        {
            *this = invalidValue();
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(value)
        LUA_BIND_VALUE_TYPE_END

    private:
        DelayedEventId(uint16 value) :
            value(value)
        {
        }
    };

    template<typename T_Event>
    inline bool operator == (const DelayedEventId<T_Event>& lhs, const DelayedEventId<T_Event>& rhs)
    {
        return lhs.value == rhs.value;
    }

    template<typename T_Event>
    inline bool operator != (const DelayedEventId<T_Event>& lhs, const DelayedEventId<T_Event>& rhs)
    {
        return lhs.value != rhs.value;
    }
}
