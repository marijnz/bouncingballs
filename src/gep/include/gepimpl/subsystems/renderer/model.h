#pragma once

#include "gep/container/DynamicArray.h"
#include "gep/container/hashmap.h"
#include "gep/interfaces/resourceManager.h"
#include "gepimpl/subsystems/renderer/shader.h"
#include "gep/modelloader.h"
#include "gep/ReferenceCounting.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace gep
{
    // forward references
    class Texture2D;
    class Shader;
    class Model;
    class Renderer;

    /// \brief interface for loading a 2d texture
    class IModelLoader : public IResourceLoader
    {
    public:
        virtual Model* loadResource(Model* pInPlace) = 0;
        virtual IResource* loadResource(IResource* pInPlace) override;
        virtual const char* getResourceType() override;
        virtual void deleteResource(IResource* pResource) override;
        virtual void release() override;
    };

    /// \brief dummy model loader
    class ModelDummyLoader : public IModelLoader
    {
    public:
        virtual Model* loadResource(Model* pInPlace) override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual ModelDummyLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    /// \brief loads a Model from a file
    class ModelFileLoader : public IModelLoader
    {
    private:
        std::string m_filename;
        Renderer* m_pRenderer;

    public:
        ModelFileLoader(const char* filename);
        virtual Model* loadResource(Model* pInPlace) override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual ModelFileLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    /// \brief loads a Model from given data
    class ModelLoaderFromData : public IModelLoader
    {
    private:
        SmartPtr<ReferenceCounted> m_pDataHolder;
        ArrayPtr<vec4> m_vertices;
        ArrayPtr<uint32> m_indices;
        Renderer* m_pRenderer;
        std::string m_resourceID;

    public:
        ModelLoaderFromData(SmartPtr<ReferenceCounted> dataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices, const char* resourceId);

 
        virtual Model* loadResource(Model* pInPlace) override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual ModelLoaderFromData* moveToHeap() override;
        virtual const char* getResourceId() override;
    };
    /**
    * a Material for a subpart of a model
    */
    class ModelMaterial {
    private:
        struct TextureSlot
        {
            ShaderConstant<Texture2D> constant;
            ResourcePtr<Texture2D> texture;

            TextureSlot(){}
            TextureSlot(ShaderConstant<Texture2D> constant, ResourcePtr<Texture2D> texture)
            {
                this->constant = constant;
                this->texture = texture;
            }
        };

        DynamicArray<TextureSlot> m_textures;
        ResourcePtr<Shader> m_pShader;
        ShaderConstant<mat4> m_modelMatrixConstant;
        ShaderConstant<mat4> m_viewMatrixConstant;
        ShaderConstant<mat4> m_projectionMatrixConstant;
        ShaderConstant<uint32> m_numBonesConstant;
        ShaderConstant<mat4> m_bonesArrayConstant;

    public:
        //inline ModelMaterial(){}
        //inline ~ModelMaterial(){}
        /**
        * Sets the shader of a material
        * Params:
        *         pShader = the shader to use
        */
        void setShader(ResourcePtr<Shader> pShader);

        /**
        * Gets the shader of a material
        * Returns: the current shader in use by this material
        */
        inline ResourcePtr<Shader> getShader() {
            return m_pShader;
        }

        /**
        * Get textures
        * Returns: A map of the current textures in use
        */
        inline decltype(m_textures)& getTextures() {
            return m_textures;
        }

        inline ShaderConstant<mat4>& getModelMatrixConstant()
        {
            return m_modelMatrixConstant;
        }

        inline ShaderConstant<mat4>& getViewMatrixConstant()
        {
            return m_viewMatrixConstant;
        }

        inline ShaderConstant<mat4>& getProjectionMatrixConstant()
        {
            return m_projectionMatrixConstant;
        }

        inline ShaderConstant<uint32>& getNumBonesConstant()
        {
            return m_numBonesConstant;
        }

        inline ShaderConstant<mat4>& getBonesArrayConstant()
        {
            return m_bonesArrayConstant;
        }

        void addTexture(ShaderConstant<Texture2D> constant, ResourcePtr<Texture2D> pTexture);

        /**
        * Removes all Textures from a Material
        */
        void resetTextures();
    };

    class Model : public IModel
    {
    public:

        /**
        * Information about a texture needed by the model
        */
        struct ModelTextureInfo {
            std::string name; /// texture type as string
            std::string file; /// texture file path
            TextureType type; /// texture type
            int index; /// used texture channel
        };

    private:

        struct MeshDrawData
        {
            Vertexbuffer* vertexbuffer;
            uint32 startIndex, numIndices, materialIndex;

            ~MeshDrawData();
        };

        DynamicArray<ModelMaterial> m_materials;
        ModelLoader m_modelLoader;
        DynamicArray<MeshDrawData> m_meshDrawData;
        Vertexbuffer* m_pVertexbuffer;
        ID3D11Device* m_pDevice;
        ID3D11DeviceContext* m_pDeviceContext;
        IModelLoader* m_pLoader;
        ResourcePtr<Shader> m_pLastShader;
        DynamicArray<mat4> m_bones;

        bool m_debugDrawingEnabled;

        void drawHelper(ID3D11DeviceContext* pContext, mat4 transformation, ArrayPtr<mat4> bones, const ModelLoader::NodeDrawData* pNode, mat4& view, mat4& projection);
        void doFindMinMax(mat4 transformation, const ModelLoader::NodeDrawData* pNode, vec3& min, vec3& max);

        void doPrintNodes(const ModelLoader::NodeDrawData* node, int depth = 0);
        gep::mat4 accumulatedOffsetMatrix(const ArrayPtr<mat4>& transformations, const gep::ModelLoader::BoneNode& node);

    public:

        /**
        * consructor
        * Params:
        *        pManager = the vertex buffer manage to use by this model
        */
        Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

        ~Model();

        void setBones(const ArrayPtr<mat4>& transformations);
        
        bool hasBones() { return m_modelLoader.getModelData().bones.length() > 0; }
        
        /// \brief draws the model
        void draw(const mat4& modelMatrix, ArrayPtr<mat4> bones, mat4& viewMatrix, mat4& projectionMatrix, ID3D11DeviceContext* pContext);

        /// \brief loads the model from a file
        void loadFile(const char* filename);
        void loadFromData(SmartPtr<ReferenceCounted> pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices);
        /**
        * Generates the meshes
        */
        void generateMeshes();

        /**
        * gets the number of materials used
        */
        inline size_t getNumMaterials() const
        {
            return m_modelLoader.getModelData().materials.length();
        }

        /**
        * sets a material
        * Params:
        *        pSet = the set to use
        *        pNum = the number of the material
        *        pMaterial = the material
        */
        inline void setMaterial(uint32 num, ModelMaterial& pMaterial)
        {
            m_materials[num] = pMaterial;
        }

        /**
        * gets a material
        * Params:
        *        pSet = the set to use
        *        pNum = the number of the material
        * Returns: the material
        */
        inline ModelMaterial& getMaterial(uint32 pNum)
        {
            return m_materials[pNum];
        }

        /**
        * Gets the info about the materiales used in the loaded model file
        * Has to be called after the model file has been loaded
        * Returns: A array of MaterialInfo structs
        */
        inline const ArrayPtr<ModelLoader::MaterialData> getMaterialInfo() const
        {
            return m_modelLoader.getModelData().materials;
        }

        /**
        * Searches for the minimum and maximum coordinates of the model
        * Params:
        *        pMin = result minimum
        *        pMax = result maximum
        */
        void findMinMax(vec3& min, vec3& max);

        inline void printNodes()
        {
            doPrintNodes(m_modelLoader.getModelData().rootNode);
        }

        const std::string& getFilename() const {
            return m_modelLoader.getFilename();
        }

        inline ArrayPtr<mat4> getBones() { return m_bones.toArray(); }
        inline const ArrayPtr<mat4> getBones() const { return m_bones.toArray(); }
        virtual void getBoneNames(DynamicArray<const char*>& names) override;

        //IResource interface
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;
        virtual IModelLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual bool isLoaded() override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;

        //IModel interface
        virtual void extract(IRendererExtractor& extractor, mat4 modelMatrix) override;

        virtual void setDebugDrawingEnabled(bool value) override;
        virtual bool getDebugDrawingEnabled() const override;
        virtual void toggleDebugDrawing() override;
    };
}
