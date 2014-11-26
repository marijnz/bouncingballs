#pragma once
#include "gep/interfaces/renderer.h"
#include "gep/settings.h"
#include <d3d11.h>
#include "gep/interfaces/resourceManager.h"

#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/color.h"

#include "gepimpl/subsystems/renderer/shader.h"
#include "gep/container/DynamicArray.h"
#include "gep/memory/allocators.h"


typedef int (WINAPI * D3DPERF_EndEvent_Func)();
typedef int (WINAPI * D3DPERF_BeginEvent_Func)(DWORD, LPCWSTR);
struct ID3DUserDefinedAnnotation;

#if _DEBUG
extern D3DPERF_EndEvent_Func D3DPREF_EndEvent;
extern D3DPERF_BeginEvent_Func D3DPREF_BeginEvent;
#else
#define D3DPREF_EndEvent()
#define D3DPREF_BeginEvent(first, second)
#endif


namespace gep
{
    //forward declarations
    class Texture2D;
    class ITexture2DLoader;
    class Font;
    enum class FontHorizontalOrientation;
    class Shader;
    class Vertexbuffer;
    class Model;
    class RendererExtractor;
    struct CommandBase;
    struct LineInfo;
    struct LineInfo2D;

    struct RenderTextInfo
    {
        uint32 startIndex, numIndices;
    };


    enum class TextureMode
    {
        Static,
        Dynamic
    };

    class Renderer
        : public IRenderer
    {
    private:
        IDebugRenderer* m_pDebugRenderer;
        settings::Video& m_settings;
        bool m_actualVSync;

        void createWindow();
        void destroyWindow();
        void initD3DDevice();
        void render();
        void prepareCommands(RendererExtractor& extractor, CommandBase* firstCommand);
        void executeCommands(RendererExtractor& extractor, CommandBase* firstCommand);
        void execute2DCommands(RendererExtractor& extractor, CommandBase* firstCommand);

        //windows specific stuff
        HINSTANCE m_hInstance;
        HWND m_hWnd;
        virtual HWND getWindowHandle() override { return m_hWnd; }

        static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

        //DirectX specific stuff
        ID3D11Device*           m_pd3dDevice;
        IDXGISwapChain*         m_pSwapChain;
        ID3D11RenderTargetView* m_pRenderTargetView;
        ID3D11DepthStencilView* m_pDepthStencilView;
        ID3DUserDefinedAnnotation* m_pUserDefinedAnnotation;
        ID3D11DeviceContext*    m_pDeviceContext;
        D3D_FEATURE_LEVEL       m_featureLevel;

        Texture2D* m_pDummyTexture;
        Shader* m_pDummyShader;
        Model* m_pDummyModel;

        ResourcePtr<Font> m_pDefaultFont;

        ResourcePtr<Shader> m_pFontShader;
        ShaderConstant<vec2> m_fontPosition;
        ShaderConstant<Color> m_fontColor;
        ShaderConstant<vec2> m_fontScreenSize;
        ShaderConstant<Texture2D> m_fontTexture;
        Vertexbuffer* m_pFontBuffer;
        Vertexbuffer* m_pLinesBuffer;
        Vertexbuffer* m_pLines2DBuffer;
        bool m_isFontBufferOutOfDate;
        
        ResourcePtr<Shader> m_pLightingShader;
        ResourcePtr<Shader> m_pLightingAnimatedShader;
        ResourcePtr<Shader> m_pLinesShader;
        ResourcePtr<Shader> m_pWireframeShader;
        ShaderConstant<Color> m_lineColor;
        ShaderConstant<mat4> m_lineView;
        ShaderConstant<mat4> m_lineProjection;
        mat4 m_projection;
        mat4 m_view;

        // Lines2D
        ResourcePtr<Shader> m_pLines2DShader;
        ShaderConstant<Color> m_line2DColor;
        ShaderConstant<vec2> m_lines2DScreenSize;

        // Text Billboard
        ResourcePtr<Shader> m_pTextBillboardShader;
        ShaderConstant<mat4> m_textBillboardView;
        ShaderConstant<mat4> m_textBillboardProjection;
        ShaderConstant<vec2> m_textBillboardScreenSize;
        ShaderConstant<vec3> m_textBillboardPosition;
        ShaderConstant<Color> m_textBillboardColor;
        ShaderConstant<Texture2D> m_textBillboardTexture;

        volatile uint32 m_dataModelNum;

        void BeginDebugMarker(LPCWSTR name);
        void EndDebugMarker();


    public:
        Renderer(settings::Video& settings);

        // ISubsystem interface
        virtual void initialize() override;
        virtual void destroy() override;
        virtual void update(float elapsedTime) override;

        // Factory methods
        Texture2D* createTexture2D(const char* name, ITexture2DLoader* pLoader, TextureMode mode);
        Shader* createShader();
        Model* createModel();

        // helper methods

        RenderTextInfo prepareText(const char* text, FontHorizontalOrientation hOrientation);
        void renderText(const uvec2& screenPosition, Color color, const RenderTextInfo& info);
        void renderTextBillboard(const vec3& screenPosition, Color color, const RenderTextInfo& info);

        // IRenderer interface
        virtual IDebugRenderer& getDebugRenderer() override;
        virtual ResourcePtr<IModel> loadModel(const char* path) override;
        virtual ResourcePtr<IModel> loadModel(ReferenceCounted* pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices) override;

        virtual ResourcePtr<IResource> createGeneratedTexture(uint32 width, uint32 height, const char* resourceId, std::function<void(ArrayPtr<uint8>)> generatorFunction) override;

        virtual uint32 getScreenWidth() const override { return m_settings.screenResolution.x; }
        virtual uint32 getScreenHeight() const override { return m_settings.screenResolution.y; }

        virtual bool getVSyncEnabled() const override { return m_settings.vsyncEnabled; }
        virtual void setVSyncEnabled(bool value) override { m_settings.vsyncEnabled = value; }

        virtual bool getAdaptiveVSyncEnabled() const override { return m_settings.adaptiveVSyncEnabled; }
        virtual void setAdaptiveVSyncEnabled(bool value) override { m_settings.adaptiveVSyncEnabled = value; }

        virtual uvec2 toAbsoluteScreenPosition(const vec2& screenPosNormalized) const override;
        virtual gep::vec2 toNormalizedScreenPosition(const uvec2& screenPos) const override;

    };

    class DebugRenderer
        : public IDebugRenderer
    {
    private:
        struct LineGroup
        {
            Color color;
            ArrayPtr<LineInfo> lines;
        };
        struct LineGroup2D
        {
            Color color;
            ArrayPtr<LineInfo2D> lines;
        };

        struct TextInfo
        {
            enum TextType
            {
                INVALID,

                Text2D,
                Text3DBillboard,

                NUM_ELEMENTS
            };

            TextType type;

            // Uses only one of the following, according to the value set for 'type'
            vec2 pos2D; ///< normalized screen position
            vec3 pos3D;

            Color color;
            const char* text;

            TextInfo() :
                type(INVALID),
                pos2D(DO_NOT_INITIALIZE),
                pos3D(DO_NOT_INITIALIZE),
                color(1.0f, 1.0f, 1.0f, 1.0f),
                text("")
            {
            }

            TextInfo(TextType type) :
                type(type),
                pos2D(DO_NOT_INITIALIZE),
                pos3D(DO_NOT_INITIALIZE),
                color(1.0f, 1.0f, 1.0f, 1.0f),
                text("")
            {
            }
        };

        DynamicArray<LineGroup> m_lineGroups;
        DynamicArray<LineGroup2D> m_lineGroups2D;
        Color m_currentLineColor;
        Color m_currentLineColor2D;
        DynamicArray<LineInfo> m_tempLines;
        DynamicArray<LineInfo2D> m_tempLines2D;
        DynamicArray<TextInfo> m_tempText;
        DynamicArray<TextInfo> m_textInfos;
        StackAllocator m_frameAllocator;
        void* m_pStartMarker;
        Mutex m_mutex;

        void finishLineGroup();
        void finishLineGroup2D();
        void checkLineGroup(const Color& color);
        void checkLineGroup2D(const Color& color);
        void addLine(const vec3& start, const vec3& end);
        void addLine(const vec2& start, const vec2& end);
        
        void addText(TextInfo& textInfo);

    public:
        DebugRenderer();

        void extract(IRendererExtractor& extractor);

        virtual void drawLine(const vec3& start, const vec3& end, Color color = Color::white()) override;
        virtual void drawLine(const vec2& start, const vec2& end, Color color = Color::white()) override;
        virtual void drawArrow(const vec3& start, const vec3& end, Color color = Color::white()) override;
        virtual void drawBox(const vec3& min, const vec3& max, Color color = Color::white()) override;

        virtual void printText(const vec2& screenPositionNormalized, const char* text, Color color = Color::white()) override;
        virtual void printText(const vec3& worldPosition, const char* text, Color color = Color::white()) override;

        virtual void drawLocalAxes(
            const vec3& objectPosition,
            const Quaternion& objectRotation,
            float axesScale = 10.0f,
            Color colorX = Color::red(),
            Color colorY = Color::green(),
            Color colorZ = Color::blue()) override;

        virtual void drawLocalAxes(
            const vec3& objectPosition,
            float axesScale = 10.0f,
            Color colorX = Color::red(),
            Color colorY = Color::green(),
            Color colorZ = Color::blue()) override;

    };
}
