
inline
std::string gpp::sm::State::getNameCopy() const
{
    return m_nameInfo.getName();
}

//////////////////////////////////////////////////////////////////////////

inline
bool gpp::sm::State::ConditionCheckResult::hasNextState()
{
    return nextState != nullptr;
}
