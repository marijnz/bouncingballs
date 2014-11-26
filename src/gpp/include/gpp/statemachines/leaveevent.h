#pragma once
#include "gpp/stateMachines/transition.h"

#include "gep/interfaces/scripting.h"
#include "gep/interfaces/events.h"

namespace gpp { namespace sm {

    class GPP_API LeaveEventData
    {
        friend class State;
        friend class StateMachine;

    public:
        LeaveEventData();

        State* getCurrentState();

        State* getNextState();
        const State* getNextState() const;
        void setNextState(State* pNext);
        void setNextState(const std::string& next);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getCurrentState)
            LUA_BIND_FUNCTION_PTR(static_cast<void(LeaveEventData::*)(State*)>(&setNextState), "setNextState")
            LUA_BIND_FUNCTION_PTR(static_cast<void(LeaveEventData::*)(const std::string&)>(&setNextState), "setNextStateByName")
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        StateMachine* m_pCurrentStateMachine;
        State* m_pCurrentState;
        State* m_pNextState;
        TransitionArray_t* m_pTransitionArray;
    };
    typedef gep::Event<LeaveEventData*> LeaveEvent_t;

}} // namespace gpp::sm
