//----------------------------------------------------------------------------
// File: LobbyClient.cpp
//
// Desc: LobbyClient is a simple lobby client.  It displays all registered DirectPlay 
//       applications on the local system.  It allows the 
//       user to launch one or more of these applications using a chosen 
//       service provider.  A launched lobbied application may be told to either 
//       join or host a game.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <winsock.h>
#include <windows.h>
#include <basetsd.h>
#include <dplay8.h>
#include <dplobby8.h>
#include <dpaddr.h>
#include <dxerr8.h>
#include <cguid.h>
#include <tchar.h>
#include "DXUtil.h"
#include "resource.h"



//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#define DPLAY_SAMPLE_KEY        TEXT("Software\\Microsoft\\DirectX DirectPlay Samples")
#define WM_APP_APPDISCONNECTED  (WM_APP + 1)
#define WM_APP_SETSTATUS        (WM_APP + 2)




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Peer*  g_pDP                         = NULL;    // DirectPlay peer object
IDirectPlay8LobbyClient* g_pLobbyClient          = NULL;    // DirectPlay lobby client
HINSTANCE          g_hInst                       = NULL;    // HINST of app
HWND               g_hDlg                        = NULL;    // HWND of main dialog
TCHAR              g_strAppName[256]             = TEXT("LobbyClient");
GUID*              g_pCurSPGuid                  = NULL;    // Currently selected guid
TCHAR              g_strPlayerName[MAX_PATH];               // Local player name
TCHAR              g_strSessionName[MAX_PATH];              // Session name
TCHAR              g_strPreferredProvider[MAX_PATH];        // Provider string
TCHAR              g_strLocalIP[MAX_PATH];                  // Provider string




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI   DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayLobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK LobbyClientDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT          InitDirectPlay();
HRESULT          OnInitDialog( HWND hDlg );
VOID             SetupAddressFields( HWND hDlg );
HRESULT          AllocAndInitConnectSettings( HWND hDlg, GUID* pAppGuid, DPL_CONNECTION_SETTINGS** ppdplConnectSettings );
VOID             FreeConnectSettings( DPL_CONNECTION_SETTINGS* pSettings );
HRESULT          LaunchApp( HWND hDlg );
HRESULT          EnumRegisteredApplications( HWND hDlg );
HRESULT          EnumServiceProviders( HWND hDlg );
HRESULT          EnumAdapters( HWND hDlg, GUID* pSPGuid );
HRESULT          SendMsgToApp( HWND hDlg );
HRESULT          DisconnectFromApp( HWND hDlg );







//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, 
                      LPSTR pCmdLine, INT nCmdShow )
{
    HRESULT hr;
    HKEY    hDPlaySampleRegKey;

    g_hInst = hInst; 

    // Read persistent state information from registry
    RegCreateKeyEx( HKEY_CURRENT_USER, DPLAY_SAMPLE_KEY, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    &hDPlaySampleRegKey, NULL );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), 
                             g_strPlayerName, MAX_PATH, TEXT("TestPlayer") );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), 
                             g_strSessionName, MAX_PATH, TEXT("TestGame") );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), 
                             g_strPreferredProvider, MAX_PATH, 
                             TEXT("DirectPlay8 TCP/IP Service Provider") );

    // Init COM so we can use CoCreateInstance
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if( FAILED( hr = InitDirectPlay() ) )
    {
        DXTRACE_ERR( TEXT("InitDirectPlay"), hr );
        MessageBox( NULL, TEXT("Failed initializing IDirectPlay8Peer. ")
                    TEXT("The sample will now quit."),
                    g_strAppName, MB_OK | MB_ICONERROR );
        return FALSE;
    }

    DialogBox( hInst, MAKEINTRESOURCE(IDD_LOBBY_CLIENT), NULL, 
               (DLGPROC) LobbyClientDlgProc );

    if( g_pLobbyClient )
    {
        g_pLobbyClient->Close( 0 );
        SAFE_RELEASE( g_pLobbyClient );
    }

    if( g_pDP )
    {
        g_pDP->Close(0);
        SAFE_RELEASE( g_pDP );
    }

    CoUninitialize();

    // Write information to the registry
    DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), g_strPlayerName );
    DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), g_strSessionName );
    DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), g_strPreferredProvider );

    RegCloseKey( hDPlaySampleRegKey );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectPlay()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT InitDirectPlay()
{
    HRESULT hr;

    // Create and init IDirectPlay8Peer
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Peer, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Peer, 
                                       (LPVOID*) &g_pDP ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    if( FAILED( hr = g_pDP->Initialize( NULL, DirectPlayMessageHandler, 0 ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    // Create and init IDirectPlay8LobbyClient
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8LobbyClient, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8LobbyClient, 
                                       (LPVOID*) &g_pLobbyClient ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    if( FAILED( hr = g_pLobbyClient->Initialize( NULL, DirectPlayLobbyMessageHandler, 0 ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LobbyClientDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK LobbyClientDlgProc( HWND hDlg, UINT msg, 
                                     WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
        case WM_INITDIALOG:
        {
            g_hDlg = hDlg;
            if( FAILED( hr = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), hr );
                MessageBox( NULL, TEXT("Failed initializing dialog box. ")
                            TEXT("The sample will now quit."),
                            g_strAppName, MB_OK | MB_ICONERROR );
                EndDialog( hDlg, 0 );
            }
            break;
        }

        case WM_APP_APPDISCONNECTED:
        {
            // This is an app defined msg that keeps the 
            // lobby message handler from blocking on the dialog thread.
            DPNHANDLE hDisconnectId = (DPNHANDLE) lParam;
            HRESULT   hrReason      = (HRESULT) wParam;

            // Remove this connection from the list
            TCHAR strBuffer[200];
            wsprintf( strBuffer, TEXT("0x%x"), hDisconnectId );
            int nIndex = (int) SendDlgItemMessage( g_hDlg, IDC_ACTIVE_CONNECTIONS, 
                                                   LB_FINDSTRINGEXACT, -1, (LPARAM) strBuffer );
            SendDlgItemMessage( g_hDlg, IDC_ACTIVE_CONNECTIONS, LB_DELETESTRING, nIndex, 0 );

            // Tell the user
            wsprintf( strBuffer, TEXT("0x%x was disconnected. Reason: 0x%0.8x"), 
                      hDisconnectId, hrReason );
            MessageBox( g_hDlg, strBuffer, g_strAppName, MB_OK );
            break;
        }

        case WM_APP_SETSTATUS:
        {
            // This is an app defined msg that keeps the 
            // lobby message handler from blocking on the dialog thread.
            TCHAR* strBuffer = (TCHAR*) lParam;                     

            if( strBuffer )
            {
                SetDlgItemText( g_hDlg, IDC_STATUS, strBuffer );
                SAFE_DELETE_ARRAY( strBuffer );
            }
            break;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDC_HOST_SESSION:
                    SetupAddressFields( hDlg );
                    break;

                case IDC_SP_COMBO:
                {
                    // If the pSPGuid changed then re-enum the adapters, and
                    // update the address fields.
                    int nSPIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
                    GUID* pSPGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, nSPIndex, 0 );
                    if( pSPGuid != NULL && g_pCurSPGuid != pSPGuid )
                    {
                        g_pCurSPGuid = pSPGuid;
                        SetupAddressFields( hDlg );
                        EnumAdapters( hDlg, pSPGuid );
                    }
                    break;
                }

                case IDC_LAUNCH:
                    if( FAILED( hr = LaunchApp( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("LaunchApp"), hr );
                        MessageBox( NULL, TEXT("Failure trying to launch app. ")
                                    TEXT("The sample will now quit."),
                                    g_strAppName, MB_OK | MB_ICONERROR );
                        EndDialog( hDlg, 0 );
                    }
                    break;

                case IDC_SEND_MSG:
                    // Send a dummy message to a connected app for demo purposes
                    SendMsgToApp( hDlg );
                    break;

                case IDC_DISCONNECT:
                    // Disconnect from an app
                    DisconnectFromApp( hDlg );
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, 0 );
                    return TRUE;
            }
            break;
        }

        case WM_DESTROY:
        {
            GetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strPlayerName, MAX_PATH );
            GetDlgItemText( hDlg, IDC_SESSION_NAME, g_strSessionName, MAX_PATH );
            int nIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
            SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETLBTEXT, nIndex, (LPARAM) g_strPreferredProvider );

            GUID* pGuid;
            int nCount,i;
            nCount = (int)SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_GETCOUNT, 0, 0 );
            for( i=0; i<nCount; i++ )
            {
                pGuid = (LPGUID) SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_GETITEMDATA, i, 0 );
                SAFE_DELETE( pGuid );
            }

            nCount = (int)SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCOUNT, 0, 0 );
            for( i=0; i<nCount; i++ )
            {
                pGuid = (LPGUID) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, i, 0 );
                SAFE_DELETE( pGuid );
            }

            nCount = (int)SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_GETCOUNT, 0, 0 );
            for( i=0; i<nCount; i++ )
            {
                pGuid = (LPGUID) SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_GETITEMDATA, i, 0 );
                SAFE_DELETE( pGuid );
            }

            break;
        }
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnInitDialog( HWND hDlg )
{
    HRESULT hr;

    // Load and set the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    CheckDlgButton( hDlg, IDC_HOST_SESSION, BST_CHECKED );
    CheckRadioButton( hDlg, IDC_LAUNCH_NEW, IDC_DONT_LAUNCH, IDC_LAUNCH_NOT_FOUND );

    SetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strPlayerName );
    SetDlgItemText( hDlg, IDC_SESSION_NAME, g_strSessionName );

    if( FAILED( hr = EnumRegisteredApplications( hDlg ) ) )
        return DXTRACE_ERR( TEXT("EnumRegisteredApplications"), hr );

    if( FAILED( hr = EnumServiceProviders( hDlg ) ) )
        return DXTRACE_ERR( TEXT("EnumServiceProviders"), hr );
    
    WSADATA WSAData;
    _tcscpy( g_strLocalIP, TEXT("") );
    if( WSAStartup (MAKEWORD(1,0), &WSAData) == 0) 
    {
        CHAR strLocalHostName[MAX_PATH];
        gethostname( strLocalHostName, MAX_PATH );
        HOSTENT* pHostEnt = gethostbyname( strLocalHostName );
        if( pHostEnt )
        {
            in_addr* pInAddr = (in_addr*) pHostEnt->h_addr_list[0];
            char* strLocalIP = inet_ntoa( *pInAddr );
            if( strLocalIP )
                DXUtil_ConvertAnsiStringToGeneric( g_strLocalIP, strLocalIP );
        }

        WSACleanup();
    }
    
    SetupAddressFields( hDlg );

    int nSPIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
    GUID* pSPGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, nSPIndex, 0 );
    if( pSPGuid != NULL )
    {
        g_pCurSPGuid = pSPGuid;
        EnumAdapters( hDlg, pSPGuid );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetupAddressFields
// Desc: Based on the SP selected, update the address UI 
//-----------------------------------------------------------------------------
VOID SetupAddressFields( HWND hDlg )
{
    int nSPIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
    if( nSPIndex == LB_ERR )
        return;
    GUID* pGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, nSPIndex, 0 );
    if( pGuid == NULL )
        return;

    BOOL bHosting = IsDlgButtonChecked( hDlg, IDC_HOST_SESSION );

    if( *pGuid == CLSID_DP8SP_TCPIP )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2), TRUE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2, TEXT("2323") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2_TEXT), TRUE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2_TEXT, TEXT("Port:") );

        if( bHosting )
        {
            EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1), FALSE );
            SetDlgItemText( hDlg, IDC_ADDRESS_LINE1, TEXT("") );
            EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1_TEXT), FALSE );
            SetDlgItemText( hDlg, IDC_ADDRESS_LINE1_TEXT, TEXT("") );
        }
        else
        {
            EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1), TRUE );
            SetDlgItemText( hDlg, IDC_ADDRESS_LINE1, g_strLocalIP );
            EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1_TEXT), TRUE );
            SetDlgItemText( hDlg, IDC_ADDRESS_LINE1_TEXT, TEXT("IP Address:") );
        }
    }
    else if( *pGuid == CLSID_DP8SP_MODEM )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1), TRUE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE1, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1_TEXT), TRUE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE1_TEXT, TEXT("Phone Number:") );

        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2_TEXT), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2_TEXT, TEXT("") );
    }
    else 
    {
        // pGuid is CLSID_DP8SP_IPX or CLSID_DP8SP_SERIAL or unknown 
        // so disable all the address lines 
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE1, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1_TEXT), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE1_TEXT, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2_TEXT), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2_TEXT, TEXT("") );
    }
}




//-----------------------------------------------------------------------------
// Name: EnumRegisteredApplications
// Desc: Enum all the lobby launchable apps and put them in the listbox
//-----------------------------------------------------------------------------
HRESULT EnumRegisteredApplications( HWND hDlg )
{
    HRESULT hr;
    DWORD   dwSize     = 0;
    DWORD   dwPrograms = 0;
    DWORD   iProgram;
    BYTE*   pData   = NULL;

    hr = g_pLobbyClient->EnumLocalPrograms( NULL, pData, &dwSize, &dwPrograms, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL && FAILED(hr) )
        return DXTRACE_ERR( TEXT("EnumLocalPrograms"), hr );

    if( dwSize == 0 )
    {
        MessageBox( NULL, TEXT("There are no applications registered as lobby launchable. ")
                    TEXT("The sample will now quit."),
                    g_strAppName, MB_OK | MB_ICONERROR );
        EndDialog( hDlg, 0 );
        return S_OK;
    }

    pData = new BYTE[dwSize];
    if( FAILED( hr = g_pLobbyClient->EnumLocalPrograms( NULL, pData, &dwSize, &dwPrograms, 0 ) ) )
        return DXTRACE_ERR( TEXT("EnumLocalPrograms"), hr );

    DPL_APPLICATION_INFO* pAppInfo = (DPL_APPLICATION_INFO*) pData;
    for( iProgram=0; iProgram<dwPrograms; iProgram++ )
    {
        TCHAR strAppName[MAX_PATH];
        DXUtil_ConvertWideStringToGeneric( strAppName, pAppInfo->pwszApplicationName );

        // Add the name to the listbox
        int nIndex = (int) SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_ADDSTRING, 0, (LPARAM) strAppName );

        // Store the guid in the listbox
        GUID* pGuid = new GUID;
        memcpy( pGuid, &pAppInfo->guidApplication, sizeof(GUID) );
        SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_SETITEMDATA, nIndex, (LPARAM) pGuid );

        pAppInfo++;
    }

    SAFE_DELETE_ARRAY( pData );

    SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_SETCURSEL, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumServiceProviders()
// Desc: Fills the combobox with service providers
//-----------------------------------------------------------------------------
HRESULT EnumServiceProviders( HWND hDlg )
{
    DPN_SERVICE_PROVIDER_INFO* pdnSPInfo = NULL;
    HRESULT hr;
    DWORD   dwItems = 0;
    DWORD   dwSize  = 0;
    int     nIndex;

    // Enumerate all DirectPlay service providers, and store them in the listbox
    hr = g_pDP->EnumServiceProviders( NULL, NULL, pdnSPInfo, &dwSize,
                                      &dwItems, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL )
        return DXTRACE_ERR( TEXT("EnumServiceProviders"), hr );

    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];
    if( FAILED( hr = g_pDP->EnumServiceProviders( NULL, NULL, pdnSPInfo,
                                                  &dwSize, &dwItems, 0 ) ) )
        return DXTRACE_ERR( TEXT("EnumServiceProviders"), hr );

    DPN_SERVICE_PROVIDER_INFO* pdnSPInfoEnum = pdnSPInfo;
    for ( DWORD i = 0; i < dwItems; i++ )
    {
        TCHAR strName[MAX_PATH];
        DXUtil_ConvertWideStringToGeneric( strName, pdnSPInfoEnum->pwszName );

        // Found a service provider, so put it in the listbox
        nIndex = (int)SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_ADDSTRING, 
                                              0, (LPARAM) strName );

        // Store pointer to GUID in listbox
        GUID* pGuid = new GUID;
        memcpy( pGuid, &pdnSPInfoEnum->guid, sizeof(GUID) );
        SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_SETITEMDATA, 
                            nIndex, (LPARAM) pGuid );

        pdnSPInfoEnum++;
    }

    SAFE_DELETE_ARRAY( pdnSPInfo );

    // Try to select the default preferred provider
    nIndex = (int)SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                      (LPARAM)g_strPreferredProvider );
    if( nIndex != LB_ERR )
        SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_SETCURSEL, nIndex, 0 );
    else
        SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_SETCURSEL, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumAdapters()
// Desc: Fills the combobox with adapters for a specified SP
//-----------------------------------------------------------------------------
HRESULT EnumAdapters( HWND hDlg, GUID* pSPGuid )
{
    DPN_SERVICE_PROVIDER_INFO* pdnSPInfo = NULL;
    TCHAR   strName[MAX_PATH];
    HRESULT hr;
    DWORD   dwItems = 0;
    DWORD   dwSize  = 0;
    int     nIndex;
    int     nAllAdaptersIndex = 0;

    SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_RESETCONTENT, 0, 0 );

    // Enumerate all DirectPlay service providers, and store them in the listbox
    hr = g_pDP->EnumServiceProviders( pSPGuid, NULL, pdnSPInfo, &dwSize,
                                      &dwItems, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL )
        return DXTRACE_ERR( TEXT("EnumServiceProviders"), hr );

    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];
    if( FAILED( hr = g_pDP->EnumServiceProviders( pSPGuid, NULL, pdnSPInfo,
                                                  &dwSize, &dwItems, 0 ) ) )
        return DXTRACE_ERR( TEXT("EnumServiceProviders"), hr );

    DPN_SERVICE_PROVIDER_INFO* pdnSPInfoEnum = pdnSPInfo;
    for ( DWORD i = 0; i < dwItems; i++ )
    {
        DXUtil_ConvertWideStringToGeneric( strName, pdnSPInfoEnum->pwszName );

        // Found a service provider, so put it in the listbox
        nIndex = (int)SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_ADDSTRING, 
                                          0, (LPARAM) strName );

        if( _tcscmp( strName, TEXT("All Adapters") ) == 0 )
            nAllAdaptersIndex = nIndex;

        // Store pointer to GUID in listbox
        GUID* pGuid = new GUID;
        memcpy( pGuid, &pdnSPInfoEnum->guid, sizeof(GUID) );

        SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_SETITEMDATA, 
                            nIndex, (LPARAM) pGuid );

        pdnSPInfoEnum++;
    }

    SAFE_DELETE_ARRAY( pdnSPInfo );

    SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_SETCURSEL, nAllAdaptersIndex, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LaunchApp
// Desc: Launch the selected app based on the UI settings
//-----------------------------------------------------------------------------
HRESULT LaunchApp( HWND hDlg )
{
    HRESULT   hr;
    DPNHANDLE hApplication = NULL;

    BOOL bLaunchNew      = IsDlgButtonChecked( hDlg, IDC_LAUNCH_NEW );
    BOOL bLaunchNotFound = IsDlgButtonChecked( hDlg, IDC_LAUNCH_NOT_FOUND );

    int nAppIndex = (int) SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_GETCURSEL, 0, 0 );
    GUID* pAppGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_APP_LIST, LB_GETITEMDATA, nAppIndex, 0 );
    if( nAppIndex == LB_ERR || pAppGuid == NULL )
        return E_INVALIDARG;

    // Setup the DPL_CONNECT_INFO struct
    DPL_CONNECT_INFO dnConnectInfo;
    ZeroMemory( &dnConnectInfo, sizeof(DPL_CONNECT_INFO) );
    dnConnectInfo.dwSize = sizeof(DPL_CONNECT_INFO);
    dnConnectInfo.pvLobbyConnectData = NULL;
    dnConnectInfo.dwLobbyConnectDataSize = 0;
    dnConnectInfo.dwFlags = 0;
    if( bLaunchNew )
        dnConnectInfo.dwFlags |= DPLCONNECT_LAUNCHNEW;
    if( bLaunchNotFound )
        dnConnectInfo.dwFlags |= DPLCONNECT_LAUNCHNOTFOUND;
    dnConnectInfo.guidApplication = *pAppGuid;
    if( FAILED( hr = AllocAndInitConnectSettings( hDlg, pAppGuid, &dnConnectInfo.pdplConnectionSettings ) ) ) 
        return S_FALSE;

    hr = g_pLobbyClient->ConnectApplication( &dnConnectInfo, NULL, &hApplication, 
                                             INFINITE, 0 );
    if( FAILED(hr) )
    {
        if( hr == DPNERR_NOCONNECTION && !bLaunchNew && !bLaunchNotFound )
        {
            MessageBox( NULL, TEXT("There was no waiting application. "),
                        g_strAppName, MB_OK | MB_ICONERROR );
        }
        else
        {
            return DXTRACE_ERR( TEXT("ConnectApplication"), hr );
        }
    }
    else
    {
        TCHAR strBuffer[20];
        wsprintf( strBuffer, TEXT("0x%x"), hApplication );
        int nIndex = (int) SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_ADDSTRING, 
                                         0, (LPARAM) strBuffer );
        SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_SETITEMDATA, 
                            nIndex, (LPARAM) hApplication );

        if( LB_ERR == SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_GETCURSEL, 0, 0 ) )
            SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_SETCURSEL, 0, 0 );
    }

    FreeConnectSettings( dnConnectInfo.pdplConnectionSettings );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: AllocAndInitConnectSettings
// Desc: Alloc and fill up a DPL_CONNECTION_SETTINGS.  Call FreeConnectSettings
//       later to free it.
//-----------------------------------------------------------------------------
HRESULT AllocAndInitConnectSettings( HWND hDlg, GUID* pAppGuid, 
                                     DPL_CONNECTION_SETTINGS** ppdplConnectSettings ) 
{
    HRESULT hr;

    BOOL bHosting    = IsDlgButtonChecked( hDlg, IDC_HOST_SESSION );
    BOOL bNoSettings = IsDlgButtonChecked( hDlg, IDC_NO_SETTINGS );
    IDirectPlay8Address* pHostAddress   = NULL;
    IDirectPlay8Address* pDeviceAddress = NULL;

    if( bNoSettings )
    {
        *ppdplConnectSettings = NULL;
        return S_OK;
    }

    int nSPIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
    GUID* pSPGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, nSPIndex, 0 );

    if( *pSPGuid == CLSID_DP8SP_IPX && !bHosting )
    {
        MessageBox( hDlg, TEXT("A typical lobby client would have sent a complete IPX address, this ") \
                          TEXT("lobby client does not support this. "), g_strAppName, MB_OK );
        return E_NOTIMPL;
    }

    if( !bHosting )
    {
        // Create a host address if connecting to a host, 
        // otherwise keep it as NULL
        if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, 
                                           IID_IDirectPlay8Address, (void **) &pHostAddress ) ) )
            return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

        // Set the SP to pHostAddress
        if( FAILED( hr = pHostAddress->SetSP( pSPGuid ) ) )
            return DXTRACE_ERR( TEXT("SetSP"), hr );
    }

    // Create a device address to specify which device we are using 
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, 
                                       IID_IDirectPlay8Address, (void **) &pDeviceAddress ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Set the SP to pDeviceAddress
    if( FAILED( hr = pDeviceAddress->SetSP( pSPGuid ) ) )
        return DXTRACE_ERR( TEXT("SetSP"), hr );

    // Add the adapter to pHostAddress
    int nAdapterIndex = (int) SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_GETCURSEL, 0, 0 );
    GUID* pAdapterGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_GETITEMDATA, 
                                                     nAdapterIndex, 0 );
    if( FAILED( hr = pDeviceAddress->SetDevice( pAdapterGuid ) ) )
        return DXTRACE_ERR( TEXT("SetDevice"), hr );

    if( *pSPGuid == CLSID_DP8SP_TCPIP )
    {
        TCHAR strIP[MAX_PATH];
        TCHAR strPort[MAX_PATH];

        GetDlgItemText( hDlg, IDC_ADDRESS_LINE1, strIP, MAX_PATH );
        GetDlgItemText( hDlg, IDC_ADDRESS_LINE2, strPort, MAX_PATH );

        if( bHosting )
        {
            // Add the port to pDeviceAddress
            DWORD dwPort = _ttoi( strPort );
            if( FAILED( hr = pDeviceAddress->AddComponent( DPNA_KEY_PORT, 
                                                           &dwPort, sizeof(dwPort),
                                                           DPNA_DATATYPE_DWORD ) ) )
                return DXTRACE_ERR( TEXT("AddComponent"), hr );
        }
        else
        {
            // Add the IP address to pHostAddress
            if( _tcslen( strIP ) > 0 )
            {
                WCHAR wstrIP[MAX_PATH];
                DXUtil_ConvertGenericStringToWide( wstrIP, strIP );

                if( FAILED( hr = pHostAddress->AddComponent( DPNA_KEY_HOSTNAME, 
                                                             wstrIP, (wcslen(wstrIP)+1)*sizeof(WCHAR), 
                                                             DPNA_DATATYPE_STRING ) ) )
                    return DXTRACE_ERR( TEXT("AddComponent"), hr );
            }

            // Add the port to pHostAddress
            DWORD dwPort = _ttoi( strPort );
            if( FAILED( hr = pHostAddress->AddComponent( DPNA_KEY_PORT, 
                                                         &dwPort, sizeof(dwPort),
                                                         DPNA_DATATYPE_DWORD ) ) )
                return DXTRACE_ERR( TEXT("AddComponent"), hr );
        }
    }
    else if( *pSPGuid == CLSID_DP8SP_MODEM )
    {
        TCHAR strPhone[MAX_PATH];
        GetDlgItemText( hDlg, IDC_ADDRESS_LINE1, strPhone, MAX_PATH );

        if( !bHosting )
        {
            // Add the phonenumber to pHostAddress
            if( _tcslen( strPhone ) > 0 )
            {
                WCHAR wstrPhone[MAX_PATH];
                DXUtil_ConvertGenericStringToWide( wstrPhone, strPhone );

                if( FAILED( hr = pHostAddress->AddComponent( DPNA_KEY_PHONENUMBER, 
                                                             wstrPhone, (wcslen(wstrPhone)+1)*sizeof(WCHAR), 
                                                             DPNA_DATATYPE_STRING ) ) )
                    return DXTRACE_ERR( TEXT("AddComponent"), hr );
            }
        }
    }
    else if( *pSPGuid == CLSID_DP8SP_SERIAL )
    {
        // This simple client doesn't have UI to query for the various
        // fields needed for the serial.  So we just let DPlay popup a dialog
        // to ask the user which settings are needed.
    }
    else
    {
        // Unknown SP, so leave as is
    }

    // Setup the DPL_CONNECTION_SETTINGS
    DPL_CONNECTION_SETTINGS* pSettings = new DPL_CONNECTION_SETTINGS;
    ZeroMemory( pSettings, sizeof(DPL_CONNECTION_SETTINGS) );
    pSettings->dwSize = sizeof(DPL_CONNECTION_SETTINGS);
    pSettings->dpnAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
    pSettings->dwFlags = 0;
    if( bHosting )
        pSettings->dwFlags |= DPLCONNECTSETTINGS_HOST;
    pSettings->dpnAppDesc.guidApplication = *pAppGuid;
    pSettings->dpnAppDesc.guidInstance    = GUID_NULL;
    pSettings->pdp8HostAddress = pHostAddress;
    pSettings->ppdp8DeviceAddresses = new IDirectPlay8Address*;
    pSettings->ppdp8DeviceAddresses[0] = pDeviceAddress;
    pSettings->cNumDeviceAddresses = 1;

    // Set the pSettings->dpnAppDesc.pwszSessionName
    TCHAR strSessionName[MAX_PATH];
    GetDlgItemText( hDlg, IDC_SESSION_NAME, strSessionName, MAX_PATH );
    if( _tcslen( strSessionName ) == 0 )
    {
        pSettings->dpnAppDesc.pwszSessionName = NULL;
    }
    else
    {
        WCHAR wstrSessionName[MAX_PATH];
        DXUtil_ConvertGenericStringToWide( wstrSessionName, strSessionName );
        pSettings->dpnAppDesc.pwszSessionName = new WCHAR[wcslen(wstrSessionName)+1];
        wcscpy( pSettings->dpnAppDesc.pwszSessionName, wstrSessionName );
    }

    // Set the pSettings->pwszPlayerName
    TCHAR strPlayerName[MAX_PATH];
    GetDlgItemText( hDlg, IDC_PLAYER_NAME, strPlayerName, MAX_PATH );
    if( _tcslen( strPlayerName ) == 0 )
    {
        pSettings->pwszPlayerName = NULL;
    }
    else
    {
        WCHAR wstrPlayerName[MAX_PATH];
        DXUtil_ConvertGenericStringToWide( wstrPlayerName, strPlayerName );
        pSettings->pwszPlayerName = new WCHAR[wcslen(wstrPlayerName)+1];
        wcscpy( pSettings->pwszPlayerName, wstrPlayerName );
    }

    *ppdplConnectSettings = pSettings;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeConnectSettings
// Desc: Releases everything involved in a DPL_CONNECTION_SETTINGS struct
//-----------------------------------------------------------------------------
VOID FreeConnectSettings( DPL_CONNECTION_SETTINGS* pSettings )
{
    if( !pSettings )
        return;

    SAFE_DELETE_ARRAY( pSettings->pwszPlayerName ); 
    SAFE_DELETE_ARRAY( pSettings->dpnAppDesc.pwszSessionName );
    SAFE_DELETE_ARRAY( pSettings->dpnAppDesc.pwszPassword );
    SAFE_DELETE_ARRAY( pSettings->dpnAppDesc.pvReservedData );
    SAFE_DELETE_ARRAY( pSettings->dpnAppDesc.pvApplicationReservedData );
    SAFE_RELEASE( pSettings->pdp8HostAddress );
    SAFE_RELEASE( pSettings->ppdp8DeviceAddresses[0] );
    SAFE_DELETE_ARRAY( pSettings->ppdp8DeviceAddresses );
    SAFE_DELETE( pSettings );
}




//-----------------------------------------------------------------------------
// Name: DirectPlayMessageHandler
// Desc: Handler for DirectPlay messages.  This function is called by
//       the DirectPlay message handler pool of threads, so be care of thread
//       synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, 
                                         DWORD dwMessageId, 
                                         PVOID pMsgBuffer )
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DirectPlayLobbyMessageHandler
// Desc: Handler for DirectPlay lobby messages.  This function is called by
//       the DirectPlay lobby message handler pool of threads, so be careful of 
//       thread synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayLobbyMessageHandler( PVOID pvUserContext, 
                                              DWORD dwMessageId, 
                                              PVOID pMsgBuffer )
{
    switch( dwMessageId )
    {
        case DPL_MSGID_DISCONNECT:
        {
            PDPL_MESSAGE_DISCONNECT pDisconnectMsg;
            pDisconnectMsg = (PDPL_MESSAGE_DISCONNECT)pMsgBuffer;

            // We should free any data associated with the 
            // app here, but there is none.

            // Tell the update the UI so show that a application was disconnected
            // Note: The lobby handler must not become blocked on the dialog thread, 
            // since the dialog thread will be blocked on lobby handler thread
            // when it calls ConnectApplication().  So to avoid blocking on
            // the dialog thread, we'll post a message to the dialog thread.
            PostMessage( g_hDlg, WM_APP_APPDISCONNECTED, 0, pDisconnectMsg->hDisconnectId );            
            break;
        }

        case DPL_MSGID_RECEIVE:
        {
            PDPL_MESSAGE_RECEIVE pReceiveMsg;
            pReceiveMsg = (PDPL_MESSAGE_RECEIVE)pMsgBuffer;

            // The lobby app sent us data.  This sample doesn't
            // expected data from the app, but it is useful 
            // for more complex lclients.
            break;
        }

        case DPL_MSGID_SESSION_STATUS:
        {
            PDPL_MESSAGE_SESSION_STATUS pStatusMsg;
            pStatusMsg = (PDPL_MESSAGE_SESSION_STATUS)pMsgBuffer;

            TCHAR* strBuffer = new TCHAR[200];
            wsprintf( strBuffer, TEXT("0x%x: "), pStatusMsg->hSender );
            switch( pStatusMsg->dwStatus )
            {
                case DPLSESSION_CONNECTED:
                    _tcscat( strBuffer, TEXT("Session connected") ); break;
                case DPLSESSION_COULDNOTCONNECT:
                    _tcscat( strBuffer, TEXT("Session could not connect") ); break;
                case DPLSESSION_DISCONNECTED:
                    _tcscat( strBuffer, TEXT("Session disconnected") ); break;
                case DPLSESSION_TERMINATED:
                    _tcscat( strBuffer, TEXT("Session terminated") ); break;
                case DPLSESSION_HOSTMIGRATED:
                	_tcscat( strBuffer, TEXT("Host migrated") ); break;
                case DPLSESSION_HOSTMIGRATEDHERE:
                	_tcscat( strBuffer, TEXT("Host migrated to this client") ); break;

                default:
                {
                    TCHAR strStatus[30];
                    wsprintf( strStatus, TEXT("%d"), pStatusMsg->dwStatus );
                    _tcscat( strBuffer, strStatus );
                    break;
                }
            }

            // Tell the update the UI so show that a application status changed
            // Note: The lobby handler must not become blocked on the dialog thread, 
            // since the dialog thread will be blocked on lobby handler thread
            // when it calls ConnectApplication().  So to avoid blocking on
            // the dialog thread, we'll post a message to the dialog thread.
            PostMessage( g_hDlg, WM_APP_SETSTATUS, 0, (LPARAM) strBuffer );            
            break;
        }

        case DPL_MSGID_CONNECTION_SETTINGS:
        {
            PDPL_MESSAGE_CONNECTION_SETTINGS pConnectionStatusMsg;
            pConnectionStatusMsg = (PDPL_MESSAGE_CONNECTION_SETTINGS)pMsgBuffer;

            // The app has changed the connection settings.  
            // This simple client doesn't handle this, but more complex clients may
            // want to.
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SendMsgToApp
// Desc: Send a dummy message to a connected app for demo purposes 
//-----------------------------------------------------------------------------
HRESULT SendMsgToApp( HWND hDlg )
{
    TCHAR strBuffer[MAX_PATH];
    HRESULT hr;

    int nConnectIndex = (int) SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_GETCURSEL, 0, 0 );
    if( nConnectIndex == LB_ERR )
        return S_OK;

    DPNHANDLE hApplication = (DPNHANDLE) SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, 
                                                             LB_GETITEMDATA, nConnectIndex, 0 );

    // For demonstration purposes, just send a buffer to the app.  This can be used
    // by more complex lobby clients to pass custom information to apps that which
    // can then recieve and process it.
    BYTE buffer[20];
    memset( buffer, 0x03, 20 );
    if( FAILED( hr = g_pLobbyClient->Send( hApplication, buffer, 20, 0 ) ) )
    {
        DXTRACE_ERR( TEXT("Send"), hr );
        wsprintf( strBuffer, TEXT("Failure trying to send message to 0x%0.8x."), hApplication );
        MessageBox( NULL, strBuffer, g_strAppName, MB_OK | MB_ICONERROR );
    }
    else
    {
        wsprintf( strBuffer, TEXT("Successfully sent a message to 0x%0.8x."), hApplication );
        MessageBox( NULL, strBuffer, g_strAppName, MB_OK | MB_ICONERROR );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DisconnectFromApp
// Desc: Disconnect from an app
//-----------------------------------------------------------------------------
HRESULT DisconnectFromApp( HWND hDlg )
{
    HRESULT hr;

    int nConnectIndex = (int) SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_GETCURSEL, 0, 0 );
    if( nConnectIndex == LB_ERR )
        return S_OK;

    DPNHANDLE hApplication = (DPNHANDLE) SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, 
                                                             LB_GETITEMDATA, nConnectIndex, 0 );

    if( FAILED( hr = g_pLobbyClient->ReleaseApplication( hApplication, 0 ) ) )
    {
        DXTRACE_ERR( TEXT("LaunchApp"), hr );
        MessageBox( NULL, TEXT("Failure trying to disconnect from app. "), 
                    g_strAppName, MB_OK | MB_ICONERROR );
    }

    SendDlgItemMessage( hDlg, IDC_ACTIVE_CONNECTIONS, LB_DELETESTRING, nConnectIndex, 0 );

    return S_OK;
}

