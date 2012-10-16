//----------------------------------------------------------------------------
// File: EnumDevices.cpp
//
// Desc: This sample shows how to enumerate DirectSound sound and capture 
//       devices.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <mmreg.h>
#include <dxerr8.h>
#include <dsound.h>
#include "resource.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,
                                  VOID* pContext );
HRESULT OnInitDialog( HWND hDlg );
HRESULT InitDirectSound( HWND hDlg );
HRESULT FreeDirectSound();



//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LPDIRECTSOUND        g_pDS        = NULL;
LPDIRECTSOUNDCAPTURE g_pDSCapture = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    // Initialize COM
    CoInitialize( NULL );

    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );

    // Release COM
    CoUninitialize();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr; 

    switch( msg ) 
    {
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_CREATE:
                    // Init DirectSound
                    if( SUCCEEDED( hr = InitDirectSound( hDlg ) ) )
                    {
                        MessageBox( hDlg, TEXT("DirectSound interface created successfully"), 
                                          TEXT("EnumDevices"), MB_OK );
                    }
                    else
                    {
                        DXTRACE_ERR( TEXT("InitDirectSound"), hr );
                        MessageBox( hDlg, TEXT("DirectSound interface creatation failed"), 
                                          TEXT("EnumDevices"), MB_OK | MB_ICONERROR );
                    }
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_INITDIALOG:
            if( FAILED( hr = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), hr );
                MessageBox( hDlg, "Error enumerating DirectSound devices. "
                                  "Sample will now exit.", "DirectSound Sample", 
                                  MB_OK | MB_ICONERROR );
                EndDialog( hDlg, IDABORT );
            }
            break;

        case WM_DESTROY:
            // Cleanup everything
            FreeDirectSound();
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
HRESULT OnInitDialog( HWND hDlg )
{
    HRESULT hr;

    // Load the icon
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    PostMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    PostMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Enumerate the sound devices and place them in the combo box
    HWND hSoundDeviceCombo = GetDlgItem( hDlg, IDC_SOUND_DEVICE_COMBO );
    if( FAILED( hr = DirectSoundEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                           (VOID*)hSoundDeviceCombo ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundEnumerate"), hr );

    // Enumerate the capture devices and place them in the combo box
    HWND hCaptureDeviceCombo = GetDlgItem( hDlg, IDC_CAPTURE_DEVICE_COMBO );
    if( FAILED( hr = DirectSoundCaptureEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                                  (VOID*)hCaptureDeviceCombo ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCaptureEnumerate"), hr );

    // Select the first device in the combo box
    SendMessage( hSoundDeviceCombo,   CB_SETCURSEL, 0, 0 );
    SendMessage( hCaptureDeviceCombo, CB_SETCURSEL, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DSoundEnumCallback()
// Desc: Enumeration callback called by DirectSoundEnumerate
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName,
                                  VOID* pContext )
{
    // Set aside static storage space for 20 audio drivers
    static GUID  AudioDriverGUIDs[20];
    static DWORD dwAudioDriverIndex = 0;

    GUID* pTemp  = NULL;

    if( pGUID )
    {
        if( dwAudioDriverIndex >= 20 )
            return TRUE;

        pTemp = &AudioDriverGUIDs[dwAudioDriverIndex++];
        memcpy( pTemp, pGUID, sizeof(GUID) );
    }

    HWND hSoundDeviceCombo = (HWND)pContext;

    // Add the string to the combo box
    SendMessage( hSoundDeviceCombo, CB_ADDSTRING, 
                 0, (LPARAM) (LPCTSTR) strDesc );

    // Get the index of the string in the combo box
    INT nIndex = (INT)SendMessage( hSoundDeviceCombo, CB_FINDSTRING, 
                                   0, (LPARAM) (LPCTSTR) strDesc );

    // Set the item data to a pointer to the static guid stored in AudioDriverGUIDs
    SendMessage( hSoundDeviceCombo, CB_SETITEMDATA, 
                 nIndex, (LPARAM) pTemp );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT InitDirectSound( HWND hDlg )
{
    HRESULT hr;

    // Free any previous DirectSound objects
    FreeDirectSound();

    // Get the HWNDs the combo boxes
    HWND hSoundDeviceCombo   = GetDlgItem( hDlg, IDC_SOUND_DEVICE_COMBO );
    HWND hCaptureDeviceCombo = GetDlgItem( hDlg, IDC_CAPTURE_DEVICE_COMBO );

    // Get the index of the currently selected devices
    INT nSoundIndex   = (INT)SendMessage( hSoundDeviceCombo,   CB_GETCURSEL, 0, 0 ); 
    INT nCaptureIndex = (INT)SendMessage( hCaptureDeviceCombo, CB_GETCURSEL, 0, 0 ); 

    // Get the GUID attached to the combo box item
    GUID* pSoundGUID = (GUID*) SendMessage( hSoundDeviceCombo, CB_GETITEMDATA, 
                                            nSoundIndex, 0 );
    GUID* pCaptureGUID = (GUID*) SendMessage( hCaptureDeviceCombo, CB_GETITEMDATA, 
                                              nCaptureIndex, 0 );

    // Create IDirectSound using the select sound device
    if( FAILED( hr = DirectSoundCreate( pSoundGUID, &g_pDS, NULL ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCreate"), hr );

    // Release the IDirectSound object immediately since we don't want
    // to limit this sample to only computers that support full duplex audio
    SAFE_RELEASE( g_pDS ); 

    // Create IDirectSoundCapture using the select capture device
    if( FAILED( hr = DirectSoundCaptureCreate( pCaptureGUID, &g_pDSCapture, NULL ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCaptureCreate"), hr );

    // Release g_pDSCapture, since we don't need it really 
    SAFE_RELEASE( g_pDSCapture ); 

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectSound()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT FreeDirectSound()
{
    // Release DirectSound interfaces
    SAFE_RELEASE( g_pDSCapture ); 
    SAFE_RELEASE( g_pDS ); 

    return S_OK;
}



