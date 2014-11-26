#pragma once
#include "gep/interfaces/physics.h"
#include "gepimpl/havok/util.h"

namespace gep
{
    class HavokShapeBase
    {
        hkRefPtr<hkpShape> m_pHkpShape;
    public:
        virtual ~HavokShapeBase() {}

        void initialize();

        inline       hkpShape* getHkpShape()       { return m_pHkpShape.val(); }
        inline const hkpShape* getHkpShape() const { return m_pHkpShape.val(); }
        inline void setHkpShape(hkpShape* pHkpShape) { m_pHkpShape = pHkpShape; }
    };

    class HavokShape_Box :
        public IBoxShape,
        public HavokShapeBase
    {
    public:
        HavokShape_Box(const vec3& halfExtents);
        virtual ~HavokShape_Box() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual vec3 getHalfExtents() const override;
    };

    class HavokShape_Sphere :
        public ISphereShape,
        public HavokShapeBase
    {
    public:
        HavokShape_Sphere(float radius);
        virtual ~HavokShape_Sphere() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual float getRadius() const override;
    };

    class HavokShape_Capsule :
        public ICapsuleShape,
        public HavokShapeBase
    {
    public:
        HavokShape_Capsule(const vec3& start, const vec3& end, float radius);
        virtual ~HavokShape_Capsule() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual vec3 getStart() const override;
        virtual vec3 getEnd() const override;
        virtual float getRadius() const override;
    };

    class HavokShape_Cylinder :
        public ICylinderShape,
        public HavokShapeBase
    {
    public:
        HavokShape_Cylinder(const vec3& start, const vec3& end, float radius);
        virtual ~HavokShape_Cylinder() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual vec3 getStart() const override;
        virtual vec3 getEnd() const override;
        virtual float getRadius() const override;
    };

    class HavokShape_Triangle :
        public ITriangleShape,
        public HavokShapeBase
    {
    public:
        HavokShape_Triangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2);
        virtual ~HavokShape_Triangle() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual vec3 getVertex(int32 index) const override;
    };

    class HavokShape_ConvexTranslate :
        public IConvexTranslateShape,
        public HavokShapeBase
    {
        /// \brief Used only so it lives at least as long as this instance.
        mutable SmartPtr<IShape> m_pChildShape;
    public:
        HavokShape_ConvexTranslate(IShape* pShape, const vec3& translation);
        virtual ~HavokShape_ConvexTranslate() {}
        
        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual IShape* getChildShape() override;
        virtual vec3 getTranslation() const override;
    };

    class HavokShape_Transform :
        public ITransformShape,
        public HavokShapeBase
    {
        /// \brief Used only so it lives at least as long as this instance.
        mutable SmartPtr<IShape> m_pChildShape;
    public:
        HavokShape_Transform(IShape* pShape, const vec3& translation, const Quaternion& rotation);
        virtual ~HavokShape_Transform() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual IShape* getChildShape() override;
        virtual vec3 getTranslation() const override;
        virtual Quaternion getRotation() const override;
    };

    class HavokShape_BoundingVolume :
        public IBoundingVolumeShape,
        public HavokShapeBase
    {
        /// \brief Used only so they live at least as long as this instance.
        mutable SmartPtr<IShape> m_pBounding;
        mutable SmartPtr<IShape> m_pChild;

    public:
        HavokShape_BoundingVolume(IShape* pBounding, IShape* pChild);
        virtual ~HavokShape_BoundingVolume() {}

        virtual void IShape::initialize() override { HavokShapeBase::initialize(); }

        virtual IShape* getBoundingShape() const override;
        virtual IShape* getChildShape() const override;
    };

    class HavokMeshShape : public IShape
    {
        hkpShape* m_pHkActualShape;
        const ITransform* m_pTransform;
    public:
        HavokMeshShape(hkpShape* havokShape) : 
            m_pHkActualShape(nullptr),
            m_pTransform(nullptr)
        {
            setHkShape(havokShape); 
        }
        ~HavokMeshShape()
        {
            hk::removeRefAndNull(m_pHkActualShape); 
            DELETE_AND_NULL(m_pTransform);
        }

        virtual void initialize() override { }

        virtual ShapeType::Enum getShapeType() const override { return ShapeType::BoundingVolumeCompressedMesh; }

        hkpShape* getHkShape() { return m_pHkActualShape; }
        const hkpShape* getHkShape() const { return m_pHkActualShape; }
        void setHkShape(hkpShape* value)
        {
            if (m_pHkActualShape == value) return;
            
            if (m_pHkActualShape)
                m_pHkActualShape->removeReference();
            
            m_pHkActualShape = value;

            if (m_pHkActualShape)
                m_pHkActualShape->addReference();
        }

        void setTransform(const ITransform* transform) {m_pTransform = transform;}

        const ITransform* getTransform() const{return m_pTransform;}


    };

    class HavokPhantomCallbackShapeGep;

    class HavokPhantomCallbackShapeHk : public hkpPhantomCallbackShape
    {
        friend class HavokPhantomCallbackShapeGep;
    public:
        HavokPhantomCallbackShapeHk() :
            hkpPhantomCallbackShape(),
            m_pGepShape(nullptr)
        {
        }

        virtual void phantomEnterEvent(const hkpCollidable* phantomColl, const hkpCollidable* otherColl, const hkpCollisionInput& env) override;
        virtual void phantomLeaveEvent(const hkpCollidable* phantomColl, const hkpCollidable* otherColl) override;

        inline HavokPhantomCallbackShapeGep* getOwner() { return m_pGepShape; }

    private:
        HavokPhantomCallbackShapeGep* m_pGepShape;
    };

    class HavokPhantomCallbackShapeGep : public IPhantomCallbackShape
    {
        const ITransform* m_pTransform;
    public:

        HavokPhantomCallbackShapeGep(): m_pTransform(new Transform()) { m_hkShape.m_pGepShape = this; }
        ~HavokPhantomCallbackShapeGep(){DELETE_AND_NULL(m_pTransform);}

        virtual void initialize() override {}

        virtual Event<IRigidBody*>* getEnterEvent() override { return &m_enterEvent; }
        virtual Event<IRigidBody*>* getLeaveEvent() override { return &m_leaveEvent; }

        inline HavokPhantomCallbackShapeHk* getHkShape() { return &m_hkShape; }
        const ITransform* getTransform() const{return m_pTransform;}

    private:
        Event<IRigidBody*> m_enterEvent;
        Event<IRigidBody*> m_leaveEvent;

        HavokPhantomCallbackShapeHk m_hkShape;
    };
}
