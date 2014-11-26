#include "stdafx.h"
#include "gep/modelloader.h"
#include "gep/file.h"
#include "gep/exception.h"
#include "gep/chunkfile.h"
#include <sstream>

namespace {
    float readCompressedFloat(gep::Chunkfile& file)
    {
        gep::int16 data = 0;
        file.read(data);
        return (float)data / (float)std::numeric_limits<gep::int16>::max();
    }
}

gep::ModelLoader::ModelLoader(IAllocator* pAllocator) :
    m_pModelDataAllocator(nullptr),
    m_pStartMarker(nullptr)
{
    if(pAllocator == nullptr)
        pAllocator = &g_stdAllocator;
    m_pAllocator = pAllocator;
}

gep::ModelLoader::~ModelLoader()
{
    if(m_pModelDataAllocator != nullptr)
    {
        m_pModelDataAllocator->freeToMarker(m_pStartMarker);
        GEP_DELETE(m_pAllocator, m_pModelDataAllocator);
    }
}

void gep::ModelLoader::loadFile(const char* pFilename, uint32 loadWhat)
{
    GEP_ASSERT(!m_modelData.hasData,"LoadFile can only be called once");
    m_filename = pFilename;

    if(!fileExists(pFilename))
    {
        std::ostringstream msg;
        msg << "File '" << pFilename << "' does not exist";
        throw LoadingError(msg.str());
    }

    Chunkfile file(pFilename, Chunkfile::Operation::read);

    if(file.startReading("thModel") != SUCCESS)
    {
        std::ostringstream msg;
        msg << "File '" << pFilename << "' is not a thModel format";
        throw LoadingError(msg.str());
    }

    if(file.getFileVersion() < ModelFormatVersion::Version3)
    {
        std::ostringstream msg;
        msg << "File '" << pFilename << "' does have a old format, please reexport";
        throw LoadingError(msg.str());
    }

    MemoryStatistics memstat;

    uint32 nodeNameMemory;

    Load::Enum loadTexCoords[] = {Load::TexCoords0, Load::TexCoords1, Load::TexCoords2, Load::TexCoords3};

    uint32 texturePathMemory = 0;
    uint32 materialNameMemory = 0;
    uint32 numNodes = 0;
    //Read the size info
    {
        const size_t alignmentOverhead = AlignmentHelper::__ALIGNMENT - 1;
        file.startReadChunk();
        if(file.getCurrentChunkName() != "sizeinfo")
        {
            std::ostringstream msg;
            msg << "Expected sizeinfo chunk, got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        size_t modelDataSize=0;

        uint32 numTextures=0;
        file.read(numTextures);
        if(loadWhat & Load::Materials)
        {
            modelDataSize += sizeof(const char*) * numTextures;
            memstat.texturePathReferencesMemory = MemoryPool(sizeof(const char*) * numTextures);
        }

        file.read(texturePathMemory);
        texturePathMemory += numTextures; // trailing \0 bytes
        texturePathMemory = (uint32)AlignmentHelper::__alignedSize(texturePathMemory);
        if(loadWhat & Load::Materials)
        {
            modelDataSize += texturePathMemory;
            memstat.texturePathMemory = MemoryPool(texturePathMemory);
        }

        if(file.getFileVersion() >= ModelFormatVersion::Version2)
        {
            file.read(materialNameMemory);
        }

        uint32 numBones = 0;
        uint32 numBoneInfos = 0;
        if (file.getFileVersion() >= ModelFormatVersion::Version3)
        {
            file.read(numBones);
            file.read(numBoneInfos);
        }

        uint32 numMaterials=0;
        file.read(numMaterials);
        if(loadWhat & Load::Materials)
        {
            memstat.materialData = MemoryPool(numMaterials * sizeof(MaterialData));
            modelDataSize += memstat.materialData.size;
        }

        if(file.getFileVersion() >= ModelFormatVersion::Version2)
        {
            materialNameMemory += numMaterials; /// trailing \0
            if(materialNameMemory % AlignmentHelper::__ALIGNMENT != 0)
                materialNameMemory += AlignmentHelper::__ALIGNMENT - (materialNameMemory % AlignmentHelper::__ALIGNMENT);
            if(loadWhat & Load::Materials)
            {
                modelDataSize += materialNameMemory;
                memstat.materialNameMemory = MemoryPool(materialNameMemory);
            }
        }

        if(file.getFileVersion() >= ModelFormatVersion::Version3 && loadWhat & Load::Bones)
        {
            modelDataSize += sizeof(BoneNode) * numBones;
            memstat.boneDataArray.size += sizeof(BoneNode) * numBones;

modelDataSize += sizeof(BoneInfo)* numBoneInfos;
memstat.boneDataArray.size += sizeof(BoneInfo)* numBoneInfos; // extend the existing pool
        }

        uint32 numMeshes = 0;
        file.read(numMeshes);
        if(loadWhat & Load::Meshes)
        {
            modelDataSize += sizeof(MeshData)* numMeshes;
            memstat.meshDataArray = MemoryPool(sizeof(MeshData)* numMeshes);
        }
        for(uint32 i = 0; i < numMeshes; i++)
        {
            uint32 numVertices = 0, PerVertexFlags = 0, numComponents = 0, numTexcoords = 0;
            file.read(numVertices);
            file.read(PerVertexFlags);

            if(PerVertexFlags & PerVertexData::Position)
                numComponents++;
            if((PerVertexFlags & PerVertexData::Normal) && (loadWhat & Load::Normals))
                numComponents++;
            if((PerVertexFlags & PerVertexData::Tangent) && (loadWhat & Load::Tangents))
                numComponents++;
            if((PerVertexFlags & PerVertexData::Bitangent) && (loadWhat & Load::Bitangents))
                numComponents++;
            if(PerVertexFlags & PerVertexData::TexCoord0)
                numTexcoords++;
            if(PerVertexFlags & PerVertexData::TexCoord1)
                numTexcoords++;
            if(PerVertexFlags & PerVertexData::TexCoord2)
                numTexcoords++;
            if(PerVertexFlags & PerVertexData::TexCoord3)
                numTexcoords++;

            modelDataSize += allocationSize<float>(numVertices * 3) * numComponents;
            memstat.vertexData.size += allocationSize<float>(numVertices * 3) * numComponents;

            for(uint32 j = 0; j < numTexcoords; j++)
            {
                uint8 numUVComponents;
                file.read(numUVComponents);
                if(loadWhat & loadTexCoords[j])
                {
                    modelDataSize += allocationSize<float>(numVertices * 3) * numUVComponents;
                    memstat.vertexData.size += allocationSize<float>(numVertices * 3) * numUVComponents;
                }
            }

            uint32 numFaces;
            file.read(numFaces);
            if(loadWhat & Load::Meshes)
            {
                modelDataSize += numFaces * sizeof(FaceData);
                memstat.faceDataArray.size += numFaces * sizeof(FaceData);
            }
        }

        uint32 numNodeReferences = 0, numMeshReferences = 0, numTextureReferences = 0;
        file.read(numNodes);

        file.read(numNodeReferences);
        file.read(nodeNameMemory);
        nodeNameMemory += numNodes; // trailing \0 bytes
        file.read(numMeshReferences);
        file.read(numTextureReferences);

        if(nodeNameMemory % AlignmentHelper::__ALIGNMENT != 0)
            nodeNameMemory += AlignmentHelper::__ALIGNMENT - (nodeNameMemory % AlignmentHelper::__ALIGNMENT);

        if(loadWhat & Load::Nodes)
        {
            modelDataSize += numNodes * sizeof(NodeData);
            modelDataSize += numNodes * sizeof(NodeDrawData);
            memstat.nodeData = MemoryPool(numNodes * sizeof(NodeData)+numNodes * sizeof(NodeDrawData));

            modelDataSize += allocationSize<uint32>(numMeshReferences);
            memstat.meshReferenceMemory = MemoryPool(allocationSize<uint32>(1) * numMeshReferences);

            modelDataSize += numNodeReferences * sizeof(NodeData*);
            memstat.nodeReferenceMemory = MemoryPool(numNodeReferences * sizeof(NodeDrawData*));

            modelDataSize += nodeNameMemory;
            memstat.nodeNameMemory = MemoryPool(nodeNameMemory);
        }
        if(loadWhat & Load::Materials)
        {
            modelDataSize += numTextureReferences * sizeof(TextureReference);
            memstat.textureReferenceMemory = MemoryPool(numTextureReferences * sizeof(TextureReference));
        }

        file.endReadChunk();

        m_pModelDataAllocator = GEP_NEW(m_pAllocator, StackAllocator)(true, modelDataSize, m_pAllocator);
        m_pStartMarker = m_pModelDataAllocator->getMarker();
    }

    // Pre-allocate stuff
    {
        if(loadWhat & Load::Nodes)
        {
            memstat.nodeData += allocationSize<NodeDrawData>(numNodes);
            m_nodes = GEP_NEW_ARRAY(m_pModelDataAllocator, NodeDrawData, numNodes);
        }
    }

    // Load textures
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "textures")
        {
            std::ostringstream msg;
            msg << "Expected 'textures' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }
        if(loadWhat & Load::Materials)
        {
            uint32 numTextures = 0;
            file.read(numTextures);

            if(numTextures > 0)
            {
                memstat.texturePathReferencesMemory += allocationSize<const char*>(numTextures);
                m_modelData.textures = GEP_NEW_ARRAY(m_pModelDataAllocator, const char*, numTextures);

                memstat.texturePathMemory += allocationSize<char>(texturePathMemory);
                char* textureNames = (char*)m_pModelDataAllocator->allocateMemory(texturePathMemory);
                size_t curNamePos = 0;
                for(auto& texture : m_modelData.textures)
                {
                    uint32 len;
                    file.read(len);
                    file.readArray(ArrayPtr<char>(textureNames + curNamePos, len));
                    texture = textureNames + curNamePos;
                    curNamePos += len;
                    textureNames[curNamePos++] = '\0';
                }
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    // Read Materials
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "materials")
        {
            std::ostringstream msg;
            msg << "Expected 'materials' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        if(loadWhat & Load::Materials)
        {
            uint32 numMaterials = 0;
            file.read(numMaterials);

            if(numMaterials > 0)
            {
                memstat.materialData += allocationSize<MaterialData>(numMaterials);
                m_modelData.materials = GEP_NEW_ARRAY(m_pModelDataAllocator, MaterialData, numMaterials);

                ArrayPtr<char> materialNames;
                size_t curNamePos = 0;
                if(file.getFileVersion() >= ModelFormatVersion::Version2)
                {
                    memstat.materialNameMemory += allocationSize<char>(materialNameMemory);
                    materialNames = GEP_NEW_ARRAY(m_pModelDataAllocator, char, materialNameMemory);
                }

                for(auto& material : m_modelData.materials)
                {
                    file.startReadChunk();
                    if(file.getCurrentChunkName() != "mat")
                    {
                        std::ostringstream msg;
                        msg << "Expected 'mat' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
                        throw LoadingError(msg.str());
                    }

                    //read material name
                    if(file.getFileVersion() >= ModelFormatVersion::Version2)
                    {
                        uint32 len = 0;
                        file.read(len);
                        auto data = materialNames(curNamePos, curNamePos+len);
                        curNamePos += len;
                        file.readArray(data);
                        materialNames[curNamePos++] = '\0';
                        material.name = data.getPtr();
                    }

                    uint32 numTextures = 0;
                    file.read(numTextures);
                    memstat.textureReferenceMemory += allocationSize<TextureReference>(numTextures);
                    material.textures = GEP_NEW_ARRAY(m_pModelDataAllocator, TextureReference, numTextures);
                    for(auto& texture : material.textures)
                    {
                        uint32 textureIndex;
                        file.read(textureIndex);
                        texture.file = m_modelData.textures[textureIndex];

                        uint8 semantic = (uint8)TextureType::UNKNOWN;
                        file.read<uint8>(semantic);
                        texture.semantic = (TextureType)semantic;
                    }

                    file.endReadChunk();
                }
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    // Read Bones
    if (file.getFileVersion() >= ModelFormatVersion::Version3)
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "bones")
        {
            std::ostringstream msg;
            msg << "Expected 'bones' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        if (loadWhat & Load::Bones)
        {
            uint32 numBones = 0;
            file.read(numBones);
            memstat.boneDataArray += allocationSize<BoneNode>(numBones);
            m_modelData.bones = GEP_NEW_ARRAY(m_pModelDataAllocator, BoneNode, numBones);

            for (uint32 boneIndex = 0; boneIndex < numBones; ++boneIndex)
            {
                auto& boneNode = m_modelData.bones[boneIndex];

                // Get offset matrix
                file.read(boneNode.offsetMatrix);

                // Get node id/index
                uint32 nodeIndex = std::numeric_limits<uint32>::max();
                file.read(nodeIndex);

                boneNode.node = &m_nodes[nodeIndex];
                boneNode.node->bone = &boneNode;
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    // Read Meshes
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "meshes")
        {
            std::ostringstream msg;
            msg << "Expected 'meshes' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        if(loadWhat & Load::Meshes)
        {
            uint32 numMeshes;
            file.read(numMeshes);
            memstat.meshDataArray += allocationSize<MeshData>(numMeshes);
            m_modelData.meshes = GEP_NEW_ARRAY(m_pModelDataAllocator, MeshData, numMeshes);

            for(auto& mesh : m_modelData.meshes)
            {
                file.startReadChunk();
                if(file.getCurrentChunkName() != "mesh")
                {
                    std::ostringstream msg;
                    msg << "Expected 'mesh' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
                }

                uint32 materialIndex = 0;
                file.read(mesh.materialIndex);

                vec3 minBounds, maxBounds;
                file.readArray<float>(minBounds.data);
                file.readArray<float>(maxBounds.data);
                maxBounds += vec3(0.01f, 0.01f, 0.01f);
                mesh.bbox = AABB(minBounds, maxBounds);

                uint32 numVertices = 0;
                file.read(numVertices);

                file.startReadChunk();
                if(file.getCurrentChunkName() != "vertices")
                {
                    std::ostringstream msg;
                    msg << "Expected 'vertices' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
                    throw LoadingError(msg.str());
                }
                memstat.vertexData += allocationSize<vec3>(numVertices);
                mesh.vertices = GEP_NEW_ARRAY(m_pModelDataAllocator, vec3, numVertices);

                static_assert(sizeof(vec3) == 3 * sizeof(float), "The following read call assumes that a vec3 is 3 floats big");
                file.readArray(ArrayPtr<float>(mesh.vertices[0].data, numVertices * 3));
                file.endReadChunk();

                {
                    file.startReadChunk();
                    if(file.getCurrentChunkName() == "normals")
                    {
                        if(loadWhat & Load::Normals)
                        {
                            memstat.vertexData += allocationSize<vec3>(numVertices);
                            mesh.normals = GEP_NEW_ARRAY(m_pModelDataAllocator, vec3, numVertices);
                            for(auto& normal : mesh.normals)
                            {
                                normal.x = readCompressedFloat(file);
                                normal.y = readCompressedFloat(file);
                                normal.z = readCompressedFloat(file);
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "tangents")
                    {
                        if(loadWhat & Load::Tangents)
                        {
                            memstat.vertexData += allocationSize<vec3>(numVertices);
                            mesh.tangents = GEP_NEW_ARRAY(m_pModelDataAllocator, vec3, numVertices);
                            for(auto& tangent : mesh.tangents)
                            {
                                tangent.x = readCompressedFloat(file);
                                tangent.y = readCompressedFloat(file);
                                tangent.z = readCompressedFloat(file);
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "bitangents")
                    {
                        if(loadWhat & Load::Bitangents)
                        {
                            memstat.vertexData += allocationSize<vec3>(numVertices);
                            mesh.bitangents = GEP_NEW_ARRAY(m_pModelDataAllocator, vec3, numVertices);
                            for(auto& bitangent : mesh.bitangents)
                            {
                                bitangent.x = readCompressedFloat(file);
                                bitangent.y = readCompressedFloat(file);
                                bitangent.z = readCompressedFloat(file);
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "texcoords")
                    {
                        if((loadWhat & Load::TexCoords0) || (loadWhat & Load::TexCoords1) ||
                            (loadWhat & Load::TexCoords2) || (loadWhat & Load::TexCoords3))
                        {
                            uint8 numTexCoords;
                            file.read(numTexCoords);
                            for(uint8 i=0; i<numTexCoords; i++)
                            {
                                uint8 numUVComponents;
                                file.read(numUVComponents);
                                if(numUVComponents != 2)
                                {
                                    std::ostringstream msg;
                                    msg << "Currently only 2 component texture coordinates are supported got " << numUVComponents << " components";
                                    throw LoadingError(msg.str());
                                }
                                if(loadWhat & loadTexCoords[i])
                                {
                                    memstat.vertexData += allocationSize<vec2>(numVertices);
                                    mesh.texcoords[i] = GEP_NEW_ARRAY(m_pModelDataAllocator, vec2, numVertices);
                                    static_assert(sizeof(vec2) == 2 * sizeof(float), "the following read call assumes that a vec2 is twice the size of a float");
                                    file.readArray(ArrayPtr<float>((float*)mesh.texcoords[i].getPtr(), numVertices * numUVComponents));
                                }
                                else
                                {
                                    //skip the texcoord data
                                    file.skipRead(sizeof(float) * numUVComponents * numVertices);
                                }
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if (file.getFileVersion() >= ModelFormatVersion::Version3 && file.getCurrentChunkName() == "bones")
                    {
                        if (loadWhat & Load::Bones)
                        {
                            // Read bone infos
                            uint32 numBoneInfos = 0;
                            file.read(numBoneInfos);

                            if (numVertices != numBoneInfos)
                            {
                                throw LoadingError("Wrong number of bone infos! There must be exactly as many bone infos as there are vertices.");
                            }
                            
                            memstat.boneDataArray += allocationSize<BoneInfo>(numBoneInfos);
                            mesh.boneInfos = GEP_NEW_ARRAY(m_pModelDataAllocator, BoneInfo, numBoneInfos);

                            static_assert(sizeof(BoneInfo) == sizeof(uint32) * 4 + sizeof(float) * 4,
                                "The size of the internal BoneInfo struct has changed."
                                "You should check that the code here is still correct.");

                            // Needed because the BoneInfo used by the engine is not the same size as the BoneInfo in the file.
                            struct BoneInfoInFile
                            {
                                uint16 boneIds[BoneInfo::NUM_SUPPORTED_BONES];
                                float weights[BoneInfo::NUM_SUPPORTED_BONES];
                            };

                            // Read all bone infos
                            for (size_t boneIndex = 0; boneIndex < numBoneInfos; ++boneIndex)
                            {
                                BoneInfoInFile boneInfoIntermediate;
                                file.read(boneInfoIntermediate);
                                for (size_t vertexBoneIndex = 0; vertexBoneIndex < BoneInfo::NUM_SUPPORTED_BONES; ++vertexBoneIndex)
                                {
                                    mesh.boneInfos[boneIndex].boneIds[vertexBoneIndex] = boneInfoIntermediate.boneIds[vertexBoneIndex];
                                    mesh.boneInfos[boneIndex].weights[vertexBoneIndex] = boneInfoIntermediate.weights[vertexBoneIndex];
                                }
                            }

                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "faces")
                    {
                        uint32 numFaces = 0;
                        file.read(numFaces);
                        memstat.faceDataArray += allocationSize<FaceData>(numFaces);
                        mesh.faces = GEP_NEW_ARRAY(m_pModelDataAllocator, FaceData, numFaces);
                        GEP_ASSERT(mesh.faces.getPtr() != nullptr);
                        if(numVertices > std::numeric_limits<uint16>::max())
                        {
                            static_assert(sizeof(FaceData) == sizeof(uint32) * 3, "the following read call assumes that FaceData is 3 uint32s wide");
                            file.readArray(ArrayPtr<uint32>((uint32*)mesh.faces.getPtr(), numFaces*3));
                        }
                        else
                        {
                            uint16 data = 0;
                            for(auto& face : mesh.faces)
                            {
                                file.read(data); face.indices[0] = (uint32)data;
                                file.read(data); face.indices[1] = (uint32)data;
                                file.read(data); face.indices[2] = (uint32)data;
                            }
                        }
                    }
                    else
                    {
                        std::ostringstream msg;
                        msg << "Unexepcted chunk '" << file.getCurrentChunkName() << "' in file '" << pFilename << "'";
                        throw LoadingError(msg.str());
                    }
                    file.endReadChunk();
                }

                file.endReadChunk();
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    // Read Nodes
    {
        file.startReadChunk();
        if(loadWhat & Load::Nodes)
        {
            {
                auto currentChunkName = file.getCurrentChunkName();
                if(currentChunkName != "nodes")
                {
                    std::ostringstream msg;
                    msg << "Expected 'nodes' chunk but got '" << file.getCurrentChunkName() << "' in file '" << pFilename << "'";
                    throw LoadingError(msg.str());
                }
            }
            // This is actually superfluous, the data is being read from the size info chunk
            uint32 numNodes;
            file.read(numNodes);

            memstat.nodeNameMemory += allocationSize<char>(nodeNameMemory);
            auto nodeNames = GEP_NEW_ARRAY(m_pModelDataAllocator, char, nodeNameMemory);

            size_t curNodeNamePos = 0;

            memstat.nodeData += allocationSize<NodeData>(numNodes);
            auto nodesData = GEP_NEW_ARRAY(m_pModelDataAllocator, NodeData, numNodes);

            size_t i = 0;
            for(auto& node : m_nodes)
            {
                node.data = &nodesData[i];
                uint32 nameLength = 0;
                file.read(nameLength);
                auto name = nodeNames(curNodeNamePos, curNodeNamePos + nameLength);
                file.readArray(name);
                curNodeNamePos += nameLength;
                nodeNames[curNodeNamePos++] = '\0';
                node.data->name = name.getPtr();

                m_nodeLookupByName[node.data->name] = &node;

                file.readArray<float>(node.transform.data);
                uint32 nodeParentIndex = 0;
                file.read(nodeParentIndex);
                if(nodeParentIndex == std::numeric_limits<uint32>::max())
                    node.parent = nullptr;
                else
                    node.parent = &m_nodes[nodeParentIndex];

                node.meshes = file.readAndAllocateArray<uint32, uint32>(m_pModelDataAllocator);
                memstat.meshReferenceMemory += allocationSize<uint32>((uint32)(node.meshes.length()));
                uint32 numChildren = 0;
                file.read(numChildren);
                if(numChildren > 0)
                {
                    memstat.nodeReferenceMemory += allocationSize<NodeDrawData*>(numChildren);
                    node.children = GEP_NEW_ARRAY(m_pModelDataAllocator, NodeDrawData*, numChildren);
                    for(auto& child : node.children)
                    {
                        uint32 childIndex = 0;
                        file.read(childIndex);
                        child = &m_nodes[childIndex];
                    }
                }

                i++;
            }
            m_modelData.rootNode = &m_nodes[0];

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    file.endReading();
    m_modelData.hasData = true;
}
void gep::ModelLoader::loadFromData(SmartPtr<ReferenceCounted> pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices)
{
    m_filename = "<from data>";

    auto numIndices = uint32(indices.length());
    auto numVertices = uint32(vertices.length());
    uint32 meshDataSize = 0;
    meshDataSize += allocationSize<MaterialData>(1);
    meshDataSize += allocationSize<MeshData>(1);
    meshDataSize += allocationSize<FaceData>(numIndices / 3);
    meshDataSize += allocationSize<float>(numVertices * 3);
    meshDataSize += allocationSize<NodeData>(1);
    meshDataSize += allocationSize<NodeDrawData>(1);
    meshDataSize += allocationSize<uint32>(1);
    meshDataSize += allocationSize<MeshData*>(1);

    m_pModelDataAllocator = GEP_NEW(m_pAllocator, StackAllocator)(true, meshDataSize, m_pAllocator);
    m_pStartMarker = m_pModelDataAllocator->getMarker();

    m_modelData.rootNode = GEP_NEW(m_pModelDataAllocator, NodeDrawData)();
    m_modelData.rootNode->meshes = GEP_NEW_ARRAY(m_pModelDataAllocator, uint32, 1);
    m_modelData.rootNode->meshes[0] = 0;
    m_modelData.rootNode->transform = mat4::identity().right2Left();
    m_modelData.rootNode->data = GEP_NEW(m_pModelDataAllocator, NodeData);
    m_modelData.rootNode->data->name = "root node";
    m_modelData.rootNode->parent = nullptr;
    auto mesh = GEP_NEW(m_pModelDataAllocator, MeshData)();
    m_modelData.meshes = ArrayPtr<MeshData>(mesh, 1);
    m_modelData.rootNode->data->meshData = GEP_NEW_ARRAY(m_pModelDataAllocator, MeshData*, 1);
    m_modelData.rootNode->data->meshData[0] = mesh;
    
    vec3 vmin(std::numeric_limits<float>::max());
    vec3 vmax(std::numeric_limits<float>::lowest());
    for(auto& v : vertices)
    {
        if(v.x < vmin.x) vmin.x = v.x;
        if(v.y < vmin.y) vmin.y = v.y;
        if(v.z < vmin.z) vmin.z = v.z;
        if(v.x > vmax.x) vmax.x = v.x;
        if(v.y > vmax.y) vmax.y = v.y;
        if(v.z > vmax.z) vmax.z = v.z;
    }
    mesh->bbox = AABB(vmin, vmax);
    mesh->faces = GEP_NEW_ARRAY(m_pModelDataAllocator, FaceData, indices.length() / 3);

    size_t i=0;
    for(auto& face : mesh->faces)
    {
        face.indices[0] = indices[i];
        face.indices[1] = indices[i + 1];
        face.indices[2] = indices[i + 2];
        i += 3;
    }

    mesh->materialIndex = 0;
    mesh->numFaces = numIndices / 3;
    mesh->vertices = GEP_NEW_ARRAY(m_pModelDataAllocator, vec3, vertices.length());
    
    i=0;
    for(auto& v : mesh->vertices)
    {
        v = vec3(vertices[i].x, vertices[i].y, vertices[i].z);
        i++;
    }

    m_modelData.materials = GEP_NEW_ARRAY(m_pModelDataAllocator, MaterialData, 1);
    m_modelData.materials[0].name = "dummy material";
    m_modelData.hasData = true;
    
}
