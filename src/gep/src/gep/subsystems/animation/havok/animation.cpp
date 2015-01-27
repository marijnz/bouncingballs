#include "stdafx.h"
#include "gepimpl/subsystems/animation/havok/animation.h"

#include "gep/globalManager.h"


#include "gepimpl/subsystems/animation/havok/animationFactory.h"

#include "gepimpl/subsystems/renderer/renderer.h"
#include "gepimpl/subsystems/resourceManager.h"
#include "gepimpl/subsystems/logging.h"

#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/conversion/quaternion.h"

#include "gep/math3d/quaternion.h"
#include "gep/math3d/transform.h"
#include "gepimpl/havok/util.h"




gep::AnimationResource::AnimationResource():
    m_animations(),
    m_bindings(),
    m_skeletons(),
    m_skinBinding()
{

}

gep::AnimationResource::~AnimationResource()
{

}


gep::IResourceLoader* gep::AnimationResource::getLoader()
{
    return m_pFileLoader;
}

void gep::AnimationResource::setLoader( IResourceLoader* loader )
{
    GEP_ASSERT(dynamic_cast<AnimationFileLoader*>(loader) != nullptr, "Cannot cast Resource loader! Wrong ResourceLoader Type!");
    m_pFileLoader = static_cast<AnimationFileLoader*>(loader);

}

void gep::AnimationResource::unload()
{
    m_skeletons.clear();
    m_animations.clear();
    m_bindings.clear();
    m_skinBinding.clear();
}

void gep::AnimationResource::finalize()
{
    throw std::exception("The method or operation is not implemented.");
}

gep::uint32 gep::AnimationResource::getFinalizeOptions()
{
    return ResourceFinalize::NotRequired;
}

bool gep::AnimationResource::isLoaded()
{
    throw std::exception("The method or operation is not implemented.");
}

gep::IResource* gep::AnimationResource::getSuperResource()
{
    throw std::exception("The method or operation is not implemented.");
}

const char* gep::AnimationResource::getResourceType()
{
    throw std::exception("The method or operation is not implemented.");
}

//////////////////////////////////////////////////////////////////////////


gep::IAnimationFactory* gep::AnimationSystem::getAnimationFactory() const
{
    return m_pAnimationFactory;
}

void gep::AnimationSystem::initialize()
{
    m_pAnimationFactory = new AnimationFactory(&g_stdAllocator);
    m_pAnimationFactory->initialize();

}

void gep::AnimationSystem::destroy()
{
    m_pAnimationFactory->destroy();
}

void gep::AnimationSystem::update(float elapsedSeconds)
{
    throw std::exception("The method or operation is not implemented.");
}

gep::AnimationSystem::~AnimationSystem()
{
    DELETE_AND_NULL(m_pAnimationFactory);
}

//////////////////////////////////////////////////////////////////////////

gep::AnimationFileLoader::AnimationFileLoader( const char* path ) :
    m_path(path)
{

}

gep::AnimationFileLoader::~AnimationFileLoader()
{

}

gep::IResource* gep::AnimationFileLoader::loadResource( IResource* pInPlace )
{
    return loadResource(dynamic_cast<AnimationResource*>(pInPlace));
}

gep::AnimationResource* gep::AnimationFileLoader::loadResource( AnimationResource* pInPlace )
{
    AnimationResource* result = nullptr;
    bool isInPlace = true;
    if (pInPlace == nullptr)
    {
        result = new AnimationResource();
        isInPlace = false;
    }
    auto* havokLoader = g_resourceManager.getHavokResourceLoader();
    auto* container = havokLoader->load(m_path.c_str());
    GEP_ASSERT(container != nullptr, "Could not load asset! %s", m_path.c_str());

    if (container)
    {
        hkaAnimationContainer* ac = reinterpret_cast<hkaAnimationContainer*>( container->findObjectByType( hkaAnimationContainerClass.getName() ));

        GEP_ASSERT(ac != nullptr, "Unable to load animation data!");

        // get the skeleton
        if (ac->m_skeletons.getSize() > 0)
        {
            for (auto skel : ac->m_skeletons)
            {
                result->m_skeletons.append(skel);
            }
        }
        else
        {
            g_globalManager.getLogging()->logMessage("No skeleton loaded from file %s",m_path.c_str());
        }

        // animation and binding
        if (ac->m_animations.getSize() > 0)
        {
            for (auto anim : ac->m_animations)
            {
                result->m_animations.append(anim);
            }
        }
        else
        {
            g_globalManager.getLogging()->logMessage("No animation loaded from file %s",m_path.c_str());
        }

        if (ac->m_bindings.getSize() > 0)
        {
            for (auto binding : ac->m_bindings)
            {
                result->m_bindings.append(binding);
            }
        }
        else
        {
            g_globalManager.getLogging()->logMessage("No binding loaded from file %s",m_path.c_str());
        }

        //skinning

        if(ac->m_skins.getSize() > 0)
        {
            for (auto binding : ac->m_skins)
            {
                result->m_skinBinding.append(binding);
            }
        }
        else
        {
             g_globalManager.getLogging()->logMessage("No skin binding loaded from file %s",m_path.c_str());
        }


    }
    return result;
}

void gep::AnimationFileLoader::postLoad( ResourcePtr<IResource> pResource )
{

}

void gep::AnimationFileLoader::deleteResource( IResource* pResource )
{
    auto* actualResource = dynamic_cast<IAnimationResource*>(pResource);
    GEP_ASSERT(actualResource != nullptr, "wrong type of resource to delete with this loader!");
    delete actualResource;
}

const char* gep::AnimationFileLoader::getResourceType()
{
    return "Animation";
}

const char* gep::AnimationFileLoader::getResourceId()
{
    return m_path.c_str();
}

gep::IResourceLoader* gep::AnimationFileLoader::moveToHeap()
{
    return new AnimationFileLoader(*this);
}

void gep::AnimationFileLoader::release()
{
    delete this;
}

//////////////////////////////////////////////////////////////////////////

gep::AnimatedSkeleton::AnimatedSkeleton(AnimationResource* skeleton) :
    m_drawDebug(false),
    m_debugDrawingScale(1.0f)
{
    m_pHkaAnimatedSkeleton = new hkaAnimatedSkeleton( skeleton->m_skeletons[0] );
    //m_skeletonInstance->setReferencePoseWeightThreshold( 0.5f );
    m_pPose = new hkaPose(m_pHkaAnimatedSkeleton->getSkeleton());
    m_pPose->setToReferencePose();

}

gep::AnimatedSkeleton::~AnimatedSkeleton()
{
    for (auto& control : m_animationControls)
    {
        DELETE_AND_NULL(control);
    }
   GEP_HK_REMOVE_REF_AND_NULL(m_pHkaAnimatedSkeleton);

   for (auto& bone : m_bones.values())
   {
       DELETE_AND_NULL(bone);
   }

   DELETE_AND_NULL(m_pPose);

}

gep::IAnimationControl* gep::AnimatedSkeleton::addAnimationControl(ResourcePtr<IAnimationResource> anim)
{
    GEP_ASSERT(dynamic_cast<AnimationResource*>(anim.get()) != nullptr, "Cant't cast provided animationControl parameter")
    auto resource = static_cast<AnimationResource*>(anim.get());

    auto control = new AnimationControl( resource->m_bindings[0] ); // only one animation per file!
    m_animationControls.append(control);
    m_pHkaAnimatedSkeleton->addAnimationControl(control->getHkaAnimationControl());

    return control;
}


void gep::AnimatedSkeleton::update(float elapsedSeconds)
{
    m_pHkaAnimatedSkeleton->stepDeltaTime(elapsedSeconds);
    m_pHkaAnimatedSkeleton->sampleAndCombineAnimations(m_pPose->accessSyncedPoseLocalSpace().begin(), m_pPose->getFloatSlotValues().begin());

    for (auto& bone : m_bones.values())
    {
        bone->update(elapsedSeconds);
    }

    if(m_drawDebug)
    {
        renderDebug();
    }
}

gep::uint32 gep::AnimatedSkeleton::findBoneForName(const char* name)
{
    auto skeleton = m_pHkaAnimatedSkeleton->getSkeleton();
    return hkaSkeletonUtils::findBoneWithName(*skeleton, name);
}

void gep::AnimatedSkeleton::getBoneTransformations(DynamicArray<mat4>& result)
{
    int index = 0;
    for(auto& bone : m_pPose->getSyncedPoseModelSpace())
    {
        hkQsTransform transform = m_pPose->getBoneModelSpace(index);
        //hkQsTransform transform = m_pPose->getBoneLocalSpace(index);

        __declspec(align(16)) mat4 trans; //Hack to convert hkMat4 to gep::mat4


        /* If you need the transform's components, use:
        vec3 trans = gep::conversion::hk::from(transform.getTranslation());
        Quaternion rot = gep::conversion::hk::from(transform.getRotation());
        mat4 ret = mat4::translationMatrix(trans);
        ret.setRotationPart(rot.toMat3());*/

        transform.get4x4ColumnMajor(trans.data);
        //result.append(ret);

        result.append(trans);
        index++;
    }
}

void gep::AnimatedSkeleton::renderDebug()
{
    auto renderer = g_globalManager.getRenderer();
    auto skeleton = m_pHkaAnimatedSkeleton->getSkeleton();
    const hkInt16* parents = skeleton->m_parentIndices.begin();
    const hkQsTransform* bones = m_pPose->getSyncedPoseModelSpace().begin();
    vec3 p1,p2;
    for (int i=0; i<skeleton->m_bones.getSize(); i++)
    {
        hkVector4 hp1, hp2;
        hp1 = bones[i].getTranslation();
        if (parents[i] == -1)
        {
            hp2 = hp1;
        }
        else
        {
            hp2 = bones[parents[i]].getTranslation();
        }

       conversion::hk::from(hp1,p1);
       conversion::hk::from(hp2,p2);
       p1 = p1 * m_debugDrawingScale;
       p2 = p2 * m_debugDrawingScale;

       p1 = m_pTransform->getWorldRotation().toMat3() * p1;
       p1 = m_pTransform->getWorldPosition() + p1;

       p2 = m_pTransform->getWorldRotation().toMat3() * p2;
       p2 = m_pTransform->getWorldPosition() + p2;

       if (gep::isNonZero((p1 - p2).length() ))
       {
           renderer->getDebugRenderer().drawArrow(p1, p2);
       }
    }
}


gep::IBone* gep::AnimatedSkeleton::getBoneByName(std:: string name)
{
    if(m_bones.exists(name))
    {
        return m_bones[name];
    }
    else
    {
        int index = hkaSkeletonUtils::findBoneWithName(* m_pHkaAnimatedSkeleton->getSkeleton(), name.c_str());
        auto bone = new Bone(index,m_pPose);
        m_bones[name] = bone;
        return bone;
    }
}


//////////////////////////////////////////////////////////////////////////



gep::AnimationControl::AnimationControl(hkRefPtr<hkaAnimationBinding> binding)
{
    m_pHkaDefaultAnimationControl = new hkaDefaultAnimationControl( binding );
    m_pHkaDefaultAnimationControl->setMasterWeight(1.0f);
    m_pHkaDefaultAnimationControl->setPlaybackSpeed(1.0f);
}

gep::AnimationControl::~AnimationControl()
{
    GEP_HK_REMOVE_REF_AND_NULL(m_pHkaDefaultAnimationControl);
}

void gep::AnimationControl::setMasterWeight(const float weight)
{
    m_pHkaDefaultAnimationControl->setMasterWeight(weight);
}

void gep::AnimationControl::setPlaybackSpeed(const float speed)
{
    m_pHkaDefaultAnimationControl->setPlaybackSpeed(speed);
}

hkaDefaultAnimationControl* gep::AnimationControl::getHkaAnimationControl()
{
    return m_pHkaDefaultAnimationControl;
}

//////////////////////////////////////////////////////////////////////////

gep::Bone::Bone(const int boneIndex, const hkaPose* pose) :
    m_pPose(pose),
    m_boneID(boneIndex)
{
    m_pBone = &m_pPose->getBoneModelSpace(m_boneID);
}

gep::Bone::~Bone()
{

}

void gep::Bone::setPosition(const gep::vec3& pos)
{
    GEP_ASSERT(false, "Do not set the position of a bone!")
}

void gep::Bone::setRotation(const gep::Quaternion& rot)
{
    GEP_ASSERT(false, "Do not set the rotation of a bone!")
}

gep::mat4 gep::Bone::getTransformationMatrix() const
{
      return gep::mat4::translationMatrix( conversion::hk::from(m_pBone->getTranslation())) * (conversion::hk::from(m_pBone->getRotation())).toMat4();
}

gep::vec3 gep::Bone::getPosition() const
{
    return conversion::hk::from(m_pBone->getTranslation());
}

gep::Quaternion gep::Bone::getRotation() const
{
   return conversion::hk::from(m_pBone->getRotation());
}

gep::vec3 gep::Bone::getViewDirection() const
{
    return (conversion::hk::from(m_pBone->getRotation()) * m_baseOrientation).toMat3() * gep::vec3(0,1,0);
}

gep::vec3 gep::Bone::getUpDirection() const
{
    return (conversion::hk::from(m_pBone->getRotation()) * m_baseOrientation).toMat3() * gep::vec3(0,0,1);
}

gep::vec3 gep::Bone::getRightDirection() const
{
    return (conversion::hk::from(m_pBone->getRotation()) * m_baseOrientation).toMat3() * gep::vec3(1,0,0);
}

gep::mat4 gep::Bone::getWorldTransformationMatrix() const
{
    if (m_pParent)
    {
        return  m_pParent->getWorldTransformationMatrix() * gep::mat4::translationMatrix( conversion::hk::from(m_pBone->getTranslation())) * (conversion::hk::from(m_pBone->getRotation())) .toMat4();
    }
    return gep::mat4::translationMatrix( conversion::hk::from(m_pBone->getTranslation())) * (conversion::hk::from(m_pBone->getRotation())) .toMat4();
}

gep::vec3 gep::Bone::getWorldPosition() const
{
    if (m_pParent)
    {
        return m_pParent->getWorldPosition() + conversion::hk::from(m_pBone->getTranslation());
    }
    return conversion::hk::from(m_pBone->getTranslation());
}

gep::Quaternion gep::Bone::getWorldRotation() const
{
    if (m_pParent)
    {
         return  m_pParent->getWorldRotation() * conversion::hk::from(m_pBone->getRotation());
    }
   return conversion::hk::from(m_pBone->getRotation());
}

void gep::Bone::update(float elapsedSeconds)
{
    m_pBone = &m_pPose->getBoneModelSpace(m_boneID);
}
