#pragma once

#include "gep/interfaces/physics/rayCast.h"

#include "gep/interfaces/events.h"
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/physics/constraints.h"

namespace gep
{
    class IPhysicsEntity;
    class ICharacterRigidBody;
    class CollisionArgs;
    class ContactPointArgs;

    class ICollisionFilter : public ReferenceCounted
    {
    public:
        virtual ~ICollisionFilter() = 0 {}

        LUA_BIND_REFERENCE_TYPE_BEGIN
        LUA_BIND_REFERENCE_TYPE_END
    };

    /// \brief Construction info for a physics world instance.
    struct WorldCInfo
    {
        vec3 gravity;
        float worldSize;

        inline WorldCInfo() :
            gravity(0.0f),
            worldSize(100.0f)
        {
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(gravity)
            LUA_BIND_MEMBER(worldSize)
        LUA_BIND_VALUE_TYPE_END
    };

    class IWorld : public ReferenceCounted
    {
    public:
        struct ScopedRead
        {
        private:
            IWorld* m_pWorld;

        public:
            ScopedRead(IWorld* pWorld) : m_pWorld(pWorld) { m_pWorld->markForRead(); }
            ~ScopedRead() { m_pWorld->unmarkForRead(); m_pWorld = nullptr; }
        };

        struct ScopedWrite
        {
        private:
            IWorld* m_pWorld;

        public:
            ScopedWrite(IWorld* pWorld) : m_pWorld(pWorld) { m_pWorld->markForWrite(); }
            ~ScopedWrite() { m_pWorld->unmarkForWrite(); m_pWorld = nullptr; }
        };

        struct ScopedLock
        {
        private:
            IWorld* m_pWorld;

        public:
            ScopedLock(IWorld* pWorld) : m_pWorld(pWorld) { m_pWorld->lock(); }
            ~ScopedLock() { m_pWorld->unlock(); m_pWorld = nullptr; }
        };
    public:
        virtual ~IWorld(){}

        /// \brief Adds a physics entity to this world.
        /// \param own If set to \c TakeOwnership::yes, this entity will be destroying once the world is destroyed.
        virtual void addEntity(IPhysicsEntity* entity) = 0;

        /// \brief Removes an entity from this world and transfers the ownership of it to the caller.
        virtual void removeEntity(IPhysicsEntity* entity) = 0;

        /// \brief Adds a character to the world.
        ///
        /// Will automatically add the underlying physics entity to this world as well.
        virtual void addCharacter(ICharacterRigidBody* character) = 0;

        /// \brief Removes a character from this world.
        ///
        /// Will automatically remove the underlying physics entity from this world as well.
        /// The ownership of the character rigid body is transfered to the caller.
        virtual void removeCharacter(ICharacterRigidBody* character) = 0;

        /// \brief
        //////////////////////////////////////////////////////////////////////////
        virtual void setCollisionFilter(ICollisionFilter* pFilter) = 0;

        /// \brief Register a contact listener for all collision events
        virtual Event<ContactPointArgs*>* getContactPointEvent() = 0;
        virtual Event<CollisionArgs*>* getCollisionAddedEvent() = 0;
        virtual Event<CollisionArgs*>* getCollisionRemovedEvent() = 0;

        /// \brief Casts a ray defined in \a input into this world. The output is represented by the \a output argument.
        virtual void castRay(const RayCastInput& input, RayCastOutput& output) const = 0;

        virtual void   markForRead() const = 0;
        virtual void unmarkForRead() const = 0;
        virtual void   markForWrite() = 0;
        virtual void unmarkForWrite() = 0;
        virtual void   lock() = 0;
        virtual void unlock() = 0;

        // Constraints

        virtual void addConstraint(Constraint constraint) = 0;
        virtual void removeConstraint(Constraint constraint) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_PTR(static_cast<RayCastOutput(IWorld::*)(const RayCastInput&)>(&castRay), "castRay")
            LUA_BIND_FUNCTION(setCollisionFilter)
            LUA_BIND_FUNCTION(getContactPointEvent)
            LUA_BIND_FUNCTION(getCollisionAddedEvent)
            LUA_BIND_FUNCTION(getCollisionRemovedEvent)
            LUA_BIND_FUNCTION(addConstraint)
            LUA_BIND_FUNCTION(removeConstraint)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        RayCastOutput castRay(const RayCastInput& input)
        {
            RayCastOutput out;
            castRay(input, out);
            return out;
        }
    };
}
