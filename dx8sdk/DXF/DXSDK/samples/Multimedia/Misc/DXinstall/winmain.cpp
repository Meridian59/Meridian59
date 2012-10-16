//-----------------------------------------------------------------------------
// File: WinCode.cpp
//
// Desc: All of the Windows specific code needed for the DSetup sample
//
//      The code in this file includes the main Windows entry point
//      as well as code to handle messages and our modeless version of
//      MessageBox().
//
//      Call Tree:
//         WinMain                      Main Windows Entry Point
//            DirectXInstallInit        Initializes & registers window class
//               DirectXInstallWndProc  Processes windows messages
//                  DirectXInstall      See DINSTALL.CPP
//                  DirectXGetVersion   See DINSTALL.CPP
//         DirectXInstall               See DINSTALL.CPP
//            DlgProc                   Handles all messages for our modeless MessageBox()
//               SetButtons             Initializes the text of the dialog buttons to mimmic MessageBox()
//                  ShowButton          Helper function to get and set the text of a button from resource strings
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <basetsd.h>
#include <stdio.h>
#include <string.h>
#include <dsetup.h>
#include "resource.h"
#include "DXInstall.h"




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
DWORD       g_dwStatus    = SHOW_ALL; // Filter setting for messages from DirectXSetup
HINSTANCE   g_hInstance;              // Global instance handle
HWND        g_hDlg        = NULL;     // Window handle to dialog proc
CHAR        g_strAppTitle[256];       // Application title
INT         g_iReply      = -1;       // Global value for dialog return
BOOL        g_bCheckOlder = FALSE;    // Whether or not to check for older installs




//-----------------------------------------------------------------------------
// Name: ShowButton()
// Desc: Helper function to get and set the text of a button from the
//       resource strings.
//-----------------------------------------------------------------------------
VOID ShowButton( HWND hDlg, int Id, int strid )
{
    HWND btnHwd = GetDlgItem( hDlg, Id );
    CHAR buf[20];

    LoadString( g_hInstance, strid, buf, 20 );
    SetWindowText( btnHwd, buf );
    ShowWindow( btnHwd, SW_NORMAL );
}




//-----------------------------------------------------------------------------
// Name: SetButtons()
// Desc: Initializes the text of the dialog buttons to mimmic MessageBox()
//-----------------------------------------------------------------------------
VOID SetButtons( HWND hDlg, DWORD wMsgType )
{
    LONG dwStyle;

    switch( wMsgType & 0x0000000F )
    {
        case MB_OKCANCEL:
            ShowButton( hDlg, IDBUT1, STR_OK );
            ShowButton( hDlg, IDBUT2, STR_CANCEL );
            ShowWindow( GetDlgItem( hDlg, IDBUT3 ), SW_HIDE );
            break;

        case MB_OK:
            ShowButton( hDlg, IDBUT3, STR_OK );
            break;

        case MB_RETRYCANCEL:
            ShowButton( hDlg, IDBUT1, STR_RETRY );
            ShowButton( hDlg, IDBUT2, STR_CANCEL );
            ShowWindow( GetDlgItem( hDlg, IDBUT3 ), SW_HIDE );
            break;

        case MB_ABORTRETRYIGNORE:
            ShowButton( hDlg, IDBUT1, STR_ABORT );
            ShowButton( hDlg, IDBUT3, STR_RETRY );
            ShowButton( hDlg, IDBUT2, STR_IGNORE );
            break;

        case MB_YESNOCANCEL:
            ShowButton( hDlg, IDBUT1, STR_YES );
            ShowButton( hDlg, IDBUT3, STR_NO );
            ShowButton( hDlg, IDBUT2, STR_CANCEL );
            break;

        case MB_YESNO:
            ShowButton( hDlg, IDBUT1, STR_YES );
            ShowButton( hDlg, IDBUT2, STR_NO );
            ShowWindow( GetDlgItem( hDlg, IDBUT3 ), SW_HIDE );
            break;

        default:
            ShowWindow( GetDlgItem( hDlg, IDBUT1 ), SW_HIDE );
            ShowWindow( GetDlgItem( hDlg, IDBUT2 ), SW_HIDE );
            ShowWindow( GetDlgItem( hDlg, IDBUT3 ), SW_HIDE );
			break;
    }

    if( !(wMsgType & MB_DEFBUTTON2) )
    {
        dwStyle = GetWindowLong( GetDlgItem( hDlg, IDBUT2 ), GWL_STYLE );
        SendMessage( GetDlgItem( hDlg, IDBUT2 ), BM_SETSTYLE, dwStyle & ~BS_DEFPUSHBUTTON, 0 );
    }
    else
    {
        dwStyle = GetWindowLong(GetDlgItem( hDlg, IDBUT2 ), GWL_STYLE);
        SendMessage( GetDlgItem( hDlg, IDBUT2 ), BM_SETSTYLE, dwStyle | BS_DEFPUSHBUTTON, 0 );
    }

    if (!(wMsgType & MB_DEFBUTTON3))
    {
        dwStyle = GetWindowLong( GetDlgItem( hDlg, IDBUT3 ), GWL_STYLE);
        SendMessage( GetDlgItem( hDlg, IDBUT3 ), BM_SETSTYLE, dwStyle & ~BS_DEFPUSHBUTTON, 0 );
    }
    else
    {
        dwStyle = GetWindowLong( GetDlgItem( hDlg, IDBUT3 ), GWL_STYLE);
        SendMessage( GetDlgItem( hDlg, IDBUT3 ), BM_SETSTYLE, dwStyle | BS_DEFPUSHBUTTON, 0 );
    }

    if (!(wMsgType & MB_DEFBUTTON3) && !(wMsgType & MB_DEFBUTTON2))
    {
        dwStyle = GetWindowLong( GetDlgItem( hDlg, IDBUT1 ), GWL_STYLE);
        SendMessage( GetDlgItem( hDlg, IDBUT1 ), BM_SETSTYLE, dwStyle | BS_DEFPUSHBUTTON, 0 );
    }
    else
    {
        dwStyle = GetWindowLong( GetDlgItem( hDlg, IDBUT1 ), GWL_STYLE );
        SendMessage( GetDlgItem( hDlg, IDBUT1 ), BM_SETSTYLE, dwStyle & ~BS_DEFPUSHBUTTON, 0 );
    }
}




//-----------------------------------------------------------------------------
// Name: DlgProc()
// Desc: Message proc for our modeless version of MessageBox()
//       This function sets g_wReply for GetReply()
//-----------------------------------------------------------------------------
DLGPROC DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_INITDIALOG:
            SetButtons( hDlg, -1 );
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDBUT1:
                case IDBUT2:
                case IDBUT3:
                    // Let GetReply() know the user clicked on a button
                    g_iReply = LOWORD(wParam);
                    break;
            }
            break;
        case WM_ACTIVATE:
            if( LOWORD(wParam) == WA_INACTIVE )
            {
                if( (HWND)lParam == GetParent( hDlg ) )
                {
                    SetForegroundWindow( hDlg );
                }
            }
            break;
    }

    return 0;
}




//-----------------------------------------------------------------------------
// Name: SetStatusChecks()
// Desc: Helper function to set checkmarks by status menu items
//-----------------------------------------------------------------------------
VOID SetStatusChecks( HWND hWnd )
{
    CheckMenuItem( GetMenu( hWnd ), IDSHOWALL,      MF_BYCOMMAND|MF_UNCHECKED );
    CheckMenuItem( GetMenu( hWnd ), IDSHOWUPGRADES, MF_BYCOMMAND|MF_UNCHECKED );
    CheckMenuItem( GetMenu( hWnd ), IDSHOWPROBLEMS, MF_BYCOMMAND|MF_UNCHECKED );
    CheckMenuItem( GetMenu( hWnd ), IDSHOWNOTHING,  MF_BYCOMMAND|MF_UNCHECKED );

    switch( g_dwStatus )
	{
		case SHOW_ALL:
			CheckMenuItem( GetMenu( hWnd ), IDSHOWALL, MF_BYCOMMAND|MF_CHECKED );
			break;
		case SHOW_UPGRADES:
	        CheckMenuItem( GetMenu( hWnd ), IDSHOWUPGRADES, MF_BYCOMMAND|MF_CHECKED );
			break;
		case SHOW_PROBLEMS:
	        CheckMenuItem( GetMenu( hWnd ), IDSHOWPROBLEMS, MF_BYCOMMAND|MF_CHECKED );
			break;
		case SHOW_NONE:
			CheckMenuItem( GetMenu( hWnd ), IDSHOWNOTHING, MF_BYCOMMAND|MF_CHECKED );
			break;
	}
}




//-----------------------------------------------------------------------------
// Name: DirectXInstallWndProc()
// Desc: Processes windows messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DirectXInstallWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_COMMAND:
			// Process menu items
			switch( LOWORD(wParam) )
			{
				case IDINSTALL:
					DirectXInstall( hWnd );
					break;
				case IDGETVERSION:
					DirectXGetVersion();
					break;
				case IDEXIT:
					DestroyWindow( hWnd );
					break;
				case IDSHOWALL:
					g_dwStatus = SHOW_ALL;
					SetStatusChecks( hWnd );
					break;
				case IDSHOWUPGRADES:
					g_dwStatus = SHOW_UPGRADES;
					SetStatusChecks( hWnd );
					break;
				case IDSHOWPROBLEMS:
					g_dwStatus = SHOW_PROBLEMS;
					SetStatusChecks( hWnd );
					break;
				case IDSHOWNOTHING:
					g_dwStatus = SHOW_NONE;
					SetStatusChecks( hWnd );
					break;
				case IDCHECKOLDERINSTALL:
					g_bCheckOlder = !g_bCheckOlder;
					CheckMenuItem( GetMenu(hWnd), IDCHECKOLDERINSTALL,
								                  MF_BYCOMMAND|MF_CHECKED );
					break;
			}
            return 0;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

	return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: DirectXInstallInit()
// Desc: Initializes window data and registers window class
//       Sets up a structure to register the window class.  Structure includes
//       such information as what function will process messages, what cursor
//       and icon to use, etc.
//-----------------------------------------------------------------------------
BOOL DirectXInstallInit( HINSTANCE hInstance )
{
    WNDCLASS wndClass;      // structure pointer
    ZeroMemory( &wndClass, sizeof(WNDCLASS) );
    wndClass.style         = CS_GLOBALCLASS;
    wndClass.lpfnWndProc   = (WNDPROC) DirectXInstallWndProc;
    wndClass.hInstance     = hInstance;
    wndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName  = "MainMenu";
    wndClass.lpszClassName = (LPSTR) "DirectXInstall";

	// Returns result of registering the window
    return RegisterClass( &wndClass );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Calls initialization function, processes message loop
//       This will initialize the window class if it is the first time this
//       application is run.  It then creates the window, and processes the
//       message loop until a PostQuitMessage is received.  It exits the
//       application by returning the value passed by the PostQuitMessage.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				    LPSTR strCmdLine, int nCmdShow )
{
    // Has application been initialized?
    if( NULL == hPrevInstance )
        if( 0 == DirectXInstallInit( hInstance ) )
            return 0;

    g_hInstance = hInstance;

    HWND hWnd = CreateWindow( "DirectXInstall", "DirectX Install",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                              NULL, NULL, g_hInstance, NULL );
    if( NULL == hWnd )
        return 0;

    LoadString( g_hInstance, STR_TITLE, g_strAppTitle, 200 );
    ShowWindow( hWnd, SW_NORMAL );
    UpdateWindow( hWnd );     // Send a WM_PAINT message
    SetStatusChecks( hWnd );  // Check the default message menu item

    MSG msg;
    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

	// Returns the value from PostQuitMessage
    return (int)msg.wParam;    
}
