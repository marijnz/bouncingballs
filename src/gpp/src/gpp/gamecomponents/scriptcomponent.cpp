#include "stdafx.h"
#include "gpp/gameComponents/scriptComponent.h"
#include "gep/globalManager.h"
#include "gep/interfaces/scripting.h"


gpp::ScriptComponent::ScriptComponent() :
    m_funcRef_initialize(),
    m_funcRef_destroy(),
    m_funcRef_update()
{
}

gpp::ScriptComponent::~ScriptComponent()
{
}

void gpp::ScriptComponent::initalize()
{
    if(m_state == State::Inactive) { return; }

    if (m_funcRef_initialize.isValid())
    {
        g_globalManager.getScriptingManager()->callFunction<void>(
            m_funcRef_initialize,          ///< Function reference
            m_pParentGameObject->getGuid() ///< ID of the game object
        );
    }
    setState(State::Active); // In case we were in the initial state.
}

void gpp::ScriptComponent::destroy()
{
    if(m_state == State::Inactive) { return; }

    if (m_funcRef_destroy.isValid())
    {
        g_globalManager.getScriptingManager()->callFunction<void>(
            m_funcRef_destroy,             ///< Function reference
            m_pParentGameObject->getGuid() ///< ID of the game object
        );
    }
}

void gpp::ScriptComponent::update(float deltaSeconds)
{
    if(m_state == State::Inactive) { return; }

    if (m_funcRef_update.isValid())
    {
        g_globalManager.getScriptingManager()->callFunction<void>(
            m_funcRef_update,               ///< Function reference
            m_pParentGameObject->getGuid(), ///< ID of the game object
            deltaSeconds
        );
    }
}

void gpp::ScriptComponent::setInitializationFunction(gep::ScriptFunctionWrapper funcRef)
{
    m_funcRef_initialize = funcRef;
}

void gpp::ScriptComponent::setDestroyFunction(gep::ScriptFunctionWrapper funcRef)
{
    m_funcRef_destroy = funcRef;
}

void gpp::ScriptComponent::setUpdateFunction(gep::ScriptFunctionWrapper funcRef)
{
    m_funcRef_update = funcRef;
}
