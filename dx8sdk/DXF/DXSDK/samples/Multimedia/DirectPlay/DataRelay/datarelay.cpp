//----------------------------------------------------------------------------
// File: DataRelay.cpp
//
// Desc: The main game file for the DataRelay sample.  It connects 
//       players together with two dialog boxes to prompt users on the 
//       connection settings to join or create a session. After the user 
//       connects to a sesssion, the sample displays a multiplayer stage. 
// 
//       This sample uses DirectPlay to process high volumes of incoming 
//       data on a seperate worker while avoiding copying the incoming packets
//       by using the ReturnBuffer() call.  It also shows some basics of 
//       thread synconization between the worker thread and the DirectPlay
//       message handler thread pool.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <process.h>
#include <mmsystem.h>
#include <dxerr8.h>
#include <dplay8.h>
#include <dplobby8.h>
#include <stdio.h>
#include <assert.h>
#include "NetConnect.h"
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
#define DPLAY_SAMPLE_KEY        TEXT("Software\\Microsoft\\DirectX DirectPlay Samples")
#define MAX_PLAYER_NAME         14
#define TIMERID_NETWORK         0
#define TIMERID_STATS           1
#define WM_APP_UPDATE_TARGETS   (WM_APP + 0)
#define WM_APP_APPEND_TEXT      (WM_APP + 1)

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {BA214178-AAE6-4ea6-84E0-65CE36F84479}
GUID g_guidApp = { 0xba214178, 0xaae6, 0x4ea6, { 0x84, 0xe0, 0x65, 0xce, 0x36, 0xf8, 0x44, 0x79 } };

struct APP_PLAYER_INFO
{
    LONG  lRefCount;                        // Ref count so we can cleanup when all threads 
                                            // are done w/ this object
    DPNID dpnidPlayer;                      // DPNID of player
    TCHAR strPlayerName[MAX_PLAYER_NAME];  // Player name
    DWORD dwFlags;                          // Player flags
    APP_PLAYER_INFO* pNext;
    APP_PLAYER_INFO* pPrev;
};




//-----------------------------------------------------------------------------
// App specific DirectPlay messages and structures 
//-----------------------------------------------------------------------------
#define GAME_MSGID_GAMEPACKET       1

// Change compiler pack alignment to be BYTE aligned, and pop the current value
#pragma pack( push, 1 )

struct GAMEMSG_GENERIC
{
    DWORD dwType;
    DWORD dwPacketId;
};

#define GAMEMSG_GENERIC_SIZE 8

struct GAMEMSG_DATA_512 : GAMEMSG_GENERIC
{
    BYTE pBuffer[512-GAMEMSG_GENERIC_SIZE];
};

struct GAMEMSG_DATA_256 : GAMEMSG_GENERIC
{
    BYTE pBuffer[256-GAMEMSG_GENERIC_SIZE];
};

struct GAMEMSG_DATA_128 : GAMEMSG_GENERIC
{
    BYTE pBuffer[128-GAMEMSG_GENERIC_SIZE];
};

struct GAMEMSG_DATA_64 : GAMEMSG_GENERIC
{
    BYTE pBuffer[64-GAMEMSG_GENERIC_SIZE];
};

struct GAMEMSG_DATA_32 : GAMEMSG_GENERIC
{
    BYTE pBuffer[32-GAMEMSG_GENERIC_SIZE];
};

struct GAMEMSG_DATA_16 : GAMEMSG_GENERIC
{
    BYTE pBuffer[16-GAMEMSG_GENERIC_SIZE];
};

#define DATA_TYPE_NETPACKET_RECIEVE       1
#define DATA_TYPE_NETPACKET_SENT          2
#define DATA_TYPE_NETPACKET_TIMEOUT       3

struct GAMEMSG_DATA_NODE
{
    GAMEMSG_GENERIC*    pDataMsg;
    DWORD               dwPacketId;
    APP_PLAYER_INFO*    pPlayerFrom; 
    DWORD               dwReceiveDataSize;
    DPNHANDLE           hBufferHandle;
    DWORD               dwType;
    GAMEMSG_DATA_NODE*  pNext;
    GAMEMSG_DATA_NODE*  pPrev;
};

// Pop the old pack alignment
#pragma pack( pop )




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
TCHAR              g_strAppName[256]          = TEXT("DataRelay");
IDirectPlay8Peer*  g_pDP                      = NULL;   // DirectPlay peer object
CNetConnectWizard* g_pNetConnectWizard        = NULL;   // Connection wizard
IDirectPlay8LobbiedApplication* g_pLobbiedApp = NULL;   // DirectPlay lobbied app 
BOOL               g_bWasLobbyLaunched        = FALSE;   // TRUE if lobby launched
HINSTANCE          g_hInst                    = NULL;   // HINST of app
HWND               g_hDlg                     = NULL;   // HWND of main dialog
HRESULT            g_hrDialog;                          // Exit code for app 
TCHAR              g_strLocalPlayerName[MAX_PATH];      // Local player name
TCHAR              g_strSessionName[MAX_PATH];          // Session name
TCHAR              g_strPreferredProvider[MAX_PATH];    // Provider string
HANDLE             g_hDPDataAvailEvent        = NULL;   // Signaled if there is data to process
HANDLE             g_hShutdownEvent           = NULL;   // Signaled if shutting down
DWORD              g_dwPacketId               = 0;      // Packet id counter
APP_PLAYER_INFO*   g_pTargetPlayer            = NULL;   // The current target player
BOOL               g_bSendingData             = FALSE;  // TRUE if sending data
DPNID              g_dpnidLocalPlayer         = 0;      // DPNID of local player
DWORD              g_dwDataRecieved           = 0;      // Amount of data recieved 
DWORD              g_dwDataSent               = 0;      // Amount of data sent
DWORD              g_dwSendTimeout            = 0;      // Length of send timeout
DWORD              g_dwTimeBetweenSends       = 0;      // Time between packet sends
DWORD              g_dwSendSize               = 0;      // Size in bytes of packet
UINT               g_dwProcessNetDataThreadID = 0;      // Worker thread ID
HANDLE             g_hProcessNetDataThread    = NULL;   // Worker thread handle
APP_PLAYER_INFO    g_PlayerHead;                        // Linked list of players connected
GAMEMSG_DATA_NODE  g_DataHead;                          // Linked list of data to process
CRITICAL_SECTION   g_csPlayerList;                      // CS for g_PlayerHead
CRITICAL_SECTION   g_csDataList;                        // CS for g_DataHead
APP_PLAYER_INFO*   g_pConnInfoTargetPlayer    = NULL;   // The current target for Connection Info




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI   DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayLobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK SampleDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
UINT WINAPI     ProcessNetDataProc( LPVOID lpParameter );
HRESULT InitDirectPlay();
HRESULT OnInitDialog( HWND hDlg );
VOID    FillTargetCombo( HWND hDlg );
VOID    FillOneTimeCombos( HWND hDlg );
HRESULT LinkPlayer( DPNID dpnid, APP_PLAYER_INFO** ppPlayerInfo );
HRESULT SendNetworkData();
HRESULT ProcessData();
VOID    AppendTextToEditControl( HWND hDlg, TCHAR* strNewLogLine );
VOID    ReadCombos( HWND hDlg );
VOID    UpdateSendQueueInfo( HWND hDlg );
VOID    UpdateStats( HWND hDlg );




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, 
                      LPSTR pCmdLine, INT nCmdShow )
{
    HRESULT hr;
    BOOL    bConnectSuccess     = FALSE;
    HKEY    hDPlaySampleRegKey;

    // Create events and critical sections
    g_hDPDataAvailEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); 
    g_hShutdownEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); 
    InitializeCriticalSection( &g_csPlayerList );
    InitializeCriticalSection( &g_csDataList );
    InitializeCriticalSection( &g_csPlayerContext );

    g_hInst = hInst;

    // Init circular linked list
    ZeroMemory( &g_PlayerHead, sizeof(APP_PLAYER_INFO) );
    g_PlayerHead.pNext = &g_PlayerHead;
    g_PlayerHead.pPrev = &g_PlayerHead;
    g_PlayerHead.dpnidPlayer = DPNID_ALL_PLAYERS_GROUP;
    _tcscpy( g_PlayerHead.strPlayerName, TEXT("Everyone") );

    // Init circular linked list
    ZeroMemory( &g_DataHead, sizeof(GAMEMSG_DATA_NODE) );
    g_DataHead.pNext = &g_DataHead;
    g_DataHead.pPrev = &g_DataHead;  

    // Read persistent state information from registry
    RegCreateKeyEx( HKEY_CURRENT_USER, DPLAY_SAMPLE_KEY, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    &hDPlaySampleRegKey, NULL );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), 
                             g_strLocalPlayerName, MAX_PATH, TEXT("TestPlayer") );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), 
                             g_strSessionName, MAX_PATH, TEXT("TestGame") );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), 
                             g_strPreferredProvider, MAX_PATH, TEXT("DirectPlay8 TCP/IP Service Provider") );

    // Init COM so we can use CoCreateInstance
    CoInitializeEx( NULL, COINIT_MULTITHREADED );
    
    // Create helper class
    g_pNetConnectWizard = new CNetConnectWizard( hInst, NULL, g_strAppName, &g_guidApp );

    if( FAILED( hr = InitDirectPlay() ) )
    {
        DXTRACE_ERR( TEXT("InitDirectPlay"), hr );
        MessageBox( NULL, TEXT("Failed initializing IDirectPlay8Peer. ")
                    TEXT("The sample will now quit."),
                    TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
        return FALSE;
    }

    // Check if we were launched from a lobby client
    if( g_bWasLobbyLaunched && g_pNetConnectWizard->HaveConnectionSettingsFromLobby() )
    {
        // If were lobby launched then DPL_MSGID_CONNECT has already been
        // handled, so we can just tell the wizard to connect to the lobby
        // that has sent us a DPL_MSGID_CONNECT msg.
        if( FAILED( hr = g_pNetConnectWizard->ConnectUsingLobbySettings() ) )
        {
            DXTRACE_ERR( TEXT("ConnectUsingLobbySettings"), hr );
            MessageBox( NULL, TEXT("Failed to connect using lobby settings. ")
                        TEXT("The sample will now quit."),
                        TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );

            bConnectSuccess = FALSE;
        }
        else
        {
            // Read information from g_pNetConnectWizard
            _tcsncpy( g_strLocalPlayerName, g_pNetConnectWizard->GetPlayerName(), MAX_PLAYER_NAME );
            g_strLocalPlayerName[MAX_PLAYER_NAME-1]=0;

            bConnectSuccess = TRUE; 
        }
    }
    else
    {
        // If not lobby launched, prompt the user about the network 
        // connection and which session they would like to join or 
        // if they want to create a new one.

        // Setup connection wizard
        g_pNetConnectWizard->SetPlayerName( g_strLocalPlayerName );
        g_pNetConnectWizard->SetSessionName( g_strSessionName );
        g_pNetConnectWizard->SetPreferredProvider( g_strPreferredProvider );

        // Do the connection wizard
        hr = g_pNetConnectWizard->DoConnectWizard( FALSE );        
        if( FAILED( hr ) ) 
        {
            DXTRACE_ERR( TEXT("DoConnectWizard"), hr );
            MessageBox( NULL, TEXT("Multiplayer connect failed. ")
                        TEXT("The sample will now quit."),
                        TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
            bConnectSuccess = FALSE;
        } 
        else if( hr == NCW_S_QUIT ) 
        {
            // The user canceled the Multiplayer connect, so quit 
            bConnectSuccess = FALSE;
        }
        else
        {
            bConnectSuccess = TRUE; 

            // Read information from g_pNetConnectWizard
            _tcsncpy( g_strLocalPlayerName, g_pNetConnectWizard->GetPlayerName(), MAX_PLAYER_NAME );
            g_strLocalPlayerName[MAX_PLAYER_NAME-1]=0;
            _tcscpy( g_strSessionName, g_pNetConnectWizard->GetSessionName() );
            _tcscpy( g_strPreferredProvider, g_pNetConnectWizard->GetPreferredProvider() );

            // Write information to the registry
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), g_strLocalPlayerName );
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), g_strSessionName );
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), g_strPreferredProvider );
        }
    }

    if( bConnectSuccess )
    {
        // For this sample, we just start a simple dialog box game.
        g_hrDialog = S_OK;
        DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN_GAME), NULL, 
                   (DLGPROC) SampleDlgProc );
    }

    // Close down process data thread
    SetEvent( g_hShutdownEvent );
    WaitForSingleObject( g_hProcessNetDataThread, INFINITE );
    CloseHandle( g_hProcessNetDataThread );

    if( FAILED( g_hrDialog ) )
    {
        if( g_hrDialog == DPNERR_CONNECTIONLOST )
        {
            MessageBox( NULL, TEXT("The DirectPlay session was lost. ")
                        TEXT("The sample will now quit."),
                        TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
        }
        else
        {
            DXTRACE_ERR( TEXT("DialogBox"), g_hrDialog );
            MessageBox( NULL, TEXT("An error occured during the game. ")
                        TEXT("The sample will now quit."),
                        TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
        }
    }

    // Cleanup DirectPlay and helper classes
    g_pNetConnectWizard->Shutdown();

    if( g_pDP )
    {
        g_pDP->Close(0);
        SAFE_RELEASE( g_pDP );
    }

    if( g_pLobbiedApp )
    {
        g_pLobbiedApp->Close( 0 );
        SAFE_RELEASE( g_pLobbiedApp );
    }    

    // Don't delete the wizard until we know that 
    // DirectPlay is out of its message handlers.
    // This will be true after Close() has been called. 
    SAFE_DELETE( g_pNetConnectWizard );

    // Cleanup circular linked list, g_PlayerHead
    {
        EnterCriticalSection( &g_csPlayerList );
        APP_PLAYER_INFO* pNode = g_PlayerHead.pNext;
        APP_PLAYER_INFO* pDeleteNode;
        while( pNode != &g_PlayerHead ) 
        {
            pDeleteNode = pNode;
            pNode = pNode->pNext;
            SAFE_DELETE( pDeleteNode );
        }
        LeaveCriticalSection( &g_csPlayerList );
    }

    // Cleanup circular linked list, g_DataHead
    {
        EnterCriticalSection( &g_csDataList );
        GAMEMSG_DATA_NODE* pNode = g_DataHead.pNext;
        GAMEMSG_DATA_NODE* pDeleteNode;
        while( pNode != &g_DataHead ) 
        {
            pDeleteNode = pNode;
            pNode = pNode->pNext;
            SAFE_DELETE( pDeleteNode );
        }
        LeaveCriticalSection( &g_csDataList );
    }

    // Cleanup Win32 resources
    RegCloseKey( hDPlaySampleRegKey );
    CloseHandle( g_hShutdownEvent );
    CloseHandle( g_hDPDataAvailEvent );
    DeleteCriticalSection( &g_csPlayerList );
    DeleteCriticalSection( &g_csDataList );
    DeleteCriticalSection( &g_csPlayerContext );
    CoUninitialize();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: InitDirectPlay()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT InitDirectPlay()
{
    DPNHANDLE hLobbyLaunchedConnection = NULL;
    HRESULT hr;

    // Create IDirectPlay8Peer
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Peer, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Peer, 
                                       (LPVOID*) &g_pDP ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Create IDirectPlay8LobbiedApplication
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8LobbiedApplication, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8LobbiedApplication, 
                                       (LPVOID*) &g_pLobbiedApp ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Init the helper class, now that g_pDP and g_pLobbiedApp are valid
    g_pNetConnectWizard->Init( g_pDP, g_pLobbiedApp );

    // Init IDirectPlay8Peer
    if( FAILED( hr = g_pDP->Initialize( NULL, DirectPlayMessageHandler, 0 ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    // Init IDirectPlay8LobbiedApplication.  Before this Initialize() returns 
    // a DPL_MSGID_CONNECT msg may come in to the DirectPlayLobbyMessageHandler 
    // so be prepared ahead of time.
    if( FAILED( hr = g_pLobbiedApp->Initialize( NULL, DirectPlayLobbyMessageHandler, 
                                                &hLobbyLaunchedConnection, 0 ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    // IDirectPlay8LobbiedApplication::Initialize returns a handle to a connnection
    // if we have been lobby launced.  Initialize is guanteeded to return after 
    // the DPL_MSGID_CONNECT msg has been processed.  So unless a we are expected 
    // multiple lobby connections, we do not need to remember the lobby connection
    // handle since it will be recorded upon the DPL_MSGID_CONNECT msg.
    g_bWasLobbyLaunched = ( hLobbyLaunchedConnection != NULL );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SampleDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK SampleDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
        {
            g_hDlg = hDlg;
            if( FAILED( g_hrDialog = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), g_hrDialog );
                EndDialog( hDlg, 0 );
            }

            break;
        }

        case WM_APP_UPDATE_TARGETS:
        {
            // Display the player names in the UI
            CheckDlgButton( hDlg, IDC_SEND_READY, (g_bSendingData) ? BST_CHECKED : BST_UNCHECKED );
            PostMessage( hDlg, WM_COMMAND, IDC_SEND_READY, 0 );
            FillTargetCombo( g_hDlg );
            ReadCombos( g_hDlg );
            break;
        }

        case WM_APP_APPEND_TEXT:
        {
            // Append a string to the edit control
            TCHAR* strNewLogLine = (TCHAR*) wParam;
            AppendTextToEditControl( g_hDlg, strNewLogLine );
            SAFE_DELETE( strNewLogLine );
            break;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDC_SEND_READY:
                    g_bSendingData = (IsDlgButtonChecked( hDlg, IDC_SEND_READY ) == BST_CHECKED );
                    if( g_bSendingData )
                    {
                        SetTimer( g_hDlg, TIMERID_NETWORK, g_dwTimeBetweenSends, NULL );
                        SetDlgItemText( hDlg, IDC_SEND_READY, TEXT("Sending...") );
                    }
                    else
                    {
                        KillTimer( g_hDlg, TIMERID_NETWORK );
                        SetDlgItemText( hDlg, IDC_SEND_READY, TEXT("Push to Send") );
                    }
                    break;

                case IDCANCEL:
                    g_hrDialog = S_OK;
                    EndDialog( hDlg, 0 );
                    return TRUE;

                case IDC_SEND_SIZE_COMBO:
                case IDC_SEND_RATE_COMBO:
                case IDC_SEND_TARGET_COMBO:
                case IDC_CONNINFO_COMBO:
                    if( HIWORD(wParam) == CBN_SELENDOK )
                        ReadCombos( hDlg );
                    break;
            }
            break;
        }

        case WM_TIMER:
        {
            switch( wParam )
            {
                case TIMERID_NETWORK:
                {
                    // Send network data
                    if( FAILED( g_hrDialog = SendNetworkData() ) )
                    {
                        DXTRACE_ERR( TEXT("SendNetworkData"), g_hrDialog );
                        EndDialog( hDlg, 0 );
                    }
                    break;
                }
                
                case TIMERID_STATS:
                {
                    UpdateStats( hDlg );
                    break;
                }
            }
            break;
        }
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Inits the dialog.
//-----------------------------------------------------------------------------
HRESULT OnInitDialog( HWND hDlg )
{
    // Load and set the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Display local player's name
    SetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strLocalPlayerName );

    // Start a thread to process the network data
    g_hProcessNetDataThread = (HANDLE) _beginthreadex( NULL, 0, ProcessNetDataProc, 
                                                       hDlg, 0, &g_dwProcessNetDataThreadID );

    // Update the dialog box
    SendMessage( hDlg, WM_APP_UPDATE_TARGETS, 0, 0 );
    FillOneTimeCombos( hDlg );
    ReadCombos( hDlg );

    // Update the stats every second
    SetTimer( hDlg, TIMERID_STATS, 1000, NULL );

    if( g_pNetConnectWizard->IsHostPlayer() )
        SetWindowText( hDlg, TEXT("DataRelay (Host)") );
    else
        SetWindowText( hDlg, TEXT("DataRelay") );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FillTargetCombo()
// Desc: Fills the target combo with all of the players
//-----------------------------------------------------------------------------
VOID FillTargetCombo( HWND hDlg )
{
    DWORD    dwNumberOfActivePlayers = 0;
    int      nIndex;
    int      nCurSelect;
    int      nNewSelect = 0;
    LONG_PTR lCurItemData; 
    HWND     hTargetCombo;
    HWND     hInfoTargetCombo;

    if( hDlg == NULL )
        return;

    hTargetCombo = GetDlgItem( hDlg, IDC_SEND_TARGET_COMBO );
    if( hTargetCombo == NULL )
        return;

    nCurSelect   = (int)SendMessage( hTargetCombo, CB_GETCURSEL, 0, 0 );
    lCurItemData = (LONG_PTR)SendMessage( hTargetCombo, CB_GETITEMDATA, nCurSelect, 0 );

    // Clear combo box
    SendMessage( hTargetCombo, CB_RESETCONTENT, 0, 0 );

    // Add "everyone"
    nIndex = (int)SendMessage( hTargetCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("Everyone") );
    SendMessage( hTargetCombo, CB_SETITEMDATA, nIndex, (LPARAM) &g_PlayerHead );

    // Enter the player list CS, because we are about to read shared memory, and 
    // the data may be changed concurrently from the DirectPlay
    // message handler threads
    EnterCriticalSection( &g_csPlayerList );

    // Add each player
    APP_PLAYER_INFO* pNode = g_PlayerHead.pNext;
    while( pNode != &g_PlayerHead ) 
    {
        if( (pNode->dwFlags & DPNPLAYER_LOCAL) == 0 )
        {
            nIndex = (int)SendMessage( hTargetCombo, CB_ADDSTRING, 0, (LPARAM) pNode->strPlayerName );
            SendMessage( hTargetCombo, CB_SETITEMDATA, nIndex, (LPARAM) pNode );
            if( lCurItemData == (LONG_PTR) pNode )
                nNewSelect = nIndex;
        }

        dwNumberOfActivePlayers++;
        pNode = pNode->pNext;
    }

    LeaveCriticalSection( &g_csPlayerList );
     
    SendMessage( hTargetCombo, CB_SETCURSEL, nNewSelect, 0 );  

    // Update the number of players in game counter
    TCHAR strNumberPlayers[32];
    wsprintf( strNumberPlayers, TEXT("%d"), dwNumberOfActivePlayers );
    SetDlgItemText( hDlg, IDC_NUM_PLAYERS, strNumberPlayers );

    if( dwNumberOfActivePlayers > 1 )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_SEND_READY ), TRUE );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_SEND_READY ), FALSE );
        CheckDlgButton( hDlg, IDC_SEND_READY, BST_UNCHECKED );
        SetDlgItemText( hDlg, IDC_SEND_READY, TEXT("Push to Send") );
    }

    // Populate the connection info box    
    hInfoTargetCombo = GetDlgItem( hDlg, IDC_CONNINFO_COMBO );
        
    nCurSelect   = (int)SendMessage( hInfoTargetCombo, CB_GETCURSEL, 0, 0 );
    lCurItemData = (LONG_PTR)SendMessage( hInfoTargetCombo, CB_GETITEMDATA, nCurSelect, 0 );
    
    // Clear combo box
    SendMessage( hInfoTargetCombo, CB_RESETCONTENT, 0, 0 );
    
    // Add "none"
    nIndex = (int)SendMessage( hInfoTargetCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("None") );
    SendMessage( hInfoTargetCombo, CB_SETITEMDATA, nIndex, (LPARAM) &g_PlayerHead );

    // Enter the player list CS, because we are about to read shared memory, and 
    // the data may be changed concurrently from the DirectPlay
    // message handler threads
    EnterCriticalSection( &g_csPlayerList );

    // Add each player    
    pNode = g_PlayerHead.pNext;
    nNewSelect = 0;
    while( pNode != &g_PlayerHead ) 
    {
        if( (pNode->dwFlags & DPNPLAYER_LOCAL) == 0 )
        {
            nIndex = (int)SendMessage( hInfoTargetCombo, CB_ADDSTRING, 0, (LPARAM) pNode->strPlayerName );
            SendMessage( hInfoTargetCombo, CB_SETITEMDATA, nIndex, (LPARAM) pNode );
            if( lCurItemData == (LONG_PTR) pNode )
                nNewSelect = nIndex;
        }

        dwNumberOfActivePlayers++;
        pNode = pNode->pNext;
    }

    LeaveCriticalSection( &g_csPlayerList );
     
    SendMessage( hInfoTargetCombo, CB_SETCURSEL, nNewSelect, 0 );  
}




//-----------------------------------------------------------------------------
// Name: FillOneTimeCombos()
// Desc: Fill the unchanging UI combos box
//-----------------------------------------------------------------------------
VOID FillOneTimeCombos( HWND hDlg )
{
    if( hDlg == NULL )
        return;

    HWND hRateCombo = GetDlgItem( hDlg, IDC_SEND_RATE_COMBO );
    SendMessage( hRateCombo, WM_SETREDRAW, FALSE, 0 );
    SendMessage( hRateCombo, CB_RESETCONTENT, 0, 0 );
    SendMessage( hRateCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("1000") );
    SendMessage( hRateCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("500") );
    SendMessage( hRateCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("250") );
    SendMessage( hRateCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("100") );
    SendMessage( hRateCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("50") );
    SendMessage( hRateCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("0") );
    SendMessage( hRateCombo, WM_SETREDRAW, TRUE, 0 );
    SendMessage( hRateCombo, CB_SETCURSEL, 0, 0 );  
    
    HWND hSizeCombo = GetDlgItem( hDlg, IDC_SEND_SIZE_COMBO );
    SendMessage( hSizeCombo, WM_SETREDRAW, FALSE, 0 );
    SendMessage( hSizeCombo, CB_RESETCONTENT, 0, 0 );
    SendMessage( hSizeCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("512") );
    SendMessage( hSizeCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("256") );
    SendMessage( hSizeCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("128") );
    SendMessage( hSizeCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("64") );
    SendMessage( hSizeCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("32") );
    SendMessage( hSizeCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("16") );
    SendMessage( hSizeCombo, WM_SETREDRAW, TRUE, 0 );
    SendMessage( hSizeCombo, CB_SETCURSEL, 0, 0 );  
 
    HWND hTimeoutCombo = GetDlgItem( hDlg, IDC_SEND_TIMEOUT_COMBO );
    SendMessage( hTimeoutCombo, WM_SETREDRAW, FALSE, 0 );
    SendMessage( hTimeoutCombo, CB_RESETCONTENT, 0, 0 );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("5") );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("10") );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("20") );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("50") );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("100") );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("250") );
    SendMessage( hTimeoutCombo, CB_ADDSTRING, 0, (LPARAM) TEXT("500") );
    SendMessage( hTimeoutCombo, WM_SETREDRAW, TRUE, 0 );
    SendMessage( hTimeoutCombo, CB_SETCURSEL, 2, 0 );  
}




//-----------------------------------------------------------------------------
// Name: ReadCombos()
// Desc: Reads the state of the combos and updates the global vars
//-----------------------------------------------------------------------------
VOID ReadCombos( HWND hDlg )
{
    TCHAR strText[128];
    int nCurSelect;

    // Get target player
    HWND hTargetCombo  = GetDlgItem( hDlg, IDC_SEND_TARGET_COMBO );
    nCurSelect = (int)SendMessage( hTargetCombo, CB_GETCURSEL, 0, 0 );
    if( nCurSelect != CB_ERR ) 
    {
        g_pTargetPlayer = (APP_PLAYER_INFO*) SendMessage( hTargetCombo, CB_GETITEMDATA, 
                                                          nCurSelect, 0 );
    }

    // Get rate (in ms)
    HWND hRateCombo  = GetDlgItem( hDlg, IDC_SEND_RATE_COMBO );
    nCurSelect = (int)SendMessage( hRateCombo, CB_GETCURSEL, 0, 0 );
    if( nCurSelect != CB_ERR ) 
    {
        SendMessage( hRateCombo, CB_GETLBTEXT, nCurSelect, (LPARAM) strText );
        g_dwTimeBetweenSends = _ttoi( strText );
        KillTimer( g_hDlg, TIMERID_NETWORK );
        SetTimer( g_hDlg, TIMERID_NETWORK, g_dwTimeBetweenSends, NULL );
    }

    // Get size (in bytes)
    HWND hSizeCombo = GetDlgItem( hDlg, IDC_SEND_SIZE_COMBO );
    nCurSelect = (int)SendMessage( hSizeCombo, CB_GETCURSEL, 0, 0 );
    if( nCurSelect != CB_ERR ) 
    {
        SendMessage( hSizeCombo, CB_GETLBTEXT, nCurSelect, (LPARAM) strText );
        g_dwSendSize = _ttoi( strText );
    }

    // Get timeout (in ms)
    HWND hTimeoutCombo = GetDlgItem( hDlg, IDC_SEND_TIMEOUT_COMBO );
    nCurSelect = (int)SendMessage( hTimeoutCombo, CB_GETCURSEL, 0, 0 );
    if( nCurSelect != CB_ERR ) 
    {
        SendMessage( hTimeoutCombo, CB_GETLBTEXT, nCurSelect, (LPARAM) strText );
        g_dwSendTimeout = _ttoi( strText );
    }

    // Get the ConnectionInfo Target player
    HWND hConnTargetCombo  = GetDlgItem( hDlg, IDC_CONNINFO_COMBO );
    nCurSelect = (int)SendMessage( hConnTargetCombo, CB_GETCURSEL, 0, 0 );
    if( nCurSelect != CB_ERR ) 
    {
        g_pConnInfoTargetPlayer = (APP_PLAYER_INFO*) SendMessage( hConnTargetCombo, CB_GETITEMDATA, 
                                                      nCurSelect, 0 );
    }
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
    // Try not to stay in this message handler for too long, otherwise
    // there will be a backlog of data.  The best solution is to 
    // queue data as it comes in, and then handle it on other threads
    // as this sample shows

    // This function is called by the DirectPlay message handler pool of 
    // threads, so be care of thread synchronization problems with shared memory

    HRESULT hReturn = S_OK;

    switch( dwMessageId )
    {
        case DPN_MSGID_CREATE_PLAYER:
        {
            HRESULT hr;
            PDPNMSG_CREATE_PLAYER pCreatePlayerMsg;
            pCreatePlayerMsg = (PDPNMSG_CREATE_PLAYER) pMsgBuffer;

            APP_PLAYER_INFO* pPlayerInfo;
            if( FAILED( hr = LinkPlayer( pCreatePlayerMsg->dpnidPlayer, 
                                         &pPlayerInfo ) ) )
                return DXTRACE_ERR( TEXT("LinkPlayer"), hr );

            // Tell DirectPlay to store this pPlayerInfo 
            // pointer in the pvPlayerContext.
            pCreatePlayerMsg->pvPlayerContext = pPlayerInfo;

            // Post a message to the dialog thread to update the 
            // UI.  This keeps the DirectPlay message handler 
            // from blocking
            if( g_hDlg != NULL )
                PostMessage( g_hDlg, WM_APP_UPDATE_TARGETS, 0, 0 );
            break;
        }

        case DPN_MSGID_DESTROY_PLAYER:
        {
            PDPNMSG_DESTROY_PLAYER pDestroyPlayerMsg;
            pDestroyPlayerMsg = (PDPNMSG_DESTROY_PLAYER)pMsgBuffer;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pDestroyPlayerMsg->pvPlayerContext;

            // Enter the player list CS, because we are about to 
            // modify shared memory, and the data may be changed 
            // concurrently from other DirectPlay message handler threads 
            EnterCriticalSection( &g_csPlayerList );

            // Unlink pPlayerInfo from the chain, m_PlayerHead.
            pPlayerInfo->pPrev->pNext = pPlayerInfo->pNext;
            pPlayerInfo->pNext->pPrev = pPlayerInfo->pPrev;

            LeaveCriticalSection( &g_csPlayerList );

            // Stop sending if the target was destroyed
            if( g_pTargetPlayer )
            {
                if( g_pTargetPlayer->dpnidPlayer == pPlayerInfo->dpnidPlayer )
                    g_bSendingData = FALSE;
            }

            PLAYER_LOCK();                  // enter player context CS
            PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
            PLAYER_UNLOCK();                // leave player context CS

            // Post a message to the dialog thread to update the 
            // UI.  This keeps the DirectPlay message handler 
            // from blocking
            if( g_hDlg != NULL )
                PostMessage( g_hDlg, WM_APP_UPDATE_TARGETS, 0, 0 );
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

        case DPN_MSGID_HOST_MIGRATE:
        {
            PDPNMSG_HOST_MIGRATE pHostMigrateMsg;
            pHostMigrateMsg = (PDPNMSG_HOST_MIGRATE)pMsgBuffer;

            if( pHostMigrateMsg->dpnidNewHost == g_dpnidLocalPlayer )
                SetWindowText( g_hDlg, TEXT("DataRelay (Host)") );
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
            if( pMsg->dwType == GAME_MSGID_GAMEPACKET )
            {
                GAMEMSG_GENERIC* pDataMsg = (GAMEMSG_GENERIC*) pMsg;

                // Make a new GAMEMSG_DATA_NODE and hand it off to the
                // app worker thread.  It will process the node, and
                // then update the UI to show that the packet was processed
                GAMEMSG_DATA_NODE* pDataMsgNode = new GAMEMSG_DATA_NODE;                
                ZeroMemory( pDataMsgNode, sizeof(GAMEMSG_DATA_NODE) ); 

                pDataMsgNode->dwType            = DATA_TYPE_NETPACKET_RECIEVE;
                pDataMsgNode->dwPacketId        = pDataMsg->dwPacketId;
                pDataMsgNode->pDataMsg          = pDataMsg;
                pDataMsgNode->dwReceiveDataSize = pReceiveMsg->dwReceiveDataSize;
                pDataMsgNode->pPlayerFrom       = pPlayerInfo;
                pDataMsgNode->hBufferHandle     = pReceiveMsg->hBufferHandle;

                // Enter the data list CS, because we are about to modify shared memory, and 
                // the data may be changed concurrently from other DirectPlay
                // message handler threads, as well as the the app worker thread
                EnterCriticalSection( &g_csDataList );

                g_dwDataRecieved += pReceiveMsg->dwReceiveDataSize;

                // Then add it to the circular linked list, g_DataHead
                // so it can be processed by a worker thread
                pDataMsgNode->pNext = &g_DataHead;
                pDataMsgNode->pPrev = g_DataHead.pPrev;
                g_DataHead.pPrev->pNext = pDataMsgNode;
                g_DataHead.pPrev        = pDataMsgNode;

                LeaveCriticalSection( &g_csDataList );

                // Tell the app worker thread that there 
                // is new network data availible
                SetEvent( g_hDPDataAvailEvent );

                // Tell DirectPlay to assume that ownership of the buffer 
                // has been transferred to the application, and so it will 
                // neither free nor modify it until ownership is returned 
                // to DirectPlay through the ReturnBuffer() call.
                hReturn = DPNSUCCESS_PENDING;
            }
            break;
        }

        case DPN_MSGID_SEND_COMPLETE:
        {
            PDPNMSG_SEND_COMPLETE pSendCompleteMsg;
            pSendCompleteMsg = (PDPNMSG_SEND_COMPLETE)pMsgBuffer;

            GAMEMSG_GENERIC* pGameMsg;
            pGameMsg = (GAMEMSG_GENERIC*) pSendCompleteMsg->pvUserContext;

            if( pSendCompleteMsg->hResultCode == DPNERR_TIMEDOUT )
            {
                // Make a new GAMEMSG_DATA_NODE and hand it off to the
                // app worker thread.  It will process the node, and
                // then update the UI to show that the packet timed out
                GAMEMSG_DATA_NODE* pDataMsgNode = new GAMEMSG_DATA_NODE;
                ZeroMemory( pDataMsgNode, sizeof(GAMEMSG_DATA_NODE) ); 
                
                pDataMsgNode->dwType     = DATA_TYPE_NETPACKET_TIMEOUT;
                pDataMsgNode->dwPacketId = pGameMsg->dwPacketId;

                // Enter the data list CS, because we are about to modify shared memory, and 
                // the data may be changed concurrently from other DirectPlay
                // message handler threads, as well as the the app worker thread
                EnterCriticalSection( &g_csDataList );

                // Then add it to the circular linked list, g_DataHead
                // so it can be processed by a worker thread
                pDataMsgNode->pNext = &g_DataHead;
                pDataMsgNode->pPrev = g_DataHead.pPrev;
                g_DataHead.pPrev->pNext = pDataMsgNode;
                g_DataHead.pPrev        = pDataMsgNode;

                LeaveCriticalSection( &g_csDataList );

                // Tell the app worker thread that there 
                // is new data availible. 
                SetEvent( g_hDPDataAvailEvent );
            }

            SAFE_DELETE( pGameMsg );

            break;
        }
    }

    // Make sure the DirectPlay MessageHandler calls the CNetConnectWizard handler, 
    // so it can be informed of messages such as DPN_MSGID_ENUM_HOSTS_RESPONSE.
    if( DPNSUCCESS_PENDING != hReturn && SUCCEEDED(hReturn) && g_pNetConnectWizard )
        hReturn = g_pNetConnectWizard->MessageHandler( pvUserContext, dwMessageId, pMsgBuffer );
    
    return hReturn;
}




//-----------------------------------------------------------------------------
// Name: LinkPlayer()
// Desc: Link a new player to the circular linked list, g_PlayerHead.  This
//       is called by the DirectPlay message handler threads, so we enter a 
//       CS to avoid thread syncoronization problems.
//-----------------------------------------------------------------------------
HRESULT LinkPlayer( DPNID dpnid, APP_PLAYER_INFO** ppPlayerInfo )
{
    HRESULT hr;

    // Create a new and fill in a APP_PLAYER_INFO
    APP_PLAYER_INFO* pPlayerInfo = new APP_PLAYER_INFO;
    pPlayerInfo->dpnidPlayer = dpnid;
    pPlayerInfo->lRefCount   = 1;

    // Get the peer info and extract its name
    DWORD dwSize = 0;
    DPN_PLAYER_INFO* pdpPlayerInfo = NULL;
    hr = DPNERR_CONNECTING;
    
    // GetPeerInfo might return DPNERR_CONNECTING when connecting, 
    // so just keep calling it if it does
    while( hr == DPNERR_CONNECTING ) 
        hr = g_pDP->GetPeerInfo( dpnid, pdpPlayerInfo, &dwSize, 0 );                                
        
    if( hr == DPNERR_BUFFERTOOSMALL )
    {
        pdpPlayerInfo = (DPN_PLAYER_INFO*) new BYTE[ dwSize ];
        ZeroMemory( pdpPlayerInfo, dwSize );
        pdpPlayerInfo->dwSize = sizeof(DPN_PLAYER_INFO);
        
        hr = g_pDP->GetPeerInfo( dpnid, pdpPlayerInfo, &dwSize, 0 );
        if( SUCCEEDED(hr) )
        {
            // This stores a extra TCHAR copy of the player name for 
            // easier access.  This will be redundent copy since DPlay 
            // also keeps a copy of the player name in GetPeerInfo()
            DXUtil_ConvertWideStringToGeneric( pPlayerInfo->strPlayerName, 
                                               pdpPlayerInfo->pwszName, MAX_PLAYER_NAME );    
                                               
            pPlayerInfo->dwFlags     = pdpPlayerInfo->dwPlayerFlags;
            
            if( pdpPlayerInfo->dwPlayerFlags & DPNPLAYER_LOCAL )
                g_dpnidLocalPlayer = dpnid;
        }
        
        SAFE_DELETE_ARRAY( pdpPlayerInfo );
    }

    // Enter the player list CS, because we are about to modify shared memory, and 
    // the data may be changed concurrently from the DirectPlay
    // message handler threads 
    EnterCriticalSection( &g_csPlayerList );

    // Then add it to the circular linked list, g_PlayerHead,
    pPlayerInfo->pNext = g_PlayerHead.pNext;
    pPlayerInfo->pPrev = &g_PlayerHead;
    g_PlayerHead.pNext->pPrev = pPlayerInfo;
    g_PlayerHead.pNext        = pPlayerInfo;

    LeaveCriticalSection( &g_csPlayerList );

    if( ppPlayerInfo )
        *ppPlayerInfo = pPlayerInfo;

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
        case DPL_MSGID_CONNECT:
        {
            PDPL_MESSAGE_CONNECT pConnectMsg;
            pConnectMsg = (PDPL_MESSAGE_CONNECT)pMsgBuffer;

            // The CNetConnectWizard will handle this message for us,
            // so there is nothing we need to do here for this simple
            // sample.
            break;
        }

        case DPL_MSGID_DISCONNECT:
        {
            PDPL_MESSAGE_DISCONNECT pDisconnectMsg;
            pDisconnectMsg = (PDPL_MESSAGE_DISCONNECT)pMsgBuffer;

            // We should free any data associated with the lobby 
            // client here, but there is none.
            break;
        }

        case DPL_MSGID_RECEIVE:
        {
            PDPL_MESSAGE_RECEIVE pReceiveMsg;
            pReceiveMsg = (PDPL_MESSAGE_RECEIVE)pMsgBuffer;

            // The lobby client sent us data.  This sample doesn't
            // expected data from the client, but it is useful 
            // for more complex apps.
            break;
        }

        case DPL_MSGID_CONNECTION_SETTINGS:
        {
            PDPL_MESSAGE_CONNECTION_SETTINGS pConnectionStatusMsg;
            pConnectionStatusMsg = (PDPL_MESSAGE_CONNECTION_SETTINGS)pMsgBuffer;

            // The lobby client has changed the connection settings.  
            // This simple sample doesn't handle this, but more complex apps may
            // want to.
            break;
        }
    }

    // Make sure the DirectPlay MessageHandler calls the CNetConnectWizard handler, 
    // so the wizard can be informed of lobby messages such as DPL_MSGID_CONNECT
    if( g_pNetConnectWizard )
        return g_pNetConnectWizard->LobbyMessageHandler( pvUserContext, dwMessageId, 
                                                         pMsgBuffer );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SendNetworkData()
// Desc: Sends a network packet of the specified size to the specified target.
//       This is called by the dialog UI thread.
//-----------------------------------------------------------------------------
HRESULT SendNetworkData()
{
    HRESULT hr;    

    if( !g_bSendingData || 
        IsDlgButtonChecked( g_hDlg, IDC_SEND_READY ) == BST_UNCHECKED )
    {
        KillTimer( g_hDlg, TIMERID_NETWORK );
        return S_OK;
    }

    if( g_pTargetPlayer == NULL )
        return E_FAIL;

    GAMEMSG_GENERIC* pGameMsg = NULL;
    DWORD            dwBufferSize;

    // Create a game packet based on the selected choices
    switch( g_dwSendSize )
    {
        case 512:
        {
            GAMEMSG_DATA_512* pMsg512 = new GAMEMSG_DATA_512;
            FillMemory( pMsg512->pBuffer, sizeof(pMsg512->pBuffer), 1 );
            pGameMsg = pMsg512;
            dwBufferSize = sizeof(GAMEMSG_DATA_512);
            break;
        }
        
        case 256:
        {
            GAMEMSG_DATA_256* pMsg256 = new GAMEMSG_DATA_256;
            FillMemory( pMsg256->pBuffer, sizeof(pMsg256->pBuffer), 2 );
            pGameMsg = pMsg256;
            dwBufferSize = sizeof(GAMEMSG_DATA_256);
            break;
        }

        case 128:
        {
            GAMEMSG_DATA_128* pMsg128 = new GAMEMSG_DATA_128;
            FillMemory( pMsg128->pBuffer, sizeof(pMsg128->pBuffer), 3 );
            pGameMsg = pMsg128;
            dwBufferSize = sizeof(GAMEMSG_DATA_128);
            break;
        }

        case 64:
        {
            GAMEMSG_DATA_64* pMsg64 = new GAMEMSG_DATA_64;
            FillMemory( pMsg64->pBuffer, sizeof(pMsg64->pBuffer), 4 );
            pGameMsg = pMsg64;
            dwBufferSize = sizeof(GAMEMSG_DATA_64);
            break;
        }

        case 32:
        {
            GAMEMSG_DATA_32* pMsg32 = new GAMEMSG_DATA_32;
            FillMemory( pMsg32->pBuffer, sizeof(pMsg32->pBuffer), 5 );
            pGameMsg = pMsg32;
            dwBufferSize = sizeof(GAMEMSG_DATA_32);
            break;
        }

        default:
        case 16:
        {
            GAMEMSG_DATA_16* pMsg16 = new GAMEMSG_DATA_16;
            FillMemory( pMsg16->pBuffer, sizeof(pMsg16->pBuffer), 6 );
            pGameMsg = pMsg16;
            dwBufferSize = sizeof(GAMEMSG_DATA_16);
            break;
        }
    }

    // Update the rest of the game msg 
    g_dwPacketId++;
    pGameMsg->dwPacketId   = g_dwPacketId;
    pGameMsg->dwType       = GAME_MSGID_GAMEPACKET;
            
    // Make a new GAMEMSG_DATA_NODE and hand it off to the
    // app worker thread.  It will process the node, and
    // then update the UI to show that a packet was sent
    GAMEMSG_DATA_NODE* pDataMsgNode = new GAMEMSG_DATA_NODE;
    ZeroMemory( pDataMsgNode, sizeof(GAMEMSG_DATA_NODE) ); 

    pDataMsgNode->dwType            = DATA_TYPE_NETPACKET_SENT;
    pDataMsgNode->dwPacketId        = pGameMsg->dwPacketId;
    pDataMsgNode->dwReceiveDataSize = dwBufferSize;

    // Enter the data list CS, because we are about to modify shared memory, and 
    // the data may be changed concurrently from other DirectPlay
    // message handler threads, as well as the the app worker thread
    EnterCriticalSection( &g_csDataList );

    // Then add it to the circular linked list, g_DataHead
    // so it can be processed by a worker thread
    pDataMsgNode->pNext = &g_DataHead;
    pDataMsgNode->pPrev = g_DataHead.pPrev;
    g_DataHead.pPrev->pNext = pDataMsgNode;
    g_DataHead.pPrev        = pDataMsgNode;

    LeaveCriticalSection( &g_csDataList );

    // This var is only accessed by the dialog 
    // thread, so it is safe to access it directly
    g_dwDataSent += dwBufferSize;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = dwBufferSize;
    bufferDesc.pBufferData  = (BYTE*) pGameMsg;

    DPNHANDLE hAsync;
    hr = g_pDP->SendTo( g_pTargetPlayer->dpnidPlayer, &bufferDesc, 1,
                        g_dwSendTimeout, pGameMsg, &hAsync, 
                        DPNSEND_NOLOOPBACK | DPNSEND_NOCOPY );
    // Ignore all errors except DPNERR_INVALIDPLAYER
    if( hr == DPNERR_INVALIDPLAYER )
    {
        // Stop sending if the target left game
        g_bSendingData = FALSE;
        PostMessage( g_hDlg, WM_APP_UPDATE_TARGETS, 0, 0 );
        return S_OK;
    }
    

    // Tell the app worker thread that there 
    // is new data availible. 
    SetEvent( g_hDPDataAvailEvent );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ProcessNetDataProc()
// Desc: Worker thread that processes data found in g_DataHead, 
//       and updates the UI state based on the data 
//-----------------------------------------------------------------------------
UINT WINAPI ProcessNetDataProc( LPVOID lpParameter )
{
    HWND   hDlg = (HWND) lpParameter;
    DWORD  dwResult;
    BOOL   bDone = FALSE;
    HANDLE ahHandles[2];

    ahHandles[0] = g_hDPDataAvailEvent;
    ahHandles[1] = g_hShutdownEvent;

    while( !bDone ) 
    { 
        dwResult = WaitForMultipleObjects( 2, ahHandles, FALSE, INFINITE );

        switch( dwResult )
        {
            case WAIT_OBJECT_0 + 0:
            {
                // g_hDPDataAvailEvent is signaled, so there is 
                // network data available to process
                if( FAILED( g_hrDialog = ProcessData() ) ) 
                {
                    DXTRACE_ERR( TEXT("ProcessData"), g_hrDialog );
                    bDone = TRUE;
                }
                break;
            }

            case WAIT_OBJECT_0 + 1:
            {
                // g_hShutdownEvent is signaled, so shut down
                bDone = TRUE;
                break;
            }
        }
    }

    return 0;
}   




//-----------------------------------------------------------------------------
// Name: ProcessData()
// Desc: This is called by the app worker thread to process the data found in
//       g_DataHead.  The DirectPlay message handler threads add new nodes 
//       upon events such as packet packet recieve/send or timeout.
//-----------------------------------------------------------------------------
HRESULT ProcessData()
{
    TCHAR* strNewLogLine = NULL;

    // Enter the data list CS, because we are about to read shared memory, and 
    // the data may be changed concurrently from the DirectPlay
    // message handler threads 
    EnterCriticalSection( &g_csDataList );

    GAMEMSG_DATA_NODE* pDeleteNode; 
    GAMEMSG_DATA_NODE* pNode = g_DataHead.pNext;
    while( pNode != &g_DataHead )
    {
        // Allocate a string on the heap, and fill it
        // with text about what happened.
        strNewLogLine = new TCHAR[MAX_PATH];

        switch( pNode->dwType )
        {
            case DATA_TYPE_NETPACKET_RECIEVE:
                wsprintf( strNewLogLine, TEXT("Got %s's #%d (%d bytes)\r\n"), 
                        pNode->pPlayerFrom->strPlayerName,
                        pNode->dwPacketId,            
                        pNode->dwReceiveDataSize );           

                // The app can do more process of the network 
                // packet here here -- i.e. change the
                // state of the game.  This simple sample just
                // updates the UI.
                break;

            case DATA_TYPE_NETPACKET_SENT:
                wsprintf( strNewLogLine, TEXT("Sending #%d to %s (%d bytes)\r\n"), 
                                         pNode->dwPacketId,            
                                         g_pTargetPlayer->strPlayerName,
                                         pNode->dwReceiveDataSize );           
                break;

            case DATA_TYPE_NETPACKET_TIMEOUT:
                wsprintf( strNewLogLine, TEXT("Packet #%d timed out\r\n"), 
                         pNode->dwPacketId );           
                break;
        }

        // Post a message to the dialog thread, so it can
        // take the string on the heap, and appead it to the log.
        // If the log is appended from this thread, then if the dialog
        // thread tries to enter the g_csDataList CS, then it will be
        // locked until this thread release it, and the 
        // AppendTextToEditControl function relies on reponses from the 
        // dialog thread, so a race condition is possible unless
        // the dialog thread itself appeads the text.
        PostMessage( g_hDlg, WM_APP_APPEND_TEXT, (WPARAM) strNewLogLine, 0 );

        // The dialog thread will free the string
        strNewLogLine = NULL;

        if( pNode->hBufferHandle != NULL )
        {
            // Done with the buffer, so return it DirectPlay, 
            // so that the memory can be reused
            g_pDP->ReturnBuffer( pNode->hBufferHandle,0 );
        }

        // Unlink pPlayerInfo from the chain, g_DataHead.
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;

        pDeleteNode = pNode;
        pNode = pNode->pNext;

        assert( pDeleteNode != &g_DataHead );
        SAFE_DELETE( pDeleteNode );
    }

    LeaveCriticalSection( &g_csDataList );

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
    if( SendMessage( hEdit, EM_GETLINECOUNT, 0, 0 ) > 18 )
        nSecondLine = (int)SendMessage( hEdit, EM_LINEINDEX, 1, 0 );

    SetWindowText( hEdit, &strText[nSecondLine] );

    SendMessage( hEdit, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect( hEdit, NULL, TRUE );
    UpdateWindow( hEdit );
}




//-----------------------------------------------------------------------------
// Name: UpdateConnectionInfo()
// Desc: Using the info buffer passed in,  display some interesting tidbits about
//       the connection.
//-----------------------------------------------------------------------------
VOID UpdateConnectionInfo( HWND hDlg, PDPN_CONNECTION_INFO pConnectionInfo,
                           DWORD dwHighMessages, DWORD dwHighBytes, 
                           DWORD dwNormalMessages, DWORD dwNormalBytes,
                           DWORD dwLowMessages, DWORD dwLowBytes )
{
    TCHAR strText[1024];
    int   nDrops;
    int   nSends;

    HWND hEdit = GetDlgItem( hDlg, IDC_CONNINFO_EDIT );
    int nThumbIndex = GetScrollPos( hEdit, SB_VERT );

    if( pConnectionInfo )
    {
        nDrops = pConnectionInfo->dwPacketsDropped + pConnectionInfo->dwPacketsRetried;
        nDrops *= 10000;

        nSends = pConnectionInfo->dwPacketsSentGuaranteed + pConnectionInfo->dwPacketsSentNonGuaranteed;

        if(nSends)
            nDrops /= nSends; // percent nDrops * 100

        _stprintf(strText, 
                           TEXT("Send Queue Messages High Priority=%d\r\n")       \
                           TEXT("Send Queue Bytes High Priority=%d\r\n")     \
                           
                           TEXT("Send Queue Messages Normal Priority=%d\r\n")       \
                           TEXT("Send Queue Bytes Normal Priority=%d\r\n")     \

                           TEXT("Send Queue Messages Low Priority=%d\r\n")       \
                           TEXT("Send Queue Bytes Low Priority=%d\r\n")     \

                           TEXT("Round Trip Latency MS=%dms\r\n")                  \
                           TEXT("Throughput BPS=%d\r\n")                         \
                           TEXT("Peak Throughput BPS=%d\r\n")                     \
                                                                            \
                           TEXT("Bytes Sent Guaranteed=%d\r\n")                   \
                           TEXT("Packets Sent Guaranteed=%d\r\n")                 \
                           TEXT("Bytes Sent Non-Guaranteed=%d\r\n")                \
                           TEXT("Packets Sent Non-Guaranteed=%d\r\n")              \
                                                                            \
                           TEXT("Bytes Retried Guaranteed=%d\r\n")        \
                           TEXT("Packets Retried Guaranteed=%d\r\n")      \
                           TEXT("Bytes Dropped Non-Guaranteed=%d\r\n")    \
                           TEXT("Packets Dropped Non-Guaranteed=%d\r\n")  \
                                                                            \
                           TEXT("Messages Transmitted High Priority=%d\r\n")       \
                           TEXT("Messages Timed Out High Priority=%d\r\n")          \
                           TEXT("Messages Transmitted Normal Priority=%d\r\n")     \
                           TEXT("Messages Timed Out Normal Priority=%d\r\n")        \
                           TEXT("Messages Transmitted Low Priority=%d\r\n")        \
                           TEXT("Messages Timed Out Low Priority=%d\r\n")           \
                                                                            \
                           TEXT("Bytes Received Guaranteed=%d\r\n")               \
                           TEXT("Packets Received Guaranteed=%d\r\n")             \
                           TEXT("Bytes Received Non-Guaranteed=%d\r\n")            \
                           TEXT("Packets Received Non-Guaranteed=%d\r\n")          \
                           TEXT("Messages Received=%d\r\n")                      \
                                                                            \
                           TEXT("Loss Rate=%d.%02d%%\r\n"),
                            
                           dwHighMessages, dwHighBytes,
                           dwNormalMessages, dwNormalBytes,
                           dwLowMessages, dwLowBytes,

                           pConnectionInfo->dwRoundTripLatencyMS, 
                           pConnectionInfo->dwThroughputBPS, 
                           pConnectionInfo->dwPeakThroughputBPS,

                           pConnectionInfo->dwBytesSentGuaranteed,
                           pConnectionInfo->dwPacketsSentGuaranteed,
                           pConnectionInfo->dwBytesSentNonGuaranteed,
                           pConnectionInfo->dwPacketsSentNonGuaranteed,

                           pConnectionInfo->dwBytesRetried,
                           pConnectionInfo->dwPacketsRetried,
                           pConnectionInfo->dwBytesDropped,
                           pConnectionInfo->dwPacketsDropped,

                           pConnectionInfo->dwMessagesTransmittedHighPriority,
                           pConnectionInfo->dwMessagesTimedOutHighPriority,
                           pConnectionInfo->dwMessagesTransmittedNormalPriority,
                           pConnectionInfo->dwMessagesTimedOutNormalPriority,
                           pConnectionInfo->dwMessagesTransmittedLowPriority,
                           pConnectionInfo->dwMessagesTimedOutLowPriority,

                           pConnectionInfo->dwBytesReceivedGuaranteed,
                           pConnectionInfo->dwPacketsReceivedGuaranteed,
                           pConnectionInfo->dwBytesReceivedNonGuaranteed,
                           pConnectionInfo->dwPacketsReceivedNonGuaranteed,
                           pConnectionInfo->dwMessagesReceived,

                           (nDrops/100), (nDrops % 100) );
    }
    else 
    {
        strText[0] = 0;
    }

    SetWindowText( hEdit, strText );  
    SendMessage( hEdit, EM_LINESCROLL, 0, nThumbIndex );
}




//-----------------------------------------------------------------------------
// Name: UpdateStats()
// Desc: 
//-----------------------------------------------------------------------------
VOID UpdateStats( HWND hDlg )
{
    HRESULT hr;
    static DWORD s_dwLastTime = timeGetTime();
    DWORD dwCurTime = timeGetTime();

    if( (dwCurTime - s_dwLastTime) < 200 )
        return;

    TCHAR strDataIn[32];
    TCHAR strDataOut[32];

    FLOAT fSecondsPassed = (dwCurTime - s_dwLastTime) / 1000.0f;
    FLOAT fDataIn  = g_dwDataRecieved / fSecondsPassed;
    FLOAT fDataOut = g_dwDataSent / fSecondsPassed;

    s_dwLastTime     = dwCurTime;
    g_dwDataRecieved = 0;
    g_dwDataSent     = 0;

    _stprintf( strDataIn,  TEXT("%0.1f BPS"), fDataIn );
    _stprintf( strDataOut, TEXT("%0.1f BPS"), fDataOut );
    SetDlgItemText( hDlg, IDC_ACTUAL_SEND_RATE, strDataOut );
    SetDlgItemText( hDlg, IDC_ACTUAL_RECIEVE_RATE, strDataIn );

    //  If a player has been selected for Connection Info Display we will handle it next
    if( g_pConnInfoTargetPlayer->dpnidPlayer != 0 )
    {
        // Call GetConnectionInfo and display results
        DPN_CONNECTION_INFO dpnConnectionInfo;
        ZeroMemory( &dpnConnectionInfo, sizeof(DPN_CONNECTION_INFO) );
        dpnConnectionInfo.dwSize = sizeof(DPN_CONNECTION_INFO);
        hr = g_pDP->GetConnectionInfo( g_pConnInfoTargetPlayer->dpnidPlayer, 
                                                   &dpnConnectionInfo,
												   0);

        if( SUCCEEDED(hr) )
        {
            DWORD dwHighMessages, dwHighBytes;
            DWORD dwNormalMessages, dwNormalBytes;
            DWORD dwLowMessages, dwLowBytes;

            hr = g_pDP->GetSendQueueInfo( g_pConnInfoTargetPlayer->dpnidPlayer, 
                                          &dwHighMessages, &dwHighBytes, 
                                          DPNGETSENDQUEUEINFO_PRIORITY_HIGH );
            if( FAILED(hr) )
                DXTRACE_ERR( TEXT("GetSendQueueInfo"), hr );

            hr = g_pDP->GetSendQueueInfo( g_pConnInfoTargetPlayer->dpnidPlayer, 
                                     &dwNormalMessages, &dwNormalBytes, 
                                     DPNGETSENDQUEUEINFO_PRIORITY_NORMAL );
            if( FAILED(hr) )
                DXTRACE_ERR( TEXT("GetSendQueueInfo"), hr );

            hr = g_pDP->GetSendQueueInfo( g_pConnInfoTargetPlayer->dpnidPlayer, 
                                     &dwLowMessages, &dwLowBytes, 
                                     DPNGETSENDQUEUEINFO_PRIORITY_LOW );
            if( FAILED(hr) )
                DXTRACE_ERR( TEXT("GetSendQueueInfo"), hr );

            UpdateConnectionInfo( hDlg, &dpnConnectionInfo,
                                  dwHighMessages, dwHighBytes,
                                  dwNormalMessages, dwNormalBytes,
                                  dwLowMessages, dwLowBytes );
        }
        else
        {
            // If the player goes away, the set the target to none
            SendDlgItemMessage( hDlg, IDC_CONNINFO_COMBO, CB_SETCURSEL, 0, 0 );  
            ReadCombos( hDlg );
        }
    }
    else 
    {
        // Clear the conn info window if no connection is selected to display        
        UpdateConnectionInfo( hDlg, NULL, 0, 0, 0, 0, 0, 0 );
    }
}


