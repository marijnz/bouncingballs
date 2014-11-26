#include "stdafx.h"
#include "gpp/stateMachines/updateEvent.h"
#include "gpp/stateMachines/updateStepBehavior.h"

gpp::sm::UpdateEventData::UpdateEventData() :
    m_pCurrentState(nullptr),
    m_elapsedTime(0.0f),
    m_behavior(UpdateStepBehavior::Continue)
{
}

gpp::sm::State* gpp::sm::UpdateEventData::getCurrentState()
{
    return m_pCurrentState;
}

float gpp::sm::UpdateEventData::getElapsedTime()
{
    return m_elapsedTime;
}

gpp::sm::UpdateStepBehavior::Enum gpp::sm::UpdateEventData::getUpdateStepBehavior() const
{
    return m_behavior;
}

void gpp::sm::UpdateEventData::setUpdateStepBehavior(UpdateStepBehavior::Enum value)
{
    m_behavior = value;
}
