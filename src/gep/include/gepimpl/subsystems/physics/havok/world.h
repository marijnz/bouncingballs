#pragma once
#include "gep/interfaces/physics/world.h"
#include "gep/interfaces/events.h"
#include "gep/container/DynamicArray.h"
#include "gep/interfaces/physics/contact.h"
#include "gepimpl/subsystems/physics/havok/contact.h"

namespace gep
{
    class IPhysicsEntity;

    class HavokBaseAction;

    class HavokCollisionFilterWrapper : public ICollisionFilter
    {
        hkRefPtr<hkpCollisionFilter> m_pHkCollisionFilter;
    public:
        HavokCollisionFilterWrapper(hkpCollisionFilter* pFilter) :
            m_pHkCollisionFilter(pFilter)
        {
        }

        hkpCollisionFilter* getHkCollisionFilter() { return m_pHkCollisionFilter; }
    };

    class HavokCollisionFilter_Simple :
        public hkpCollisionFilter
    {
    public:

        virtual hkBool isCollisionEnabled(const hkpCollidable& a,
                                          const hkpCollidable& b) const override;
        virtual hkBool isCollisionEnabled(const hkpCollisionInput& input,
                                          const hkpCdBody& a,
                                          const hkpCdBody& b,
                                          const HK_SHAPE_CONTAINER& bContainer,
                                          hkpShapeKey bKey) const override;
        virtual hkBool isCollisionEnabled(const hkpCollisionInput& input,
                                          const hkpCdBody& collectionBodyA,
                                          const hkpCdBody& collectionBodyB,
                                          const HK_SHAPE_CONTAINER& containerShapeA,
                                          const HK_SHAPE_CONTAINER& containerShapeB,
                                          hkpShapeKey keyA,
                                          hkpShapeKey keyB) const override;
        virtual hkBool isCollisionEnabled(const hkpShapeRayCastInput& aInput,
                                          const HK_SHAPE_CONTAINER& bContainer,
                                          hkpShapeKey bKey) const override;
        virtual hkBool isCollisionEnabled(const hkpWorldRayCastInput& a,
                                          const hkpCollidable& collidableB) const override;

    protected:

        // The base collision filtering logic - all the other function calls will forward to this one
        hkBool isCollisionEnabled(hkUint32 infoA, hkUint32 infoB) const;
    };


    class HavokWorld : public IWorld, public IContactListener
    {
        hkRefPtr<hkpWorld> m_pWorld;

        DynamicArray< SmartPtr<IPhysicsEntity> > m_entities;
        DynamicArray< SmartPtr<ICharacterRigidBody> > m_characters;
        DynamicArray<HavokContactListener*> m_contactListeners;
        HavokContactListener m_actualContactListener;
        gep::Event<gep::ContactPointArgs*> m_event_contactPoint;
        gep::Event<gep::CollisionArgs*> m_event_collisionAdded;
        gep::Event<gep::CollisionArgs*> m_event_collisionRemoved;
    public:
        HavokWorld(const WorldCInfo& cinfo);

        virtual ~HavokWorld();

        virtual void addEntity(IPhysicsEntity* entity) override;
        void addEntity(hkpEntity* entity);
        virtual void removeEntity(IPhysicsEntity* entity);
        void removeEntity(hkpEntity* entity);

        virtual void addCharacter(ICharacterRigidBody* character) override;
        virtual void removeCharacter(ICharacterRigidBody* character) override;

        void update(float elapsedTime);

        virtual void castRay(const RayCastInput& input, RayCastOutput& output) const;

        hkpWorld* getHkpWorld() const { return m_pWorld; }

        virtual void setCollisionFilter(ICollisionFilter* pFilter) override;

        virtual gep::Event<gep::ContactPointArgs*>* getContactPointEvent() override;
        virtual gep::Event<gep::CollisionArgs*>* getCollisionAddedEvent() override;
        virtual gep::Event<gep::CollisionArgs*>* getCollisionRemovedEvent() override;

        virtual void contactPointCallback(const ContactPointArgs& evt) override;
        virtual void collisionAddedCallback(const CollisionArgs& evt) override;
        virtual void collisionRemovedCallback(const CollisionArgs& evt) override;

        virtual void markForRead() const override;
        virtual void unmarkForRead() const override;

        virtual void markForWrite() override;
        virtual void unmarkForWrite() override;

        virtual void lock() override;
        virtual void unlock() override;

        virtual void addConstraint(Constraint constraint) override;
        virtual void removeConstraint(Constraint constraint) override;
    };
}
