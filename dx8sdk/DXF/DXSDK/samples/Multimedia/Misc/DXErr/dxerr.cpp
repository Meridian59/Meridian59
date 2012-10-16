//----------------------------------------------------------------------------
// File: DXErr.cpp
//
// Desc: The DXErr sample allows users to enter a numberical HRESULT and get back
//       the string match its define.  For example, entering 0x8878000a will
//       return DSERR_ALLOCATED.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <dxerr8.h>
#include <tchar.h>
#include "resource.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
VOID OnInitDialog( HWND hDlg );
VOID LookupValue( HWND hDlg );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
            OnInitDialog( hDlg );
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_LOOKUP:
                    LookupValue( hDlg );
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_DESTROY:
            break; 

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------
VOID OnInitDialog( HWND hDlg )
{
    // Load the icon
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    SendMessage( hDlg, EM_LIMITTEXT, 20, 0 );  
}



//-----------------------------------------------------------------------------
// Name: LookupValue()
// Desc: 
//-----------------------------------------------------------------------------
VOID LookupValue( HWND hDlg )
{
    HRESULT hrErr = 0;
    TCHAR strValue[MAX_PATH];
    const TCHAR* strHRESULT;
    const TCHAR* strDescription;
    TCHAR strHRESULTCopy[MAX_PATH*2];
    int nIndex;
    int nPower = 0;
    int nDigit = 0;
    GetDlgItemText( hDlg, IDC_VALUE, strValue, MAX_PATH );

    nIndex = lstrlen(strValue) - 1;

    // skip whitespace
    while( nIndex >= 0 )
    {
    	if( strValue[nIndex] != ' ' && 
            strValue[nIndex] != 'L' )
    	    break;
    	    
        nIndex--;
    }

    while( nIndex >= 0 )
    {
        // Convert to uppercase
        if( strValue[nIndex] >= 'a' && strValue[nIndex] <= 'z' )
            strValue[nIndex] += 'A' - 'a';

        if( strValue[nIndex] >= 'A' && strValue[nIndex] <= 'F' )
            nDigit = strValue[nIndex] - 'A' + 10;
        else if( strValue[nIndex] >= '0' && strValue[nIndex] <= '9' )
            nDigit = strValue[nIndex] - '0';
        else
            break;

        hrErr += ( nDigit << (nPower*4) );

        nIndex--;
        nPower++;
    }

    // Use DXErr8.lib to lookup HRESULT.
    strHRESULT = DXGetErrorString8( hrErr );
    _tcscpy( strHRESULTCopy, TEXT("HRESULT: ") );
    _tcscat( strHRESULTCopy, strHRESULT );

    strDescription = DXGetErrorDescription8( hrErr );

    TCHAR* strTemp;
    while( strTemp = _tcschr( strHRESULTCopy, '&') )
    {
        strTemp[0] = '\r';
        strTemp[1] = '\n';
    }

    if( lstrlen(strDescription) > 0 )
    {
        _tcscat( strHRESULTCopy, TEXT("\r\nDescription: ") );
        _tcscat( strHRESULTCopy, strDescription );
    }

    SetDlgItemText( hDlg, IDC_MESSAGE, strHRESULTCopy );
    return;
}
