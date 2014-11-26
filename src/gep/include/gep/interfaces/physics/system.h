#pragma once
#include "gep/interfaces/subsystem.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    class IWorld;
    class IPhysicsFactory;

    class IPhysicsSystem : public ISubsystem
    {
    public:
        virtual ~IPhysicsSystem() {}

        virtual void setDebugDrawingEnabled(bool value) = 0;
        virtual bool getDebugDrawingEnabled() const = 0;

        virtual void setWorld(IWorld* world) = 0;
        virtual IWorld* getWorld() = 0;
        virtual const IWorld* getWorld() const = 0;

        virtual IPhysicsFactory* getPhysicsFactory() = 0;
        
        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setDebugDrawingEnabled)
            LUA_BIND_FUNCTION(getDebugDrawingEnabled)
            LUA_BIND_FUNCTION(setWorld)
        LUA_BIND_REFERENCE_TYPE_END
    };
}
