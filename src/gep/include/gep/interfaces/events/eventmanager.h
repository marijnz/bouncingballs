#pragma once

#include "gep/interfaces/subsystem.h"

#include "gep/interfaces/events/event.h"

namespace gep
{
    class IEventManager : public ISubsystem
    {
    public:
        virtual ~IEventManager() = 0 {}

        virtual Event<float>* getUpdateEvent() = 0;
        virtual Event<nullptr_t>* getPostInitializationEvent() = 0;

        virtual Event<ScriptTableWrapper>* createScriptTableEvent() = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getUpdateEvent)
            LUA_BIND_FUNCTION(getPostInitializationEvent)
            LUA_BIND_FUNCTION_NAMED(createScriptTableEvent, "createGenericEvent")
        LUA_BIND_REFERENCE_TYPE_END
    };
}
