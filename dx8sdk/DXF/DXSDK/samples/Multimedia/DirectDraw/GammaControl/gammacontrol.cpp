//-----------------------------------------------------------------------------
// File: GammaControl.cpp
//
// Desc: This sample demonstrates how to use DirectDraw full-screen mode
//       using the exclusive DirectDraw cooperative level.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include "resource.h"
#include "ddutil.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define SCREEN_WIDTH        640
#define SCREEN_HEIGHT       480
#define SCREEN_BPP          32

#define NUM_COLOR_BARS      63

#define HELPTEXT  TEXT("Adjust the gamma level by clicking either mouse button.  Press Escape to quit.")
#define GAMMATEXT TEXT("Displaying with linear gamma ramp between 0 and %0.5d")

CDisplay*                g_pDisplay          = NULL;
CSurface*                g_pLogoSurface      = NULL;  
CSurface*                g_pHelpTextSurface  = NULL;  
CSurface*                g_pGammaTextSurface = NULL;  
LPDIRECTDRAWGAMMACONTROL g_pGammaControl = NULL;
BOOL                     g_bActive       = FALSE; 
LONG                     g_lGammaRamp    = 256;




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
HRESULT InitDirectDraw( HWND hWnd );
VOID    FreeDirectDraw();
BOOL    HasGammaSupport();
HRESULT UpdateGammaRamp();
HRESULT ProcessNextFrame();
HRESULT DisplayFrame();
HRESULT RestoreSurfaces();
HRESULT DrawGammaText();




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything and calls
//       UpdateFrame() when idle from the message pump.
//-----------------------------------------------------------------------------
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{
    MSG		 msg;
    HWND     hWnd;
    HACCEL   hAccel;
    HRESULT  hr;

    if( FAILED( WinInit( hInst, nCmdShow, &hWnd, &hAccel ) ) )
        return FALSE;

    if( FAILED( InitDirectDraw( hWnd ) ) )
    {
        SAFE_DELETE( g_pDisplay );

        MessageBox( hWnd, TEXT("DirectDraw init failed. ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    if( HasGammaSupport() == FALSE ) 
    {
        SAFE_DELETE( g_pDisplay );

        MessageBox( hWnd, TEXT("The primary surface does not support gamma control.  ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    if( FAILED( hr = g_pDisplay->GetFrontBuffer()->QueryInterface( IID_IDirectDrawGammaControl, 
                                                    (LPVOID*) &g_pGammaControl ) ) )
    {
        SAFE_DELETE( g_pDisplay );

        MessageBox( hWnd, TEXT("The primary surface does not support IDirectDrawGammaControl.  ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    while( TRUE )
    {
        // Look for messages, if none are found then 
        // update the state and display it
        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if( 0 == GetMessage(&msg, NULL, 0, 0 ) )
            {
                // WM_QUIT was posted, so exit
                return (int)msg.wParam;
            }

            // Translate and dispatch the message
            if( 0 == TranslateAccelerator( hWnd, hAccel, &msg ) )
            {
                TranslateMessage( &msg ); 
                DispatchMessage( &msg );
            }
        }
        else
        {
            if( g_bActive )
            {
                // Move the sprites, blt them to the back buffer, then 
                // flip or blt the back buffer to the primary buffer
                if( FAILED( ProcessNextFrame() ) )
                {
                    SAFE_DELETE( g_pDisplay );

                    MessageBox( hWnd, TEXT("Displaying the next frame failed. ")
                                TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                                MB_ICONERROR | MB_OK );
                    return FALSE;
                }
            }
            else
            {
                // Make sure we go to sleep if we have nothing else to do
                WaitMessage();
            }
        }
    }
}




//-----------------------------------------------------------------------------
// Name: WinInit()
// Desc: Init the window
//-----------------------------------------------------------------------------
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel )
{
    WNDCLASS wc;
    HWND     hWnd;
    HACCEL   hAccel;

    // Register the Window Class
    wc.lpszClassName = TEXT("GammaControl");
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( RegisterClass( &wc ) == 0 )
        return E_FAIL;

    // Load keyboard accelerators
    hAccel = LoadAccelerators( hInst, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    // Create and show the main window
    hWnd = CreateWindowEx( 0, TEXT("GammaControl"), TEXT("DirectDraw GammaControl Sample"),
                           WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
  	                       CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL );
    if( hWnd == NULL )
    	return E_FAIL;

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    *phWnd   = hWnd;
    *phAccel = hAccel;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDirectDraw()
// Desc: Create the DirectDraw object, and init the surfaces
//-----------------------------------------------------------------------------
HRESULT InitDirectDraw( HWND hWnd )
{
    HRESULT             hr;

    g_pDisplay = new CDisplay();
    if( FAILED( hr = g_pDisplay->CreateFullScreenDisplay( hWnd, SCREEN_WIDTH, 
                                                          SCREEN_HEIGHT, SCREEN_BPP ) ) )
    {
        MessageBox( hWnd, TEXT("This display card does not support 640x480x8. "),
                    TEXT("DirectDraw Sample"), MB_ICONERROR | MB_OK );
        return hr;
    }

    // Create a surface, and draw text to it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromText( &g_pHelpTextSurface, NULL, HELPTEXT, 
                                                        RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    TCHAR strGammaText[256];
    wsprintf( strGammaText, GAMMATEXT, g_lGammaRamp * 256 );

    if( FAILED( hr = g_pDisplay->CreateSurfaceFromText( &g_pGammaTextSurface, NULL, strGammaText, 
                                                        RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectDraw()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID FreeDirectDraw()
{
    SAFE_DELETE( g_pHelpTextSurface );
    SAFE_DELETE( g_pGammaTextSurface );
    SAFE_DELETE( g_pDisplay );
}





//-----------------------------------------------------------------------------
// Name: MainWndProc()
// Desc: The main window procedure
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch (msg)
    {
        case WM_LBUTTONDOWN:
            g_lGammaRamp -= 8;
            if( g_lGammaRamp < 0 )
                g_lGammaRamp = 0;

            if( FAILED( UpdateGammaRamp() ) )
            {
                SAFE_DELETE( g_pDisplay );

                MessageBox( hWnd, TEXT("Failed setting the new gamma level.  ")
                            TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                            MB_ICONERROR | MB_OK );

                PostQuitMessage( 0 );
            }
            break;

        case WM_RBUTTONDOWN:
            g_lGammaRamp += 8;
            if( g_lGammaRamp > 256 )
                g_lGammaRamp = 256;

            if( FAILED( UpdateGammaRamp() ) )
            {
                SAFE_DELETE( g_pDisplay );

                MessageBox( hWnd, TEXT("Failed setting the new gamma level.  ")
                            TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                            MB_ICONERROR | MB_OK );

                PostQuitMessage( 0 );
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_EXIT:
                    // Received key/menu command to exit app
            	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0L;
            }
            break; // Continue with default processing

        case WM_SETCURSOR:
            // Hide the cursor in fullscreen 
            SetCursor( NULL );
            return TRUE;

        case WM_SIZE:
            // Check to see if we are losing our window...
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                g_bActive = FALSE;
            else
                g_bActive = TRUE;
            break;
            
        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in fullscreen mode
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_MONITORPOWER:
                    return TRUE;
            }
            break;
            
        case WM_DESTROY:
            // Cleanup and close the app
            FreeDirectDraw();
            PostQuitMessage( 0 );
            return 0L;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: HasGammaSupport()
// Desc: Returns TRUE if the device supports gamma control, FALSE otherwise
//-----------------------------------------------------------------------------
BOOL HasGammaSupport()
{
    // Get driver capabilities to determine gamma support.
    DDCAPS  ddcaps;    
    ZeroMemory( &ddcaps, sizeof(ddcaps) );
    ddcaps.dwSize = sizeof(ddcaps);

    g_pDisplay->GetDirectDraw()->GetCaps( &ddcaps, NULL );
    
    // Does the driver support gamma? 
    // The DirectDraw emulation layer does not support overlays
    // so gamma related APIs will fail without hardware support.  
    if( ddcaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA )
        return TRUE;
    else
        return FALSE;    
}




//-----------------------------------------------------------------------------
// Name: UpdateGammaRamp()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT UpdateGammaRamp()
{
    HRESULT     hr;
    DDGAMMARAMP ddgr;
    WORD dwGamma;

    ZeroMemory( &ddgr, sizeof(ddgr) );

    if( FAILED( hr = g_pGammaControl->GetGammaRamp( 0, &ddgr ) ) )
        return hr;

    dwGamma = 0;

    for( int iColor = 0; iColor < 256; iColor++ )
    {
        ddgr.red[iColor]   = dwGamma;
        ddgr.green[iColor] = dwGamma;
        ddgr.blue[iColor]  = dwGamma;

        dwGamma += (WORD) g_lGammaRamp;
    }

    if( FAILED( hr = g_pGammaControl->SetGammaRamp( 0, &ddgr ) ) )
        return hr;

    if( FAILED( hr = DrawGammaText() ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ProcessNextFrame()
// Desc: Move the sprites, blt them to the back buffer, then 
//       flips the back buffer to the primary buffer
//-----------------------------------------------------------------------------
HRESULT ProcessNextFrame()
{
    HRESULT hr;

    // Display the sprites on the screen
    if( FAILED( hr = DisplayFrame() ) )
    {
        if( hr != DDERR_SURFACELOST )
            return hr;

        // The surfaces were lost so restore them 
        RestoreSurfaces();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DisplayFrame()
// Desc: Blts a the color bars to the back buffer, then flips the 
//       back buffer onto the primary buffer.
//-----------------------------------------------------------------------------
HRESULT DisplayFrame()
{
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    DDBLTFX ddbltfx;
    RECT    rctDest;
    DWORD   dwRShift;
    DWORD   dwGShift;
    DWORD   dwBShift;
    DWORD   dwRColorLevels;
    DWORD   dwGColorLevels;
    DWORD   dwBColorLevels;
    DWORD   dwBits;
    DWORD   dwColor;
    LPDIRECTDRAWSURFACE7 pFrontBuffer;
    LPDIRECTDRAWSURFACE7 pBackBuffer;

    pFrontBuffer = g_pDisplay->GetFrontBuffer();
    pBackBuffer = g_pDisplay->GetBackBuffer();

    ZeroMemory( &ddsd,sizeof(ddsd) );
    ddsd.dwSize = sizeof(ddsd);
    if( FAILED( hr = pFrontBuffer->GetSurfaceDesc( &ddsd ) ) )
        return hr;

    // Fill the back buffer with black, ignoring errors until the flip
    g_pDisplay->Clear(0);

    CSurface::GetBitMaskInfo( ddsd.ddpfPixelFormat.dwRBitMask, &dwRShift, &dwBits );
    CSurface::GetBitMaskInfo( ddsd.ddpfPixelFormat.dwGBitMask, &dwGShift, &dwBits );
    CSurface::GetBitMaskInfo( ddsd.ddpfPixelFormat.dwBBitMask, &dwBShift, &dwBits );

    dwRColorLevels = ddsd.ddpfPixelFormat.dwRBitMask >> dwRShift;
    dwGColorLevels = ddsd.ddpfPixelFormat.dwGBitMask >> dwGShift;
    dwBColorLevels = ddsd.ddpfPixelFormat.dwBBitMask >> dwBShift;

    ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
    ddbltfx.dwSize      = sizeof(ddbltfx);

    for( DWORD i = 0; i <= NUM_COLOR_BARS; i++ )
    {
        rctDest.left   = SCREEN_WIDTH / (NUM_COLOR_BARS + 1) * (i + 0);
        rctDest.right  = SCREEN_WIDTH / (NUM_COLOR_BARS + 1) * (i + 1);

        // Figure out the color for the red color bar
        dwColor = (DWORD) ( dwRColorLevels * i / (float) NUM_COLOR_BARS );
        ddbltfx.dwFillColor = dwColor << dwRShift;

        rctDest.top    = SCREEN_HEIGHT / 3 * 0;
        rctDest.bottom = SCREEN_HEIGHT / 3 * 1;
        pBackBuffer->Blt( &rctDest, NULL, NULL, 
                         DDBLT_COLORFILL | DDBLT_WAIT, 
                         &ddbltfx );

        // Figure out the color for the green color bar
        dwColor = (DWORD) ( dwGColorLevels * i / (float) NUM_COLOR_BARS );
        ddbltfx.dwFillColor = dwColor << dwGShift;

        rctDest.top    = SCREEN_HEIGHT / 3 * 1;
        rctDest.bottom = SCREEN_HEIGHT / 3 * 2;
        pBackBuffer->Blt( &rctDest, NULL, NULL, 
                         DDBLT_COLORFILL | DDBLT_WAIT, 
                         &ddbltfx );

        // Figure out the color for the blue color bar
        dwColor = (DWORD) ( dwBColorLevels * i / (float) NUM_COLOR_BARS );
        ddbltfx.dwFillColor = dwColor << dwBShift;

        rctDest.top    = SCREEN_HEIGHT / 3 * 2;
        rctDest.bottom = SCREEN_HEIGHT / 3 * 3;
        pBackBuffer->Blt( &rctDest, NULL, NULL, 
                         DDBLT_COLORFILL | DDBLT_WAIT, 
                         &ddbltfx );

    }

    // Blt the help text on the backbuffer, ignoring errors until the flip
    g_pDisplay->Blt( 10, 10, g_pHelpTextSurface, NULL );

    g_pDisplay->Blt( 10, 30, g_pGammaTextSurface, NULL );

    // We are in fullscreen mode, so perform a flip and return 
    // any errors like DDERR_SURFACELOST
    if( FAILED( hr = g_pDisplay->Present() ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreSurfaces()
// Desc: Restore all the surfaces, and redraw the sprite surfaces.
//-----------------------------------------------------------------------------
HRESULT RestoreSurfaces()
{
    HRESULT hr;

    if( FAILED( hr = g_pDisplay->GetDirectDraw()->RestoreAllSurfaces() ) )
        return hr;

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = g_pHelpTextSurface->DrawText( NULL, HELPTEXT, 
                                                   0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    DrawGammaText();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawGammaText()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT DrawGammaText()
{
    HRESULT hr;
    TCHAR   strGammaText[256];

    wsprintf( strGammaText, GAMMATEXT, g_lGammaRamp * 256 );

    if( FAILED( hr = g_pGammaTextSurface->DrawText( NULL, strGammaText, 
                                                    0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    return S_OK;
}




