#include "stdafx.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"
#include "gepimpl/subsystems/renderer/renderer.h"
#include "gepimpl/subsystems/renderer/model.h"
#include "gepimpl/subsystems/renderer/texture2d.h"
#include "gepimpl/subsystems/renderer/vertexbuffer.h"
#include "gepimpl/subsystems/renderer/extractor.h"
#include "gep/exception.h"

void gep::ModelMaterial::setShader(ResourcePtr<Shader> pShader)
{
    m_pShader = pShader;
    m_modelMatrixConstant = ShaderConstant<mat4>("Model", pShader);
    m_viewMatrixConstant = ShaderConstant<mat4>("View", pShader);
    m_projectionMatrixConstant = ShaderConstant<mat4>("Projection", pShader);
    m_numBonesConstant = ShaderConstant<uint32>("NumBones", pShader);
    m_bonesArrayConstant = ShaderConstant<mat4>("Bones", pShader);
}

void gep::ModelMaterial::addTexture(ShaderConstant<Texture2D> constant, ResourcePtr<Texture2D> pTexture)
{
    m_textures.append(TextureSlot(constant, pTexture));
}

void gep::ModelMaterial::resetTextures()
{
    m_textures.resize(0);
}

gep::Model::MeshDrawData::~MeshDrawData()
{
}

gep::Model::Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) :
    m_pVertexbuffer(nullptr),
    m_pDevice(pDevice),
    m_pDeviceContext(pContext),
    m_pLoader(nullptr),
    m_debugDrawingEnabled(false)//,
    //m_bones(nullptr, 0)
{
    GEP_ASSERT(pDevice != nullptr);
    GEP_ASSERT(pContext != nullptr);
}

gep::Model::~Model()
{
    DELETE_AND_NULL(m_pVertexbuffer);
}

void gep::Model::doFindMinMax(mat4 transformation, const ModelLoader::NodeDrawData* pNode, vec3& minOut, vec3& maxOut)
{
    GEP_ASSERT(pNode != nullptr, "pNode may not be null");
    transformation = transformation * pNode->transform;
    //writefln("pNode = %x",cast(void*)pNode);
    //Search min-max for all meshes inside
    for(auto meshIndex : pNode->meshes){
        const ModelLoader::MeshData& meshData = m_modelLoader.getModelData().meshes[meshIndex];
        vec3 min = transformation.transformPosition(meshData.bbox.getMin());
        vec3 max = transformation.transformPosition(meshData.bbox.getMax());

        //The transformation may swapped a axis, fix if neccesary
        if(min.x > max.x) std::swap(min.x,max.x);
        if(min.y > max.y) std::swap(min.y,max.y);
        if(min.z > max.z) std::swap(min.z,max.z);

        if(min.x < minOut.x) minOut.x = min.x;
        if(min.y < minOut.y) minOut.y = min.y;
        if(min.z < minOut.z) minOut.z = min.z;

        if(max.x > maxOut.x) maxOut.x = max.x;
        if(max.y > maxOut.y) maxOut.y = max.y;
        if(max.z > maxOut.z) maxOut.z = max.z;
    }

    //Follow tree
    for(auto& c : pNode->children){
        doFindMinMax(transformation, c, minOut, maxOut);
    }
}

void gep::Model::drawHelper(ID3D11DeviceContext* pContext, mat4 transformation, ArrayPtr<mat4> bones, const ModelLoader::NodeDrawData* pNode, mat4& view, mat4& projection)
{
    GEP_ASSERT(pNode != nullptr,"pNode may not be null");
    transformation = transformation * pNode->transform;

    //Draw meshes
    for(auto& meshIndex : pNode->meshes){
        MeshDrawData& drawData = m_meshDrawData[meshIndex];
        ModelMaterial& material = m_materials[drawData.materialIndex];

        drawData.vertexbuffer->use(pContext);
        material.getModelMatrixConstant().set(transformation);

        for(auto& slot : material.getTextures())
        {
            slot.constant.set(slot.texture);
        }

        if(material.getShader().get() != m_pLastShader.get())
        {
            material.getViewMatrixConstant().set(view);
            material.getProjectionMatrixConstant().set(projection);
            material.getNumBonesConstant().set(uint32(bones.length()));
            material.getBonesArrayConstant().setArray(bones);
            m_pLastShader = material.getShader();
        }
        material.getShader()->use(pContext, drawData.vertexbuffer);
        drawData.vertexbuffer->draw(pContext, drawData.startIndex, drawData.numIndices);
    }

    for(auto child : pNode->children)
    {
        drawHelper(pContext, transformation, bones, child, view, projection);
    }
}


void gep::Model::draw(const mat4& modelMatrix, ArrayPtr<mat4> bones, mat4& viewMatrix, mat4& projectionMatrix, ID3D11DeviceContext* pContext)
{
    GEP_ASSERT(m_meshDrawData.length() > 0, "GenerateMeshes has not been called on this model");
    mat4 transform = modelMatrix; //mat4::identity().right2Left();
    drawHelper(pContext, transform, bones, m_modelLoader.getModelData().rootNode, viewMatrix, projectionMatrix);
    m_pLastShader = ResourcePtr<Shader>();
}

void gep::Model::loadFile(const char* filename)
{
    m_modelLoader.loadFile(filename, ModelLoader::Load::Everything);
    m_materials.resize(getMaterialInfo().length());
}
void gep::Model::loadFromData(SmartPtr<ReferenceCounted> pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices)
{
    m_modelLoader.loadFromData(pDataHolder, vertices, indices);
    m_materials.resize(1);
}

void gep::Model::generateMeshes()
{
    // Check if the model has been loaded correctly
    GEP_ASSERT(m_modelLoader.getModelData().hasData, "No file has been loaded yet");
#ifdef _DEBUG
    {
        int i=0;
        for(auto& mat : m_materials){
            if(mat.getShader() == nullptr)
            {
                GEP_ASSERT(false, "no shader has been set for material", i);
            }
            i++;
        }
    }
#endif

    //Generate Meshes
    m_meshDrawData.resize(m_modelLoader.getModelData().meshes.length());
    size_t i = 0;
    for(auto& mesh : m_modelLoader.getModelData().meshes)
    {

        // Generate the array of attributes we can use
        DynamicArray<Vertexbuffer::DataChannel> neededChannels;
        neededChannels.reserve(5);

        neededChannels.append(Vertexbuffer::DataChannel::POSITION);
        if(mesh.normals.length() > 0)
        {
            neededChannels.append(Vertexbuffer::DataChannel::NORMAL);
        }
        if(mesh.tangents.length() > 0)
        {
            neededChannels.append(Vertexbuffer::DataChannel::TANGENT);
        }
        if(mesh.bitangents.length() > 0)
        {
            neededChannels.append(Vertexbuffer::DataChannel::BINORMAL);
        }
        if(mesh.texcoords[0].length() > 0)
        {
            neededChannels.append(Vertexbuffer::DataChannel::TEXCOORD0);
        }
        if(mesh.boneInfos.length() > 0)
        {
            neededChannels.append(Vertexbuffer::DataChannel::BONE_INDICES);
            neededChannels.append(Vertexbuffer::DataChannel::BONE_WEIGHTS);
        }

        //Lets check if the mesh that we are loading has all the requested data channels
        int VertexSize = 0;
        for( auto channel : neededChannels)
        {
            switch(channel){
            case Vertexbuffer::DataChannel::NORMAL:
                if(mesh.normals.length() == 0){
                    std::ostringstream msg;
                    msg << "Error loading file '" << m_modelLoader.getFilename() << "' normal requested but not in mesh number " << i;
                    throw LoadingError(msg.str());
                }
                break;
            case Vertexbuffer::DataChannel::BINORMAL:
            case Vertexbuffer::DataChannel::TANGENT:
                if(mesh.tangents.length() == 0 || mesh.bitangents.length() == 0){
                    std::ostringstream msg;
                    msg << "Error loading file '" << m_modelLoader.getFilename() << "' tangent and binormal requested but not in mesh number " << i;
                    throw LoadingError(msg.str());
                }
                break;
            case Vertexbuffer::DataChannel::TEXCOORD0:
                if(mesh.texcoords[(uint32)channel - (uint32)(Vertexbuffer::DataChannel::TEXCOORD0)].length() == 0){
                    std::ostringstream msg;
                    msg << "Error loading file '" << m_modelLoader.getFilename() << "' texcoord " << ((uint32)channel - (uint32)(Vertexbuffer::DataChannel::TEXCOORD0))
                        << " requested but not found in mesh number " << i;
                    throw LoadingError(msg.str());
                }
                break;
            case Vertexbuffer::DataChannel::POSITION:
                if(mesh.vertices.length() == 0){
                    std::ostringstream msg;
                    msg << "Error loading file '" << m_modelLoader.getFilename() << "' position requested but not in mesh number " << i;
                    throw LoadingError(msg.str());
                }
                break;
            case Vertexbuffer::DataChannel::BONE_INDICES:
            case Vertexbuffer::DataChannel::BONE_WEIGHTS:
                if(mesh.boneInfos.length() == 0){
                    std::ostringstream msg;
                    msg << "Error loading file '" << m_modelLoader.getFilename() << "' bones requested but there are no bones in mesh number " << i;
                    throw LoadingError(msg.str());
                }
                break;
            default:
                {
                    std::ostringstream msg;
                    msg << "Error loading '" << m_modelLoader.getFilename() << "' request of not supported data channel (not implemented yet)";
                    throw LoadingError(msg.str());
                }
            }
        }

        //Insert the data into the vertex buffer
        Vertexbuffer* vb = m_pVertexbuffer;

        if(vb == nullptr){
            vb = new Vertexbuffer(m_pDevice, neededChannels.toArray(), Vertexbuffer::Primitive::Triangle, Vertexbuffer::Usage::Static);
            m_pVertexbuffer = vb;
        }

        m_meshDrawData[i].vertexbuffer = vb;
        m_meshDrawData[i].startIndex = (uint32)vb->getIndices().length();
        m_meshDrawData[i].numIndices = (uint32)(mesh.faces.length() * 3);
        m_meshDrawData[i].materialIndex = mesh.materialIndex;

        size_t IndexOffset = vb->getCurrentNumVertices();

        for(size_t j=0; j<mesh.vertices.length(); j++)
        {
            for(auto channel : neededChannels)
            {
                uint32 dataChannelSize = Vertexbuffer::getDataChannelSize(channel);
                switch(channel){
                case Vertexbuffer::DataChannel::POSITION:
                    GEP_ASSERT(dataChannelSize <= 3 && dataChannelSize > 0);
                    vb->getData().append(ArrayPtr<float>((float*)mesh.vertices[j].data, dataChannelSize));
                    break;
                case Vertexbuffer::DataChannel::NORMAL:
                    GEP_ASSERT(dataChannelSize <= 3 && dataChannelSize > 0);
                    vb->getData().append(ArrayPtr<float>((float*)mesh.normals[j].data, dataChannelSize));
                    break;
                case Vertexbuffer::DataChannel::BINORMAL:
                    GEP_ASSERT(dataChannelSize <= 3 && dataChannelSize > 0);
                    vb->getData().append(ArrayPtr<float>((float*)mesh.bitangents[j].data, dataChannelSize));
                    break;
                case Vertexbuffer::DataChannel::TANGENT:
                    GEP_ASSERT(dataChannelSize <= 3 && dataChannelSize > 0);
                    vb->getData().append(ArrayPtr<float>((float*)mesh.tangents[j].data, dataChannelSize));
                    break;
                case Vertexbuffer::DataChannel::TEXCOORD0:
                    {
                        int index = (int)channel - (int)Vertexbuffer::DataChannel::TEXCOORD0;
                        GEP_ASSERT(index >= 0 && index < 4);
                        GEP_ASSERT(dataChannelSize == 2); //currently only 2 component uv channels supported
                        GEP_ASSERT(mesh.texcoords[index].length() > 0);
                        vb->getData().append(ArrayPtr<float>((float*)mesh.texcoords[index][j].data, dataChannelSize));
                    }
                    break;
                case Vertexbuffer::DataChannel::BONE_INDICES:
                    {
                        auto boneIndexData = ArrayPtr<float>((float*)mesh.boneInfos[j].boneIds, dataChannelSize);
                        vb->getData().append(boneIndexData);
                    }
                    break;
                case Vertexbuffer::DataChannel::BONE_WEIGHTS:
                    {
                        auto boneWeightData = ArrayPtr<float>((float*)mesh.boneInfos[j].weights, dataChannelSize);
                        vb->getData().append(boneWeightData);
                    }
                    break;
                default:
                    GEP_ASSERT(0, "Trying to add unsupported data type to vertex buffer");
                }
            }
        }

        for(auto& face : mesh.faces){
            uint32 indices[3];
            for(int c=0;c<3;c++)
                indices[c] = (uint32)(face.indices[c] + IndexOffset);
            vb->getIndices().append(indices);
        }
        i++;
    }
}

void gep::Model::findMinMax(vec3& min, vec3& max){
    mat4 m = mat4::identity();
    min = vec3(std::numeric_limits<float>::max());
    max = vec3(-std::numeric_limits<float>::max());
    doFindMinMax(m, m_modelLoader.getModelData().rootNode, min, max);
}

void gep::Model::doPrintNodes(const ModelLoader::NodeDrawData* node, int depth){
    static const char* empty = "                                    ";
    g_globalManager.getLogging()->logMessage("%.*s%s", depth, empty, node->data->name);
    for(auto child : node->children)
        doPrintNodes(child, depth+1);
}

gep::IResource* gep::Model::getSuperResource()
{
    return nullptr;
}

const char* gep::Model::getResourceType()
{
    return "Model";
}

gep::IModelLoader* gep::Model::getLoader()
{
    return m_pLoader;
}

void gep::Model::setLoader(IResourceLoader* loader)
{
    GEP_ASSERT(loader != nullptr);
    #ifdef _DEBUG
    m_pLoader = dynamic_cast<IModelLoader*>(loader);
    GEP_ASSERT(m_pLoader != nullptr);
    #else
    m_pLoader = static_cast<IModelLoader*>(loader);
    #endif
}

bool gep::Model::isLoaded()
{
    return m_modelLoader.getModelData().hasData;
}

void gep::Model::unload()
{
    DELETE_AND_NULL(m_pVertexbuffer);
    m_meshDrawData.resize(0);
}

void gep::Model::finalize()
{
    GEP_ASSERT(m_pVertexbuffer == nullptr);
    generateMeshes();
    m_pVertexbuffer->upload(m_pDeviceContext);
}

gep::uint32 gep::Model::getFinalizeOptions()
{
    if(!m_modelLoader.getModelData().hasData)
        return ResourceFinalize::NotYet | ResourceFinalize::FromRenderer;
    return ResourceFinalize::FromRenderer;
}

void gep::Model::extract(IRendererExtractor& extractor, mat4 modelMatrix)
{
    auto& cmd = static_cast<RendererExtractor&>(extractor).makeCommand<CommandRenderModel>();
    cmd.model = this->makeResourcePtrFromThis<Model>();
    cmd.modelMatrix = modelMatrix;
    cmd.bones = GEP_NEW_ARRAY(extractor.getCurrentAllocator(), mat4, m_bones.length());
    cmd.bones.copyFrom(m_bones.toArray());
}

void gep::Model::setDebugDrawingEnabled(bool value)
{
    m_debugDrawingEnabled = value;
}

bool gep::Model::getDebugDrawingEnabled() const
{
    return m_debugDrawingEnabled;
}

void gep::Model::toggleDebugDrawing()
{
    setDebugDrawingEnabled(!getDebugDrawingEnabled());
}

void gep::Model::setBones(const ArrayPtr<mat4>& transformations)
{
    m_bones = transformations;

    int index = 0;
    for (auto& bone : m_modelLoader.getModelData().bones)
    {
        mat4 result = m_bones[index];

        float f[16] = {1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, -1, 0,
                       0, 0, 0, 1};
        mat4 toggleY(f);
        
        float f2[16] = {1, 0, 0, 0,
                        0, -1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1};
        mat4 toggleZ(f2);

        float f3[16] = {1, 0, 0, 0,
                        0, 0, 1, 0,
                        0, 1, 0, 0,
                        0, 0, 0, 1};
        mat4 toggleYZ(f3);

        //m_bones[index] = m_bones[index] * toggleYZ;
        //m_bones[index] = cummulatedOffsetMatrix(transformations, bone);
        m_bones[index] = /*toggleY * toggleZ */ m_bones[index] * bone.offsetMatrix;
        
        //m_bones[index] = m_bones[index] * mat4::rotationMatrixXYZ(vec3(-90, 0, 0));
        //m_bones[index] = m_bones[index]*cummulatedOffsetMatrix(trans, result);
        index++;
    }
}

// Use this method if you want to create the combined bone matrix by yourself.
// Caution: You have to use bone transformations in local space, instead of modelspace, so
// in model.cpp use "m_pPose->getBoneLocalSpace(index)"
gep::mat4 gep::Model::accumulatedOffsetMatrix(const ArrayPtr<mat4>& transformations, const gep::ModelLoader::BoneNode& bone)
{
    mat4 result = mat4::identity();
    auto parentNode = bone.node->parent;
    while(parentNode != nullptr)
    {
        if (parentNode->bone != nullptr)
        {
            auto parentBoneId = parentNode->bone - m_modelLoader.getModelData().bones.getPtr();
            result =  result * transformations[parentBoneId];
        }
        else
        {
            result = result * parentNode->transform;
        }
        parentNode = parentNode->parent;
    }
    return result;
}

gep::IResource* gep::IModelLoader::loadResource(IResource* pInPlace)
{
    return loadResource(dynamic_cast<Model*>(pInPlace));
}

const char* gep::IModelLoader::getResourceType()
{
    return "Model";
}

void gep::IModelLoader::release()
{
    delete this;
}

void gep::IModelLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<Model*>(pResource);
    GEP_ASSERT(res != nullptr, "given resource is not a Model");
    delete res;
}

gep::Model* gep::ModelDummyLoader::loadResource(Model* pInPlace)
{
    return pInPlace;
}

void gep::ModelDummyLoader::postLoad(ResourcePtr<IResource> pResource)
{

}

gep::ModelDummyLoader* gep::ModelDummyLoader::moveToHeap()
{
    return new ModelDummyLoader(*this);
}

const char* gep::ModelDummyLoader::getResourceId()
{
    return "<Dummy Model>";
}

gep::ModelFileLoader::ModelFileLoader(const char* filename) :
    m_filename(filename)
{
    m_pRenderer = static_cast<Renderer*>(g_globalManager.getRenderer());
}

gep::Model* gep::ModelFileLoader::loadResource(Model* pInPlace)
{
    Model* result = pInPlace;
    bool isInPlace = true;
    if(pInPlace == nullptr || pInPlace->isLoaded())
    {
        result = m_pRenderer->createModel();
        isInPlace = false;
    }
    try {
        result->loadFile(m_filename.c_str());
        return result;
    }
    catch(LoadingError& ex)
    {
        if(!isInPlace)
            deleteResource(result);
        g_globalManager.getLogging()->logError("Error loading model from file '%s':\n%s", m_filename.c_str(), ex.what());
    }
    return pInPlace;
}

void gep::ModelFileLoader::postLoad(ResourcePtr<IResource> pResource)
{
}

gep::ModelFileLoader* gep::ModelFileLoader::moveToHeap()
{
    return new ModelFileLoader(*this);
}

const char* gep::ModelFileLoader::getResourceId()
{
    return m_filename.c_str();
}

gep::ModelLoaderFromData::ModelLoaderFromData(SmartPtr<ReferenceCounted> dataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices, const char* resourceId) :
    m_vertices(vertices),
    m_indices(indices),
    m_pDataHolder(dataHolder),
    m_resourceID(resourceId)
    
{
    m_pRenderer = static_cast<Renderer*>(g_globalManager.getRenderer());
}


gep::Model* gep::ModelLoaderFromData::loadResource(Model* pInPlace)
{
    Model* result = pInPlace;
    bool isInPlace = true;
    if(pInPlace == nullptr || pInPlace->isLoaded())
    {
        result = m_pRenderer->createModel();
        isInPlace = false;
    }
    try {
        result->loadFromData(m_pDataHolder, m_vertices, m_indices);
        return result;
    }
    catch(LoadingError& ex)
    {
        if(!isInPlace)
            deleteResource(result);
        g_globalManager.getLogging()->logError("Error loading model from data %s: %s", m_resourceID.c_str(), ex.what());
    }
    return pInPlace;
}

void gep::Model::getBoneNames(DynamicArray<const char*>& names)
{
    const auto& bones = m_modelLoader.getModelData().bones;

    names.reserve(names.length() + bones.length());

    for(auto& bone : bones)
    {
        names.append(bone.node->data->name);
    }
}

void gep::ModelLoaderFromData::postLoad(ResourcePtr<IResource> pResource)
{

}

gep::ModelLoaderFromData* gep::ModelLoaderFromData::moveToHeap()
{
    return new ModelLoaderFromData(*this);
}

const char* gep::ModelLoaderFromData::getResourceId()
{
    return m_resourceID.c_str();
}

