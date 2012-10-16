//-----------------------------------------------------------------------------
// File: DirectSurfaceWrite.cpp
//
// Desc: This sample demonstrates how to animate sprites using
//       DirectDraw.  The samples runs in full-screen mode.  Pressing any
//       key will exit the sample.
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
#define SCREEN_BPP          16

#define SPRITE_DIAMETER     250
#define NUM_SPRITES         5

#define HELPTEXT TEXT("Press Escape to quit.")

struct SPRITE_STRUCT
{
    FLOAT fPosX; 
    FLOAT fPosY;
    FLOAT fVelX; 
    FLOAT fVelY;
};

CDisplay*            g_pDisplay           = NULL;
CSurface*            g_pSpriteSurface     = NULL;  
CSurface*            g_pTextSurface       = NULL;  
RECT                 g_rcViewport;          
RECT                 g_rcScreen;            
BOOL                 g_bActive            = FALSE; 
DWORD                g_dwLastTick;
SPRITE_STRUCT        g_Sprite[NUM_SPRITES]; 




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
HRESULT InitDirectDraw( HWND hWnd );
HRESULT DrawSprite();
VOID    FreeDirectDraw();
HRESULT ProcessNextFrame();
VOID    UpdateSprite( SPRITE_STRUCT* pSprite, FLOAT fTimeDelta );
HRESULT DisplayFrame();
HRESULT RestoreSurfaces();




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

    ZeroMemory( &g_Sprite, sizeof(SPRITE_STRUCT) * NUM_SPRITES );
    srand( GetTickCount() );

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

    g_dwLastTick = timeGetTime();

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

                // Ignore time spent inactive 
                g_dwLastTick = timeGetTime();
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
    wc.lpszClassName = TEXT("DirectSurfaceWrite");
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
    hWnd = CreateWindowEx( 0, TEXT("DirectSurfaceWrite"), TEXT("DirectDraw DirectSurfaceWrite Sample"),
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
    HRESULT        hr;

    g_pDisplay = new CDisplay();
    if( FAILED( hr = g_pDisplay->CreateFullScreenDisplay( hWnd, SCREEN_WIDTH, 
                                                          SCREEN_HEIGHT, SCREEN_BPP ) ) )
    {
        MessageBox( hWnd, TEXT("This display card does not support 640x480x8. "),
                    TEXT("DirectDraw Sample"), MB_ICONERROR | MB_OK );
        return hr;
    }

    // Create a DirectDrawSurface for this bitmap
    if( FAILED( hr = g_pDisplay->CreateSurface( &g_pSpriteSurface, SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return hr;

    if( FAILED( hr = DrawSprite() ) )
        return hr;
    
    // Create a surface, and draw text to it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromText( &g_pTextSurface, NULL, HELPTEXT, 
                                                        RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    // Init all the sprites.  All of these sprites using the 
    // same g_pDDSAnimationSheet surface, but depending on the
    // sprite's lFrame value, it indexes a different rect on the 
    // surface.
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
    {
        // Set the sprite's position and velocity
        g_Sprite[iSprite].fPosX = (float) (rand() % SCREEN_WIDTH);
        g_Sprite[iSprite].fPosY = (float) (rand() % SCREEN_HEIGHT); 

        g_Sprite[iSprite].fVelX = 500.0f * rand() / RAND_MAX - 250.0f;
        g_Sprite[iSprite].fVelY = 500.0f * rand() / RAND_MAX - 250.0f;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawSprite()
// Desc: Draws a pattern of colors to a DirectDraw surface by directly writing
//       to the surface memory.  This function was designed to work only in 
//       16-bit color.
//-----------------------------------------------------------------------------
HRESULT DrawSprite()
{
    DDSURFACEDESC2 ddsd;
    HRESULT        hr;
    DWORD dwShift;
    DWORD dwBits;
    DWORD dwRed;
    DWORD dwGreen;
    DWORD dwBlue;
    FLOAT fPercentX;
    FLOAT fPercentY;
    FLOAT fPercentXY;

    LPDIRECTDRAWSURFACE7 pDDS = g_pSpriteSurface->GetDDrawSurface();

    ZeroMemory( &ddsd,sizeof(ddsd) );
    ddsd.dwSize = sizeof(ddsd);

    // Lock the surface to directly write to the surface memory 
    if( FAILED( hr = pDDS->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) ) )
        return hr;

    // Get a pointer into the memory starting at ddsd.lpSurface.  Cast this pointer to a 
    // 16-bit WORD since we are in 16 bit color, so each pixel has 16 bits of color information.  
    WORD* pDDSColor = (WORD*) ddsd.lpSurface;

    for( DWORD iY = 0; iY < ddsd.dwHeight; iY++ )
    {
        for( DWORD iX = 0; iX < ddsd.dwWidth; iX++ )
        {
            // Figure out the red component as a function of the Y location of the pixel
            CSurface::GetBitMaskInfo( ddsd.ddpfPixelFormat.dwRBitMask, &dwShift, &dwBits );
            fPercentY = (float) abs( ddsd.dwHeight / 2 - iY ) / (float) ( ddsd.dwHeight / 2 ) + 0.25f;
            if( fPercentY > 1.00f )
                fPercentY = 1.00f;
            dwRed = (DWORD) ( (ddsd.ddpfPixelFormat.dwRBitMask >> dwShift) * fPercentY) << dwShift;

            // Figure out the green component as a function of the X location of the pixel
            CSurface::GetBitMaskInfo( ddsd.ddpfPixelFormat.dwGBitMask, &dwShift, &dwBits );
            fPercentX = (float) abs( ddsd.dwWidth / 2 - iX ) / (float) ( ddsd.dwWidth / 2 ) + 0.25f;
            if( fPercentX > 1.00f )
                fPercentX = 1.00f;
            dwGreen = (DWORD) ( (ddsd.ddpfPixelFormat.dwGBitMask >> dwShift) * fPercentX) << dwShift;

            // Figure out the blue component as a function of the X and Y location of the pixel
            CSurface::GetBitMaskInfo( ddsd.ddpfPixelFormat.dwBBitMask, &dwShift, &dwBits );
            fPercentX = (float) abs( ddsd.dwWidth / 2 - iX ) / (float) ( ddsd.dwWidth / 4 );
            fPercentX = 1.0f - fPercentX * fPercentX;
            fPercentY = (float) abs( ddsd.dwHeight / 2 - iY ) / (float) ( ddsd.dwHeight / 4 );
            fPercentY = 1.0f - fPercentY * fPercentY;
            fPercentXY = fPercentX + fPercentY;
            if( fPercentXY > 1.0f )
                fPercentXY = 1.0f;
            if( fPercentXY < 0.0f )
                fPercentXY = 0.0f;
            dwBlue = (DWORD) ( (ddsd.ddpfPixelFormat.dwBBitMask >> dwShift) * fPercentXY ) << dwShift;

            // Make the dwDDSColor by combining all the color components
            *pDDSColor = (WORD) ( dwRed | dwGreen | dwBlue );

            // Advance the surface pointer by 16 bits (one pixel) 
            pDDSColor++;
        }

        // Multiply ddsd.lPitch by iY to figure out offset needed to access 
        // the next scan line on the surface. 
        pDDSColor = (WORD*) ( (BYTE*) ddsd.lpSurface + ( iY + 1 ) * ddsd.lPitch );
    }
    
    // Unlock the surface
    pDDS->Unlock(NULL); 

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectDraw()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID FreeDirectDraw()
{
    SAFE_DELETE( g_pSpriteSurface );
    SAFE_DELETE( g_pTextSurface );
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

        case WM_EXITMENULOOP:
            // Ignore time spent in menu
            g_dwLastTick = timeGetTime();
            break;

        case WM_EXITSIZEMOVE:
            // Ignore time spent resizing
            g_dwLastTick = timeGetTime();
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
// Name: ProcessNextFrame()
// Desc: Move the sprites, blt them to the back buffer, then 
//       flips the back buffer to the primary buffer
//-----------------------------------------------------------------------------
HRESULT ProcessNextFrame()
{
    HRESULT hr;

    // Figure how much time has passed since the last time
    DWORD dwCurrTick = timeGetTime();
    DWORD dwTickDiff = dwCurrTick - g_dwLastTick;

    // Don't update if no time has passed 
    if( dwTickDiff == 0 )
        return S_OK; 

    g_dwLastTick = dwCurrTick;

    // Update the sprites according to how much time has passed
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
        UpdateSprite( &g_Sprite[ iSprite ], dwTickDiff / 1000.0f );

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
// Name: UpdateSprite()
// Desc: Moves and bounces the sprite based on how much time has passed.  
//       It also changes the sprite state based on random values it gets 
//       from the array g_lRandTable
//-----------------------------------------------------------------------------
VOID UpdateSprite( SPRITE_STRUCT* pSprite, FLOAT fTimeDelta )
{    
    // Update the sprite position
    pSprite->fPosX += pSprite->fVelX * fTimeDelta;
    pSprite->fPosY += pSprite->fVelY * fTimeDelta;

    // Clip the position, and bounce if it hits the edge
    if( pSprite->fPosX < 0.0f )
    {
        pSprite->fPosX  = 0;
        pSprite->fVelX = -pSprite->fVelX;
    }

    if( pSprite->fPosX >= SCREEN_WIDTH - SPRITE_DIAMETER )
    {
        pSprite->fPosX = SCREEN_WIDTH - 1 - SPRITE_DIAMETER;
        pSprite->fVelX = -pSprite->fVelX;
    }

    if( pSprite->fPosY < 0 )
    {
        pSprite->fPosY = 0;
        pSprite->fVelY = -pSprite->fVelY;
    }

    if( pSprite->fPosY > SCREEN_HEIGHT - SPRITE_DIAMETER )
    {
        pSprite->fPosY = SCREEN_HEIGHT - 1 - SPRITE_DIAMETER;
        pSprite->fVelY = -pSprite->fVelY;
    }   
}




//-----------------------------------------------------------------------------
// Name: DisplayFrame()
// Desc: Blts a the sprites to the back buffer, then flips the 
//       back buffer onto the primary buffer.
//-----------------------------------------------------------------------------
HRESULT DisplayFrame()
{
    HRESULT hr;

    // Fill the back buffer with black, ignoring errors until the flip
    g_pDisplay->Clear( 0 );

    // Blt the help text on the backbuffer, ignoring errors until the flip
    g_pDisplay->Blt( 10, 10, g_pTextSurface, NULL );

    // Blt all the sprites onto the back buffer using color keying,
    // ignoring errors until the flip. Note that all of these sprites 
    // use the same DirectDraw surface.
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
    {
        g_pDisplay->Blt( (DWORD)g_Sprite[iSprite].fPosX, 
                         (DWORD)g_Sprite[iSprite].fPosY, 
                         g_pSpriteSurface, NULL );
    }

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
    if( FAILED( hr = g_pTextSurface->DrawText( NULL, HELPTEXT, 
                                               0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = DrawSprite() ) )
        return hr;

    return S_OK;
}




