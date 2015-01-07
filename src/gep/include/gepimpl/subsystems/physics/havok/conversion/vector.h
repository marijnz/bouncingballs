#pragma once

#include "gep/math3d/vec3.h"

namespace gep {
namespace conversion {
namespace hk {

    inline void to(const vec3& vec, hkVector4& hkVec)
    {
        hkVec.set(
            vec.x,
            vec.y,
            vec.z);
    }

    inline hkVector4 to(const vec3& vec)
    {
        hkVector4 result;
        result.set(vec.x, vec.y, vec.z);
        return result;
    }

    inline void from(const hkVector4& hkVec, vec3& vec)
    {
        vec.x = hkVec.getComponent<0>();
        vec.y = hkVec.getComponent<1>();
        vec.z = hkVec.getComponent<2>();
    }

    inline vec3 from(const hkVector4& vec)
    {
        return vec3(
            vec.getComponent<0>(),
            vec.getComponent<1>(),
            vec.getComponent<2>());
    }

}}} // namespace gep::conversion::hk
