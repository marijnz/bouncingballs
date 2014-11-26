#include "stdafx.h"
#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/enterEvent.h"
#include "gpp/stateMachines/leaveEvent.h"
#include "gpp/stateMachines/updateEvent.h"

#include "gep/memory/allocator.h"
#include "gep/globalManager.h"
#include "gep/interfaces/scripting.h"

gpp::sm::State::State(const NameInfo& nameInfo, gep::IAllocator* pAllocator) :
    m_pAllocator(pAllocator),
    m_pLogging(&DummyLogging::instance()),
    m_nameInfo(nameInfo),
    m_transitions(pAllocator),
    m_onEnter(EnterEvent_t::CInfo(pAllocator)),
    m_onLeave(LeaveEvent_t::CInfo(pAllocator)),
    m_onUpdate(UpdateEvent_t::CInfo(pAllocator))
{
}

gpp::sm::State::~State()
{
    m_pLogging = nullptr;
    m_pAllocator = nullptr;
}

void gpp::sm::State::enter(EnterEventData* pData)
{
    m_pLogging->logMessage(">> %s", getQualifiedName().c_str());
    pData->m_pEnteredState = this;
    getEnterEvent()->trigger(pData);
}

void gpp::sm::State::leave(LeaveEventData* pData)
{
    m_pLogging->logMessage("<< %s", getQualifiedName().c_str());
    pData->m_pCurrentState = this;
    pData->m_pTransitionArray = &m_transitions;
    getLeaveEvent()->trigger(pData);
}

void gpp::sm::State::update(UpdateEventData* pData)
{
    //m_pLogging->logMessage("++ %s", getQualifiedName().c_str());
    pData->m_pCurrentState = this;
    getUpdateEvent()->trigger(pData);
}

gpp::sm::EnterEvent_t* gpp::sm::State::getEnterEvent()
{
    return &m_onEnter;
}

gpp::sm::LeaveEvent_t* gpp::sm::State::getLeaveEvent()
{
    return &m_onLeave;
}

gpp::sm::UpdateEvent_t* gpp::sm::State::getUpdateEvent()
{
    return &m_onUpdate;
}

const std::string& gpp::sm::State::getName() const
{
    return m_nameInfo.getName();
}

std::string gpp::sm::State::getQualifiedName() const
{
    return m_nameInfo.getQualifiedName();
}

void gpp::sm::State::setLogging(gep::ILogging* pLogging)
{
    m_pLogging = pLogging ? pLogging : &DummyLogging::instance();
}

void gpp::sm::State::addTransition(State* to, ConditionFunc_t condition /*= nullptr*/)
{
    GEP_ASSERT(to, "State transition target is invalid.");

    Transition transition;
    transition.to = to;
    transition.condition = condition;

    m_transitions.append(transition);
}

gpp::sm::State::ConditionCheckResult gpp::sm::State::checkConditions() const
{
    ConditionCheckResult result;
    for (auto& transtion : m_transitions)
    {
        if(!transtion.condition || transtion.condition())
        {
            result.nextState = transtion.to;
            continue;
        }
    }
    return result;
}

gpp::sm::NameInfo& gpp::sm::State::getNameInfo()
{
    return m_nameInfo;
}

//////////////////////////////////////////////////////////////////////////

gpp::sm::State::ConditionCheckResult::ConditionCheckResult() :
    nextState(nullptr)
{
}
