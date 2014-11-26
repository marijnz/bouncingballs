#pragma once

#include "gep/math3d/vec2.h"
#include "gep/math3d/color.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    namespace settings
    {
        struct General
        {
            std::wstring applicationTitle;

            General() :
                applicationTitle(L"Gameplay Programming")
            {
            }
        };

        struct Scripts
        {
            std::string mainScript;
            std::string setupScript;
            std::string importantScriptsRoot;
            std::string userScriptsRoot;

            Scripts()
                : mainScript("data/scripts/main.lua")
                , setupScript("data/scripts/setup.lua")
                , importantScriptsRoot("data/base/")
                , userScriptsRoot("data/scripts/")
            {
            }

        };

        struct Video
        {
            uvec2 initialRenderWindowPosition;
            uvec2 screenResolution;
            bool vsyncEnabled;
            bool adaptiveVSyncEnabled;
            float adaptiveVSyncThreshold;
            float adaptiveVSyncTolerance;
            Color clearColor;

            Video() :
                initialRenderWindowPosition(CW_USEDEFAULT, CW_USEDEFAULT),
                screenResolution(1280, 720),
                vsyncEnabled(false),
                adaptiveVSyncEnabled(true),
                adaptiveVSyncThreshold(1.0f / 59.0f), // 59 FPS
                adaptiveVSyncTolerance(1),            // +- 1 FPS
                clearColor(0.0f, 0.125f, 0.3f, 1.0f)  // Blueish color
            {
            }
        };
        
        struct Lua
        {
            size_t maxStackDumpLevel;
            bool callstackTracebackEnabled;
            bool stackDumpEnabled;

            Lua() :
                maxStackDumpLevel(2),
#ifdef _DEBUG
                callstackTracebackEnabled(true),
                stackDumpEnabled(true)
#else
                callstackTracebackEnabled(false),
                stackDumpEnabled(false)
#endif // _DEBUG

            {
            }
        };
    }
    
    // Can be set in scripts
    class ISettings
    {
    public:
        ISettings() {}
        virtual ~ISettings() {}

        virtual void setGeneralSettings(const settings::General& settings) = 0;
        virtual       settings::General& getGeneralSettings() = 0;
        virtual const settings::General& getGeneralSettings() const = 0;

        virtual void setVideoSettings(const settings::Video& settings) = 0;
        virtual       settings::Video& getVideoSettings() = 0;
        virtual const settings::Video& getVideoSettings() const = 0;
        
        virtual void setScriptsSettings(const settings::Scripts& settings) = 0;
        virtual       settings::Scripts& getScriptsSettings() = 0;
        virtual const settings::Scripts& getScriptsSettings() const = 0;

        virtual void setLuaSettings(const settings::Lua& settings) = 0;
        virtual       settings::Lua& getLuaSettings() = 0;
        virtual const settings::Lua& getLuaSettings() const = 0;

        virtual void loadFromScriptTable(ScriptTableWrapper table) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(loadFromScriptTable, "load")
        LUA_BIND_REFERENCE_TYPE_END
    };
}
