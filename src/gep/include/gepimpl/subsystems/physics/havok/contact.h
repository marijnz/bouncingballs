#pragma once
#include "gep/interfaces/physics/contact.h"

namespace gep
{
    class HavokContactPointArgs : public ContactPointArgs
    {
        const hkpContactPointEvent& m_hkpEvent;
    public:
        HavokContactPointArgs(const hkpContactPointEvent& evt) :
            ContactPointArgs(),
            m_hkpEvent(evt)
        {
        }

        HavokContactPointArgs(const hkpContactPointEvent& evt, CallbackSource::Enum source, IRigidBody* first, IRigidBody* second) :
            ContactPointArgs(source, first, second),
            m_hkpEvent(evt)
        {
        }

        virtual void accessVelocities(int32 bodyIndex) const override
        {
            m_hkpEvent.accessVelocities(bodyIndex);
        }

        virtual void updateVelocities(int32 bodyIndex) const override
        {
            m_hkpEvent.updateVelocities(bodyIndex);
        }
    };
    
    class HavokContactListener : public hkpContactListener
    {
        IContactListener* m_pListener;
    public:

        HavokContactListener(IContactListener* listener) : m_pListener(listener)
        {
            GEP_ASSERT(m_pListener, "nullptr not allowed as contact listener!");
        }
        virtual ~HavokContactListener(){ m_pListener = nullptr; }

        virtual void contactPointCallback(const hkpContactPointEvent& evt) override
        {
            auto source = static_cast<CollisionArgs::CallbackSource::Enum>(evt.m_source);
            auto* first = reinterpret_cast<IRigidBody*>(evt.m_bodies[0]->getUserData());
            auto* second = reinterpret_cast<IRigidBody*>(evt.m_bodies[1]->getUserData());
            GEP_ASSERT(m_pListener, "I have a nullptr as actual listener!");
            GEP_ASSERT(first, "Havok user data is null!");
            GEP_ASSERT(second, "Havok user data is null!");
            m_pListener->contactPointCallback(HavokContactPointArgs(evt, source, first, second));
        }

        virtual void collisionAddedCallback(const hkpCollisionEvent& evt) override
        {
            auto source = static_cast<CollisionArgs::CallbackSource::Enum>(evt.m_source);
            auto* first = reinterpret_cast<IRigidBody*>(evt.m_bodies[0]->getUserData());
            auto* second = reinterpret_cast<IRigidBody*>(evt.m_bodies[1]->getUserData());
            GEP_ASSERT(m_pListener, "I have a nullptr as actual listener!");
            GEP_ASSERT(first, "Havok user data is null!");
            GEP_ASSERT(second, "Havok user data is null!");
            m_pListener->collisionAddedCallback(CollisionArgs(source, first, second));
        }

        virtual void collisionRemovedCallback(const hkpCollisionEvent& evt) override
        {
            auto source = static_cast<CollisionArgs::CallbackSource::Enum>(evt.m_source);
            auto* first = reinterpret_cast<IRigidBody*>(evt.m_bodies[0]->getUserData());
            auto* second = reinterpret_cast<IRigidBody*>(evt.m_bodies[1]->getUserData());
            GEP_ASSERT(m_pListener, "I have a nullptr as actual listener!");
            GEP_ASSERT(first, "Havok user data is null!");
            GEP_ASSERT(second, "Havok user data is null!");
            m_pListener->collisionRemovedCallback(CollisionArgs(source, first, second));
        }

        virtual void contactPointAddedCallback(hkpContactPointAddedEvent& evt) override
        {
            // deprecated?
        }

        virtual void contactPointRemovedCallback(hkpContactPointRemovedEvent& evt) override
        {
            // deprecated?
        }

        virtual void contactProcessCallback(hkpContactProcessEvent& evt) override
        {
            // deprecated?
        }

        inline       IContactListener* getActualListener()       { return m_pListener; }
        inline const IContactListener* getActualListener() const { return m_pListener; }
    };
}
