#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/havokPhysicsShape.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/conversion/shape.h"
#include "gepimpl/subsystems/physics/havok/conversion/quaternion.h"
#include "gepimpl/subsystems/physics/havok/conversion/vector.h"

void gep::HavokShapeBase::initialize()
{
    GEP_ASSERT(m_pHkpShape, "The hkpShape must not be null!");
    GEP_ASSERT(m_pHkpShape->getReferenceCount() > 1, "Invalid ref count!");

    m_pHkpShape->setUserData(reinterpret_cast<hkUlong>(this));
    m_pHkpShape->removeReference();
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_Box::HavokShape_Box(const vec3& halfExtents)
{
    setHkpShape(new hkpBoxShape(conversion::hk::to(halfExtents)));
}

gep::vec3 gep::HavokShape_Box::getHalfExtents() const
{
    return conversion::hk::from(static_cast<const hkpBoxShape*>(getHkpShape())->getHalfExtents());
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_Sphere::HavokShape_Sphere(float radius)
{
    setHkpShape(new hkpSphereShape(radius));
}

float gep::HavokShape_Sphere::getRadius() const
{
    return static_cast<const hkpSphereShape*>(getHkpShape())->getRadius();
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_Capsule::HavokShape_Capsule(const vec3& start, const vec3& end, float radius)
{
    hkVector4 hkStart;
    hkVector4 hkEnd;
    conversion::hk::to(start, hkStart);
    conversion::hk::to(end, hkEnd);

    auto pHkpShape = new hkpCapsuleShape(hkStart, hkEnd, radius);
    setHkpShape(pHkpShape);
}

gep::vec3 gep::HavokShape_Capsule::getStart() const
{
    return conversion::hk::from(static_cast<const hkpCapsuleShape*>(getHkpShape())->getVertex<0>());
}

gep::vec3 gep::HavokShape_Capsule::getEnd() const
{
    return conversion::hk::from(static_cast<const hkpCapsuleShape*>(getHkpShape())->getVertex<1>());
}

float gep::HavokShape_Capsule::getRadius() const
{
    return static_cast<const hkpCapsuleShape*>(getHkpShape())->getRadius();
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_Cylinder::HavokShape_Cylinder(const vec3& start, const vec3& end, float radius)
{
    hkVector4 hkStart;
    hkVector4 hkEnd;
    conversion::hk::to(start, hkStart);
    conversion::hk::to(end, hkEnd);

    auto pHkpShape = new hkpCylinderShape(hkStart, hkEnd, radius);
    setHkpShape(pHkpShape);
}

gep::vec3 gep::HavokShape_Cylinder::getStart() const
{
    return conversion::hk::from(static_cast<const hkpCylinderShape*>(getHkpShape())->getVertex<0>());
}

gep::vec3 gep::HavokShape_Cylinder::getEnd() const
{
    return conversion::hk::from(static_cast<const hkpCylinderShape*>(getHkpShape())->getVertex<1>());
}

float gep::HavokShape_Cylinder::getRadius() const
{
    return static_cast<const hkpCylinderShape*>(getHkpShape())->getCylinderRadius();
}

gep::HavokShape_Triangle::HavokShape_Triangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2)
{
    auto hkVertex0 = conversion::hk::to(vertex0);
    auto hkVertex1 = conversion::hk::to(vertex1);
    auto hkVertex2 = conversion::hk::to(vertex2);
    auto pHkpShape = new hkpTriangleShape(hkVertex0, hkVertex1, hkVertex2);
    setHkpShape(pHkpShape);
}

gep::vec3 gep::HavokShape_Triangle::getVertex(int32 index) const
{
    auto pHkpShape = static_cast<const hkpTriangleShape*>(getHkpShape());
    return conversion::hk::from(pHkpShape->getVertex(index));
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_ConvexTranslate::HavokShape_ConvexTranslate(IShape* pShape, const vec3& translation) :
    m_pChildShape(pShape)
{
    auto pHkpShape = conversion::hk::to(pShape);
    auto hkTranslation = conversion::hk::to(translation);
    GEP_ASSERT(dynamic_cast<hkpConvexShape*>(pHkpShape), "Invalid type of shape!");
    auto pShapeToWrap = new hkpConvexTranslateShape(static_cast<hkpConvexShape*>(pHkpShape), hkTranslation);
    setHkpShape(pShapeToWrap);
}

gep::vec3 gep::HavokShape_ConvexTranslate::getTranslation() const
{
    auto pActualShape = static_cast<const hkpConvexTranslateShape*>(getHkpShape());
    return conversion::hk::from(pActualShape->getTranslation());
}

gep::IShape* gep::HavokShape_ConvexTranslate::getChildShape()
{
    return m_pChildShape.get();
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_Transform::HavokShape_Transform(IShape* pShape, const vec3& translation, const Quaternion& rotation)
{
    auto pHkpShape = conversion::hk::to(pShape);
    auto hkTranslation = conversion::hk::to(translation);
    auto hkRotation = conversion::hk::to(rotation);
    auto theTransform = hkTransform(hkRotation, hkTranslation);
    
    auto pShapeToWrap = new hkpTransformShape(pHkpShape, theTransform);
    setHkpShape(pShapeToWrap);
}

gep::IShape* gep::HavokShape_Transform::getChildShape()
{
    return m_pChildShape.get();
}

gep::vec3 gep::HavokShape_Transform::getTranslation() const
{
    auto& transform = static_cast<const hkpTransformShape*>(getHkpShape())->getTransform();
    return conversion::hk::from(transform.getTranslation());
}

gep::Quaternion gep::HavokShape_Transform::getRotation() const
{
    auto& transform = static_cast<const hkpTransformShape*>(getHkpShape())->getTransform();
    return conversion::hk::from(hkQuaternion(transform.getRotation()));
}

//////////////////////////////////////////////////////////////////////////

gep::HavokShape_BoundingVolume::HavokShape_BoundingVolume(IShape* pBounding, IShape* pChild)
{
    auto pHkpBounding = conversion::hk::to(pBounding);
    auto pHkpChild = conversion::hk::to(pChild);
    auto pShapeToWrap = new hkpBvShape(pHkpBounding, pHkpChild);
    setHkpShape(pShapeToWrap);
}

gep::IShape* gep::HavokShape_BoundingVolume::getBoundingShape() const
{
    return m_pBounding.get();
}

gep::IShape* gep::HavokShape_BoundingVolume::getChildShape() const
{
    return m_pChild.get();
}

//////////////////////////////////////////////////////////////////////////

void gep::HavokPhantomCallbackShapeHk::phantomEnterEvent(const hkpCollidable* phantomColl, const hkpCollidable* otherColl, const hkpCollisionInput& env)
{
    auto pHkBody = hkpGetRigidBody(otherColl);
    auto pGepBody = reinterpret_cast<HavokRigidBody*>(pHkBody->getUserData());
    m_pGepShape->getEnterEvent()->trigger(pGepBody);
}

void gep::HavokPhantomCallbackShapeHk::phantomLeaveEvent(const hkpCollidable* phantomColl, const hkpCollidable* otherColl)
{
    auto pHkBody = hkpGetRigidBody(otherColl);
    auto pGepBody = reinterpret_cast<HavokRigidBody*>(pHkBody->getUserData());
    m_pGepShape->getLeaveEvent()->trigger(pGepBody);
}
