#include "stdafx.h"
#include "gepimpl/subsystems/renderer/imageData2d.h"
#include "gepimpl/subsystems/renderer/ddsLoader.h"
#include <sstream>


void gep::ImageData2D::fillComponentSizes(ImageCompression compression){
    switch(m_format){
    case ImageFormat::R8:
        m_sizeOfComponent = 1;
        m_numComponents = 1;
        m_component = ImageComponent::UNSIGNED_BYTE;
        m_baseFormat = ImageBaseFormat::R;
        m_isCompressed = false;
        break;
    case ImageFormat::R16F:
    case ImageFormat::R32F:
        m_sizeOfComponent = 4;
        m_numComponents = 1;
        m_component =    ImageComponent::FLOAT;
        m_baseFormat = ImageBaseFormat::R;
        m_isCompressed = false;
        break;
    case ImageFormat::RGB8:
        m_sizeOfComponent = 1;
        m_numComponents = 3;
        m_component = ImageComponent::UNSIGNED_BYTE;
        m_baseFormat = ImageBaseFormat::RGB;
        if(compression == ImageCompression::AUTO)
        {
            m_format = ImageFormat::COMPRESSED_RGB_DXT1;
        }
        m_isCompressed = false;
        break;
    case ImageFormat::RGB16:
        m_sizeOfComponent = 2;
        m_numComponents = 3;
        m_component = ImageComponent::UNSIGNED_SHORT;
        m_baseFormat = ImageBaseFormat::RGB;
        m_isCompressed = false;
        break;
    case ImageFormat::RGB16F:
    case ImageFormat::RGB32F:
        m_sizeOfComponent = 4;
        m_numComponents = 3;
        m_component = ImageComponent::FLOAT;
        m_baseFormat = ImageBaseFormat::RGB;
        m_isCompressed = false;
        break;
    case ImageFormat::RGBA8:
        m_sizeOfComponent = 1;
        m_numComponents = 4;
        m_component = ImageComponent::UNSIGNED_BYTE;
        m_baseFormat = ImageBaseFormat::RGBA;
        if(compression == ImageCompression::AUTO)
        {
            m_format = ImageFormat::COMPRESSED_RGBA_DXT5;
        }
        m_isCompressed = false;
        break;
    case ImageFormat::RGBA16:
        m_sizeOfComponent = 2;
        m_numComponents = 4;
        m_component = ImageComponent::UNSIGNED_SHORT;
        m_baseFormat = ImageBaseFormat::RGBA;
        m_isCompressed = false;
        break;
    case ImageFormat::RGBA16F:
    case ImageFormat::RGBA32F:
        m_sizeOfComponent = 4;
        m_numComponents = 4;
        m_component = ImageComponent::FLOAT;
        m_baseFormat = ImageBaseFormat::RGBA;
        m_isCompressed = false;
        break;
    case ImageFormat::DEPTH16:
        m_sizeOfComponent = 2;
        m_numComponents = 1;
        m_component = ImageComponent::UNSIGNED_SHORT;
        m_baseFormat = ImageBaseFormat::DEPTH;
        m_isCompressed = false;
        break;
    case ImageFormat::DEPTH24:
    case ImageFormat::DEPTH32:
        m_sizeOfComponent = 4;
        m_numComponents = 1;
        m_component = ImageComponent::UNSIGNED_INT;
        m_baseFormat = ImageBaseFormat::DEPTH;
        m_isCompressed = false;
        break;
    case ImageFormat::DEPTH24STENCIL:
        m_sizeOfComponent = 4;
        m_numComponents = 1;
        m_component = ImageComponent::UNSIGNED_INT_24_8;
        m_baseFormat = ImageBaseFormat::DEPTH_STENCIL;
        m_isCompressed = false;
        break;
    case ImageFormat::COMPRESSED_RGBA_DXT1:
    case ImageFormat::COMPRESSED_RGBA_DXT3:
    case ImageFormat::COMPRESSED_RGBA_DXT5:
        m_sizeOfComponent = 1;
        m_numComponents = 4;
        m_component = ImageComponent::UNSIGNED_BYTE;
        m_baseFormat = ImageBaseFormat::RGBA;
        m_isCompressed = true;
        break;
    case ImageFormat::COMPRESSED_RGB_DXT1:
        m_sizeOfComponent = 1;
        m_numComponents = 1;
        m_component = ImageComponent::UNSIGNED_BYTE;
        m_baseFormat = ImageBaseFormat::RGB;
        m_isCompressed = true;
        break;
    default:
        GEP_ASSERT(false, "invalid image format");
    }
}

gep::ImageData2D::ImageData2D() :
    m_pAllocator(nullptr)
{
}

gep::ImageData2D::ImageData2D(IAllocator* pAllocator)
{
    m_pAllocator = pAllocator;
}

gep::ImageData2D::~ImageData2D(){
    ImageData2D::free();
}

void gep::ImageData2D::setData(IAllocator* pAllocator, image_data_t data, size_t width, size_t height, ImageFormat format, ImageCompression compression)
{
    GEP_ASSERT(pAllocator != nullptr);
    GEP_ASSERT(width > 0 && height > 0);
    m_format = format;
    fillComponentSizes(compression);
    m_width = width;
    m_height = height;
    m_pAllocator = pAllocator;
    m_data = data;
    m_pDataOwner = nullptr;
}

void gep::ImageData2D::setData(ReferenceCounted* pDataOwner, image_data_t data, size_t width, size_t height, ImageFormat format, ImageCompression compression)
{
    GEP_ASSERT(width > 0 && height > 0);
    m_format = format;
    fillComponentSizes(compression);
    m_width = width;
    m_height = height;
    m_pAllocator = nullptr;
    m_data = data;
    m_pDataOwner = pDataOwner;
}

void gep::ImageData2D::createEmpty(size_t width, size_t height, ImageFormat format, ImageCompression compression, IAllocator* pAllocator)
{
    GEP_ASSERT(width > 0 && height > 0);
    if(pAllocator == nullptr)
        pAllocator = &g_stdAllocator;
    m_pAllocator = pAllocator;
    m_format = format;
    fillComponentSizes(compression);
    m_width = width;
    m_height = height;
    size_t imageSize = m_width * m_height * m_numComponents * m_sizeOfComponent;
    m_data = GEP_NEW_ARRAY(m_pAllocator, mipmap_data_t, 1);
    m_data[0] = GEP_NEW_ARRAY(m_pAllocator, channel_data_t, imageSize);
}


void gep::ImageData2D::insert(const ImageData2D& src, size_t x, size_t y, size_t mipmap)
{
    GEP_ASSERT(src.m_format == m_format, "Can only insert image data of the same format");
    GEP_ASSERT(x + src.m_width <= m_width, "Insert out of bounds on x axis");
    GEP_ASSERT(y + src.m_height <= m_height, "Insert out of bounds on y axis");
    GEP_ASSERT(m_data.length() > 0, "Image data to insert in has no data");
    size_t sizeOfPixel = m_numComponents * m_sizeOfComponent;
    size_t sizeOfRow = m_width * sizeOfPixel;

    auto dstData = m_data[mipmap];
    auto srcData = src.getData()[mipmap];
    size_t sizeOfSrcRow = src.m_width * sizeOfPixel;
    for(size_t dstY=0; dstY<src.m_height; dstY++){
        size_t dstIndex = (y + dstY) * sizeOfRow + x * sizeOfPixel;
        size_t srcIndex = dstY * sizeOfSrcRow;
        dstData(dstIndex, dstIndex + sizeOfSrcRow).copyFrom( srcData(srcIndex, srcIndex + sizeOfSrcRow) );
    }
}

void gep::ImageData2D::free()
{
    if(m_pDataOwner)
    {
        GEP_ASSERT(m_pAllocator == nullptr);
        m_pDataOwner = nullptr;
        m_data = nullptr;
    }
    else if(m_data.getPtr() != nullptr)
    {
        GEP_ASSERT(m_pAllocator != nullptr);
        for(auto mipmap : m_data)
        {
            m_pAllocator->freeMemory(mipmap.getPtr());
        }
        m_pAllocator->freeMemory(m_data.getPtr());
        m_data = nullptr;
    }
}

void gep::ImageData2D::loadFromFile(const char* filename, ImageCompression compression)
{
    GEP_ASSERT(m_pAllocator != nullptr);
    const size_t len = strlen(filename);
    if(len > 4 && _stricmp(filename + len - 4, ".dds") == 0)
    {
        DDSLoader loader(m_pAllocator);

        loader.loadFile(filename);
        if(loader.isCubemap())
        {
            std::ostringstream msg;
            msg << "Trying to load file " << filename << " which is a cubemap as 2d texture";
            throw Exception(msg.str());
        }

        if(loader.getImages().length() > 1)
        {
            std::ostringstream msg;
            msg << "Trying to loader file '" << filename << "' which is a texture array of size " << loader.getImages().length() << " as 2d texture";
            throw Exception(msg.str());
        }

        ImageFormat imgformat;

        switch(loader.getDataFormat())
        {
        case DDSLoader::D3DFORMAT::DXT1:
            imgformat = ImageFormat::COMPRESSED_RGBA_DXT1;
            GEP_ASSERT(compression != ImageCompression::NONE, "can not uncompress compressed dxt1 texture");
            break;
        case DDSLoader::D3DFORMAT::DXT3:
            imgformat = ImageFormat::COMPRESSED_RGBA_DXT3;
            GEP_ASSERT(compression != ImageCompression::NONE, "can not uncompress compressed dxt3 texture");
            break;
        case DDSLoader::D3DFORMAT::DXT5:
            imgformat = ImageFormat::COMPRESSED_RGBA_DXT5;
            GEP_ASSERT(compression != ImageCompression::NONE, "can not uncompress compressed dxt5 texture");
            break;
        default:
            {
                std::ostringstream msg;
                msg << "trying to load file " << filename << " which has a unsupported format";
                throw Exception(msg.str());
            }
        }

        setData(loader.getData(), loader.getImages()[0], loader.getWidth(), loader.getHeight(), imgformat, ImageCompression::PRECOMPRESSED);
    }
    else
    {
        std::ostringstream msg;
        msg << "trying to load file '" << filename << "' which has a unkown format";
        throw Exception(msg.str());
    }
}

DXGI_FORMAT gep::toDxgiFormat(ImageFormat format)
{
    switch(format)
    {
    case ImageFormat::R8:
        return DXGI_FORMAT_R8_UNORM;
    case ImageFormat::RGBA8:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ImageFormat::COMPRESSED_RGB_DXT1:
    case ImageFormat::COMPRESSED_RGBA_DXT1:
        return DXGI_FORMAT_BC1_UNORM;
    case ImageFormat::COMPRESSED_RGBA_DXT3:
        return DXGI_FORMAT_BC3_UNORM;
    case ImageFormat::COMPRESSED_RGBA_DXT5:
        return DXGI_FORMAT_BC5_UNORM;
    default:
        GEP_ASSERT(false, "not implemented yet");
    }
    return DXGI_FORMAT_UNKNOWN;
}
