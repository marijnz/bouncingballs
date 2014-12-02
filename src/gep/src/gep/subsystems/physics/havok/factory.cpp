#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/factory.h"
#include "gepimpl/subsystems/physics/havok/conversion/transformation.h"
#include "gep/memory/allocator.h"
#include "gepimpl/subsystems/havok.h"
#include "gep/interfaces/MemoryManager.h"


gep::CollisionMesh::CollisionMesh() :
    m_pShape(nullptr),
    m_pTransform(nullptr)
{
}

gep::CollisionMesh::~CollisionMesh()
{
}

void gep::CollisionMesh::setShape(IShape* shape)
{
    m_pShape = shape;
}

void gep::CollisionMesh::setTransform(Transform* transform)
{
    m_pTransform = transform;
}

gep::IShape* gep::CollisionMesh::getShape()
{
    return m_pShape;
}

const gep::IShape* gep::CollisionMesh::getShape() const
{
    return m_pShape;
}

const char* gep::CollisionMesh::getResourceType()
{
    throw std::logic_error("The method or operation is not implemented.");
}

gep::IResource* gep::CollisionMesh::getSuperResource()
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool gep::CollisionMesh::isLoaded()
{
    throw std::logic_error("The method or operation is not implemented.");
}

gep::uint32 gep::CollisionMesh::getFinalizeOptions()
{
    return ResourceFinalize::NotRequired;
}

void gep::CollisionMesh::finalize()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void gep::CollisionMesh::unload()
{
    DELETE_AND_NULL(m_pShape);
}

void gep::CollisionMesh::setLoader(IResourceLoader* loader)
{
    GEP_ASSERT(loader != nullptr);
#ifdef _DEBUG
    m_pCollisionMeshFileLoader = dynamic_cast<CollisionMeshFileLoader*>(loader);
    GEP_ASSERT(m_pCollisionMeshFileLoader != nullptr);
#else
    m_pCollisionMeshFileLoader = static_cast<CollisionMeshFileLoader*>(loader);
#endif
}

gep::IResourceLoader* gep::CollisionMesh::getLoader()
{
    return m_pCollisionMeshFileLoader;
}

//////////////////////////////////////////////////////////////////////////

gep::CollisionMeshFileLoader::CollisionMeshFileLoader(const char* path) : m_path(path)
{

}

gep::CollisionMeshFileLoader::~CollisionMeshFileLoader()
{

}

void gep::CollisionMeshFileLoader::release()
{
    delete this;
}

gep::IResourceLoader* gep::CollisionMeshFileLoader::moveToHeap()
{
    return new CollisionMeshFileLoader(*this);
}

const char* gep::CollisionMeshFileLoader::getResourceId()
{
    return m_path.c_str();
}

const char* gep::CollisionMeshFileLoader::getResourceType()
{
    return "CollisionMesh";
}

void gep::CollisionMeshFileLoader::deleteResource(IResource* pResource)
{
    auto* actualResource = dynamic_cast<ICollisionMesh*>(pResource);
    GEP_ASSERT(actualResource != nullptr, "wrong type of resource to delete with this loader!");
    delete actualResource;
}

void gep::CollisionMeshFileLoader::postLoad(ResourcePtr<IResource> pResource)
{

}

gep::CollisionMesh* gep::CollisionMeshFileLoader::loadResource(CollisionMesh* pInPlace)
{
    CollisionMesh* result = nullptr;
    bool isInPlace = true;
    if (pInPlace == nullptr)
    {
        result = new CollisionMesh();
        isInPlace = false;
    }
    auto* havokLoader = g_resourceManager.getHavokResourceLoader();
    auto* container = havokLoader->load(m_path.c_str());
    GEP_ASSERT(container != nullptr, "Could not load asset! %s", m_path.c_str());

    if (container)
    {
        auto* physicsData = reinterpret_cast<hkpPhysicsData*>(container->findObjectByType(hkpPhysicsDataClass.getName()));
        GEP_ASSERT(physicsData != nullptr, "Unable to load physics data!");

        if (physicsData)
        {
            const auto& physicsSystems = physicsData->getPhysicsSystems();
            GEP_ASSERT(physicsSystems.getSize() == 1, "Wrong number of physics systems!");
            auto* body = physicsSystems[0]->getRigidBodies()[0];
            const auto* hkShape = body->getCollidable()->getShape();

            auto shape = conversion::hk::from(const_cast<hkpShape*>(hkShape));

            auto type = shape->getShapeType();
            if ( type == hkcdShapeType::TRIANGLE || 
                 type == hkcdShapeType::BV_COMPRESSED_MESH ||
                 type == hkcdShapeType::CONVEX_VERTICES )
            {
                auto transform = body->getTransform();
                auto meshShape = static_cast<HavokMeshShape*>(shape);
                Transform* tempTrans = new Transform();
                conversion::hk::from(transform, *tempTrans);
                
                // Since havok content tools are buggy (?) and no custom transformation can be applied,
                // we have to convert into our engine's space by hand.
                // TODO: Ensure, that this transformation is correct in every case
                tempTrans->setRotation(tempTrans->getRotation() * Quaternion(vec3(1,0,0),180));
                meshShape->setTransform(tempTrans);
            }


            result->setShape(shape);
            
        }

    }
    return result;
}

gep::IResource* gep::CollisionMeshFileLoader::loadResource(IResource* pInPlace)
{
    return loadResource(dynamic_cast<CollisionMesh*>(pInPlace));
}

//////////////////////////////////////////////////////////////////////////

void* gep::HavokPhysicsFactoryAllocator::allocateMemory(size_t size)
{
    Allocation allocation;
    allocation.m_size = size;

    auto ptr = m_pParentAllocator->allocateMemory(size);
    m_allocations[ptr] = allocation;

    m_cachedNumBytesUsed += size;

    return ptr;
}

void gep::HavokPhysicsFactoryAllocator::freeMemory(void* mem)
{
    if (mem == nullptr)
    {
        return;
    }

    Allocation allocation;

    if(m_allocations.tryGet(mem, allocation) == FAILURE)
    {
        GEP_ASSERT(false, "double or invalid free.");
    }
    
    m_allocations.remove(mem);
    m_cachedNumBytesUsed -= allocation.m_size;
    ++m_numFrees;

    m_pParentAllocator->freeMemory(mem);
}

gep::HavokPhysicsFactoryAllocator::HavokPhysicsFactoryAllocator(IAllocatorStatistics* pParentAllocator) :
    m_pParentAllocator(pParentAllocator),
    m_cachedNumBytesUsed(0),
    m_numFrees(0)
{
}

gep::HavokPhysicsFactoryAllocator::~HavokPhysicsFactoryAllocator()
{
    m_pParentAllocator = nullptr;
}

//////////////////////////////////////////////////////////////////////////
gep::HavokPhysicsFactory::HavokPhysicsFactory(IAllocatorStatistics* pAllocator) :
    m_pAllocator(pAllocator)
{
    GEP_ASSERT(pAllocator, "The allocator may not be null!");
}

gep::HavokPhysicsFactory::~HavokPhysicsFactory()
{
    m_pAllocator = nullptr;
}

void gep::HavokPhysicsFactory::initialize()
{
    g_globalManager.getMemoryManager()->registerAllocator("Physics Factory", m_pAllocator);
    g_globalManager.getResourceManager()->registerResourceType("CollisionMesh", nullptr);
}

void gep::HavokPhysicsFactory::destroy()
{
    g_globalManager.getMemoryManager()->deregisterAllocator(m_pAllocator);
}

gep::IAllocatorStatistics* gep::HavokPhysicsFactory::getAllocator()
{
    return m_pAllocator;
}

void gep::HavokPhysicsFactory::setAllocator(IAllocatorStatistics* allocator)
{
    m_pAllocator = allocator;
}

gep::IWorld* gep::HavokPhysicsFactory::createWorld(const WorldCInfo& cinfo) const
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
    auto pWorld = GEP_NEW(m_pAllocator, HavokWorld)(cinfo);
    return pWorld;
}

gep::IRigidBody* gep::HavokPhysicsFactory::createRigidBody(const RigidBodyCInfo& cinfo) const
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
    HavokRigidBody* pRigidBody = GEP_NEW(m_pAllocator, HavokRigidBody)(cinfo);
    pRigidBody->initialize();
    return pRigidBody;
}

gep::ICharacterRigidBody* gep::HavokPhysicsFactory::createCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) const
{
    GEP_ASSERT(m_pAllocator, "Allocator cannot be nullptr!");
    auto pCharacterRigidBody = GEP_NEW(m_pAllocator, HavokCharacterRigidBody)(cinfo);
    pCharacterRigidBody->initialize();
    return pCharacterRigidBody;
}

gep::ResourcePtr<gep::ICollisionMesh> gep::HavokPhysicsFactory::loadCollisionMesh(const char* path)
{
    auto pResult = g_globalManager.getResourceManager()->loadResource<CollisionMesh>(CollisionMeshFileLoader(path), LoadAsync::No);
    postProcessNewShape(pResult->getShape());
    return pResult;
}

gep::IShape* gep::HavokPhysicsFactory::loadCollisionMeshFromLua(const char* path)
{
    auto pResult = loadCollisionMesh(path).get()->getShape();
    return postProcessNewShape(pResult);
}

gep::IBoxShape* gep::HavokPhysicsFactory::createBox(const vec3& halfExtends)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_Box)(halfExtends);
    return postProcessNewShape(pResult);
}

gep::ISphereShape* gep::HavokPhysicsFactory::createSphere(float radius)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_Sphere)(radius);
    return postProcessNewShape(pResult);
}

gep::ICapsuleShape* gep::HavokPhysicsFactory::createCapsule(const vec3& start, const vec3& end, float radius)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_Capsule)(start, end, radius);
    return postProcessNewShape(pResult);
}

gep::ICylinderShape* gep::HavokPhysicsFactory::createCylinder(const vec3& start, const vec3& end, float radius)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_Cylinder)(start, end, radius);
    return postProcessNewShape(pResult);
}

gep::ITriangleShape* gep::HavokPhysicsFactory::createTriangle(const vec3& vertexA, const vec3& vertexB, const vec3& vertexC)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_Triangle)(vertexA, vertexB, vertexC);
    return postProcessNewShape(pResult);
}

gep::IConvexTranslateShape* gep::HavokPhysicsFactory::createConvexTranslateShape(IShape* pShape, const vec3& translation)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_ConvexTranslate)(pShape, translation);
    return postProcessNewShape(pResult);
}

gep::ITransformShape* gep::HavokPhysicsFactory::createTransformShape(IShape* pShape, const vec3& translation, const Quaternion& rotation)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_Transform)(pShape, translation, rotation);
    return postProcessNewShape(pResult);
}

gep::IBoundingVolumeShape* gep::HavokPhysicsFactory::createBoundingVolumeShape(IShape* pBounding, IShape* pChild)
{
    auto pResult = GEP_NEW(m_pAllocator, HavokShape_BoundingVolume)(pBounding, pChild);
    return postProcessNewShape(pResult);
}

gep::IPhantomCallbackShape* gep::HavokPhysicsFactory::createPhantomCallbackShape()
{
    auto pResult = GEP_NEW(m_pAllocator, HavokPhantomCallbackShapeGep)();
    return postProcessNewShape(pResult);
}

gep::ICollisionFilter* gep::HavokPhysicsFactory::createCollisionFilter_Simple()
{
    auto pHkFilter = new HavokCollisionFilter_Simple();
    auto pFilterWrapper = GEP_NEW(m_pAllocator, HavokCollisionFilterWrapper)(pHkFilter);
    pHkFilter->removeReference();
    return pFilterWrapper;
}