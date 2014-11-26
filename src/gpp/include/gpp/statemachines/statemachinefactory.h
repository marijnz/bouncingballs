#pragma once
#include <string>

#include "gep/interfaces/scripting.h"
#include "gpp/stateMachines/stateNameInfo.h"

namespace gep
{
    class IAllocator;
}

namespace gpp { namespace sm {

    class State;
    class StateMachine;

    class GPP_API StateMachineFactory
    {
    public:
        StateMachineFactory(gep::IAllocator* pAllocator);
        ~StateMachineFactory();
        /// \brief Releases all allocated buffers.
        void initialize();
        void destroy();

        StateMachine* create(const std::string& name);
        gep::Result destroy(const std::string& name);
        gep::Result destroy(StateMachine* pInstance);

        /// \brief Gets a state machine by its fully qualified name.
        ///
        /// You can also get inner state machines using this method,
        /// as long as the top-level state machine was created with
        /// this factory instance.
        StateMachine* get(const std::string& qualifiedName);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(create)
            LUA_BIND_FUNCTION_PTR(static_cast<gep::Result(StateMachineFactory::*)(const std::string&)>(&destroy), "destroy")
            LUA_BIND_FUNCTION(get)
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        gep::IAllocator* m_pAllocator;
        std::string m_rootName;
        NameInfo m_rootNameInfo;
        gep::Hashmap<std::string, StateMachine*, gep::StringHashPolicy> m_stateMachines;

        GEP_DISALLOW_COPY_AND_ASSIGNMENT(StateMachineFactory);
    };
}} // namespace gpp::sm

#include "gpp/stateMachines/stateMachineFactory.inl"
