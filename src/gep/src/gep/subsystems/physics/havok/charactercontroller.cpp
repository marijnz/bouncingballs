#include "stdafx.h"

#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/characterController.h"

#include "gepimpl/subsystems/physics/havok/conversion/shape.h"
#include "gepimpl/subsystems/physics/havok/conversion/characterInputAndOutput.h"
#include "gepimpl/subsystems/physics/havok/conversion/surfaceInfo.h"

#include "gepimpl/havok/util.h"


gep::HavokCharacterRigidBody::HavokCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) :
    m_pRigidBody(nullptr),
    m_pHkCharacterRigidBody(nullptr),
    m_pHkCharacterContext(nullptr)
{
    {
        hkpCharacterRigidBodyCinfo hkcinfo;

        hkcinfo.m_mass = cinfo.mass;
        hkcinfo.m_maxForce = cinfo.maxForce;
        hkcinfo.m_friction = cinfo.friction;
        hkcinfo.m_maxSlope = cinfo.maxSlope;
        hkcinfo.m_unweldingHeightOffsetFactor = cinfo.unweldingHeightOffsetFactor;
        conversion::hk::to(cinfo.up, hkcinfo.m_up);
        hkcinfo.m_maxLinearVelocity = cinfo.maxLinearVelocity;
        hkcinfo.m_allowedPenetrationDepth = cinfo.allowedPenetrationDepth;
        hkcinfo.m_maxSpeedForSimplexSolver = cinfo.maxSpeedForSimplexSolver;
        hkcinfo.m_collisionFilterInfo = cinfo.collisionFilterInfo;
        conversion::hk::to(cinfo.position, hkcinfo.m_position);
        conversion::hk::to(cinfo.rotation, hkcinfo.m_rotation);
        hkcinfo.m_supportDistance = cinfo.supportDistance;
        hkcinfo.m_hardSupportDistance = cinfo.hardSupportDistance;
        hkcinfo.m_shape = conversion::hk::to(cinfo.shape);

        m_pHkCharacterRigidBody = new hkpCharacterRigidBody(hkcinfo);
        if(hkcinfo.m_shape) hkcinfo.m_shape->removeReference();
    }

    // Set default character listener
    {
        hkpCharacterRigidBodyListener* listener = new hkpCharacterRigidBodyListener();
        m_pHkCharacterRigidBody->setListener( listener );
        listener->removeReference();
    }

    // Create an internal havok rigid body from the existing rigid body so we can use this class's getRigidBody.
    m_pRigidBody = GEP_NEW(g_stdAllocator, HavokRigidBody)(m_pHkCharacterRigidBody->getRigidBody());
    
    // Create the Character state machine and context
    {
        hkpCharacterState* state;
        hkpCharacterStateManager* manager = new hkpCharacterStateManager();

        state = new hkpCharacterStateOnGround();
        manager->registerState(state, HK_CHARACTER_ON_GROUND);
        state->removeReference();

        state = new hkpCharacterStateInAir();
        manager->registerState(state, HK_CHARACTER_IN_AIR);
        state->removeReference();

        state = new hkpCharacterStateJumping();
        manager->registerState(state, HK_CHARACTER_JUMPING);
        state->removeReference();

        state = new hkpCharacterStateClimbing();
        manager->registerState(state, HK_CHARACTER_CLIMBING);
        state->removeReference();

        m_pHkCharacterContext = new hkpCharacterContext(manager, HK_CHARACTER_ON_GROUND );
        manager->removeReference();

        // Set character type
        m_pHkCharacterContext->setCharacterType(hkpCharacterContext::HK_CHARACTER_RIGIDBODY);
    }
}

gep::HavokCharacterRigidBody::~HavokCharacterRigidBody()
{
}

void gep::HavokCharacterRigidBody::initialize()
{
    m_pRigidBody->initialize();
}

void gep::HavokCharacterRigidBody::destroy()
{
}

void gep::HavokCharacterRigidBody::update(const CharacterInput& input, CharacterOutput& output)
{
    hkpCharacterInput hkinput;
    hkpCharacterOutput hkoutput;

    conversion::hk::to(input, hkinput);
    m_pHkCharacterContext->update(hkinput, hkoutput);
    conversion::hk::from(hkoutput, output);
}

void gep::HavokCharacterRigidBody::checkSupport(float deltaTime, SurfaceInfo& surfaceinfo)
{
    hkStepInfo step;
    step.m_deltaTime = deltaTime;
    step.m_invDeltaTime = 1.0f/step.m_deltaTime;

    hkpSurfaceInfo surface;
    conversion::hk::to(surfaceinfo, surface);

    m_pHkCharacterRigidBody->checkSupport(step, surface);
    conversion::hk::from(surface, surfaceinfo);
}

gep::IRigidBody* gep::HavokCharacterRigidBody::getRigidBody()
{
    return m_pRigidBody.get();
}

void gep::HavokCharacterRigidBody::setLinearVelocity(const vec3& newVelocity, float deltaTime)
{
    GEP_ASSERT(deltaTime > 0.0f || !epsilonCompare(deltaTime, 0.0f), "The delta time must be greater than zero!");
    hkVector4 linearVelocity;
    conversion::hk::to(newVelocity, linearVelocity);
    m_pHkCharacterRigidBody->setLinearVelocity(linearVelocity, deltaTime);
}

gep::CharacterState::Enum gep::HavokCharacterRigidBody::getState() const
{
    return static_cast<CharacterState::Enum>(m_pHkCharacterContext->getState());
}
