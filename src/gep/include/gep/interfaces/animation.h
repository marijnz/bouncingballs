#pragma once
#include "gep/interfaces/subsystem.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/container/DynamicArray.h"

#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/transform.h"

namespace gep
{
    // Forward declarations (declared somewhere else)
    //////////////////////////////////////////////////////////////////////////

    // Forward declarations (declared later in this file)
    //////////////////////////////////////////////////////////////////////////
    class IAnimationFactory;
    class IAnimationResource;
    class IAnimatedSkeleton;
    struct BoneTransform;

    // Class definitions
    //////////////////////////////////////////////////////////////////////////

    class IAnimationSystem : public ISubsystem
    {
    public:
        virtual ~IAnimationSystem() = 0 {}

        virtual IAnimationFactory* getAnimationFactory() const = 0;
    };

    class IAnimationFactory
    {
    public:
        virtual ~IAnimationFactory() = 0 {}

        virtual void initialize() = 0;
        virtual void destroy() = 0;

        virtual IAllocator* getAllocator() = 0;
        virtual void setAllocator(IAllocator* allocator) = 0;
        virtual ResourcePtr<IAnimationResource> loadAnimation(const char* path) = 0;
        virtual IAnimationResource* loadAnimationFromLua( const char* path ) = 0;
        virtual IAnimatedSkeleton* createAnimatedSkeleton(ResourcePtr<IAnimationResource> skeleton) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(loadAnimationFromLua, "loadAnimation")
        LUA_BIND_REFERENCE_TYPE_END
    };

    class ITransform;
    class IAnimationResource : public IResource
    {
    public:
        virtual ~IAnimationResource() = 0 {}
    };

    class IAnimationControl
    {
    public:
        virtual ~IAnimationControl() = 0 {};
        virtual void setMasterWeight(const float weight) = 0;
        virtual float getMasterWeight() =0;
        virtual void setPlaybackSpeed(const float speed) = 0;
        virtual float getAnimationDuration() = 0;
        virtual float getLocalTime() = 0;
        virtual void setLocalTime(const float time) =0;
        virtual float ease(const float duration, const bool easeInIfTrue) =0;
    };

    class IBone : public Transform
    {
    public:
        virtual ~IBone() = 0 {};

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(getWorldPosition)
            LUA_BIND_FUNCTION(getWorldRotation)
            LUA_BIND_FUNCTION(getViewDirection)
            LUA_BIND_FUNCTION(getRightDirection)
            LUA_BIND_FUNCTION(getUpDirection)
            LUA_BIND_FUNCTION(setParent)
        LUA_BIND_REFERENCE_TYPE_END;
    };


    class IAnimatedSkeleton
    {
    public:
        virtual  ~IAnimatedSkeleton() = 0 {};
        virtual void update( float elapsedMS ) = 0;
        virtual IAnimationControl* addAnimationControl(ResourcePtr<IAnimationResource> anim) = 0;
        virtual gep::uint32 findBoneForName(const char* name) = 0;
        virtual void getBoneTransformations(DynamicArray<mat4>& result) = 0;
        virtual void setBoneDebugDrawingEnabled(const bool enabled) = 0;
        virtual void setParentTransform(ITransform* parent) =0;
        virtual void setReferencePoseWeightThreshold(const float threshold) = 0;
        virtual void setBoneDebugDrawingScale(const float scale) = 0;
        virtual IBone* getBoneByName(std:: string name) = 0;
    };

}
