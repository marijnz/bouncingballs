#include "stdafx.h"
#include "gpp/game.h"

#include "gep/globalManager.h"
#include "gep/settings.h"
#include "gep/exception.h"
#include "gep/cameras.h"
#include "gep/utils.h"

#include "gep/interfaces/logging.h"
#include "gep/interfaces/physics.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/inputHandler.h"
#include "gep/interfaces/sound.h"

#include "gep/math3d/vec3.h"
#include "gep/math3d/color.h"

#include "gpp/gameObjectSystem.h"

#include "gep/interfaces/cameraManager.h"

#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/stateMachineFactory.h"

#include "gep/memory/leakDetection.h"

#include "gpp/gameComponents/cameraComponent.h"
#include "gep/interfaces/memoryManager.h"

namespace
{
    gep::MallocAllocator g_mallocAllocator;
}

using namespace gep;

gpp::Game::Game() :
    m_pStateMachineFactory(nullptr),
    m_pDummyCam(nullptr),
    m_pStateMachine(nullptr),
    m_continueRunningGame(false),
    m_memoryManagerUpdate(-1),
    m_vsyncMode(VSyncMode::EnumMin),
    m_printDebugInfo(false)
{
#ifdef _DEBUG
    m_printDebugInfo = true;
#endif
}

gpp::Game::~Game()
{
    m_pDummyCam = nullptr;
    m_pStateMachine = nullptr;
}

void gpp::Game::initialize()
{
    auto pRenderer = g_globalManager.getRenderer();

    if      (pRenderer->getVSyncEnabled())         { m_vsyncMode = VSyncMode::On;       }
    else if (pRenderer->getAdaptiveVSyncEnabled()) { m_vsyncMode = VSyncMode::Adaptive; }
    else                                           { m_vsyncMode = VSyncMode::Off;      }

    m_continueRunningGame = true;

    // register render callback
    g_globalManager.getRendererExtractor()->registerExtractionCallback(std::bind(&Game::render, this, std::placeholders::_1));
    m_pDummyCam = []{
        auto temp = new FreeCameraHorizon();
        temp->setViewAngle(60.0f);
        return temp;
    }();
    g_globalManager.getCameraManager()->setActiveCamera(m_pDummyCam);

    m_pStateMachineFactory = GEP_NEW(g_stdAllocator, sm::StateMachineFactory)(&g_stdAllocator);
    m_pStateMachineFactory->initialize();

    m_pStateMachine = m_pStateMachineFactory->create("game");
    setUpStateMachine();

    // Scripting related initialization
    //////////////////////////////////////////////////////////////////////////

    auto scripting = g_globalManager.getScriptingManager();

    makeScriptBindings();

    {
        auto scriptSettings = g_globalManager.getSettings()->getScriptsSettings();
        scripting->setManagerState(IScriptingManager::State::LoadingEnabled);
        SCOPE_EXIT { scripting->setManagerState(IScriptingManager::State::LoadingDisabled); });

        // Default: data/base/setup.lua
        scripting->loadScript(scriptSettings.setupScript, IScriptingManager::LoadOptions::PathIsAbsolute);

        // Default: data/scripts/main.lua
        scripting->loadScript(scriptSettings.mainScript, IScriptingManager::LoadOptions::PathIsAbsolute);
    }

    auto pPhysicsWorld = g_globalManager.getPhysicsSystem()->getWorld();
    GEP_ASSERT(pPhysicsWorld != nullptr,
               "You need to create a proper physics world or include \"defaults/physicsWorld.lua\"!");
    gep::IWorld::ScopedLock physicsWorldLock(pPhysicsWorld);

    m_pStateMachine->run();
    g_gameObjectManager.initialize();
}

void gpp::Game::destroy()
{
    m_pStateMachineFactory->destroy();
    GEP_DELETE(g_stdAllocator, m_pStateMachineFactory);

    g_gameObjectManager.destroy();

    DELETE_AND_NULL(m_pDummyCam);
}

void gpp::Game::update(float elapsedTime)
{
    if (!m_continueRunningGame)
    {
        return;
    }

    auto pInputHandler = g_globalManager.getInputHandler();
    auto pPhysicsSystem = g_globalManager.getPhysicsSystem();
    auto pRenderer = g_globalManager.getRenderer();

    if (pInputHandler->wasTriggered(gep::Key::F9))
    {
        pPhysicsSystem->setDebugDrawingEnabled(!pPhysicsSystem->getDebugDrawingEnabled());
    }

    if (pInputHandler->wasTriggered(gep::Key::F8)) // Toggle VSync
    {
        m_vsyncMode.advanceToNext();
        updateVSyncState();
    }

    if (pInputHandler->wasTriggered(gep::Key::F7))
    {
        toggleDisplayMemoryStatistics();
    }

    if (pInputHandler->wasTriggered(gep::Key::F6))
    {
        m_printDebugInfo = !m_printDebugInfo;
    }

    if (m_printDebugInfo)
    {
        auto averageElapsedTime = g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60);

        auto& debugRenderer = pRenderer->getDebugRenderer();
        debugRenderer.printText(pRenderer->toNormalizedScreenPosition(uvec2(10, 10)), gep::format("Current FPS: %.1f", 1 / elapsedTime).c_str());
        debugRenderer.printText(pRenderer->toNormalizedScreenPosition(uvec2(10, 30)), gep::format("Average FPS: %.1f (last 60 frames)", 1 / averageElapsedTime).c_str());
        debugRenderer.printText(pRenderer->toNormalizedScreenPosition(uvec2(10, 50)), gep::format("VSync: %s", m_vsyncMode.toString()).c_str());
    }

    g_gameObjectManager.update(elapsedTime);

    auto pSoundSystem = g_globalManager.getSoundSystem();
    auto activeCamObj = g_gameObjectManager.getCurrentCameraObject();
    if(activeCamObj)
    {
        pSoundSystem->setListenerPosition(activeCamObj->getWorldPosition());
        pSoundSystem->setListenerOrientation(activeCamObj->getWorldRotation());
    }
    else
    {
        pSoundSystem->setListenerPosition(vec3());
        pSoundSystem->setListenerOrientation(Quaternion());
    }
}

void gpp::Game::render(gep::IRendererExtractor& extractor)
{
    auto activeCam = g_globalManager.getCameraManager()->getActiveCamera();
    DebugMarkerSection marker(extractor, "Main");
    extractor.setCamera(activeCam);
}

void gpp::Game::setUpStateMachine()
{
    using namespace sm;

    auto pLogging = g_globalManager.getLogging();
    m_pStateMachine->setLogging(pLogging);

    // Add listeners
    //////////////////////////////////////////////////////////////////////////
    m_pStateMachine->getLeaveEvent()->registerListener([this](LeaveEventData*){
        g_globalManager.getUpdateFramework()->stop();
		m_continueRunningGame = false;
        return gep::EventResult::Handled;
    });

    // Finalize
    //////////////////////////////////////////////////////////////////////////
    pLogging->logMessage("State machine legend:\n"
                         "  >> Entering state or state machine\n"
                         "  << Leaving state or state machine");
}

void gpp::Game::toggleDisplayMemoryStatistics()
{
    auto pUpdateFramework = g_globalManager.getUpdateFramework();

    if (m_memoryManagerUpdate.id == -1)
    {
        m_memoryManagerUpdate = pUpdateFramework->registerUpdateCallback(
            std::bind(&gep::IMemoryManager::update, g_globalManager.getMemoryManager(), std::placeholders::_1));
    }
    else
    {
        pUpdateFramework->deregisterUpdateCallback(m_memoryManagerUpdate);
        m_memoryManagerUpdate.id = -1;
    }
}

void gpp::Game::updateVSyncState()
{
    auto pRenderer = g_globalManager.getRenderer();

    if (m_vsyncMode.isOff())
    {
        pRenderer->setVSyncEnabled(false);
        pRenderer->setAdaptiveVSyncEnabled(false);
    }
    else if(m_vsyncMode.isOn())
    {
        pRenderer->setVSyncEnabled(true);
        pRenderer->setAdaptiveVSyncEnabled(false);
    }
    else if (m_vsyncMode.isAdaptive())
    {
        pRenderer->setVSyncEnabled(false);
        pRenderer->setAdaptiveVSyncEnabled(true);
    }
    else
    {
        GEP_ASSERT(false, "Invalid VSync state.");
    }
}
