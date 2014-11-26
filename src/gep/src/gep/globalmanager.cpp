#include "stdafx.h"
#include "gep/globalManager.h"

#include "gepimpl/subsystems/renderer/renderer.h"
#include "gepimpl/subsystems/renderer/extractor.h"
#include "gepimpl/settings.h"
#include "gepimpl/subsystems/updateFramework.h"
#include "gepimpl/subsystems/logging.h"
#include "gepimpl/subsystems/memoryManager.h"
#include "gepimpl/subsystems/resourceManager.h"

#include "gepimpl/subsystems/inputHandler.h"

#include "gep/threading/taskQueue.h"

#include "gepimpl/subsystems/sound/system.h"


#include "gepimpl/subsystems/havok.h"
#include "gepimpl/subsystems/physics/havokPhysics.h"
#include "gepimpl/subsystems/scripting.h"
#include "gepimpl/subsystems/cameraManager.h"
#include "gepimpl/subsystems/animation/havok/animation.h"
#include "gepimpl/subsystems/events/eventManager.h"

#include "gep/timer.h"

//singleton static members
gep::GlobalManager* volatile gep::DoubleLockingSingleton<gep::GlobalManager>::s_instance = nullptr;
gep::Mutex gep::DoubleLockingSingleton<gep::GlobalManager>::s_creationMutex;

gep::GlobalManager::GlobalManager() :
    m_pRenderer(nullptr),
    m_pRendererExtractor(nullptr),
    m_pUpdateFramework(nullptr),
    m_pLogging(nullptr),
    m_ConsoleLogSink(nullptr),
    m_FileLogSink(nullptr),
    m_pMemoryManager(nullptr),
    m_pResourceManager(nullptr),
    m_pTimer(nullptr)
    , m_pInputHandler(nullptr)
    , m_pTaskQueue(nullptr)
    , m_pSoundSystem(nullptr)
    , m_pPhysicsSystem(nullptr)
    , m_pScriptingManager(nullptr)
    , m_pEventManager(nullptr)
    , m_pCameraManager(nullptr)
    , m_pSettings(nullptr),
    m_pAnimationSystem(nullptr)
{
}

gep::GlobalManager::~GlobalManager()
{
}

void gep::GlobalManager::initialize()
{
    m_pLogging = new gep::Logging();
#ifdef _DEBUG
    m_ConsoleLogSink = new ConsoleLogSink();
    m_pLogging->registerSink(m_ConsoleLogSink);
#endif
    m_FileLogSink = new FileLogSink("logfile.txt");
    m_pLogging->registerSink(m_FileLogSink);
    m_pLogging->logMessage("log system initialized");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing settings");
    m_pSettings = new Settings();
    m_pLogging->logMessage("settings initialized");
    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing memory manager");
    m_pMemoryManager = new gep::MemoryManager();
    m_pMemoryManager->initialize();
    m_pLogging->logMessage("memory manager initialized");

    m_pLogging->logMessage("initializing scripting manager");
    {
        auto scripting = new ScriptingManager(&m_pSettings->getScriptsSettings());
        scripting->initialize();
        scripting->makeBasicBindings();
        m_pScriptingManager = scripting;
    }
    m_pLogging->logMessage("scripting manager initialized");
    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("loading settings");
    {
        m_pScriptingManager->setManagerState(IScriptingManager::State::LoadingEnabled);
        m_pScriptingManager->loadScript("data/settings.lua", IScriptingManager::LoadOptions::PathIsAbsolute);
        m_pScriptingManager->setManagerState(IScriptingManager::State::LoadingDisabled);
    }
    m_pLogging->logMessage("settings loaded");
    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing timer");
    m_pTimer = new Timer();
    m_pLogging->logMessage("timer initialized");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing update framework");
    m_pUpdateFramework = new gep::UpdateFramework();
    m_pLogging->logMessage("update framework initialized");

    m_pLogging->logMessage("\n==================================================");
    
    m_pLogging->logMessage("initializing havok system");

    m_pUpdateFramework->registerInitializeCallback([](){ hk::initialize(); });
    m_pUpdateFramework->registerDestroyCallback([](){ hk::shutdown(); });

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing task queue");
    m_pTaskQueue = new TaskQueue();
    m_pLogging->logMessage("task queue initialized");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing resource manager");
    m_pResourceManager = new gep::ResourceManager();
    m_pResourceManager->initialize();
    m_pLogging->logMessage("resource manager initialized");


    m_pUpdateFramework->registerInitializeCallback([&]()
    {
        m_pResourceManager->initializeInGameThread();
    });

    m_pUpdateFramework->registerDestroyCallback([&]()
    {
        m_pLogging->logMessage("destroying resource manager");
        if(m_pResourceManager) m_pResourceManager->destroy();
        DELETE_AND_NULL(m_pResourceManager);
        m_pLogging->logMessage("resource manager destroyed");
        m_pLogging->logMessage("\n==================================================");
    });

    m_pUpdateFramework->registerDestroyCallback([&]()
    {
        m_pResourceManager->destroyInGameThread();
    });

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing renderer extractor");
    m_pRendererExtractor = new gep::RendererExtractor();
    m_pLogging->logMessage("renderer extractor initialized");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing renderer");
    m_pRenderer = new gep::Renderer(m_pSettings->getVideoSettings());
    m_pRenderer->initialize();
    m_pLogging->logMessage("renderer initialized");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("initializing input handler");
    m_pInputHandler = new InputHandler();
    m_pInputHandler->initialize();
    m_pLogging->logMessage("input handler initialized");

    m_pLogging->logMessage("\n==================================================");
    m_pLogging->logMessage("initializing sound system");
    m_pSoundSystem = new FmodSoundSystem();
    m_pSoundSystem->initialize();
    m_pLogging->logMessage("sound system initialized");

    m_pLogging->logMessage("\n==================================================");

    m_pUpdateFramework->registerInitializeCallback([&]()
    {
        m_pLogging->logMessage("initializing physics system");
        m_pPhysicsSystem = new HavokPhysicsManager();
        m_pPhysicsSystem->initialize();
        m_pLogging->logMessage("physics system initialized");
    });
    m_pUpdateFramework->registerDestroyCallback([&]()
    {
        m_pLogging->logMessage("destroying physics system");
        if(m_pPhysicsSystem) m_pPhysicsSystem->destroy();
        DELETE_AND_NULL(m_pPhysicsSystem);
        m_pLogging->logMessage("physics system destroyed");
    });

    // Make sure the scripting system is destroyed BEFORE the physics system.
    m_pUpdateFramework->registerDestroyCallback([&](){ m_pScriptingManager->destroy(); });

    m_pUpdateFramework->registerInitializeCallback([&]()
    {
        m_pEventManager = new GlobalEventManager();
        m_pEventManager->initialize();
    });
    m_pUpdateFramework->registerDestroyCallback([&]()
    {
        m_pEventManager->destroy();
        DELETE_AND_NULL(m_pEventManager);
    });
    m_pUpdateFramework->registerUpdateCallback([&](float elapsedMilliseconds)
    {
        m_pEventManager->update(elapsedMilliseconds);
    });

    m_pUpdateFramework->registerInitializeCallback([&]()
    {
        m_pLogging->logMessage("initializing camera system");
        m_pCameraManager = new CameraManager();
        m_pLogging->logMessage("camera system initialized");
    });
    m_pUpdateFramework->registerDestroyCallback([&]()
    {
        m_pLogging->logMessage("destroying camera system");
        DELETE_AND_NULL(m_pCameraManager);
        m_pLogging->logMessage("camera system destroyed");
    });

    m_pUpdateFramework->registerInitializeCallback([&]()
    {
        m_pLogging->logMessage("initializing animation system");
        m_pAnimationSystem = new AnimationSystem();
        m_pAnimationSystem->initialize();
        m_pLogging->logMessage("animation system initialized");
    });
    m_pUpdateFramework->registerDestroyCallback([&]()
    {
        m_pLogging->logMessage("destroying animation system");
        if(m_pAnimationSystem) m_pAnimationSystem->destroy();
        DELETE_AND_NULL(m_pAnimationSystem);
        m_pLogging->logMessage("camera animation destroyed");
    });

    m_pLogging->logMessage("\n"
        "==================================================\n"
        "=== Global Manager Initialized ===================\n"
        "==================================================\n");
}

void gep::GlobalManager::destroy()
{
    m_pLogging->logMessage("\n==================================================");

    // Make sure the scripting manager is destroyed.
    m_pScriptingManager->destroy();

    m_pLogging->logMessage("destroying update framework");
    DELETE_AND_NULL(m_pUpdateFramework);
    m_pLogging->logMessage("update framework destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying input handler");
    if(m_pInputHandler) m_pInputHandler->destroy();
    DELETE_AND_NULL(m_pInputHandler);
    m_pLogging->logMessage("input handler destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying timer");
    DELETE_AND_NULL(m_pTimer);
    m_pLogging->logMessage("timer destroyed");

    m_pLogging->logMessage("\n==================================================");
    

    m_pLogging->logMessage("destroying resource manager");
    if(m_pResourceManager) m_pResourceManager->destroy();
    DELETE_AND_NULL(m_pResourceManager);
    m_pLogging->logMessage("resource manager destroyed");
    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying sound system");
    if(m_pSoundSystem) m_pSoundSystem->destroy();
    DELETE_AND_NULL(m_pSoundSystem);
    m_pLogging->logMessage("sound system destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying renderer");
    if(m_pRenderer) m_pRenderer->destroy();
    DELETE_AND_NULL(m_pRenderer);
    m_pLogging->logMessage("renderer destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying renderer extractor");
    DELETE_AND_NULL(m_pRendererExtractor);
    m_pLogging->logMessage("renderer extractor destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying memory manager");
    if(m_pMemoryManager) m_pMemoryManager->destroy();
    DELETE_AND_NULL(m_pMemoryManager);
    m_pLogging->logMessage("memory manager destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying task queue");
    DELETE_AND_NULL(m_pTaskQueue);
    m_pLogging->logMessage("task queue destroyed");

    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying scripting system");
    DELETE_AND_NULL(m_pScriptingManager);
    m_pLogging->logMessage("scripting system destroyed");
    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying settings");
    DELETE_AND_NULL(m_pSettings);
    m_pLogging->logMessage("settings destroyed");
    m_pLogging->logMessage("\n==================================================");

    m_pLogging->logMessage("destroying log system");
    m_pLogging->deregisterSink(m_FileLogSink);
    DELETE_AND_NULL(m_FileLogSink);
#ifdef _DEBUG
    m_pLogging->deregisterSink(m_ConsoleLogSink);
    DELETE_AND_NULL(m_ConsoleLogSink);
#endif
    DELETE_AND_NULL(m_pLogging);
}

