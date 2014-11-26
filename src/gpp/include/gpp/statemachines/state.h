#pragma once

#include "gep/interfaces/scripting.h"

#include "gep/interfaces/events.h"
#include "gep/container/hashmap.h"
#include "gep/interfaces/updateFramework.h"
#include "gpp/dummyLogging.h"

#include "gpp/stateMachines/stateNameInfo.h"
#include "gpp/stateMachines/transition.h"
#include "gpp/stateMachines/enterEvent.h"
#include "gpp/stateMachines/leaveEvent.h"
#include "gpp/stateMachines/updateEvent.h"

namespace gep
{
    class IAllocator;
}

namespace gpp { namespace sm {

    class StateMachine;

    class GPP_API State
    {
        friend class StateMachine;

    private:

        struct ConditionCheckResult
        {
            State* nextState;

            /// \brief Returns \c if at least one condition was true
            bool hasNextState();

            ConditionCheckResult();
        };

    public:

        virtual ~State();

        virtual void enter(EnterEventData* pData);
        virtual void leave(LeaveEventData* pData);
        virtual void update(UpdateEventData* pData);

        virtual EnterEvent_t* getEnterEvent();
        virtual LeaveEvent_t* getLeaveEvent();
        virtual UpdateEvent_t* getUpdateEvent();

        virtual const std::string& getName() const;
        virtual std::string getQualifiedName() const;

        virtual void setLogging(gep::ILogging* pLogging);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getEnterEvent)
            LUA_BIND_FUNCTION(getLeaveEvent)
            LUA_BIND_FUNCTION(getUpdateEvent)
            LUA_BIND_FUNCTION_NAMED(getNameCopy, "getName")
            LUA_BIND_FUNCTION(getQualifiedName)
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        gep::IAllocator* m_pAllocator;
        gep::ILogging* m_pLogging;
        NameInfo m_nameInfo;
        TransitionArray_t m_transitions;
        EnterEvent_t m_onEnter;
        LeaveEvent_t m_onLeave;
        UpdateEvent_t m_onUpdate;

        explicit State(const NameInfo& nameInfo, gep::IAllocator* pAllocator);
        virtual void addTransition(State* to, ConditionFunc_t condition = nullptr);
        std::string getNameCopy() const;
        virtual NameInfo& getNameInfo();
        virtual ConditionCheckResult checkConditions() const;
    };
}} // namespace gpp::sm

#include "gpp/stateMachines/state.inl"
