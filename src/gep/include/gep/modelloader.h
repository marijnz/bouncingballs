#pragma once

#include "gep/types.h"
#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/vec4.h"
#include "gep/math3d/mat4.h"
#include "gep/math3d/aabb.h"
#include "gep/memory/allocators.h"
#include "gep/container/hashmap.h"

struct ID3D11Device;

namespace gep
{
    /// \brief supported texture types
    enum class TextureType : gep::uint8
    {
        DIFFUSE, /// diffuse texture
        AMBIENT, /// ambient texture
        DISPLACEMENT, /// displacement texture
        EMISSIVE, /// emissive texture
        HEIGHT, /// heightmap (normal map for collada)
        LIGHTMAP, /// lightmap
        NONE, ///none (should never happen)
        NORMALS, /// normal map
        OPACITY, /// opacity map
        REFLECTION, /// reflection map
        SHININESS, /// shininess texture
        SPECULAR, /// specular texture
        UNKNOWN /// unkown texture type
    };

    struct ModelFormatVersion
    {
        enum Enum {
            Version1 = 1, //Initial version
            Version2 = 2, //saving material names
            Version3 = 3, //Bones, baby!
        };
    };

    struct PerVertexData
    {
        enum Enum {
            Position   = 0x0001,
            Normal     = 0x0002,
            Tangent    = 0x0004,
            Bitangent  = 0x0008,
            TexCoord0  = 0x0010,
            TexCoord1  = 0x0020,
            TexCoord2  = 0x0040,
            TexCoord3  = 0x0080,
            BoneIds    = 0x0400,
            BoneWeights= 0x0800
        };
    };

    class ModelLoader
    {
    public:
        struct Load
        {
            enum Enum {
                Materials  = 0x0001,
                Meshes     = 0x0002,
                Normals    = 0x0004,
                Tangents   = 0x0008,
                Bitangents = 0x0010,
                TexCoords0 = 0x0020,
                TexCoords1 = 0x0040,
                TexCoords2 = 0x0080,
                TexCoords3 = 0x0100,
                Nodes      = 0x0200,
                Bones      = 0x0400,
                Everything = 0xFFFF
            };

        };
        struct BoneInfo;
        struct BoneNode;

        struct BoneData
        {
            const char* name;
            mat4 offsetMatrix;
        };


        struct FaceData
        {
            uint32 indices[3];
        };

        struct TextureReference
        {
            TextureType semantic;
            const char* file;
        };

        struct MaterialData
        {
            ArrayPtr<TextureReference> textures;
            const char* name;
        };

        struct MeshData
        {
            uint32 materialIndex;
            AABB bbox;
            uint32 numFaces;
            ArrayPtr<FaceData> faces;
            ArrayPtr<vec3> vertices;
            ArrayPtr<vec3> normals;
            ArrayPtr<vec3> tangents;
            ArrayPtr<vec3> bitangents;
            ArrayPtr<vec2> texcoords[4];
            ArrayPtr<BoneInfo> boneInfos;
            BoneNode* rootBone; // TODO: We need an array of all bones which influence this mesh
        };

        struct NodeData
        {
            const char* name;
            ArrayPtr<MeshData*> meshData;
        };

        struct NodeDrawData
        {
            mat4 transform;
            NodeDrawData* parent;
            ArrayPtr<NodeDrawData*> children;
            ArrayPtr<uint32> meshes;
            NodeData* data;
            BoneNode* bone;
        };



        struct BoneInfo
        {
            enum { NUM_SUPPORTED_BONES = 4 };
            uint32 boneIds[NUM_SUPPORTED_BONES];
            float weights[NUM_SUPPORTED_BONES];
        };

        struct BoneNode
        {
            NodeDrawData* node;
            mat4 offsetMatrix;
        };

        static_assert(
            sizeof(BoneInfo) == sizeof(uint32) * BoneInfo::NUM_SUPPORTED_BONES + sizeof(float) * BoneInfo::NUM_SUPPORTED_BONES,
            "Size of BoneInfo is wrong!");

        struct ModelData
        {
            ArrayPtr<const char*> textures;
            ArrayPtr<MaterialData> materials;
            ArrayPtr<MeshData> meshes;
            NodeDrawData* rootNode;
            ArrayPtr<BoneNode> bones;
            bool hasData;

            inline ModelData() : rootNode(nullptr), hasData(false) {}
        };

    private:

        StackAllocator* m_pModelDataAllocator;
        IAllocator* m_pAllocator;
        void * m_pStartMarker;
        ModelData m_modelData;
        std::string m_filename;

        Hashmap<const char*, NodeDrawData*, StringHashPolicy> m_nodeLookupByName;
        ArrayPtr<NodeDrawData> m_nodes;

        template <typename T>
        uint32 allocationSize(uint32 num)
        {
            uint32 size = (uint32)sizeof(T) * num;
            if(size % AlignmentHelper::__ALIGNMENT != 0)
            {
                size += AlignmentHelper::__ALIGNMENT - (size & (AlignmentHelper::__ALIGNMENT * 2 - 1));
            }
            GEP_ASSERT(size % AlignmentHelper::__ALIGNMENT == 0);
            return size;
        }

        /// Helper struct to prevent overflows within memory pools
        struct MemoryPool
        {
            uint32 size;
            uint32 cur;

            MemoryPool() : size(0), cur(0) {}

            inline MemoryPool(size_t size) : cur(0)
            {
                GEP_ASSERT(size <= std::numeric_limits<uint32>::max());
                this->size = (uint32)size;
            }

            inline void operator += (size_t val)
            {
                cur += (uint32)val;
                GEP_ASSERT(cur <= size, "Memory pool overflow");
            }
        };

        /// \brief struct that holds all memory pools
        struct MemoryStatistics
        {
            MemoryPool materialData;
            MemoryPool nodeNameMemory;
            MemoryPool texturePathMemory;
            MemoryPool materialNameMemory;
            MemoryPool texturePathReferencesMemory;
            MemoryPool meshDataArray;
            MemoryPool vertexData;
            MemoryPool faceDataArray;
            MemoryPool nodeData;
            MemoryPool meshReferenceMemory;
            MemoryPool nodeReferenceMemory;
            MemoryPool textureReferenceMemory;
            MemoryPool boneNameMemory;
            MemoryPool boneDataArray;
        };

    public:

        /**
        * provides read only access for the loaded data
        */
        inline const ModelData& getModelData() const
        {
            GEP_ASSERT(m_modelData.hasData, "no data has been loaded yet");
            return m_modelData;
        }

        inline const std::string& getFilename() const
        {
            return m_filename;
        }

        ModelLoader(IAllocator* pAllocator = nullptr);

        ~ModelLoader();

        /// \brief loads a model from a file
        /// \param pFilename
        ///   the filename to load the model from
        /// \param loadWhat
        ///   which data should be loaded. Combination of Load::Enum values
        void loadFile(const char* pFilename, uint32 loadWhat);
        void loadFromData(SmartPtr<ReferenceCounted> pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices);
    };
}
