//-----------------------------------------------------------------------------
// File: FullScreenDialog.cpp
//
// Desc: This sample demonstrates how to bring up a dialog box, or any other 
//       type of window while running in DirectDraw full-screen exclusive mode.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
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

#define SPRITE_DIAMETER 48
#define NUM_SPRITES     25

#define HELPTEXT TEXT("Press Escape to quit.  Press F1 to bring up the dialog.")

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
BOOL                 g_bActive     = FALSE; 
DWORD                g_dwLastTick;
SPRITE_STRUCT        g_Sprite[NUM_SPRITES]; 
HWND                 g_hWndDlg     = NULL;




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK SampleDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
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
    MSG	   msg;
    HWND   hWnd;
    HACCEL hAccel;

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

    // Show the dialog immediately
    g_hWndDlg = CreateDialog( hInst, MAKEINTRESOURCE(IDD_DIALOG_SAMPLE),
                              hWnd, (DLGPROC) SampleDlgProc );
    ShowWindow( g_hWndDlg, SW_SHOWNORMAL );

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

            // Translate and dispatch the message. If the dialog is showing, 
            // translate messages for it since it's a modeless dialog.
            if( g_hWndDlg == NULL || !IsDialogMessage( g_hWndDlg, &msg ) )
            {
                if( 0 == TranslateAccelerator( hWnd, hAccel, &msg ) )
                {
                    TranslateMessage( &msg ); 
                    DispatchMessage( &msg );
                }
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
    wc.lpszClassName = TEXT("FullScreenDialog");
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
    hWnd = CreateWindowEx( 0, TEXT("FullScreenDialog"), TEXT("DirectDraw FullScreenDialog Sample"),
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
        return hr;
    }

    // Check if the device supports DDCAPS2_CANRENDERWINDOWED.  
    // If it does, then it supports GDI writing directly to the primary surface
    // Otherwise, to do GDI displaying on these cards, you you must create a 
    // bitmap of the GDI window and BitBlt the bitmap to the backbuffer 
    // then flip as normal. However, this sample does not show this. 
    DDCAPS ddcaps;
    ZeroMemory( &ddcaps, sizeof(ddcaps) );
    ddcaps.dwSize = sizeof(ddcaps);
    g_pDisplay->GetDirectDraw()->GetCaps( &ddcaps, NULL );
    if( (ddcaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) == 0 )
    {
        MessageBox( hWnd, TEXT("This display card can not render GDI."),
                    TEXT("DirectDraw Sample"), MB_ICONERROR | MB_OK );
        return E_FAIL;
    }

    // Create a clipper so DirectDraw will not blt over the GDI dialog
    if( FAILED( hr = g_pDisplay->InitClipper() ) )
        return hr;

    // Create a surface, and draw text to it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromText( &g_pTextSurface, NULL, HELPTEXT, 
                                                        RGB(0,0,0), RGB(255, 255, 0) ) ) )
        return hr;

    // Create a surface, and draw a bitmap resource on it.  
    if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pLogoSurface, MAKEINTRESOURCE( IDB_DIRECTX ), 
                                                          SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return hr;

    // Set the color key for the logo sprite to black
    if( FAILED( hr = g_pLogoSurface->SetColorKey( 0 ) ) )
        return hr;

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

                case IDM_SHOW_DIALOG:
                    {
                        HINSTANCE hInst;
#ifdef _WIN64
                        hInst = (HINSTANCE) GetWindowLongPtr( hWnd, GWLP_HINSTANCE );
#else
                        hInst = (HINSTANCE) GetWindowLong( hWnd, GWL_HINSTANCE );
#endif
                        g_hWndDlg = CreateDialog( hInst, MAKEINTRESOURCE(IDD_DIALOG_SAMPLE),
                                                 hWnd, (DLGPROC) SampleDlgProc );
                        ShowWindow( g_hWndDlg, SW_SHOWNORMAL );
                    }
                    break;
            }
            break; // Continue with default processing

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
// Name: SampleDlgProc()
// Desc: A simple dialog that has some standard controls, so you can see that
//       they update properly.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK SampleDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    static TCHAR* pszCombo[] = {"One","Two","Three","Four","Five","Six"};

    switch (msg)
    {
        case WM_INITDIALOG:
            {
                for( int i = 0; i < 6; i++ )
                    SendDlgItemMessage( hDlg, IDC_COMBO1, CB_ADDSTRING, 
                                        0, (LPARAM) pszCombo[i] );

                SendDlgItemMessage( hDlg, IDC_COMBO1, CB_SETCURSEL, 0, 0 );
                CheckDlgButton( hDlg, IDC_RADIO1, BST_CHECKED );
            }
            return TRUE;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                case IDOK:
                    EndDialog( hDlg, TRUE );
                    return TRUE;
            }
            break;

        case WM_MOVE:
            // The window is moving around, so re-draw the backbuffer
            DisplayFrame();
            break;

        case WM_DESTROY:
            g_hWndDlg = NULL;
            break;
    }

    return FALSE;
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
    // ignoring errors until the last call. Note that all of these sprites 
    // use the same DirectDraw surface.
    for( int iSprite = 0; iSprite < NUM_SPRITES; iSprite++ )
    {
        g_pDisplay->Blt( (DWORD)g_Sprite[iSprite].fPosX, 
                         (DWORD)g_Sprite[iSprite].fPosY, 
                         g_pLogoSurface, NULL );
    }

    // Updating the primary buffer with a blt
    LPDIRECTDRAWSURFACE7 pddsFrontBuffer = g_pDisplay->GetFrontBuffer();
    LPDIRECTDRAWSURFACE7 pddsBackBuffer = g_pDisplay->GetBackBuffer();

    if( FAILED( hr = pddsFrontBuffer->Blt( NULL, pddsBackBuffer, NULL, 
                                           DDBLT_WAIT, NULL ) ) )
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
    if( FAILED( hr = g_pLogoSurface->DrawBitmap( MAKEINTRESOURCE( IDB_DIRECTX ),
                                                 SPRITE_DIAMETER, SPRITE_DIAMETER ) ) )
        return hr;

    return S_OK;
}



