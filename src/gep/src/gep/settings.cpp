#include "stdafx.h"
#include "gepimpl/settings.h"

void gep::Settings::loadFromScriptTable(ScriptTableWrapper table)
{
    /// Load settings from the argument 'table' like this:
    /// ScriptTableWrapper innerTable;
    /// table.tryGet("myInnerTableNameInLua", innerTable);
    /// int theSetting = 42;
    /// innerTable.tryGet("mySetting", theSetting);
    /// // Now you can use theSetting!

    ScriptTableWrapper generalSettings;
    if (table.tryGet("general", generalSettings))
    {
        // Extract the settings
        std::string temp;
        generalSettings.tryGet("applicationTitle", temp);
        m_general.applicationTitle = convertToWideString(temp);
    }

    ScriptTableWrapper scriptsSettings;
    if (table.tryGet("scripts", scriptsSettings))
    {
        scriptsSettings.tryGet("mainScript", m_scripts.mainScript);
        scriptsSettings.tryGet("setupScript", m_scripts.setupScript);
        scriptsSettings.tryGet("importantScriptsRoot", m_scripts.importantScriptsRoot);
        scriptsSettings.tryGet("userScriptsRoot", m_scripts.userScriptsRoot);
    }

    // Get inner table.
    ScriptTableWrapper videoSettings;
    if (table.tryGet("video", videoSettings))
    {
        // Extract the settings
        videoSettings.tryGet("initialRenderWindowPosition", m_video.initialRenderWindowPosition);
        videoSettings.tryGet("screenResolution", m_video.screenResolution);
        videoSettings.tryGet("vsyncEnabled", m_video.vsyncEnabled);
        videoSettings.tryGet("adaptiveVSyncEnabled", m_video.adaptiveVSyncEnabled);
        videoSettings.tryGet("adaptiveVSyncThreshold", m_video.adaptiveVSyncThreshold);
        videoSettings.tryGet("adaptiveVSyncTolerance", m_video.adaptiveVSyncTolerance);
        videoSettings.tryGet("clearColor", m_video.clearColor);
    }

    // NOTE: Make sure to load lua settings last!
    ScriptTableWrapper luaSettings;
    if (table.tryGet("lua", luaSettings))
    {
        // Extract the settings
        luaSettings.tryGet("maxStackDumpLevel", m_lua.maxStackDumpLevel);
        luaSettings.tryGet("callstackTracebackEnabled", m_lua.callstackTracebackEnabled);
        luaSettings.tryGet("stackDumpEnabled", m_lua.stackDumpEnabled);
    }
}
