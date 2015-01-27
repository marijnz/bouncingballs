#include "stdafx.h"
#include "gpp/gameObjectSystem.h"
#include <algorithm>

#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

//GameObjectManager

//singleton static members
gpp::GameObjectManager* volatile gep::DoubleLockingSingleton<gpp::GameObjectManager>::s_instance = nullptr;
gep::Mutex gep::DoubleLockingSingleton<gpp::GameObjectManager>::s_creationMutex;

gpp::GameObjectManager::GameObjectManager():
    m_gameObjects(),
    m_state(State::PreInitialization),
    m_tempAllocator(true, 1024),
    m_pCurrentCameraObject(nullptr)
{
}

gpp::GameObjectManager::~GameObjectManager()
{
}

gpp::GameObject* gpp::GameObjectManager::doCreateGameObject(const std::string& guid)
{
    GameObject* pGameObject = nullptr;
    GEP_ASSERT(!m_gameObjects.tryGet(guid, pGameObject), "GameObject already exists!", guid);
    pGameObject = new GameObject();
    pGameObject->m_guid = guid;
    m_gameObjects[guid] = pGameObject;
    return pGameObject;
}

void gpp::GameObjectManager::destroyGameObject(GameObject* pGameObject)
{
    GEP_ASSERT(pGameObject, "Invalid input");
    GEP_ASSERT(m_gameObjects.exists(pGameObject->getGuid()), "Trying to delete non-existant game object!");
    GEP_ASSERT(m_gameObjects[pGameObject->getGuid()] == pGameObject, "Detected same GUID but different pointers!");
    g_globalManager.getLogging()->logWarning("Deleting game object '%s' at the end of the frame."
                                             " All references to this object "
                                             "(including the ones in your Lua code) will be invalid!",
                                             pGameObject->getGuid().c_str());
    m_garbage.append(pGameObject);
}

gpp::GameObject* gpp::GameObjectManager::getGameObject(const std::string& guid)
{
    GameObject* pGameObject = nullptr;
    m_gameObjects.tryGet(guid, pGameObject);
    return pGameObject;
}

void gpp::GameObjectManager::initialize()
{
    for(auto pGameObject : m_gameObjects.values())
    {
        pGameObject->initialize();
    }
    m_state = State::PostInitialization;
}

void gpp::GameObjectManager::destroy()
{
    for(auto pGameObject : m_gameObjects.values())
    {
        if (pGameObject)
        {
            pGameObject->destroy();
            delete pGameObject;
        }
    }
    m_gameObjects.clear();
}

void gpp::GameObjectManager::update(float elapsedMs)
{
    for(auto pGameObject : m_gameObjects.values())
    {
        if (pGameObject)
        {
            pGameObject->update(elapsedMs);
        }
    }

    // Collect garbage
    for (auto pGarbage : m_garbage)
    {
        const auto& guid = pGarbage->getGuid();
        auto result = m_gameObjects.remove(guid);
        GEP_ASSERT(result == gep::SUCCESS, "Failed to remove game object from list of all game objects?!?!");
        pGarbage->destroy();
    }
    m_garbage.clear();
}

gpp::GameObject::GameObject() :
    m_guid(),
    m_isInitialized(false),
    m_isActive(true),
    m_defaultTransform(),
    m_transform(&m_defaultTransform),
    m_components(),
    m_updateQueue()
{
}

gpp::GameObject::~GameObject()
{

}

void gpp::GameObject::setPosition(const gep::vec3& pos)
{
    m_transform->setPosition(pos);
}

void gpp::GameObject::setRotation(const gep::Quaternion& rot)
{
    m_transform->setRotation(rot);
}

void gpp::GameObject::setBaseOrientation(const gep::Quaternion& orientation)
{
    m_transform->setBaseOrientation(orientation);
}

gep::vec3 gpp::GameObject::getWorldPosition() const
{
    return m_transform->getWorldPosition();
}

gep::Quaternion gpp::GameObject::getWorldRotation() const
{
    return m_transform->getWorldRotation();
}

gep::mat4 gpp::GameObject::getTransformationMatrix() const
{
    return m_transform->getTransformationMatrix();
}

gep::vec3 gpp::GameObject::getPosition() const
{
    return m_transform->getPosition();
}

gep::Quaternion gpp::GameObject::getRotation() const
{
    return m_transform->getRotation();
}

void gpp::GameObject::update(float elapsedMs)
{
    GEP_ASSERT(m_isInitialized,
               "Game object not initialized. "
               "You have to call :initialize() on game objects created by :createGameObjectUninitialized()");

    for(auto component : m_updateQueue)
    {
        component.component->update(elapsedMs);
    }
}

void gpp::GameObject::initialize()
{
    GEP_ASSERT(!m_isInitialized, "Cannot initialize a game object twice.");

    auto pAllocator = GameObjectManager::instance().getTempAllocator();

    auto toInit = gep::DynamicArray<ComponentWrapper*>(pAllocator);
    toInit.reserve(m_components.count());

    for(auto& wrapper : m_components.values())
    {
        toInit.append(&wrapper);
    }

    std::sort(toInit.begin(), toInit.end(), [](ComponentWrapper* lhs, ComponentWrapper* rhs){
        return lhs->initializationPriority < rhs->initializationPriority;
    });

    for(auto wrapper : toInit)
    {
        wrapper->component->initalize();
        GEP_ASSERT(wrapper->component->getState() != IComponent::State::Initial,
            "A game component must set its state within its initialize function!");
    }

    m_isInitialized = true;
}

void gpp::GameObject::destroy()
{
    auto pAllocator = GameObjectManager::instance().getTempAllocator();

    // Create a sorted array of all component instances.
    auto toDestroy = gep::DynamicArray<ComponentWrapper*>(pAllocator);
    toDestroy.reserve(m_components.count());

    for(auto& wrapper : m_components.values())
    {
        toDestroy.append(&wrapper);
    }

    // sort so least prioritized components come first.
    std::sort(toDestroy.begin(), toDestroy.end(), [](ComponentWrapper* lhs, ComponentWrapper* rhs){
        return lhs->initializationPriority > rhs->initializationPriority;
    });

    // Call destroy on all components
    for(auto wrapper : toDestroy)
    {
        wrapper->component->destroy();
    }

    // Delete the components.
    for(auto wrapper : toDestroy)
    {
        gep::deleteAndNull(wrapper->component);
    }

    m_components.clear();
    m_updateQueue.resize(0);

    m_isInitialized = false;
}

gep::mat4 gpp::GameObject::getWorldTransformationMatrix() const
{
    return m_transform->getWorldTransformationMatrix();
}

gep::vec3 gpp::GameObject::getViewDirection() const
{
    return m_transform->getViewDirection();
}

gep::vec3 gpp::GameObject::getUpDirection() const
{
    return m_transform->getUpDirection();
}

gep::vec3 gpp::GameObject::getRightDirection() const
{
    return m_transform->getRightDirection();
}

void gpp::GameObject::setComponentStates(IComponent::State::Enum value)
{
    for (auto& wrapper : m_components.values())
    {
        wrapper.component->setState(value);
    }
}

void gpp::GameObject::setBaseViewDirection(const gep::vec3& direction)
{
    m_transform->setBaseViewDirection(direction);
}

gep::ITransform* gpp::GameObject::getParent()
{
   return m_transform->getParent();
}

const gep::ITransform* gpp::GameObject::getParent() const
{
    return m_transform->getParent();
}

void gpp::GameObject::setParent(gep::ITransform* parent)
{
   m_transform->setParent(parent);
}
