#pragma once

#include "gep/math3d/quaternion.h"

namespace gep {
namespace conversion {
namespace hk {

    inline void to(const Quaternion& quat, hkQuaternion& hkQuat)
    {
        hkQuat.set(
            quat.x,
            quat.y,
            quat.z,
            quat.angle);
    }

    inline hkQuaternion to(const Quaternion& quat)
    {
        return hkQuaternion(
            quat.x,
            quat.y,
            quat.z,
            quat.angle);
    }

    inline void from(const hkQuaternion& hkQuat, Quaternion& quat)
    {
        quat.x     = hkQuat.getComponent<0>().getReal();
        quat.y     = hkQuat.getComponent<1>().getReal();
        quat.z     = hkQuat.getComponent<2>().getReal();
        quat.angle = hkQuat.getComponent<3>().getReal();
    }

    inline Quaternion from(const hkQuaternion& hkQuat)
    {
        Quaternion quat(DO_NOT_INITIALIZE);
        from(hkQuat, quat);
        return quat;
    }

}}} // namespace gep::conversion::hk
