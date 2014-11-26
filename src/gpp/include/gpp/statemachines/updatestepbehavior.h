#pragma once

namespace gpp { namespace sm {

    struct UpdateStepBehavior
    {
        enum Enum
        {
            Continue = 0,
            Leave,
            LeaveWithNoConditionChecks,
        };

        GEP_DISALLOW_CONSTRUCTION(UpdateStepBehavior);
    };

}} // namespace gpp::sm
