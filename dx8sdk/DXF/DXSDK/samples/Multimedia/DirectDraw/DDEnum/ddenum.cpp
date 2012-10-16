//-----------------------------------------------------------------------------
// File: ddenum.cpp
//
// Desc: This sample demonstrates how to enumerate all of the devices and show
//       the driver information about each.
//
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <ddraw.h>
#include "resource.h"



//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define MAX_DEVICES     32

struct DEVICEIDENT_STRUCT
{
    DDDEVICEIDENTIFIER2 DeviceInfo;
    DDDEVICEIDENTIFIER2 DeviceInfoHost;
};

DEVICEIDENT_STRUCT g_DeviceIdent[MAX_DEVICES];
int                g_iMaxDevices = 0;




//-----------------------------------------------------------------------------
// Name: UpdateInfoDlgText()
// Desc: Update all of the text and buttons in the dialog
//-----------------------------------------------------------------------------
void UpdateInfoDlgText( HWND hDlg, int iCurrent, DWORD dwHost ) 
{
    TCHAR                 strBuffer[128];
    GUID*                 pGUID;
    LPDDDEVICEIDENTIFIER2 pDI;

    if( dwHost == DDGDI_GETHOSTIDENTIFIER )
        CheckRadioButton( hDlg, IDC_RADIO_DEVICE, IDC_RADIO_HOST, IDC_RADIO_HOST );
    else
        CheckRadioButton( hDlg, IDC_RADIO_DEVICE, IDC_RADIO_DEVICE, IDC_RADIO_DEVICE );

    pDI = &g_DeviceIdent[iCurrent].DeviceInfo;
    if( dwHost == DDGDI_GETHOSTIDENTIFIER )
        pDI = &g_DeviceIdent[iCurrent].DeviceInfoHost;

    wsprintf( strBuffer, "Device information for device %d of %d", 
              iCurrent + 1, g_iMaxDevices );
    SetDlgItemText( hDlg, IDC_RADIO_DEVICE, strBuffer );

    // Device ID stuff:
    wsprintf( strBuffer,"%08X",pDI->dwVendorId );
    SetDlgItemText( hDlg, IDC_DWVENDORID, strBuffer );

    wsprintf( strBuffer,"%08X",pDI->dwDeviceId );
    SetDlgItemText( hDlg, IDC_DWDEVICEID, strBuffer );

    wsprintf( strBuffer,"%08X",pDI->dwSubSysId );
    SetDlgItemText( hDlg, IDC_DWSUBSYS, strBuffer );

    wsprintf( strBuffer,"%08X",pDI->dwRevision );
    SetDlgItemText( hDlg, IDC_DWREVISION, strBuffer );

    // Driver version:
    wsprintf( strBuffer, "%d.%02d.%02d.%04d",  
              HIWORD( pDI->liDriverVersion.u.HighPart ),
              LOWORD( pDI->liDriverVersion.u.HighPart ),
              HIWORD( pDI->liDriverVersion.u.LowPart  ),
              LOWORD( pDI->liDriverVersion.u.LowPart  ) );
    SetDlgItemText( hDlg, IDC_VERSION, strBuffer );

    // Device description and HAL filename
    SetDlgItemText( hDlg, IDC_DESCRIPTION, pDI->szDescription );
    SetDlgItemText( hDlg, IDC_FILENAME,    pDI->szDriver );

    // Unique driver/device identifier:
    pGUID = &pDI->guidDeviceIdentifier;
    wsprintf( strBuffer, "%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
              pGUID->Data1, 
              pGUID->Data2, 
              pGUID->Data3,
              pGUID->Data4[0], pGUID->Data4[1], pGUID->Data4[2], pGUID->Data4[3],
              pGUID->Data4[4], pGUID->Data4[5], pGUID->Data4[6], pGUID->Data4[7] );
    SetDlgItemText( hDlg, IDC_GUID, strBuffer );

    // WHQL Level
    wsprintf( strBuffer,"%08x", pDI->dwWHQLLevel );
    SetDlgItemText( hDlg, IDC_STATIC_WHQLLEVEL, strBuffer );

    // Change the state and style of the Prev and Next buttons if needed
    HWND hNext = GetDlgItem( hDlg, IDC_NEXT );
    HWND hPrev = GetDlgItem( hDlg, IDC_PREV );

    if( 0 == iCurrent )
    {
        // The Prev button should be disabled
        SetFocus( GetDlgItem( hDlg, IDOK ) );
        SendDlgItemMessage( hDlg, IDC_PREV, BM_SETSTYLE, BS_PUSHBUTTON, TRUE );
        SendMessage( hDlg, DM_SETDEFID, IDOK, 0 );
        EnableWindow( hPrev, FALSE );
    }
    else
    {
        if( IsWindowEnabled( hPrev ) == FALSE )
            EnableWindow( hPrev, TRUE );
    }

    if( iCurrent >= (g_iMaxDevices - 1) )
    {
        // The Next button should be disabled
        SetFocus( GetDlgItem( hDlg, IDOK ) );
        SendDlgItemMessage( hDlg, IDC_NEXT, BM_SETSTYLE, BS_PUSHBUTTON, TRUE );
        SendMessage(hDlg, DM_SETDEFID, IDOK, 0 );
        EnableWindow( hNext, FALSE );
    }
    else
    {
        if( IsWindowEnabled( hNext ) == FALSE)
            EnableWindow( hNext, TRUE );
    }
}




//-----------------------------------------------------------------------------
// Name: InfoDlgProc()
// Desc: The dialog window proc
//-----------------------------------------------------------------------------
BOOL CALLBACK InfoDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static int   iCurrent = 0;
    static DWORD dwHost   = 0;

    switch (message)
    {
        case WM_INITDIALOG:
            // Setup the first devices text
            UpdateInfoDlgText( hDlg, iCurrent, dwHost );
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog( hDlg, TRUE );
                    break;

                case IDC_PREV:
                    // Show the previous device
                    if( iCurrent )
                        iCurrent--;

                    UpdateInfoDlgText( hDlg, iCurrent, dwHost );
                    break;

                case IDC_NEXT:
                    // Show the next device
                    if( iCurrent < g_iMaxDevices )
                        iCurrent++;

                    UpdateInfoDlgText( hDlg, iCurrent, dwHost );
                    break;

                case IDC_RADIO_HOST:
                    dwHost = DDGDI_GETHOSTIDENTIFIER;

                    UpdateInfoDlgText( hDlg, iCurrent, dwHost );
                    break;

                case IDC_RADIO_DEVICE:
                    dwHost = 0;

                    UpdateInfoDlgText( hDlg, iCurrent, dwHost );
                    break;

                default:
                    return FALSE; // Message not handled 
            }

        default:
            return FALSE; // Message not handled 
    }

    return TRUE; // Message handled 
}




//-----------------------------------------------------------------------------
// Name: DDEnumCallbackEx()
// Desc: This callback gets the information for each device enumerated
//-----------------------------------------------------------------------------
BOOL WINAPI DDEnumCallbackEx( GUID *pGUID, LPSTR pDescription, LPSTR strName,
                              LPVOID pContext, HMONITOR hm )
{
    LPDIRECTDRAW7 pDD = NULL;	
    HRESULT hr;

    // Create a DirectDraw object using the enumerated GUID
    if( FAILED( hr = DirectDrawCreateEx( pGUID, (VOID**)&pDD, 
                                         IID_IDirectDraw7, NULL ) ) )
        return DDENUMRET_CANCEL;

    // Get the device information and save it
    pDD->GetDeviceIdentifier( &g_DeviceIdent[g_iMaxDevices].DeviceInfo, 0 );
    pDD->GetDeviceIdentifier( &g_DeviceIdent[g_iMaxDevices].DeviceInfoHost, 
                              DDGDI_GETHOSTIDENTIFIER );

    // Finished with the DirectDraw object, so release it
    SAFE_RELEASE( pDD ); 

    // Bump to the next open slot or finish the callbacks if full
    if( g_iMaxDevices < MAX_DEVICES )
        g_iMaxDevices++;
    else
        return DDENUMRET_CANCEL;

    return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: DDEnumCallback()
// Desc: Old style callback retained for backwards compatibility
//-----------------------------------------------------------------------------
BOOL WINAPI DDEnumCallback( GUID *pGUID, LPSTR pDescription, 
                            LPSTR strName, LPVOID pContext )
{
    return ( DDEnumCallbackEx( pGUID, pDescription, strName, pContext, NULL ) );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything and calls
//       DirectDrawEnumerateEx() to get all of the device info.
//-----------------------------------------------------------------------------
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{
    LPDIRECTDRAWENUMERATEEX pDirectDrawEnumerateEx;
    HINSTANCE               hDDrawDLL = NULL;

    // Do a GetModuleHandle and GetProcAddress in order to get the
    // DirectDrawEnumerateEx function
    hDDrawDLL = GetModuleHandle("DDRAW");
    if( NULL == hDDrawDLL )
    {
        MessageBox( NULL, "LoadLibrary() FAILED", 
                    "DirectDraw Sample", MB_OK | MB_ICONERROR );
        return -1;
    }

    pDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress( hDDrawDLL, "DirectDrawEnumerateExA" );
    if( pDirectDrawEnumerateEx )
    {
        pDirectDrawEnumerateEx( DDEnumCallbackEx, NULL,
                                DDENUM_ATTACHEDSECONDARYDEVICES |
                                DDENUM_DETACHEDSECONDARYDEVICES |
                                DDENUM_NONDISPLAYDEVICES );
    }
    else
    {
        // Old DirectDraw, so do it the old way
        DirectDrawEnumerate( DDEnumCallback, NULL );
    }

    if( 0 == g_iMaxDevices )
    {
        MessageBox( NULL, "No devices to enumerate.", 
                    "DirectDraw Sample", MB_OK | MB_ICONERROR );
        return -1;
    }

    // Bring up the dialog to show all the devices
    DialogBox( hInst, MAKEINTRESOURCE(IDD_DRIVERINFO), 
               GetDesktopWindow(), (DLGPROC)InfoDlgProc );

    return 0;
}

