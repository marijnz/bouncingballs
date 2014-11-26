#pragma once

#include "gep/gepmodule.h"

namespace gep
{
    class GEP_API ISubsystem
    {
    public:
        virtual ~ISubsystem(){}
        virtual void initialize() = 0;
        virtual void destroy() = 0;
        virtual void update(float elapsedTime) {};
    };
}
