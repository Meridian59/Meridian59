//-----------------------------------------------------------------------------
// File: OverlayAnimate.cpp
//
// Desc: This sample demonstrates how to animate using DirectDraw overlays
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include "resource.h"
#include "ddutil.h"
#include "dxutil.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define WINDOW_WIDTH        128
#define WINDOW_HEIGHT       128
#define SPRITE_DIAMETER     48
#define NUM_FRAMES          30


LPDIRECTDRAW7        g_pDD                = NULL;	
LPDIRECTDRAWSURFACE7 g_pDDSPrimary        = NULL;  
LPDIRECTDRAWSURFACE7 g_pDDSOverlay        = NULL;  
LPDIRECTDRAWSURFACE7 g_pDDSOverlayBack    = NULL;
LPDIRECTDRAWSURFACE7 g_pDDSAnimationSheet = NULL;  
DDOVERLAYFX          g_OverlayFX;         
DWORD                g_dwOverlayFlags     = 0;  
DWORD                g_dwFrame            = 0;
DDCAPS               g_ddcaps;
BOOL                 g_bActive            = FALSE; 
RECT                 g_rcSrc              = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
RECT                 g_rcDst              = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
DWORD                g_dwXRatio;
DWORD                g_dwYRatio; 
 
// This will be used as the color key, so try to make it something
// that doesn't appear in the source image.
COLORREF             g_dwBackgroundColor  = RGB(10, 0, 10);



//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
HRESULT InitDirectDraw( HWND hWnd );
BOOL    HasOverlaySupport();
VOID    FreeDirectDraw();
HRESULT CreateDirectDrawSurfaces( HWND hWnd );
VOID    AdjustSizeForHardwareLimits();
HRESULT ProcessNextFrame( HWND hWnd );
HRESULT DisplayFrame();
HRESULT RestoreSurfaces();




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything and calls
//       UpdateFrame() when idle from the message pump.
//-----------------------------------------------------------------------------
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{
    HRESULT  hr;
    MSG		 msg;
    HWND     hWnd;
    HACCEL   hAccel;

    if( FAILED( WinInit( hInst, nCmdShow, &hWnd, &hAccel ) ) )
        return FALSE;

    if( FAILED( InitDirectDraw( hWnd ) ) )
    {
        MessageBox( hWnd, TEXT("DirectDraw init failed. ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    if( HasOverlaySupport() == FALSE )
    {
        MessageBox( hWnd, TEXT("This DirectDraw device does not support overlays. ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    // Create the DirectDraw surfaces needed
    if( FAILED( hr = CreateDirectDrawSurfaces( hWnd ) ) )
    {
        MessageBox( hWnd, TEXT("Failed to create surfaces.  This DirectDraw device may not support overlays. ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    ShowWindow( hWnd, SW_SHOW );

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
                if( FAILED( ProcessNextFrame( hWnd ) ) )
                {
                    g_pDD->SetCooperativeLevel( NULL, DDSCL_NORMAL );

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
    WNDCLASSEX wc;
    HWND       hWnd;
    HACCEL     hAccel;

    // If we are in 8 bit mode, then we need to choose a system palette
    // color because most colors will be dithered expect ones in the palette
    HDC hDC = GetDC( NULL ); 
    if( GetDeviceCaps( hDC, NUMCOLORS ) != -1 ) 
        g_dwBackgroundColor = RGB( 255, 0, 255 );    
    ReleaseDC( NULL, hDC );
    
    // Register the Window Class
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = TEXT("OverlayAnimate");
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hIconSm       = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    
    // Let windows paint the colorkey'ed background which our overlay will use
    wc.hbrBackground = CreateSolidBrush( g_dwBackgroundColor ); 
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( RegisterClassEx( &wc ) == 0 )
        return E_FAIL;

    // Load keyboard accelerators
    hAccel = LoadAccelerators( hInst, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    // Calculate the proper size for the window given a client of 640x480
    DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
    DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
    DWORD dwMenuHeight    = GetSystemMetrics( SM_CYMENU );
    DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );
    DWORD dwWindowWidth   = WINDOW_WIDTH  + dwFrameWidth * 2;
    DWORD dwWindowHeight  = WINDOW_HEIGHT + dwFrameHeight * 2 + 
                            dwMenuHeight + dwCaptionHeight;

    // Create and show the main window
    hWnd = CreateWindow( TEXT("OverlayAnimate"), TEXT("DirectDraw OverlayAnimate Sample"),
                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
  	                     dwWindowWidth, dwWindowHeight, NULL, NULL, hInst, NULL );
    if( hWnd == NULL )
    	return E_FAIL;

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
    DDSURFACEDESC2 ddsd;
    HRESULT	hr;

    // Create the main DirectDraw object
    if( FAILED( hr = DirectDrawCreateEx( NULL, (VOID**)&g_pDD, 
                                         IID_IDirectDraw7, NULL ) ) )
        return hr;

    // Request normal cooperative level to put us in windowed mode
    if( FAILED( hr = g_pDD->SetCooperativeLevel( hWnd, DDSCL_NORMAL ) ) ) 
        return hr;

    // Get driver capabilities to determine Overlay support.
    ZeroMemory( &g_ddcaps, sizeof(g_ddcaps) );
    g_ddcaps.dwSize = sizeof(g_ddcaps);

    if( FAILED( hr = g_pDD->GetCaps( &g_ddcaps, NULL ) ) )
        return hr;

    // Create the primary surface, which in windowed mode is the desktop.
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSPrimary, NULL ) ) )
        return hr;
        
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateDirectDrawSurfaces()
// Desc: Creates the DirectDraw surfaces
//-----------------------------------------------------------------------------
HRESULT CreateDirectDrawSurfaces( HWND hWnd )
{
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT  ddpfOverlayFormat;
    DDSCAPS2       ddscaps;
    HRESULT		   hr;
    
    // Release any previous surfaces
    SAFE_RELEASE( g_pDDSOverlay ); 

    // Set the overlay format to 16 bit RGB 5:6:5
    ZeroMemory( &ddpfOverlayFormat, sizeof(ddpfOverlayFormat) );
    ddpfOverlayFormat.dwSize        = sizeof(ddpfOverlayFormat);
    ddpfOverlayFormat.dwFlags       = DDPF_RGB;
    ddpfOverlayFormat.dwRGBBitCount = 16;
    ddpfOverlayFormat.dwRBitMask    = 0xF800; 
    ddpfOverlayFormat.dwGBitMask    = 0x07E0;
    ddpfOverlayFormat.dwBBitMask    = 0x001F; 

    // Setup the overlay surface's attributes in the surface descriptor
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize            = sizeof(ddsd);
    ddsd.dwFlags           = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | 
                             DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_FLIP | 
                             DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;
    ddsd.dwWidth           = WINDOW_WIDTH;
    ddsd.dwHeight          = WINDOW_HEIGHT;
    ddsd.ddpfPixelFormat   = ddpfOverlayFormat;  // Use 16 bit RGB 5:6:5 pixel format

    // Attempt to create the surface with theses settings
    if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSOverlay, NULL ) ) ) 
        return hr;

    ZeroMemory(&ddscaps, sizeof(ddscaps));
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if( FAILED( hr = g_pDDSOverlay->GetAttachedSurface( &ddscaps, &g_pDDSOverlayBack ) ) )
        return hr;

    // Setup effects structure
    ZeroMemory( &g_OverlayFX, sizeof(g_OverlayFX) );
    g_OverlayFX.dwSize = sizeof(g_OverlayFX);

    // Setup overlay flags.
    g_dwOverlayFlags = DDOVER_SHOW;

    // Check for destination color keying capability
    if (g_ddcaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY)
    {
        // Using a color key will clip the overlay 
        // when the mouse or other windows go on top of us. 
        DWORD dwDDSColor;

        // The color key can be any color, but a near black (not exactly) allows
        // the cursor to move around on the window without showing off the
        // color key, and also clips windows with exactly black text.
        CSurface frontSurface; 
        frontSurface.Create( g_pDDSPrimary );
        dwDDSColor = frontSurface.ConvertGDIColor( g_dwBackgroundColor );
        g_OverlayFX.dckDestColorkey.dwColorSpaceLowValue  = dwDDSColor;
        g_OverlayFX.dckDestColorkey.dwColorSpaceHighValue = dwDDSColor;
        g_dwOverlayFlags |= DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
    }
    else
    {
        LPDIRECTDRAWCLIPPER pClipper = NULL;

        // If not, we'll setup a clipper for the window.  This will fix the
        // problem on a few video cards - but the ones that don't shouldn't
        // care.
        if( FAILED( hr = g_pDD->CreateClipper(0, &pClipper, NULL) ) )
            return hr;

        if( FAILED( hr = pClipper->SetHWnd(0, hWnd ) ) )
            return hr;

        if( FAILED( hr = g_pDDSPrimary->SetClipper( pClipper ) ) )
            return hr;

        SAFE_RELEASE( pClipper );
    }


    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize          = sizeof(ddsd);
    ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth         = SPRITE_DIAMETER * 5;
    ddsd.dwHeight        = SPRITE_DIAMETER * 6;
    ddsd.ddpfPixelFormat = ddpfOverlayFormat;  // Use 16 bit RGB 5:6:5 pixel format

    // Attempt to create the surface with theses settings
    if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pDDSAnimationSheet, NULL ) ) ) 
        return hr;

    CSurface animateSurface; 
    animateSurface.Create( g_pDDSAnimationSheet );
    if( FAILED( hr = animateSurface.DrawBitmap( MAKEINTRESOURCE( IDB_ANIMATE_SHEET ), 
                                               SPRITE_DIAMETER * 5, SPRITE_DIAMETER * 6 ) ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: HasOverlaySupport()
// Desc: Returns TRUE if the device supports overlays, FALSE otherwise
//-----------------------------------------------------------------------------
BOOL HasOverlaySupport()
{
    // Get driver capabilities to determine overlay support.
    ZeroMemory( &g_ddcaps, sizeof(g_ddcaps) );
    g_ddcaps.dwSize = sizeof(g_ddcaps);
    g_pDD->GetCaps( &g_ddcaps, NULL );
    
    // Does the driver support overlays in the current mode? 
    // The DirectDraw emulation layer does not support overlays
    // so overlay related APIs will fail without hardware support.  
    if( g_ddcaps.dwCaps & DDCAPS_OVERLAY )
    {
        // Make sure it supports stretching (scaling)
        if ( g_ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH )
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        return FALSE;    
    }
}




//-----------------------------------------------------------------------------
// Name: FreeDirectDraw()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID FreeDirectDraw()
{
    SAFE_RELEASE( g_pDDSOverlay ); // g_pDDSOverlayBack will be automatically released here
    SAFE_RELEASE( g_pDDSPrimary );
    SAFE_RELEASE( g_pDDSAnimationSheet ); 
    SAFE_RELEASE( g_pDD );
}




//-----------------------------------------------------------------------------
// Name: MainWndProc()
// Desc: The main window procedure
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch (msg)
    {
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_EXIT:
                    // Received key/menu command to exit app
            	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0L;
            }
            break; // Continue with default processing

        case WM_PAINT:
            // Update the screen if we need to refresh. This case occurs 
            // when in windowed mode and the window is behind others.
            // The app will not be active, but it will be visible.
            if( g_pDDSPrimary )
            {
                // UpdateOverlay is how we put the overlay on the screen.
                if( g_rcDst.top == g_rcDst.bottom )
                {
                    g_pDDSOverlay->UpdateOverlay( NULL, g_pDDSPrimary, NULL, 
                                                  DDOVER_HIDE, NULL );
                }
                else
                {
                    g_pDDSOverlay->UpdateOverlay( &g_rcSrc, g_pDDSPrimary,
                                                  &g_rcDst, g_dwOverlayFlags,
                                                  &g_OverlayFX);
                }
            }
            break; // Continue with default processing to validate the region

        case WM_QUERYNEWPALETTE:
            if( g_pDDSPrimary )
            {
                // If we are in windowed mode with a desktop resolution in 8 bit 
                // color, then the palette we created during init has changed 
                // since then.  So get the palette back from the primary 
                // DirectDraw surface, and set it again so that DirectDraw 
                // realises the palette, then release it again. 
                LPDIRECTDRAWPALETTE pDDPal = NULL; 
                g_pDDSPrimary->GetPalette( &pDDPal );
                g_pDDSPrimary->SetPalette( pDDPal );
                SAFE_RELEASE( pDDPal );
            }
            break;

        case WM_MOVE:
            // Make sure we're not moving to be minimized - because otherwise
            // our ratio varialbes (g_dwXRatio and g_dwYRatio) will end up
            // being 0, and once we hit CheckBoundries it divides by 0.
            if (!IsIconic(hWnd))
            {
                POINT p = {0, 0}; // Translation point for the window's client region

                g_rcSrc.left   = 0;
                g_rcSrc.right  = WINDOW_WIDTH;
                g_rcSrc.top    = 0;
                g_rcSrc.bottom = WINDOW_HEIGHT;

                GetClientRect(hWnd, &g_rcDst);

                g_dwXRatio = (g_rcDst.right - g_rcDst.left) * 1000 /
                             (g_rcSrc.right - g_rcSrc.left);
                g_dwYRatio = (g_rcDst.bottom - g_rcDst.top) * 1000 /
                             (g_rcSrc.bottom - g_rcSrc.top);

                ClientToScreen( hWnd, &p );
                g_rcDst.left   = p.x;
                g_rcDst.top    = p.y;
                g_rcDst.bottom += p.y;
                g_rcDst.right  += p.x;
                if( g_pDD )
                    AdjustSizeForHardwareLimits();
            }
            else
            {
                // Else, hide the overlay... just in case we can't do
                // destination color keying, this will pull the overlay
                // off of the screen for the user.
                if (g_pDDSOverlay && g_pDDSPrimary)
                    g_pDDSOverlay->UpdateOverlay( NULL, g_pDDSPrimary, NULL, 
                                                  DDOVER_HIDE, NULL);
            }

            // Check to make sure our window exists before we tell it to
            // repaint. This will fail the first time (while the window is
            // being created).
            if (hWnd)
            {
                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd);
            }
            return 0L;


        case WM_SIZE:
            // Another check for the minimization action.  This check is
            // quicker though...
            // Check to see if we are losing our window...
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                g_bActive = FALSE;
            else
                g_bActive = TRUE;

            if( g_bActive )
            {
                POINT p = {0, 0}; // Translation point for the window's client region

                GetClientRect(hWnd, &g_rcDst);
                ClientToScreen(hWnd, &p);

                g_rcDst.left   = p.x;
                g_rcDst.top    = p.y;
                g_rcDst.bottom += p.y;
                g_rcDst.right  += p.x;

                g_rcSrc.left   = 0;
                g_rcSrc.right  = WINDOW_WIDTH;
                g_rcSrc.top    = 0;
                g_rcSrc.bottom = WINDOW_HEIGHT;

                // Here we multiply by 1000 to preserve 3 decimal places in the
                // division opperation (we picked 1000 to be on the same order
                // of magnitude as the stretch factor for easier comparisons)
                g_dwXRatio = (g_rcDst.right - g_rcDst.left) * 1000 /
                             (g_rcSrc.right - g_rcSrc.left);

                g_dwYRatio = (g_rcDst.bottom - g_rcDst.top) * 1000 /
                             (g_rcSrc.bottom - g_rcSrc.top);

                AdjustSizeForHardwareLimits();
            }
            return 0L;


        case WM_DISPLAYCHANGE:
            // This not only checks for overlay support in the new video mode -
            // but gets the new caps for the new display settings.  That way we
            // have more accurate info about min/max stretch factors, color
            // keying
            if( HasOverlaySupport() == FALSE)
            {
                MessageBox( hWnd, "You have changed your adapter settings such "
                            " that you no longer support this overlay.", "Overlay", MB_OK );
            	PostMessage( hWnd, WM_CLOSE, 0, 0 );
            }
            return 0L;
        
        case WM_DESTROY:
            // Cleanup and close the app
            FreeDirectDraw();
            PostQuitMessage( 0 );
            return 0L;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: AdjustSizeForHardwareLimits()
// Desc: Checks and corrects all boundries for alignment and stretching
//-----------------------------------------------------------------------------
VOID AdjustSizeForHardwareLimits()
{
    // Setup effects structure
    // Make sure the coordinates fulfill the stretching requirements.  Often
    // the hardware will require a certain ammount of stretching to do
    // overlays. This stretch factor is held in dwMinOverlayStretch as the
    // stretch factor multiplied by 1000 (to keep an accuracy of 3 decimal
    // places).
    if( (g_ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) && 
        (g_ddcaps.dwMinOverlayStretch)            && 
        (g_dwXRatio < g_ddcaps.dwMinOverlayStretch) )
    {
        // Window is too small
        g_rcDst.right = 2 * GetSystemMetrics(SM_CXSIZEFRAME) + g_rcDst.left + (WINDOW_WIDTH
                                 * (g_ddcaps.dwMinOverlayStretch + 1)) / 1000;
    }

    if( (g_ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) && 
        (g_ddcaps.dwMaxOverlayStretch)            && 
        (g_dwXRatio > g_ddcaps.dwMaxOverlayStretch) )
    {
        // Window is too large
        g_rcDst.right = 2 * GetSystemMetrics(SM_CXSIZEFRAME) + g_rcDst.left + (WINDOW_HEIGHT
                               * (g_ddcaps.dwMaxOverlayStretch + 999)) / 1000;
    }

    // Recalculate the ratio's for the upcoming calculations
    g_dwXRatio = (g_rcDst.right  - g_rcDst.left) * 1000 / (g_rcSrc.right  - g_rcSrc.left);
    g_dwYRatio = (g_rcDst.bottom - g_rcDst.top)  * 1000 / (g_rcSrc.bottom - g_rcSrc.top);

    // Check to make sure we're within the screen's boundries, if not then fix
    // the problem by adjusting the source rectangle which we draw from.
    if (g_rcDst.left < 0)
    {
        g_rcSrc.left = -g_rcDst.left * 1000 / g_dwXRatio;
        g_rcDst.left = 0;
    }

    if (g_rcDst.right > GetSystemMetrics(SM_CXSCREEN))
    {
        g_rcSrc.right = WINDOW_WIDTH - ((g_rcDst.right - GetSystemMetrics(SM_CXSCREEN)) *
                                1000 / g_dwXRatio);
        g_rcDst.right = GetSystemMetrics(SM_CXSCREEN);
    }

    if (g_rcDst.bottom > GetSystemMetrics(SM_CYSCREEN))
    {
        g_rcSrc.bottom = WINDOW_HEIGHT - ((g_rcDst.bottom - GetSystemMetrics(SM_CYSCREEN))
                                 * 1000 / g_dwYRatio);
        g_rcDst.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if (g_rcDst.top < 0)
    {
        g_rcSrc.top = -g_rcDst.top * 1000 / g_dwYRatio;
        g_rcDst.top = 0;
    }

    // Make sure the coordinates fulfill the alignment requirements
    // these expressions (x & -y) just do alignment by dropping low order bits...
    // so to round up, we add first, then truncate.
    if( (g_ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC) && 
        (g_ddcaps.dwAlignBoundarySrc) )
    {
        g_rcSrc.left = (g_rcSrc.left + g_ddcaps.dwAlignBoundarySrc / 2) & 
                        -(signed) (g_ddcaps.dwAlignBoundarySrc);
    }

    if( (g_ddcaps.dwCaps & DDCAPS_ALIGNSIZESRC) && 
        (g_ddcaps.dwAlignSizeSrc) )
    {
        g_rcSrc.right = g_rcSrc.left + (g_rcSrc.right - g_rcSrc.left + g_ddcaps.dwAlignSizeSrc / 2) & 
                         -(signed) (g_ddcaps.dwAlignSizeSrc);
    }

    if( (g_ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST) && 
        (g_ddcaps.dwAlignBoundaryDest) )
    {
        g_rcDst.left = ( g_rcDst.left + g_ddcaps.dwAlignBoundaryDest / 2 ) & 
                       -(signed) (g_ddcaps.dwAlignBoundaryDest);
    }

    if( (g_ddcaps.dwCaps & DDCAPS_ALIGNSIZEDEST) && 
        (g_ddcaps.dwAlignSizeDest) )
    {
        g_rcDst.right = g_rcDst.left + (g_rcDst.right - g_rcDst.left) & 
                        -(signed) (g_ddcaps.dwAlignSizeDest);
    }
}




//-----------------------------------------------------------------------------
// Name: ProcessNextFrame()
// Desc: Move the sprites, blt them to the back buffer, then 
//       flip or blt the back buffer to the primary buffer
//-----------------------------------------------------------------------------
HRESULT ProcessNextFrame( HWND hWnd )
{
    static  s_dwFrameSkip = 0; 
    HRESULT hr;

    // Only advance the frame animation number every 5th frame
    s_dwFrameSkip++;
    s_dwFrameSkip %= 5;

    if( s_dwFrameSkip == 0 )
    {
        g_dwFrame++;
        g_dwFrame %= NUM_FRAMES;
    }

    // Check the cooperative level before rendering
    if( FAILED( hr = g_pDD->TestCooperativeLevel() ) )
    {
        switch( hr )
        {
            case DDERR_EXCLUSIVEMODEALREADYSET:
                // Do nothing because some other app has exclusive mode
                Sleep(10);
                return S_OK;

            case DDERR_WRONGMODE:
                // The display mode changed on us. Update the
                // DirectDraw surfaces accordingly
                return CreateDirectDrawSurfaces( hWnd );
        }
        return hr;
    }

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
// Desc: Blts a the sprites to the back buffer, then flips the 
//       back buffer onto the primary buffer.
//-----------------------------------------------------------------------------
HRESULT DisplayFrame()
{
    HRESULT hr;
    RECT    rcSrc;

    rcSrc.left   = (g_dwFrame % 5) * SPRITE_DIAMETER;
    rcSrc.top    = (g_dwFrame / 5) * SPRITE_DIAMETER;
    rcSrc.right  = rcSrc.left + SPRITE_DIAMETER;
    rcSrc.bottom = rcSrc.top  + SPRITE_DIAMETER;

    g_pDDSOverlayBack->Blt( NULL, g_pDDSAnimationSheet, 
                            &rcSrc, DDBLT_WAIT, NULL );

    if( FAILED( hr = g_pDDSOverlay->Flip( NULL, DDFLIP_WAIT ) ) )
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

    if( FAILED( hr = g_pDD->RestoreAllSurfaces() ) )
        return hr;

    // No need to re-create the surface, just re-draw it.
    CSurface animateSurface; 
    animateSurface.Create( g_pDDSAnimationSheet );
    if( FAILED( hr = animateSurface.DrawBitmap( MAKEINTRESOURCE( IDB_ANIMATE_SHEET ),
                                               SPRITE_DIAMETER * 5, SPRITE_DIAMETER * 6 ) ) )
        return hr;

    return S_OK;
}



