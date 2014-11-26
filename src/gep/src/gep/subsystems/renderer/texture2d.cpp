#include "stdafx.h"
#include "gepimpl/subsystems/renderer/texture2d.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"
#include "gepimpl/subsystems/renderer/renderer.h"
#include "gepimpl/subsystems/renderer/ddsLoader.h"

gep::IResource* gep::ITexture2DLoader::loadResource(IResource* pInPlace)
{
    return loadResource(dynamic_cast<Texture2D*>(pInPlace));
}

const char* gep::ITexture2DLoader::getResourceType()
{
    return "Texture2D";
}

void gep::ITexture2DLoader::release()
{
    delete this;
}

void gep::ITexture2DLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<Texture2D*>(pResource);
    GEP_ASSERT(res != nullptr, "given resource is not a Texture2D");
    delete res;
}

gep::Texture2D* gep::DummyTexture2DLoader::loadResource(Texture2D* pInPlace)
{
    return pInPlace;
}

void gep::DummyTexture2DLoader::postLoad(ResourcePtr<IResource> pResource)
{
}

gep::DummyTexture2DLoader* gep::DummyTexture2DLoader::moveToHeap()
{
    return new DummyTexture2DLoader(*this);
}

const char* gep::DummyTexture2DLoader::getResourceId()
{
    return "<Dummy Texture2D>";
}

gep::Texture2DFileLoader::Texture2DFileLoader(const char* filename) :
    m_filename(filename),
    m_isRegistered(false)
{
    m_pRenderer = static_cast<Renderer*>(g_globalManager.getRenderer());
}

gep::Texture2DFileLoader::~Texture2DFileLoader()
{
    if(m_isRegistered)
    {
        g_globalManager.getResourceManager()->deregisterLoaderForReload(m_filename.c_str(), this);
    }
}

gep::Texture2D* gep::Texture2DFileLoader::loadResource(Texture2D* pInPlace)
{
    Texture2D* result = pInPlace;
    bool isInPlace = true;
    if(pInPlace == nullptr || pInPlace->isLoaded())
    {
        result = m_pRenderer->createTexture2D(m_filename.c_str(), this, TextureMode::Static);
        isInPlace = false;
    }
    try {
        DDSLoader loader(&g_stdAllocator);
        loader.loadFile(m_filename.c_str());
        if(loader.isCubemap())
        {
            if(!isInPlace)
                deleteResource(result);
            g_globalManager.getLogging()->logError("Error loading 2d texture from file '%s':\n The given file is a cubemap but is expected to be a 2d texture", m_filename.c_str());
            return pInPlace;
        }

        ImageFormat format;
        ImageCompression compression;

        switch(loader.getDataFormat())
        {
        case DDSLoader::D3DFORMAT::R8G8B8:
            format = ImageFormat::RGB8;
            compression = ImageCompression::NONE;
            break;
        case DDSLoader::D3DFORMAT::A8R8G8B8:
            format = ImageFormat::RGBA8;
            compression = ImageCompression::NONE;
            break;
        case DDSLoader::D3DFORMAT::DXT1:
            format = ImageFormat::COMPRESSED_RGB_DXT1;
            compression = ImageCompression::PRECOMPRESSED;
            break;
        case DDSLoader::D3DFORMAT::DXT3:
            format = ImageFormat::COMPRESSED_RGB_DXT1;
            compression = ImageCompression::PRECOMPRESSED;
            break;
        case DDSLoader::D3DFORMAT::DXT5:
            format = ImageFormat::COMPRESSED_RGB_DXT1;
            compression = ImageCompression::PRECOMPRESSED;
            break;
        default:
            {
                if(!isInPlace)
                    deleteResource(result);
                g_globalManager.getLogging()->logError("Error loading 2d texture from file '%s':\n The given textue format is not supported", m_filename.c_str());
                return pInPlace;
            }
        }

        auto data = loader.getData();
        result->getImageData().setData(data, data->imageData, data->width, data->height, format, compression);
        result->setHasData(true);
        return result;
    }
    catch(LoadingError& ex)
    {
        if(!isInPlace)
            deleteResource(result);
        g_globalManager.getLogging()->logError("Error loading 2d texture from file '%s':\n%s", m_filename.c_str(), ex.what());
        return nullptr;
    }
    return pInPlace;
}

void gep::Texture2DFileLoader::postLoad(ResourcePtr<IResource> pResource)
{
    if(!m_isRegistered)
    {
        m_isRegistered = true;
        g_globalManager.getResourceManager()->registerLoaderForReload(m_filename, this, pResource);
    }
}

gep::Texture2DFileLoader* gep::Texture2DFileLoader::moveToHeap()
{
    return new Texture2DFileLoader(*this);
}

const char* gep::Texture2DFileLoader::getResourceId()
{
    return m_filename.c_str();
}

gep::GeneratorTextureLoader::GeneratorTextureLoader(uint32 width, uint32 height, std::function<void(ArrayPtr<uint8>)>& generatorFunction, const char* resourceId)
    : m_generatorFunction(generatorFunction),
    m_resourceId(resourceId),
    m_width(width),
    m_height(height)
{
    m_pRenderer = static_cast<Renderer*>(g_globalManager.getRenderer());
}

gep::Texture2D* gep::GeneratorTextureLoader::loadResource(Texture2D* pInPlace)
{
    Texture2D* result = pInPlace;
    bool isInPlace = true;
    if(pInPlace == nullptr)
    {
        result = m_pRenderer->createTexture2D(m_resourceId.c_str(), this, TextureMode::Dynamic);
        result->createEmpty(m_width, m_height, ImageFormat::RGBA8);
        isInPlace = false;
    }
    try {
        m_generatorFunction(result->getImageData().getData()[0]);
        if(!isInPlace)
            result->setHasData(true);
        return result;
    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("Exception while generating texture '%s':\n%s", m_resourceId.c_str(), ex.what());
        if(!isInPlace)
            deleteResource(result);
    }
    return pInPlace;
}

void gep::GeneratorTextureLoader::postLoad(ResourcePtr<IResource> ptr)
{
}

gep::GeneratorTextureLoader* gep::GeneratorTextureLoader::moveToHeap()
{
    return new GeneratorTextureLoader(*this);
}

const char* gep::GeneratorTextureLoader::getResourceId()
{
    return m_resourceId.c_str();
}

gep::Texture2D::Texture2D(const char* name, ITexture2DLoader* pLoader, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, TextureMode mode) :
    m_pLoader(pLoader),
    m_name(name),
    m_pDevice(pDevice),
    m_pDeviceContext(pDeviceContext),
    m_pTexture(nullptr),
    m_hasData(false),
    m_pResourceView(nullptr),
    m_mode(mode)
{
}

gep::Texture2D::~Texture2D()
{
    Texture2D::unload();
}

gep::uint32 gep::Texture2D::getFinalizeOptions()
{
    if(!m_hasData)
        return ResourceFinalize::NotYet | ResourceFinalize::FromRenderer;
    return ResourceFinalize::FromRenderer;
}

gep::IResource* gep::Texture2D::getSuperResource()
{
    return nullptr;
}

const char* gep::Texture2D::getResourceType()
{
    return "Texture2D";
}

gep::ITexture2DLoader* gep::Texture2D::getLoader()
{
    return m_pLoader;
}

void gep::Texture2D::setLoader(IResourceLoader* loader)
{
    m_pLoader = dynamic_cast<ITexture2DLoader*>(loader);
    GEP_ASSERT(m_pLoader != nullptr);
}

bool gep::Texture2D::isLoaded()
{
    return m_hasData;
}

void gep::Texture2D::createEmpty(uint32 width, uint32 height, ImageFormat format)
{
    GEP_ASSERT(width != 0,"Width may not be 0");
    GEP_ASSERT(height != 0,"Height may not be 0");

    m_data.createEmpty(width, height, format, ImageCompression::NONE);
}

void gep::Texture2D::unload()
{
    GEP_RELEASE_AND_NULL(m_pResourceView)
    GEP_RELEASE_AND_NULL(m_pTexture)
}

void gep::Texture2D::finalize()
{
    if(m_mode == TextureMode::Dynamic &&
        m_pTexture != nullptr &&
        m_gpuWidth == m_data.getWidth() &&
        m_gpuHeight == m_data.getHeight() &&
        m_gpuFormat == m_data.getFormat())
    {
        // update already existing texture
        D3D11_MAPPED_SUBRESOURCE res;
        auto hr = m_pDeviceContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
        GEP_ASSERT(SUCCEEDED(hr));
        res.RowPitch = 0;
        res.DepthPitch = 0;
        auto newData = m_data.getData()[0];
        memcpy(res.pData, newData.getPtr(), newData.length());
        m_pDeviceContext->Unmap(m_pTexture, 0);
    }
    else
    {
        GEP_RELEASE_AND_NULL(m_pResourceView)
        GEP_RELEASE_AND_NULL(m_pTexture)
        // create a new texture
        D3D11_TEXTURE2D_DESC desc;
        desc.Width = (UINT)m_data.getWidth();
        desc.Height = (UINT)m_data.getHeight();
        desc.MipLevels = (UINT)m_data.getData().length();
        desc.ArraySize = 1;
        desc.Format = toDxgiFormat(m_data.getFormat());
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = (m_mode == TextureMode::Static) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = (m_mode == TextureMode::Static) ? 0 : D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA buffer[32];
        memset(buffer, 0, sizeof(buffer));
        GEP_ASSERT(m_data.getData().length() < GEP_ARRAY_SIZE(buffer));
        uint32 i=0;
        size_t mipWidth = m_data.getWidth();
        for(auto& mipmapLevel : m_data.getData())
        {
            buffer[i].pSysMem = mipmapLevel.getPtr();
            if(m_data.isCompressed())
            {
                if(m_data.getFormat() == ImageFormat::COMPRESSED_RGBA_DXT1 || m_data.getFormat() == ImageFormat::COMPRESSED_RGB_DXT1)
                    buffer[i].SysMemPitch = (UINT)(8 * GEP_MAX(mipWidth / 4, 1));
                else
                    buffer[i].SysMemPitch = (UINT)(16 * GEP_MAX(mipWidth / 4, 1));
            }
            else
            {
                buffer[i].SysMemPitch = (UINT)(mipWidth * m_data.getSizeOfComponent() * m_data.getNumberOfComponents());
            }
            buffer[i].SysMemSlicePitch = 0;
            i++;
            mipWidth /= 2;
        }

        HRESULT hr = m_pDevice->CreateTexture2D(&desc, buffer, &m_pTexture);
        GEP_ASSERT(SUCCEEDED(hr), "failed to create 2d texture");

        D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
        resDesc.Format = toDxgiFormat(m_data.getFormat());
        resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        resDesc.Texture2D.MostDetailedMip = 0;
        resDesc.Texture2D.MipLevels = (UINT)m_data.getData().length();
        hr = m_pDevice->CreateShaderResourceView(m_pTexture, &resDesc, &m_pResourceView);
        GEP_ASSERT(SUCCEEDED(hr), "failed to create shader resource view");
    }
}
