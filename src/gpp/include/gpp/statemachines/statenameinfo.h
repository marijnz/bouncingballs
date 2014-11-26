#pragma once
#include <string>

namespace gpp { namespace sm {

    struct NameInfo
    {
        NameInfo(const std::string& name, NameInfo* pParent);

        const std::string& getName() const;

        /// \brief gets the fully qualified name.
        std::string getQualifiedName() const;

        const NameInfo* getParent() const;

    private:
        /// \brief Short name.
        const std::string m_name;

        /// \brief can be used to get the parents' (qualified) name.
        NameInfo* m_pParent;
    };

}} // namespace gpp::sm

#include "gpp/stateMachines/stateNameInfo.inl"
