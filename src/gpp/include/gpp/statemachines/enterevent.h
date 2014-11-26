#pragma once
#include "gep/interfaces/scripting.h"

#include "gep/interfaces/events.h"

namespace gpp { namespace sm {

    class GPP_API EnterEventData
    {
        friend class State;
        friend class StateMachine;

    public:
        EnterEventData();

        State* getEnteredState();
        State* getLeftState();

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getEnteredState)
            LUA_BIND_FUNCTION(getLeftState)
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        State* m_pEnteredState;
        State* m_pLeftState;
    };
    typedef gep::Event<EnterEventData*> EnterEvent_t;

}} // namespace gpp::sm
