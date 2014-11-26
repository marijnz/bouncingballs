#pragma once

#include "gep/interfaces/physics/shape.h"

#include "gepimpl/subsystems/physics/havok/havokPhysicsShape.h"

namespace gep {
namespace conversion {
namespace hk {

    //////////////////////////////////////////////////////////////////////////

    namespace detail
    {
        template<typename T_WrapperShape>
        hkpShape* to_shape(const IShape* in_pGepShape)
        {
            GEP_ASSERT(dynamic_cast<const T_WrapperShape*>(in_pGepShape) != nullptr, "Shape type does not match the actual class type!");
            auto pWrapper = static_cast<const T_WrapperShape*>(in_pGepShape);
            return const_cast<T_WrapperShape*>(pWrapper)->getHkpShape();
        }

        template<typename T_HavokShape, typename T_WrapperShape>
        IShape* from_shape(const hkpShape* in_pHkShape)
        {
            auto pActual = static_cast<const T_HavokShape*>(in_pHkShape);
            return static_cast<T_WrapperShape*>(reinterpret_cast<HavokShapeBase*>(pActual->getUserData()));
        }
    }

    inline hkpShape* to(IShape* in_gepShape)
    {
        GEP_ASSERT(in_gepShape, "Shape is a nullptr!");

        switch (in_gepShape->getShapeType())
        {
        case ShapeType::Box:
            return detail::to_shape<HavokShape_Box>(in_gepShape);
        case ShapeType::ConvexTranslate:
            return detail::to_shape<HavokShape_ConvexTranslate>(in_gepShape);
        case ShapeType::Transform:
            return detail::to_shape<HavokShape_Transform>(in_gepShape);
        case ShapeType::Sphere:
            return detail::to_shape<HavokShape_Sphere>(in_gepShape);
        case ShapeType::Capsule:
            return detail::to_shape<HavokShape_Capsule>(in_gepShape);
        case ShapeType::Cylinder:
            return detail::to_shape<HavokShape_Cylinder>(in_gepShape);
        case ShapeType::Triangle:
            return detail::to_shape<HavokShape_Triangle>(in_gepShape);
        case ShapeType::BoundingVolumeCompressedMesh:
            {
                auto mesh = static_cast<HavokMeshShape*>(in_gepShape);
                GEP_ASSERT(dynamic_cast<HavokMeshShape*>(in_gepShape) != nullptr, "Shape type does not match the actual class type!");
                return mesh->getHkShape();
            }
            break;
        case ShapeType::BoundingVolume:
            return detail::to_shape<HavokShape_BoundingVolume>(in_gepShape);
        case ShapeType::PhantomCallback:
            return static_cast<HavokPhantomCallbackShapeGep*>(in_gepShape)->getHkShape();
        default:
            GEP_ASSERT(false, "Unsupported shape type!");
            break;
        }

        return nullptr;
    }

    inline const hkpShape* to(const IShape* in_gepShape)
    {
        return to( const_cast<IShape*>(in_gepShape) );
    }

    inline IShape* from(hkpShape* in_hkShape)
    {
        GEP_ASSERT(in_hkShape, "The shape is a nullptr!");

        switch (in_hkShape->getType())
        {
        case hkcdShapeType::BOX:
            return detail::from_shape<hkpBoxShape, HavokShape_Box>(in_hkShape);
        case hkcdShapeType::CONVEX_TRANSLATE:
            return detail::from_shape<hkpConvexTranslateShape, HavokShape_ConvexTranslate>(in_hkShape);
        case hkcdShapeType::TRANSFORM:
            return detail::from_shape<hkpTransformShape, HavokShape_Transform>(in_hkShape);
        case hkcdShapeType::SPHERE:
            return detail::from_shape<hkpSphereShape, HavokShape_Sphere>(in_hkShape);
        case hkcdShapeType::CAPSULE:
            return detail::from_shape<hkpCapsuleShape, HavokShape_Capsule>(in_hkShape);
        case hkcdShapeType::CYLINDER:
            return detail::from_shape<hkpCylinderShape, HavokShape_Cylinder>(in_hkShape);
        case hkcdShapeType::TRIANGLE:
            return detail::from_shape<hkpTriangleShape, HavokShape_Triangle>(in_hkShape);

        //TODO: Check if it is actually ok to put all of these into a HavokMeshShape!
        case hkcdShapeType::BV_COMPRESSED_MESH:
        case hkcdShapeType::CONVEX_VERTICES:
            return GEP_NEW(g_stdAllocator, HavokMeshShape)(in_hkShape);

        case hkcdShapeType::BV:
            return detail::from_shape<hkpBvShape, HavokShape_BoundingVolume>(in_hkShape);
        case ShapeType::PhantomCallback:
            return static_cast<HavokPhantomCallbackShapeHk*>(in_hkShape)->getOwner();
        default:
            GEP_ASSERT(false, "Unsupported shape type!");
            break;
        }

        return nullptr;
    }

    inline const IShape* from(const hkpShape* in_hkShape)
    {
        return from( const_cast<hkpShape*>(in_hkShape) );
    }

}}} // namespace gep::conversion::hk
