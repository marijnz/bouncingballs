#include "stdafx.h"
#include "gpp/gameComponents/physicsComponent.h"

#include "gep/interfaces/physics.h"
#include "gep/globalManager.h"
#include "gpp/ExperimentalContactListener.h"

gpp::PhysicsComponent::PhysicsComponent():
    Component(),
    m_pRigidBody(nullptr),
    m_pWorld(g_globalManager.getPhysicsSystem()->getWorld()),
    m_event_contactPoint(),
    m_transform()
{
}

gpp::PhysicsComponent::~PhysicsComponent()
{
}

void gpp::PhysicsComponent::initalize()
{
    GEP_ASSERT(m_pRigidBody, "Physics component needs a valid rigid body! "
                             "Use createRigidBody to create one.");
    if (m_state != State::Initial) { return; } // User already the state.
    setState(State::Active);
}

void gpp::PhysicsComponent::update(float elapsedMS)
{
    GEP_UNUSED(elapsedMS);

    gep::IWorld::ScopedLock raii(m_pWorld.get());

    if(m_pRigidBody->isTriggerVolume() || m_pRigidBody->getMotionType() == gep::MotionType::Fixed)
    {
        m_pRigidBody->setPosition(m_transform.getWorldPosition());
        m_pRigidBody->setRotation(m_transform.getWorldRotation());
    }
    else
    {
        m_transform.setPosition(m_pRigidBody->getPosition() - m_transform.getParent()->getWorldPosition());
        m_transform.setRotation(m_transform.getParent()->getWorldRotation().inverse() * m_pRigidBody->getRotation());
    }
}

void gpp::PhysicsComponent::destroy()
{
    setState(State::Inactive);
}

void gpp::PhysicsComponent::setBaseOrientation(const gep::Quaternion& viewDir)
{
    m_baseOrientation = viewDir;
}

void gpp::PhysicsComponent::setBaseViewDirection(const gep::vec3& direction)
{
    m_baseOrientation = gep::Quaternion(direction, gep::vec3(0,1,0));
}


void gpp::PhysicsComponent::setPosition(const gep::vec3& pos)
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    m_transform.setPosition(pos);
    m_pRigidBody->setPosition(m_transform.getWorldPosition());
}

void gpp::PhysicsComponent::setRotation(const gep::Quaternion& rot)
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    m_transform.setRotation(rot);
    m_pRigidBody->setRotation(m_transform.getWorldRotation());
}

gep::mat4 gpp::PhysicsComponent::getWorldTransformationMatrix() const
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    //TODO: Extend for scale
    return m_transform.getWorldTransformationMatrix();
}

gep::vec3 gpp::PhysicsComponent::getWorldPosition() const
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    return  m_transform.getWorldPosition();
}

gep::Quaternion gpp::PhysicsComponent::getWorldRotation() const
{
    GEP_ASSERT(m_pRigidBody, "When calling this method, the rigid body must not be null!");
    return m_transform.getWorldRotation();
}

gep::mat4 gpp::PhysicsComponent::getTransformationMatrix()const
{
    return m_transform.getTransformationMatrix();
}

gep::vec3 gpp::PhysicsComponent::getPosition() const
{
     return m_transform.getPosition();
}

gep::Quaternion gpp::PhysicsComponent::getRotation() const
{
    return m_transform.getRotation();
}

void gpp::PhysicsComponent::setParent(gep::ITransform* parent)
{
   m_transform.setParent(parent);
}

gep::ITransform* gpp::PhysicsComponent::getParent()
{
    return m_transform.getParent();
}

const gep::ITransform* gpp::PhysicsComponent::getParent() const
{
    return m_transform.getParent();
}

gep::IRigidBody* gpp::PhysicsComponent::getRigidBody()
{
    return m_pRigidBody.get();
}

gep::IRigidBody* gpp::PhysicsComponent::createRigidBody(gep::RigidBodyCInfo& cinfo)
{
    GEP_ASSERT(m_pWorld.get() != nullptr, "The Physics World for of the PhysicsComponent on has not been set.", m_pParentGameObject->getGuid().c_str());
    GEP_ASSERT(!bool(m_pRigidBody), "The rigid body for of the PhysicsComponent has already been set.", m_pParentGameObject->getGuid().c_str());
    
    if (m_pParentGameObject->getParent() != gep::getIdentityTransform())
    {
        this->setParent(m_pParentGameObject->getParent());
    }

    m_pParentGameObject->setTransform(*this); //TODO: Set parent gameObject in Component Constructor!

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

    m_pRigidBody = physicsFactory->createRigidBody(cinfo);
    m_pRigidBody->initialize();

    if(cinfo.shape->getShapeType() == gep::ShapeType::Triangle)
    {
        auto shapeTransform = cinfo.shape->getTransform();
        m_pRigidBody->setInitialTransform(shapeTransform);
    }
     
    return m_pRigidBody.get();
}

// PRIVATE:
void gpp::PhysicsComponent::setRigidBody(gep::IRigidBody* rigidBody)
{
    m_pRigidBody = rigidBody;
    m_pParentGameObject->setTransform(*this);
}

void gpp::PhysicsComponent::contactPointCallback(const gep::ContactPointArgs& evt)
{
    m_event_contactPoint.trigger(&const_cast<gep::ContactPointArgs&>(evt));
}

void gpp::PhysicsComponent::collisionAddedCallback(const gep::CollisionArgs& evt)
{
}

void gpp::PhysicsComponent::collisionRemovedCallback(const gep::CollisionArgs& evt)
{
}

gep::vec3 gpp::PhysicsComponent::getViewDirection() const
{
    return (m_pRigidBody->getRotation() * m_baseOrientation ).toMat3() * gep::vec3(0,1,0);
}
gep::vec3 gpp::PhysicsComponent::getUpDirection() const
{
    return (m_pRigidBody->getRotation() * m_baseOrientation).toMat3() * gep::vec3(0,0,1);
}
gep::vec3 gpp::PhysicsComponent::getRightDirection() const 
{
    return (m_pRigidBody->getRotation() * m_baseOrientation).toMat3() * gep::vec3(1,0,0);
}

void gpp::PhysicsComponent::setState(State::Enum newState)
{
    GEP_ASSERT(newState != State::Initial);

    auto oldState = m_state;

    if (oldState == newState) { return; } // Nothing to do here.

    switch (newState)
    {
    case State::Active:
        if (oldState != State::Active)
        {
            activate();
        }
        break;
    case State::Inactive:
        if (oldState == State::Active)
        {
            deactivate();
        }
        break;
    default:
        GEP_ASSERT(false, "Invalid requested state.", oldState, newState);
        break;
    }

    m_state = newState;
}

void gpp::PhysicsComponent::activate()
{
    m_state = State::Active;
    m_pWorld->addEntity(m_pRigidBody.get());
    m_pRigidBody->addContactListener(this);
}

void gpp::PhysicsComponent::deactivate()
{
    m_state = State::Inactive;
    m_pRigidBody->removeContactListener(this);
    m_pWorld->removeEntity(m_pRigidBody.get());
}
