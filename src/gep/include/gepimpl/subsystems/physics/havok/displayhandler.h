#pragma once

#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/math3d/vec3.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/updateFramework.h"

namespace gep
{
    class HavokDisplayManager : public hkDebugDisplayHandler
    {
        struct DisplayGeometryInfo
        {
            ResourcePtr< IModel > pModel;
            mat4 transform;
        };

    public:

        HavokDisplayManager();
        ~HavokDisplayManager();

        void initialize();
        void destroy();

        virtual hkResult addGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint, hkGeometry::GeometryType createDyanamicGeometry = hkGeometry::GEOMETRY_STATIC) override;

        virtual hkResult addGeometryInstance(hkUlong origianalGeomId, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint) override;

        virtual hkResult setGeometryPickable(hkBool isPickable, hkUlong id, int tag) override;

        virtual hkResult setGeometryVisibility(int geometryIndex, bool isEnabled, hkUlong id, int tag) override;

        virtual hkResult setGeometryColor(hkColor::Argb color, hkUlong id, int tag) override;

        virtual hkResult setGeometryTransparency(float alpha, hkUlong id, int tag) override;

        virtual hkResult updateGeometry(const hkTransform& transform, hkUlong id, int tag) override;

        virtual hkResult updateGeometry(const hkMatrix4& transform, hkUlong id, int tag) override;

        virtual hkResult skinGeometry(hkUlong* ids, int numIds, const hkMatrix4* poseModel, int numPoseModel, const hkMatrix4& worldFromModel, int tag) override;

        virtual hkResult removeGeometry(hkUlong id, int tag, hkUlong shapeIdHint) override;

        virtual hkResult updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name) override;

        virtual hkResult displayPoint(const hkVector4& position, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayLine(const hkVector4& start, const hkVector4& end, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayTriangle(const hkVector4& a, const hkVector4& b, const hkVector4& c, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayText(const char* text, hkColor::Argb color, int id, int tag) override;

        virtual hkResult display3dText(const char* text, const hkVector4& pos, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayPoint2d(const hkVector4& position, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayLine2d(const hkVector4& start, const hkVector4& end, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayTriangle2d(const hkVector4& a, const hkVector4& b, const hkVector4& c, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayText2d(const char* text, const hkVector4& pos, hkReal sizeScale, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkColor::Argb color, int id, int tag) override;

        virtual hkResult displayGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, hkColor::Argb color, int id, int tag) override;

        virtual hkResult sendMemStatsDump(const char* data, int length) override;

        virtual hkResult holdImmediate() override;

        void extract(IRendererExtractor& extractor);

        void setEnabled(bool value);
        bool isEnabled() const;

    private:
        CallbackId m_extractionCallbackId;

        Hashmap<hkUlong, DynamicArray<DisplayGeometryInfo>> m_geometries;
        Hashmap<hkUlong, mat4> m_transformations;
    };
}
