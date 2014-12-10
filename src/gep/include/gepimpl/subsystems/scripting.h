#pragma once

#include "gep/settings.h"
#include "gep/interfaces/scripting.h"
#include "gep/container/DynamicArray.h"
#include "gep/container/hashmap.h"

namespace gep
{
    class IAllocatorStatistics;

    class ScriptingAllocator : public IAllocatorStatistics
    {
    public:
        ScriptingAllocator(IAllocatorStatistics* pParentAllocator);
        ~ScriptingAllocator();

        virtual size_t getNumAllocations() const override;
        virtual size_t getNumFrees() const override;
        virtual size_t getNumBytesReserved() const override;
        virtual size_t getNumBytesUsed() const override;

        virtual IAllocator* getParentAllocator() const override;

        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;

        inline void setLuaState(lua_State* L) { m_L = L; }
        inline lua_State* getLuaState() { return m_L; }

    private:
        lua_State* m_L;
        IAllocatorStatistics* m_pParentAllocator;
        size_t m_numAllocations;
        size_t m_numFrees;
    };

    class ScriptingManager : public IScriptingManager
    {
    public:

        ScriptingManager(settings::Scripts* pScriptSettings);
        virtual ~ScriptingManager();

        /// \brief IScriptingManager interface
        virtual void initialize();
        virtual void update(float elapsedTime);
        virtual void destroy();

        virtual void loadScript(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default);

        virtual void setManagerState(State state) override { m_state = state; }
        virtual State getManagerState() const override { return m_state; }

        virtual       std::string& getScriptsRoot()       override { return m_pScriptSettings->userScriptsRoot; }
        virtual const std::string& getScriptsRoot() const override { return m_pScriptSettings->userScriptsRoot; }
        virtual void setScriptsRoot(const std::string& value) override { m_pScriptSettings->userScriptsRoot = value; }

        virtual       std::string& getImportantScriptsRoot()       override { return m_pScriptSettings->importantScriptsRoot; }
        virtual const std::string& getImportantScriptsRoot() const override { return m_pScriptSettings->importantScriptsRoot; }
        virtual void setImportantScriptsRoot(const std::string& value) override { m_pScriptSettings->importantScriptsRoot = value; }

        virtual void bindEnum(const char* enumName, ...) override;

        virtual int32 memoryUsed() const override;
        virtual void collectGarbage() override;

        virtual void debugBreak(const char* message) const override;

        void makeBasicBindings();

    private:
        ScriptingAllocator m_allocator;

        lua_State* m_L;
        State m_state;

        settings::Scripts* m_pScriptSettings;
        gep::DynamicArray<std::string> m_loadedScripts;

        bool isLoaded(const std::string& fileName);

        std::string constructFileName(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default);

        virtual lua_State* getState() override { return m_L; }
    };
}
