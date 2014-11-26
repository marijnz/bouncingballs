#include "stdafx.h"
#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/mat3.h"
#include "gep/math3d/mat4.h"
#include "gepimpl/subsystems/renderer/renderer.h"
#include "gepimpl/subsystems/renderer/shader.h"
#include "gepimpl/subsystems/renderer/vertexbuffer.h"
#include "gepimpl/subsystems/renderer/texture2d.h"
#include "gep/exception.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <stdint.h>
#include "d3dcompiler.h"
#include "d3d11shader.h"
#include "effects11/d3dx11effect.h"
#include "effects11/d3dxGlobal.h"
#include "effects11/effect.h"
#include "effects11/EffectLoad.h"

gep::IResource* gep::IShaderLoader::loadResource(IResource* ptr)
{
    return loadResource(dynamic_cast<Shader*>(ptr));
}

const char* gep::IShaderLoader::getResourceType()
{
    return "Shader";
}

void gep::IShaderLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<Shader*>(pResource);
    GEP_ASSERT(res != nullptr, "given resource is not a Shader");
    delete res;
}

void gep::IShaderLoader::release()
{
    delete this;
}

gep::ShaderFileLoader::ShaderFileLoader(const char* filename) :
    m_filename(filename),
    m_isRegistered(false)
{
    m_pRenderer = static_cast<Renderer*>(g_globalManager.getRenderer());
}

gep::ShaderFileLoader::~ShaderFileLoader()
{
    if(m_isRegistered)
    {
        g_globalManager.getResourceManager()->deregisterLoaderForReload(m_filename.c_str(), this);
    }
}

gep::Shader* gep::ShaderFileLoader::loadResource(Shader* pInPlace)
{
    Shader* result = pInPlace;
    bool isInPlace = true;
    if(pInPlace == nullptr || pInPlace->isLoaded())
    {
        result = m_pRenderer->createShader();
        isInPlace = false;
    }
    try {
        result->loadFromFX(m_filename.c_str());
        return result;
    }
    catch(LoadingError& ex)
    {
        if(!isInPlace)
            deleteResource(result);
        g_globalManager.getLogging()->logError("Error loading shader from file '%s':\n%s", m_filename.c_str(), ex.what());
        return nullptr;
    }
    return pInPlace;
}

void gep::ShaderFileLoader::postLoad(ResourcePtr<IResource> pResource)
{
    if(!m_isRegistered)
    {
        m_isRegistered = true;
        g_globalManager.getResourceManager()->registerLoaderForReload(m_filename.c_str(), this, pResource);
    }
}

gep::ShaderFileLoader* gep::ShaderFileLoader::moveToHeap()
{
    return new ShaderFileLoader(*this);
}

const char* gep::ShaderFileLoader::getResourceId()
{
    return m_filename.c_str();
}

gep::Shader::Shader(ID3D11Device* pDevice)
    : m_pEffect(nullptr),
    m_pLoader(nullptr),
    m_pDevice(pDevice),
    m_pTechnique(nullptr),
    m_pByteCode(nullptr)
{
}

gep::Shader::~Shader()
{
    Shader::unload();
}

gep::IResource* gep::Shader::getSuperResource()
{
    return nullptr;
}

const char* gep::Shader::getResourceType()
{
    return "Shader";
}

gep::IShaderLoader* gep::Shader::getLoader()
{
    return m_pLoader;
}

void gep::Shader::setLoader(IResourceLoader* pLoader)
{
    auto loader = dynamic_cast<IShaderLoader*>(pLoader);
    GEP_ASSERT(loader != nullptr, "the given loader is not a shader loader");
    m_pLoader = loader;
}

bool gep::Shader::isLoaded()
{
    return m_pEffect != nullptr || m_pByteCode != nullptr;
}

void gep::Shader::unload()
{
    for(auto inputLayout : m_inputLayouts.values())
    {
        inputLayout->Release();
    }
    m_inputLayouts.clear();
    if(m_pEffect != nullptr)
    {
        m_pEffect->Release();
        m_pEffect = nullptr;
    }
}

void gep::Shader::finalize()
{
    GEP_ASSERT(m_pByteCode != nullptr);
    if(m_pEffect != nullptr)
        m_pEffect->Release();
    HRESULT hr = D3DX11CreateEffectFromMemory(m_pByteCode->GetBufferPointer(), m_pByteCode->GetBufferSize(), 0, m_pDevice, &m_pEffect);
    m_pByteCode->Release(); m_pByteCode = nullptr;
    if( FAILED(hr) )
    {
        throw LoadingError("Failed to create effect");
    }
    m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
    m_pPass = m_pTechnique->GetPassByIndex(0);
}

gep::uint32 gep::Shader::getFinalizeOptions()
{
    if(m_pByteCode == nullptr)
        return ResourceFinalize::FromRenderer | ResourceFinalize::NotYet;
    return ResourceFinalize::FromRenderer;
}

void gep::Shader::loadFromFX(const char* filename)
{
    ID3D10Blob* pByteCode = nullptr;
    ID3D10Blob* pErrors = nullptr;
    HRESULT hr;
    hr = D3DX11CompileFromFileA(filename, nullptr, nullptr, nullptr, "fx_5_0",
        //D3D10_SHADER_OPTIMIZATION_LEVEL2 | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR,
        D3D10_SHADER_DEBUG | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR,
        0, nullptr, &pByteCode, &pErrors, nullptr);
    if( FAILED(hr) )
    {
        if(hr == D3D11_ERROR_FILE_NOT_FOUND)
        {
            if(pErrors) pErrors->Release();
            if(pByteCode) pByteCode->Release();

            std::ostringstream msg;
            msg << "The file '" << filename << "' could not be opened";

            throw LoadingError(msg.str());
        }
        auto ex = LoadingError(std::string((pErrors != nullptr) ? (char*)pErrors->GetBufferPointer() : nullptr,
                                           (pErrors != nullptr) ? pErrors->GetBufferSize() : 0));
        if(pErrors) pErrors->Release();
        if(pByteCode) pByteCode->Release();
        throw ex;
    }
    if(m_pByteCode != nullptr)
    {
        m_pByteCode->Release();
    }
    m_pByteCode = pByteCode;
    if(pErrors) pErrors->Release();
}

void gep::Shader::use(ID3D11DeviceContext* pContext, Vertexbuffer* pVertexbuffer)
{
    GEP_ASSERT(m_pEffect != nullptr, "not finalized yet");
    uint32 layoutHash = pVertexbuffer->getLayoutHash();
    ID3D11InputLayout* pInputLayout = nullptr;
    if(m_inputLayouts.tryGet(layoutHash, pInputLayout) == FAILURE)
    {
        D3DX11_PASS_DESC passDesc;
        m_pPass->GetDesc(&passDesc);
        HRESULT hr = m_pDevice->CreateInputLayout(pVertexbuffer->getLayout().getPtr(), (UINT)pVertexbuffer->getLayout().length(), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayout);
        GEP_ASSERT(SUCCEEDED(hr), "Creating input layout failed");
        m_inputLayouts[layoutHash] = pInputLayout;
    }
    GEP_ASSERT(pInputLayout != nullptr);
    m_pPass->Apply(0, pContext);
    pContext->IASetInputLayout(pInputLayout);
}

gep::ShaderConstantBase::ShaderConstantBase(const char* name, ResourcePtr<Shader> pShader)
    : m_name(name),
    m_pShader(pShader),
    m_pLastValidShader(nullptr),
    m_isValid(false)
{
}

gep::ShaderConstantBase::ShaderConstantBase()
    :
    m_pLastValidShader(nullptr),
    m_isValid(false)
{
}

gep::ShaderConstant<gep::vec2>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}

gep::ShaderConstant<gep::vec3>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}

gep::ShaderConstant<gep::Color>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}

gep::ShaderConstant<gep::mat3>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}


gep::ShaderConstant<gep::mat4>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}

gep::ShaderConstant<gep::Texture2D>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}

gep::ShaderConstant<gep::uint32>::ShaderConstant(const char* name, ResourcePtr<Shader> pShader)
    : ShaderConstantBase(name, pShader)
{
    checkUpToDate();
}


void gep::ShaderConstant<gep::vec2>::set(const vec2& value)
{
    checkUpToDate();
    if(m_isValid)
    {
        float values[4] = { value.x, value.y, 0.0f, 0.0f };
        m_pVar->SetFloatVector(values);
    }
}

void gep::ShaderConstant<gep::vec3>::set(const vec3& value)
{
    checkUpToDate();
    if(m_isValid)
    {
        float values[4] = { value.x, value.y, value.z, 0.0f };
        m_pVar->SetFloatVector(values);
    }
}

void gep::ShaderConstant<gep::Color>::set(Color& value)
{
    checkUpToDate();
    if(m_isValid)
    {
        m_pVar->SetFloatVector(value.data);
    }
}

void gep::ShaderConstant<gep::mat3>::set(const mat3& value)
{
    checkUpToDate();
    if(m_isValid)
    {
        mat4 temp = mat4::identity();
        temp.setRotationPart(value);
        m_pVar->SetMatrix(temp.data);
    }
}

void gep::ShaderConstant<gep::mat4>::set(mat4& value)
{
    checkUpToDate();
    if(m_isValid)
    {
        m_pVar->SetMatrix(value.data);
    }
}

void gep::ShaderConstant<gep::mat4>::setArray(ArrayPtr<mat4> arr)
{
    checkUpToDate();
    if(m_isValid)
    {
        m_pVar->SetMatrixArray(arr.getPtr()->data, 0, UINT(arr.length()));
    }
}

void gep::ShaderConstant<gep::Texture2D>::set(ResourcePtr<Texture2D> value)
{
    checkUpToDate();
    if(m_isValid)
    {
        m_pVar->SetResource(value->getResourceView());
    }
}

void gep::ShaderConstant<gep::uint32>::set(uint32 value)
{
    checkUpToDate();
    if(m_isValid)
    {
        m_pVar->SetInt(int(value));
    }
}


void gep::ShaderConstant<gep::vec2>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsVector();
        m_isValid = true;
    }
}

void gep::ShaderConstant<gep::vec3>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsVector();
        m_isValid = true;
    }
}

void gep::ShaderConstant<gep::Color>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsVector();
        m_isValid = true;
    }
}

void gep::ShaderConstant<gep::mat3>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsMatrix();
        m_isValid = true;
    }
}

void gep::ShaderConstant<gep::mat4>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsMatrix();
        m_isValid = true;
    }
}

void gep::ShaderConstant<gep::Texture2D>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsShaderResource();
        m_isValid = true;
    }
}

void gep::ShaderConstant<gep::uint32>::checkUpToDate()
{
    if(m_pShader != m_pLastValidShader && m_pShader->getEffect() != nullptr)
    {
        m_pLastValidShader = m_pShader;
        m_pVar = m_pLastValidShader->getEffect()->GetVariableByName(m_name.c_str())->AsScalar();
        m_isValid = true;
    }
}
