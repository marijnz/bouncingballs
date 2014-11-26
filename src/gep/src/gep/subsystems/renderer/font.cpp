#include "stdafx.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "gep/common.h"
#include "gepimpl/subsystems/renderer/renderer.h"
#include "gepimpl/subsystems/renderer/font.h"
#include "gepimpl/subsystems/renderer/imageData2d.h"
#include "gepimpl/subsystems/renderer/texture2d.h"
#include "gepimpl/subsystems/renderer/vertexbuffer.h"
#include "gep/exception.h"
#include "gep/globalManager.h"
#include "gep/interfaces/resourceManager.h"

const wchar_t* gep::Font::s_charsToLoad  = L"? 1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ???abcdefghijklmnopqrstuvwxyz????+*/\\#,.:;_-()[]{}\"'<>|@=!";

void gep::Font::buildTexture(Texture2D& texture)
{
    int maxWidth, minY, maxY, maxHeight;
    maxWidth = m_glyphs[0].m_width;
    maxY = m_glyphs[0].m_top;
    minY = m_glyphs[0].m_height - m_glyphs[0].m_top;
    for(auto& g : m_glyphs)
    {
        maxWidth = (g.m_width > maxWidth) ? g.m_width : maxWidth;
        maxY = (g.m_top > maxY) ? g.m_top : maxY;
        minY = (g.m_top - g.m_height< minY) ? g.m_top - g.m_height: minY;
    }
    maxHeight = maxY - minY;
    m_maxHeight = maxHeight;

    int size=64;
    int x,y;
    while(size < 2048){
        x=y=0;
        for(auto& g : m_glyphs)
        {
            if((x + g.m_width + 1) > size)
            {
                y += maxHeight;
                x = 0;
            }
            x+=g.m_width + 1;
        }
        if(y > size)
            size*=2;
        else
            break;
    }
    if(size == 2048){
        throw Exception("Font texture is to large");
    }

    texture.createEmpty(size, size, ImageFormat::R8);
    ImageData2D& ImageData = texture.getImageData();
    memset(ImageData.getData()[0].getPtr(), 0, ImageData.getData()[0].length());

    x=y=0;
    for(auto& g : m_glyphs)
    {
        if((x + g.m_width + 1) > size){
            y += maxHeight;
            x=0;
        }
        if(g.m_data != nullptr)
        {
            ImageData.insert(*g.m_data, x, y + maxHeight - g.m_top + minY);
            delete g.m_data;
            g.m_data = nullptr;
        }
        float fStep = 1.0f / (float)size;
        g.m_minTexY = (float)y * fStep;
        g.m_minTexX = (float)(x) * fStep;
        x += g.m_width;
        g.m_maxTexY = (float)(y + maxHeight) * fStep;
        g.m_maxTexX = (float)(x) * fStep;
        x++;
        g.m_fWidth = (float)(g.m_width + g.m_right);
        g.m_realWidth = (float)g.m_width;
    }
}

void gep::Font::buildChar(void* pFace, wchar_t c, size_t num){
    FT_Face face = (FT_Face)pFace;
    // Load the glyph
    if(FT_Load_Glyph( face, FT_Get_Char_Index( face, c ), FT_LOAD_DEFAULT ) != 0)
    {
        std::ostringstream msg;
        msg << "Error loading character " << (uint32)c;
        throw Exception(msg.str());
    }

    FT_Glyph glyph;
    FT_Get_Glyph( face->glyph, &glyph);

    // Convert it to a bitmap
    FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, nullptr, 1 );
    FT_BitmapGlyph bitmap_glyph((FT_BitmapGlyph)glyph);
    FT_Bitmap* bitmap = &bitmap_glyph->bitmap;

    // Allocate the bitmap
    ImageData2D::image_data_t buffer;
    if(bitmap->width > 0 || bitmap->rows > 0) {
        buffer = GEP_NEW_ARRAY(g_stdAllocator, ImageData2D::mipmap_data_t, 1);
        buffer[0] = GEP_NEW_ARRAY(g_stdAllocator, ImageData2D::channel_data_t, bitmap->rows * bitmap->width);
        auto& mipmap = buffer[0];
        for(int i=0; i<bitmap->rows * bitmap->width; i++){
            mipmap[i] = bitmap->buffer[i];
        }
    }

    auto& g = m_glyphs[num];
    if(bitmap->width > 0 || bitmap->rows > 0) {
        g.m_data = GEP_NEW(g_stdAllocator, ImageData2D);
        g.m_data->setData(&g_stdAllocator, buffer, bitmap->width, bitmap->rows, ImageFormat::R8, ImageCompression::NONE);
    }
    else
    {
        g.m_data = nullptr;
    }
    g.m_width = bitmap->width;
    g.m_height = bitmap->rows;
    g.m_top = bitmap_glyph->top;
    g.m_left = bitmap_glyph->left;
    g.m_right = (face->glyph->advance.x >> 6) - bitmap_glyph->left - bitmap->width;
}

void gep::Font::load(const char* filename, int size)
{
    m_filename = filename;
    m_size = size;

    FT_Library library;
    if( FT_Init_FreeType( &library )){
        throw Exception("Couldn't init freetype library");
    }
    SCOPE_EXIT
    {
        FT_Done_FreeType(library);
    });


    FT_Face face;
    if(FT_Error error = FT_New_Face( library, filename, 0, &face ))
    {
        std::ostringstream msg;
        msg << "Couldn't load font '" << m_name << "' from file '" << filename << " error " << (uint32)error;
        throw Exception(msg.str());
    }
    SCOPE_EXIT
    {
        FT_Done_Face(face);
    });

    FT_Set_Char_Size( face, size * 64, size * 64, 72, 72 );

    auto numCharsToLoad = wcslen(s_charsToLoad);
    m_glyphs.resize(numCharsToLoad);

    //Search for starting character
    wchar_t sc = s_charsToLoad[0];
    wchar_t ec = s_charsToLoad[0];
    for(size_t i=0; i<numCharsToLoad; i++)
    {
        auto c = s_charsToLoad[i];
        if(c < sc)
            sc = c;
        if(c > ec)
            ec = c;
    }
    m_startIndex = sc;
    m_endIndex = ec;
    m_anzChars = m_glyphs.length();

    size_t CharAnz = (size_t)(ec - sc) + 1;
    m_charAsignment = GEP_NEW_ARRAY(g_stdAllocator, size_t, CharAnz);
    for(auto& c : m_charAsignment)
    {
        c = 0;
    }

    //Building the chars
    for(size_t i=0; i<numCharsToLoad; i++)
    {
        auto toLoad = s_charsToLoad[i];
        buildChar(face, toLoad, i);
        m_charAsignment[toLoad - m_startIndex] = i;
    }

    //TODO LoadAsync::no
    m_pFontTexture = g_globalManager.getResourceManager()->loadResource<Texture2D>(Texture2DFromFontLoader(makeResourcePtrFromThis<Font>()));
    m_isPrintable = true;
}

gep::Font::Font(const char* name, IFontLoader* pLoader) :
    m_name(name),
    m_pLoader(pLoader),
    m_isPrintable(false)
{
}

gep::Font::~Font()
{
    unload();
}

void gep::Font::print(Vertexbuffer& pFontBuffer, const char* text, FontHorizontalOrientation orientation /*= Orientation::Left*/) const
{
    GEP_ASSERT(m_isPrintable, "font is not printable yet");
    float TempBuffer[16];
    float maxHeight = (float)m_maxHeight;
    float offsetX=0.0f, offsetY = 0.0f;
    
    switch (orientation)
    {
    case gep::FontHorizontalOrientation::Left:
        break;
    case gep::FontHorizontalOrientation::Centered:
        {
            int width = 0;
            int height = 0;
            getTextSize(width, height, text);
            offsetX = -width / 2.0f;
        }
        break;
    default:
        GEP_ASSERT(false, "Unsupported orientation type!");
        break;
    }
    
    size_t len = strlen(text);
    for(size_t i=0; i<len; i++)
    {
        auto c = text[i];
        GEP_ASSERT(c > 0, "ascii string contains non-ascii characters. Please use wide char version to print non ascii characters");
        size_t index = 0;
        if(c == '\n'){
            offsetY += floor(maxHeight * 1.5f);
            offsetX = 0.0;
            continue;
        }
        if(c >= m_startIndex && c <= m_endIndex)
            index = m_charAsignment[c - m_startIndex];
            
        //Print the glyph
        auto& glyph = m_glyphs[index];
        offsetX += (float)glyph.m_left;

        TempBuffer[0] = offsetX; TempBuffer[1] = offsetY;
        TempBuffer[2] = glyph.m_minTexX; TempBuffer[3] = glyph.m_minTexY;

        TempBuffer[4] = offsetX; TempBuffer[5] = maxHeight+offsetY;
        TempBuffer[6] = glyph.m_minTexX; TempBuffer[7] = glyph.m_maxTexY;

        TempBuffer[8] = glyph.m_realWidth+offsetX; TempBuffer[9] = maxHeight+offsetY;
        TempBuffer[10] = glyph.m_maxTexX; TempBuffer[11] = glyph.m_maxTexY;

        TempBuffer[12] = glyph.m_realWidth+offsetX; TempBuffer[13] = offsetY;
        TempBuffer[14] = glyph.m_maxTexX; TempBuffer[15] = glyph.m_minTexY;

        uint32 startIndex = (uint32)pFontBuffer.getCurrentNumVertices();
        uint32 indices[] = { startIndex, startIndex + 2, startIndex + 1, //first triangle
                             startIndex , startIndex + 3, startIndex + 2 }; // second triangle
        pFontBuffer.getIndices().append(indices);
        pFontBuffer.getData().append(TempBuffer);

        offsetX += glyph.m_fWidth;
    }
}

void gep::Font::getTextSize(int& width, int& height, const char* text) const
{
    width=0;
    size_t index = 0;

    if(text[0] > m_startIndex && text[0] <= m_endIndex)
        index = m_charAsignment[text[0] - m_startIndex];

    int maxY = m_glyphs[index].m_top;
    int minY = m_glyphs[index].m_height - m_glyphs[index].m_top;
    size_t len = strlen(text);
    for(size_t i=0; i<len; ++i)
    {
        auto c = text[i];
        index = 0;
        if(c >= m_startIndex && c <= m_endIndex)
            index = m_charAsignment[c - m_startIndex];

        auto& g = m_glyphs[index];
        width += g.m_width + g.m_left + g.m_right;
        maxY = (g.m_top > maxY) ? g.m_top : maxY;
        minY = (g.m_top - g.m_height< minY) ? g.m_top - g.m_height: minY;
    }
    height = maxY - minY;
}

size_t gep::Font::getCharAtWidth(uint32 width, const char* text) const
{
    uint32 currentWidth = 0;
    size_t len = strlen(text);
    for(size_t i=0; i<len; i++)
    {
        auto c = text[i];
        size_t index = 0;
        if(c == '\n')
            currentWidth = 0;
        if(c >= m_startIndex && c <= m_endIndex)
            index = m_charAsignment[c - m_startIndex];

        auto& g = m_glyphs[index];
        currentWidth += g.m_width + g.m_left + g.m_right;

        if(currentWidth > width)
            return i;
    }
    return -1;
}

gep::IResource* gep::Font::getSuperResource()
{
    return nullptr;
}

const char* gep::Font::getResourceType()
{
    return "Font";
}

gep::IFontLoader* gep::Font::getLoader()
{
    return m_pLoader;
}

void gep::Font::setLoader(IResourceLoader* pLoader)
{
    m_pLoader = dynamic_cast<IFontLoader*>(pLoader);
}

bool gep::Font::isLoaded()
{
    return m_isPrintable;
}

void gep::Font::unload()
{
    if(m_isPrintable)
    {
        m_isPrintable = false;
        g_globalManager.getResourceManager()->deleteResource(m_pFontTexture);
        GEP_DELETE_ARRAY(g_stdAllocator, m_charAsignment);
    }
}

void gep::Font::finalize()
{
}

gep::uint32 gep::Font::getFinalizeOptions()
{
    return 0;
}

// IFontLoader
gep::IResource* gep::IFontLoader::loadResource(IResource* pInPlace)
{
    auto res = dynamic_cast<Font*>(pInPlace);
    GEP_ASSERT(pInPlace == nullptr || res != nullptr);
    return loadResource(res);
}

void gep::IFontLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<Font*>(pResource);
    GEP_ASSERT(pResource == nullptr || res != nullptr);
    delete res;
}

const char* gep::IFontLoader::getResourceType()
{
    return "Font";
}

void gep::IFontLoader::release()
{
    delete this;
}

// FontFileLoader
gep::FontFileLoader::FontFileLoader(const char* filename, uint32 fontSize) :
    m_filename(filename),
    m_fontSize(fontSize)
{

}

gep::Font* gep::FontFileLoader::loadResource(Font* pInPlace)
{
    if(pInPlace == nullptr)
    {
        char name[256];
        sprintf_s(name, "%.*s %d", m_filename.length()-4, m_filename.c_str(), m_fontSize);
        pInPlace = new Font(name, this);
    }
    pInPlace->unload();
    pInPlace->load(m_filename.c_str(), m_fontSize);
    return pInPlace;
}

void gep::FontFileLoader::postLoad(ResourcePtr<IResource> pResource)
{

}

gep::FontFileLoader* gep::FontFileLoader::moveToHeap()
{
    return new FontFileLoader(*this);
}

const char* gep::FontFileLoader::getResourceId()
{
    return m_filename.c_str();
}

// Texture2DFromFontLoader
gep::Texture2DFromFontLoader::Texture2DFromFontLoader(ResourcePtr<Font> pFont) :
    m_pFont(pFont)
{
    m_pRenderer = static_cast<Renderer*>(g_globalManager.getRenderer());
}

gep::Texture2D* gep::Texture2DFromFontLoader::loadResource(Texture2D* pInPlace)
{
    GEP_ASSERT(pInPlace == nullptr, "reloading not implemented yet");
    if(pInPlace == nullptr)
    {
        char name[256];
        sprintf_s(name, "%s - font texture", m_pFont->getName().c_str());
        pInPlace = m_pRenderer->createTexture2D(name, this, TextureMode::Static);
    }
    m_pFont->buildTexture(*pInPlace);
    pInPlace->setHasData(true);
    return pInPlace;
}

void gep::Texture2DFromFontLoader::postLoad(ResourcePtr<IResource> pResource)
{
}

gep::Texture2DFromFontLoader* gep::Texture2DFromFontLoader::moveToHeap()
{
    return new Texture2DFromFontLoader(*this);
}

const char* gep::Texture2DFromFontLoader::getResourceId()
{
    return m_resourceId.c_str();
}
