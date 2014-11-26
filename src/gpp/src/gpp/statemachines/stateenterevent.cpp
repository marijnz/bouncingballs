#include "stdafx.h"
#include "gpp/stateMachines/enterEvent.h"

gpp::sm::EnterEventData::EnterEventData() :
    m_pEnteredState(nullptr),
    m_pLeftState(nullptr)
{
}

gpp::sm::State* gpp::sm::EnterEventData::getEnteredState()
{
    return m_pEnteredState;
}

gpp::sm::State* gpp::sm::EnterEventData::getLeftState()
{
    return m_pLeftState;
}
