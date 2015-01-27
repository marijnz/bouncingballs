#pragma once

#include "gep/math3d/transform.h"

#include "gep/singleton.h"
#include "gep/utils.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/exception.h"
#include "gep/weakPtr.h"

#include "gep/interfaces/scripting.h"
#include "gep/memory/allocators.h"

#include "gep/math3d/transform.h"

namespace gpp
{
    class GameObject;

    class GameObjectManager: public gep::DoubleLockingSingleton<GameObjectManager>
    {
        friend class gep::DoubleLockingSingleton<GameObjectManager>;
    public:

        struct State
        {
            enum Enum
            {
                PreInitialization = 0,
                PostInitialization = 1
            };
        };

        GameObject* createGameObject(const std::string& guid)
        {
            GEP_ASSERT(m_state == State::PreInitialization,
                       "You are not allowed to create game objects after the initialization process.");
            return doCreateGameObject(guid);
        }
        void destroyGameObject(GameObject* pGameObject);
        GameObject* getGameObject(const std::string& guid);

        GameObject* getCurrentCameraObject(){return m_pCurrentCameraObject;}
        void setCurrentCameraObject(GameObject* object) {m_pCurrentCameraObject = object;}

        virtual void initialize();
        virtual void destroy();
        virtual void update(float elapsedMs);

        State::Enum getState() { return m_state; }

        inline gep::StackAllocator* getTempAllocator() { return &m_tempAllocator; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createGameObject)
            LUA_BIND_FUNCTION(createGameObjectUninitialized)
            LUA_BIND_FUNCTION(destroyGameObject)
            LUA_BIND_FUNCTION(getGameObject)
        LUA_BIND_REFERENCE_TYPE_END;

    protected:
        GameObjectManager();
        virtual ~GameObjectManager();
    private:
        gep::Hashmap<std::string, GameObject*, gep::StringHashPolicy> m_gameObjects;
        gep::DynamicArray<GameObject*> m_garbage;
        State::Enum m_state;
        gep::StackAllocator m_tempAllocator;
        GameObject* m_pCurrentCameraObject;

        GameObject* createGameObjectUninitialized(const std::string& guid)
        {
            GEP_ASSERT(m_state == State::PostInitialization,
                       "Can only create uninitialized game objects after the initial initialization phase");
            return doCreateGameObject(guid);
        }

        GameObject* doCreateGameObject(const std::string& guid);
    };

    class IComponent
    {
        friend class GameObject;
    public:

        struct State
        {
            enum Enum
            {
                Initial,
                Active,
                Inactive
            };

            GEP_DISALLOW_CONSTRUCTION(State);
        };

        virtual ~IComponent() {}

        virtual void initalize() = 0;
        virtual void update(float elapsedMS) = 0;
        virtual void destroy() = 0;

        virtual void setState(State::Enum state) = 0;
        virtual State::Enum getState() const = 0;

        virtual       GameObject* getParentGameObject()       = 0;
        virtual const GameObject* getParentGameObject() const = 0;
        virtual gep::uint32 getPriority() const = 0;

    protected:
        virtual void setParentGameObject(GameObject* object) = 0;
    };

    /// \brief abstract base class of all components.
    class Component : public IComponent
    {
    public:
        Component() : m_pParentGameObject(nullptr), m_state(State::Initial) {}
        virtual ~Component() {}

        virtual       GameObject* getParentGameObject()       override { return m_pParentGameObject; }
        virtual const GameObject* getParentGameObject() const override { return m_pParentGameObject; }

        virtual void setState(State::Enum state) override { m_state = state; }
        virtual State::Enum getState() const override { return m_state; }
        virtual gep::uint32 getPriority() const override { return 1; };

    protected:
        GameObject* m_pParentGameObject;
        State::Enum m_state;

        virtual void setParentGameObject(GameObject* object) override { m_pParentGameObject = object; }
    };


    class GameObject : public gep::ITransform
    {
        friend class GameObjectManager;
        struct ComponentWrapper
        {
            int initializationPriority;
            int updatePriority;
            IComponent* component;

            ComponentWrapper() :
                initializationPriority(-1),
                updatePriority(-1),
                component(nullptr)
            {
            }
        };

    public:
        GameObject();
        ~GameObject();

        void update(float elapsedMs);
        void initialize();
        void destroy();

        template<typename T>
        T* createComponent()
        {
            GEP_ASSERT(!m_isInitialized, "Cannot create a new component on an initialized game object.");
            auto pInstance = new T();
            if(addComponent(pInstance))
            {
                return pInstance;
            }
            gep::deleteAndNull(pInstance);
            GEP_ASSERT(false, "Failed to add component. Check the log for more information.");
            return nullptr;
        }

        template<typename T>
        T* getComponent()
        {
            T* pComponent = nullptr;
            auto name = ComponentMetaInfo<T>::name();
            m_components.ifExists(name, [&](const ComponentWrapper& wrapper){
                pComponent = static_cast<T*>(wrapper.component);
            });
            return pComponent;
        }

        virtual void setPosition(const gep::vec3& pos) override;
        virtual void setRotation(const gep::Quaternion& rot) override;

        virtual gep::vec3 getWorldPosition() const override;
        virtual gep::Quaternion getWorldRotation() const override;
        virtual gep::mat4 getWorldTransformationMatrix() const override;

        virtual gep::Quaternion getRotation() const override;
        virtual gep::vec3 getPosition() const override;
        virtual gep::mat4 getTransformationMatrix() const override;

        virtual gep::vec3 getViewDirection() const override;
        virtual gep::vec3 getUpDirection() const override;
        virtual gep::vec3 getRightDirection() const override;
        virtual void setBaseOrientation(const gep::Quaternion& viewDir) override;
        virtual void setBaseViewDirection(const gep::vec3& direction) override;

        void setComponentStates(IComponent::State::Enum value);

        inline const std::string& getGuid() const { return m_guid; }

        inline       gep::ITransform& getTransform()       { return *m_transform; }
        inline const gep::ITransform& getTransform() const { return *m_transform; }
        inline void setTransform(gep::ITransform& transform) { m_transform = &transform; }

        virtual       gep::ITransform* getParent()       override;
        virtual const gep::ITransform* getParent() const override;
        virtual void setParent(gep::ITransform* parent) override;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(createComponent<CameraComponent>, "createCameraComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<RenderComponent>, "createRenderComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<PhysicsComponent>, "createPhysicsComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<ScriptComponent>, "createScriptComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<AnimationComponent>, "createAnimationComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<CharacterComponent>, "createCharacterComponent")
            LUA_BIND_FUNCTION_NAMED(createComponent<AudioComponent>, "createAudioComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<CameraComponent>, "getCameraComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<RenderComponent>, "getRenderComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<PhysicsComponent>, "getPhysicsComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<ScriptComponent>, "getScriptComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<AnimationComponent>, "getAnimationComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<CharacterComponent>, "getCharacterComponent")
            LUA_BIND_FUNCTION_NAMED(getComponent<AudioComponent>, "getAudioComponent")
            LUA_BIND_FUNCTION_NAMED(initializeManually, "initialize")
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(getWorldPosition)
            LUA_BIND_FUNCTION(setRotation)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(getWorldRotation)
            LUA_BIND_FUNCTION(getViewDirection)
            LUA_BIND_FUNCTION(getUpDirection)
            LUA_BIND_FUNCTION(getRightDirection)
            LUA_BIND_FUNCTION(setComponentStates)
            LUA_BIND_FUNCTION(setBaseOrientation)
            LUA_BIND_FUNCTION(setBaseViewDirection)
            LUA_BIND_FUNCTION_NAMED(getGuidCopy, "getGuid")
            LUA_BIND_FUNCTION(setParent)
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        std::string m_guid;
        bool m_isInitialized; ///< Used for checks/asserts.
        bool m_isActive;
        gep::Transform m_defaultTransform;
        gep::ITransform* m_transform;
        gep::Hashmap<const char*, ComponentWrapper> m_components;
        gep::DynamicArray<ComponentWrapper> m_updateQueue;

        void initializeManually()
        {
            GEP_ASSERT(GameObjectManager::instance().getState() == GameObjectManager::State::PostInitialization,
                       "Calling `initialize` manually on a game object "
                       "is only allowed after the initial initialization phase.");
            initialize();
        }

        inline std::string getGuidCopy() { return getGuid(); }

        template<typename T>
        bool addComponent(T* specializedComponent)
        {
            //check weather T is really an ICompontent
            auto component = static_cast<IComponent*>(specializedComponent);
            ComponentWrapper wrapper;
            wrapper.initializationPriority = ComponentMetaInfo<T>::initializationPriority();
            wrapper.updatePriority = ComponentMetaInfo<T>::updatePriority();
            wrapper.component = component;

            const char* const typeName = ComponentMetaInfo<T>::name();
            if(m_components[typeName].component != nullptr)
            {
                GEP_ASSERT(false, "A component of the same type has already been added to this gameObject", typeName, m_guid);
                g_globalManager.getLogging()->logError("The component %s has already been added to gameObject %s", typeName, m_guid);
                return false;
            }
            component->setParentGameObject(this);
            m_components[typeName] = wrapper;

            if (wrapper.updatePriority == std::numeric_limits<gep::int32>::max())
            {
                // Component needs no update
                return true;
            }
            //insert into update Queue
            size_t index = 0;

            for (auto entry : m_updateQueue)
            {
                if(entry.updatePriority > wrapper.updatePriority)
                {
                    break;
                }
                ++index;
            }
            m_updateQueue.insertAtIndex(index, wrapper);
            return true;
        }
    };

    template<typename T>
    struct ComponentMetaInfo
    {
        static const char* name()
        {
            static_assert(false, "Please specialize this template in the specific component class!");
            return nullptr;
        }

        /// The smaller this value, the earlier this component type will be initialized.
        static const gep::int32 initializationPriority()
        {
            static_assert(false, "Please specialize this template in the specific component class!");
            return 0;
        }

        /// The smaller, the earlier the component is updated within one frame.
        /// If this value is std::numeric_limits<gep::int32>::max(), this component type will never be updated.
        static const gep::int32 updatePriority()
        {
            static_assert(false, "Please specialize this template in the specific component class!");
            return std::numeric_limits<gep::int32>::max();
        }
    };

}

#define g_gameObjectManager (::gpp::GameObjectManager::instance())
