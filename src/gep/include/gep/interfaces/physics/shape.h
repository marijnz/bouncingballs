#pragma once
#include "gep/math3d/vec3.h"
#include "gep/math3d/transform.h"
#include "gep/ReferenceCounting.h"
#include "gep/interfaces/events.h"

namespace gep
{
    class IRigidBody;

    /// \brief Purely static struct that serves as an enum.
    struct ShapeType
    {
        enum Enum
        {
            Sphere = 0,
            Cylinder = 1,
            Triangle = 2,
            Box = 3,
            Capsule = 4,
            ConvexVertices = 5,
            ConvexTranslate = 10,
            ConvexTransform = 11,
            Transform = 14,
            BoundingVolumeCompressedMesh = 17,
            BoundingVolume = 30,
            PhantomCallback = 32,
        };

        GEP_DISALLOW_CONSTRUCTION(ShapeType);
    };
    
    class IShape : public ReferenceCounted
    {
    public:
        virtual void initialize() = 0;
        virtual ShapeType::Enum getShapeType() const = 0;
        virtual const ITransform* getTransform() const { return nullptr; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class IBoxShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Box; }

        virtual vec3 getHalfExtents() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getHalfExtents)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class ISphereShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Sphere; }

        virtual float getRadius() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getRadius)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class ICapsuleShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Capsule; }

        virtual float getRadius() const = 0;
        virtual vec3 getStart() const = 0;
        virtual vec3 getEnd() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getRadius)
            LUA_BIND_FUNCTION(getStart)
            LUA_BIND_FUNCTION(getEnd)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class ICylinderShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Cylinder; }

        virtual float getRadius() const = 0;
        virtual vec3 getStart() const = 0;
        virtual vec3 getEnd() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getRadius)
            LUA_BIND_FUNCTION(getStart)
            LUA_BIND_FUNCTION(getEnd)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class ITriangleShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Triangle; }

        virtual vec3 getVertex(int32 index) const = 0;
    };

    class IBoundingVolumeShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::BoundingVolume; }

        virtual IShape* getBoundingShape() const = 0;
        virtual IShape* getChildShape() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getBoundingShape)
            LUA_BIND_FUNCTION(getChildShape)
        LUA_BIND_REFERENCE_TYPE_END;
    };
    
    class IPhantomCallbackShape : public IShape
    {
    public:

        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::PhantomCallback; }

        virtual Event<IRigidBody*>* getEnterEvent() = 0;
        virtual Event<IRigidBody*>* getLeaveEvent() = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getEnterEvent)
            LUA_BIND_FUNCTION(getLeaveEvent)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class IConvexTranslateShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::ConvexTranslate; }

        virtual IShape* getChildShape() = 0;
        virtual vec3 getTranslation() const = 0;

        virtual const ITransform* getTransform() const override { return nullptr; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getChildShape)
            LUA_BIND_FUNCTION(getTranslation)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class ITransformShape : public IShape
    {
    public:
        inline virtual ShapeType::Enum getShapeType() const override { return ShapeType::Transform; }

        virtual IShape* getChildShape() = 0;
        virtual vec3 getTranslation() const = 0;
        virtual Quaternion getRotation() const = 0;

        virtual const ITransform* getTransform() const override { return nullptr; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getChildShape)
            LUA_BIND_FUNCTION(getTranslation)
            LUA_BIND_FUNCTION(getRotation)
        LUA_BIND_REFERENCE_TYPE_END;
    };
}
