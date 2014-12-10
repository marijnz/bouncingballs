#include "stdafx.h"

#include "gepimpl/subsystems/scripting.h"
#include "gep/exception.h"
#include "gep/utils.h"
#include "gep/memory/allocator.h"
#include "gep/memory/leakDetection.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/MemoryManager.h"
#include "gep/settings.h"

namespace gep
{
    namespace detail
    {
        void destroyScriptTypeInfoMap()
        {
            getScriptTypeInfoMap().clear();
        }

        ScriptTypeInfoMap_t& getScriptTypeInfoMap()
        {
            static ScriptTypeInfoMap_t scriptTypeInfos;
            static bool initialized = false;

            if (!initialized)
            {
                initialized = true;
                gep::atexit(&destroyScriptTypeInfoMap);
            }

            return scriptTypeInfos;
        }
    }


    void* scriptAllocator(void* userData, void* ptr, size_t originalSize, size_t newSize)
    {
        GEP_ASSERT(userData != nullptr, "Lua allocation called with invalid user data!");
        auto pAllocator = static_cast<ScriptingAllocator*>(userData);

        if (newSize == 0) // We are supposed to free the pointer.
        {
            pAllocator->freeMemory(ptr);
            return nullptr;
        }

        if (ptr == nullptr) // we are supposed to allocate new memory.
        {
            return pAllocator->allocateMemory(newSize);
        }

        auto newPtr = pAllocator->allocateMemory(newSize);
        GEP_ASSERT(newPtr, "Out of memory.");

        auto size = GEP_MIN(originalSize, newSize);
        GEP_ASSERT(size <= newSize);
        GEP_ASSERT(size <= originalSize);

        memcpy(newPtr, ptr, size);
        pAllocator->freeMemory(ptr);

        return newPtr;
    }

    int scriptErrorHandler(lua_State* L)
    {
        auto luaMessage = lua_tostring(L, -1);
        lua_pop(L, 1);

        std::string callStack = "stack <disabled in settings>";
        if(g_globalManager.getSettings()->getLuaSettings().callstackTracebackEnabled)
        {
            callStack = lua::utils::traceback(L);
        }

        std::string stackDump = "<disabled in settings>";
        if(g_globalManager.getSettings()->getLuaSettings().callstackTracebackEnabled)
        {
            stackDump = lua::utils::dumpStack(L);
        }

        auto fmt = "in Lua: %s\n"
                   "Lua call%s\n"
                   "Lua stack dump %s";
        g_globalManager.getLogging()->logError(fmt, luaMessage, callStack.c_str(), stackDump.c_str());
        throw ScriptExecutionException("An error occurred in lua. Check the log.");
        return 0;
    }
}

gep::ScriptingAllocator::ScriptingAllocator(IAllocatorStatistics* pParentAllocator) :
    m_L(nullptr),
    m_pParentAllocator(pParentAllocator),
    m_numAllocations(0),
    m_numFrees(0)
{
}

gep::ScriptingAllocator::~ScriptingAllocator()
{
    m_L = nullptr;
    m_pParentAllocator = nullptr;
}

size_t gep::ScriptingAllocator::getNumAllocations() const
{
    return m_numAllocations;
}

size_t gep::ScriptingAllocator::getNumFrees() const
{
    return m_numFrees;
}

size_t gep::ScriptingAllocator::getNumBytesReserved() const
{
    return 0;
}

size_t gep::ScriptingAllocator::getNumBytesUsed() const
{
    GEP_ASSERT(m_L, "The scripting allocator needs a valid lua state!");
    return lua_gc(m_L, LUA_GCCOUNT, 0) * 1000 + lua_gc(m_L, LUA_GCCOUNTB, 0);
}

gep::IAllocator* gep::ScriptingAllocator::getParentAllocator() const
{
    return m_pParentAllocator;
}

void* gep::ScriptingAllocator::allocateMemory(size_t size)
{
    auto ptr = m_pParentAllocator->allocateMemory(size);
    ++m_numAllocations;
    return ptr;
}

void gep::ScriptingAllocator::freeMemory(void* mem)
{
    if (mem == nullptr)
    {
        return;
    }

    ++m_numFrees;
    m_pParentAllocator->freeMemory(mem);
}

//////////////////////////////////////////////////////////////////////////

gep::ScriptingManager::ScriptingManager(settings::Scripts* pScriptSettings) :
    m_allocator(&g_stdAllocator),
    m_L(nullptr),
    m_state(State::LoadingDisabled),
    m_pScriptSettings(pScriptSettings)
{
}

gep::ScriptingManager::~ScriptingManager()
{
}

void gep::ScriptingManager::initialize()
{
    // Make sure we're clean.
    destroy();

    // create a Lua state with a custom allocator
    m_L = lua_newstate(&gep::scriptAllocator, &m_allocator);
    lua_atpanic(m_L, &gep::scriptErrorHandler);

    m_allocator.setLuaState(m_L);
    g_globalManager.getMemoryManager()->registerAllocator("Lua", &m_allocator);

    // open all standard libraries
    luaL_openlibs(m_L);
}

void gep::ScriptingManager::destroy()
{
    // Make sure it is safe to call this method multiple times.
    if (m_L == nullptr)
        return;

    g_globalManager.getMemoryManager()->deregisterAllocator(&m_allocator);

    lua_close(m_L);
    m_L = nullptr;
}

void gep::ScriptingManager::update(float elapsedTime)
{
}

#define GEP_SCRIPT_LOAD_ERROR(scriptFileNameAsCString, context) \
/* the top of the stack should be the error string */ \
if (lua_isstring(m_L, lua_gettop(m_L)))\
{\
    /* get the top of the stack as the error and pop it off */ \
    const char* theError = lua_tostring(m_L, lua_gettop(m_L));\
    lua_pop(m_L, 1);\
    luaL_error(m_L, theError);\
}\
else\
{\
    luaL_error(m_L, "Unknown error %s Lua file \"%s\"", context, scriptFileNameAsCString);\
}

void gep::ScriptingManager::loadScript(const std::string& filename, LoadOptions::Enum loadOptions)
{
    GEP_ASSERT(m_state == IScriptingManager::State::LoadingEnabled,
               "You are not allowed to load other scripts at this point!", filename, loadOptions);
    auto scriptFileName = constructFileName(filename, loadOptions);

    if (isLoaded(scriptFileName))
    {
        g_globalManager.getLogging()->logMessage("Script is already loaded: %s", scriptFileName.c_str());
        return;
    }

    g_globalManager.getLogging()->logMessage("Loading and executing script: %s", scriptFileName.c_str());

    // load and execute a Lua file
    int err = luaL_loadfile(m_L, scriptFileName.c_str());
    if(err != LUA_OK)
    {
        GEP_SCRIPT_LOAD_ERROR(scriptFileName.c_str(), "loading");
    }
    else
    {
        // if not an error, then the top of the stack will be the function to call to run the file
        err = lua_pcall(m_L, 0, LUA_MULTRET, 0);
        if (err != LUA_OK)
        {
            GEP_SCRIPT_LOAD_ERROR(scriptFileName.c_str(), "executing");
        }
    }

    m_loadedScripts.append(scriptFileName);
}

#undef GEP_SCRIPT_LOAD_ERROR

bool gep::ScriptingManager::isLoaded(const std::string& fileName)
{
    for (auto& loadedScript : m_loadedScripts)
    {
        if (loadedScript == fileName)
            return true;
    }

    return false;
}


std::string gep::ScriptingManager::constructFileName(const std::string& filename, LoadOptions::Enum loadOptions)
{
    if (loadOptions == LoadOptions::None)
    {
        return filename;
    }

    std::stringstream completeName;
    switch (loadOptions)
    {
    case LoadOptions::PathIsAbsolute:
        completeName << filename;
        break;
    case LoadOptions::PathIsRelative:
        completeName << m_pScriptSettings->userScriptsRoot << filename;
        break;
    case LoadOptions::IsImportantScript:
        completeName << m_pScriptSettings->importantScriptsRoot << filename;
        break;
    default:
        GEP_ASSERT(false, "Invalid script loading option!", loadOptions);
        break;
    }
    return completeName.str();
}


void gep::ScriptingManager::bindEnum(const char* enumName, ...)
{
    lua_newtable(m_L);
    const char* ename;
    int         evalue;
    va_list args;
    va_start(args, enumName);
    while ((ename = va_arg(args, const char*)) != 0)
    {
        evalue = va_arg(args, int);
        lua::push(m_L, ename);
        lua::push(m_L, evalue);
        lua_settable(m_L, -3);
    }
    va_end(args);
    lua_setglobal(m_L, enumName);
}

gep::int32 gep::ScriptingManager::memoryUsed() const
{
    return lua_gc(m_L, LUA_GCCOUNT, 0);
}

void gep::ScriptingManager::collectGarbage()
{
    lua_gc(m_L, LUA_GCCOLLECT, 0);
}

void gep::ScriptingManager::debugBreak(const char* message) const
{
    g_globalManager.getLogging()->logMessage("Lua debug break: %s", message);
    auto trace = lua::utils::traceback(m_L);
    GEP_DEBUG_BREAK;
}
