//-----------------------------------------------------------------------------
// File: WindowedMode.cpp
//
// Desc: This sample demonstrates how to use DirectDraw in windowed mode.
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

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480

#define SPRITE_DIAMETER 48
#define NUM_SPRITES     25

#define HELPTEXT TEXT("Press Escape to quit.")

struct SPRITE_STRUCT
{
    FLOAT fPosX; 
    FLOAT fPosY;
    FLOAT fVelX; 
    FLOAT fVelY;
};

CDisplay*            g_pDisplay        = NULL;
CSurface*            g_pLogoSurface    = NULL;  
CSurface*            g_pTextSurface    = NULL;  
RECT                 g_rcViewport;          
RECT                 g_rcScreen;            
BOOL                 g_bActive     = FALSE; 
DWORD                g_dwLastTick;
SPRITE_STRUCT        g_Sprite[NUM_SPRITES]; 




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
HRESULT InitDirectDraw( HWND hWnd );
VOID    FreeDirectDraw();
HRESULT ProcessNextFrame( HWND hWnd );
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
                if( FAILED( ProcessNextFrame( hWnd ) ) )
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
    WNDCLASSEX wc;
    HWND       hWnd;
    HACCEL     hAccel;

    // Register the Window Class
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = TEXT("WindowedMode");
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hIconSm       = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
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
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
    hWnd = CreateWindowEx( 0, TEXT("WindowedMode"), TEXT("DirectDraw WindowedMode Sample"),
                           dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
  	                       dwWindowWidth, dwWindowHeight, NULL, NULL, hInst, NULL );
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
    LPDIRECTDRAWPALETTE pDDPal = NULL; 
    HRESULT	hr;
    int     iSprite;

    g_pDisplay = new CDisplay();
    if( FAILED( hr = g_pDisplay->CreateWindowedDisplay( hWnd, WINDOW_WIDTH, WINDOW_HEIGHT ) ) )
    {
        MessageBox( hWnd, TEXT("Failed initializing DirectDraw."),
                    TEXT("DirectDraw Sample"), MB_ICONERROR | MB_OK );
        return hr;
    }

    // Create and set the palette when in palettized color
    if( FAILED( hr = g_pDisplay->CreatePaletteFromBitmap( &pDDPal, MAKEINTRESOURCE( IDB_DIRECTX ) ) ) )
        return hr;

    g_pDisplay->SetPalette( pDDPal );

    SAFE_RELEASE( pDDPal );

    // Create a surface, and draw a bitmap resource on it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pLogoSurface, MAKEINTRESOURCE( IDB_DIRECTX ), 
                                                          SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return hr;

    // Create a surface, and draw text to it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromText( &g_pTextSurface, NULL, HELPTEXT, 
                                                        RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    // Set the color key for the logo sprite to black
    if( FAILED( hr = g_pLogoSurface->SetColorKey( 0 ) ) )
        return hr;

    // Init all the sprites.  All of these sprites look the same, 
    // using the g_pDDSLogo surface. 
    for( iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
    {
        // Set the sprite's position and velocity
        g_Sprite[iSprite].fPosX = (float) (rand() % WINDOW_WIDTH);
        g_Sprite[iSprite].fPosY = (float) (rand() % WINDOW_HEIGHT); 

        g_Sprite[iSprite].fVelX = 500.0f * rand() / RAND_MAX - 250.0f;
        g_Sprite[iSprite].fVelY = 500.0f * rand() / RAND_MAX - 250.0f;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectDraw()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID FreeDirectDraw()
{
    SAFE_DELETE( g_pLogoSurface );
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

        case WM_PAINT:
            // Update the screen if we need to refresh. This case occurs 
            // when in windowed mode and the window is behind others.
            // The app will not be active, but it will be visible.
            if( g_pDisplay )
            {
                // Display the new position of the sprite
                if( DisplayFrame() == DDERR_SURFACELOST )
                {
                    // If the surfaces were lost, then restore and try again
                    RestoreSurfaces();
                    DisplayFrame();
                }
            }
            break; // Continue with default processing to validate the region

        case WM_QUERYNEWPALETTE:
            if( g_pDisplay )
            {
                // If we are in windowed mode with a desktop resolution in 8 bit 
                // color, then the palette we created during init has changed 
                // since then.  So get the palette back from the primary 
                // DirectDraw surface, and set it again so that DirectDraw 
                // realises the palette, then release it again. 
                LPDIRECTDRAWPALETTE pDDPal = NULL; 
                g_pDisplay->GetFrontBuffer()->GetPalette( &pDDPal );
                g_pDisplay->GetFrontBuffer()->SetPalette( pDDPal );
                SAFE_RELEASE( pDDPal );
            }
            break;

        case WM_GETMINMAXINFO:
            {
                // Don't allow resizing in windowed mode.  
                // Fix the size of the window to 640x480 (client size)
                MINMAXINFO* pMinMax = (MINMAXINFO*) lParam;

                DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
                DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
                DWORD dwMenuHeight    = GetSystemMetrics( SM_CYMENU );
                DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );

                pMinMax->ptMinTrackSize.x = WINDOW_WIDTH  + dwFrameWidth * 2;
                pMinMax->ptMinTrackSize.y = WINDOW_HEIGHT + dwFrameHeight * 2 + 
                                            dwMenuHeight + dwCaptionHeight;

                pMinMax->ptMaxTrackSize.x = pMinMax->ptMinTrackSize.x;
                pMinMax->ptMaxTrackSize.y = pMinMax->ptMinTrackSize.y;
            }
            return 0L;

        case WM_MOVE:
	        if( g_pDisplay )
		        g_pDisplay->UpdateBounds();
            return 0L;

        case WM_EXITMENULOOP:
            // Ignore time spent in menu
            g_dwLastTick = timeGetTime();
            break;

        case WM_EXITSIZEMOVE:
            // Ignore time spent resizing
            g_dwLastTick = timeGetTime();
            break;

        case WM_SIZE:
            // Check to see if we are losing our window...
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                g_bActive = FALSE;
            else
                g_bActive = TRUE;

	        if( g_pDisplay )
		        g_pDisplay->UpdateBounds();
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
//       flip or blt the back buffer to the primary buffer
//-----------------------------------------------------------------------------
HRESULT ProcessNextFrame( HWND hWnd )
{
    HRESULT hr;

    // Figure how much time has passed since the last time
    DWORD dwCurrTick = timeGetTime();
    DWORD dwTickDiff = dwCurrTick - g_dwLastTick;

    // Don't update if no time has passed 
    if( dwTickDiff == 0 )
        return S_OK; 

    g_dwLastTick = dwCurrTick;

    // Move the sprites according to how much time has passed
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
        UpdateSprite( &g_Sprite[ iSprite ], dwTickDiff / 1000.0f );

    // Check the cooperative level before rendering
    if( FAILED( hr = g_pDisplay->GetDirectDraw()->TestCooperativeLevel() ) )
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
                FreeDirectDraw();
                return InitDirectDraw( hWnd );
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
// Name: UpdateSprite()
// Desc: Move the sprite around and make it bounce based on how much time 
//       has passed
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

    if( pSprite->fPosX >= WINDOW_WIDTH - SPRITE_DIAMETER )
    {
        pSprite->fPosX = WINDOW_WIDTH - 1 - SPRITE_DIAMETER;
        pSprite->fVelX = -pSprite->fVelX;
    }

    if( pSprite->fPosY < 0 )
    {
        pSprite->fPosY = 0;
        pSprite->fVelY = -pSprite->fVelY;
    }

    if( pSprite->fPosY > WINDOW_HEIGHT - SPRITE_DIAMETER )
    {
        pSprite->fPosY = WINDOW_HEIGHT - 1 - SPRITE_DIAMETER;
        pSprite->fVelY = -pSprite->fVelY;
    }   
}




//-----------------------------------------------------------------------------
// Name: DisplayFrame()
// Desc: Blts a the sprites to the back buffer, then it blts or flips the 
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
    // ignoring errors until the last blt. Note that all of these sprites 
    // use the same DirectDraw surface.
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
    {
        g_pDisplay->Blt( (DWORD)g_Sprite[iSprite].fPosX, 
                         (DWORD)g_Sprite[iSprite].fPosY, 
                         g_pLogoSurface, NULL );
    }

    // We are in windowed mode so perform a blt from the backbuffer 
    // to the primary, returning any errors like DDERR_SURFACELOST
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
    LPDIRECTDRAWPALETTE pDDPal = NULL; 
    HRESULT hr;

    if( FAILED( hr = g_pDisplay->GetDirectDraw()->RestoreAllSurfaces() ) )
        return hr;

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = g_pTextSurface->DrawText( NULL, HELPTEXT, 
                                               0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    // We need to release and re-load, and set the palette again to 
    // redraw the bitmap on the surface.  Otherwise, GDI will not 
    // draw the bitmap on the surface with the right palette
    if( FAILED( hr = g_pDisplay->CreatePaletteFromBitmap( &pDDPal, MAKEINTRESOURCE( IDB_DIRECTX ) ) ) )
        return hr;

    g_pDisplay->SetPalette( pDDPal );

    SAFE_RELEASE( pDDPal );

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = g_pLogoSurface->DrawBitmap( MAKEINTRESOURCE( IDB_DIRECTX ),
                                                 SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return hr;

    return S_OK;
}




