//-----------------------------------------------------------------------------
// File: Scrawl.cpp
//
// Desc: Demonstrates an application which receives relative mouse data
//       in non-exclusive mode via a dialog timer.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <basetsd.h>
#include <dinput.h>
#include "resource.h"




//-----------------------------------------------------------------------------
// Function prototypes 
//-----------------------------------------------------------------------------
HRESULT InitVariables();
HWND    RegisterWindowClass( HINSTANCE hInst );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

HRESULT OnClear( HWND hWnd );
VOID    InvalidateCursorRect(HWND hWnd);
VOID    OnPaint( HWND hWnd );
BOOL    OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct );
VOID    OnInitMenuPopup( HWND hWnd, HMENU hMenu, UINT item, BOOL fSystemMenu );
VOID    OnKeyDown( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags );
HRESULT InitDirectInput( HWND hWnd );
HRESULT SetAcquire();
HRESULT FreeDirectInput();
VOID    OnMouseInput( HWND hWnd );
VOID    OnLeftButtonDown( HWND hWnd );
VOID    OnRightButtonUp( HWND hWnd );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define SCRAWL_CXBITMAP             512
#define SCRAWL_CYBITMAP             300
#define SAMPLE_BUFFER_SIZE           16
#define IDC_CLEAR               64
#define IDC_ABOUT               65

struct LEFTBUTTONINFO 
{
    HDC   hdcWindow;
    BOOL  bMoved;
    DWORD dwSeqLastSeen;
};


HDC     g_hDC           = NULL; // Memory DC our picture lives in 
HBITMAP g_hBitmap       = NULL; // Our picture 
HBITMAP g_hbmpDeselect  = NULL; // Stock bitmap for deselecting 
HCURSOR g_hCursorCross  = NULL; // cross hair
int     g_cxCross;              // Width of crosshairs cursor 
int     g_cyCross;              // Height of crosshairs cursor 
int     g_dxCrossHot;           // Hotspot location of crosshairs 
int     g_dyCrossHot;           // Hotspot location of crosshairs 
BOOL    g_bShowCursor   = TRUE; // Should the cursor be shown? 
int     g_x;                    // Virtual x-coordinate 
int     g_y;                    // Virtual y-coordinate 
int     g_dxFuzz;               // Leftover x-fuzz from scaling 
int     g_dyFuzz;               // Leftover y-fuzz from scaling 
int     g_iSensitivity;         // Mouse sensitivity 

LPDIRECTINPUT8          g_pDI           = NULL;         
LPDIRECTINPUTDEVICE8    g_pMouse        = NULL;     
HANDLE                  g_hMouseEvent   = NULL;
BOOL                    g_bActive       = TRUE;
BOOL                    g_bSwapMouseButtons;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int )
{
    HRESULT hr;
    HWND    hWnd;
    BOOL    bDone;
    DWORD   dwResult;
    MSG     msg;

    // Initialize global varibles
    if ( FAILED( hr = InitVariables() ) )
    {
        MessageBox( NULL, _T("Error Initializing Variables"), 
                    _T("Scrawl"),  MB_ICONERROR | MB_OK );
        return TRUE;
    }

    // Display the main dialog box.
    hWnd = RegisterWindowClass( hInstance );
    if( NULL == hWnd )
    {
        MessageBox( NULL, _T("Error Creating Window"), 
                    _T("Scrawl"), MB_ICONERROR | MB_OK );
        return TRUE;
    }

    // Start message pump. Since we use notification handles, we need to use
    // MsgWaitForMultipleObjects() to wait for the event or a message, 
    // whichever comes first.

    bDone = FALSE;
    while( !bDone ) 
    {
        dwResult = MsgWaitForMultipleObjects( 1, &g_hMouseEvent, 
                                              FALSE, INFINITE, QS_ALLINPUT );

        switch( dwResult ) 
        {
            // WAIT_OBJECT_0 + 0 means that g_hevtMouse was signalled 
            case WAIT_OBJECT_0 + 0:
                OnMouseInput( hWnd );
                break;

            // WAIT_OBJECT_0 + 1 means that we have messages to process 
            case WAIT_OBJECT_0 + 1:
                while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
                {
                    if( msg.message == WM_QUIT ) 
                    {
                        // Stop loop if it's a quit message
                        bDone = TRUE;
                    } 
                    else 
                    {
                        TranslateMessage( &msg );
                        DispatchMessage( &msg );
                    }
                }
                break;
        }
    }

    // Clean up
    FreeDirectInput();

    // Delete bitmaps
    if( g_hDC ) 
    {
        if( g_hbmpDeselect ) 
            SelectObject( g_hDC, g_hbmpDeselect );

        DeleteDC( g_hDC );
    }

    if( g_hBitmap ) 
        DeleteObject( g_hBitmap );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitVariables()
// Desc: Initialize global varibles 
//-----------------------------------------------------------------------------
HRESULT InitVariables()
{
    ICONINFO iconInfo;
    BITMAP   bitmap;
    HDC      hDC;

    // Get our crosshairs cursor and extract the the width and
    // hotspot location so we can draw it manually.
    g_hCursorCross = LoadCursor( NULL, IDC_CROSS );

    GetIconInfo( g_hCursorCross, &iconInfo );
    GetObject( iconInfo.hbmMask, sizeof(BITMAP), &bitmap );

    // Delete un-needed handles
    if( iconInfo.hbmMask)  
        DeleteObject( iconInfo.hbmMask );
    if( iconInfo.hbmColor) 
        DeleteObject( iconInfo.hbmColor );

    // Save x-y info 
    g_dxCrossHot = iconInfo.xHotspot;
    g_dyCrossHot = iconInfo.yHotspot;

    g_cxCross = bitmap.bmWidth;
    g_cyCross = bitmap.bmHeight;

    // create and setup our scrawl bitmap.
    hDC = GetDC( NULL );
    g_hDC = CreateCompatibleDC( hDC );  
    ReleaseDC( NULL, hDC );
    if( NULL == g_hDC ) 
        return E_FAIL;

    g_hBitmap = CreateBitmap( SCRAWL_CXBITMAP, SCRAWL_CYBITMAP, 1, 1, 0 );
    if( NULL == g_hBitmap ) 
        return E_FAIL;

    g_hbmpDeselect = (HBITMAP)SelectObject( g_hDC, g_hBitmap );

    // Clear bitmap
    OnClear( NULL );  

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RegisterWindowClass()
// Desc: Set up the window class. 
//-----------------------------------------------------------------------------
HWND RegisterWindowClass( HINSTANCE hInst )
{
    WNDCLASS wc;
    wc.hCursor        = LoadCursor( 0, IDC_ARROW );
    wc.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN) );
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = _T("Scrawl");
    wc.hbrBackground  = NULL;
    wc.hInstance      = hInst;
    wc.style          = 0;
    wc.lpfnWndProc    = WndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    if( FALSE == RegisterClass(&wc) ) 
        return NULL;

    RECT rc;
    rc.left     = 0;
    rc.top      = 0;
    rc.right    = SCRAWL_CXBITMAP;
    rc.bottom   = SCRAWL_CYBITMAP;
    AdjustWindowRectEx( &rc, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, 
                        FALSE, WS_EX_APPWINDOW );

    HWND hWnd = CreateWindowEx( WS_EX_APPWINDOW, _T("Scrawl"), _T("Scrawl"),  
                                WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                rc.right - rc.left, 
                                rc.bottom - rc.top, 
                                NULL, NULL, hInst, NULL );

    ShowWindow( hWnd, TRUE );

    return hWnd;
}




//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Handles window messages
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, 
                          LPARAM lParam )
{
    LRESULT lr = 0;

    switch( msg ) 
    {
        // Pass these messages to user defined functions
        HANDLE_MSG( hWnd, WM_CREATE,        OnCreate );
        HANDLE_MSG( hWnd, WM_PAINT,         OnPaint );
        HANDLE_MSG( hWnd, WM_INITMENUPOPUP, OnInitMenuPopup );
        HANDLE_MSG( hWnd, WM_KEYDOWN,       OnKeyDown );

        case WM_ACTIVATE:   // sent when window changes active state
            if( WA_INACTIVE == wParam )
                g_bActive = FALSE;
            else
                g_bActive = TRUE;

            // Set exclusive mode access to the mouse based on active state
            SetAcquire();
            return 0;

		case WM_NCLBUTTONDOWN:
			switch (wParam)
			{
				case HTMINBUTTON:
					ShowWindow( hWnd, SW_MINIMIZE);
					break;

				case HTCLOSE:
					PostQuitMessage(0);
					break;
			}

        case WM_ENTERMENULOOP:
        case WM_ENTERSIZEMOVE:
            // un-acquire device when entering menu or re-sizing
            // this will show the mouse cursor again
            g_bActive = FALSE;
            SetAcquire();
            return 0;

        case WM_EXITMENULOOP:
            // If we aren't returning from the popup menu, let the user continue
            // to be in non-exclusive mode (to move the window for example)
            if( (BOOL)wParam == FALSE )
                return 0;

        case WM_EXITSIZEMOVE:
            // re-acquire device when leaving menu or re-sizing
            // this will show the mouse cursor again

            // even though the menu is going away, the app
            // might have lost focus or be an icon
            if( GetActiveWindow() == hWnd || !IsIconic( hWnd ) )
                g_bActive = TRUE;
            else
                g_bActive = FALSE;

            SetAcquire();
            return 0;
        
        case WM_SYSCOMMAND:
            lr = 0;
            switch ( LOWORD(wParam) ) 
            {
                case IDC_CLEAR:
                    OnClear( hWnd );
                    break;
            
                case IDC_ABOUT:
                    MessageBox( hWnd, _T("Scrawl DirectInput Sample v1.0"),
                                _T("Scrawl"), MB_OK );
                    break;
            
                case SC_SCREENSAVE:
                    // eat the screen-saver notification.
                    break;

                case IDC_SENSITIVITY_LOW:
                    g_iSensitivity = -1;
                    break;

                case IDC_SENSITIVITY_NORMAL:
                    g_iSensitivity = 0;
                    break;

                case IDC_SENSITIVITY_HIGH:
                    g_iSensitivity = 1;
                    break;
            
                default:
                    lr = DefWindowProc( hWnd, msg, wParam, lParam );
                    break;
            }
        
            // The WM_SYSCOMMAND might've been a WM_CLOSE, 
            // in which case our window no longer exists.  
            if( IsWindow(hWnd) ) 
                SetAcquire();
            return lr;


       case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: OnCreate()
// Desc: Handles the WM_CREATE window message
//-----------------------------------------------------------------------------
BOOL OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
    HRESULT hr;
    HMENU   hMenu;

    // Initialize direct input
    hr = InitDirectInput( hWnd );
    if( FAILED(hr) )
    {
        MessageBox( NULL, _T("Error Initializing DirectInput"), 
                    _T("Scrawl"), MB_ICONERROR | MB_OK );
        return FALSE;
    }

    // Fix up the popup system menu with custom commands
    hMenu = GetSystemMenu( hWnd, FALSE );

    EnableMenuItem( hMenu, SC_SIZE,     MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
    EnableMenuItem( hMenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
    AppendMenu( hMenu, MF_ENABLED | MF_STRING, IDC_CLEAR, _T("C&lear\tDel") );
    AppendMenu( hMenu, MF_ENABLED | MF_STRING, IDC_ABOUT, _T("&About\tF1") );

    AppendMenu( hMenu, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)LoadMenu( (HINSTANCE)GetModuleHandle(NULL), 
                                    MAKEINTRESOURCE(IDM_SENSITIVITY) ),
                _T("Sensitivit&y") );

    return TRUE;    
}




//-----------------------------------------------------------------------------
// Name: OnPaint()
// Desc: Handles the WM_PAINT window message
//-----------------------------------------------------------------------------
VOID OnPaint( HWND hWnd )
{
    PAINTSTRUCT ps;
    HDC         hDC;
    
    hDC = BeginPaint( hWnd, &ps );
    if( NULL == hDC ) 
        return;

    BitBlt( hDC, ps.rcPaint.left, ps.rcPaint.top,
            ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
            g_hDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY );

    if( g_bActive && g_bShowCursor ) 
        DrawIcon( hDC, g_x - g_dxCrossHot, g_y - g_dyCrossHot, g_hCursorCross );

    EndPaint( hWnd, &ps );
}




//-----------------------------------------------------------------------------
// Name: OnInitMenuPopup()
// Desc: Handles the WM_INITMENUPOPUP window message
//-----------------------------------------------------------------------------
VOID OnInitMenuPopup( HWND hWnd, HMENU hMenu, UINT item, BOOL fSystemMenu )
{
    for( int iSensitivity = -1; iSensitivity <= 1; iSensitivity++ ) 
    {
        if( g_iSensitivity == iSensitivity ) 
        {
            CheckMenuItem( hMenu, IDC_SENSITIVITY_NORMAL + iSensitivity,
                           MF_BYCOMMAND | MF_CHECKED );
        } 
        else 
        {
            CheckMenuItem( hMenu, IDC_SENSITIVITY_NORMAL + iSensitivity,
                           MF_BYCOMMAND | MF_UNCHECKED );
        }
    }
}




//-----------------------------------------------------------------------------
// Name: OnKeyDown()
// Desc: Handles the WM_KEYDOWN window message
//-----------------------------------------------------------------------------
VOID OnKeyDown( HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags )
{
    switch( vk ) 
    {
        case '1':
        case '2':
        case '3':
            PostMessage( hWnd, WM_SYSCOMMAND, IDC_SENSITIVITY_NORMAL + vk - '2', 0 );
            break;

        case VK_DELETE:
            PostMessage( hWnd, WM_SYSCOMMAND, IDC_CLEAR, 0 );
            break;

        case VK_F1:
            PostMessage( hWnd, WM_SYSCOMMAND, IDC_ABOUT, 0 );
            break;
    }
}




//-----------------------------------------------------------------------------
// Name: OnClear()
// Desc: Makes the bitmap white
//-----------------------------------------------------------------------------
HRESULT OnClear( HWND hWnd )
{
    PatBlt( g_hDC, 0, 0, SCRAWL_CXBITMAP, SCRAWL_CYBITMAP, WHITENESS );

    if( hWnd ) 
        InvalidateRect( hWnd, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateCursorRect()
// Desc: Invalidate the rectangle that contains the cursor.
//       The coordinates are in client coordinates.
//-----------------------------------------------------------------------------
VOID InvalidateCursorRect( HWND hWnd )
{
    RECT rc = { g_x - g_dxCrossHot,             g_y - g_dyCrossHot,
                g_x - g_dxCrossHot + g_cxCross, g_y - g_dyCrossHot + g_cyCross };
    InvalidateRect( hWnd, &rc, 0 );
}




//-----------------------------------------------------------------------------
// Name: UpdateCursorPosition()
// Desc: Move our private cursor in the requested direction, subject
//       to clipping, scaling, and all that other stuff.
//
//       This does not redraw the cursor.  You need to do that yourself.
//-----------------------------------------------------------------------------
VOID UpdateCursorPosition( int dx, int dy )
{   
    // Pick up any leftover fuzz from last time.  This is important
    // when scaling down mouse motions.  Otherwise, the user can
    // drag to the right extremely slow for the length of the table
    // and not get anywhere.
    dx += g_dxFuzz;     
    g_dxFuzz = 0;

    dy += g_dyFuzz;     
    g_dyFuzz = 0;

    switch( g_iSensitivity ) 
    {
        case 1:     // High sensitivity: Magnify! 
            dx *= 2;
            dy *= 2;
            break;

        case -1:    // Low sensitivity: Scale down 
            g_dxFuzz = dx % 2;  // remember the fuzz for next time 
            g_dyFuzz = dy % 2;
            dx /= 2;
            dy /= 2;
            break;

        case 0:     // normal sensitivity 
            // No adjustments needed 
            break;
    }

    g_x += dx;
    g_y += dy;

    // clip the cursor to our client area
    if( g_x < 0 ) 
        g_x = 0;

    if( g_x >= SCRAWL_CXBITMAP )
        g_x = SCRAWL_CXBITMAP - 1;

    if( g_y < 0 )  
        g_y = 0;

    if( g_y >= SCRAWL_CYBITMAP ) 
        g_y = SCRAWL_CYBITMAP - 1;
}




//-----------------------------------------------------------------------------
// Name: StartPenDraw()
// Desc: Called when starting pen draw.
//-----------------------------------------------------------------------------
VOID StartPenDraw( HWND hWnd, LEFTBUTTONINFO* plbInfo )
{
    // Hide the cursor while scrawling 
    g_bShowCursor = FALSE;

    plbInfo->hdcWindow = GetDC( hWnd );
    MoveToEx( plbInfo->hdcWindow, g_x, g_y, 0 );
    MoveToEx( g_hDC, g_x, g_y, 0 );

    SelectObject( plbInfo->hdcWindow, GetStockObject(BLACK_PEN) );
    SelectObject( g_hDC, GetStockObject(BLACK_PEN) );

    plbInfo->bMoved = FALSE;
    plbInfo->dwSeqLastSeen = 0;
}




//-----------------------------------------------------------------------------
// Name: FinishPenDraw()
// Desc: Called when ending pen draw.
//-----------------------------------------------------------------------------
VOID FinishPenDraw( HANDLE hWnd )
{
    g_bShowCursor = TRUE;
}




//-----------------------------------------------------------------------------
// Name: OnLeftButtonDown_FlushMotion()
// Desc: Flush out any motion that we are holding.
//-----------------------------------------------------------------------------
VOID OnLeftButtonDown_FlushMotion( LEFTBUTTONINFO* plbInfo )
{
    if( plbInfo->bMoved ) 
    {
        plbInfo->bMoved = FALSE;
        plbInfo->dwSeqLastSeen = 0;
        LineTo( plbInfo->hdcWindow, g_x, g_y );
        LineTo( g_hDC, g_x, g_y );
    }
}




//-----------------------------------------------------------------------------
// Name: OnRightButtonUp()
// Desc: Pop up a context menu.
//-----------------------------------------------------------------------------
VOID OnRightButtonUp( HWND hWnd )
{
    // Place a popup menu where the mouse curent is
    POINT pt;
    pt.x = g_x;
    pt.y = g_y;
    ClientToScreen( hWnd, &pt );
    HMENU hMenuPopup = GetSystemMenu( hWnd, FALSE );

    // Hide the cursor while moving it so you don't get annoying flicker.
    ShowCursor( FALSE );
    InvalidateCursorRect( hWnd );

    // Unacquire the devices so the user can interact with the menu.
    g_bActive = FALSE;
    SetAcquire();

    // Put the Windows cursor at the same location as our virtual cursor.
    SetCursorPos( pt.x, pt.y );

    // Show the cursor now that it is moved 
    ShowCursor( TRUE );
    InvalidateCursorRect( hWnd );

    // Track the popup menu and return the menu item selected
    UINT iMenuID = TrackPopupMenuEx( hMenuPopup, TPM_RIGHTBUTTON|TPM_RETURNCMD,
                                     pt.x, pt.y, hWnd, 0 );

    if( 0 != iMenuID ) // If a menu item was selected
        PostMessage( hWnd, WM_SYSCOMMAND, iMenuID, 0L );
}




//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT InitDirectInput( HWND hWnd )
{
    HRESULT hr;

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return hr;
    
    // Obtain an interface to the system mouse device.
    if( FAILED( hr = g_pDI->CreateDevice( GUID_SysMouse, &g_pMouse, NULL ) ) )
        return hr;

    // Set the data format to "mouse format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing a
    // DIMOUSESTATE structure to IDirectInputDevice::GetDeviceState.
    if( FAILED( hr = g_pMouse->SetDataFormat( &c_dfDIMouse ) ) )
        return hr;

    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    if( FAILED( hr = g_pMouse->SetCooperativeLevel( hWnd, 
                                         DISCL_EXCLUSIVE|DISCL_FOREGROUND ) ) )
        return hr;

    // Create a win32 event which is signaled when mouse data is availible
    g_hMouseEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( NULL == g_hMouseEvent )
        return E_FAIL;

    // Give the event to the mouse device
    if( FAILED( hr = g_pMouse->SetEventNotification( g_hMouseEvent ) ) )
        return hr;

    // Setup the buffer size for the mouse data
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

    if( FAILED( hr = g_pMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
        return hr;

    // Not necessary, but nice for left handed users that have
    // their swapped mouse buttons
    g_bSwapMouseButtons = GetSystemMetrics( SM_SWAPBUTTON );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetAcquire()
// Desc: Acquire or unacquire the mouse, depending on if the app is active
//       Input device must be acquired before the GetDeviceState is called
//-----------------------------------------------------------------------------
HRESULT SetAcquire()
{
    // Nothing to do if g_pMouse is NULL
    if( NULL == g_pMouse )
        return S_FALSE;

    if( g_bActive ) 
        g_pMouse->Acquire();
    else 
        g_pMouse->Unacquire();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT FreeDirectInput()
{
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pMouse ) 
        g_pMouse->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pMouse );
    SAFE_RELEASE( g_pDI );

    if( g_hMouseEvent )
        CloseHandle( g_hMouseEvent );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnMouseInput()
// Desc: Handles responding to any mouse input that is generated from
//       the mouse event being triggered.
//-----------------------------------------------------------------------------
VOID OnMouseInput( HWND hWnd )
{
    BOOL                bDone;
    DIDEVICEOBJECTDATA  od;
    DWORD               dwElements;
    HRESULT             hr;

    // Invalidate the old cursor so it will be erased 
    InvalidateCursorRect( hWnd );

    // Attempt to read one data element.  Continue as long as
    // device data is available.
    bDone = FALSE;
    while( !bDone ) 
    {
        dwElements = 1;
        hr = g_pMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), 
                                      &od, &dwElements, 0 );

        if( hr == DIERR_INPUTLOST ) 
        {
            SetAcquire();
            break;
        }

        // Unable to read data or no data available
        if( FAILED(hr) || dwElements == 0 ) 
        {
            break;
        }

        // Look at the element to see what happened
        switch( od.dwOfs ) 
        {     
            case DIMOFS_X:       // Mouse horizontal motion 
                UpdateCursorPosition( od.dwData, 0 ); 
                break;

            case DIMOFS_Y:       // Mouse vertical motion 
                UpdateCursorPosition( 0, od.dwData ); 
                break;

            case DIMOFS_BUTTON0: // Right button pressed or released 
            case DIMOFS_BUTTON1: // Left button pressed or released 
                // Is the right or a swapped left button down?
                if( ( g_bSwapMouseButtons  && DIMOFS_BUTTON1 == od.dwOfs ) ||
                    ( !g_bSwapMouseButtons && DIMOFS_BUTTON0 == od.dwOfs ) )
                {
                    if( od.dwData & 0x80 ) 
                    { 
                        // left button pressed, so go into button-down mode 
                        bDone = TRUE;
                        OnLeftButtonDown( hWnd ); 
                    }
                }

                // is the left or a swapped right button down?
                if( ( g_bSwapMouseButtons  && DIMOFS_BUTTON0 == od.dwOfs ) ||
                    ( !g_bSwapMouseButtons && DIMOFS_BUTTON1 == od.dwOfs ) )
                {
                    if( !(od.dwData & 0x80) ) 
                    {  
                        // button released, so check context menu 
                        bDone = TRUE;
                        OnRightButtonUp( hWnd ); 
                    }
                }
                break;
        }
    }

    // Invalidate the new cursor so it will be drawn 
    InvalidateCursorRect( hWnd );
}




//-----------------------------------------------------------------------------
// Name: OnLeftButtonDown()
// Desc: If we are drawing a curve, then read buffered data and draw
//      lines from point to point.  By reading buffered data, we can
//      track the motion of the mouse accurately without coalescing.
//
//      This function illustrates how a non-message-based program can
//      process buffered data directly from a device, processing
//      messages only occasionally (as required by Windows).
//
//      This function also illustrates how an application can piece
//      together buffered data elements based on the sequence number.
//      A single mouse action (e.g., moving diagonally) is reported
//      as a series of events, all with the same sequence number.
//      Zero is never a valid DirectInput sequence number, so it is
//      safe to use it as a sentinel value.
//-----------------------------------------------------------------------------
VOID OnLeftButtonDown( HWND hWnd )
{
    HRESULT             hr;
    LEFTBUTTONINFO      lbInfo;
    BOOL                bDone;
    DIDEVICEOBJECTDATA  od;
    DWORD               dwElements;
    MSG                 msg;

    // For performance, draw directly onto the window's DC instead of
    // invalidating and waiting for the WM_PAINT message.  Of course,
    // we always draw onto our bitmap, too, since that's what really
    // counts.

    // hide cursor and initialize button info with cursor position
    StartPenDraw( hWnd, &lbInfo );
    InvalidateCursorRect( hWnd );
    UpdateWindow( hWnd );

    // Keep reading data elements until we see a "mouse button up" event.
    bDone = FALSE;
    while( !bDone ) 
    {
        dwElements = 1;
        hr = g_pMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), 
                                      &od, &dwElements, 0 );
        if( FAILED(hr) )       
            break;

        // If theres no data available, finish the element 
        // we have been collecting, and then process our message 
        // queue so the system doesn't think the app has hung.
        if( dwElements == 0 ) 
        {
            // if there is a partial motion, flush it out 
            OnLeftButtonDown_FlushMotion( &lbInfo );

            while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
            {
                // If it's a quit message, we're outta here 
                if( msg.message == WM_QUIT ) 
                {
                    // Re-post the quit message so the
                    // outer loop will see it and exit.
                    PostQuitMessage( (int)msg.wParam );
                    bDone = TRUE;                    
                    break;
                } 
                else 
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
            }
            continue;
        }

        // If this is the start of a new event, flush out the old one 
        if( od.dwSequence != lbInfo.dwSeqLastSeen ) 
        {
            OnLeftButtonDown_FlushMotion( &lbInfo );
            lbInfo.dwSeqLastSeen = od.dwSequence;
        }

        // Look at the element to see what happened 
        switch( od.dwOfs ) 
        {
            case DIMOFS_X:      // Mouse horizontal motion 
                UpdateCursorPosition( od.dwData, 0 );
                lbInfo.bMoved = TRUE;
                break;

            case DIMOFS_Y:      // Mouse vertical motion 
                UpdateCursorPosition( 0, od.dwData );
                lbInfo.bMoved = TRUE;
                break;

            case DIMOFS_BUTTON0: // Button 0 pressed or released 
            case DIMOFS_BUTTON1: // Button 1 pressed or released 
                if( ( g_bSwapMouseButtons  && DIMOFS_BUTTON1 == od.dwOfs ) ||
                    ( !g_bSwapMouseButtons && DIMOFS_BUTTON0 == od.dwOfs ) )
                {
                    if( !(od.dwData & 0x80) ) 
                    { 
                        // Button released, so flush out dregs 
                        bDone = TRUE;
                        OnLeftButtonDown_FlushMotion( &lbInfo ); 
                    }
                }
                break;
        }
    }

    ReleaseDC( hWnd, lbInfo.hdcWindow );

    // Re-show the cursor now that scrawling is finished 
    FinishPenDraw( hWnd );
    InvalidateCursorRect( hWnd );
}



