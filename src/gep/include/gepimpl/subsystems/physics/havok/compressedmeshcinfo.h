#pragma once

namespace gep{
    class CompressedMeshCinfo : public hkpBvCompressedMeshShapeCinfo
    {

  
    public:
        CompressedMeshCinfo();
        ~CompressedMeshCinfo();
        virtual int getNumVertices() const override;

        virtual int getNumTriangles() const override;

        virtual void getVertex(int vertexIndex, hkVector4& vertexOut) const override;

        virtual void getIndices(int triangleIndex, int* indices) const override;

        virtual int getNumConvexShapes() const override;

        virtual void getConvexShape(int convexIndex, const hkpConvexShape*& convexShapeOut, hkQsTransform& transformOut) const override;

    };
}
