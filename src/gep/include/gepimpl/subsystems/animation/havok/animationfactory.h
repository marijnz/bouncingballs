#pragma once

#include "gep/interfaces/animation.h"
#include "gepimpl/subsystems/animation/havok/animationFactory.h"

namespace gep{

    class AnimationControl;

    class AnimationFactory : public IAnimationFactory
    {
        
        IAllocator* m_pAllocator;
    public:
        AnimationFactory(IAllocator* allocator);
        virtual ~AnimationFactory();

        virtual ResourcePtr<IAnimationResource> loadAnimation( const char* path );

        virtual void initialize();

        virtual void destroy();

        virtual IAllocator* getAllocator();

        virtual void setAllocator( IAllocator* allocator );

        IAnimatedSkeleton* loadSkeleton(IAnimationResource* skeleton);

        IAnimationResource* loadAnimationFromLua(const char* path );

        virtual IAnimatedSkeleton* createAnimatedSkeleton(ResourcePtr<IAnimationResource> skeleton);

      
        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(loadAnimationFromLua, "loadAnimation")
        LUA_BIND_REFERENCE_TYPE_END


    private:
        DynamicArray<IAnimatedSkeleton*> m_Skeletons;
        DynamicArray<AnimationControl*> m_AnimationControls;
    };

}
