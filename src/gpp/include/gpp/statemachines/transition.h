#pragma once
#include "gep/container/DynamicArray.h"

namespace gpp { namespace sm {

    class State;

    typedef std::function<bool()> ConditionFunc_t;

    struct Transition
    {
        State* to;
        ConditionFunc_t condition;
    };
    typedef gep::DynamicArray<Transition> TransitionArray_t;

}} // namespace gpp::sm
