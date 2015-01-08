#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/world.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/action.h"
#include "gepimpl/subsystems/physics/havok/contact.h"

#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/physics/characterController.h"

hkBool gep::HavokCollisionFilter_Simple::isCollisionEnabled(const hkpCollidable& a,
                                                            const hkpCollidable& b) const
{
    return isCollisionEnabled(a.getCollisionFilterInfo(), b.getCollisionFilterInfo());
}

hkBool gep::HavokCollisionFilter_Simple::isCollisionEnabled(const hkpCollisionInput& input,
                                                            const hkpCdBody& a,
                                                            const hkpCdBody& b,
                                                            const HK_SHAPE_CONTAINER& bContainer,
                                                            hkpShapeKey bKey) const
{
    hkUint32 infoB = bContainer.getCollisionFilterInfo(bKey);
    // We need a corresponding filter info for 'a'. Whether we should get this from a parent/grandparent/etc... of 'a' in the case that
    // 'a' is part of a shape collection depends on how we decide to handle the 'collection vs collection' case.
    // Here we just assume that we do not have collections colliding against collections, and use the filter info of the root collidable of 'a'
    return isCollisionEnabled(a.getRootCollidable()->getCollisionFilterInfo(), infoB);
}

hkBool gep::HavokCollisionFilter_Simple::isCollisionEnabled(const hkpCollisionInput& input,
                                                            const hkpCdBody& collectionBodyA,
                                                            const hkpCdBody& collectionBodyB,
                                                            const HK_SHAPE_CONTAINER& containerShapeA,
                                                            const HK_SHAPE_CONTAINER& containerShapeB,
                                                            hkpShapeKey keyA,
                                                            hkpShapeKey keyB) const
{
    hkUint32 infoA = containerShapeA.getCollisionFilterInfo(keyA);
    hkUint32 infoB = containerShapeB.getCollisionFilterInfo(keyB);
    return isCollisionEnabled(infoA, infoB);
}

hkBool gep::HavokCollisionFilter_Simple::isCollisionEnabled(const hkpShapeRayCastInput& aInput,
                                                            const HK_SHAPE_CONTAINER& bContainer,
                                                            hkpShapeKey bKey) const
{
    hkUint32 infoB = bContainer.getCollisionFilterInfo(bKey);
    return isCollisionEnabled(aInput.m_filterInfo, infoB);
}

hkBool gep::HavokCollisionFilter_Simple::isCollisionEnabled(const hkpWorldRayCastInput& a,
                                                            const hkpCollidable& collidableB) const
{
    return isCollisionEnabled(a.m_filterInfo, collidableB.getCollisionFilterInfo());
}

hkBool gep::HavokCollisionFilter_Simple::isCollisionEnabled(hkUint32 infoA,
                                                            hkUint32 infoB) const
{
    // Examine the collisionfilterinfo of each. This is "user" information which can be interpreted
    // however the filter sees fit. Another example would be to let every body have a unique
    // collisionfilterinfo value, and allow/disallow collisions based on a pairwise lookup.
    // We use a much simpler rule here for ease of illustration.
    // Let's say that bodies with collisionfilters X and Y are allowed to collide if and only if X == Y,
    // ie. if they are in the same "group"

    return infoA & infoB;
}

//////////////////////////////////////////////////////////////////////////

gep::HavokWorld::HavokWorld(const WorldCInfo& cinfo) :
    m_pWorld(nullptr),
    m_entities(),
    m_characters(),
    m_actualContactListener(this),
    m_event_contactPoint(),
    m_event_collisionAdded(),
    m_event_collisionRemoved()
{
    m_entities.reserve(64);

    hkpWorldCinfo worldInfo;
    worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
    conversion::hk::to(cinfo.gravity, worldInfo.m_gravity);
    worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY;
    worldInfo.setBroadPhaseWorldSize(cinfo.worldSize);
    m_pWorld = new hkpWorld(worldInfo);
    //m_pWorld->removeReference();

    // Register all collision agents
    // It's important to register collision agents before adding any entities to the world.
    hkpAgentRegisterUtil::registerAllAgents(m_pWorld->getCollisionDispatcher());

    m_pWorld->addContactListener(&m_actualContactListener);
}

gep::HavokWorld::~HavokWorld()
{
    m_pWorld->removeContactListener(&m_actualContactListener);
}

void gep::HavokWorld::addEntity(IPhysicsEntity* entity)
{
    //TODO: can only add rigid bodies at the moment.
    auto* actualEntity = static_cast<HavokRigidBody*>(entity);
    GEP_ASSERT(dynamic_cast<HavokRigidBody*>(entity) != nullptr, "Attempted to add wrong kind of entity. (only rigid bodies are supported at the moment)");
    addEntity(actualEntity->getHkpRigidBody());
    m_entities.append(entity);
}

void gep::HavokWorld::addEntity(hkpEntity* entity)
{
    m_pWorld->addEntity(entity);
    auto* action = new HavokRigidBodySyncAction();
    action->setEntity(entity);
    m_pWorld->addAction(action);
    action->removeReference();
}

void gep::HavokWorld::removeEntity(IPhysicsEntity* entity)
{
    GEP_ASSERT(entity != nullptr, "Attempt to remove nullptr.");

    // TODO Can only remove rigid bodies at the moment.
    auto* actualEntity = dynamic_cast<HavokRigidBody*>(entity);
    GEP_ASSERT(actualEntity != nullptr, "Attempt to remove wrong kind of entity. (only rigid bodies are supported at the moment)");

    size_t index;
    for (index = 0; index < m_entities.length(); ++index)
    {
        auto& entityPtr = m_entities[index];
        if (entityPtr.get() == actualEntity)
        {
            break;
        }
    }
    GEP_ASSERT(index < m_entities.length(), "Attempt to remove character from world that does not exist there", actualEntity, index, m_entities.length());
    m_entities.removeAtIndex(index);

    // Remove the actual havok entity
    removeEntity(actualEntity->getHkpRigidBody());
}

void gep::HavokWorld::removeEntity(hkpEntity* entity)
{
    m_pWorld->removeEntity(entity);
}


void gep::HavokWorld::addCharacter(ICharacterRigidBody* character)
{
    addEntity(character->getRigidBody());
    m_characters.append(character);
}

void gep::HavokWorld::removeCharacter(ICharacterRigidBody* character)
{
    GEP_ASSERT(character != nullptr);

    size_t index;
    for (index = 0; index < m_characters.length(); ++index)
    {
        auto& characterPtr = m_characters[index];
        if (characterPtr.get() == character)
        {
            break;
        }
    }
    GEP_ASSERT(index < m_characters.length(), "Attempt to remove character from world that does not exist there", character, index, m_characters.length());
    m_characters.removeAtIndex(index);

    removeEntity(character->getRigidBody());
}

void gep::HavokWorld::update(float elapsedTime)
{
    GEP_UNUSED(elapsedTime);
    //TODO tweak this value if havok is complaining too hard about the simulation becoming unstable.
    auto averageDelta = g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60);
    // Do update
    {
        m_pWorld->lock();
        m_pWorld->stepDeltaTime(averageDelta);
        m_pWorld->unlock();
    }
}

void gep::HavokWorld::castRay(const RayCastInput& input, RayCastOutput& output) const
{
    hkpWorldRayCastInput actualInput;
    hkpWorldRayCastOutput actualOutput;

    // Process input
    conversion::hk::to(input.from, actualInput.m_from);
    conversion::hk::to(input.to, actualInput.m_to);
    actualInput.m_filterInfo = input.filterInfo;

    // Cast the ray
    {
        m_pWorld->lock();
        m_pWorld->castRay(actualInput, actualOutput);
        m_pWorld->unlock();
    }

    // Process output
    output.hitFraction = actualOutput.m_hitFraction;
    conversion::hk::from(actualOutput.m_normal, output.normal);
    if (actualOutput.m_rootCollidable)
    {
        // We only support rigid bodies right now.
        auto actualEntity = static_cast<hkpEntity*>(actualOutput.m_rootCollidable->getOwner());
        output.pHitBody = static_cast<IRigidBody*>(reinterpret_cast<IPhysicsEntity*>(actualEntity->getUserData()));
    }
}

void gep::HavokWorld::contactPointCallback(const ContactPointArgs& evt)
{
    m_event_contactPoint.trigger(&const_cast<gep::ContactPointArgs&>(evt));
}

void gep::HavokWorld::collisionAddedCallback(const CollisionArgs& evt)
{
    m_event_collisionAdded.trigger(&const_cast<gep::CollisionArgs&>(evt));
}

void gep::HavokWorld::collisionRemovedCallback(const CollisionArgs& evt)
{
    m_event_collisionRemoved.trigger(&const_cast<gep::CollisionArgs&>(evt));
}

gep::Event<gep::ContactPointArgs*>* gep::HavokWorld::getContactPointEvent()
{
    return &m_event_contactPoint;
}

gep::Event<gep::CollisionArgs*>* gep::HavokWorld::getCollisionAddedEvent()
{
    return &m_event_collisionAdded;
}

gep::Event<gep::CollisionArgs*>* gep::HavokWorld::getCollisionRemovedEvent()
{
    return &m_event_collisionRemoved;
}

void gep::HavokWorld::setCollisionFilter(ICollisionFilter* pFilter)
{
    auto pHkFilterWrapper = static_cast<HavokCollisionFilterWrapper*>(pFilter);
    GEP_ASSERT(dynamic_cast<HavokCollisionFilterWrapper*>(pFilter), "Invalid type of collision filter!");

    auto pHkFilter = pHkFilterWrapper->getHkCollisionFilter();
    GEP_ASSERT(pHkFilter, "Filter wrapper has an invalid hk object!");

    m_pWorld->setCollisionFilter(pHkFilter);
}

void gep::HavokWorld::markForRead() const
{
    m_pWorld->markForRead();
}

void gep::HavokWorld::unmarkForRead() const
{
    m_pWorld->unmarkForRead();
}

void gep::HavokWorld::markForWrite()
{
    m_pWorld->markForWrite();
}

void gep::HavokWorld::unmarkForWrite()
{
    m_pWorld->unmarkForWrite();
}

void gep::HavokWorld::lock()
{
    m_pWorld->lock();
}

void gep::HavokWorld::unlock()
{
    m_pWorld->unlock();
}

void gep::HavokWorld::addConstraint(Constraint constraint)
{
    m_pWorld->addConstraint(reinterpret_cast<hkpConstraintInstance*>(constraint.pData));
}

void gep::HavokWorld::removeConstraint(Constraint constraint)
{
    m_pWorld->removeConstraint(reinterpret_cast<hkpConstraintInstance*>(constraint.pData));
}
