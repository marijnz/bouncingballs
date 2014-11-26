#include "stdafx.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/state.h"

#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"


//////////////////////////////////////////////////////////////////////////

gpp::sm::StateMachine::StateMachine(const NameInfo& nameInfo, gep::IAllocator* pAllocator) :
    State(nameInfo, pAllocator),
    m_defaultLeaveState(NameInfo(leaveStateName(), &m_nameInfo), pAllocator),
    m_pLeaveState(&m_defaultLeaveState),
    m_pCurrentState(m_pLeaveState),
    m_innerStates(m_pAllocator)
{
}

gpp::sm::StateMachine::~StateMachine()
{
    for(auto pFsm : m_innerStates.values())
    {
        GEP_DELETE(m_pAllocator, pFsm);
    }
    m_innerStates.clear();
}

void gpp::sm::StateMachine::run()
{
    // The other run() will take care to get the default update framework.
    run(*g_globalManager.getUpdateFramework());
}

void gpp::sm::StateMachine::run(gep::IUpdateFramework& updateFramework)
{
    EnterEventData data;
    run(updateFramework, data);
}

void gpp::sm::StateMachine::run(gep::IUpdateFramework& updateFramework, EnterEventData& data)
{
    enter(&data);
    hookIntoUpdateFramework(&updateFramework);
}

void gpp::sm::StateMachine::addTransition(const std::string& from, const std::string& to, ConditionFunc_t condition /*= nullptr*/)
{
    auto fromState = get<State>(from);
    GEP_ASSERT(fromState, "The state you want to add a transition to does not exist.", from);

    auto toState = get<State>(to);
    GEP_ASSERT(toState, "The state you want to make a transition to does not exist.", to);

    if(fromState == this)
    {
        State::addTransition(toState, condition);
    }
    else
    {
        fromState->addTransition(toState, condition);
    }
}

void gpp::sm::StateMachine::addTransition(State* to, ConditionFunc_t condition)
{
    m_pLeaveState->addTransition(to, condition);
}

void gpp::sm::StateMachine::addTransition(const std::string& from, const std::string& to, gep::ScriptFunctionWrapper condition)
{
    addTransition(from, to, [=](){
        return g_globalManager.getScriptingManager()->callFunction<bool>(condition);
    });
}

void gpp::sm::StateMachine::enter(EnterEventData* pData)
{
    State::enter(pData);
    pData->m_pLeftState = m_pCurrentState;
    if (m_pCurrentState == m_pLeaveState)
    {
        m_pCurrentState = State::checkConditions().nextState;
    }

    GEP_ASSERT(m_pCurrentState, "There is no state to enter to!");

    m_pCurrentState->enter(pData);
}

void gpp::sm::StateMachine::leave(LeaveEventData* pData)
{    
    m_pLogging->logMessage("<< %s", getQualifiedName().c_str());
    pData->m_pCurrentState = this;
    pData->m_pTransitionArray = &m_pLeaveState->m_transitions;
    getLeaveEvent()->trigger(pData);
    auto conditionResult = checkConditions();
    pData->m_pNextState = conditionResult.nextState;
}

void gpp::sm::StateMachine::update(UpdateEventData* pData)
{
    GEP_ASSERT(m_pCurrentState);
    pData->m_pCurrentState = this;

    UpdateEventData updateData;
    updateData.m_elapsedTime = pData->m_elapsedTime;
    m_pCurrentState->update(&updateData);

    auto updateStepBehavior = updateData.getUpdateStepBehavior();
    ConditionCheckResult conditionResult;

    if(updateStepBehavior != UpdateStepBehavior::LeaveWithNoConditionChecks)
    {
        conditionResult = m_pCurrentState->checkConditions();
    }
    
    // If there are no next states to get into and we are supposed to continue with the current state, abort.
    if(!conditionResult.hasNextState() && updateStepBehavior == UpdateStepBehavior::Continue)
    {
        return;
    }

    // Leave the current state and enter the next one
    //////////////////////////////////////////////////////////////////////////
    auto pLeftState = m_pCurrentState;
    LeaveEventData leaveData;
    leaveData.m_pCurrentStateMachine = this;
    leaveData.m_pNextState = conditionResult.nextState;
    pLeftState->leave(&leaveData);


    // Try to get the next state
    GEP_ASSERT(leaveData.getNextState(), "There must be a next state after leaving the current state!");
    m_pCurrentState = leaveData.getNextState();

    // If we can leave, signalize it to the parent state machine.
    if(m_pCurrentState == m_pLeaveState)
    {
        pData->setUpdateStepBehavior(UpdateStepBehavior::Leave);
        return;
    }

    // Enter the next state
    EnterEventData enterData;
    enterData.m_pLeftState = pLeftState;
    m_pCurrentState->enter(&enterData);
}

void gpp::sm::StateMachine::hookIntoUpdateFramework(gep::IUpdateFramework* ufx)
{
    GEP_ASSERT(ufx, "Update framework is a nullptr.");
    // register update()
    auto id = ufx->registerUpdateCallback([=](float dt){
        UpdateEventData data;
        data.m_elapsedTime = dt;
        update(&data);
        if(data.getUpdateStepBehavior() != UpdateStepBehavior::Continue)
        {
            LeaveEventData data;
            leave(&data);
        }
    });
    // deregister update() once we are leaving this state machine
    getLeaveEvent()->registerListener([=](LeaveEventData*){
        ufx->deregisterUpdateCallback(id);
        return gep::EventResult::Handled;
    });
}

gpp::sm::State::ConditionCheckResult gpp::sm::StateMachine::checkConditions() const
{
    return m_pLeaveState->checkConditions();
}

gpp::sm::NameInfo& gpp::sm::StateMachine::getNameInfo()
{
    return State::getNameInfo();
}
