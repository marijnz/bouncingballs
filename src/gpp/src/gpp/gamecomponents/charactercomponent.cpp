#include "stdafx.h"
#include "gpp/gameComponents/characterComponent.h"

#include "gep/globalManager.h"
#include "gep/interfaces/physics.h"
#include "gep/interfaces/physics/characterController.h"
#include "gep/interfaces/physics/factory.h"
#include "gep/interfaces/inputHandler.h"
#include "gep/interfaces/logging.h"

#include "gep/interfaces/renderer.h"

#include "gpp/gameComponents/physicsComponent.h"


gpp::CharacterComponent::CharacterComponent():
    PhysicsComponent(),
    m_pCharacterRigidBody(nullptr)
{
}

gpp::CharacterComponent::~CharacterComponent()
{
}

void gpp::CharacterComponent::initalize()
{
    PhysicsComponent::initalize();
}

void gpp::CharacterComponent::destroy()
{
    PhysicsComponent::destroy();
}

void gpp::CharacterComponent::update( float elapsedMS )
{
    PhysicsComponent::update(elapsedMS);
    auto deltaSeconds = g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60);

    gep::IWorld::ScopedLock raii(m_pWorld.get());

    auto* pInputHandler = g_globalManager.getInputHandler();
    auto* pLogging = g_globalManager.getLogging();
    auto& debugRenderer = g_globalManager.getRenderer()->getDebugRenderer();

    gep::CharacterInput input;
    gep::CharacterOutput output;

    input.deltaTime = deltaSeconds;
    input.velocity = m_pCharacterRigidBody->getRigidBody()->getLinearVelocity();
    input.position = m_pCharacterRigidBody->getRigidBody()->getPosition();

    if (pInputHandler->isPressed(gep::Key::Up))
    {
        pLogging->logMessage("up");
        input.inputUD = -1.0f;
    }
    if (pInputHandler->isPressed(gep::Key::Down))
    {
        pLogging->logMessage("down");
        input.inputUD = 1.0f;
    }
    if (pInputHandler->isPressed(gep::Key::Left))
    {
        pLogging->logMessage("left");
        input.inputLR = 1.0f;
    }
    if (pInputHandler->isPressed(gep::Key::Right))
    {
        pLogging->logMessage("right");
        input.inputLR = -1.0f;
    }

    input.up = m_pParentGameObject->getUpDirection();
    input.forward = m_pParentGameObject->getViewDirection();
    input.characterGravity.z = -9.81f;

    auto result = m_event_updateCharacterInput.trigger(&input);

    // If no one handled the result, there is nothing to do here.
    //if (result == gep::EventResult::Ignored) { return; }

    m_pCharacterRigidBody->checkSupport(deltaSeconds, input.surfaceInfo);

    switch (input.surfaceInfo.supportedState)
    {
    case gep::SurfaceInfo::SupportedState::Supported:
        debugRenderer.printText(getWorldPosition() + gep::vec3(0.0f, 0.0f, -10.0f), "Supported", gep::Color::yellow());
        break;
    case gep::SurfaceInfo::SupportedState::Unsupported:
        debugRenderer.printText(getWorldPosition() + gep::vec3(0.0f, 0.0f, -10.0f), "Unsupported", gep::Color::yellow());
        break;
    case gep::SurfaceInfo::SupportedState::Sliding:
        debugRenderer.printText(getWorldPosition() + gep::vec3(0.0f, 0.0f, -10.0f), "Sliding", gep::Color::yellow());
        break;
    default:
        break;
    }

    debugRenderer.printText(getWorldPosition() + gep::vec3(0.0f, 0.0f, -5.0f),
        gep::CharacterState::toString(m_pCharacterRigidBody->getState()),
        gep::Color::yellow());

    m_pCharacterRigidBody->update(input, output);

    m_pCharacterRigidBody->setLinearVelocity(output.velocity, deltaSeconds);
}

gep::ICharacterRigidBody* gpp::CharacterComponent::createCharacterRigidBody(const gep::CharacterRigidBodyCInfo& cinfo)
{

    // cinfo.position > 0
    if(!gep::epsilonCompare(cinfo.position.squaredLength(), 0.0f))
    {
        m_transform.setPosition(m_transform.getWorldPosition() + cinfo.position);
    }

    if(!cinfo.rotation.isIdentity())
    {
        m_transform.setRotation(m_transform.getWorldRotation() * cinfo.rotation);
    }

    auto physicsSystem = g_globalManager.getPhysicsSystem();
    auto physicsFactory = physicsSystem->getPhysicsFactory();

    m_pCharacterRigidBody = physicsFactory->createCharacterRigidBody(cinfo);
    m_pRigidBody = m_pCharacterRigidBody->getRigidBody();
    m_pCharacterRigidBody->initialize();

    if(cinfo.shape->getShapeType() == gep::ShapeType::Triangle)
    {
        auto shapeTransform = cinfo.shape->getTransform();
        m_pRigidBody->setInitialTransform(shapeTransform);
    }

    return m_pCharacterRigidBody.get();
}
