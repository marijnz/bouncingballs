#pragma once

#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateNameInfo.h"

#include "gep/interfaces/events.h"
#include "gep/container/hashmap.h"

namespace gpp { namespace sm  {

    class StateMachineFactory;

    class GPP_API StateMachine : public State
    {
        friend class StateMachineFactory;

        static const char* enterStateName();
        static const char* leaveStateName();

    public:

        virtual ~StateMachine();

        /// \brief Manually run this state machine with the default update framework.
        void run();

        /// \brief Manually run this state machine with the given update framework
        void run(gep::IUpdateFramework& updateFramework);

        /// \brief Manually run this state machine with the given update framework and event data.
        ///
        /// \remark The enter event may change the state of \a data
        void run(gep::IUpdateFramework& updateFramework, EnterEventData& data);

        /// \brief Creates a State or StateMachine
        template<typename T_State>
        T_State* create(const std::string& name);

        template<typename T_State>
        T_State* get(const std::string& name);

        void addTransition(const std::string& from, const std::string& to);
        void addTransition(const std::string& from, const std::string& to, ConditionFunc_t condition);
        void addTransition(const std::string& from, const std::string& to, gep::ScriptFunctionWrapper condition);

        // from gpp::State
        //////////////////////////////////////////////////////////////////////////

        virtual void enter(EnterEventData* pData) override;
        virtual void leave(LeaveEventData* pData) override;
        virtual void update(UpdateEventData* pData) override;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(create<State>, "createState")
            LUA_BIND_FUNCTION_NAMED(create<StateMachine>, "createStateMachine")
            LUA_BIND_FUNCTION_NAMED(get<State>, "getState")
            LUA_BIND_FUNCTION_NAMED(get<StateMachine>, "getStateMachine")
            LUA_BIND_FUNCTION_PTR(
                static_cast<void(StateMachine::*)(const std::string&, const std::string&, gep::ScriptFunctionWrapper)>(&addTransition),
                "addTransition")
            LUA_BIND_FUNCTION_PTR(static_cast<void(StateMachine::*)()>(&run), "run")

            // From gpp::State
            LUA_BIND_FUNCTION(getEnterEvent)
            LUA_BIND_FUNCTION(getLeaveEvent)
            LUA_BIND_FUNCTION(getUpdateEvent)
            LUA_BIND_FUNCTION_NAMED(getNameCopy, "getName")
            LUA_BIND_FUNCTION(getQualifiedName)
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        State m_defaultLeaveState;
        State* m_pLeaveState;
        State* m_pCurrentState;
        gep::Hashmap<std::string, State*, gep::StringHashPolicy> m_innerStates;

        explicit StateMachine(const NameInfo& nameInfo, gep::IAllocator* pAllocator);

        virtual void addTransition(State* to, ConditionFunc_t condition = nullptr) override;
        void hookIntoUpdateFramework(gep::IUpdateFramework* ufx);
        virtual ConditionCheckResult checkConditions() const override;
        virtual NameInfo& getNameInfo() override;
    };
}} // namespace gpp::sm

#include "gpp/stateMachines/stateMachine.inl"
