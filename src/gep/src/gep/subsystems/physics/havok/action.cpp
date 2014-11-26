#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/action.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

void gep::HavokRigidBodySyncAction::applyAction(const hkStepInfo& stepInfo)
{
    const auto* rb = getRigidBody();
    GEP_ASSERT(rb);

    hkUlong userData = rb->getUserData();
    GEP_ASSERT(userData, "It seems you did not initialize your rigid body!");

    reinterpret_cast<HavokRigidBody*>(userData)->triggerSimulationCallbacks();
}

hkpAction* gep::HavokRigidBodySyncAction::clone(const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms) const
{
    //TODO: Implement me?
    g_globalManager.getLogging()->logMessage("<EntitySyncAction> cloning!");
    return nullptr;
}
