#pragma once

#include "gepmodule.h"
#include "gep/singleton.h"
#include "gep/interfaces/subsystem.h"

namespace gep
{
    //forward declarations
    class IRenderer;
    class IRendererExtractor;
    class ISettings;
    class IUpdateFramework;
    class ILogging;
    class ILogSink;
    class IMemoryManager;
    class IResourceManager;
    class Timer;
    class IInputHandler;
    class TaskQueue;
    class ISoundSystem;
    class IPhysicsSystem;
    class IScriptingManager;
    class IEventManager;
    class ICameraManager;
    class IAnimationSystem;

    /// \brief the global manager responsible for destroying most subsystems
    class GEP_API GlobalManager : public DoubleLockingSingleton<GlobalManager>
        , public ISubsystem
    {
        friend class DoubleLockingSingleton<GlobalManager>;
    private:
        IRenderer* m_pRenderer;
        IRendererExtractor* m_pRendererExtractor;
        ISettings* m_pSettings;
        IUpdateFramework* m_pUpdateFramework;
        ILogging* m_pLogging;
        ILogSink* m_ConsoleLogSink;
        ILogSink* m_FileLogSink;
        IMemoryManager* m_pMemoryManager;
        IResourceManager* m_pResourceManager;
        Timer* m_pTimer;
        IInputHandler* m_pInputHandler;
        TaskQueue* m_pTaskQueue;
        ISoundSystem* m_pSoundSystem;
        IPhysicsSystem* m_pPhysicsSystem;
        IScriptingManager* m_pScriptingManager;
        IEventManager* m_pEventManager;
        ICameraManager* m_pCameraManager;
        IAnimationSystem* m_pAnimationSystem;

        GlobalManager();
        ~GlobalManager();

    public:
        // ISubsystem interface
        virtual void initialize() override;
        virtual void destroy() override;

        inline IRenderer* getRenderer()
        {
            GEP_ASSERT(m_pRenderer != nullptr, "renderer has not been created yet");
            return m_pRenderer;
        }
        inline IRendererExtractor* getRendererExtractor()
        {
            GEP_ASSERT(m_pRendererExtractor != nullptr, "renderer extractor has not been created yet");
            return m_pRendererExtractor;
        }
        inline ISettings* getSettings()
        {
            GEP_ASSERT(m_pSettings != nullptr, "settings have not been initialized yet");
            return m_pSettings;
        }
        inline IUpdateFramework* getUpdateFramework()
        {
            GEP_ASSERT(m_pUpdateFramework != nullptr, "update framework has not been created yet");
            return m_pUpdateFramework;
        }
        inline ILogging* getLogging()
        {
            GEP_ASSERT(m_pLogging != nullptr, "logging has not been created yet");
            return m_pLogging;
        }
        inline IMemoryManager* getMemoryManager()
        {
            GEP_ASSERT(m_pMemoryManager != nullptr, "memory manager has not been created yet");
            return m_pMemoryManager;
        }
        inline IResourceManager* getResourceManager()
        {
            GEP_ASSERT(m_pResourceManager != nullptr, "resource manager has not been created yet");
            return m_pResourceManager;
        }
        inline Timer& getTimer()
        {
            GEP_ASSERT(m_pTimer != nullptr, "timer has not been created yet");
            return *m_pTimer;
        }
        inline IInputHandler* getInputHandler()
        {
            GEP_ASSERT(m_pInputHandler != nullptr, "input handler has not been created yet");
            return m_pInputHandler;
        }

        inline TaskQueue* getTaskQueue()
        {
            GEP_ASSERT(m_pTaskQueue != nullptr, "task queue has not been created yet");
            return m_pTaskQueue;
        }
        inline ISoundSystem* getSoundSystem()
        {
            GEP_ASSERT(m_pSoundSystem != nullptr, "sound system has not been created yet");
            return m_pSoundSystem;
        }
        inline ICameraManager* getCameraManager()
        {
            GEP_ASSERT(m_pCameraManager != nullptr, "camera system has not been created yet");
            return m_pCameraManager;
        }

        inline IAnimationSystem* getAnimationSystem()
        {
            GEP_ASSERT(m_pAnimationSystem != nullptr, "animation system has not been created yet");
            return m_pAnimationSystem;
        }
        inline IPhysicsSystem* getPhysicsSystem()
        {
            GEP_ASSERT(m_pPhysicsSystem != nullptr, "physics system has not been created yet");
            return m_pPhysicsSystem;
        }
        inline IScriptingManager* getScriptingManager()
        {
            GEP_ASSERT(m_pScriptingManager != nullptr, "scripting system has not been created yet");
            return m_pScriptingManager;
        }
        inline IEventManager* getEventManager()
        {
            GEP_ASSERT(m_pEventManager != nullptr, "event system has not been created yet");
            return m_pEventManager;
        }
    };
}


#define g_globalManager gep::GlobalManager::instance()
