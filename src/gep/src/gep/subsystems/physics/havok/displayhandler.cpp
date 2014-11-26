#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/displayHandler.h"
#include "gep/math3d/vec3.h"
#include "gepimpl/subsystems/physics/havok/conversion/transformation.h"
#include "gep/globalManager.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/logging.h"

#include "gepimpl/havok/util.h"




namespace gep {

    class HavokDataHolder : public ReferenceCounted
    {
        hkDisplayGeometry* m_pDisplayGeometry;
        DynamicArray<uint32> m_indices;

    public:
        HavokDataHolder(hkDisplayGeometry* displayGeometry, const hkArray<hkGeometry::Triangle>& triangles)
        {
            m_pDisplayGeometry = displayGeometry;
            displayGeometry->addReference();

            m_indices.reserve(triangles.getSize() * 3);
            for(auto& triangle : triangles)
            {
                m_indices.append(triangle.m_a);
                m_indices.append(triangle.m_b);
                m_indices.append(triangle.m_c);
            }

        }

        ~HavokDataHolder()
        {
            GEP_HK_REMOVE_REF_AND_NULL(m_pDisplayGeometry); 
        }

        inline const ArrayPtr<uint32> getIndices() const { return m_indices.toArray(); }
    };

    HavokDisplayManager::HavokDisplayManager() : 
        m_extractionCallbackId(0)
    {
    }

    HavokDisplayManager::~HavokDisplayManager()
    {
    }

    void HavokDisplayManager::initialize()
    {
    }

    void HavokDisplayManager::destroy()
    {
        for(auto& geometryInfos : m_geometries.values())
        {
            for(auto& info : geometryInfos)
            {
                g_globalManager.getResourceManager()->deleteResource(info.pModel);
            }
            geometryInfos.clear();
        }
    }

    hkResult HavokDisplayManager::addGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint, hkGeometry::GeometryType createDyanamicGeometry /*= hkGeometry::GEOMETRY_STATIC*/)
    {
        auto& a = m_geometries[id];
       
        vec3 newTranslation(DO_NOT_INITIALIZE);
        Quaternion newRotation(DO_NOT_INITIALIZE);
        gep::conversion::hk::from(transform, newTranslation, newRotation);

        newRotation = newRotation * Quaternion(vec3(1,0,0), -90);
        mat4 newTransform =  mat4::translationMatrix(newTranslation) * newRotation.toMat4();
        m_transformations[id] = newTransform;
        
        for (auto geom : geometries)
        {
           
            geom->buildGeometry();
            auto pGeom = geom->getGeometry();
            auto pDataHolder = GEP_NEW(g_stdAllocator, HavokDataHolder)(geom, pGeom->m_triangles);
            auto vertices = ArrayPtr<vec4>((vec4*)&pGeom->m_vertices[0], pGeom->m_vertices.getSize());
            auto indices = pDataHolder->getIndices();

            DisplayGeometryInfo info;
            info.pModel = g_globalManager.getRenderer()->loadModel(pDataHolder, vertices, indices);

            gep::conversion::hk::from(geom->getTransform(), newTranslation, newRotation);
            newRotation = newRotation * Quaternion(vec3(1,0,0), -90);
            info.transform = mat4::translationMatrix(newTranslation) * newRotation.toMat4();
            a.append(info);
        }

        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::addGeometryInstance(hkUlong origianalGeomId, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint)
    {
       return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::setGeometryPickable(hkBool isPickable, hkUlong id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::setGeometryVisibility(int geometryIndex, bool isEnabled, hkUlong id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::setGeometryColor(hkColor::Argb color, hkUlong id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::setGeometryTransparency(float alpha, hkUlong id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::updateGeometry(const hkTransform& transform, hkUlong id, int tag)
    {
        vec3 newTranslation(DO_NOT_INITIALIZE);
        Quaternion newRotation(DO_NOT_INITIALIZE);
        gep::conversion::hk::from(transform, newTranslation, newRotation);
        mat4 newTransform =  mat4::translationMatrix(newTranslation) * newRotation.toMat4();
        m_transformations[id] = newTransform;
        
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::updateGeometry(const hkMatrix4& transform, hkUlong id, int tag)
    {
        auto trans = hkTransform();
        hkFloat32* matrix = nullptr;
        transform.get4x4ColumnMajor(matrix);
        trans.set4x4ColumnMajor(matrix);
        return updateGeometry(trans, id, tag);
    }

    hkResult HavokDisplayManager::skinGeometry(hkUlong* ids, int numIds, const hkMatrix4* poseModel, int numPoseModel, const hkMatrix4& worldFromModel, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::removeGeometry(hkUlong id, int tag, hkUlong shapeIdHint)
    {
        m_geometries.remove(id);
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayPoint(const hkVector4& position, hkColor::Argb color, int id, int tag)
    {
       return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayLine(const hkVector4& start, const hkVector4& end, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayTriangle(const hkVector4& a, const hkVector4& b, const hkVector4& c, hkColor::Argb color, int id, int tag)
    {
       return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayText(const char* text, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::display3dText(const char* text, const hkVector4& pos, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayPoint2d(const hkVector4& position, hkColor::Argb color, int id, int tag)
    {
       return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayLine2d(const hkVector4& start, const hkVector4& end, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayTriangle2d(const hkVector4& a, const hkVector4& b, const hkVector4& c, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayText2d(const char* text, const hkVector4& pos, hkReal sizeScale, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::displayGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, hkColor::Argb color, int id, int tag)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::sendMemStatsDump(const char* data, int length)
    {
        return HK_SUCCESS;
    }

    hkResult HavokDisplayManager::holdImmediate()
    {
        return HK_SUCCESS;
    }

    void HavokDisplayManager::extract(IRendererExtractor& extractor)
    {
        DebugMarkerSection marker(extractor, "havokDisplayManager");
        for(auto& it : m_geometries)
        {
            auto& transform = m_transformations[it.key];
            for(auto& info : it.value)
            {
                auto actualTransform = transform * info.transform;
                info.pModel->extract(extractor, actualTransform);
            }
        }
    }

    void HavokDisplayManager::setEnabled(bool value)
    {
        if (value)
        {
            if (m_extractionCallbackId.id != 0) return; // we are already enabled
            
            m_extractionCallbackId = g_globalManager.getRendererExtractor()->registerExtractionCallback(std::bind(&HavokDisplayManager::extract, this, std::placeholders::_1));
            GEP_ASSERT(m_extractionCallbackId.id != 0, "Unable to register extraction!");
        }
        else
        {
            if (m_extractionCallbackId.id == 0) return; // we are already disabled
            
            g_globalManager.getRendererExtractor()->deregisterExtractionCallback(m_extractionCallbackId);
            m_extractionCallbackId = CallbackId(0);
        }
    }

    bool HavokDisplayManager::isEnabled() const
    {
        return m_extractionCallbackId.id != 0;
    }

}
