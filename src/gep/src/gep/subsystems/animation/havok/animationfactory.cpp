#include "stdafx.h"
#include "gepimpl/subsystems/animation/havok/animationFactory.h"
#include "gepimpl/subsystems/animation/havok/animation.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/animation.h"

#include "gep/globalManager.h"



gep::AnimationFactory::AnimationFactory(IAllocator* allocator):
    m_pAllocator(allocator),
    m_Skeletons(),
    m_AnimationControls()
{
   GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
}

gep::AnimationFactory::~AnimationFactory()
{
}

gep::ResourcePtr<gep::IAnimationResource> gep::AnimationFactory::loadAnimation( const char* path )
{
    return g_globalManager.getResourceManager()->loadResource<IAnimationResource>(AnimationFileLoader(path), LoadAsync::No);
}

void gep::AnimationFactory::initialize()
{
     g_globalManager.getResourceManager()->registerResourceType("Animation", nullptr);
}

void gep::AnimationFactory::destroy()
{
    for(auto& skeleton : m_Skeletons)
    {
        DELETE_AND_NULL(skeleton);
    }

    for(auto& animControl : m_AnimationControls)
    {
        DELETE_AND_NULL(animControl);
    }
}

gep::IAllocator* gep::AnimationFactory::getAllocator()
{
    return m_pAllocator;
}

void gep::AnimationFactory::setAllocator( IAllocator* allocator )
{
    m_pAllocator = allocator;
}

gep::IAnimationResource* gep::AnimationFactory::loadAnimationFromLua( const char* path )
{
    return loadAnimation(path).get();
}

gep::IAnimatedSkeleton* gep::AnimationFactory::createAnimatedSkeleton(ResourcePtr<IAnimationResource> skeleton)
{
    GEP_ASSERT(dynamic_cast<AnimationResource*>(skeleton.get()) != nullptr, "Cant't cast provided skeleton parameter to AnimationResource")
    auto skel = static_cast<AnimationResource*>(skeleton.get());
    auto result = new AnimatedSkeleton(skel);
    m_Skeletons.append(result);

    return result;
}

