#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/compressedMeshCinfo.h"




void gep::CompressedMeshCinfo::getConvexShape(int convexIndex, const hkpConvexShape*& convexShapeOut, hkQsTransform& transformOut) const
{
    throw std::logic_error("The method or operation is not implemented.");
}

int gep::CompressedMeshCinfo::getNumConvexShapes() const
{
    throw std::logic_error("The method or operation is not implemented.");
}

void gep::CompressedMeshCinfo::getIndices(int triangleIndex, int* indices) const
{
    throw std::logic_error("The method or operation is not implemented.");
}

void gep::CompressedMeshCinfo::getVertex(int vertexIndex, hkVector4& vertexOut) const
{
    throw std::logic_error("The method or operation is not implemented.");
}

int gep::CompressedMeshCinfo::getNumTriangles() const
{
    throw std::logic_error("The method or operation is not implemented.");
}

int gep::CompressedMeshCinfo::getNumVertices() const
{
    throw std::logic_error("The method or operation is not implemented.");
}
