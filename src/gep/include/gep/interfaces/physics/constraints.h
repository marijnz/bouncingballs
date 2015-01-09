#pragma once

namespace gep
{
    struct ConstraintType
    {
        enum Enum
        {
            INVALID = -1,

            BallAndSocket,
            Hinge,
            PointToPlane,
            Prismatic,

            COUNT
        };
    };

    struct Constraint
    {
        void* pData;

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER_NAMED(pData, "_data")
        LUA_BIND_VALUE_TYPE_END
    };
}
