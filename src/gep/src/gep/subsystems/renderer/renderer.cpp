#include "stdafx.h"
#include "gepimpl/subsystems/renderer/renderer.h"

#include "gep/exception.h"
#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/logging.h"
#include "gep/timer.h"
#include "gepimpl/subsystems/renderer/texture2d.h"
#include "gepimpl/subsystems/renderer/font.h"
#include "gepimpl/subsystems/renderer/shader.h"
#include "gepimpl/subsystems/renderer/vertexbuffer.h"
#include "gepimpl/subsystems/renderer/model.h"
#include "gepimpl/subsystems/renderer/extractor.h"
#include "gep/modelloader.h"
#include "gep/math3d/algorithm.h"
#include "gep/settings.h"
#include "gep/container/DynamicArray.h"

struct ID3DUserDefinedAnnotation : public IUnknown
{
public:
    virtual INT STDMETHODCALLTYPE BeginEvent( _In_  LPCWSTR Name) = 0;   
    virtual INT STDMETHODCALLTYPE EndEvent( void) = 0;    
    virtual void STDMETHODCALLTYPE SetMarker( _In_  LPCWSTR Name) = 0;  
    virtual BOOL STDMETHODCALLTYPE GetStatus( void) = 0;   
};

#include "effects11/d3dx11effect.h"
#include <dxgidebug.h>

#ifdef _DEBUG
extern D3DPERF_EndEvent_Func D3DPREF_EndEvent = nullptr;
extern D3DPERF_BeginEvent_Func D3DPREF_BeginEvent = nullptr;
#endif


// Use this to enable or disable verbose debug messages from the renderer.
#define GEP_VERBOSE_LOG_MESSAGE(...)
//#define GEP_VERBOSE_LOG_MESSAGE g_globalManager.getLogging()->logMessage

gep::Renderer::Renderer(settings::Video& settings)
    :
    m_settings(settings),
    m_actualVSync(settings.vsyncEnabled),
    m_pd3dDevice(nullptr),
    m_pSwapChain(nullptr),
    m_pRenderTargetView(nullptr),
    m_pDepthStencilView(nullptr),
    m_pUserDefinedAnnotation(nullptr),
    m_pDeviceContext(nullptr),
    m_pDummyTexture(nullptr),
    m_pDummyShader(nullptr),
    m_pFontBuffer(nullptr),
    m_pLinesBuffer(nullptr),
    m_pLines2DBuffer(nullptr),
    m_isFontBufferOutOfDate(false),
    m_hWnd(nullptr),
    m_dataModelNum(0)
{
}

gep::IDebugRenderer& gep::Renderer::getDebugRenderer()
{
    GEP_ASSERT(m_pDebugRenderer != nullptr);
    return *m_pDebugRenderer;
}

void gep::Renderer::initialize()
{
    m_pDebugRenderer = new DebugRenderer();
    createWindow();
    initD3DDevice();

    g_globalManager.getLogging()->logMessage("Using DirectX Version: %d.%d sdk %d", D3D11_MAJOR_VERSION, D3D11_MINOR_VERSION, D3D11_SDK_VERSION);

    // Create the dummy 2d texture
    {
        m_pDummyTexture = createTexture2D("dummy texture 2d", new DummyTexture2DLoader(), TextureMode::Static);
        m_pDummyTexture->createEmpty(4, 4, ImageFormat::RGBA8);
        auto& dummyData = m_pDummyTexture->getImageData().getData()[0];
        for(uint32 i = 0; i < 4 * 4 * 4; i+=4)
        {
            dummyData[i] = 0xFF;   //R
            dummyData[i+1] = 0;    //G
            dummyData[i+2] = 0xFF; //B
            dummyData[i+3] = 0xFF; //A
        }
        m_pDummyTexture->setHasData(true);
        m_pDummyTexture->finalize();
        g_globalManager.getResourceManager()->registerResourceType("Texture2D", m_pDummyTexture);
    }

    // Creates the dummy shader
    {
        m_pDummyShader = createShader();
        m_pDummyShader->setLoader(new ShaderFileLoader("data/base/dummy.fx"));
        m_pDummyShader->getLoader()->loadResource(m_pDummyShader);
        m_pDummyShader->finalize();
        g_globalManager.getResourceManager()->registerResourceType("Shader", m_pDummyShader);
    }

    // Register the font resource type (no fallback)
    g_globalManager.getResourceManager()->registerResourceType("Font", nullptr);

    // load engine resources
    m_pDefaultFont = g_globalManager.getResourceManager()->loadResource<Font>(FontFileLoader("data/base/dejavusans.ttf", 11), LoadAsync::No);
    m_pFontShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/base/font.fx"), LoadAsync::No);

    g_globalManager.getResourceManager()->finalizeResourcesWithFlags(ResourceFinalize::FromRenderer);

    m_fontPosition = ShaderConstant<vec2>("position", m_pFontShader);
    m_fontColor = ShaderConstant<Color>("color", m_pFontShader);
    m_fontScreenSize = ShaderConstant<vec2>("targetSize", m_pFontShader);
    m_fontTexture = ShaderConstant<Texture2D>("diffuse", m_pFontShader);
    m_fontScreenSize.set(vec2((float)m_settings.screenResolution.x,
                              (float)m_settings.screenResolution.y));

    // text billboard
    m_pTextBillboardShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/base/fontBillboard.fx"), LoadAsync::No);
    m_textBillboardView = ShaderConstant<mat4>("View", m_pTextBillboardShader);
    m_textBillboardProjection = ShaderConstant<mat4>("Projection", m_pTextBillboardShader);
    m_textBillboardPosition = ShaderConstant<vec3>("position", m_pTextBillboardShader);
    m_textBillboardColor = ShaderConstant<Color>("color", m_pTextBillboardShader);
    m_textBillboardTexture = ShaderConstant<Texture2D>("diffuse", m_pTextBillboardShader);
    m_textBillboardScreenSize = ShaderConstant<vec2>("targetSize", m_pTextBillboardShader);
    m_textBillboardScreenSize.set(vec2((float)m_settings.screenResolution.x,
                                       (float)m_settings.screenResolution.y));
    {
        auto aspectRatio = float(m_settings.screenResolution.x) / float(m_settings.screenResolution.y);
        m_projection = mat4::projectionMatrix(60.0f, aspectRatio, 0.1f, 10000.0f);
    }
    m_view = mat4::lookAtMatrix(vec3(300, 0, 205), vec3(0, 0, 150), vec3(0,0,1));

    {
        Vertexbuffer::DataChannel dataChannels[] =
        { Vertexbuffer::DataChannel::POSITION_2D,
        Vertexbuffer::DataChannel::TEXCOORD0 };
        m_pFontBuffer = new Vertexbuffer(m_pd3dDevice, dataChannels, Vertexbuffer::Primitive::Triangle, Vertexbuffer::Usage::Dynamic);
    }

    {
        Vertexbuffer::DataChannel dataChannels[] =
        {
            Vertexbuffer::DataChannel::POSITION
        };
        m_pLinesBuffer = new Vertexbuffer(m_pd3dDevice, dataChannels, Vertexbuffer::Primitive::Line, Vertexbuffer::Usage::Dynamic);
    }

    {
        Vertexbuffer::DataChannel dataChannels[] =
        {
            Vertexbuffer::DataChannel::POSITION_2D
        };
        m_pLines2DBuffer = new Vertexbuffer(m_pd3dDevice, dataChannels, Vertexbuffer::Primitive::Line, Vertexbuffer::Usage::Dynamic);
    }

    {
        m_pDummyModel = createModel();
        m_pDummyModel->loadFile("data/base/dummy.thModel");
        m_pDummyModel->setLoader(ModelDummyLoader().moveToHeap());
        m_pDummyModel->getLoader()->loadResource(m_pDummyModel);
        m_pDummyModel->getMaterial(0).setShader(m_pDummyShader->makeResourcePtrFromThis<Shader>());
        m_pDummyModel->finalize();
        g_globalManager.getResourceManager()->registerResourceType("Model", m_pDummyModel);
    }

    //Loading additional resources
    m_pLightingShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/shaders/lighting.fx"));
    m_pLightingAnimatedShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/shaders/lightingAnimated.fx"));
    m_pLinesShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/base/lines.fx"));
    m_pWireframeShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/shaders/wireframe.fx"));
    m_lineColor = ShaderConstant<Color>("Color", m_pLinesShader);
    m_lineView = ShaderConstant<mat4>("View", m_pLinesShader);
    m_lineProjection = ShaderConstant<mat4>("Projection", m_pLinesShader);

    m_pLines2DShader = g_globalManager.getResourceManager()->loadResource<Shader>(ShaderFileLoader("data/base/lines2D.fx"));
    m_line2DColor = ShaderConstant<Color>("Color", m_pLines2DShader);
    m_lines2DScreenSize = ShaderConstant<vec2>("targetSize", m_pLines2DShader);
    m_lines2DScreenSize.set(vec2((float)m_settings.screenResolution.x,
                                 (float)m_settings.screenResolution.y));

    #ifdef _DEBUG
    const GUID ID_ID3DUserDefinedAnnotation = { 0xb2daad8b, 0x03d4, 0x4dbf, { 0x95, 0xeb,  0x32,  0xab,  0x4b,  0x63,  0xd0,  0xab } };
    m_pDeviceContext->QueryInterface(ID_ID3DUserDefinedAnnotation, (void**)&m_pUserDefinedAnnotation);
    if(m_pUserDefinedAnnotation == nullptr || !m_pUserDefinedAnnotation->GetStatus())
    {
        GEP_RELEASE_AND_NULL(m_pUserDefinedAnnotation);
        HMODULE pModule = LoadLibraryA("d3d9.dll");
        D3DPREF_BeginEvent = (D3DPERF_BeginEvent_Func)GetProcAddress(pModule, "D3DPERF_BeginEvent");
        D3DPREF_EndEvent = (D3DPERF_EndEvent_Func)GetProcAddress(pModule, "D3DPERF_EndEvent");
    }
    #endif

}

void gep::Renderer::destroy()
{
    m_pDummyTexture = nullptr;
    m_pDummyShader = nullptr;
    m_pDummyModel = nullptr;

    DELETE_AND_NULL(m_pFontBuffer);
    DELETE_AND_NULL(m_pLinesBuffer);
    DELETE_AND_NULL(m_pLines2DBuffer);

    GEP_RELEASE_AND_NULL(m_pRenderTargetView);
    GEP_RELEASE_AND_NULL(m_pDepthStencilView);
    GEP_RELEASE_AND_NULL(m_pSwapChain);
    if(m_pDeviceContext)
    {
        m_pDeviceContext->ClearState();
        m_pDeviceContext->Flush();
        m_pDeviceContext->Release();
        m_pDeviceContext = nullptr;
    }
    if(m_pd3dDevice) {
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr;
    }
    destroyWindow();


    const GUID DXGI_DEBUG_ALL2 = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6,  0x43,  0xe9,  0xa9,  0xcf,  0xda,  0x8 } };

    delete m_pDebugRenderer; m_pDebugRenderer = nullptr;
#ifdef _DEBUG
    //ComLeakFinder::destroyInstance();
    auto handle = GetModuleHandle(L"dxgidebug.dll");
    if(handle != INVALID_HANDLE_VALUE)
    {
        auto fun = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(handle, "DXGIGetDebugInterface"));
        if (fun) // TODO FIXME: "DXGIGetDebugInterface" not found on certain systems
        {
            IDXGIDebug* pDebug = nullptr;
            fun(__uuidof(IDXGIDebug), (void**)&pDebug);
            if (pDebug)
            {
                pDebug->ReportLiveObjects(DXGI_DEBUG_ALL2, DXGI_DEBUG_RLO_ALL);
            }
        }
    }
#endif
}

void gep::Renderer::update(float elapsedTime)
{
    float time = g_globalManager.getTimer().getTimeAsFloat() / 500.0f ;
    //m_view = mat4::lookAtMatrix(vec3(sin(time) * 20, cos(time) * 20, 205), vec3(0, 0, 200), vec3(0,0,1));
    //m_view = mat4::lookAtMatrix(vec3(sin(time) * 20, cos(time) * 20, 5), vec3(0, 0, 5), vec3(0,0,1));

    // Main message loop
    MSG msg = {0};
    while( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
    {
        if( msg.message == WM_QUIT )
        {
            g_globalManager.getUpdateFramework()->stop();
        }
        else
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    //finalize all renderer resources
    BeginDebugMarker(L"finalize resources");
    g_globalManager.getResourceManager()->finalizeResourcesWithFlags( ResourceFinalize::FromRenderer );
    EndDebugMarker();

    if (m_settings.vsyncEnabled)
    {
        m_actualVSync = true;
    }
    else if (m_settings.adaptiveVSyncEnabled)
    {
        auto currentFPS = 1.0f / g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60);
        if(currentFPS > m_settings.adaptiveVSyncThreshold + m_settings.adaptiveVSyncTolerance)
        {
            m_actualVSync = true;
        }
        else if (currentFPS < m_settings.adaptiveVSyncThreshold - m_settings.adaptiveVSyncTolerance)
        {
            m_actualVSync = false;
        }
    }
    else
    {
        m_actualVSync = false;
    }

    GEP_VERBOSE_LOG_MESSAGE(
        "==============================\n"
        "%20s: %s\n"
        "%20s: %s\n"
        "%20s: %f\n"
        "%20s: %f\n"
        "%20s: %f\n"
        "%20s: %s",
        "VSync",                    m_settings.vsyncEnabled ? "yes" : "no",
        "Adaptive VSync",           m_settings.adaptiveVSyncEnabled ? "yes" : "no",
        "Elapsed Time",             elapsedTime,
        "Adaptive VSync Threshold", m_settings.adaptiveVSyncThreshold,
        "Adaptive VSync Tolerance", m_settings.adaptiveVSyncTolerance,
        "-> Result",                m_actualVSync ? "will sync" : "no sync");

    render();
}

void gep::Renderer::createWindow()
{
    m_hInstance = GetModuleHandle(nullptr);
    auto szWindowClass = L"GEP";

    //
    // Register class
    //
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = m_hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_WINLOGO);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(NULL, IDI_WINLOGO);
    if( !RegisterClassEx(&wcex) )
        throw Exception("couldn't create window class");

    //
    // Create window
    //
    RECT rc = { 0, 0, m_settings.screenResolution.x, m_settings.screenResolution.y };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    auto title = g_globalManager.getSettings()->getGeneralSettings().applicationTitle;
    m_hWnd = CreateWindow( szWindowClass, title.c_str(), WS_OVERLAPPEDWINDOW,
        m_settings.initialRenderWindowPosition.x, m_settings.initialRenderWindowPosition.y,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL,
        m_hInstance, NULL);

    if( !m_hWnd )
        throw Exception("Failed to create window");

    ShowWindow( m_hWnd, SW_SHOW );
    ShowCursor(FALSE);
}

void gep::Renderer::destroyWindow()
{
    if( m_hWnd )
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

void gep::Renderer::initD3DDevice()
{
    HRESULT hr = S_OK;;

    RECT rc;
    GetClientRect( m_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    GEP_ASSERT(width == m_settings.screenResolution.x);
    GEP_ASSERT(height == m_settings.screenResolution.y);

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr,
        0, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &m_featureLevel, &m_pDeviceContext);

    if( FAILED( hr ) )
        throw Exception("failed to create DirectX 11 device");

    const char* featureLevelName = "";
    switch(m_featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_0:
        featureLevelName = "DX 11.0";
        break;
    case D3D_FEATURE_LEVEL_10_1:
        featureLevelName = "DX 10.1";
        break;
    case D3D_FEATURE_LEVEL_10_0:
        featureLevelName = "DX 10.0";
        break;
    case D3D_FEATURE_LEVEL_9_3:
        featureLevelName = "DX 9.3";
        break;
    case D3D_FEATURE_LEVEL_9_2:
        featureLevelName = "DX 9.2";
        break;
    case D3D_FEATURE_LEVEL_9_1:
        featureLevelName = "DX 9.1";
        break;
    }

    g_globalManager.getLogging()->logMessage("Initialized DirectX with feature level %s", featureLevelName);

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        throw Exception("failed to create main DirectX rendertarget");

    hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
    pBackBuffer->Release(); pBackBuffer = nullptr;
    if( FAILED( hr ) )
        throw Exception("failed to create render target view");

    //create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ID3D11Texture2D* pDepthStencil = nullptr;

    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    if( FAILED( m_pd3dDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil ) ) )
        throw Exception("Could not create depth stencil texture");

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    descDSV.Flags = 0;

    if( FAILED( m_pd3dDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &m_pDepthStencilView ) ) )
        throw Exception("Could not create depth stencil view");
    pDepthStencil->Release(); pDepthStencil = nullptr;

    m_pDeviceContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pDeviceContext->RSSetViewports( 1, &vp );
}

void gep::Renderer::render()
{
    m_pFontBuffer->getData().resize(0);
    m_pFontBuffer->getIndices().resize(0);
    m_pLinesBuffer->getData().resize(0);
    m_pLines2DBuffer->getData().resize(0);

    {
        auto& extractor = *static_cast<RendererExtractor*>(g_globalManager.getRendererExtractor());

        CommandBase* firstCommand = extractor.startReadCommands();
        SCOPE_EXIT { extractor.endReadCommands(); });

        prepareCommands(extractor, firstCommand);

        m_pFontBuffer->upload(m_pDeviceContext);
        m_pLinesBuffer->upload(m_pDeviceContext);
        m_pLines2DBuffer->upload(m_pDeviceContext);

        m_fontScreenSize.set(vec2((float)m_settings.screenResolution.x,
                                  (float)m_settings.screenResolution.y));
        m_textBillboardScreenSize.set(vec2((float)m_settings.screenResolution.x,
                                           (float)m_settings.screenResolution.y));
        m_lines2DScreenSize.set(vec2((float)m_settings.screenResolution.x,
                                     (float)m_settings.screenResolution.y));

        // Just clear the backbuffer
        m_pDeviceContext->ClearRenderTargetView( m_pRenderTargetView, m_settings.clearColor.data );
        m_pDeviceContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0 );

        executeCommands(extractor, firstCommand);
        execute2DCommands(extractor, firstCommand);
    }

    m_pSwapChain->Present(m_actualVSync ? 1 : 0, 0);
}

void gep::Renderer::prepareCommands(RendererExtractor& extractor, CommandBase* currentCommand)
{
    BeginDebugMarker(L"prepareCommands");
    SCOPE_EXIT{ EndDebugMarker(); });
    while(currentCommand != nullptr)
    {
        switch(currentCommand->getType())
        {
        case CommandType::Text:
            {
                auto cmd = RendererExtractor::command_cast<CommandDrawText>(currentCommand);
                cmd->textInfo = prepareText(cmd->text, FontHorizontalOrientation::Left);
            }
            break;
        case CommandType::TextBillboard:
            {
                auto cmd = RendererExtractor::command_cast<CommandDrawTextBillboard>(currentCommand);
                cmd->textInfo = prepareText(cmd->text, FontHorizontalOrientation::Centered);
            }
            break;
        case CommandType::RenderLines:
            {
                auto cmd = RendererExtractor::command_cast<CommandRenderLines>(currentCommand);
                cmd->startIndex = m_pLinesBuffer->getCurrentNumVertices();
                static_assert(sizeof(LineInfo) == sizeof(float) * 6, "the following append assumes this");
                m_pLinesBuffer->getData().append(ArrayPtr<float>((float*)cmd->lines.getPtr(), sizeof(float) * 6 * cmd->lines.length()));
            }
            break;
        case CommandType::RenderLines2D:
            {
                auto cmd = RendererExtractor::command_cast<CommandRenderLines2D>(currentCommand);
                cmd->startIndex = m_pLines2DBuffer->getCurrentNumVertices();
                static_assert(sizeof(LineInfo2D) == sizeof(float) * 4, "the following append assumes this");
                m_pLines2DBuffer->getData().append(ArrayPtr<float>((float*)cmd->lines.getPtr(), sizeof(float) * 4 * cmd->lines.length()));
            }
            break;
        default:
            // do nothing
            break;
        }

        currentCommand = extractor.nextCommand(currentCommand);
    }
}

void gep::Renderer::BeginDebugMarker(LPCWSTR name)
{
    #ifdef _DEBUG
    if(m_pUserDefinedAnnotation)
    {
        m_pUserDefinedAnnotation->BeginEvent(name);
    }
    else if (D3DPREF_BeginEvent)
    {
        D3DPREF_BeginEvent(0x0000FF00, name);
    }
    #endif
}

void gep::Renderer::EndDebugMarker()
{
    #ifdef _DEBUG
    if(m_pUserDefinedAnnotation)
    {
        m_pUserDefinedAnnotation->EndEvent();
    }
    else if (D3DPREF_EndEvent)
    {
        D3DPREF_EndEvent();
    }
    #endif
}

void gep::Renderer::executeCommands(RendererExtractor& extractor, CommandBase* currentCommand)
{
    BeginDebugMarker(L"3D Rendering");
    SCOPE_EXIT{ EndDebugMarker(); });
    while(currentCommand != nullptr)
    {
        switch(currentCommand->getType())
        {
        case CommandType::RenderModel:
            {
                auto cmd = RendererExtractor::command_cast<CommandRenderModel>(currentCommand);
                
                // We have to use different transformations for animated/static models
                // since havok's combined matrices are in another space...
                // TODO: Find a better way to process havok's transformations correctly
                if(cmd->model->hasBones())
                {
                    auto modelMatrix = cmd->modelMatrix * mat4::rotationMatrixXYZ(vec3(-90, 0, 0));
                    cmd->model->draw(modelMatrix, cmd->bones, m_view, m_projection, m_pDeviceContext);
                }
                else
                {
                    cmd->model->draw(cmd->modelMatrix, cmd->bones, m_view, m_projection, m_pDeviceContext);
                }
            }
            break;
    
        case CommandType::Camera:
            {
                auto cmd = RendererExtractor::command_cast<CommandCamera>(currentCommand);
                m_view = cmd->viewMatrix;
                m_projection = cmd->projectionMatrix;
            }
            break;
        case CommandType::DebugMarkerBegin:
            {
                auto cmd = RendererExtractor::command_cast<CommandDebugMarkerBegin>(currentCommand);
                BeginDebugMarker(cmd->name);
            }
            break;
        case CommandType::DebugMarkerEnd:
            EndDebugMarker();
            break;
        // Skip 2d commands
        case CommandType::RenderLines:
        case CommandType::TextBillboard:
        case CommandType::RenderLines2D:
        case CommandType::Text:
            break;
        default:
            GEP_ASSERT(false, "unhandeled command type");
            break;
        }

        currentCommand = extractor.nextCommand(currentCommand);
    }
}

void gep::Renderer::execute2DCommands(RendererExtractor& extractor, CommandBase* currentCommand)
{
    BeginDebugMarker(L"2D Rendering");
    SCOPE_EXIT{ EndDebugMarker(); });
    bool isFirstLineDrawCall = true;
    bool isFirstTextBillboardDrawCall = true;
    while(currentCommand != nullptr)
    {
        switch(currentCommand->getType())
        {
        // skip 3d commands
        case CommandType::RenderModel:
        case CommandType::Camera:
            break;
        case CommandType::DebugMarkerBegin:
            {
                auto cmd = RendererExtractor::command_cast<CommandDebugMarkerBegin>(currentCommand);
                BeginDebugMarker(cmd->name);
            }
            break;
        case CommandType::DebugMarkerEnd:
            EndDebugMarker();
            break;
        case CommandType::RenderLines:
            {
                if(isFirstLineDrawCall)
                {
                    isFirstLineDrawCall = false;
                    m_lineView.set(m_view);
                    m_lineProjection.set(m_projection);
                }
                auto cmd = RendererExtractor::command_cast<CommandRenderLines>(currentCommand);
                m_pLinesBuffer->use(m_pDeviceContext);
                m_lineColor.set(cmd->color);
                m_pLinesShader->use(m_pDeviceContext, m_pLinesBuffer);
                m_pLinesBuffer->draw(m_pDeviceContext, cmd->startIndex, (uint32)cmd->lines.length() * 2);
            }
            break;
        case CommandType::TextBillboard:
            {
                if (isFirstTextBillboardDrawCall)
                {
                    isFirstTextBillboardDrawCall = false;
                    m_textBillboardView.set(m_view);
                    m_textBillboardProjection.set(m_projection);
                }
                auto cmd = RendererExtractor::command_cast<CommandDrawTextBillboard>(currentCommand);
                renderTextBillboard(cmd->position, cmd->color, cmd->textInfo);
            }
            break;
        case CommandType::Text:
            {
                auto cmd = RendererExtractor::command_cast<CommandDrawText>(currentCommand);
                renderText(cmd->position, cmd->color, cmd->textInfo);
            }
            break;
        case CommandType::RenderLines2D:
            {
                auto cmd = RendererExtractor::command_cast<CommandRenderLines2D>(currentCommand);
                m_pLines2DBuffer->use(m_pDeviceContext);
                m_line2DColor.set(cmd->color);
                m_pLines2DShader->use(m_pDeviceContext, m_pLines2DBuffer);
                m_pLines2DBuffer->draw(m_pDeviceContext, cmd->startIndex, (uint32)cmd->lines.length() * 2);
            }
            break;
        default:
            GEP_ASSERT(false, "unhandeled command type");
            break;
        }

        currentCommand = extractor.nextCommand(currentCommand);
    }
}


LRESULT CALLBACK gep::Renderer::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

gep::Texture2D* gep::Renderer::createTexture2D(const char* name, ITexture2DLoader* pLoader, TextureMode mode)
{
    return new Texture2D(name, pLoader, m_pd3dDevice, m_pDeviceContext, mode);
}

gep::Shader* gep::Renderer::createShader()
{
    return new Shader(m_pd3dDevice);
}

gep::Model* gep::Renderer::createModel()
{
    return new Model(m_pd3dDevice, m_pDeviceContext);
}

gep::RenderTextInfo gep::Renderer::prepareText(const char* text, FontHorizontalOrientation hOrientation)
{
    GEP_ASSERT(m_pFontBuffer != nullptr);
    RenderTextInfo result;
    result.startIndex = (uint32)m_pFontBuffer->getIndices().length();
    m_pDefaultFont->print(*m_pFontBuffer, text, hOrientation);
    result.numIndices = (uint32)(m_pFontBuffer->getIndices().length() - result.startIndex);
    return result;
}

void gep::Renderer::renderText(const uvec2& screenPosition, Color color, const RenderTextInfo& info)
{
    m_fontPosition.set(vec2((float)screenPosition.x, (float)screenPosition.y));
    m_fontColor.set(color);
    m_fontTexture.set(m_pDefaultFont->getFontTexture());
    m_pFontShader->use(m_pDeviceContext, m_pFontBuffer);
    m_pFontBuffer->use(m_pDeviceContext);
    m_pFontBuffer->draw(m_pDeviceContext, info.startIndex, info.numIndices);
}

void gep::Renderer::renderTextBillboard(const vec3& screenPosition, Color color, const RenderTextInfo& info)
{
    m_textBillboardPosition.set(screenPosition);
    m_textBillboardColor.set(color);
    m_textBillboardTexture.set(m_pDefaultFont->getFontTexture());
    m_pTextBillboardShader->use(m_pDeviceContext, m_pFontBuffer);
    m_pFontBuffer->use(m_pDeviceContext);
    m_pFontBuffer->draw(m_pDeviceContext, info.startIndex, info.numIndices);
}

gep::ResourcePtr<gep::IModel> gep::Renderer::loadModel(const char* path)
{
    auto model = g_globalManager.getResourceManager()->loadResource<Model>(ModelFileLoader(path), LoadAsync::No);
    if(model == m_pDummyModel)
        return model;
    uint32 matId = 0;

    gep::ResourcePtr<Shader> shader;
    if (model->hasBones())
    {
        shader = m_pLightingAnimatedShader;
    }
    else
    {
        shader = m_pLightingShader;
    }

    auto diffuseConstant = ShaderConstant<Texture2D>("diffuse", shader);
    for(auto& matInfo : model->getMaterialInfo())
    {
        model->getMaterial(matId).setShader(shader);
        for(auto& tex : matInfo.textures)
        {
            if(tex.semantic == TextureType::DIFFUSE)
            {
                auto pTexture = g_globalManager.getResourceManager()->loadResource<Texture2D>(Texture2DFileLoader(tex.file));
                model->getMaterial(matId).addTexture(diffuseConstant, pTexture);
            }
        }
        matId++;
    }
    return model;
}
 gep::ResourcePtr<gep::IModel>  gep::Renderer::loadModel( gep::ReferenceCounted* pDataHolder,  gep::ArrayPtr< gep::vec4> vertices,  gep::ArrayPtr< gep::uint32> indices)
 {
    auto id = InterlockedIncrement(&m_dataModelNum);
    char idString[256];
    sprintf(idString, "DataModel%d", id);
    auto model = g_globalManager.getResourceManager()->loadResource<Model>(ModelLoaderFromData(pDataHolder,vertices, indices, idString) , LoadAsync::No);
    if(model == m_pDummyModel)
        return model;
    uint32 matId = 0;
    model->getMaterial(0).setShader(m_pWireframeShader);
   
    return model;
}

gep::ResourcePtr<gep::IResource> gep::Renderer::createGeneratedTexture(uint32 width, uint32 height, const char* resourceId, std::function<void(ArrayPtr<uint8>)> generatorFunction)
{
    GeneratorTextureLoader loader(width, height, generatorFunction, resourceId);
    return g_globalManager.getResourceManager()->loadResource<Texture2D>(loader, LoadAsync::No);
}

gep::uvec2 gep::Renderer::toAbsoluteScreenPosition(const vec2& screenPosNormalized) const
{
    GEP_ASSERT(screenPosNormalized.x >= -1.0f && screenPosNormalized.x <= 1.0f);
    GEP_ASSERT(screenPosNormalized.y >= -1.0f && screenPosNormalized.y <= 1.0f);

    auto x(m_settings.screenResolution.x * ((screenPosNormalized.x + 1.0f) / 2.0f));
    auto y(m_settings.screenResolution.y * (1.0f - ((screenPosNormalized.y + 1.0f) / 2.0f)));

    uvec2 result(static_cast<uvec2::component_t>(x), static_cast<uvec2::component_t>(y));
    return result;
}

gep::vec2 gep::Renderer::toNormalizedScreenPosition(const uvec2& screenPos) const
{
    GEP_ASSERT(screenPos.x <= m_settings.screenResolution.x);
    GEP_ASSERT(screenPos.y <= m_settings.screenResolution.y);

    auto x(screenPos.x * 2.0f / m_settings.screenResolution.x - 1.0f);
    auto y(1.0f - 2.0f * screenPos.y / m_settings.screenResolution.y);

    vec2 result(static_cast<vec2::component_t>(x), static_cast<vec2::component_t>(y));
    return result;
}


gep::DebugRenderer::DebugRenderer() :
    m_frameAllocator(true, 512 * 1024 * 1024)
{
    m_pStartMarker = m_frameAllocator.getMarker();
    g_globalManager.getRendererExtractor()->registerExtractionCallback(std::bind(&DebugRenderer::extract, this, std::placeholders::_1));
}

void gep::DebugRenderer::finishLineGroup()
{
    if(m_tempLines.length() > 0)
    {
        LineInfo* lines = (LineInfo*)m_frameAllocator.allocateMemory(sizeof(LineInfo) * m_tempLines.length());
        memcpy(lines, m_tempLines.begin(), sizeof(LineInfo) * m_tempLines.length());

        LineGroup group;
        group.color = m_currentLineColor;
        group.lines = ArrayPtr<LineInfo>(lines, m_tempLines.length());
        m_tempLines.resize(0);

        m_lineGroups.append(group);
    }
}

void gep::DebugRenderer::finishLineGroup2D()
{
    if(m_tempLines2D.length() > 0)
    {
        LineInfo2D* lines = (LineInfo2D*)m_frameAllocator.allocateMemory(sizeof(LineInfo2D) * m_tempLines2D.length());
        memcpy(lines, m_tempLines2D.begin(), sizeof(LineInfo2D) * m_tempLines2D.length());

        LineGroup2D group;
        group.color = m_currentLineColor2D;
        group.lines = ArrayPtr<LineInfo2D>(lines, m_tempLines2D.length());
        m_tempLines2D.resize(0);

        m_lineGroups2D.append(group);
    }
}

void gep::DebugRenderer::addLine(const vec3& start, const vec3& end)
{
    LineInfo line;
    line.start = start;
    line.end = end;
    m_tempLines.append(line);
}

void gep::DebugRenderer::addLine(const vec2& start, const vec2& end)
{
    LineInfo2D line;
    line.start = start;
    line.end = end;
    m_tempLines2D.append(line);
}


void gep::DebugRenderer::addText(TextInfo& textInfo)
{
    // Need to copy string, since it is only allocated temporarily on the stack by the caller.
    size_t size = strlen(textInfo.text);
    char* text = static_cast<char*>(m_frameAllocator.allocateMemory(size + 1));
    text[size] = '\0';
    MemoryUtils::copy(text, textInfo.text, size);
    textInfo.text = text;

    ScopedLock<Mutex> lock(m_mutex);
    m_textInfos.append(textInfo);
}


void gep::DebugRenderer::checkLineGroup(const Color& color)
{
    if(m_currentLineColor != color)
    {
        finishLineGroup();
        m_currentLineColor = color;
    }
}

void gep::DebugRenderer::checkLineGroup2D(const Color& color)
{
    if (m_currentLineColor2D != color)
    {
        finishLineGroup2D();
        m_currentLineColor2D = color;
    }
}



void gep::DebugRenderer::drawLine(const vec3& start, const vec3& end, Color color)
{
    ScopedLock<Mutex> lock(m_mutex);
    checkLineGroup(color);
    addLine(start, end);
}

void gep::DebugRenderer::drawLine(const vec2& start, const vec2& end, Color color)
{
    ScopedLock<Mutex> lock(m_mutex);
    checkLineGroup2D(color);
    addLine(start, end);
}

void gep::DebugRenderer::drawArrow(const vec3& start, const vec3& end, Color color)
{
    vec3 x,y;
    vec3 dir = end - start;
    float len = dir.length() * 0.1f;
    auto arrowEnd = start + (dir * 0.9f);
    if( vec3(0,0,1).dot(dir) < 0.01f )
    {
        x = dir.cross(vec3(1,0,0)).normalized();
        y = dir.cross(x).normalized();
    }
    else
    {
        x = dir.cross(vec3(0,0,1)).normalized();
        y = dir.cross(x).normalized();
    }

    ScopedLock<Mutex> lock(m_mutex);
    checkLineGroup(color);

    addLine(start, end);
    addLine(end, arrowEnd + (x * len));
    addLine(end, arrowEnd + (x * -len));
    addLine(end, arrowEnd + (y * len));
    addLine(end, arrowEnd + (y * -len));
}

void gep::DebugRenderer::drawBox(const vec3& min, const vec3& max, Color color)
{
    ScopedLock<Mutex> lock(m_mutex);
    checkLineGroup(color);

    addLine(min                    , vec3(max.x,min.y,min.z));
    addLine(vec3(max.x,min.y,min.z), vec3(max.x,max.y,min.z));
    addLine(vec3(max.x,max.y,min.z), vec3(min.x,max.y,min.z));
    addLine(vec3(min.x,max.y,min.z), min);
    addLine(vec3(min.x,min.y,max.z), vec3(max.x,min.y,max.z));
    addLine(vec3(max.x,min.y,max.z), max);
    addLine(max                    , vec3(min.x,max.y,max.z));
    addLine(vec3(min.x,max.y,max.z), vec3(min.x,min.y,max.z));
    addLine(min                    , vec3(min.x,min.y,max.z));
    addLine(vec3(max.x,min.y,min.z), vec3(max.x,min.y,max.z));
    addLine(vec3(max.x,max.y,min.z), max);
    addLine(vec3(min.x,max.y,min.z), vec3(min.x,max.y,max.z));
}

void gep::DebugRenderer::printText(const vec2& screenPosition, const char* text, Color color)
{

    TextInfo ti(TextInfo::Text2D);
    ti.pos2D = screenPosition;
    ti.color = color;
    ti.text = text;

    addText(ti);

}

void gep::DebugRenderer::printText(const vec3& worldPosition, const char* text, Color color)
{

    TextInfo ti(TextInfo::Text3DBillboard);
    ti.pos3D = worldPosition;
    ti.color = color;
    ti.text = text;

    addText(ti);

}

void gep::DebugRenderer::drawLocalAxes(const vec3& objectPosition,
                                       const Quaternion& objectRotation,
                                       float axesScale /*= 10.0f*/,
                                       Color colorX /*= Color::red()*/,
                                       Color colorY /*= Color::green()*/,
                                       Color colorZ /*= Color::blue() */)
{
    auto rotMatrix = objectRotation.toMat3();
    gep::vec3 axisX = rotMatrix * vec3(axesScale, 0.0f,      0.0f     ); // x
    gep::vec3 axisY = rotMatrix * vec3(0.0f,      axesScale, 0.0f     ); // y
    gep::vec3 axisZ = rotMatrix * vec3(0.0f,      0.0f,      axesScale); // z
    drawLine(objectPosition, objectPosition + axisX, colorX);
    drawLine(objectPosition, objectPosition + axisY, colorY);
    drawLine(objectPosition, objectPosition + axisZ, colorZ);
}

void gep::DebugRenderer::drawLocalAxes(const vec3& objectPosition,
                                       float axesScale /*= 10.0f*/,
                                       Color colorX /*= Color::red()*/,
                                       Color colorY /*= Color::green()*/,
                                       Color colorZ /*= Color::blue() */)
{
    gep::vec3 axisX = vec3(axesScale, 0.0f,      0.0f     ); // x
    gep::vec3 axisY = vec3(0.0f,      axesScale, 0.0f     ); // y
    gep::vec3 axisZ = vec3(0.0f,      0.0f,      axesScale); // z
    drawLine(objectPosition, objectPosition + axisX, colorX);
    drawLine(objectPosition, objectPosition + axisY, colorY);
    drawLine(objectPosition, objectPosition + axisZ, colorZ);
}

void gep::DebugRenderer::extract(IRendererExtractor& extractor)
{
    ScopedLock<Mutex> lock(m_mutex);
    DebugMarkerSection marker(extractor, "DebugRenderer");
    finishLineGroup();
    finishLineGroup2D();
    auto& e = static_cast<RendererExtractor&>(extractor);
    auto& context2D = extractor.getContext2D();

    for(auto& group : m_lineGroups)
    {
        auto& cmd = e.makeCommand<CommandRenderLines>();
        cmd.color = group.color;
        //cmd.lines = ArrayPtr<LineInfo>((LineInfo*)e.getCurrentAllocator()->allocateMemory(sizeof(LineInfo) * group.lines.length()), group.lines.length());
        cmd.lines = GEP_NEW_ARRAY(e.getCurrentAllocator(), LineInfo, group.lines.length());
        cmd.lines.copyFrom(group.lines);
    }

    for (auto& group : m_lineGroups2D)
    {
        auto& cmd = e.makeCommand<CommandRenderLines2D>();
        cmd.color = group.color;
        cmd.lines = GEP_NEW_ARRAY(e.getCurrentAllocator(), LineInfo2D, group.lines.length());
        cmd.lines.copyFrom(group.lines);
    }

    for (auto& textInfo : m_textInfos)
    {
        switch (textInfo.type)
        {
        case TextInfo::Text2D:
            context2D.printText(textInfo.pos2D, textInfo.text, textInfo.color);
            break;
        case TextInfo::Text3DBillboard:
            {
                auto& cmd = e.makeCommand<CommandDrawTextBillboard>();
                cmd.position = textInfo.pos3D;
                cmd.color = textInfo.color;

                auto len = strlen(textInfo.text);

                #ifdef _DEBUG
                for (size_t i = 0; i < len; i++)
                {
                    GEP_ASSERT(textInfo.text[i] > 0, "ascii string contains non-ascii characters. Please use wide char version to print non ascii characters");
                }
                #endif // _DEBUG

                cmd.text = GEP_NEW_ARRAY(extractor.getCurrentAllocator(), char, len + 1).getPtr();
                memcpy((void*)cmd.text, textInfo.text, len + 1);
            }
            break;
        default:
            break;
        }
    }

    m_lineGroups.clear();
    m_lineGroups2D.clear();
    m_textInfos.clear();
    if(m_frameAllocator.getMarker() != m_pStartMarker)
        m_frameAllocator.freeToMarker(m_pStartMarker);
}




#undef GEP_VERBOSE_LOG_MESSAGE

