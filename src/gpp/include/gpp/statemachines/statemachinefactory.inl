#include "gpp/stringUtils.h"

inline
gpp::sm::StateMachine* gpp::sm::StateMachineFactory::get(const std::string& qualifiedName)
{
    GEP_ASSERT(!qualifiedName.empty(), "The name must not be empty!");
    GEP_ASSERT(qualifiedName[0] == '/', "The name is not a qualified name!", qualifiedName);
    
    std::string name;
    std::string rest;

    stringSplit(qualifiedName, '/', name, rest);

    StateMachine* pInstance = m_stateMachines[name];

    while(!rest.empty())
    {
        stringSplit(rest, '/', name, rest);
        pInstance = pInstance->get<StateMachine>(name);
    }

    GEP_ASSERT(pInstance, "No state machine found with the given name.", name, qualifiedName);
    return pInstance;
}
