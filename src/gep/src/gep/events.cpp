#include "stdafx.h"
#include "gep/interfaces/events.h"
#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/logging.h"

gep::ILogging* gep::EventBase::getLogging() const
{
    return g_globalManager.getLogging();
}

//////////////////////////////////////////////////////////////////////////

gep::IUpdateFramework* gep::EventUpdateFramework::s_pInstance = nullptr;
gep::IScriptingManager* gep::EventScriptingManager::s_pInstance = nullptr;

gep::IUpdateFramework& gep::EventUpdateFramework::instance()
{
    if (s_pInstance == nullptr)
    {
        s_pInstance = g_globalManager.getUpdateFramework();
    }
    
    return *s_pInstance;
}

void gep::EventUpdateFramework::patchInstance(IUpdateFramework* pInstance)
{
    s_pInstance = pInstance;
}

gep::IScriptingManager& gep::EventScriptingManager::instance()
{
    if(s_pInstance == nullptr)
    {
        s_pInstance = g_globalManager.getScriptingManager();
    }

    return *s_pInstance;
}

void gep::EventScriptingManager::patchInstance(IScriptingManager* pInstance)
{
    s_pInstance = pInstance;
}
