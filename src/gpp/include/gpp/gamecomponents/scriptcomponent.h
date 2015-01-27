#pragma once

#include "gpp/gameObjectSystem.h"

namespace gpp
{
    class ScriptComponent : public Component
    {
    public:
        ScriptComponent();
        virtual ~ScriptComponent();

        virtual void initalize();
        virtual void destroy();
        virtual void update(float elapsedMS);

        void setInitializationFunction(gep::ScriptFunctionWrapper funcRef);
        void setDestroyFunction(gep::ScriptFunctionWrapper funcRef);
        void setUpdateFunction(gep::ScriptFunctionWrapper funcRef);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setInitializationFunction)
            LUA_BIND_FUNCTION(setDestroyFunction)
            LUA_BIND_FUNCTION(setUpdateFunction)
            LUA_BIND_FUNCTION(setState)
        LUA_BIND_REFERENCE_TYPE_END

    private:

        gep::ScriptFunctionWrapper m_funcRef_initialize;
        gep::ScriptFunctionWrapper m_funcRef_destroy;
        gep::ScriptFunctionWrapper m_funcRef_update;
    };

    template<>
    struct ComponentMetaInfo<ScriptComponent>
    {
        static const char* name(){ return "ScriptComponent"; }
        static const gep::int32 initializationPriority() { return 0; }
        static const gep::int32 updatePriority() { return 42; }
    };
}
