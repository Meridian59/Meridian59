//-----------------------------------------------------------------------------
// File: Textures.cpp
//
// Desc: DirectShow sample code - uses the Direct3D Textures Tutorial05 as 
//       a base to create an application that uses DirectShow to draw a video 
//       on a DirectX 8.0 Texture surface.

// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------


#include "textures.h"
#include "resource.h"

#pragma warning( disable : 4100 4238)

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D8             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER8 g_pVB        = NULL; // Buffer to hold vertices
LPDIRECT3DTEXTURE8      g_pTexture   = NULL; // Our texture
HINSTANCE               hInstance    = 0;

// A structure for our custom vertex type. We added texture coordinates
struct CUSTOMVERTEX
{
    D3DXVECTOR3 position; // The position
    D3DCOLOR    color;    // The color
    FLOAT       tu, tv;   // The texture coordinates
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define CLASSNAME   TEXT("DShow Texture3D Sample")

// Function prototypes
void AddAboutMenuItem(HWND hWnd);
LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    HRESULT hr;

    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate8( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Get the current desktop display mode, so we can set up a back
    // buffer of the same format
    D3DDISPLAYMODE d3ddm;
    if ( FAILED( hr = g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
    {
        Msg(TEXT("Could not read adapter display mode!  hr=0x%x"), hr);
        return hr;
    }

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY_VSYNC;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                               D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                               &d3dpp, &g_pd3dDevice );                                     
    if (FAILED(hr))                                      
    {
        Msg(TEXT("Could not create the D3D device!  hr=0x%x\r\n\r\n")
            TEXT("This sample is attempting to create a buffer that might not\r\n")
            TEXT("be supported by your video card in its current mode.\r\n\r\n")
            TEXT("You may want to reduce your screen resolution or bit depth\r\n")
            TEXT("and try to run this sample again."), hr);
        return hr;
    }

    // Turn off culling
    hr = g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting
    hr = g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Turn on the zbuffer
    hr = g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // Set texture states
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    // Add filtering
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    hr = g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    return hr;
}



//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Create the textures and vertex buffers
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    HRESULT hr;

    // DShow: Set up filter graph with our custom renderer.
    if( FAILED( InitDShowTextureRenderer(g_pTexture ) ) )
        return E_FAIL;

    // Create the vertex buffer.
    if( FAILED( hr = g_pd3dDevice->CreateVertexBuffer( 50*2*sizeof(CUSTOMVERTEX),
                                      0, D3DFVF_CUSTOMVERTEX,
                                      D3DPOOL_DEFAULT, &g_pVB ) ) )
    {
        Msg(TEXT("Could not create a vertex buffer!  hr=0x%x"), hr);
        return E_FAIL;
    }

    // Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    CUSTOMVERTEX* pVertices;
    if ( FAILED( hr = g_pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
    {
        Msg(TEXT("Could not lock the vertex buffer!  hr=0x%x"), hr);
        return E_FAIL;
    }

    for( DWORD i=0; i<50; i++ )
    {
        FLOAT theta = (2*D3DX_PI*i)/(50-1);

        pVertices[2*i+0].position = D3DXVECTOR3( sinf(theta),-1.0f, cosf(theta) );
        pVertices[2*i+0].color    = 0xffffffff;
        pVertices[2*i+0].tu       = ((FLOAT)i)/(50-1);
        pVertices[2*i+0].tv       = 1.0f;

        pVertices[2*i+1].position = D3DXVECTOR3( sinf(theta), 1.0f, cosf(theta) );
        pVertices[2*i+1].color    = 0xff808080;
        pVertices[2*i+1].tu       = ((FLOAT)i)/(50-1);
        pVertices[2*i+1].tv       = 0.0f;
    }

    g_pVB->Unlock();
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    CleanupDShow();

    if( g_pTexture != NULL )   
        g_pTexture->Release();
        
    if( g_pVB != NULL )
        g_pVB->Release();
        
    if( g_pd3dDevice != NULL ) 
        g_pd3dDevice->Release();
        
    if( g_pD3D != NULL )
        g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    HRESULT hr;

    // For our world matrix, we will just leave it as the identity
    D3DXMATRIX matWorld;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixRotationX( &matWorld, timeGetTime()/2000.0f );
    hr = g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    if (FAILED(hr))                                      
    {
        Msg(TEXT("Could not set D3DTS_WORLD transform!  hr=0x%x"), hr);
    }

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0.0f, 3.0f,-5.0f ),
                                  &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
    hr = g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    if (FAILED(hr))                                      
    {
        Msg(TEXT("Could not set D3DTS_VIEW transform!  hr=0x%x"), hr);
    }

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
    hr = g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
    if (FAILED(hr))                                      
    {
        Msg(TEXT("Could not set D3DTS_PROJECTION transform!  hr=0x%x"), hr);
    }
}



//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer and the zbuffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

    // Begin the scene
    g_pd3dDevice->BeginScene();

    // Setup the world, view, and projection matrices
    SetupMatrices();

    // Setup our texture. Using textures introduces the texture stage states,
    // which govern how textures get blended together (in the case of multiple
    // textures) and lighting information. In this case, we are modulating
    // (blending) our texture with the diffuse color of the vertices.
    g_pd3dDevice->SetTexture( 0, g_pTexture );

    // Render the vertex buffer contents
    g_pd3dDevice->SetStreamSource( 0, g_pVB, sizeof(CUSTOMVERTEX) );
    g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2*50-2 );

    // End the scene
    g_pd3dDevice->EndScene();

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

    // Check to see if we need to restart the movie
    CheckMovieStatus();
}



//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;

        case WM_KEYUP:
        {
            // Close the app if the ESC key is pressed
            if (wParam == VK_ESCAPE)
                PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;

        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case ID_HELP_ABOUT:
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, 
                             (DLGPROC) AboutDlgProc);
                    return 0;
            }
        }
        break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


//-----------------------------------------------------------------------------
// Name: AboutDlgProc()
// Desc: Message handler for About box
//-----------------------------------------------------------------------------

LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
    	    return TRUE;

        case WM_COMMAND:
	        if (wParam == IDOK)
	        {
                EndDialog(hWnd, TRUE);
	            return TRUE;
	        }
	        break;
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// Name: AddAboutMenuItem()
// Desc: Adds a menu item to the end of the app's system menu
//-----------------------------------------------------------------------------
void AddAboutMenuItem(HWND hWnd)
{
    // Add About box menu item
    HMENU hwndMain = GetSystemMenu(hWnd, FALSE);

    // Add separator
    BOOL rc = AppendMenu(hwndMain, MF_SEPARATOR, 0, NULL);

    // Add menu item
    rc = AppendMenu(hwndMain, MF_STRING | MF_ENABLED, 
                    ID_HELP_ABOUT, 
                    TEXT("About Texture3D...\0"));
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpCmdLine, INT nCmdShow)
{
    // Initialize COM
    CoInitialize (NULL);

    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), 
                      LoadIcon(hInst, MAKEINTRESOURCE(IDI_TEXTURES)), 
                      NULL, NULL, NULL,
                      CLASSNAME, NULL };
    RegisterClassEx( &wc );
    hInstance = hInst;

    // Create the application's window
    HWND hWnd = CreateWindow( CLASSNAME, TEXT("DShow Texture3D Sample"),
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Add a menu item to the app's system menu
    AddAboutMenuItem(hWnd);

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the scene geometry
        if( SUCCEEDED( InitGeometry() ) )
        {
            // Show the window
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            // Enter the message loop
            MSG msg;
            ZeroMemory( &msg, sizeof(msg) );
            while( msg.message!=WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                {
                    Render();

                    // We're not attempting to achieve a fast D3D frame rate
                    // in this sample, so let's just sleep for a while to 
                    // reduce CPU utilization.  Otherwise, this app will use
                    // as much CPU horsepower as possible, since the PeekMessage()
                    // loop will run until the user closes the application.
                    Sleep(25);
                }
            }
        }
    }

    // Clean up everything and exit the app
    Cleanup();
    UnregisterClass( CLASSNAME, wc.hInstance );
    CoUninitialize();
    return 0L;
}


