#pragma once
#include "gpp/gameObjectSystem.h"
#include "gep/interfaces/animation.h"
#include "gep/container/hashmap.h"


namespace gpp
{
   // class gep::IAnimationControl;


    class AnimationComponent : public Component
    {
    public:
        AnimationComponent();
        virtual ~AnimationComponent();


        virtual void initalize() override;

        virtual void update( float elapsedMS ) override;

        virtual void destroy() override;

        void setSkeletonFile(const char* path);
        void addAnimationFile(const char* animationName, const char* path);
        void setSkinFile(const char* path);

        void setMasterWeight(const std::string& animName, float weight );
        float getMasterWeight(const std::string& animName);
        void setPlaybackSpeed(const std::string& animName, float speed);
        void setBoneDebugDrawingEnabled(const bool enabled);
        void setReferencePoseWeightThreshold(const float threshold);
        float getAnimationDuration(const std::string& animName);

        void setDebugDrawingScale(const float scale);
        float getLocalTime(const std::string& animName);
        float getLocalTimeNormalized(const std::string& animName);
        void setLocalTime(const std::string& animName, const float time);
        void setLocalTimeNormalized(const std::string& animName, const float time);
        gep::IBone* getBoneByName(const std::string& boneName);

        //\returns The time at which it will be fully eased in
        float easeIn(const std::string& animName, const float duration);
         //\returns The time at which it will be fully eased out
        float easeOut(const std::string& animName, const float duration);

        void getBoneMapping(gep::ArrayPtr<const char*> boneNames, gep::DynamicArray<gep::uint32>& boneIds);


        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setSkeletonFile)
            LUA_BIND_FUNCTION(addAnimationFile)
            LUA_BIND_FUNCTION(setSkinFile)
            LUA_BIND_FUNCTION(setBoneDebugDrawingEnabled)
            LUA_BIND_FUNCTION(setMasterWeight)
            LUA_BIND_FUNCTION(getMasterWeight)
            LUA_BIND_FUNCTION(setPlaybackSpeed)
            LUA_BIND_FUNCTION(setReferencePoseWeightThreshold)
            LUA_BIND_FUNCTION(getAnimationDuration)
            LUA_BIND_FUNCTION(setDebugDrawingScale)
            LUA_BIND_FUNCTION(getLocalTime)
            LUA_BIND_FUNCTION(getLocalTimeNormalized)
            LUA_BIND_FUNCTION(setLocalTime)
            LUA_BIND_FUNCTION(setLocalTimeNormalized)
            LUA_BIND_FUNCTION(easeIn)
            LUA_BIND_FUNCTION(easeOut)
            LUA_BIND_FUNCTION(getBoneByName)
            LUA_BIND_FUNCTION(setState)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        void updateBoneTransformations();

        const char* m_skeletonPath;
        gep::Hashmap<std::string, std::string, gep::StringHashPolicy> m_animationPaths;
        const char* m_skinPath;

        gep::DynamicArray<gep::mat4> m_boneTransformations;

        gep::ResourcePtr<gep::IAnimationResource> m_skeleton;
        gep::Hashmap<std::string, gep::ResourcePtr<gep::IAnimationResource>, gep::StringHashPolicy> m_animations;
        gep::Hashmap<std::string, gep::IAnimationControl*, gep::StringHashPolicy> m_animationControls;
        gep::ResourcePtr<gep::IAnimationResource> m_skins;



        gep::IAnimatedSkeleton*  m_AnimatedSkeleton;

        bool m_renderBoneDebugDrawing;
        float m_boneDebugDrawingScale;
    };

    template<>
    struct ComponentMetaInfo<AnimationComponent>
    {
        static const char* name(){ return "AnimationComponent"; }
        static const gep::int32 initializationPriority() { return 23; }
        static const gep::int32 updatePriority() { return 1; }
    };

}
