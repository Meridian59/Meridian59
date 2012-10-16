//-----------------------------------------------------------------------------
// File: Multimon.cpp
//
// Desc: This sample demonstrates the following programming concepts:
//   Writing code for a multi-monitor program that works on both Windows 95 
//     (which does not support multiple monitors) and later Windows versions 
//     (which do support it).
//   Using DirectDrawEnumerateEx to enumerate displays.
//   Working with separate device and focus windows.
//   Creating a video-memory sprite that spans multiple screens using multiple
//     copies of the image data.
//   Creating a system-memory sprite that spans multiple screens using a shared
//     copy of the image data.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------




// Multimon.h implements support of the multimon APIs for when your program
// runs on Windows 95 systems.  You can #include multimon.h in all your source
// files that use multimon APIs, and you should #define COMPILE_MULTIMON_STUBS 
// in only one of your source files.
#define COMPILE_MULTIMON_STUBS 
#include <windows.h>
#include <multimon.h>
#include <ddraw.h>
#include "resource.h"



//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define NAME  TEXT("MultiMon DirectDraw Sample")
#define TITLE TEXT("MultiMon DirectDraw Sample")


// An EnumInfo contains extra enumeration information that is passed
// into the the DDEnumCallbackEx function.
struct EnumInfo
{
    BOOL    bMultimonSupported;
    HRESULT hr;
};


// A Screen represents one display that images can be drawn to.
struct Screen
{
    GUID                 guid;
    TCHAR                szDesc[200];
    HMONITOR             hmon;
    LPDIRECTDRAW7        pDD;
    LPDIRECTDRAWSURFACE7 pDDSFront;
    LPDIRECTDRAWSURFACE7 pDDSBack;
    Screen*              pScreenNext; // For linked list
};


// A ScreenSurface holds a DirectDrawSurface that can be used on a 
// particular screen.
struct ScreenSurface
{
    Screen*              pScreen;
    LPDIRECTDRAWSURFACE7 pDDS;
    ScreenSurface*       pScreenSurfaceNext; // For linked list
    // Could add a "last used time" field, which could be used to
    // determine whether this ScreenSurface should be
    // removed to free up video memory for another surface
};


// A Sprite holds generic information about a drawable image, and
// a linked list of ScreenSurfaces (one per screen).
struct Sprite
{
    TCHAR          szName[64];   // Name of this Sprite
    BOOL           bForceSystem; // If TRUE, don't try to create video memory surfaces
    RECT           rcSrc;        // Dimensions of the image
    RECT           rcDest;       // Destination rectangle to draw image into
    LONG           xVel;         // X-Velocity for animation
    LONG           yVel;         // Y-Velocity for animation
    HBITMAP        hbmImage;     // Loaded bitmap image
    BYTE*          pImageData;   // Sharable pointer to DD surface data
    DDSURFACEDESC2 ddsd;         // Holds pitch and pixel format of pImageData
    ScreenSurface* pScreenSurfaceFirst; // Linked list of ScreenSurfaces
};


Screen* g_pScreenFirst = NULL;   // Linked list of Screens
Sprite* g_pSprite1     = NULL;   // A sprite that uses video memory where possible
Sprite* g_pSprite2     = NULL;   // A sprite that always uses system memory
HWND    g_hWnd         = NULL;   // Main app focus HWND
BOOL    g_bActive      = TRUE;   // Whether app is actively drawing




//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow );
HRESULT CreateFocusWindow( HINSTANCE hInstance );
HRESULT EnumerateScreens( VOID );
BOOL WINAPI DDEnumCallback( GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName, VOID* pContext );
BOOL WINAPI DDEnumCallbackEx( GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName, VOID* pContext, HMONITOR hmon );
HRESULT InitScreens( VOID );
HRESULT InitSprites( VOID );
HRESULT MainLoop( VOID );
LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
VOID UpdateFrame( VOID );
BOOL RectIntersectsMonitor( RECT* prcSrc, HMONITOR hmon, RECT* prcScreen );
HRESULT FindOrBuildScreenSurface( Sprite* pSprite, Screen* pScreen, ScreenSurface** ppScreenSurface );
HRESULT SetupScreenSurfaceDDS( Sprite* pSprite, ScreenSurface* pScreenSurface );
VOID DestroyScreenSurfaces( Sprite* pSprite );
VOID Cleanup( VOID );




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Initializes the application, then starts the main application loop.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow )
{
    HRESULT hr;

    if( FAILED( hr = CreateFocusWindow(hInst) ) )
        return 0;

    if( FAILED( hr = EnumerateScreens() ) )
        return 0;

    if( FAILED( hr = InitScreens() ) )
        return 0;

    if( FAILED( hr = InitSprites() ) )
        return 0;

    if( FAILED( hr = MainLoop() ) )
        return 0;

    return 0;
}




//-----------------------------------------------------------------------------
// Name: CreateFocusWindow()
// Desc: Creates the focus window, which is the window which will receive user 
//       input.
//-----------------------------------------------------------------------------
HRESULT CreateFocusWindow( HINSTANCE hInstance )
{
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ICON1) );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = NAME;
    RegisterClass( &wc );
    
    // Window style and dimensions are not important, 
    // since they will be adjusted elsewhere.
    g_hWnd = CreateWindowEx( 0, NAME, TITLE, 0, 0, 0, 0, 0, 
                             NULL, NULL, hInstance, NULL );

    // It is important to call ShowWindow on this window to ensure 
    // that any activity from other windows is obscured.
    ShowWindow( g_hWnd, SW_SHOW );

    if( g_hWnd == NULL )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumerateScreens()
// Desc: Creates a Screen structure for every appropriate screen in the user's
//       computer.  
//-----------------------------------------------------------------------------
HRESULT EnumerateScreens()
{
    HRESULT  hr;
    EnumInfo enumInfo;

    ZeroMemory( &enumInfo, sizeof(enumInfo) );
    enumInfo.bMultimonSupported = TRUE;

    if( FAILED( hr = DirectDrawEnumerateEx( DDEnumCallbackEx, 
                                            &enumInfo, 
                                            DDENUM_ATTACHEDSECONDARYDEVICES ) ) )
        return hr;

    // If something failed inside the enumeration, be sure to return that HRESULT
    if( FAILED(enumInfo.hr) )
        return enumInfo.hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DDEnumCallbackEx()
// Desc: This callback function is called by DirectDraw once for each 
//       available DirectDraw device.  In this implementation, it saves the
//       GUID, device description, and hmon in a Screen structure for later use.
//-----------------------------------------------------------------------------
BOOL WINAPI DDEnumCallbackEx( GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName,
                              VOID* pContext, HMONITOR hmon )
{
    Screen*   pScreenNew;
    EnumInfo* pEnumInfo = (EnumInfo*)pContext;
    GUID      guidNull;

    ZeroMemory( &guidNull, sizeof(GUID) );
    if( g_pScreenFirst != NULL && 
        g_pScreenFirst->guid == guidNull )
    {
        // We must be running on a multimon system, so get rid of the 
        // guidNull Screen -- we want Screens with specific GUIDs.
        delete g_pScreenFirst;
        g_pScreenFirst = NULL;
    }

    // Store all the info in a Screen structure
    pScreenNew = new Screen;
    if( pScreenNew == NULL )
    {
        pEnumInfo->hr = E_OUTOFMEMORY;
        return FALSE; // fatal error, stop enumerating
    }

    ZeroMemory( pScreenNew, sizeof(Screen) );
    if( pGuid == NULL )
        pScreenNew->guid = guidNull;
    else
        pScreenNew->guid = *pGuid;

    lstrcpy( pScreenNew->szDesc, pszDesc );
    pScreenNew->hmon = hmon;

    // Insert Screen into global linked list
    if( g_pScreenFirst == NULL )
    {
        g_pScreenFirst = pScreenNew;
    }
    else
    {
        // Insert at end of list
        Screen* pScreen = g_pScreenFirst; 
        while( pScreen->pScreenNext != NULL )
            pScreen = pScreen->pScreenNext;

        pScreen->pScreenNext = pScreenNew;
    }

    return TRUE; // Keep enumerating
}




//-----------------------------------------------------------------------------
// Name: InitScreens()
// Desc: For each Screen, this function initializes DirectDraw and sets up the 
//       front buffer, back buffer, and a clipper.  Many fullscreen DirectDraw
//       programs don't need to create clippers, but this one does so to allow
//       the sprites to be automatically clipped to each display.
//-----------------------------------------------------------------------------
HRESULT InitScreens( VOID )
{
    HRESULT             hr;
    Screen*             pScreen = NULL;
    GUID*               pGuid   = NULL;
    DWORD               dwFlags;
    DDSURFACEDESC2      ddsd;
    LPDIRECTDRAWCLIPPER pClip   = NULL;
    DDSCAPS2            ddsCaps;
    RECT                rc;
    HRGN                hrgn;
    BYTE                rgnDataBuffer[1024];
    GUID                guidNull;

    ZeroMemory( &guidNull, sizeof(GUID) );

    for( pScreen = g_pScreenFirst; pScreen != NULL; pScreen = pScreen->pScreenNext )
    {
        if( pScreen->guid == guidNull )
            pGuid = NULL;
        else
            pGuid = &pScreen->guid;

        if( FAILED(hr = DirectDrawCreateEx( pGuid, (VOID**) &(pScreen->pDD), 
                                            IID_IDirectDraw7, NULL ) ) )
            return hr;

        if( pScreen == g_pScreenFirst)
        {
            dwFlags = DDSCL_SETFOCUSWINDOW;
            if( FAILED( hr = pScreen->pDD->SetCooperativeLevel( g_hWnd, dwFlags ) ) )
                return hr;

            dwFlags = DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
            if( FAILED( hr = pScreen->pDD->SetCooperativeLevel(g_hWnd, dwFlags ) ) )
                return hr;
        }
        else
        {
            dwFlags = DDSCL_SETFOCUSWINDOW | DDSCL_CREATEDEVICEWINDOW |
                      DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
            if( FAILED( hr = pScreen->pDD->SetCooperativeLevel(g_hWnd, dwFlags ) ) )
                return hr;
        }

        if( FAILED( hr = pScreen->pDD->SetDisplayMode( 640, 480, 16, 0, 0 ) ) )
            return hr;
    }

    // Note: It is recommended that programs call SetDisplayMode on all screens
    // before creating/acquiring any DirectDrawSurfaces.
    for( pScreen = g_pScreenFirst; pScreen != NULL; pScreen = pScreen->pScreenNext )
    {
        ZeroMemory( &ddsd, sizeof(ddsd) );
        ddsd.dwSize            = sizeof(ddsd);
        ddsd.dwFlags           = DDSD_BACKBUFFERCOUNT | DDSD_CAPS;
        ddsd.dwBackBufferCount = 1;
        ddsd.ddsCaps.dwCaps    = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE;

        if( FAILED( hr = pScreen->pDD->CreateSurface( &ddsd, 
                                                      &pScreen->pDDSFront, NULL ) ) )
        {
            return hr;
        }


        ZeroMemory( &ddsCaps, sizeof(ddsCaps) );
        ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
        if( FAILED( hr = pScreen->pDDSFront->GetAttachedSurface( &ddsCaps, 
                                                                 &pScreen->pDDSBack ) ) )
            return hr;

        ZeroMemory( &ddsd, sizeof(ddsd) );
        ddsd.dwSize = sizeof(ddsd);
        if( FAILED( hr = pScreen->pDDSFront->GetSurfaceDesc( &ddsd ) ) )
            return hr;

        SetRect( &rc, 0, 0, ddsd.dwWidth, ddsd.dwHeight );
        hrgn = CreateRectRgn( 0, 0, ddsd.dwWidth, ddsd.dwHeight );
        GetRegionData( hrgn, sizeof(rgnDataBuffer), (RGNDATA*)rgnDataBuffer );
        DeleteObject( hrgn );

        if( FAILED( hr = pScreen->pDD->CreateClipper( 0, &pClip, 0 ) ) )
            return hr;

        if( FAILED( hr = pClip->SetClipList( (RGNDATA*)rgnDataBuffer, 0 ) ) )
        {
            pClip->Release();
            return hr;
        }

        if( FAILED( hr = pScreen->pDDSFront->SetClipper( pClip ) ) )
        {
            pClip->Release();
            return hr;
        }

        if( FAILED( hr = pScreen->pDDSBack->SetClipper( pClip ) ) )
        {
            pClip->Release();
            return hr;
        }

        pClip->Release();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitSprites()
// Desc: Initializes the objects that will be drawn and animated.  Note that
//       the ScreenSurfaces are created when they are first needed, not here.
//-----------------------------------------------------------------------------
HRESULT InitSprites( VOID )
{
    BITMAP bm;

    // Initialize the first Sprite.  This sprite will try to use video memory
    // on each Screen.
    g_pSprite1 = new Sprite;
    if( g_pSprite1 == NULL )
        return E_OUTOFMEMORY;

    ZeroMemory( g_pSprite1, sizeof(Sprite) );
    lstrcpy( g_pSprite1->szName, TEXT("Sprite 1") );
    g_pSprite1->bForceSystem = FALSE; // This sprite will try to use video memory

    g_pSprite1->hbmImage = (HBITMAP) LoadImage( GetModuleHandle(NULL), 
                                                MAKEINTRESOURCE(IDB_BITMAP1), 
                                                IMAGE_BITMAP, 0, 0,
                                                LR_CREATEDIBSECTION );
    if( g_pSprite1->hbmImage == NULL )
        return E_FAIL;

    GetObject( g_pSprite1->hbmImage, sizeof(bm), &bm );  // get size of bitmap
    SetRect( &g_pSprite1->rcSrc, 0, 0, bm.bmWidth, bm.bmHeight );

    g_pSprite1->rcDest = g_pSprite1->rcSrc;
    g_pSprite1->xVel = 2; // Animation velocity
    g_pSprite1->yVel = 1;

    // Initialize the second Sprite.  This sprite will use system memory (and
    // share that memory between ScreenSurfaces whenever possible).
    g_pSprite2 = new Sprite;
    if( g_pSprite2 == NULL )
        return E_OUTOFMEMORY;

    ZeroMemory( g_pSprite2, sizeof(Sprite) );
    lstrcpy(g_pSprite2->szName, TEXT("Sprite 2"));
    g_pSprite2->bForceSystem = TRUE; // This sprite will always use system memory
    g_pSprite2->hbmImage = (HBITMAP) LoadImage( GetModuleHandle(NULL), 
                                                MAKEINTRESOURCE(IDB_BITMAP2), 
                                                IMAGE_BITMAP, 0, 0, 
                                                LR_CREATEDIBSECTION );
    if( g_pSprite2->hbmImage == NULL )
        return E_FAIL;

    GetObject( g_pSprite2->hbmImage, sizeof(bm), &bm );  // get size of bitmap
    SetRect( &g_pSprite2->rcSrc, 0, 0, bm.bmWidth, bm.bmHeight );
    g_pSprite2->rcDest = g_pSprite1->rcSrc;
    g_pSprite2->xVel = -1; // Animation velocity
    g_pSprite2->yVel = -2;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MainLoop()
// Desc: The main window message pump.  When the application is active (not
//       minimized), it uses PeekMessage so that UpdateFrame can be called
//       frequently once all window messages are handled.  When it is not 
//       active, GetMessage is used instead to give more processing time to
//       other running programs.
//-----------------------------------------------------------------------------
HRESULT MainLoop( VOID )
{
    MSG msg;
    BOOL bGotMsg;

    while( TRUE )
    {
        if( g_bActive)
            bGotMsg = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
        else
            bGotMsg = GetMessage( &msg, NULL, 0, 0);

        if( msg.message == WM_QUIT)
            return S_OK;
        
        if( bGotMsg)
        {
            TranslateMessage( &msg ); 
            DispatchMessage( &msg );
        }
        else if( g_bActive)
        {
            UpdateFrame();
        }
    }
}




//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: Handler for window messages.
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
    case WM_SIZE:
        if( SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam )
        {
            g_bActive = FALSE;
            // Give window an icon and system menu on the taskbar when minimized
            SetWindowLong(hWnd, GWL_STYLE, WS_SYSMENU); 
        }
        else
        {
            g_bActive = TRUE;
            // Remove any window "decoration" when fullscreen
            SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
        }

        return DefWindowProc( hWnd, uMsg, wParam, lParam );

    case WM_CLOSE:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );

    case WM_CHAR:
        DestroyWindow( hWnd );
        return 0;

    case WM_DESTROY:
        Cleanup();
        PostQuitMessage( 0 );
        return 0;

    default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateFrame()
// Desc: Renders one frame of animation, then updates the sprites for the next
//       frame.
//-----------------------------------------------------------------------------
VOID UpdateFrame( VOID )
{
    HRESULT        hr;
    Screen*        pScreen;
    RECT           rcScreen;
    ScreenSurface* pScreenSurface;
    DDBLTFX        ddbltfx;

    for( pScreen = g_pScreenFirst; pScreen != NULL; pScreen = pScreen->pScreenNext )
    {
        // Handle lost surfaces
        if( pScreen->pDDSFront->IsLost() == DDERR_SURFACELOST )
        {
            // Though surface memory can be reaquired via RestoreAllSurfaces, the
            // image contents will be undefined.  So we destroy all the ScreenSurfaces
            // and let them be recreated as needed.  Calling RestoreAllSurfaces
            // takes care of the remaining surfaces (the front and back buffers).
            DestroyScreenSurfaces( g_pSprite1 );
            DestroyScreenSurfaces( g_pSprite2 );
            hr = pScreen->pDD->RestoreAllSurfaces();
        }

        // Clear the back buffer for this Screen
        ZeroMemory( &ddbltfx, sizeof(ddbltfx) );
        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwFillColor = 0; // Black
        hr = pScreen->pDDSBack->Blt( NULL, NULL, NULL, 
                                     DDBLT_COLORFILL | DDBLT_WAIT, 
                                     &ddbltfx );

        // Draw first sprite
        if( RectIntersectsMonitor( &g_pSprite1->rcDest, pScreen->hmon, &rcScreen ) )
        {
            hr = FindOrBuildScreenSurface( g_pSprite1, pScreen, &pScreenSurface );

            if( SUCCEEDED(hr) )
            {
                hr = pScreen->pDDSBack->Blt( &rcScreen, pScreenSurface->pDDS, 
                                             &g_pSprite1->rcSrc, 
                                             DDBLT_WAIT, NULL );
            }
        }

        // Draw second sprite
        if( RectIntersectsMonitor( &g_pSprite2->rcDest, pScreen->hmon, &rcScreen ) )
        {
            hr = FindOrBuildScreenSurface( g_pSprite2, pScreen, &pScreenSurface );
            if( SUCCEEDED( hr ) )
            {
                hr = pScreen->pDDSBack->Blt( &rcScreen, pScreenSurface->pDDS, 
                                             &g_pSprite2->rcSrc, DDBLT_WAIT, NULL );
            }
        }
    }

    // Flip all screens.  This is done in a separate loop to make the flips happen
    // as close together in time as possible
    for( pScreen = g_pScreenFirst; pScreen != NULL; pScreen = pScreen->pScreenNext )
    {
        hr = pScreen->pDDSFront->Flip( NULL, DDFLIP_WAIT );
    }

    // Animate Sprites for the next frame.  The sprites are bounced against the 
    // virtual desktop, which may cause them to partially or totally disappear
    // if your screens are set up in a way that is non-rectangular.  Since the
    // animation is not the purpose of this demo, this simplified approach is used.
    RECT rcDesktop;
    rcDesktop.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
    rcDesktop.right  = rcDesktop.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    rcDesktop.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
    rcDesktop.bottom = rcDesktop.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // Animate first sprite
    OffsetRect( &g_pSprite1->rcDest, g_pSprite1->xVel, g_pSprite1->yVel );

    if( ( g_pSprite1->rcDest.right > rcDesktop.right && g_pSprite1->xVel > 0 ) ||
        ( g_pSprite1->rcDest.left < rcDesktop.left && g_pSprite1->xVel < 0 ) )
    {
        g_pSprite1->xVel = -g_pSprite1->xVel;
    }

    if( ( g_pSprite1->rcDest.bottom > rcDesktop.bottom && g_pSprite1->yVel > 0 ) ||
        ( g_pSprite1->rcDest.top < rcDesktop.top && g_pSprite1->yVel < 0 ) )
    {
        g_pSprite1->yVel = -g_pSprite1->yVel;
    }

    // Animate second sprite
    OffsetRect( &g_pSprite2->rcDest, g_pSprite2->xVel, g_pSprite2->yVel );
    if( ( g_pSprite2->rcDest.right > rcDesktop.right && g_pSprite2->xVel > 0 ) ||
        ( g_pSprite2->rcDest.left < rcDesktop.left  && g_pSprite2->xVel < 0 ) )
    {
        g_pSprite2->xVel = -g_pSprite2->xVel;
    }

    if( ( g_pSprite2->rcDest.bottom > rcDesktop.bottom && g_pSprite2->yVel > 0 ) ||
        ( g_pSprite2->rcDest.top < rcDesktop.top && g_pSprite2->yVel < 0 ) )
    {
        g_pSprite2->yVel = -g_pSprite2->yVel;
    }
}




//-----------------------------------------------------------------------------
// Name: RectIntersectsMonitor()
// Desc: Returns TRUE if prcSrc intersects the monitor hmon, and uses prcScreen
//       to store prcSrc in that monitor's local coordinate system.
//-----------------------------------------------------------------------------
BOOL RectIntersectsMonitor( RECT* prcSrc, HMONITOR hmon, RECT* prcScreen )
{
    MONITORINFO mi;
    RECT rcIntersection;
    BOOL bIntersects;

    ZeroMemory( &mi, sizeof(mi) );
    mi.cbSize = sizeof(mi);
    if( hmon == NULL )
    {
        SetRect( &mi.rcMonitor, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
                 GetSystemMetrics(SM_CYSCREEN) );
    }
    else
    {
        GetMonitorInfo(hmon, &mi);
    }

    bIntersects = IntersectRect( &rcIntersection, prcSrc, &mi.rcMonitor );
    if( !bIntersects)
        return FALSE;

    *prcScreen = *prcSrc;
    OffsetRect( prcScreen, -mi.rcMonitor.left, -mi.rcMonitor.top );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: FindOrBuildScreenSurface()
// Desc: This is called when UpdateFrame needs to draw the image of a Sprite 
//       onto a particular Screen.  If a ScreenSurface already exists for this
//       Screen and Sprite, a pointer to it is returned.  Otherwise, a new
//       ScreenSurface is created (and stored for future reuse).
//-----------------------------------------------------------------------------
HRESULT FindOrBuildScreenSurface( Sprite* pSprite, Screen* pScreen, 
                                  ScreenSurface** ppScreenSurface )
{
    HRESULT hr;
    ScreenSurface* pScreenSurface;

    for( pScreenSurface = pSprite->pScreenSurfaceFirst; 
         pScreenSurface != NULL; 
         pScreenSurface = pScreenSurface->pScreenSurfaceNext )
    {
        if( pScreenSurface->pScreen == pScreen )
        {
            // ScreenSurface exists for this Screen, so return a pointer to it
            *ppScreenSurface = pScreenSurface;
            return S_OK;
        }
    }

    // No ScreenSurface for this Screen exists yet, so build one.
    pScreenSurface = new ScreenSurface;
    if( pScreenSurface == NULL )
        return E_OUTOFMEMORY;

    ZeroMemory( pScreenSurface, sizeof(ScreenSurface) );

    pScreenSurface->pScreen = pScreen;
    if( FAILED( hr = SetupScreenSurfaceDDS(pSprite, pScreenSurface ) ) )
    {
        delete pScreenSurface;
        return hr;
    }

    // Insert this new ScreenSurface in the Sprite's list:
    pScreenSurface->pScreenSurfaceNext = pSprite->pScreenSurfaceFirst;
    pSprite->pScreenSurfaceFirst = pScreenSurface;
    *ppScreenSurface = pScreenSurface;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetupScreenSurfaceDDS()
// Desc: Generates the DirectDrawSurface for a new ScreenSurface, and draws
//       the appropriate image into it.
//-----------------------------------------------------------------------------
HRESULT SetupScreenSurfaceDDS( Sprite* pSprite, ScreenSurface* pScreenSurface )
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    TCHAR sz[200];
    Screen* pScreen = pScreenSurface->pScreen;

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth = pSprite->rcSrc.right - pSprite->rcSrc.left;
    ddsd.dwHeight = pSprite->rcSrc.bottom - pSprite->rcSrc.top;
    ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

    // Try to create the surface in video memory, unless the bForceSystem flag
    // is set on the Sprite.
    if( pSprite->bForceSystem || 
        FAILED( hr = pScreen->pDD->CreateSurface( &ddsd, &pScreenSurface->pDDS, NULL ) ) )
    {
        // Either this sprite has the bForceSystem flag, or creation in video
        // memory failed, so try to create the surface in system memory.
        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;

        if( FAILED( hr = pScreen->pDD->CreateSurface( &ddsd, &pScreenSurface->pDDS, NULL ) ) )
            return hr;
    }

    if( ddsd.ddsCaps.dwCaps == DDSCAPS_SYSTEMMEMORY && 
        pSprite->pImageData != NULL )
    {
        // See if we can reuse the image data that is stored in the Sprite.
        // As long as the pixel formats match, the image is reusable.
        ZeroMemory( &ddsd, sizeof(ddsd) );
        ddsd.dwSize = sizeof(ddsd);

        if( FAILED( hr = pScreenSurface->pDDS->GetSurfaceDesc( &ddsd ) ) )
            return hr;

        if( ddsd.ddpfPixelFormat.dwRGBBitCount == pSprite->ddsd.ddpfPixelFormat.dwRGBBitCount &&
            ddsd.ddpfPixelFormat.dwRBitMask    == pSprite->ddsd.ddpfPixelFormat.dwRBitMask &&
            ddsd.ddpfPixelFormat.dwGBitMask    == pSprite->ddsd.ddpfPixelFormat.dwGBitMask &&
            ddsd.ddpfPixelFormat.dwBBitMask    == pSprite->ddsd.ddpfPixelFormat.dwBBitMask )
        {
            // Make the DDS use the Sprite's pImageData for its surface contents
            if( FAILED( hr = pScreenSurface->pDDS->SetSurfaceDesc( &pSprite->ddsd, 0 ) ) )
                return hr;

            return S_OK; // All done!  This DDS is ready to use.
        }
        // Otherwise, we can't share image data, and this system memory surface
        // will be for this Screen only.
    }

    // Copy image data from the Sprite to this ScreenSurface:
    HDC hdc;
    if( FAILED( hr = pScreenSurface->pDDS->GetDC( &hdc ) ) )
        return hr;

    HDC     hdcImage;
    HGDIOBJ hgdiobjOld;
    DWORD   dwWidth = pSprite->rcSrc.right - pSprite->rcSrc.left;
    DWORD   dwHeight = pSprite->rcSrc.bottom - pSprite->rcSrc.top;

    hdcImage   = CreateCompatibleDC(NULL);
    hgdiobjOld = SelectObject(hdcImage, pSprite->hbmImage);
    StretchBlt( hdc, 0, 0, dwWidth, dwHeight, 
                hdcImage, 0, 0, dwWidth, dwHeight, SRCCOPY );

    SelectObject( hdcImage, hgdiobjOld ); // restore previously selected object
    DeleteDC( hdcImage );
    TextOut( hdc, 0, 0, pSprite->szName, lstrlen(pSprite->szName) );
    pScreenSurface->pDDS->ReleaseDC(hdc);

    if( ddsd.ddsCaps.dwCaps == DDSCAPS_VIDEOMEMORY )
    {
        if( FAILED( hr = pScreenSurface->pDDS->GetDC( &hdc ) ) )
            return hr;

        wsprintf( sz, TEXT("Video memory copy") );
        TextOut( hdc, 0, 20, sz, lstrlen(sz) );

        wsprintf( sz, TEXT("for %s"), pScreen->szDesc );
        TextOut( hdc, 0, 40, sz, lstrlen(sz) );

        pScreenSurface->pDDS->ReleaseDC(hdc);
    }
    else if( pSprite->pImageData == NULL )
    {
        // No shared copy exists yet, so create one using data in this 
        // system memory surface.
        if( FAILED( hr = pScreenSurface->pDDS->GetDC( &hdc ) ) )
            return hr;

        wsprintf(sz, TEXT("Shared System memory copy") );
        TextOut(hdc, 0, 20, sz, lstrlen(sz) );
        pScreenSurface->pDDS->ReleaseDC( hdc );

        // Copy image to pImageData so it can be shared among ScreenSurfaces:
        if( SUCCEEDED( hr = pScreenSurface->pDDS->Lock( NULL, &ddsd, 
                                                        DDLOCK_READONLY | DDLOCK_WAIT, 
                                                        NULL ) ) )
        {
            pSprite->pImageData = new BYTE[ ddsd.lPitch * ddsd.dwHeight ];
            if( pSprite->pImageData != NULL )
            {
                // Store size, pitch, pixel format, and surface pointer info in Sprite
                pSprite->ddsd           = ddsd; 
                pSprite->ddsd.lpSurface = pSprite->pImageData;
                pSprite->ddsd.dwFlags   = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | 
                                          DDSD_PIXELFORMAT | DDSD_LPSURFACE;

                // Copy image data from DDS's surface memory to Sprite's buffer
                CopyMemory( pSprite->pImageData, ddsd.lpSurface, 
                            ddsd.lPitch * ddsd.dwHeight );
            }

            pScreenSurface->pDDS->Unlock(NULL);

            if( pSprite->pImageData != NULL )
            {
                // May as well make this ScreenSurface use the sharable copy too:
                if( FAILED( hr = pScreenSurface->pDDS->SetSurfaceDesc( &pSprite->ddsd, 0 ) ) )
                    return hr;
            }
        }
    }
    else
    {
        // Shared copy exists, but attempt to use it failed (probably due to
        // mismatched pixel format), so indicate that this as a non-shared sysmem copy:
        if( FAILED( hr = pScreenSurface->pDDS->GetDC( &hdc ) ) )
            return hr;

        wsprintf( sz, TEXT("System memory copy") );
        TextOut( hdc, 0, 20, sz, lstrlen(sz) );
        wsprintf( sz, TEXT("for %s"), pScreen->szDesc );
        TextOut( hdc, 0, 40, sz, lstrlen(sz) );

        pScreenSurface->pDDS->ReleaseDC( hdc );
    }
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DestroyScreenSurfaces()
// Desc: Destroys all ScreenSurfaces attached to the given Sprite.  This is
//       called after restoring all surfaces (since image data may be lost) and
//       when preparing to exit the program.
//-----------------------------------------------------------------------------
VOID DestroyScreenSurfaces( Sprite* pSprite )
{
    ScreenSurface* pScreenSurface;
    ScreenSurface* pScreenSurfaceNext;

    pScreenSurface = pSprite->pScreenSurfaceFirst;
    pSprite->pScreenSurfaceFirst = NULL;
    while( pScreenSurface != NULL )
    {
        pScreenSurfaceNext = pScreenSurface->pScreenSurfaceNext;
        pScreenSurface->pDDS->Release();

        delete pScreenSurface;

        pScreenSurface = pScreenSurfaceNext;
    }

    if( pSprite->pImageData != NULL )
    {
        delete pSprite->pImageData;
        pSprite->pImageData = NULL;
    }

    ZeroMemory( &pSprite->ddsd, sizeof(pSprite->ddsd) );
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all resources allocated during this program.
//-----------------------------------------------------------------------------
VOID Cleanup( VOID )
{
    if( g_pSprite1 != NULL )
    {
        DestroyScreenSurfaces(g_pSprite1);
        delete g_pSprite1;
        g_pSprite1 = NULL;
    }

    if( g_pSprite2 != NULL )
    {
        DestroyScreenSurfaces(g_pSprite2);
        delete g_pSprite2;
        g_pSprite2 = NULL;
    }

    Screen* pScreen;
    Screen* pScreenNext;

    pScreen = g_pScreenFirst;
    g_pScreenFirst = NULL;

    while( pScreen != NULL )
    {
        pScreenNext = pScreen->pScreenNext;

        pScreen->pDDSBack->Release();
        pScreen->pDDSFront->Release();
        pScreen->pDD->RestoreDisplayMode();
        pScreen->pDD->SetCooperativeLevel(g_hWnd, DDSCL_NORMAL);
        pScreen->pDD->Release();

        delete pScreen;
        pScreen = pScreenNext;
    }
}


