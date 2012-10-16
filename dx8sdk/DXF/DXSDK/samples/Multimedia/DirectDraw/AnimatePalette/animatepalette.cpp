//-----------------------------------------------------------------------------
// File: AnimatePalette.cpp
//
// Desc: This sample demonstrates how to do DirectDraw palette animatation
//       when in full-screen mode.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <dxerr8.h>
#include "resource.h"
#include "ddutil.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      8

#define SPRITE_DIAMETER 48
#define NUM_SPRITES     25

#define HELPTEXT TEXT("Press Escape to quit.")

struct SPRITE_STRUCT
{
    FLOAT                fPosX; 
    FLOAT                fPosY;
    FLOAT                fVelX; 
    FLOAT                fVelY;
};

CDisplay*            g_pDisplay        = NULL;
CSurface*            g_pLogoSurface    = NULL;  
CSurface*            g_pTextSurface    = NULL;  
LPDIRECTDRAWPALETTE  g_pDDPal          = NULL; 
PALETTEENTRY         g_pe[256];
HWND                 g_hWnd            = NULL;
BOOL                 g_bActive         = FALSE; 
DWORD                g_dwLastTick;
SPRITE_STRUCT        g_Sprite[NUM_SPRITES]; 




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
HRESULT InitDirectDraw( HWND hWnd );
VOID    FreeDirectDraw();
HRESULT ProcessNextFrame();
VOID    UpdateSprite( SPRITE_STRUCT* pSprite, FLOAT fTimeDelta );
HRESULT CyclePalette();
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
    HACCEL   hAccel;
    HRESULT  hr;

    if( FAILED( WinInit( hInst, nCmdShow, &g_hWnd, &hAccel ) ) )
        return FALSE;

    if( FAILED( hr = InitDirectDraw( g_hWnd ) ) )
    {
        DXTRACE_ERR( TEXT("InitDirectDraw"), hr );
        MessageBox( g_hWnd, TEXT("DirectDraw init failed. ")
                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                    MB_ICONERROR | MB_OK );
        return FALSE;
    }

    // Display the sprites on the screen
    DisplayFrame();
  
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
            if( 0 == TranslateAccelerator( g_hWnd, hAccel, &msg ) )
            {
                TranslateMessage( &msg ); 
                DispatchMessage( &msg );
            }
        }
        else
        {
            if( g_bActive )
            {
                // Display the bitmap to the screen
                if( FAILED( hr = ProcessNextFrame() ) )
                {
                    SAFE_DELETE( g_pDisplay );
                           
                    if( hr == E_NOTIMPL )
                    {
                        MessageBox( NULL, TEXT("The driver does not support waiting for vertical blank. ")
                                    TEXT("This is expected if the HW acceleration is disabled. ")
                                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                                    MB_ICONERROR | MB_OK );
                    }
                    else
                    {                                                                          
                        DXTRACE_ERR( TEXT("ProcessNextFrame"), hr );
                        MessageBox( g_hWnd, TEXT("Displaying the next frame failed. ")
                                    TEXT("The sample will now exit. "), TEXT("DirectDraw Sample"), 
                                    MB_ICONERROR | MB_OK );
                    }
                    
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
    wc.lpszClassName = TEXT("AnimatePalette");
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
    hWnd = CreateWindowEx( 0, TEXT("AnimatePalette"), TEXT("DirectDraw AnimatePalette Sample"),
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
    int                 iSprite;

    g_pDisplay = new CDisplay();
    if( FAILED( hr = g_pDisplay->CreateFullScreenDisplay( hWnd, SCREEN_WIDTH, 
                                                          SCREEN_HEIGHT, SCREEN_BPP ) ) )
    {        
        MessageBox( hWnd, TEXT("This display card does not support 640x480x8. "),
                    TEXT("DirectDraw Sample"), MB_ICONERROR | MB_OK );
        return DXTRACE_ERR( TEXT("CreateFullScreenDisplay"), hr );
    }

    // Create and set the palette when in palettized color
    if( FAILED( hr = g_pDisplay->CreatePaletteFromBitmap( &g_pDDPal, MAKEINTRESOURCE( IDB_DIRECTX ) ) ) )
        return DXTRACE_ERR( TEXT("CreatePaletteFromBitmap"), hr );

    if( FAILED( hr = g_pDisplay->SetPalette( g_pDDPal ) ) )
        return DXTRACE_ERR( TEXT("SetPalette"), hr );

    // Store the palette entires in a global data structure 
    // so we can animate the palette
    if( FAILED( hr = g_pDDPal->GetEntries(0, 0, 256, g_pe ) ) )
        return DXTRACE_ERR( TEXT("GetEntries"), hr );

    // Create a surface, and draw a bitmap resource on it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pLogoSurface, MAKEINTRESOURCE( IDB_DIRECTX ), 
                                                          SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return DXTRACE_ERR( TEXT("CreateSurfaceFromBitmap"), hr );

    // Create a surface, and draw text to it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromText( &g_pTextSurface, NULL, HELPTEXT, 
                                                        RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return DXTRACE_ERR( TEXT("CreateSurfaceFromText"), hr );

    // Set the color key for the logo sprite to black
    if( FAILED( hr = g_pLogoSurface->SetColorKey( 0 ) ) )
        return DXTRACE_ERR( TEXT("SetColorKey"), hr );

    // Init all the sprites.  All of these sprites look the same, 
    // using the g_pDDSLogo surface. 
    for( iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
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

        case WM_EXITMENULOOP:
            // Ignore time spent in menu
            g_dwLastTick = timeGetTime();
            break;

        case WM_EXITSIZEMOVE:
            // Ignore time spent resizing
            g_dwLastTick = timeGetTime();
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

    // Move the sprites according to how much time has passed
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
        UpdateSprite( &g_Sprite[ iSprite ], dwTickDiff / 1000.0f );

    // Cycle the palette every frame
    if( FAILED( hr = CyclePalette() ) )
        return DXTRACE_ERR( TEXT("CyclePalette"), hr );

    // Display the sprites on the screen
    if( FAILED( hr = DisplayFrame() ) )
    {
        if( hr != DDERR_SURFACELOST )
        {
            FreeDirectDraw();
            return DXTRACE_ERR( TEXT("DisplayFrame"), hr );
        }

        // The surfaces were lost so restore them 
        RestoreSurfaces();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CyclePalette()
// Desc: Cycle the palette colors 2 through 37 - this is
//       will change the color for 'X' in the sprite.
//-----------------------------------------------------------------------------
HRESULT CyclePalette()
{
    HRESULT hr;

    for( int i = 2; i < 37; i++ )
    {
        // This just does something interesting but simple by shifting each 
        // color component independently 
        g_pe[ i ].peBlue  += 8;       
        g_pe[ i ].peRed   += 4;
        g_pe[ i ].peGreen += 2;
    }

    // Wait until the screen is synchronzied at a vertical-blank interval.
    // This may return E_NOTIMPL if the ddraw is not hardware accelerated
    hr = g_pDisplay->GetDirectDraw()->WaitForVerticalBlank( DDWAITVB_BLOCKBEGIN, NULL );
    if( FAILED( hr ) )
    {
        FreeDirectDraw();
        return DXTRACE_ERR( TEXT("WaitForVerticalBlank"), hr );
    }

    // Now that we are synchronzied at a vertical-blank 
    // interval, update the palette 
    if( FAILED( hr = g_pDDPal->SetEntries( 0, 0, 256, g_pe ) ) )
    {
        FreeDirectDraw();
        return DXTRACE_ERR( TEXT("SetEntries"), hr );
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

    if( NULL == g_pDisplay )
        return S_OK;

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
                         g_pLogoSurface, NULL );
    }

    // We are in fullscreen mode, so perform a flip and return 
    // any errors like DDERR_SURFACELOST
    if( FAILED( hr = g_pDisplay->Present() ) )
    {
        return hr;
    }

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
        return DXTRACE_ERR( TEXT("RestoreAllSurfaces"), hr );

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = g_pTextSurface->DrawText( NULL, HELPTEXT, 
                                               0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return DXTRACE_ERR( TEXT("DrawText"), hr );

    // We need to release and re-load, and set the palette again to 
    // redraw the bitmap on the surface.  Otherwise, GDI will not 
    // draw the bitmap on the surface with the right palette
    SAFE_RELEASE( g_pDDPal );

    if( FAILED( hr = g_pDisplay->CreatePaletteFromBitmap( &g_pDDPal, MAKEINTRESOURCE( IDB_DIRECTX ) ) ) )
        return DXTRACE_ERR( TEXT("CreatePaletteFromBitmap"), hr );

    if( FAILED( hr = g_pDisplay->SetPalette( g_pDDPal ) ) )
        return DXTRACE_ERR( TEXT("SetPalette"), hr );

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = g_pLogoSurface->DrawBitmap( MAKEINTRESOURCE( IDB_DIRECTX ),
                                                 SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return DXTRACE_ERR( TEXT("DrawBitmap"), hr );

    return S_OK;
}


