#pragma once

#include "gep/interfaces/physics/characterController.h"
#include "gepimpl/subsystems/physics/havok/conversion/vector.h"

namespace gep {
namespace conversion {
namespace hk {

    inline void to(const SurfaceInfo& in_from, hkpSurfaceInfo& out_to)
    {
        out_to.m_supportedState = static_cast<hkpSurfaceInfo::SupportedState>(in_from.supportedState);
        out_to.m_surfaceDistanceExcess = in_from.surfaceDistanceExcess;
        out_to.m_surfaceIsDynamic = in_from.surfaceIsDynamic;
        conversion::hk::to(in_from.surfaceNormal, out_to.m_surfaceNormal);
        conversion::hk::to(in_from.surfaceNormal, out_to.m_surfaceNormal);
    }

    inline void from(const hkpSurfaceInfo& in_from, SurfaceInfo& out_to)
    {
        out_to.supportedState = static_cast<SurfaceInfo::SupportedState::Enum>(in_from.m_supportedState);
        out_to.surfaceDistanceExcess = in_from.m_surfaceDistanceExcess;
        out_to.surfaceIsDynamic = in_from.m_surfaceIsDynamic;
        conversion::hk::from(in_from.m_surfaceNormal, out_to.surfaceNormal);
        conversion::hk::from(in_from.m_surfaceNormal, out_to.surfaceNormal);
    }

}}} // namespace gep::conversion::hk
