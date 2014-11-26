#pragma once

#include "gep/ArrayPtr.h"

namespace gep
{
    /// \brief all aviable image formats
    enum class ImageFormat : unsigned int {
        R8, ///8 bit int for red channel
        R16F, ///16 bit float for red channel
        R32F, ///32 bit float for red channel
        RGB8, ///ubyte for R, G and B channel
        RGB16, ///ushort for R, G and B channel
        RGB16F, ///16bit float for R, G and B channel
        RGB32F, ///32bit float for R, G and B channel
        RGBA8, ///ubyte for R, G, B and Alpha channel
        RGBA16, ///ushort for R, G, B and Alpha channel
        RGBA16F, ///16 bit float for R, G, B and Alpha channel
        RGBA32F, ///32 bit float for R, G, B and Alpha channel
        DEPTH16, ///16 bit uint depth channel
        DEPTH24, ///24 bit uint depth channel
        DEPTH32, ///32 bit uint depth channel
        DEPTH24STENCIL, ///24 bit uint depth channel, ubyte stencil
        COMPRESSED_RGB,
        COMPRESSED_RGBA,
        COMPRESSED_RGB_DXT1,
        COMPRESSED_RGBA_DXT1,
        COMPRESSED_RGBA_DXT3,
        COMPRESSED_RGBA_DXT5
    };


    /// \brief Base image formats
    enum ImageBaseFormat : unsigned int {
        R, ///1 channel R
        RGB, ///3 channel RGB
        RGBA, ///4 channel RGBA
        DEPTH, ///1 channel depth
        DEPTH_STENCIL ///1 channel depth, 1 channel stencil
    };

    /// \brief Compression options
    enum class ImageCompression : unsigned int {
        NONE,
        AUTO,
        PRECOMPRESSED
    };

    /// \brief Image data storage formats
    enum class ImageComponent : unsigned int {
        UNSIGNED_BYTE, ///8 bit unsigned int
        UNSIGNED_SHORT, ///16 bit unsigned int
        UNSIGNED_INT, ///32 bit unsigned int
        UNSIGNED_INT_24_8, ///32 bit unsigned int split into 24 bit unsigned int and 8 bit unsigned int
        FLOAT ///32 bit float
    };

    DXGI_FORMAT toDxgiFormat(ImageFormat format);

    /// \brief Wrapper class for holding image data of any format
    class ImageData2D {
    public:
        typedef unsigned char channel_data_t;
        typedef ArrayPtr<unsigned char> mipmap_data_t;
        typedef ArrayPtr<mipmap_data_t> image_data_t;
    private:
        size_t             m_width, m_height;
        ImageFormat     m_format;
        ImageBaseFormat    m_baseFormat;
        ImageComponent     m_component;
        image_data_t    m_data;
        SmartPtr<ReferenceCounted> m_pDataOwner;
        IAllocator*     m_pAllocator;
        size_t            m_sizeOfComponent;
        size_t            m_numComponents;
        bool            m_isCompressed;

        void fillComponentSizes(ImageCompression compression);

    public:

        /// \brief constructor
        ImageData2D();
        ImageData2D(IAllocator* allocator);
        ~ImageData2D();

        /// \brief Function to set data
        /// \param pAllocator
        ///    the allocator that was used to allocate the image data
        /// \param data
        ///    data to set, may not be null
        /// \param width
        ///    width of the image data to set, hast to be > 0
        /// \param    height
        ///    height of the image data to set, hast to be > 0
        /// \param format
        ///    format of the image data to set
        /// \param compression
        ///    the compression method which should be used for the data
        void setData(IAllocator* pAllocator, image_data_t data, size_t width, size_t height, ImageFormat format, ImageCompression compression);

        /// \brief Function to set data
        /// \param pDataOwner
        ///   a reference counted object which owns the data
        /// \param data
        ///    data to set, may not be null
        /// \param width
        ///    width of the image data to set, hast to be > 0
        /// \param    height
        ///    height of the image data to set, hast to be > 0
        /// \param format
        ///    format of the image data to set
        /// \param compression
        ///    the compression method which should be used for the data
        void setData(ReferenceCounted* pDataOwner, image_data_t data, size_t width, size_t height, ImageFormat format, ImageCompression compression);

        /// \brief Creates empty image data
        /// \param width
        ///    width of the data
        /// \param height
        ///    height of the data
        /// \parma format
        ///    format of the data
        /// \param pAllocator
        ///    the allocator to be used
        void createEmpty(size_t width, size_t height, ImageFormat format, ImageCompression compression, IAllocator* pAllocator = nullptr);

        /// \brief Inserts another image into this one
        /// \param    src
        ///    image to insert
        /// \param x
        ///    point to insert x coordinate
        /// \param y
        ///    point to insert y coordinate
        /// \param mipmap
        ///    on which mipmap level the operation should happen
        void insert(const ImageData2D& src, size_t x, size_t y, size_t mipmap = 0);

        /// \brief Releases the internal data
        void free();

        /// \brief loads a image from a file
        /// \param filename
        ///    the filename of the file to load
        /// \param compression
        ///    the type of compression to use
        void loadFromFile(const char* filename, ImageCompression compression);

        inline const image_data_t getData() const { return m_data; } ///get internal data
        inline image_data_t getData() { return m_data; } ///get internal data
        inline size_t getWidth() const { return m_width; } ///get width
        inline size_t getHeight() const { return m_height; } ///get height
        inline ImageFormat getFormat() const { return m_format; } ///get format
        inline ImageBaseFormat getBaseFormat() const { return m_baseFormat; } ///get base format
        inline ImageComponent getComponent() const { return m_component; } ///get component
        inline size_t getSizeOfComponent() const { return m_sizeOfComponent; } ///get size of component
        inline size_t getNumberOfComponents() const { return m_numComponents; } ///get number of components (channels)
        inline bool isCompressed() const { return m_isCompressed; }

        /// \brief Checks if internal data is present or not
        inline bool isEmpty() const { return (m_data.length() == 0); }
    };
}
