#include "stdafx.h"
#include "gepimpl/subsystems/updateFramework.h"
#include "gep/globalManager.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/inputHandler.h"
#include "gep/interfaces/sound.h"
#include "gep/interfaces/physics.h"

gep::UpdateFramework::UpdateFramework() :
    m_FrameTimesPtr(m_pFrameTimesArray)
    , m_frameIdx(m_FrameTimesPtr.length()-1)
    , m_running(true)
    , m_timeOfLastFrame(g_globalManager.getTimer())
    , m_gameThread(this)
{
    // initialize the frame times array to some default value
    const float defaultTime = 1.0f / 60.0f;
    for (auto& frametime : m_FrameTimesPtr)
    {
        frametime = defaultTime;
    }

}

void gep::UpdateFramework::stop()
{
    m_running = false;
    m_gameThread.m_execute = false;
}

void gep::UpdateFramework::run()
{
    m_timeOfLastFrame = g_globalManager.getTimer();
    // start the game simulation
    m_gameThread.start();
    while(m_running)
    {
        PointInTime now(g_globalManager.getTimer());
        float elapsedTime = now - m_timeOfLastFrame;
        m_timeOfLastFrame = now;

        m_frameIdx = (m_frameIdx + 1) % m_FrameTimesPtr.length();
        m_FrameTimesPtr[m_frameIdx] = elapsedTime;

        m_gameThread.m_gameEndLock.waitAndDecrement();
        // From here on only 1 thread runs

        m_gameThread.m_elapsedTime = elapsedTime;

        g_globalManager.getInputHandler()->update(elapsedTime);

        g_globalManager.getResourceManager()->update(elapsedTime);

        g_globalManager.getSoundSystem()->update(elapsedTime);

        m_gameThread.m_gameStartLock.increment(); //signal the game thread
        // From here on multiple threads run

        g_globalManager.getRenderer()->update(elapsedTime);
    }
    // wait for the game thread to finish
    m_gameThread.m_gameDestroyLock.increment();
    m_gameThread.join();
}

void gep::UpdateFramework::runGame(float elapsedTime)
{
    for(auto& listener : m_toUpdate)
    {
        if(listener)
            listener(elapsedTime);
    }

    g_globalManager.getPhysicsSystem()->update(elapsedTime);

    g_globalManager.getRendererExtractor()->extract();
}


void gep::UpdateFramework::initializeGame()
{
    g_globalManager.getLogging()->logMessage("Initializing in game thread.");
    for(auto& listener : m_toInitialize)
    {
        if(listener)
            listener();
    }
}

void gep::UpdateFramework::destroyGame()
{
    g_globalManager.getLogging()->logMessage("Destroying in game thread.");
    while (m_toDestroy.length() > 0)
    {
        auto& listener = m_toDestroy.lastElement();
        if (listener)
            listener();
        m_toDestroy.removeLastElement();
    }
}


float gep::UpdateFramework::getElapsedTime() const
{
    return m_FrameTimesPtr[m_frameIdx];
}

float gep::UpdateFramework::calcElapsedTimeAverage(size_t numFrames) const
{
    GEP_ASSERT(numFrames <= m_FrameTimesPtr.length(), "numFrames is too large!");
    GEP_ASSERT(numFrames > 0, "numFrames must not be 0!");

    float averageTime = 0.0f;

    for (size_t tempFrameIdx = m_frameIdx, i = 0; i < numFrames; ++i)
    {
        float frameTime = m_FrameTimesPtr[tempFrameIdx];

        averageTime += frameTime;

        // Count the index down and wrap around if needed
        tempFrameIdx = (tempFrameIdx == 0 ? m_FrameTimesPtr.length() - 1 : tempFrameIdx - 1);
    }

    return averageTime / float(numFrames);
}

gep::CallbackId gep::UpdateFramework::registerUpdateCallback(std::function<void(float elapsedTime)> callback)
{
    for(size_t i=0; i < m_toUpdate.length(); ++i)
    {
        if(!m_toUpdate[i])
        {
            m_toUpdate[i] = callback;
            return CallbackId(i);
        }
    }
    m_toUpdate.append(callback);
    return gep::CallbackId(m_toUpdate.length() - 1);
}

gep::CallbackId gep::UpdateFramework::registerInitializeCallback(std::function<void()> callback)
{
    for(size_t i=0; i <m_toInitialize.length(); ++i)
    {
        if(!m_toInitialize[i])
        {
            m_toInitialize[i] = callback;
            return CallbackId(i);
        }
    }
    m_toInitialize.append(callback);
    return gep::CallbackId(m_toInitialize.length() - 1);
}

gep::CallbackId gep::UpdateFramework::registerDestroyCallback(std::function<void()> callback)
{
    for(size_t i=0; i <m_toDestroy.length(); ++i)
    {
        if(!m_toDestroy[i])
        {
            m_toDestroy[i] = callback;
            return CallbackId(i);
        }
    }
    m_toDestroy.append(callback);
    return gep::CallbackId(m_toDestroy.length() - 1);
}

void gep::UpdateFramework::deregisterUpdateCallback(CallbackId id)
{
    GEP_ASSERT(id.id < m_toUpdate.length(), "callback id out of bounds");
    GEP_ASSERT(m_toUpdate[id.id], "callback was already deregistered");
    m_toUpdate[id.id] = nullptr;
}

void gep::UpdateFramework::deregisterInitializeCallback(CallbackId id)
{
    GEP_ASSERT(id.id < m_toInitialize.length(), "callback id out of bounds");
    GEP_ASSERT(m_toInitialize[id.id], "callback was already deregistered");
    m_toInitialize[id.id] = nullptr;
}

void gep::UpdateFramework::deregisterDestroyCallback(CallbackId id)
{
    GEP_ASSERT(id.id < m_toDestroy.length(), "callback id out of bounds");
    GEP_ASSERT(m_toDestroy[id.id], "callback was already deregistered");
    m_toDestroy[id.id] = nullptr;
}

gep::GameThread::GameThread(UpdateFramework* pUpdateFramework) :
    m_gameStartLock(1),
    m_gameEndLock(0),
    m_gameDestroyLock(0),
    m_execute(true),
    m_pUpdateFramework(pUpdateFramework),
    m_elapsedTime(1.0f / 60.0f)
{
}

void gep::GameThread::run()
{
    try
    {
        m_pUpdateFramework->initializeGame();

        while(m_execute)
        {
            m_gameStartLock.waitAndDecrement();
            m_pUpdateFramework->runGame(m_elapsedTime);
            m_gameEndLock.increment();
        }

        m_gameDestroyLock.waitAndDecrement();
        m_pUpdateFramework->destroyGame(); //Synchronize with renderer Thread!
    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("Exception in game thread:\n%s", ex.what());
    }
    m_execute = false;
}

