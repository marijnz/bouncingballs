#pragma once
#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/physics/shape.h"
#include "gep/interfaces/physics/constraints.h"

namespace gep
{
    struct WorldCInfo;
    class IWorld;

    struct RigidBodyCInfo;
    class IRigidBody;

    struct CharacterRigidBodyCInfo;
    class ICharacterRigidBody;

    class ICollisionMesh;
    class IShape;

    class ICollisionFilter;

    //////////////////////////////////////////////////////////////////////////

    class IPhysicsFactory
    {
    public:
        virtual ~IPhysicsFactory(){}
        virtual void initialize() = 0;
        virtual void destroy() = 0;

        virtual IAllocatorStatistics* getAllocator() = 0;
        virtual void setAllocator(IAllocatorStatistics* allocator) = 0;

        /// \brief Keep in mind that IWorld is a ReferenceCounted object. Use SmartPtrs whenever possible.
        virtual IWorld* createWorld(const WorldCInfo& cinfo) const = 0;

        /// \brief Keep in mind that IRigidBody is a ReferenceCounted object. Use SmartPtrs whenever possible.
        virtual IRigidBody* createRigidBody(const RigidBodyCInfo& cinfo) const = 0;

        /// \brief Keep in mind that ICharacterRigidBody is a ReferenceCounted object. Use SmartPtrs whenever possible.
        virtual ICharacterRigidBody* createCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) const = 0;

        virtual ResourcePtr<ICollisionMesh> loadCollisionMesh(const char* path) = 0;
        virtual IShape* loadCollisionMeshFromLua(const char* path) = 0;

        virtual ICollisionFilter* createCollisionFilter_Simple() = 0;

        virtual IBoxShape* createBox(const vec3& halfExtends) = 0;
        virtual ISphereShape* createSphere(float radius) = 0;
        virtual ICapsuleShape* createCapsule(const vec3& start, const vec3& end, float radius) = 0;
        virtual ICylinderShape* createCylinder(const vec3& start, const vec3& end, float radius) = 0;
        virtual ITriangleShape* createTriangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2) = 0;
        virtual IConvexTranslateShape* createConvexTranslateShape(IShape* pShape, const vec3& translation) = 0;
        virtual ITransformShape* createTransformShape(IShape* pShape, const vec3& translation, const Quaternion& rotation) = 0;
        virtual IBoundingVolumeShape* createBoundingVolumeShape(IShape* pBounding, IShape* pChild) = 0;
        virtual IPhantomCallbackShape* createPhantomCallbackShape() = 0;

        virtual Constraint createConstraint(ScriptTableWrapper& scriptTable) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createWorld)
            LUA_BIND_FUNCTION(createRigidBody)
            LUA_BIND_FUNCTION(createCollisionFilter_Simple)
            LUA_BIND_FUNCTION(createBox)
            LUA_BIND_FUNCTION(createSphere)
            LUA_BIND_FUNCTION(createCapsule)
            LUA_BIND_FUNCTION(createCylinder)
            LUA_BIND_FUNCTION(createTriangle)
            LUA_BIND_FUNCTION(createConvexTranslateShape)
            LUA_BIND_FUNCTION(createBoundingVolumeShape)
            LUA_BIND_FUNCTION(createPhantomCallbackShape)
            LUA_BIND_FUNCTION_NAMED(loadCollisionMeshFromLua, "loadCollisionMesh")
            LUA_BIND_FUNCTION_NAMED(createConstraint, "_createConstraint")
        LUA_BIND_REFERENCE_TYPE_END
    };


    class ICollisionMesh : public IResource
    {

    public:
        virtual ~ICollisionMesh(){}

        virtual IShape* getShape() = 0;
        virtual const IShape* getShape() const = 0;
    };
}
