#pragma once

#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/interfaces/resourceManager.h"
#include "gepimpl/subsystems/renderer/texture2d.h"

// forward declrations for freetype

namespace gep
{
    //forward declarations
    class Renderer;
    class Texture2D;
    class ImageData2D;
    class Vertexbuffer;
    class Font;

    /// \brief interface for loading a font
    class IFontLoader : public IResourceLoader
    {
    public:
        virtual Font* loadResource(Font* pInPlace) = 0;
        virtual IResource* loadResource(IResource* pInPlace) override;
        virtual void deleteResource(IResource* pResource) override;
        virtual const char* getResourceType() override;
        virtual void release() override;
    };

    /// \brief loads a font from a file
    class FontFileLoader : public IFontLoader
    {
    private:
        std::string m_filename;
        uint32 m_fontSize;

    public:
        FontFileLoader(const char* filename, uint32 fontSize);
        virtual Font* loadResource(Font* pInPlace) override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual FontFileLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    /// \brief generates a 2d texture from a font
    class Texture2DFromFontLoader : public ITexture2DLoader
    {
    private:
        ResourcePtr<Font> m_pFont;
        Renderer* m_pRenderer;
        std::string m_resourceId;

    public:
        Texture2DFromFontLoader(ResourcePtr<Font> pFont);
        virtual Texture2D* loadResource(Texture2D* pInPlace) override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual Texture2DFromFontLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    enum class FontHorizontalOrientation
    {
        Left,
        Centered
    };

    /// \brief holds a font
    class Font : public IResource
    {
        friend class Texture2DFromFontLoader;
    public:

        struct FontGlyph {
            int m_width;
            int m_height;
            int m_top;
            int m_left;
            int m_right;
            float m_minTexX,m_maxTexX;
            float m_minTexY,m_maxTexY;
            float m_fWidth,m_realWidth;
            ImageData2D* m_data;
        };

    private:
        static const wchar_t* s_charsToLoad;

        IFontLoader* m_pLoader;
        uint32 m_id;
        wchar_t m_startIndex;
        wchar_t m_endIndex;
        size_t m_anzChars;
        DynamicArray<FontGlyph> m_glyphs;
        ResourcePtr<Texture2D> m_pFontTexture;
        std::string m_name;
        int m_maxHeight;
        ArrayPtr<size_t> m_charAsignment;
        bool m_isPrintable;
        std::string m_filename;
        int m_size;

        void buildTexture(Texture2D& texture);
        void buildChar(void* pFace, wchar_t c, size_t num);

    public:
        /**
        * constructor
        * Params:
        *        pName = name of the font has to be unique
        */
        Font(const char* pName, IFontLoader* pLoader);

        ~Font();

        /**
        * prints text into a vertexbuffer
        * the vertexbuffer has to have 2 component position
        * and a 2 component texture coordinate
        * Params:
        *        pFontBuffer = the vertex buffer to add the data to
        *        pFmt the text to print
        */
        void print(Vertexbuffer& pFontBuffer, const char* text, FontHorizontalOrientation orientation = FontHorizontalOrientation::Left) const;

        /**
        * gets the size of a text in pixels
        * Params:
        *        pWidth = width output
        *        pHeight = height output
        *        fmt = the text
        * Returns: the formated string
        */
        void getTextSize(int& width, int& height, const char* text) const;

        /**
        * gets the index of the char at a certain width
        * usefull to determinate when to make a line break
        * Params:
        *        pWidth = the maximum width
        *        pText = the text to check
        */
        size_t getCharAtWidth(uint32 width, const char* text) const;

        /**
        * loads a font from a ttf file
        * Params:
        *        pFilename = the filename of the font
        *        pSize = the font size to load
        */
        void load(const char* pFilename, int pSize);

        /**
        * gets the maximum height of the font
        */
        int getMaxFontHeight() const { return m_maxHeight; }

        /**
        * gets the font texture
        */
        inline ResourcePtr<Texture2D> getFontTexture() { return m_pFontTexture; }

        inline const std::string& getName() const { return m_name; }

        // IResource interface
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;
        virtual IFontLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual bool isLoaded() override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;
    };
}
