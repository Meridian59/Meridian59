//-----------------------------------------------------------------------------
// File: DXVer.cpp
//
// Desc: Windows code that calls GetDXVersion and displays the results.
//
// (C) Copyright 1995-2001 Microsoft Corp.  All rights reserved.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <tchar.h>




//-----------------------------------------------------------------------------
// External function-prototypes
//-----------------------------------------------------------------------------
extern DWORD GetDXVersion();




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and pops
//       up a message box with the results of the GetDXVersion call
//-----------------------------------------------------------------------------
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR strCmdLine, int nCmdShow )
{
    TCHAR* strResult;
    DWORD  dwDXVersion = GetDXVersion();

    switch( dwDXVersion )
    {
        case 0x000:
            strResult = _T("No DirectX installed" );
            break;
        case 0x100:
            strResult = _T("DirectX 1 installed" );
            break;
        case 0x200:
            strResult = _T("DirectX 2 installed" );
            break;
        case 0x300:
            strResult = _T("DirectX 3 installed" );
            break;
        case 0x500:
            strResult = _T("DirectX 5 installed" );
            break;
        case 0x600:
            strResult = _T("DirectX 6 installed" );
            break;
        case 0x601:
            strResult = _T("DirectX 6.1 installed" );
            break;
        case 0x700:
            strResult = _T("DirectX 7" );
            break;
        case 0x800:
            strResult = _T("DirectX 8.0 installed" );
            break;
        case 0x801:
            strResult = _T("DirectX 8.1 or better installed" );
            break;
        default:
            strResult = _T("Unknown version of DirectX installed." );
            break;
    }

    MessageBox( NULL, strResult, "DirectX Version:",
                MB_OK | MB_ICONINFORMATION );
    
    return 0;
}



