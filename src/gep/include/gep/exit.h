#pragma once

#include "gep/gepmodule.h"
#include "gep/types.h"

namespace gep
{
    typedef void(*exitFunc_t)();
    Result GEP_API atexit(exitFunc_t func);
    void GEP_API destroy();
}
