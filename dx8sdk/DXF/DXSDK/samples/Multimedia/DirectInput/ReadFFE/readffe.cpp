//-----------------------------------------------------------------------------
// File: ReadFFE.cpp
//
// Desc: DirectInput support to enumerate and play all effects in stored in a 
//       DirectInput effects file.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <dinput.h>
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
struct EFFECTS_NODE
{
    LPDIRECTINPUTEFFECT pDIEffect;
    DWORD               dwPlayRepeatCount;
    EFFECTS_NODE*       pNext;
};

LPDIRECTINPUT8        g_pDI       = NULL;         
LPDIRECTINPUTDEVICE8  g_pFFDevice = NULL;
EFFECTS_NODE          g_EffectsList;




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDialogProc( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK EnumFFDevicesCallback( LPCDIDEVICEINSTANCE pDDI, VOID* pvRef );
BOOL CALLBACK EnumAndCreateEffectsCallback( LPCDIFILEEFFECT pDIFileEffect, VOID* pvRef );

HRESULT InitDirectInput( HWND hDlg );
HRESULT FreeDirectInput();
VOID    EmptyEffectList();
HRESULT OnReadFile( HWND hDlg );
HRESULT OnPlayEffects( HWND hDlg );




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.
//-----------------------------------------------------------------------------
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
{
    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: MainDialogProc
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
        case WM_INITDIALOG:
        {
            // Set the icon for this dialog.
            HICON hIcon = LoadIcon( (HINSTANCE)GetModuleHandle(NULL), 
                                    MAKEINTRESOURCE( IDI_ICON ) );
            PostMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM)hIcon );  // Set big icon
            PostMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon );  // Set small icon

            EnableWindow( GetDlgItem( hDlg, IDC_PLAY_EFFECTS ), FALSE );

            hr = InitDirectInput( hDlg );
            if( FAILED(hr) )
            {
                MessageBox( NULL, _T("Error Initializing DirectInput. ")
                                  _T("The sample will now exit."),
                                  _T("ReadFFE"), MB_ICONERROR | MB_OK );                
                EndDialog( hDlg, TRUE );
            }
            return TRUE;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, FALSE ); 
                    return TRUE;

                case IDC_READ_FILE:
                    if( FAILED( hr = OnReadFile( hDlg ) ) )
                    {
                        MessageBox( NULL, _T("Error reading effects file."),
                                          _T("ReadFFE"), MB_ICONERROR | MB_OK );
                        EnableWindow( GetDlgItem( hDlg, IDC_PLAY_EFFECTS ), FALSE );
                    }
                    return TRUE;

                case IDC_PLAY_EFFECTS:
                    if( FAILED( hr = OnPlayEffects( hDlg ) ) )
                    {
                        MessageBox( NULL, _T("Error playing DirectInput effects. ")
                                          _T("The sample will now exit."), 
                                          _T("ReadFFE"), MB_ICONERROR | MB_OK );
                        EndDialog( hDlg, 1 );
                    }
                    return TRUE;
            }
            break;

        case WM_DESTROY:
            FreeDirectInput();   
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT InitDirectInput( HWND hDlg )
{
    HRESULT hr;

    // Setup the g_EffectsList circular linked list
    ZeroMemory( &g_EffectsList, sizeof( EFFECTS_NODE ) );
    g_EffectsList.pNext = &g_EffectsList;

    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return hr;

    // Get the first enumerated force feedback device
    if( FAILED( hr = g_pDI->EnumDevices( 0, EnumFFDevicesCallback, 0, 
                                         DIEDFL_ATTACHEDONLY | 
                                         DIEDFL_FORCEFEEDBACK ) ) )
        return hr;
    
    if( g_pFFDevice == NULL )
    {
        MessageBox( hDlg, _T("No force feedback device found.  ")
                          _T("The sample will now exit."), 
                          _T("ReadFFE"), MB_ICONERROR | MB_OK );
        EndDialog( hDlg, 0 );
        return S_OK;
    }

    // Set the data format
    if( FAILED( hr = g_pFFDevice->SetDataFormat( &c_dfDIJoystick ) ) )
        return hr;

    // Set the coop level
    if( FAILED( hr = g_pFFDevice->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | 
                                                             DISCL_BACKGROUND ) ) )
        return hr;

    // Disable auto-centering spring
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = FALSE;

    if( FAILED( hr = g_pFFDevice->SetProperty( DIPROP_AUTOCENTER, &dipdw.diph ) ) )
        return hr;

    // Acquire the device
    if( FAILED( hr = g_pFFDevice->Acquire() ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumFFDevicesCallback()
// Desc: Get the first enumerated force feedback device
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumFFDevicesCallback( LPCDIDEVICEINSTANCE pDDI, VOID* pvRef )
{
    if( FAILED( g_pDI->CreateDevice( pDDI->guidInstance, &g_pFFDevice, NULL ) ) )
        return DIENUM_CONTINUE; // If failed, try again

    // Stop when a device was successfully found
    return DIENUM_STOP;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT FreeDirectInput()
{
    // Release any DirectInputEffect objects.
    if( g_pFFDevice ) 
    {
        EmptyEffectList();
        g_pFFDevice->Unacquire();
        SAFE_RELEASE( g_pFFDevice );
    }

    // Release any DirectInput objects.
    SAFE_RELEASE( g_pDI );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EmptyEffectList()
// Desc: Goes through the circular linked list and releases the effects, 
//       and deletes the nodes
//-----------------------------------------------------------------------------
VOID EmptyEffectList()
{
    EFFECTS_NODE* pEffectNode = g_EffectsList.pNext;
    EFFECTS_NODE* pEffectDelete;

    while ( pEffectNode != &g_EffectsList )
    {
        pEffectDelete = pEffectNode;       
        pEffectNode = pEffectNode->pNext;

        SAFE_RELEASE( pEffectDelete->pDIEffect );
        SAFE_DELETE( pEffectDelete );
    }

    g_EffectsList.pNext = &g_EffectsList;
}




//-----------------------------------------------------------------------------
// Name: OnReadFile()
// Desc: Reads a file contain a collection of DirectInput force feedback 
//       effects.  It creates each of effect read in and stores it 
//       in the linked list, g_EffectsList.
//-----------------------------------------------------------------------------
HRESULT OnReadFile( HWND hDlg )
{
    HRESULT hr;

    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hDlg, NULL,
                         TEXT("FEdit Files\0*.ffe\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open FEdit File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".ffe"), 0, NULL, NULL };

    // Get the default media path (something like C:\DXSDK\SAMPLES\MULTIMEDIA\DINPUT\MEDIA)
    if( '\0' == strPath[0] )
        _tcscpy( strPath, DXUtil_GetDXSDKMediaPath() );

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( FALSE == GetOpenFileName( &ofn ) )
        return S_OK;

    EmptyEffectList();

    // Enumerate the effects in the file selected, and create them in the callback
    if( FAILED( hr = g_pFFDevice->EnumEffectsInFile( strFileName, 
                                                     EnumAndCreateEffectsCallback, 
                                                     NULL, DIFEF_MODIFYIFNEEDED ) ) )
        return hr;

    // Remember the path for next time
    _tcscpy( strPath, strFileName );
    TCHAR* strLastSlash = _tcsrchr( strPath, '\\' );
    strLastSlash[0] = '\0';

    // If list of effects is empty, then we haven't been able to create any effects
    if( g_EffectsList.pNext == &g_EffectsList )
    {
        // Pop up a box informing the user
        MessageBox( hDlg, _T("Unable to create any effects."),
                          _T("ReadFFE"), MB_ICONEXCLAMATION | MB_OK );
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY_EFFECTS ), FALSE );
    }
    else
    {
        // We have effects so enable the 'play effects' button
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY_EFFECTS ), TRUE );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumAndCreateEffectsCallback()
// Desc: Create the effects as they are enumerated and add them to the 
//       linked list, g_EffectsList
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumAndCreateEffectsCallback( LPCDIFILEEFFECT pDIFileEffect, VOID* pvRef )
{   
    HRESULT hr;
    LPDIRECTINPUTEFFECT pDIEffect = NULL;

    // Create the file effect
    if( FAILED( hr = g_pFFDevice->CreateEffect( pDIFileEffect->GuidEffect, 
                                                pDIFileEffect->lpDiEffect, 
                                                &pDIEffect, NULL ) ) )
    {
        OutputDebugString( TEXT("Could not create force feedback effect on this device.\n") );
        return DIENUM_CONTINUE;
    }

    // Create a new effect node
    EFFECTS_NODE* pEffectNode = new EFFECTS_NODE;
    if( NULL == pEffectNode )
        return DIENUM_STOP;

    // Fill the pEffectNode up
    ZeroMemory( pEffectNode, sizeof( EFFECTS_NODE ) );
    pEffectNode->pDIEffect         = pDIEffect;
    pEffectNode->dwPlayRepeatCount = 1;

    // Add pEffectNode to the circular linked list, g_EffectsList
    pEffectNode->pNext  = g_EffectsList.pNext;
    g_EffectsList.pNext = pEffectNode;

    return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: OnPlayEffects()
// Desc: Plays all of the effects enumerated in the file 
//-----------------------------------------------------------------------------
HRESULT OnPlayEffects( HWND hDlg )
{
    EFFECTS_NODE*       pEffectNode = g_EffectsList.pNext;
    LPDIRECTINPUTEFFECT pDIEffect   = NULL;
    HRESULT             hr;

    // Stop all previous forces
    if( FAILED( hr = g_pFFDevice->SendForceFeedbackCommand( DISFFC_STOPALL ) ) )
        return hr;

    while ( pEffectNode != &g_EffectsList )
    {
        // Play all of the effects enumerated in the file 
        pDIEffect = pEffectNode->pDIEffect;

        if( NULL != pDIEffect )
        {
            if( FAILED( hr = pDIEffect->Start( pEffectNode->dwPlayRepeatCount, 0 ) ) )
                return hr;
        }

        pEffectNode = pEffectNode->pNext;
    }

    return S_OK;
}




