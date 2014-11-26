#pragma once
#include <Windows.h>
#include "gep/exception.h"
#include "gep/ArrayPtr.h"
#include "gep/ReferenceCounting.h"


#define GEP_MAKEFOURCC(ch0, ch1, ch2, ch3) ((int)ch0) | (((int)ch1) << 8) | (((int)ch2) << 16) | (((int)ch3) << 24)

namespace gep
{
    //forward declarations
    class IAllocator;

    class DDSLoadingException : public LoadingError
    {
    public:
        DDSLoadingException(std::string msg) : LoadingError(msg) {}
    };

    class DDSData : public ReferenceCounted
    {
    public:
        typedef ArrayPtr<uint8> mipmap_data_t;
        typedef ArrayPtr<mipmap_data_t> image_data_t;
        mipmap_data_t memory;
        image_data_t imageData;
        ArrayPtr<image_data_t> images;
        IAllocator* pAllocator;
        uint32 width;
        uint32 height;

        inline DDSData(IAllocator* pAllocator) : pAllocator(pAllocator){}
        GEP_API virtual ~DDSData();
    };

    class DDSLoader
    {
    public:
        enum class D3DFORMAT : DWORD
        {
            R8G8B8 = 20,
            A8R8G8B8 = 21,
            DXT1 = GEP_MAKEFOURCC('D','X','T','1'),
            DXT2 = GEP_MAKEFOURCC('D','X','T','2'),
            DXT3 = GEP_MAKEFOURCC('D','X','T','3'),
            DXT4 = GEP_MAKEFOURCC('D','X','T','4'),
            DXT5 = GEP_MAKEFOURCC('D','X','T','5'),
            DX10 = GEP_MAKEFOURCC('D','X','1','0')
        };

    private:
        struct DDS_PIXELFORMAT
        {
            DWORD dwSize;
            DWORD dwFlags;
            D3DFORMAT dwFourCC;
            DWORD dwRGBBitCount;
            DWORD dwRBitMask;
            DWORD dwGBitMask;
            DWORD dwBBitMask;
            DWORD dwABitMask;
        };

        struct PixelFormatFlags
        {
            enum Enum
            {
                ALPHAPIXELS = 0x1,
                ALPHA       = 0x2,
                FOURCC      = 0x4,
                RGB         = 0x40,
                YUV         = 0x200,
                LUMINANCE   = 0x20000
            };
        };

        struct DDSCAPS
        {
            enum Enum
            {
                COMPLEX = 0x8,
                MIPMAP = 0x400000,
                TEXTURE = 0x1000
            };
        };

        struct DDSCAPS2
        {
            enum Enum
            {
                CUBEMAP = 0x200,
                CUBEMAP_POSITIVEX = 0x400,
                CUBEMAP_NEGATIVEX = 0x800,
                CUBEMAP_POSITIVEY = 0x1000,
                CUBEMAP_NEGATIVEY = 0x2000,
                CUBEMAP_POSITIVEZ = 0x4000,
                CUBEMAP_NEGATIVEZ = 0x8000,
                VOLUME = 0x200000
            };
        };

        struct DDS_HEADER
        {
            DWORD           dwSize;
            DWORD           dwFlags;
            DWORD           dwHeight;
            DWORD           dwWidth;
            DWORD           dwPitchOrLinearSize;
            DWORD           dwDepth;
            DWORD           dwMipMapCount;
            DWORD           dwReserved1[11];
            DDS_PIXELFORMAT ddspf;
            DWORD           dwCaps;
            DWORD           dwCaps2;
            DWORD           dwCaps3;
            DWORD           dwCaps4;
            DWORD           dwReserved2;
        };

        struct HeaderFlags
        {
            enum Enum
            {
                CAPS = 0x1, //required in every dds file
                HEIGHT = 0x2, //required in every dds file
                WIDTH = 0x4, // required in every dds file
                PITCH = 0x8, // required when pitch is provided for uncompressed texture
                PIXELFORMAT = 0x1000, // required in every dds file
                MIPMAPCOUNT = 0x20000, // required in a mipmaped texture
                LINEARSIZE  = 0x80000, // required when a pitch is provided for a compressed texture
                DEPTH       = 0x800000 // required in a depth texture
            };
        };

        DDS_HEADER m_header;
        std::string m_filename;
        SmartPtr<DDSData> m_data;

    public:

        inline DDSData* getData() { return m_data.get(); }

        inline ArrayPtr<DDSData::image_data_t> getImages()
        {
            return m_data->images;
        }

        inline D3DFORMAT getDataFormat() const
        {
            return (D3DFORMAT)m_header.ddspf.dwFourCC;
        }

        inline bool isCubemap() const
        {
            return (m_header.dwCaps2 & DDSCAPS2::CUBEMAP) != 0;
        }

        inline unsigned int getWidth() const
        {
            return (unsigned int)m_header.dwWidth;
        }

        inline unsigned int getHeight() const
        {
            return (unsigned int)m_header.dwHeight;
        }

        GEP_API DDSLoader(IAllocator* pAllocator);
        GEP_API ~DDSLoader();

        GEP_API void loadFile(const char* filename);
    };

}
