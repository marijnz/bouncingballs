#pragma once

namespace gep { namespace hk {

    template<typename T>
    inline void removeRefAndNull(T*& havokObject)
    {
        // Make sure we are holding a havok referenced object.
        static_cast<hkReferencedObject*>(havokObject);
        if(havokObject)
        {
            havokObject->removeReference();
            havokObject = nullptr;
        }
    }

}} // namespace gep::hk

#define GEP_HK_REMOVE_REF_AND_NULL(instance) ::gep::hk::removeRefAndNull(instance)
