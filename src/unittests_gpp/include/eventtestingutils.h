#pragma once

#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/events.h"
#include "gep/interfaces/scripting.h"

class EventTestUpdateFramework : public gep::EventUpdateFramework
{
    float m_elapsedTime;
    gep::Hashmap<gep::CallbackId, std::function<void(float)>> m_updateCallbacks;
public:

    EventTestUpdateFramework() :
        m_elapsedTime(0.0f),
        m_updateCallbacks()
    {
    }

    // EventUpdateFramework interface
    virtual void stop() override
    {
        GEP_ASSERT(false, "Not supposed to be called.");
    }
    virtual void run() override
    {
        for(auto callback : m_updateCallbacks.values())
        {
            if(callback)
            {
                callback(m_elapsedTime);
            }
        }
    }
    virtual float getElapsedTime() const override
    {
        return m_elapsedTime;
    }
    virtual float calcElapsedTimeAverage(size_t numFrames) const override
    {
        return m_elapsedTime;
    }
    virtual gep::CallbackId registerUpdateCallback(std::function<void(float elapsedTime)> callback) override
    {
        static size_t callbackCounter(0);
        gep::CallbackId id(callbackCounter++);
        m_updateCallbacks[id] = callback;
        return id;
    }
    virtual void deregisterUpdateCallback(gep::CallbackId id) override
    {
        m_updateCallbacks.remove(id);
    }
    virtual gep::CallbackId registerInitializeCallback(std::function<void() > callback) override
    {
        GEP_ASSERT(false, "Not supposed to be called.");
        return gep::CallbackId(-1);
    }
    virtual gep::CallbackId registerDestroyCallback(std::function<void() > callback) override
    {
        GEP_ASSERT(false, "Not supposed to be called.");
        return gep::CallbackId(-1);
    }
    virtual void deregisterInitializeCallback(gep::CallbackId id) override
    {
        GEP_ASSERT(false, "Not supposed to be called.");
    }
    virtual void deregisterDestroyCallback(gep::CallbackId id) override
    {
        GEP_ASSERT(false, "Not supposed to be called.");
    }

    void setElapsedTime(float time)
    {
        m_elapsedTime = time;
    }
};

class TestScriptingManager : public gep::IScriptingManager
{
    virtual void loadScript(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default) override {}
    virtual void setManagerState(State state) override {}
    virtual State getManagerState() const override
    {
        return State::LoadingEnabled;
    }
    virtual lua_State* getState() override
    {
        GEP_ASSERT(false, "Not supposed to be called.");
        return nullptr;
    }
    virtual std::string& getScriptsRoot() override
    {
        static std::string str("");
        return str;
    }
    virtual const std::string& getScriptsRoot() const override
    {
        static std::string str("");
        return str;
    }
    virtual void setScriptsRoot(const std::string&) override {}
    virtual std::string& getImportantScriptsRoot() override
    {
        static std::string str("");
        return str;
    }
    virtual const std::string& getImportantScriptsRoot() const override
    {
        static std::string str("");
        return str;
    }
    virtual void setImportantScriptsRoot(const std::string&) override {}
    virtual gep::int32 memoryUsed() const override { return 0; }
    virtual void collectGarbage() override {}
    virtual void debugBreak(const char*) const override {}
    virtual void bindEnum(const char* enumName, ...) override {}
    virtual void initialize() override {}
    virtual void destroy() override {}
};

#define GEP_UNITTEST_SETUP_EVENT_GLOBALS                           \
    EventTestUpdateFramework _updateFramework;                     \
    gep::EventUpdateFramework::patchInstance(&_updateFramework);   \
    TestScriptingManager _scriptingManager;                        \
    gep::EventScriptingManager::patchInstance(&_scriptingManager); \
    SCOPE_EXIT{                                                    \
        gep::EventUpdateFramework::patchInstance(nullptr);         \
        gep::EventScriptingManager::patchInstance(nullptr); })
