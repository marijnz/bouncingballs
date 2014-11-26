#include <sstream>

inline
gpp::sm::NameInfo::NameInfo(const std::string& name, NameInfo* pParent) :
    m_name(name),
    m_pParent(pParent)
{
}

inline
const std::string& gpp::sm::NameInfo::getName() const
{
    return m_name;
}

inline
std::string gpp::sm::NameInfo::getQualifiedName() const
{
    if (m_pParent == nullptr)
    {
        return m_name;
    }
    
    std::stringstream out;
    out << m_pParent->getQualifiedName() << "/" << m_name;
    return out.str();
}

inline
const gpp::sm::NameInfo* gpp::sm::NameInfo::getParent() const
{
    return m_pParent;
}
