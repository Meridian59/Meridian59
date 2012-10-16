//-----------------------------------------------------------------------------
// File: SpriteAnimate.cpp
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

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      16

#define SPRITE_DIAMETER     48
#define NUM_SPRITES         25
#define NUM_FRAMES          30
#define NUM_RAND            100

struct SPRITE_STRUCT
{
    FLOAT fRotationSpeed;
    FLOAT fRotationTick;
    LONG  lFrame;
    BOOL  bClockwise;
    FLOAT fPosX; 
    FLOAT fPosY;
    FLOAT fVelX; 
    FLOAT fVelY;
};

CDisplay*            g_pDisplay          = NULL;
CSurface*            g_pAnimationSurface = NULL;  
RECT                 g_rcViewport;          
RECT                 g_rcScreen;            
BOOL                 g_bActive     = FALSE; 
DWORD                g_dwLastTick;
SPRITE_STRUCT        g_Sprite[NUM_SPRITES]; 
RECT                 g_rcFrame[NUM_FRAMES];
LONG                 g_lRandTable[NUM_RAND];
DWORD                g_dwRandIndex;




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd );
HRESULT InitDirectDraw( HWND hWnd );
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

    ZeroMemory( &g_Sprite, sizeof(SPRITE_STRUCT) * NUM_SPRITES );
    srand( GetTickCount() );

    if( FAILED( WinInit( hInst, nCmdShow, &hWnd ) ) )
        return FALSE;

    // Make a timer go off to re-init the table of random values every once in a while
	SetTimer( hWnd, 0, 1500, NULL );

    if( FAILED( InitDirectDraw( hWnd ) ) )
    {
        if( g_pDisplay )
            g_pDisplay->GetDirectDraw()->SetCooperativeLevel( NULL, DDSCL_NORMAL );

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
            TranslateMessage( &msg ); 
            DispatchMessage( &msg );
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
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd )
{
    WNDCLASS wc;
    HWND     hWnd;

    // Register the Window Class
    wc.lpszClassName = TEXT("SpriteAnimate");
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

    // Create and show the main window
    hWnd = CreateWindowEx( 0, TEXT("SpriteAnimate"), TEXT("DirectDraw SpriteAnimate Sample"),
                           WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
  	                       CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL );
    if( hWnd == NULL )
    	return E_FAIL;

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    *phWnd = hWnd;

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
        MessageBox( hWnd, TEXT("This display card does not support 640x480x16. "),
                    TEXT("DirectDraw Sample"), MB_ICONERROR | MB_OK );
        return hr;
    }

    // Create a surface, and draw a bitmap resource on it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pAnimationSurface, 
                                                          MAKEINTRESOURCE( IDB_ANIMATE_SHEET ), 
                                                          SPRITE_DIAMETER * 5, 
                                                          SPRITE_DIAMETER * 6 ) ) )        
        return hr;

    // Set the color key for the logo sprite to black
    if( FAILED( hr = g_pAnimationSurface->SetColorKey( 0 ) ) )
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

        g_Sprite[iSprite].lFrame          = rand() % NUM_FRAMES;
        g_Sprite[iSprite].fRotationTick   = 0.0f;
        g_Sprite[iSprite].fRotationSpeed  = (rand() % 50 + 25 ) / 1000.0f; 
        g_Sprite[iSprite].bClockwise      = rand() % 2;
    }

    // Precompute the source rects for g_pDDSAnimateSheet.  The source rects 
    // are used during the blt of the dds to the backbuffer. 
    for( int iFrame = 0; iFrame < NUM_FRAMES; iFrame++ )
    {
        g_rcFrame[iFrame].top    = (iFrame / 5) * SPRITE_DIAMETER;
        g_rcFrame[iFrame].left   = (iFrame % 5) * SPRITE_DIAMETER;

        g_rcFrame[iFrame].bottom = g_rcFrame[iFrame].top  + SPRITE_DIAMETER;
        g_rcFrame[iFrame].right  = g_rcFrame[iFrame].left + SPRITE_DIAMETER;
    }

    // Init a array of random values.  This array is used to create the
    // 'flocking' effect the seen in the sample.
    g_dwRandIndex = 0;
    for( int iRand = 0; iRand < NUM_RAND; iRand++ )
        g_lRandTable[ iRand ] = rand();    

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectDraw()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID FreeDirectDraw()
{
    SAFE_DELETE( g_pAnimationSurface );
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
        case WM_KEYDOWN:
            PostMessage( hWnd, WM_CLOSE, 0, 0 );
            return 0L;

        case WM_TIMER:
            {
                // The timer triggered, so re-init table of random values
                g_dwRandIndex = 0;
                for( int iRand = 0; iRand < NUM_RAND; iRand++ )
                    g_lRandTable[ iRand ] = rand();    
            }
            break;

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
// Name: GetNextRand()
// Desc: Gets the next element from the circular array of random values
//-----------------------------------------------------------------------------
LONG GetNextRand()
{    
    LONG lRand = g_lRandTable[ g_dwRandIndex++ ];
    g_dwRandIndex %= NUM_RAND;

    return lRand;
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

    // See if its time to advance the sprite to the next frame
    pSprite->fRotationTick += fTimeDelta;
    if( pSprite->fRotationTick > pSprite->fRotationSpeed )
    {
        // If it is, then either change the frame clockwise or counter-clockwise
        if( pSprite->bClockwise )
        {
            pSprite->lFrame++;
            pSprite->lFrame %= NUM_FRAMES; 
        }
        else
        {
            pSprite->lFrame--;
        	if( pSprite->lFrame < 0 )
        		pSprite->lFrame = NUM_FRAMES - 1;
        }

        pSprite->fRotationTick = 0;        
    }

    // Using the next element from the random arry, 
    // randomize the velocity of the sprite
    if( GetNextRand() % 100 < 2 )
    {
        pSprite->fVelX = 500.0f * GetNextRand() / RAND_MAX - 250.0f;
        pSprite->fVelY = 500.0f * GetNextRand() / RAND_MAX - 250.0f;
    }

    // Using the next element from the random arry, 
    // randomize the rotational speed of the sprite
    if( GetNextRand() % 100 < 5 )
        pSprite->fRotationSpeed = ( GetNextRand() % 50 + 5 ) / 1000.0f;

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

    // Blt all the sprites onto the back buffer using color keying,
    // ignoring errors until the flip. Note that all of these sprites 
    // use the same DirectDraw surface.
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
    {
        g_pDisplay->Blt( (DWORD)g_Sprite[iSprite].fPosX, 
                         (DWORD)g_Sprite[iSprite].fPosY, 
                         g_pAnimationSurface, 
                         &g_rcFrame[ g_Sprite[iSprite].lFrame ] );
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
    if( FAILED( hr = g_pAnimationSurface->DrawBitmap( MAKEINTRESOURCE( IDB_ANIMATE_SHEET ), 
                                                      SPRITE_DIAMETER * 5, 
                                                      SPRITE_DIAMETER * 6 ) ) )        
        return hr;
                                                          

    return S_OK;
}




