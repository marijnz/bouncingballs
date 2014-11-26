#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/manager.h"

#include "gep/globalManager.h"
#include "gep/container/DynamicArray.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/updateFramework.h"

#include "gepimpl/havok/util.h"
#include "gepimpl/subsystems/physics/havok/displayHandler.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/world.h"


gep::HavokPhysicsManager::HavokPhysicsManager(uint32 options) :
    #ifdef GEP_USE_HK_VISUAL_DEBUGGER
    m_pVisualDebugger(nullptr),
    #endif // GEP_USE_HK_VISUAL_DEBUGGER
    m_pWorld(nullptr),
    m_options(options),
    m_pPhysicsContext(nullptr),
    m_pDisplayManager(nullptr),
    /// \todo Due to a bug, the cause of which is unknown, we are producing memory leaks when creating physics shapes.
    ///       To circumvent the message about the leaks, we cause those leaks in an untracked area => malloc
    // m_factoryAllocator(&g_stdAllocator) // Use this version to see the leaks.
    m_factoryAllocator(MallocAllocatorStatisticsPolicy::getAllocator())
{
}

gep::HavokPhysicsManager::~HavokPhysicsManager()
{
}

void gep::HavokPhysicsManager::initialize()
{
    m_pFactory = new HavokPhysicsFactory(&m_factoryAllocator);
    m_pFactory->initialize();

    // set up display manager
    m_pDisplayManager = new HavokDisplayManager();
    m_pDisplayManager->initialize();
    if (m_options & Options::DebugDrawing)
        m_pDisplayManager->setEnabled(true);

    // set up contexts
    m_pPhysicsContext = new hkpPhysicsContext();

    // Make a list of the contexts, which is used later on.
    hkArray<hkProcessContext*> contexts;
    contexts.pushBack(m_pPhysicsContext);

    // set up processes

    hkArray<const char*> processNames;

    // Push back the default display so we can see the low level hkDebugDisplay lines
    hkDebugDisplayProcess::registerProcess();
    processNames.pushBack(hkDebugDisplayProcess::getName());

    hkpPhysicsContext::registerAllPhysicsProcesses();

    processNames.pushBack(hkpShapeDisplayViewer::getName());
    processNames.pushBack(hkpBroadphaseViewer::getName());
    processNames.pushBack(hkpActiveContactPointViewer::getName());
    processNames.pushBack("Trigger Volumes");

    for (const char* processName : processNames)
    {
        hkProcess* process = hkProcessFactory::getInstance().createProcess(processName, contexts);
        GEP_ASSERT(process, "Failed to create havok process - did you register all the processes for the product(s) you are using?");
        process->m_inStream = nullptr;
        process->m_outStream = nullptr;
        process->m_processHandler = nullptr;
        process->m_displayHandler = static_cast<hkDebugDisplayHandler*>(m_pDisplayManager);
        process->init();
        m_physicsProcesses.append(process);
    }

    #ifdef GEP_USE_HK_VISUAL_DEBUGGER
    g_globalManager.getLogging()->logMessage("setting up visual debugger");
    // Setup the visual debugger
    m_pVisualDebugger = new hkVisualDebugger(contexts);
    m_pVisualDebugger->serve();
    #endif // GEP_USE_HK_VISUAL_DEBUGGER
}

void gep::HavokPhysicsManager::destroy()
{
    #ifdef GEP_USE_HK_VISUAL_DEBUGGER
    GEP_HK_REMOVE_REF_AND_NULL(m_pVisualDebugger);
    #endif // GEP_USE_HK_VISUAL_DEBUGGER

    m_pFactory->destroy();
    m_pDisplayManager->destroy();

    m_pWorld = nullptr;
    DELETE_AND_NULL(m_pDisplayManager);
    DELETE_AND_NULL(m_pFactory);
}

void gep::HavokPhysicsManager::update(float elapsedTime)
{
    GEP_ASSERT(m_pWorld, "The physics system cannot be updated without a world!");
    m_pWorld->update(elapsedTime);

    #ifdef GEP_USE_HK_VISUAL_DEBUGGER
    if(m_pVisualDebugger)
        m_pVisualDebugger->step(elapsedTime);
    #endif // GEP_USE_HK_VISUAL_DEBUGGER
}

void gep::HavokPhysicsManager::setWorld(IWorld* value)
{
    GEP_ASSERT(!m_pWorld, "The physics system does currently not support setting the active world a second time!");
    m_pWorld = static_cast<HavokWorld*>(value);
    GEP_ASSERT(dynamic_cast<HavokWorld*>(value) != nullptr, "Wrong kind of world instance for this kind of physics system!");
   
    m_pPhysicsContext->addWorld(m_pWorld->getHkpWorld());

    #ifdef GEP_USE_HK_VISUAL_DEBUGGER
    //TODO: Check if this causes a memory leak.
    m_pVisualDebugger->addContext(m_pPhysicsContext);
    #endif // GEP_USE_HK_VISUAL_DEBUGGER
}

void gep::HavokPhysicsManager::setDebugDrawingEnabled(bool value)
{
    if (m_pDisplayManager)
        m_pDisplayManager->setEnabled(value);
}

bool gep::HavokPhysicsManager::getDebugDrawingEnabled() const 
{
    return m_pDisplayManager && m_pDisplayManager->isEnabled();
}
