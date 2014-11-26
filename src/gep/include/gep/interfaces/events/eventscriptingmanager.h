#pragma once
#include "gep/interfaces/scripting.h"

namespace gep
{
    class EventScriptingManager : public IScriptingManager
    {
    public:
        GEP_API static IScriptingManager& instance();
        GEP_API static void patchInstance(IScriptingManager* pInstance);

    protected:
        static IScriptingManager* s_pInstance;
    };
}
