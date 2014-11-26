#pragma once
#include "gep/settings.h"

#include "gep/math3d/vec2.h"

namespace gep
{
    class Settings : public ISettings
    {
        settings::General m_general;
        settings::Video m_video;

        settings::Scripts m_scripts;
        settings::Lua m_lua;
    public:

        virtual void setVideoSettings(const settings::Video& settings) override { m_video = settings; }
        virtual       settings::Video& getVideoSettings()       override { return m_video; }
        virtual const settings::Video& getVideoSettings() const override { return m_video; }

        virtual void setGeneralSettings(const settings::General& settings) override { m_general = m_general; }
        virtual       settings::General& getGeneralSettings()       override { return m_general; }
        virtual const settings::General& getGeneralSettings() const override { return m_general; }
        
        virtual void setScriptsSettings(const settings::Scripts& settings) override { m_scripts = m_scripts; }
        virtual       settings::Scripts& getScriptsSettings()       override { return m_scripts; }
        virtual const settings::Scripts& getScriptsSettings() const override { return m_scripts; }

        virtual void setLuaSettings(const settings::Lua& settings) override { m_lua = settings; }
        virtual       settings::Lua& getLuaSettings() override { return m_lua; }
        virtual const settings::Lua& getLuaSettings() const override { return m_lua; }

        virtual void loadFromScriptTable(ScriptTableWrapper table) override;
    };
}
