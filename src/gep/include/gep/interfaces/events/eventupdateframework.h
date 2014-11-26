#pragma once
#include "gep/interfaces/updateFramework.h"

namespace gep
{
    class EventUpdateFramework : public IUpdateFramework
    {
    public:
        GEP_API static IUpdateFramework& instance();
        GEP_API static void patchInstance(IUpdateFramework* pInstance);

    private:
        static IUpdateFramework* s_pInstance;
    };
}
