#pragma once

#include "gep/math3d/vec3.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    class IRigidBody;

    struct RayCastInput
    {
        vec3 from;
        vec3 to;
        uint32 filterInfo;

        RayCastInput() :
            from(0.0f),
            to(0.0f),
            filterInfo(0U)
        {
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(from)
            LUA_BIND_MEMBER(to)
            LUA_BIND_MEMBER(filterInfo)
        LUA_BIND_VALUE_TYPE_END
    };

    struct RayCastOutput
    {
        float hitFraction;
        vec3 normal;
        IRigidBody* pHitBody;

        RayCastOutput() :
            hitFraction(1.0f),
            normal(0.0f),
            pHitBody(nullptr)
        {
        }

        inline bool hasHit() { return pHitBody != nullptr; }

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(hasHit)
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(hitFraction)
            LUA_BIND_MEMBER(normal)
            LUA_BIND_MEMBER_NAMED(pHitBody, "hitBody")
        LUA_BIND_VALUE_TYPE_END
    };
}
