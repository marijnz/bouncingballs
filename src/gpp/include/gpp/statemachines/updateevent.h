#pragma once
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/events.h"
#include "gpp/stateMachines/updateStepBehavior.h"

namespace gpp { namespace sm {

    class GPP_API UpdateEventData
    {
        friend class State;
        friend class StateMachine;

    public:
        UpdateEventData();

        State* getCurrentState();
        float getElapsedTime();
        UpdateStepBehavior::Enum getUpdateStepBehavior() const;
        void setUpdateStepBehavior(UpdateStepBehavior::Enum value);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getCurrentState)
            LUA_BIND_FUNCTION(getElapsedTime)
            LUA_BIND_FUNCTION(getUpdateStepBehavior)
            LUA_BIND_FUNCTION(setUpdateStepBehavior)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        State* m_pCurrentState;
        float m_elapsedTime;
        UpdateStepBehavior::Enum m_behavior;
    };
    typedef gep::Event<UpdateEventData*> UpdateEvent_t;

}} // namespace gpp::sm
