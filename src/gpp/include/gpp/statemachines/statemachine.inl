
inline
const char* gpp::sm::StateMachine::enterStateName()
{
    return "__enter";
}

inline
const char* gpp::sm::StateMachine::leaveStateName()
{
    return "__leave";
}


template<typename T_State>
inline
T_State* gpp::sm::StateMachine::create(const std::string& name)
{
    GEP_ASSERT(get<State>(name) == nullptr, "Attempt to add state that already exists", name);

    auto nameInfo = NameInfo(name, &m_nameInfo);
    auto state = GEP_NEW(m_pAllocator, T_State)(nameInfo, m_pAllocator);
    m_innerStates[name] = state;
    state->setLogging(m_pLogging);
    return state;
}

template<typename T_State>
inline
T_State* gpp::sm::StateMachine::get(const std::string& name)
{
    State* pState = nullptr;
    if(name == enterStateName())
    {
        pState = this;
    }
    else if(name == leaveStateName())
    {
        pState = m_pLeaveState;
    }
    else
    {
        // Returns nullptr if there is no such state.
        pState = m_innerStates[name];
    }
    GEP_ASSERT(pState == nullptr || dynamic_cast<T_State*>(pState) != nullptr,
               "Requested state is not a state machine!", name);

    return static_cast<T_State*>(pState);
}

inline
void gpp::sm::StateMachine::addTransition(const std::string& from, const std::string& to)
{
    addTransition(from, to, nullptr);
}
