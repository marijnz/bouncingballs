#pragma once

#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/conversion/quaternion.h"
#include "gep/math3d/transform.h"

namespace gep {
namespace conversion {
namespace hk {

    
    inline void from(const hkTransform& hkTrans, vec3& translation, Quaternion& rotation)
    {
        //hkTransform trans = hkTrans; //HACK for fixing error when fromVector4 is called directly on hkTrans. TODO: Fix!
        from(hkTrans.getTranslation(),translation);
        from(hkQuaternion(hkTrans.getRotation()), rotation);
    }
    inline void from(const hkTransform& hkTrans, Transform& transform)
    {
        vec3 translation = from(hkTrans.getTranslation());
        Quaternion rotation = from(hkQuaternion(hkTrans.getRotation()));
        transform.setPosition(translation);
        transform.setRotation(rotation);
    }

}}} // namespace gep::conversion::hk
