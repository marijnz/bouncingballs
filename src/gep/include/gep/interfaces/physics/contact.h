#pragma once

#include "gep/math3d/vec3.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    class IRigidBody;

    class CollisionArgs
    {
    public:
        struct CallbackSource
        {
            enum Enum
            {
                Invalid = -1,
                A,
                B,
                World
            };
        };

        CollisionArgs() :
            m_source(CallbackSource::Invalid),
            m_first(nullptr),
            m_second(nullptr)
        {
        }

        CollisionArgs(CallbackSource::Enum source, IRigidBody* first, IRigidBody* second) :
            m_source(source),
            m_first(first),
            m_second(second)
        {
        }

        virtual ~CollisionArgs() {}

        CallbackSource::Enum getSource() const { return m_source; }

        IRigidBody* getBody(int32 index)
        {
            GEP_ASSERT(index == 0 || index == 1, "Index is out of range!", index);
            return m_bodies[index];
        }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getBody)
            LUA_BIND_FUNCTION(getSource)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        CallbackSource::Enum m_source;
        union
        {
            struct
            {
                IRigidBody* m_first;
                IRigidBody* m_second;
            };
            IRigidBody* m_bodies[2];
        };
    };

    class ContactPointArgs : public CollisionArgs
    {
    public:

        ContactPointArgs() :
            CollisionArgs()
        {
        }

        ContactPointArgs(CallbackSource::Enum source, IRigidBody* first, IRigidBody* second) :
            CollisionArgs(source, first, second)
        {
        }

        virtual void accessVelocities(int32 bodyIndex) const = 0;
        virtual void updateVelocities(int32 bodyIndex) const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getBody)
            LUA_BIND_FUNCTION(getSource)
            LUA_BIND_FUNCTION(accessVelocities)
            LUA_BIND_FUNCTION(updateVelocities)
        LUA_BIND_REFERENCE_TYPE_END
    };

    class IContactListener
    {
    public:
        virtual ~IContactListener(){}

        virtual void contactPointCallback( const ContactPointArgs& evt ) = 0;
        virtual void collisionAddedCallback( const CollisionArgs& evt ) = 0;
        virtual void collisionRemovedCallback( const CollisionArgs& evt ) = 0;
    };
}
