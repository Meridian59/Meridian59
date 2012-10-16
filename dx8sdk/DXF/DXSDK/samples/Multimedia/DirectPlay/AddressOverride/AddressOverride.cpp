//----------------------------------------------------------------------------
// File: AddressOverride.cpp
//
// Desc: The main game file for the AddressOverride sample.  AddressOverride
//       shows how to override the DirectPlay addressing in order to host or 
//       connect to another session on the network.
// 
//       After a new game has started the sample begins a very simplistic 
//       game called "The Greeting Game".  When two or more players are connected
//       to the game, the players have the option of sending a single simple 
//       DirectPlay message to all of the other players. When this message
//       is receieved by the other players, they simply display a dialog box.
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
#include <tchar.h>
#include <cguid.h>
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Player context locking defines
//-----------------------------------------------------------------------------
CRITICAL_SECTION g_csPlayerContext;
#define PLAYER_LOCK()                   EnterCriticalSection( &g_csPlayerContext ); 
#define PLAYER_ADDREF( pPlayerInfo )    if( pPlayerInfo ) pPlayerInfo->lRefCount++;
#define PLAYER_RELEASE( pPlayerInfo )   if( pPlayerInfo ) { pPlayerInfo->lRefCount--; if( pPlayerInfo->lRefCount <= 0 ) SAFE_DELETE( pPlayerInfo ); } pPlayerInfo = NULL;
#define PLAYER_UNLOCK()                 LeaveCriticalSection( &g_csPlayerContext );


//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#define DPLAY_SAMPLE_KEY                TEXT("Software\\Microsoft\\DirectX DirectPlay Samples")
#define MAX_PLAYER_NAME                 14
#define WM_APP_UPDATE_STATS             (WM_APP + 0)
#define WM_APP_DISPLAY_WAVE             (WM_APP + 1)
#define TIMER_WAIT_HOSTS_RESPONSE       (1)

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {02AE835D-9179-485f-8343-901D327CE794}
GUID g_guidApp = { 0x2ae835d, 0x9179, 0x485f, { 0x83, 0x43, 0x90, 0x1d, 0x32, 0x7c, 0xe7, 0x94 } };

struct APP_PLAYER_INFO
{
    LONG  lRefCount;                        // Ref count so we can cleanup when all threads 
                                            // are done w/ this object
    DPNID dpnidPlayer;                      // DPNID of player
    TCHAR strPlayerName[MAX_PLAYER_NAME];   // Player name
};




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Peer*       g_pDP                         = NULL;    // DirectPlay peer object
HINSTANCE               g_hInst                       = NULL;    // HINST of app
HWND                    g_hDlg                        = NULL;    // HWND of main dialog
DPNID                   g_dpnidLocalPlayer            = 0;       // DPNID of local player
LONG                    g_lNumberOfActivePlayers      = 0;       // Number of players currently in game
TCHAR                   g_strAppName[256]             = TEXT("AddressOverride");
HRESULT                 g_hrDialog;                              // Exit code for app 
TCHAR                   g_strLocalPlayerName[MAX_PATH];          // Local player name
TCHAR                   g_strSessionName[MAX_PATH];              // Session name
TCHAR                   g_strPreferredProvider[MAX_PATH];        // Provider string

BOOL                    g_bHostPlayer                 = FALSE;   // TRUE if local player is host
GUID*                   g_pCurSPGuid                  = NULL;    // Currently selected guid
TCHAR                   g_strLocalIP[100];                       // Local IP address
HANDLE                  g_hConnectCompleteEvent       = NULL;    // Event signaled when connection complete
HRESULT                 g_hrConnectComplete           = S_OK;    // Status of connection when it completes
HANDLE                  g_hEnumHostEvent              = NULL;    // Event signaled when the first session is enum
DPNHANDLE               g_hEnumAsyncOp                = NULL;    // Async handle for enuming hosts
DPN_APPLICATION_DESC*   g_pEnumedSessionAppDesc       = NULL;    // App desc of first session enumed
IDirectPlay8Address*    g_pEnumedSessionHostAddr      = NULL;    // Host addr of first session enumed
IDirectPlay8Address*    g_pEnumedSessionDeviceAddr    = NULL;    // Address of device to use


//-----------------------------------------------------------------------------
// App specific DirectPlay messages and structures 
//-----------------------------------------------------------------------------
#define GAME_MSGID_WAVE        1

// Change compiler pack alignment to be BYTE aligned, and pop the current value
#pragma pack( push, 1 )

struct GAMEMSG_GENERIC
{
    DWORD dwType;
};

// Pop the old pack alignment
#pragma pack( pop )



//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI   DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK OverrideDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK GreetingDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT          InitDirectPlay();
HRESULT          OnInitOverrideDialog( HWND hDlg );
VOID             SetupAddressFields( HWND hDlg );
HRESULT          EnumServiceProviders( HWND hDlg );
HRESULT          EnumAdapters( HWND hDlg, GUID* pSPGuid );
HRESULT          LaunchMultiplayerGame( HWND hDlg );
HRESULT          WaveToAllPlayers();
VOID             AppendTextToEditControl( HWND hDlg, TCHAR* strNewLogLine );




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
    BOOL    bConnectSuccess = FALSE;

    g_hInst = hInst; 
    InitializeCriticalSection( &g_csPlayerContext );
    g_hConnectCompleteEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); 
    g_hEnumHostEvent = CreateEvent( NULL, TRUE, FALSE, NULL ); 

    // Read persistent state information from registry
    RegCreateKeyEx( HKEY_CURRENT_USER, DPLAY_SAMPLE_KEY, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    &hDPlaySampleRegKey, NULL );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), 
                             g_strLocalPlayerName, MAX_PATH, TEXT("TestPlayer") );
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

    // Connect or host a DirectPlay session.  Pop UI to query 
    // for addressing so that DirectPlay's default dialogs are overridden
    g_hrDialog = S_OK;
    DialogBox( g_hInst, MAKEINTRESOURCE(IDD_ADDRESS_OVERRIDE), NULL, 
               (DLGPROC) OverrideDlgProc );
    
    if( FAILED( g_hrDialog ) ) 
    {
        DXTRACE_ERR( TEXT("ConnectUsingOverrideDlg"), g_hrDialog );
        MessageBox( NULL, TEXT("Multiplayer connect failed. ")
                    TEXT("The sample will now quit."),
                    g_strAppName, MB_OK | MB_ICONERROR );
        bConnectSuccess = FALSE;
    } 
    else if( g_hrDialog == S_FALSE ) 
    {
        // The user canceled the connect dialog, so quit 
        bConnectSuccess = FALSE;
    }
    else
    {
        bConnectSuccess = TRUE; 

        // Write information to the registry
        DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), g_strLocalPlayerName );
        DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), g_strSessionName );
        DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), g_strPreferredProvider );
    }

    if( bConnectSuccess )
    {
        // App is now connected via DirectPlay, so start the game.  

        // For this sample, we just start a simple dialog box game.
        g_hrDialog = S_OK;
        DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN_GAME), NULL, 
                   (DLGPROC) GreetingDlgProc );

        if( FAILED( g_hrDialog ) )
        {
            if( g_hrDialog == DPNERR_CONNECTIONLOST )
            {
                MessageBox( NULL, TEXT("The DirectPlay session was lost. ")
                            TEXT("The sample will now quit."),
                            g_strAppName, MB_OK | MB_ICONERROR );
            }
            else
            {
                DXTRACE_ERR( TEXT("DialogBox"), g_hrDialog );
                MessageBox( NULL, TEXT("An error occured during the game. ")
                            TEXT("The sample will now quit."),
                            g_strAppName, MB_OK | MB_ICONERROR );
            }
        }
    }

    if( g_pDP )
    {
        g_pDP->Close(0);
        SAFE_RELEASE( g_pDP );
    }

    RegCloseKey( hDPlaySampleRegKey );
    DeleteCriticalSection( &g_csPlayerContext );
    CloseHandle( g_hEnumHostEvent );
    CloseHandle( g_hConnectCompleteEvent );
    CoUninitialize();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectPlay()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT InitDirectPlay()
{
    HRESULT hr;

    // Create IDirectPlay8Peer
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Peer, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Peer, 
                                       (LPVOID*) &g_pDP ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Init IDirectPlay8Peer
    if( FAILED( hr = g_pDP->Initialize( NULL, DirectPlayMessageHandler, 0 ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OverrideDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK OverrideDlgProc( HWND hDlg, UINT msg, 
                                  WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
        case WM_INITDIALOG:
        {
            g_hDlg = hDlg;
            if( FAILED( hr = OnInitOverrideDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), hr );
                MessageBox( NULL, TEXT("Failed initializing dialog box. ")
                            TEXT("The sample will now quit."),
                            g_strAppName, MB_OK | MB_ICONERROR );
                EndDialog( hDlg, 0 );
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

                case IDOK:
                    if( FAILED( g_hrDialog = LaunchMultiplayerGame( hDlg ) ) )
                    {
                        if( g_hrDialog == DPNERR_ADDRESSING )
                        {
                            // This will be returned if the ip address is invalid
                            MessageBox( hDlg, TEXT("IP address not valid."),
                                        g_strAppName, MB_OK );
                        }
                        else if( g_hrDialog == DPNERR_INVALIDDEVICEADDRESS )
                        {
                            // This will be returned if the user canceled the dialog
                            MessageBox( hDlg, TEXT("User cancelled the DirectPlay dialog."),
                                        g_strAppName, MB_OK );
                        }
                        else
                        {
                            DXTRACE_ERR( TEXT("LaunchMultiplayerGame"), g_hrDialog );
                            MessageBox( NULL, TEXT("Failed to launch game. "),
                                        g_strAppName, MB_OK | MB_ICONERROR );                        
                        }
                    }
                    break;

                case IDCANCEL:
                    g_hrDialog = S_FALSE;
                    EndDialog( hDlg, 0 );
                    break;
            }
            break;
        }

        case WM_TIMER:
            if( wParam == TIMER_WAIT_HOSTS_RESPONSE )
            {
                DWORD dwResult;
                
                dwResult = WaitForSingleObject( g_hEnumHostEvent, 0 );
                if( dwResult == WAIT_OBJECT_0 )
                {
                    // Connect to host that was found. There should only be on device address in
                    // the connection settings structure when connecting to a session, so just 
                    // pass in the first one.  
                    // The enumeration is automatically cancelled after Connect is called 
                    DPNHANDLE hAsync;
                    hr = g_pDP->Connect( g_pEnumedSessionAppDesc,      // the application desc
                                         g_pEnumedSessionHostAddr,     // address of the host of the session
                                         g_pEnumedSessionDeviceAddr,   // address of the local device used to connect to the host
                                         NULL, NULL,                   // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                                         NULL, 0,                      // user data, user data size
                                         NULL,                         // player context,
                                         NULL, &hAsync,                // async context, async handle,
                                         0 );                          // flags
                    if( FAILED(hr) )
                        return DXTRACE_ERR( TEXT("Connect"), hr );

                    // Wait until the MessageHandler sets an event to tell us the 
                    // DPN_MSGID_CONNECT_COMPLETE has been processed.  Then 
                    // m_hrConnectComplete will be valid.
                    WaitForSingleObject( g_hConnectCompleteEvent, INFINITE );

                    if( FAILED( g_hrConnectComplete ) )
                    {
                        DXTRACE_ERR( TEXT("DPN_MSGID_CONNECT_COMPLETE"), g_hrConnectComplete );
                        MessageBox( hDlg, TEXT("Unable to join game."),
                                    g_strAppName, MB_OK | MB_ICONERROR );
                        hr = g_hrConnectComplete;
                    }

                    if( g_pEnumedSessionAppDesc )
                    {
                        SAFE_DELETE_ARRAY( g_pEnumedSessionAppDesc->pwszSessionName );
                        SAFE_DELETE_ARRAY( g_pEnumedSessionAppDesc );
                    }
                    SAFE_RELEASE( g_pEnumedSessionHostAddr );                    
                    SAFE_RELEASE( g_pEnumedSessionDeviceAddr );

                    EndDialog( g_hDlg, 0 );
                }
            }
            break;

        case WM_DESTROY:
        {
            GetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strLocalPlayerName, MAX_PATH );
            GetDlgItemText( hDlg, IDC_SESSION_NAME, g_strSessionName, MAX_PATH );
            int nIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
            SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETLBTEXT, nIndex, (LPARAM) g_strPreferredProvider );

            int nCount,i;
            nCount = (int)SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCOUNT, 0, 0 );
            for( i=0; i<nCount; i++ )
            {
                GUID* pGuid = (LPGUID) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, i, 0 );
                SAFE_DELETE( pGuid );
            }

            nCount = (int)SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_GETCOUNT, 0, 0 );
            for( i=0; i<nCount; i++ )
            {
                GUID* pGuid = (LPGUID) SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, 
                                                           CB_GETITEMDATA, i, 0 );
                SAFE_DELETE( pGuid );
            }            
            break;
        }
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: OnInitOverrideDialog
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnInitOverrideDialog( HWND hDlg )
{
    HRESULT hr;

    // Load and set the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    CheckDlgButton( hDlg, IDC_HOST_SESSION, BST_CHECKED );

    SetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strLocalPlayerName );
    SetDlgItemText( hDlg, IDC_SESSION_NAME, g_strSessionName );

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
    SetDlgItemText( hDlg, IDC_LOCAL_IP, g_strLocalIP );
    
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
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2, TEXT("") );
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
            SetDlgItemText( hDlg, IDC_ADDRESS_LINE1, TEXT("") );
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
    else if( *pGuid == CLSID_DP8SP_IPX )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE1, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE1_TEXT), FALSE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE1_TEXT, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2), TRUE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2, TEXT("") );
        EnableWindow( GetDlgItem(hDlg, IDC_ADDRESS_LINE2_TEXT), TRUE );
        SetDlgItemText( hDlg, IDC_ADDRESS_LINE2_TEXT, TEXT("Port:") );
    }
    else 
    {
        // CLSID_DP8SP_SERIAL or unknown so disable all the address lines 
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
    if( SUCCEEDED(hr) ) // No adapters found
        return S_OK;

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
// Name: LaunchMultiplayerGame
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LaunchMultiplayerGame( HWND hDlg ) 
{
    HRESULT hr;

    g_bHostPlayer = IsDlgButtonChecked( hDlg, IDC_HOST_SESSION );
    IDirectPlay8Address* pHostAddress     = NULL;
    IDirectPlay8Address* pDeviceAddress   = NULL;
    BOOL bOkToQuery = FALSE;

    int nSPIndex = (int) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETCURSEL, 0, 0 );
    GUID* pSPGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_SP_COMBO, CB_GETITEMDATA, nSPIndex, 0 );

    if( !g_bHostPlayer )
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
    GUID* pAdapterGuid = NULL;

    if( nAdapterIndex != CB_ERR )
    {
        GUID* pAdapterGuid = (GUID*) SendDlgItemMessage( hDlg, IDC_ADAPTER_COMBO, CB_GETITEMDATA, 
                                                         nAdapterIndex, 0 );
        if( FAILED( hr = pDeviceAddress->SetDevice( pAdapterGuid ) ) )
            return DXTRACE_ERR( TEXT("SetDevice"), hr );
    }

    if( *pSPGuid == CLSID_DP8SP_TCPIP )
    {
        TCHAR strIP[MAX_PATH];
        TCHAR strPort[MAX_PATH];

        GetDlgItemText( hDlg, IDC_ADDRESS_LINE1, strIP, MAX_PATH );
        GetDlgItemText( hDlg, IDC_ADDRESS_LINE2, strPort, MAX_PATH );

        if( g_bHostPlayer )
        {
            if( _tcslen( strPort ) > 0 )
            {
                // Add the port to pDeviceAddress
                DWORD dwPort = _ttoi( strPort );
                if( FAILED( hr = pDeviceAddress->AddComponent( DPNA_KEY_PORT, 
                                                               &dwPort, sizeof(dwPort),
                                                               DPNA_DATATYPE_DWORD ) ) )
                    return DXTRACE_ERR( TEXT("AddComponent"), hr );
            }
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

            if( _tcslen( strPort ) > 0 )
            {
                // Add the port to pHostAddress
                DWORD dwPort = _ttoi( strPort );
                if( FAILED( hr = pHostAddress->AddComponent( DPNA_KEY_PORT, 
                                                             &dwPort, sizeof(dwPort),
                                                             DPNA_DATATYPE_DWORD ) ) )
                    return DXTRACE_ERR( TEXT("AddComponent"), hr );
            }
        }
    }
    else if( *pSPGuid == CLSID_DP8SP_IPX )
    {
        TCHAR strPort[MAX_PATH];
        GetDlgItemText( hDlg, IDC_ADDRESS_LINE2, strPort, MAX_PATH );

        if( g_bHostPlayer )
        {
            if( _tcslen( strPort ) > 0 )
            {
                // Add the port to pDeviceAddress
                DWORD dwPort = _ttoi( strPort );
                if( FAILED( hr = pDeviceAddress->AddComponent( DPNA_KEY_PORT, 
                                                               &dwPort, sizeof(dwPort),
                                                               DPNA_DATATYPE_DWORD ) ) )
                    return DXTRACE_ERR( TEXT("AddComponent"), hr );
            }
        }
        else
        {
            if( _tcslen( strPort ) > 0 )
            {
                // Add the port to pHostAddress
                DWORD dwPort = _ttoi( strPort );
                if( FAILED( hr = pHostAddress->AddComponent( DPNA_KEY_PORT, 
                                                             &dwPort, sizeof(dwPort),
                                                             DPNA_DATATYPE_DWORD ) ) )
                    return DXTRACE_ERR( TEXT("AddComponent"), hr );
            }
        }
    }
    else if( *pSPGuid == CLSID_DP8SP_MODEM )
    {
        TCHAR strPhone[MAX_PATH];
        GetDlgItemText( hDlg, IDC_ADDRESS_LINE1, strPhone, MAX_PATH );

        if( !g_bHostPlayer )
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
        bOkToQuery = TRUE;
    }
    else
    {
        // Unknown SP, so leave as is
        bOkToQuery = TRUE;
    }

    DPN_APPLICATION_DESC dpnAppDesc;
    ZeroMemory( &dpnAppDesc, sizeof(DPN_APPLICATION_DESC) );
    dpnAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
    dpnAppDesc.guidApplication = g_guidApp;
    dpnAppDesc.guidInstance    = GUID_NULL;

    // Set the peer info
    WCHAR wszPeerName[MAX_PATH];
    DXUtil_ConvertGenericStringToWide( wszPeerName, g_strLocalPlayerName );
    DPN_PLAYER_INFO dpPlayerInfo;
    ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
    dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
    dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
    dpPlayerInfo.pwszName = wszPeerName;

    // Set the peer info, and use the DPNOP_SYNC since by default this
    // is an async call.  If it is not DPNOP_SYNC, then the peer info may not
    // be set by the time we call Connect() below.
    if( FAILED( hr = g_pDP->SetPeerInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
        return DXTRACE_ERR( TEXT("SetPeerInfo"), hr );

    if( g_bHostPlayer )
    {
        // Set the dpnAppDesc.pwszSessionName
        TCHAR strSessionName[MAX_PATH];
        GetDlgItemText( hDlg, IDC_SESSION_NAME, strSessionName, MAX_PATH );
        if( _tcslen( strSessionName ) == 0 )
        {
            dpnAppDesc.pwszSessionName = NULL;
        }
        else
        {
            WCHAR wstrSessionName[MAX_PATH];
            DXUtil_ConvertGenericStringToWide( wstrSessionName, strSessionName );
            dpnAppDesc.pwszSessionName = new WCHAR[wcslen(wstrSessionName)+1];
            wcscpy( dpnAppDesc.pwszSessionName, wstrSessionName );
        }

        // Host a game as described by pSettings
        hr = g_pDP->Host( &dpnAppDesc,          // the application desc
                          &pDeviceAddress,    // array of addresses of the local devices used to connect to the host
                          1,                    // number in array
                          NULL, NULL,           // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                          NULL,                 // player context
                          bOkToQuery );                 // flags

        SAFE_DELETE_ARRAY( dpnAppDesc.pwszSessionName );
        SAFE_RELEASE( pDeviceAddress );

        if( FAILED(hr) )
        {
            SAFE_RELEASE( pHostAddress );
            SAFE_RELEASE( pDeviceAddress );

            if( hr == DPNERR_INVALIDDEVICEADDRESS )  // This will be returned if the user canceled the dplay query dlg
                return hr;                
            return DXTRACE_ERR( TEXT("Host"), hr );    
        }

        EndDialog( g_hDlg, 0 );
    }
    else
    {
        // Query for the enum host timeout for this SP
        DPN_SP_CAPS dpspCaps;
        ZeroMemory( &dpspCaps, sizeof(DPN_SP_CAPS) );
        dpspCaps.dwSize = sizeof(DPN_SP_CAPS);
        if( FAILED( hr = g_pDP->GetSPCaps( pSPGuid, &dpspCaps, 0 ) ) )
            return DXTRACE_ERR( TEXT("GetSPCaps"), hr );

        // Set the host expire time to around 3 times 
        // length of the dwDefaultEnumRetryInterval
        DWORD dwEnumHostTimeout = dpspCaps.dwDefaultEnumRetryInterval * 3;

        // Enumerate hosts
        DPN_APPLICATION_DESC    dnAppDesc;
        ZeroMemory( &dnAppDesc, sizeof(DPN_APPLICATION_DESC) );
        dnAppDesc.dwSize          = sizeof(DPN_APPLICATION_DESC);
        dnAppDesc.guidApplication = g_guidApp;

        // Enumerate all the active DirectPlay games on the selected connection
        hr = g_pDP->EnumHosts( &dnAppDesc,                            // application description
                               pHostAddress,                          // host address
                               pDeviceAddress,                        // device address
                               NULL,                                  // pointer to user data
                               0,                                     // user data size
                               0,                                     // retry count (0=default)
                               0,                                     // retry interval (0=default)
                               dwEnumHostTimeout,                     // time out (forever)
                               NULL,                                  // user context
                               &g_hEnumAsyncOp,                       // async handle
                               bOkToQuery                       	  // flags
						       );
        if( FAILED(hr) )
        {
            SAFE_RELEASE( pHostAddress );
            SAFE_RELEASE( pDeviceAddress );

            if( hr == DPNERR_ADDRESSING )  // This will be returned if the ip address is invalid
                return hr;                 // For example, something like "asdf" 
            if( hr == DPNERR_INVALIDDEVICEADDRESS )  // This will be returned if the user canceled the dplay query dlg
                return hr;                
            return DXTRACE_ERR( TEXT("EnumHosts"), hr );
        }

        SetTimer( g_hDlg, TIMER_WAIT_HOSTS_RESPONSE, 100, NULL );
    }

    // Cleanup the addresses
    SAFE_RELEASE( pHostAddress );
    SAFE_RELEASE( pDeviceAddress );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GreetingDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK GreetingDlgProc( HWND hDlg, UINT msg, 
                                  WPARAM wParam, LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
        {
            g_hDlg = hDlg;

            // Load and set the icon
            HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
            SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
            SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

            if( g_bHostPlayer )
                SetWindowText( hDlg, TEXT("AddressOverride (Host)") );
            else
                SetWindowText( hDlg, TEXT("AddressOverride") );

            // Display local player's name
            SetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strLocalPlayerName );

            PostMessage( hDlg, WM_APP_UPDATE_STATS, 0, 0 );
            break;
        }

        case WM_APP_UPDATE_STATS:
        {
            // Update the number of players in the game
            TCHAR strNumberPlayers[32];

            wsprintf( strNumberPlayers, TEXT("%d"), g_lNumberOfActivePlayers );
            SetDlgItemText( hDlg, IDC_NUM_PLAYERS, strNumberPlayers );
            break;
        }

        case WM_APP_DISPLAY_WAVE:
        {
            HRESULT          hr;
            DPNID            dpnidPlayer = (DWORD)wParam;
            APP_PLAYER_INFO* pPlayerInfo = NULL;
            
            PLAYER_LOCK(); // enter player context CS

            // Get the player context accosicated with this DPNID
            hr = g_pDP->GetPlayerContext( dpnidPlayer, 
                                          (LPVOID* const) &pPlayerInfo,
										  0);

            PLAYER_ADDREF( pPlayerInfo ); // addref player, since we are using it now
            PLAYER_UNLOCK(); // leave player context CS

            if( FAILED(hr) || pPlayerInfo == NULL )
            {
                // The player who sent this may have gone away before this 
                // message was handled, so just ignore it
                break;
            }
            
            // Make wave message and display it.
            TCHAR szWaveMessage[MAX_PATH];
            wsprintf( szWaveMessage, TEXT("%s just waved at you, %s!\r\n"), 
                      pPlayerInfo->strPlayerName, g_strLocalPlayerName );

            PLAYER_LOCK();
            PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
            PLAYER_UNLOCK();

            AppendTextToEditControl( hDlg, szWaveMessage );
            break;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDC_WAVE:
                    if( FAILED( g_hrDialog = WaveToAllPlayers() ) )
                    {
                        DXTRACE_ERR( TEXT("WaveToAllPlayers"), g_hrDialog );
                        EndDialog( hDlg, 0 );
                    }

                    return TRUE;

                case IDCANCEL:
                    g_hrDialog = S_OK;
                    EndDialog( hDlg, 0 );
                    return TRUE;
            }
            break;
        }
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: DirectPlayMessageHandler
// Desc: Handler for DirectPlay messages.  This function is called by
//       the DirectPlay message handler pool of threads, so be careful of thread
//       synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, 
                                         DWORD dwMessageId, 
                                         PVOID pMsgBuffer )
{
    // Try not to stay in this message handler for too long, otherwise
    // there will be a backlog of data.  The best solution is to 
    // queue data as it comes in, and then handle it on other threads.
    
    // This function is called by the DirectPlay message handler pool of 
    // threads, so be careful of thread synchronization problems with shared memory

    switch( dwMessageId )
    {
        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg;
            pEnumHostsResponseMsg = (PDPNMSG_ENUM_HOSTS_RESPONSE)pMsgBuffer;

            // This simple sample only records the 
            // first enum responce, and connects to that host
            if( NULL == g_pEnumedSessionHostAddr )
            {
                // Duplicate pEnumHostsResponseMsg->pAddressSender in g_pEnumedSessionHostAddr.
                if( pEnumHostsResponseMsg->pAddressSender )
                    pEnumHostsResponseMsg->pAddressSender->Duplicate( &g_pEnumedSessionHostAddr );

                // Duplicate pEnumHostsResponseMsg->pAddressDevice in g_pEnumedSessionDeviceAddr.
                if( pEnumHostsResponseMsg->pAddressDevice )
                    pEnumHostsResponseMsg->pAddressDevice->Duplicate( &g_pEnumedSessionDeviceAddr );

                // Copy pEnumHostsResponseMsg->pApplicationDescription to g_pEnumedSessionAppDesc
                g_pEnumedSessionAppDesc = new DPN_APPLICATION_DESC;
                ZeroMemory( g_pEnumedSessionAppDesc, sizeof(DPN_APPLICATION_DESC) );
                memcpy( g_pEnumedSessionAppDesc, pEnumHostsResponseMsg->pApplicationDescription,
                        sizeof(DPN_APPLICATION_DESC) );
                if( pEnumHostsResponseMsg->pApplicationDescription->pwszSessionName )
                {
                    g_pEnumedSessionAppDesc->pwszSessionName = new WCHAR[ wcslen(pEnumHostsResponseMsg->pApplicationDescription->pwszSessionName)+1 ];
                    wcscpy( g_pEnumedSessionAppDesc->pwszSessionName,
                            pEnumHostsResponseMsg->pApplicationDescription->pwszSessionName );
                }

                SetEvent( g_hEnumHostEvent );
            }
            break;
        }

        case DPN_MSGID_ASYNC_OP_COMPLETE:
        {
            PDPNMSG_ASYNC_OP_COMPLETE pAsyncOpCompleteMsg;
            pAsyncOpCompleteMsg = (PDPNMSG_ASYNC_OP_COMPLETE)pMsgBuffer;

            if( pAsyncOpCompleteMsg->hAsyncOp == g_hEnumAsyncOp )
            {
                if( WAIT_TIMEOUT == WaitForSingleObject( g_hEnumHostEvent, 0 ) )
                {
                    MessageBox( g_hDlg, TEXT("No session found."), g_strAppName, MB_OK );
                    SAFE_RELEASE( g_pEnumedSessionDeviceAddr );
                    KillTimer( g_hDlg, TIMER_WAIT_HOSTS_RESPONSE );
                    g_hEnumAsyncOp = NULL;
                }

                g_hEnumHostEvent = NULL;
            }
            break;
        }

        case DPN_MSGID_CREATE_PLAYER:
        {
            HRESULT hr;
            PDPNMSG_CREATE_PLAYER pCreatePlayerMsg;
            pCreatePlayerMsg = (PDPNMSG_CREATE_PLAYER)pMsgBuffer;

            // Create a new and fill in a APP_PLAYER_INFO
            APP_PLAYER_INFO* pPlayerInfo = new APP_PLAYER_INFO;
            ZeroMemory( pPlayerInfo, sizeof(APP_PLAYER_INFO) );
            pPlayerInfo->lRefCount   = 1;
            pPlayerInfo->dpnidPlayer = pCreatePlayerMsg->dpnidPlayer;

            // Get the peer info and extract its name
            DWORD dwSize = 0;
            DPN_PLAYER_INFO* pdpPlayerInfo = NULL;
            hr = DPNERR_CONNECTING;
            
            // GetPeerInfo might return DPNERR_CONNECTING when connecting, 
            // so just keep calling it if it does
            while( hr == DPNERR_CONNECTING ) 
                hr = g_pDP->GetPeerInfo( pCreatePlayerMsg->dpnidPlayer, pdpPlayerInfo, &dwSize, 0 );                                
                
            if( hr == DPNERR_BUFFERTOOSMALL )
            {
                pdpPlayerInfo = (DPN_PLAYER_INFO*) new BYTE[ dwSize ];
                ZeroMemory( pdpPlayerInfo, dwSize );
                pdpPlayerInfo->dwSize = sizeof(DPN_PLAYER_INFO);
                
                hr = g_pDP->GetPeerInfo( pCreatePlayerMsg->dpnidPlayer, pdpPlayerInfo, &dwSize, 0 );
                if( SUCCEEDED(hr) )
                {
                    // This stores a extra TCHAR copy of the player name for 
                    // easier access.  This will be redundent copy since DPlay 
                    // also keeps a copy of the player name in GetPeerInfo()
                    DXUtil_ConvertWideStringToGeneric( pPlayerInfo->strPlayerName, 
                                                       pdpPlayerInfo->pwszName, MAX_PLAYER_NAME );    
                                                       
                    if( pdpPlayerInfo->dwPlayerFlags & DPNPLAYER_LOCAL )
                        g_dpnidLocalPlayer = pCreatePlayerMsg->dpnidPlayer;
                }

                SAFE_DELETE_ARRAY( pdpPlayerInfo );
            }
                
            // Tell DirectPlay to store this pPlayerInfo 
            // pointer in the pvPlayerContext.
            pCreatePlayerMsg->pvPlayerContext = pPlayerInfo;

            // Update the number of active players, and 
            // post a message to the dialog thread to update the 
            // UI.  This keeps the DirectPlay message handler 
            // from blocking
            InterlockedIncrement( &g_lNumberOfActivePlayers );
            if( g_hDlg != NULL )
                PostMessage( g_hDlg, WM_APP_UPDATE_STATS, 0, 0 );

            break;
        }

        case DPN_MSGID_DESTROY_PLAYER:
        {
            PDPNMSG_DESTROY_PLAYER pDestroyPlayerMsg;
            pDestroyPlayerMsg = (PDPNMSG_DESTROY_PLAYER)pMsgBuffer;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pDestroyPlayerMsg->pvPlayerContext;

            PLAYER_LOCK();                  // enter player context CS
            PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
            PLAYER_UNLOCK();                // leave player context CS

            // Update the number of active players, and 
            // post a message to the dialog thread to update the 
            // UI.  This keeps the DirectPlay message handler 
            // from blocking
            InterlockedDecrement( &g_lNumberOfActivePlayers );
            if( g_hDlg != NULL )
                PostMessage( g_hDlg, WM_APP_UPDATE_STATS, 0, 0 );

            break;
        }

        case DPN_MSGID_HOST_MIGRATE:
        {
            PDPNMSG_HOST_MIGRATE pHostMigrateMsg;
            pHostMigrateMsg = (PDPNMSG_HOST_MIGRATE)pMsgBuffer;

            // Check to see if we are the new host
            if( pHostMigrateMsg->dpnidNewHost == g_dpnidLocalPlayer )
                SetWindowText( g_hDlg, TEXT("AddressOverride (Host)") );
            break;
        }

        case DPN_MSGID_TERMINATE_SESSION:
        {
            PDPNMSG_TERMINATE_SESSION pTerminateSessionMsg;
            pTerminateSessionMsg = (PDPNMSG_TERMINATE_SESSION)pMsgBuffer;

            g_hrDialog = DPNERR_CONNECTIONLOST;
            EndDialog( g_hDlg, 0 );
            break;
        }

        case DPN_MSGID_RECEIVE:
        {
            PDPNMSG_RECEIVE pReceiveMsg;
            pReceiveMsg = (PDPNMSG_RECEIVE)pMsgBuffer;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pReceiveMsg->pvPlayerContext;
            if( NULL == pPlayerInfo )
                break;

            GAMEMSG_GENERIC* pMsg = (GAMEMSG_GENERIC*) pReceiveMsg->pReceiveData;
            if( pMsg->dwType == GAME_MSGID_WAVE )
            {
                // This message is sent when a player has waved to us, so 
                // post a message to the dialog thread to update the UI.  
                // This keeps the DirectPlay threads from blocking, and also
                // serializes the recieves since DirectPlayMessageHandler can
                // be called simultaneously from a pool of DirectPlay threads.
                PostMessage( g_hDlg, WM_APP_DISPLAY_WAVE, pPlayerInfo->dpnidPlayer, 0 );
            }
            break;
        }

        case DPN_MSGID_CONNECT_COMPLETE:
        {
            PDPNMSG_CONNECT_COMPLETE pConnectCompleteMsg;
            pConnectCompleteMsg = (PDPNMSG_CONNECT_COMPLETE)pMsgBuffer;

            // Set m_hrConnectComplete, then set an event letting 
            // everyone know that the DPN_MSGID_CONNECT_COMPLETE msg
            // has been handled
            g_hrConnectComplete = pConnectCompleteMsg->hResultCode;
            SetEvent( g_hConnectCompleteEvent );
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: WaveToAllPlayers()
// Desc: Send a app-defined "wave" DirectPlay message to all connected players
//-----------------------------------------------------------------------------
HRESULT WaveToAllPlayers()
{
    // This is called by the dialog UI thread.  This will send a message to all
    // the players or inform the player that there is no one to wave at.
    if( g_lNumberOfActivePlayers == 1 )
    {
        MessageBox( NULL, TEXT("No one is around to wave at! :("), 
                    TEXT("AddressOverride"), MB_OK );
    }
    else
    {
        // Send a message to all of the players
        GAMEMSG_GENERIC msgWave;
        msgWave.dwType = GAME_MSGID_WAVE;

        DPN_BUFFER_DESC bufferDesc;
        bufferDesc.dwBufferSize = sizeof(GAMEMSG_GENERIC);
        bufferDesc.pBufferData  = (BYTE*) &msgWave;

        // DirectPlay will tell via the message handler 
        // if there are any severe errors, so ignore any errors 
        DPNHANDLE hAsync;
        g_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                       0, NULL, &hAsync, DPNSEND_NOLOOPBACK | DPNSEND_GUARANTEED );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: AppendTextToEditControl()
// Desc: Appends a string of text to the edit control
//-----------------------------------------------------------------------------
VOID AppendTextToEditControl( HWND hDlg, TCHAR* strNewLogLine )
{
    static TCHAR strText[1024*10];

    HWND hEdit = GetDlgItem( hDlg, IDC_LOG_EDIT );
    SendMessage( hEdit, WM_SETREDRAW, FALSE, 0 );
    GetWindowText( hEdit, strText, 1024*9 );

    _tcscat( strText, strNewLogLine );

    int nSecondLine = 0;
    if( SendMessage( hEdit, EM_GETLINECOUNT, 0, 0 ) > 9 )
        nSecondLine = (int)SendMessage( hEdit, EM_LINEINDEX, 1, 0 );

    SetWindowText( hEdit, &strText[nSecondLine] );

    SendMessage( hEdit, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect( hEdit, NULL, TRUE );
    UpdateWindow( hEdit );
}



