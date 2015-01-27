#pragma once

#include "gpp/gameObjectSystem.h"
#include "gep/interfaces/updateFramework.h"

#include "gep/interfaces/animation.h"


namespace gep
{
    class IModel;
    class IRendererExtractor;
}


namespace gpp
{
    class RenderComponent : public Component
    {
    public:
        RenderComponent();
        virtual ~RenderComponent();

        //make sure the path is set before initializing this component!
        virtual void initalize();

        virtual void update(float elapsedMS);

        virtual void destroy();

        void extract(gep::IRendererExtractor& extractor);

        inline void setPath(const std::string& path){ m_path = path; }
        inline const std::string& getPath() const { return m_path; }
        inline std::string getPathCopy() const { return m_path; }
        void getBoneNames(gep::DynamicArray<const char*>& names);
        void setBoneMapping(const gep::DynamicArray<gep::uint32>& boneIds);
        void applyBoneTransformations(const gep::ArrayPtr<gep::mat4>& transformations);

        virtual void setState(State::Enum state) override;

        gep::vec3 getScale() { return m_scale; };
        void setScale(const gep::vec3& scale) { m_scale = scale; };

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setPath)
            LUA_BIND_FUNCTION_NAMED(getPathCopy, "getPath")
            LUA_BIND_FUNCTION(setState)
            LUA_BIND_FUNCTION(setScale)
            LUA_BIND_FUNCTION(getScale)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        // TODO: No raw pointers to ressources
        gep::IModel* m_pModel;
        gep::vec3 m_scale;
        gep::DynamicArray<gep::uint32> m_boneMapping;

        std::string m_path;
        gep::CallbackId m_extractionCallbackId;
        gep::DynamicArray<gep::mat4> m_bones;


    };

    template<>
    struct ComponentMetaInfo<RenderComponent>
    {
        static const char* name(){ return "RenderComponent"; }
        static const gep::int32 initializationPriority() { return -10; }
        static const gep::int32 updatePriority() { return std::numeric_limits<gep::int32>::max(); }
    };
}
