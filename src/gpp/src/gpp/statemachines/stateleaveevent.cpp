#include "stdafx.h"
#include "gpp/stateMachines/leaveEvent.h"
#include "gpp/stateMachines/stateMachine.h"

#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

gpp::sm::LeaveEventData::LeaveEventData() :
    m_pCurrentStateMachine(nullptr),
    m_pCurrentState(nullptr),
    m_pNextState(nullptr),
    m_pTransitionArray(nullptr)
{
}

gpp::sm::State* gpp::sm::LeaveEventData::getCurrentState()
{
    return m_pCurrentState;
}

gpp::sm::State* gpp::sm::LeaveEventData::getNextState()
{
    return m_pNextState;
}

const gpp::sm::State* gpp::sm::LeaveEventData::getNextState() const
{
    return m_pNextState;
}

void gpp::sm::LeaveEventData::setNextState(State* pNext)
{
    for (auto& transition : *m_pTransitionArray)
    {
        if(transition.to == pNext)
        {
            m_pNextState = pNext;
            return;
        }
    }
    
    GEP_ASSERT(false, "The next state you want to make a transition to is invalid!", pNext->getName());
    g_globalManager.getLogging()->logError("The next state you want to make a transition to is invalid!");
}

void gpp::sm::LeaveEventData::setNextState(const std::string& next)
{
    setNextState(m_pCurrentStateMachine->get<State>(next));
}
