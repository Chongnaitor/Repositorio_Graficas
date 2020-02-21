//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"


//Ramses�s libraries
#include "Camera.h"
#include "FirstCamera.h"
#include <fstream>
#include <iostream>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <vector>
#include "Encabezados/ClaseDevice.h"
#include "Encabezados/ClaseDeviceContext.h"
#include "Encabezados/ClaseSwapChain.h"
#include "Encabezados/ClaseRenderTargetView.h"
#include "Encabezados/ClaseRenderTarget.h"
#include "Encabezados/ClaseSampleState.h"
#include "Encabezados/ClaseDepthStencil.h"
#include "Encabezados/ClaseViewport.h"
#include "Encabezados/ClaseShader.h"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

struct CBNeverChanges
{
    //XMMATRIX mView;
    mathfu::float4x4 mView;
};

struct CBChangeOnResize
{
    //XMMATRIX mProjection;
    mathfu::float4x4 mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;

//ID3D11RenderTargetView*             g_pRenderTargetView = NULL;//en render Target
//ID3D11Texture2D*                    g_pDepthStencil = NULL;//en render Target
//ID3D11DepthStencilView*             g_pDepthStencilView = NULL;//en render Target
//ID3D11VertexShader*                 g_pVertexShader = NULL;//en vertex shader derivado de clase shader

ID3D11PixelShader*                  g_pPixelShader = NULL;//en pixel shader derivado de clase shader
//ID3D11InputLayout*                  g_pVertexLayout = NULL;//en input layout
ID3D11Buffer*                       g_pVertexBuffer = NULL;//en clase mesh como miebro de tipo buffer
ID3D11Buffer*                       g_pIndexBuffer = NULL;//en clase mesh como miebro de tipo buffer
ID3D11Buffer*                       g_pCBNeverChanges = NULL;//en una clase buffer
ID3D11Buffer*                       g_pCBChangeOnResize = NULL;
ID3D11Buffer*                       g_pCBChangesEveryFrame = NULL;
ID3D11ShaderResourceView*           g_pTextureRV = NULL;//en clase material o shaderResources

//ID3D11SamplerState*                 g_pSamplerLinear = NULL;// en clase samplerState 

XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;

XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );


//Ramses�s global variables

ClaseDevice CDev;
ClaseDeviceContext CDevCont;
ClaseSwapChain CSwap;
ClaseRenderTargetView CRendTarView;
RenderTarget CRendTar;
ClaseSampleState CSampleS;
ClaseDepthStencil CDepthS;
ClaseViewport CView;
ClaseShader CShader;

Camera QuieroDormir_Camara;
FirstCamera QuieroDormir2_Camara;

bool ClickPressed = false;
int LevelCubes[10][10] = { 0 };
int Rows = 0;
int Columns = 0;
int SwitchCamera = -1;

UINT width;
UINT height;

ID3D11InputLayout* Layout;


//Ramses�s functions
void Resize() {

    if (CSwap.g_pSwapChainD3D11) {
        RECT rc;
        GetClientRect(g_hWnd, &rc);
        //g_pd3dDeviceContext->OMSetRenderTargets(0, 0, 0); 
        CDevCont.g_pImmediateContextD3D11->OMSetRenderTargets(0, 0, 0);

        // Release all outstanding references to the swap chain's buffers.
        CRendTarView.g_pRenderTargetViewD3D11->Release();
        //g_pRenderTargetView->Release();
        CDepthS.g_pDepthStencilViewD3D11->Release();
        //g_pDepthStencil->Release();
        CRendTar.g_pDepthStencilD3D11->Release();

        HRESULT hr;
        // Preserve the existing buffer count and format.
        // Automatically choose the width and height to match the client rect for HWNDs.
        hr = CSwap.g_pSwapChainD3D11->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

        // Get buffer and create a render-target-view.
        ID3D11Texture2D* pBuffer;
        hr = CSwap.g_pSwapChainD3D11->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
        // Perform error handling here!

        
        //hr = CDev.m_DescDevice.g_pd3dDevice->CreateRenderTargetView(pBuffer, NULL, &g_pRenderTargetView);
        hr = CDev.g_pd3dDeviceD3D11->CreateRenderTargetView(pBuffer, NULL, &CRendTarView.g_pRenderTargetViewD3D11);
        // Perform error handling here!
        pBuffer->Release();

        
        //g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
        //CDevCont.m_DescDCont.g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
        CDevCont.g_pImmediateContextD3D11->OMSetRenderTargets(1, &CRendTarView.g_pRenderTargetViewD3D11, NULL);

        // Create depth stencil texture
        DepthStencilDesc descDepth2;
        descDepth2.Width = rc.right - rc.left;
        descDepth2.Height = rc.bottom - rc.top;
        descDepth2.MipLevels = 1;
        descDepth2.ArraySize = 1;
        descDepth2.Format = FORMAT_D24_UNORM_S8_UINT;
        descDepth2.SampleDesc.My_Count = 1;
        descDepth2.SampleDesc.My_Quality = 0;
        descDepth2.Usage = USAGE_DEFAULT;
        descDepth2.BindFlags = BIND_DEPTH_STENCIL;
        descDepth2.CPUAccessFlags = 0;
        descDepth2.MiscFlags = 0;

        CDepthS.Init(descDepth2);

        /*D3D11_TEXTURE2D_DESC descDepth;
        ZeroMemory(&descDepth, sizeof(descDepth));
        descDepth.Width = rc.right - rc.left;
        descDepth.Height = rc.bottom - rc.top;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;*/

        //hr = CDev.m_DescDevice.g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
        hr = CDev.g_pd3dDeviceD3D11->CreateTexture2D(&CDepthS.descDepthD3D11, NULL, &CRendTar.g_pDepthStencilD3D11);
        if (FAILED(hr))
            return;

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory(&descDSV, sizeof(descDSV));
        descDSV.Format = CDepthS.descDepthD3D11.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        //hr = CDev.m_DescDevice.g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
        hr = CDev.g_pd3dDeviceD3D11->CreateDepthStencilView(CRendTar.g_pDepthStencilD3D11, &descDSV, &CDepthS.g_pDepthStencilViewD3D11);
        if (FAILED(hr))
            return;
        
        
        // Set up the viewport.
        /*D3D11_VIEWPORT vp;
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;*/

        ViewportDesc vp2;
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;
        vp2.Width = width;
        vp2.Height = height;
        vp2.MinDepth = 0.0f;
        vp2.MaxDepth = 1.0f;
        vp2.TopLeftX = 0;
        vp2.TopLeftY = 0;

        CView.Init(vp2);

        //g_pd3dDeviceContext->RSSetViewports(1, &vp);
        CDevCont.g_pImmediateContextD3D11->RSSetViewports(1, &CView.vpD3D11);

        g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

        CBChangeOnResize cbChangesOnResize;
        //cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);
        CDevCont.g_pImmediateContextD3D11->UpdateSubresource(g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0);
        
        //Lo pasamos a lo de abajo

        //CBChangeOnResize cbChangesOnResize;
        QuieroDormir_Camara.SetHeight(height);
        QuieroDormir_Camara.SetWidht(width);
        QuieroDormir_Camara.GenerateProjectionMatrix();

        QuieroDormir2_Camara.SetHeight(height);
        QuieroDormir2_Camara.SetWidht(width);
        QuieroDormir2_Camara.GenerateProjectionMatrix();

        //cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
        //cbChangesOnResize.mProjection = QuieroDormir_Camara.GetProjection();

        cbChangesOnResize.mProjection = QuieroDormir2_Camara.GetProjection();
        CDevCont.g_pImmediateContextD3D11->UpdateSubresource(g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0);
    }
}

enum LevelStuff{

    Wall = 1,
    Void = 2,
    Pilares = 4,
    NotColideWalls = 5
};

void LevelMap(std::string FileLevelName){

    std::ifstream File;
    File.open(FileLevelName);

    File >> Columns >> Rows;

    for (int i = 0; i < Rows; i++){

        for (int j = 0; j < Columns; j++){

            File >> LevelCubes[i][j];
        }
    }

    for (int i = 0; i < Rows; i++){

        for (int j = 0; j < Columns; j++){

            std::cout << LevelCubes[i][j] << " ";
        }
        std::cout << std::endl;
    }
    File.close();
}

void InitCameras() {

    CameraDescriptor GodCamera;
    GodCamera.s_At = { 0,0,0 };
    GodCamera.s_Eye = { 0,0,-6 };
    GodCamera.s_Up = { 0,1,0 };
    GodCamera.s_Far = 1000;
    GodCamera.s_Near = 0.01;
    GodCamera.s_FoV = XM_PIDIV4;
    GodCamera.s_Height = height;
    GodCamera.s_Widht = width;

    CameraDescriptor FirstCamera;
    FirstCamera.s_At = { 0,0,0 };
    FirstCamera.s_Eye = { 0,0,-6 };
    FirstCamera.s_Up = { 0,1,0 };
    FirstCamera.s_Far = 1000;
    FirstCamera.s_Near = 0.01;
    FirstCamera.s_FoV = XM_PIDIV4;
    FirstCamera.s_Height = height;
    FirstCamera.s_Widht = width;
    
    QuieroDormir_Camara.Init(GodCamera);
    QuieroDormir2_Camara.Init(FirstCamera);
}





//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();

void activateConsole()
{
    //Create a console for this application
    AllocConsole();
    // Get STDOUT handle
    HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
    FILE* COutputHandle = _fdopen(SystemOutput, "w");

    // Get STDERR handle
    HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
    int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
    FILE* CErrorHandle = _fdopen(SystemError, "w");

    // Get STDIN handle
    HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
    FILE* CInputHandle = _fdopen(SystemInput, "r");

    //make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios::sync_with_stdio(true);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen_s(&CInputHandle, "CONIN$", "r", stdin);
    freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
    freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

    //Clear the error state for each of the C++ standard stream objects. We need to do this, as
    //attempts to access the standard streams before they refer to a valid target will cause the
    //iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
    //to always occur during startup regardless of whether anything has been read from or written to
    //the console or not.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();

}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();
    LevelMap("Mapa.txt");
    return ( int )msg.wParam;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    activateConsole();
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 7", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    LevelMap("Mapa.txt");
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob; //Un buffer que va a contener el resultado del shader.
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );

    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
    // Reflect shader info
    ID3D11ShaderReflection* pVertexShaderReflection = NULL;
    if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
    {
        return S_FALSE;
    }

    //Get shader info
    D3D11_SHADER_DESC shaderDesc;
    pVertexShaderReflection->GetDesc(&shaderDesc);

    //Read input layout description from shader info
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
    int offset = 0;

    for (int i = 0; i < shaderDesc.InputParameters; i++)
    {
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

        //Fill out input element desc
        D3D11_INPUT_ELEMENT_DESC elementDesc;
        elementDesc.SemanticName = paramDesc.SemanticName;
        elementDesc.SemanticIndex = paramDesc.SemanticIndex;
        elementDesc.InputSlot = 0;
        elementDesc.AlignedByteOffset = offset;
        elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        elementDesc.InstanceDataStepRate = 0;

        //Determine DXGI format
        if (paramDesc.Mask == 1)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (paramDesc.Mask <= 3)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (paramDesc.Mask <= 15)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT; offset += 12;
        }
        else if (paramDesc.Mask <= 7)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        //save element desc
        inputLayoutDesc.push_back(elementDesc);
    }

    // Try to create Input Layout
    HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

    //Free allocation shader reflection memory
    pVertexShaderReflection->Release();
    return hr;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,//Trabaja de manera combinada
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    //Mis movimientos
    DeviceDescriptor objDev;
    objDev.g_driverType = DRIVER_TYPE_NULL;
    //objDev.g_featureLevel;
    CDev.g_pd3dDeviceD3D11 = NULL;
    objDev.s_createDeviceFlags = 2;
    objDev.s_numFeatureLevels = 3;

    CDev.Init(objDev);

    /*DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;*/

    SwapChainDescriptor objSd;
    objSd.BufferCount = 1;
    objSd.BufferDesc.My_Width = width;
    objSd.BufferDesc.My_Height = height;
    objSd.BufferDesc.My_Format = FORMAT_R8G8B8A8_UNORM;
    objSd.BufferDesc.My_RefreshRate.My_Numerator = 60;
    objSd.BufferDesc.My_RefreshRate.My_Denominator = 1;
    objSd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    objSd.OutputWindow = g_hWnd;
    objSd.SampleDesc.My_Count = 1;
    objSd.SampleDesc.My_Quality = 0;
    objSd.Windowed = TRUE;

    CSwap.Init(objSd);

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {        
        CDev.g_driverTypeD3D11 = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, CDev.g_driverTypeD3D11, NULL, objDev.s_createDeviceFlags, featureLevels, objDev.s_numFeatureLevels,
                                            D3D11_SDK_VERSION, &CSwap.sdD3D11, &CSwap.g_pSwapChainD3D11, &CDev.g_pd3dDeviceD3D11, &CDev.g_featureLevelD3D11, &CDevCont.g_pImmediateContextD3D11);
        if( SUCCEEDED( hr ) )
            break;
    }
    
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = CSwap.g_pSwapChainD3D11->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    
    if( FAILED( hr ) )
        return hr;

    //hr = CDev.m_DescDevice.g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );

    hr = CDev.g_pd3dDeviceD3D11->CreateRenderTargetView( pBackBuffer, NULL, &CRendTarView.g_pRenderTargetViewD3D11);
    pBackBuffer->Release();

    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    //hr = CDev.m_DescDevice.g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );

    hr = CDev.g_pd3dDeviceD3D11->CreateTexture2D( &descDepth, NULL, &CRendTar.g_pDepthStencilD3D11);

    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    //hr = CDev.m_DescDevice.g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );

    hr = CDev.g_pd3dDeviceD3D11->CreateDepthStencilView(CRendTar.g_pDepthStencilD3D11, &descDSV, &CDepthS.g_pDepthStencilViewD3D11);
    
    if( FAILED( hr ) )
        return hr;

    //CDevCont.m_DescDCont.g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );
    
    CDevCont.g_pImmediateContextD3D11->OMSetRenderTargets( 1, &CRendTarView.g_pRenderTargetViewD3D11, CDepthS.g_pDepthStencilViewD3D11);

    // Setup the viewport
    /*D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;*/

    ViewportDesc  vp2;
    vp2.Width = (FLOAT)width;
    vp2.Height = (FLOAT)height;
    vp2.MinDepth = 0.0f;
    vp2.MaxDepth = 1.0f;
    vp2.TopLeftX = 0;
    vp2.TopLeftY = 0;
    
    CView.Init(vp2);

    CDevCont.g_pImmediateContextD3D11->RSSetViewports( 1, &CView.vpD3D11);

    // Compile the vertex shader
    //ID3DBlob* pVSBlob = NULL;

    hr = CompileShaderFromFile( L"Tutorial07.fx", "VS", "vs_4_0", &CShader.m_pVSBlobD3D11 );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader
    hr = CDev.g_pd3dDeviceD3D11->CreateVertexShader(CShader.m_pVSBlobD3D11->GetBufferPointer(), CShader.m_pVSBlobD3D11->GetBufferSize(), NULL, &CShader.g_pVertexShaderD3D11 );
    if( FAILED( hr ) )
    {    
        CShader.m_pVSBlobD3D11->Release();
        return hr;
    }

     /*Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT numElements = ARRAYSIZE( layout );*/


    //Create input layout from compiled VS
    hr = CreateInputLayoutDescFromVertexShaderSignature(CShader.m_pVSBlobD3D11, CDev.g_pd3dDeviceD3D11, &Layout);
    if (FAILED(hr))
        return hr;

    /*// Create the input layout
    hr = CDev.m_DescDevice.g_pd3dDevice->CreateInputLayout( layout, numElements, CShader.m_pVSBlob->GetBufferPointer(), CShader.m_pVSBlob->GetBufferSize(), &g_pVertexLayout );
    CShader.m_pVSBlobD3D11->Release();
    if( FAILED( hr ) )
        return hr;*/

    // Set the input layout
    CDevCont.g_pImmediateContextD3D11->IASetInputLayout(Layout);

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = CDev.g_pd3dDeviceD3D11->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = CDev.g_pd3dDeviceD3D11->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    CDevCont.g_pImmediateContextD3D11->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

    // Create index buffer
    // Create vertex buffer
    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = CDev.g_pd3dDeviceD3D11->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    CDevCont.g_pImmediateContextD3D11->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

    // Set primitive topology
    CDevCont.g_pImmediateContextD3D11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = CDev.g_pd3dDeviceD3D11->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = CDev.g_pd3dDeviceD3D11->CreateBuffer( &bd, NULL, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = CDev.g_pd3dDeviceD3D11->CreateBuffer( &bd, NULL, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    // Load the Texture
    hr = D3DX11CreateShaderResourceViewFromFile(CDev.g_pd3dDeviceD3D11, L"seafloor.dds", NULL, NULL, &g_pTextureRV, NULL );
    if( FAILED( hr ) )
        return hr;

    // Create the sample state

    SampleStateDesc sampDesc2;
    sampDesc2.Filter = FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc2.AddressU = TEXTURE_ADDRESS_WRAP;
    sampDesc2.AddressV = TEXTURE_ADDRESS_WRAP;
    sampDesc2.AddressW = TEXTURE_ADDRESS_WRAP;
    sampDesc2.ComparisonFunc = COMPARISON_NEVER;
    sampDesc2.MinLOD = 0;
    sampDesc2.MaxLOD = D3D11_FLOAT32_MAX;

    CSampleS.Init(sampDesc2);

    /*D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD         = 0;
    sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;*/

    hr = CDev.g_pd3dDeviceD3D11->CreateSamplerState( &CSampleS.sampDescD3D11, &CSampleS.g_pSamplerLinearD3D11 );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
    XMVECTOR At = XMVectorSet ( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet ( 0.0f, 1.0f, 0.0f, 0.0f );
   // g_View = XMMatrixLookAtLH( Eye, At, Up );
    
    InitCameras();
    CBNeverChanges cbNeverChanges;
    
    //cbNeverChanges.mView = XMMatrixTranspose( g_View );
    //cbNeverChanges.mView = QuieroDormir_Camara.GetView();
    cbNeverChanges.mView = QuieroDormir2_Camara.GetView();
    
    CDevCont.g_pImmediateContextD3D11->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    //g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );
    
    CBChangeOnResize cbChangesOnResize;
    
    //cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
   // cbChangesOnResize.mProjection = QuieroDormir_Camara.GetProjection();
    cbChangesOnResize.mProjection = QuieroDormir2_Camara.GetProjection();
    CDevCont.g_pImmediateContextD3D11->UpdateSubresource( g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0 );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if(CDevCont.g_pImmediateContextD3D11) CDevCont.g_pImmediateContextD3D11->ClearState();

    if(CSampleS.g_pSamplerLinearD3D11 ) CSampleS.g_pSamplerLinearD3D11->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if( g_pCBNeverChanges ) g_pCBNeverChanges->Release();
    if( g_pCBChangeOnResize ) g_pCBChangeOnResize->Release();
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if(Layout) Layout->Release();
    if(CShader.g_pVertexShaderD3D11) CShader.g_pVertexShaderD3D11->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    //if( g_pDepthStencil ) g_pDepthStencil->Release();
    if(CRendTar.g_pDepthStencilD3D11) CRendTar.g_pDepthStencilD3D11->Release();
    if(CDepthS.g_pDepthStencilViewD3D11) CDepthS.g_pDepthStencilViewD3D11->Release();
    //if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if(CRendTarView.g_pRenderTargetViewD3D11) CRendTarView.g_pRenderTargetViewD3D11->Release();
    if(CSwap.g_pSwapChainD3D11) CSwap.g_pSwapChainD3D11->Release();
    if(CDevCont.g_pImmediateContextD3D11) CDevCont.g_pImmediateContextD3D11->Release();
    if(CDev.g_pd3dDeviceD3D11) CDev.g_pd3dDeviceD3D11->Release();
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;
    POINT Temp;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

            //WM_SIZE: It is sent to a window after its size has changed.
        case WM_SIZE:
            Resize();
            break;

            //WM_KEYDOWN: A window receives keyboard input in the form of keystroke messages and character messages.
        case WM_KEYDOWN:
            if (SwitchCamera == -1) {

                QuieroDormir_Camara.inputs(wParam);
            }
            else if (SwitchCamera == 1) {

                QuieroDormir2_Camara.inputs(wParam);
            }
            break;
            
            //WM_LBUTTONDOWN: Posted when the user presses the left mouse button while the cursor is in the client area of a window.
        case WM_LBUTTONDOWN:
            if (SwitchCamera == -1) {//In this case We can move the free camera

                GetCursorPos(&Temp);
                QuieroDormir_Camara.SetOriginalMousePos(Temp.x, Temp.y);
                ClickPressed = true;
            }
            else if (SwitchCamera == 1) {//In this case We can move the first camera

                GetCursorPos(&Temp);
                QuieroDormir2_Camara.SetOriginalMousePos(Temp.x, Temp.y);
                ClickPressed = true;
            }
            break;

            //WM_LBUTTONUP: Posted when the user releases the left mouse button while the cursor is in the client area of a window.
        case WM_LBUTTONUP:
            ClickPressed = false;
            break;

            //WM_RBUTTONDOWN: Posted when the user presses the right mouse button while the cursor is in the client area of a window
        case WM_RBUTTONDOWN:
            SwitchCamera *= -1;
            break;


        //case WM_KEYUP:
        //    QuieroDormir_Camara.PitchX(wParam);
        //    break;
        //
        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render(){

    int MapX = 0;
    int MapY = 0;

    if (ClickPressed && SwitchCamera == -1) {//If we are in free camera 

        QuieroDormir_Camara.MouseRotation();
    }
    else if (ClickPressed && SwitchCamera == 1) {

        QuieroDormir2_Camara.MouseRotation();
    }

    // Update our time
    static float t = 0.0f;
    if (CDev.m_DescDevice.g_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float)XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount64();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    // Rotate cube around the origin
    //g_World = XMMatrixRotationY( t );
    //g_World = XMMatrixTranslation(-6, 0, 0);

    // Modify the color
    g_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
    g_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
    g_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    //CDevCont.m_DescDCont.g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
    CDevCont.g_pImmediateContextD3D11->ClearRenderTargetView(CRendTarView.g_pRenderTargetViewD3D11, ClearColor);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    CDevCont.g_pImmediateContextD3D11->ClearDepthStencilView(CDepthS.g_pDepthStencilViewD3D11, D3D11_CLEAR_DEPTH, 1.0f, 0);

    //
    // Update variables that change once per frame
    //
    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
    CDevCont.g_pImmediateContextD3D11->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

    //-----
    CBNeverChanges cbNeverChanges;

    if (SwitchCamera == -1) {

        cbNeverChanges.mView = QuieroDormir_Camara.GetView();
    }
    else if (SwitchCamera == 1) {

        cbNeverChanges.mView = QuieroDormir2_Camara.GetView();
    }

    CDevCont.g_pImmediateContextD3D11->UpdateSubresource(g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0);

    //
    // Render the cube
    //
    CDevCont.g_pImmediateContextD3D11->VSSetShader(CShader.g_pVertexShaderD3D11, NULL, 0);
    CDevCont.g_pImmediateContextD3D11->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
    CDevCont.g_pImmediateContextD3D11->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
    CDevCont.g_pImmediateContextD3D11->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    CDevCont.g_pImmediateContextD3D11->PSSetShader(g_pPixelShader, NULL, 0);
    CDevCont.g_pImmediateContextD3D11->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
    CDevCont.g_pImmediateContextD3D11->PSSetShaderResources(0, 1, &g_pTextureRV);
    CDevCont.g_pImmediateContextD3D11->PSSetSamplers(0, 1, &CSampleS.g_pSamplerLinearD3D11);
    CDevCont.g_pImmediateContextD3D11->DrawIndexed(36, 0, 0);

    for (int i = 0; i < Rows; i++){

        for (int j = 0; j < Columns; j++){

            if (LevelCubes[i][j]){

                MapX += 2.5;
            }
            else if (LevelCubes[i][j] == Pilares){

                MapX += 2.5;
            }
            else{

                MapX += 2.5;
            }
            //g_World = XMMatrixTranslation(MapX, 0, MapY);
            if (LevelCubes[i][j] != 0){

                CDevCont.g_pImmediateContextD3D11->VSSetShader(CShader.g_pVertexShaderD3D11, NULL, 0);
                CDevCont.g_pImmediateContextD3D11->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
                CDevCont.g_pImmediateContextD3D11->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
                CDevCont.g_pImmediateContextD3D11->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                CDevCont.g_pImmediateContextD3D11->PSSetShader(g_pPixelShader, NULL, 0);
                CDevCont.g_pImmediateContextD3D11->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
                CDevCont.g_pImmediateContextD3D11->PSSetShaderResources(0, 1, &g_pTextureRV);
                CDevCont.g_pImmediateContextD3D11->PSSetSamplers(0, 1, &CSampleS.g_pSamplerLinearD3D11);
                CDevCont.g_pImmediateContextD3D11->DrawIndexed(36, 0, 0);
            }
            g_World = XMMatrixTranslation(MapX, 0, MapY);
            cb.mWorld = XMMatrixTranspose(g_World);
            cb.vMeshColor = g_vMeshColor;
            CDevCont.g_pImmediateContextD3D11->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
        }
        MapY += 2.5;
        MapX = 0;
    }
    //
    // Present our back buffer to our front buffer
    //
    CSwap.g_pSwapChainD3D11->Present( 0, 0 );
}