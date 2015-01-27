#include "stdafx.h"
#include "gepimpl/subsystems/renderer/ddsLoader.h"
#include "gep/utils.h"
#include "gep/file.h"
#include "gep/math3d/algorithm.h"

namespace
{
    bool isPowerOfTwo(size_t value)
    {
        if(value == 1)
            return true;
        return (value % 2 == 0) && isPowerOfTwo(value / 2);
    }
}

gep::DDSData::~DDSData()
{
    GEP_DELETE_ARRAY(pAllocator, memory);
    GEP_DELETE_ARRAY(pAllocator, imageData);
    GEP_DELETE_ARRAY(pAllocator, images);
}

gep::DDSLoader::DDSLoader(IAllocator* pAllocator)
{
    m_data = GEP_NEW(pAllocator, DDSData)(pAllocator);
}

gep::DDSLoader::~DDSLoader()
{

}

void gep::DDSLoader::loadFile(const char* filename)
{
    m_filename = filename;

    RawFile file(filename, "rb");
    if(!file.isOpen())
    {
        throw DDSLoadingException(format("The file '%s' does not exist", filename));
    }

    if(file.getSize() < 128)
    {
        throw DDSLoadingException(format("The file '%s' is to small to be a valid dds file", filename));
    }

    DWORD ddsMarker;
    file.read(ddsMarker);
    if(ddsMarker != 0x20534444)
    {
        throw DDSLoadingException(format("The file '%s' is not an dds file", filename));
    }

    file.read(m_header.dwSize);
    if(m_header.dwSize != sizeof(DDS_HEADER))
    {
        throw DDSLoadingException(format("dds-header size does not match inside file '%s'", filename));
    }

    file.read(m_header.dwFlags);
    file.read(m_header.dwHeight);
    file.read(m_header.dwWidth);
    file.read(m_header.dwPitchOrLinearSize);
    file.read(m_header.dwDepth);
    file.read(m_header.dwMipMapCount);
    file.readArray(m_header.dwReserved1, GEP_ARRAY_SIZE(m_header.dwReserved1));
    file.read(m_header.ddspf);
    file.read(m_header.dwCaps);
    file.read(m_header.dwCaps2);
    file.read(m_header.dwCaps3);
    file.read(m_header.dwCaps4);
    file.read(m_header.dwReserved2);

    if((m_header.ddspf.dwFlags & PixelFormatFlags::FOURCC) && (m_header.ddspf.dwFourCC == D3DFORMAT::DX10))
    {
        throw DDSLoadingException(format("Loading DX10 dds file '%s' is not supported yet", filename));
    }

    DWORD neededFlags = HeaderFlags::WIDTH | HeaderFlags::HEIGHT | HeaderFlags::PIXELFORMAT;
    if((m_header.dwFlags & neededFlags) != neededFlags)
    {
        throw DDSLoadingException(format("The dds-header of the file '%s' is missing the following flags: %s%s%s",
                                        filename,
                                        ((m_header.dwFlags & HeaderFlags::WIDTH) == 0) ? "WIDTH " : "",
                                        ((m_header.dwFlags & HeaderFlags::HEIGHT) == 0) ? "HEIGHT " : "",
                                        ((m_header.dwFlags & HeaderFlags::PIXELFORMAT) == 0) ? "PIXELFORMAT " : ""));
    }

    m_data->width = m_header.dwWidth;
    m_data->height = m_header.dwHeight;

    size_t numMipmaps = 1;
    size_t numTextures = 1;
    // is it a mipmapped texture
    if((m_header.dwFlags & HeaderFlags::MIPMAPCOUNT) != 0)
    {
        numMipmaps = m_header.dwMipMapCount;
    }
    size_t* mipmapMemorySize = static_cast<size_t*>(alloca(numMipmaps * sizeof(size_t)));

    size_t memoryNeeded = 0;
    if((m_header.ddspf.dwFlags & PixelFormatFlags::FOURCC) != 0)
    {
        // compressed texture
        if(m_header.ddspf.dwFourCC != D3DFORMAT::DXT1 &&
           m_header.ddspf.dwFourCC != D3DFORMAT::DXT2 &&
           m_header.ddspf.dwFourCC != D3DFORMAT::DXT3 &&
           m_header.ddspf.dwFourCC != D3DFORMAT::DXT4 &&
           m_header.ddspf.dwFourCC != D3DFORMAT::DXT5)
        {
            throw DDSLoadingException(format("Unkown fourcc format in file '%s'", filename));
        }

        size_t blockSize = (m_header.ddspf.dwFourCC == D3DFORMAT::DXT1) ? 8 : 16;
        size_t pitch = GEP_MAX(1, ((m_header.dwWidth+3)/4)) * blockSize; //how many bytes one scan line has
        size_t numScanLines = GEP_MAX(1, ((m_header.dwHeight+3)/4));

        if(!isPowerOfTwo(m_header.dwWidth) || !isPowerOfTwo(m_header.dwHeight))
        {
            throw DDSLoadingException(format("The file '%s' is a compressed texture but its size is not a power of 2! "
                                             "You should use textures of size 16x16, 32x32, 64x64, etc etc...",
                                             filename));
        }

        size_t mipmapWidth = m_header.dwWidth;
        size_t mipmapHeight = m_header.dwHeight;
        for(size_t i=0; i<numMipmaps; i++)
        {
            size_t mipmapPitch = GEP_MAX(1, (mipmapWidth+3)/4) * blockSize;
            size_t mipmapNumScanlines = GEP_MAX(1, (mipmapHeight+3)/4);
            mipmapMemorySize[i] = mipmapPitch * mipmapNumScanlines;
            memoryNeeded += mipmapMemorySize[i];
            mipmapWidth /= 2;
            mipmapHeight /= 2;
        }

        // Is it a cubemap?
        if(m_header.dwCaps2 & DDSCAPS2::CUBEMAP)
        {
            DWORD allSides = DDSCAPS2::CUBEMAP_POSITIVEX | DDSCAPS2::CUBEMAP_NEGATIVEX |
                             DDSCAPS2::CUBEMAP_POSITIVEY | DDSCAPS2::CUBEMAP_NEGATIVEY |
                             DDSCAPS2::CUBEMAP_POSITIVEZ | DDSCAPS2::CUBEMAP_NEGATIVEZ;
            if((m_header.dwCaps2 & allSides) != allSides)
            {
                throw DDSLoadingException(format("File '%s' is a cubemap but does not have all 6 cube map faces", filename));
            }

            numTextures = 6;
            memoryNeeded *= numTextures;
        }
        m_data->images = GEP_NEW_ARRAY(m_data->pAllocator, DDSData::image_data_t, numTextures);
        m_data->imageData = GEP_NEW_ARRAY(m_data->pAllocator, DDSData::mipmap_data_t, numTextures * numMipmaps);
    }
    else
    {
        throw DDSLoadingException(format("Error reading file '%s'. format is not supported", filename));
    }

    m_data->memory = GEP_NEW_ARRAY(m_data->pAllocator, uint8, memoryNeeded);
    size_t memStart = 0;
    size_t arrStart = 0;
    for(size_t texture=0; texture < numTextures; texture++)
    {
        m_data->images[texture] = m_data->imageData(arrStart, arrStart+numMipmaps);
        arrStart += numMipmaps;
        for(size_t mipmap=0; mipmap<numMipmaps; mipmap++)
        {
            m_data->images[texture][mipmap] = m_data->memory(memStart, memStart+mipmapMemorySize[mipmap]);
            memStart += mipmapMemorySize[mipmap];
            if( file.readArray(m_data->images[texture][mipmap].getPtr(), m_data->images[texture][mipmap].length()) != mipmapMemorySize[mipmap] )
            {
                throw DDSLoadingException(format("Error reading texture %d mipmap level %d of file '%s'", texture, mipmap, filename));
            }
        }
    }
}
