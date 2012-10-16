//----------------------------------------------------------------------------
// File: VoiceClient.cpp
//
// Desc: The VoiceClientServer sample is a simple DirectPlay 
//       voice-based client/server application. 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commctrl.h>
#include <cguid.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <dplobby8.h>
#include <dxerr8.h>
#include <tchar.h>
#include "DXUtil.h"
#include "NetVoice.h"
#include "VoiceClientServer.h"
#include "NetClient.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// player struct locking defines
//-----------------------------------------------------------------------------
CRITICAL_SECTION g_csPlayerContext;
#define PLAYER_LOCK()                   EnterCriticalSection( &g_csPlayerContext ); 
#define PLAYER_ADDREF( pPlayerInfo )    if( pPlayerInfo ) pPlayerInfo->lRefCount++;
#define PLAYER_RELEASE( pPlayerInfo )   if( pPlayerInfo ) { pPlayerInfo->lRefCount--; if( pPlayerInfo->lRefCount <= 0 ) DestoryPlayerStruct( pPlayerInfo ); } pPlayerInfo = NULL;
#define PLAYER_UNLOCK()                 LeaveCriticalSection( &g_csPlayerContext );


//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
struct APP_PLAYER_INFO
{
    LONG  lRefCount;                        // Ref count so we can cleanup when all threads 
                                            // are done w/ this object
    DPNID dpnidPlayer;                      // DPNID of player
    BOOL  bTalking;                         // Is the player talking
    BOOL  bHalfDuplex;                      // If true, then player cannot talk
    TCHAR strPlayerName[MAX_PLAYER_NAME];   // Player name

    APP_PLAYER_INFO* pNext;
    APP_PLAYER_INFO* pPrev;
};




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Client*     g_pDPClient                   = NULL;    // DirectPlay peer object
CNetClientWizard*       g_pNetClientWizard            = NULL;    // Connection wizard
IDirectPlay8LobbiedApplication* g_pLobbiedApp         = NULL;    // DirectPlay lobbied app 
CNetVoice*              g_pNetVoice                   = NULL;    // DirectPlay voice helper class
BOOL                    g_bWasLobbyLaunched           = FALSE;   // TRUE if lobby launched
HINSTANCE               g_hInst                       = NULL;    // HINST of app
HWND                    g_hDlg                        = NULL;    // HWND of main dialog
LONG                    g_lNumberOfActivePlayers      = 0;       // Number of players currently in game
DPNID                   g_dpnidLocalPlayer            = 0;
APP_PLAYER_INFO         g_playerHead;
TCHAR                   g_strAppName[256]             = TEXT("VoiceClient");
HRESULT                 g_hrDialog;                              // Exit code for app 
TCHAR                   g_strLocalPlayerName[MAX_PATH];          // Local player name
DVCLIENTCONFIG          g_dvClientConfig;                        // Voice client config
BOOL                    g_bMixingSessionType          = FALSE;   // TRUE if the server is mixing, otherwise its forwarding.




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI   DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayLobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayVoiceClientMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK SampleDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK VoiceConfigDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
VOID     OnInitDialog( HWND hDlg );
HRESULT  InitDirectPlay();
VOID     AppendTextToEditControl( HWND hDlg, TCHAR* strNewLogLine );
HRESULT  GetPlayerStruct( DPNID dpnidPlayer, APP_PLAYER_INFO** ppPlayerInfo );
VOID     DestoryPlayerStruct( APP_PLAYER_INFO* pPlayerInfo );
VOID     AddPlayerStruct( APP_PLAYER_INFO* pPlayerInfo );
HRESULT  DisplayPlayersInChat( HWND hDlg );
VOID     VoiceConfigDlgOnOK( HWND hDlg );
void     SetPlayerTalking( APP_PLAYER_INFO* pPlayerInfo, BOOL bTalking );




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

    InitCommonControls();

    ZeroMemory( &g_playerHead, sizeof(APP_PLAYER_INFO) );
    g_playerHead.pNext = &g_playerHead;
    g_playerHead.pPrev = &g_playerHead;

    g_hInst = hInst; 
    InitializeCriticalSection( &g_csPlayerContext );

    // Read persistent state information from registry
    RegCreateKeyEx( HKEY_CURRENT_USER, DPLAY_SAMPLE_KEY, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    &hDPlaySampleRegKey, NULL );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), 
                             g_strLocalPlayerName, MAX_PATH, TEXT("TestPlayer") );

    // Init COM so we can use CoCreateInstance
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    // Create helper class
    g_pNetClientWizard = new CNetClientWizard( hInst, g_strAppName, &g_guidApp );
    g_pNetVoice        = new CNetVoice( DirectPlayVoiceClientMessageHandler, NULL );

    if( FAILED( hr = InitDirectPlay() ) )
    {
        DXTRACE_ERR( TEXT("InitDirectPlay"), hr );
        MessageBox( NULL, TEXT("Failed initializing IDirectPlay8Peer. ")
                    TEXT("The sample will now quit."),
                    g_strAppName, MB_OK | MB_ICONERROR );
        return FALSE;
    }

    // If we were launched from a lobby client, then we may have connection settings
    // that we can use either host or join a game.  If not, then we'll need to prompt 
    // the user to detrimine how to connect.
    if( g_bWasLobbyLaunched && g_pNetClientWizard->HaveConnectionSettingsFromLobby() )
    {
        // If were lobby launched then the DPL_MSGID_CONNECT has already been
        // handled, and since the lobby client also sent us connection settings
        // we can use them to either host or join a DirectPlay session. 
        if( FAILED( hr = g_pNetClientWizard->ConnectUsingLobbySettings() ) )
        {
            DXTRACE_ERR( TEXT("ConnectUsingLobbySettings"), hr );
            MessageBox( NULL, TEXT("Failed to connect using lobby settings. ")
                        TEXT("The sample will now quit."),
                        g_strAppName, MB_OK | MB_ICONERROR );

            bConnectSuccess = FALSE;
        }
        else
        {
            // Read information from g_pNetClientWizard
            _tcscpy( g_strLocalPlayerName, g_pNetClientWizard->GetPlayerName() );

            bConnectSuccess = TRUE; 
        }
    }
    else
    {
        // If not lobby launched, prompt the user about the network 
        // connection and which session they would like to join or 
        // if they want to create a new one.

        // Setup connection wizard
        g_pNetClientWizard->SetPlayerName( g_strLocalPlayerName );

        // Start a connection wizard.  The wizard uses GDI dialog boxes.
        // More complex games can use this as a starting point and add a 
        // fancier graphics layer such as Direct3D.
        hr = g_pNetClientWizard->DoConnectWizard();        
        if( FAILED( hr ) ) 
        {
            DXTRACE_ERR( TEXT("DoConnectWizard"), hr );
            MessageBox( NULL, TEXT("Multiplayer connect failed. ")
                        TEXT("The sample will now quit."),
                        g_strAppName, MB_OK | MB_ICONERROR );
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

            // Read information from g_pNetClientWizard
            _tcscpy( g_strLocalPlayerName, g_pNetClientWizard->GetPlayerName() );

            // Write information to the registry
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), g_strLocalPlayerName );
        }
    }

    if( bConnectSuccess )
    {
        // Set default DirectPlayVoice setup options
        ZeroMemory( &g_dvClientConfig, sizeof(g_dvClientConfig) );
        g_dvClientConfig.dwSize                 = sizeof(g_dvClientConfig);
        g_dvClientConfig.dwFlags                = DVCLIENTCONFIG_AUTOVOICEACTIVATED |
                                                  DVCLIENTCONFIG_AUTORECORDVOLUME;
        g_dvClientConfig.lPlaybackVolume        = DVPLAYBACKVOLUME_DEFAULT;
        g_dvClientConfig.dwBufferQuality        = DVBUFFERQUALITY_DEFAULT;
        g_dvClientConfig.dwBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT;
        g_dvClientConfig.dwThreshold            = DVTHRESHOLD_UNUSED;
        g_dvClientConfig.lRecordVolume          = DVRECORDVOLUME_LAST;
        g_dvClientConfig.dwNotifyPeriod         = 0;

        // App is now connected via DirectPlay, so start the game.  

        // For this sample, we just start a simple dialog box game.
        g_hrDialog = S_OK;
        DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN_GAME), NULL, 
                   (DLGPROC) SampleDlgProc );

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

    // Disconnect from the DirectPlayVoice session, 
    // and destory it if we are the host player.
    SAFE_DELETE( g_pNetVoice ); 

    // Cleanup DirectPlay and helper classes
    if( g_pDPClient )
    {
        g_pDPClient->Close( 0 );
        SAFE_RELEASE( g_pDPClient );
    }

    if( g_pLobbiedApp )
    {
        g_pLobbiedApp->Close( 0 );
        SAFE_RELEASE( g_pLobbiedApp );
    }

    // Don't delete the wizard until we know that 
    // DirectPlay is out of its message handlers.
    // This will be true after Close() has been called. 
    SAFE_DELETE( g_pNetClientWizard );

    RegCloseKey( hDPlaySampleRegKey );
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

    // Create IDirectPlay8Client
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Client, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Client, 
                                       (LPVOID*) &g_pDPClient ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Create IDirectPlay8LobbiedApplication
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8LobbiedApplication, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8LobbiedApplication, 
                                       (LPVOID*) &g_pLobbiedApp ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Init the helper class, now that g_pDP and g_pLobbiedApp are valid
    g_pNetClientWizard->Init( g_pDPClient, g_pLobbiedApp );

    // Init IDirectPlay8Client
    if( FAILED( hr = g_pDPClient->Initialize( NULL, DirectPlayMessageHandler, 0 ) ) )
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
INT_PTR CALLBACK SampleDlgProc( HWND hDlg, UINT msg, 
                                WPARAM wParam, LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
        {
            OnInitDialog( hDlg );
            break;
        }

        case WM_APP_DISPLAY_PLAYERS:
        {
            DisplayPlayersInChat( hDlg );
            break;
        }

        case WM_TIMER:
        {
            DWORD            dwNumPlayers;
            DWORD            iIndex;
            LVITEM           lvItem;
            APP_PLAYER_INFO* pPlayerInfo = NULL;
            HWND             hListView = GetDlgItem( hDlg, IDC_PEOPLE_LIST );

            dwNumPlayers = ListView_GetItemCount( hListView );

            // Now that they are added and the listview sorted them by name,
            // run through them all caching the listview index with its dpnid
            for( iIndex = 0; iIndex < dwNumPlayers; iIndex++ )
            {
                HRESULT hr;
                APP_PLAYER_INFO* pPlayerInfo = NULL;
                DPNID dpnidPlayer;

                lvItem.mask  = LVIF_PARAM;
                lvItem.iItem = iIndex;
                ListView_GetItem( hListView, &lvItem );

                dpnidPlayer = (DPNID) lvItem.lParam;

                PLAYER_LOCK(); // enter player context CS
                hr = GetPlayerStruct( dpnidPlayer, &pPlayerInfo );
                PLAYER_ADDREF( pPlayerInfo ); // addref player, since we are using it now
                PLAYER_UNLOCK(); // leave player context CS

                if( FAILED(hr) || pPlayerInfo == NULL )
                {
                    // The player who sent this may have gone away before this 
                    // message was handled, so just ignore it
                    continue;
                }

                TCHAR strStatus[255];

                if( pPlayerInfo->bHalfDuplex )
                {
                    _tcscpy( strStatus, TEXT("Can't talk") );
                }
                else
                {
                    if( g_bMixingSessionType )
                    {
                        // With mixing servers, you can't tell which
                        // client is talking.
                        _tcscpy( strStatus, TEXT("n/a") );
                    }
                    else
                    {
                        if( pPlayerInfo->bTalking )
                            _tcscpy( strStatus, TEXT("Talking") );
                        else
                            _tcscpy( strStatus, TEXT("Silent") );
                    }
                }

                lvItem.iItem      = iIndex;
                lvItem.iSubItem   = 1;
                lvItem.mask       = LVIF_TEXT;
                lvItem.pszText    = strStatus;

                PLAYER_LOCK();
                PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
                PLAYER_UNLOCK();

                SendMessage( hListView, LVM_SETITEM, 0, (LPARAM) &lvItem );
            }
            break;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDC_SETUP:
                    {
                        // Ask the user for DirectPlayVoice setup params
                        DWORD dwResult = (DWORD)DialogBox( g_hInst, 
                                                           MAKEINTRESOURCE(IDD_VOICE_SETUP), 
                                                           hDlg, (DLGPROC) VoiceConfigDlgProc );
                        if( dwResult != IDCANCEL )
                            g_pNetVoice->ChangeVoiceClientSettings( &g_dvClientConfig );
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
// Name: OnInitDialog()
// Desc: Inits the dialog 
//-----------------------------------------------------------------------------
VOID OnInitDialog( HWND hDlg )
{
    LVCOLUMN column;
    RECT     rctListView;
    TCHAR    strHeader[255];
    DWORD    dwVertScrollBar;
    DWORD    dwListViewWidth;

    g_hDlg = hDlg;

    // Load and set the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    SetWindowText( hDlg, g_strAppName );

    // Display local player's name
    SetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strLocalPlayerName );

    // Setup the listview
    HWND hListView = GetDlgItem( hDlg, IDC_PEOPLE_LIST );
    dwVertScrollBar = GetSystemMetrics( SM_CXVSCROLL );  
    GetClientRect( hListView, &rctListView );
    dwListViewWidth = rctListView.right - dwVertScrollBar;
    column.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.fmt      = LVCFMT_LEFT;
    column.iSubItem = -1;

    // Insert the Name column
    _tcscpy( strHeader, TEXT("Name") );
    column.cx         = dwListViewWidth * 5 / 10;
    column.pszText    = strHeader;
    column.cchTextMax = _tcslen( strHeader );
    ListView_InsertColumn( hListView, 0, &column );

    // Insert the Status column
    _tcscpy( strHeader, TEXT("Status") );
    column.cx         = dwListViewWidth * 5 / 10;
    column.pszText    = strHeader;
    column.cchTextMax = _tcslen( strHeader );
    ListView_InsertColumn( hListView, 1, &column );

    // Connect to the DirectPlay Voice session
    if( FAILED( g_hrDialog = g_pNetVoice->Init( hDlg, FALSE, TRUE,
                                                g_pDPClient, DVSESSIONTYPE_FORWARDING, 
                                                NULL, &g_dvClientConfig ) ) )
    {
        if( g_hrDialog == DVERR_USERBACK )
        {
            MessageBox( hDlg, TEXT("The user backed out of the wizard.  ")
                        TEXT("This simple sample does not handle this case, so ")
                        TEXT("the sample will quit."), TEXT("DirectPlay Sample"), MB_OK );
            g_hrDialog = S_OK;
        }

        if( g_hrDialog == DVERR_USERCANCEL )
        {
            MessageBox( hDlg, TEXT("The user canceled the wizard. ")
                        TEXT("This simple sample does not handle this case, so ")
                        TEXT("the sample will quit."), TEXT("DirectPlay Sample"), MB_OK );
            g_hrDialog = S_OK;
        }

        if( g_hrDialog == DVERR_ALREADYPENDING )
        {
            MessageBox( hDlg, TEXT("Another instance of the Voice Setup Wizard is already running. ")
                        TEXT("This simple sample does not handle this case, so ")
                        TEXT("the sample will quit."), TEXT("DirectPlay Sample"), MB_OK );
            g_hrDialog = S_OK;
        }

        if( FAILED(g_hrDialog) ) 
            DXTRACE_ERR( TEXT("Init"), g_hrDialog );

        EndDialog( hDlg, 0 );
        return;
    }

    // Get the session description to figure out if we are connected to a mixing server
    IDirectPlayVoiceClient* pVoiceClient = g_pNetVoice->GetVoiceClient();
	DVSESSIONDESC dvsd;
    ZeroMemory( &dvsd, sizeof(DVSESSIONDESC) );
    dvsd.dwSize = sizeof(DVSESSIONDESC);
    pVoiceClient->GetSessionDesc( &dvsd );
    g_bMixingSessionType = (dvsd.dwSessionType == DVSESSIONTYPE_MIXING);

    // Display a warning to the user if they are in half duplex mode
    if( g_pNetVoice->IsHalfDuplex() ) 
    {
        MessageBox( hDlg, TEXT("You are running in half duplex mode. ")
                    TEXT("In half duplex mode no recording takes place."), 
                    TEXT("DirectPlay Sample"), MB_OK );
    }

    // Make a timer to update the listbox 
    // 'Status' column every so often 
    SetTimer( hDlg, 0, 250, NULL );

    PostMessage( hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );
}




//-----------------------------------------------------------------------------
// Name: DisplayPlayersInChat()
// Desc: Displays the active players in the listview
//-----------------------------------------------------------------------------
HRESULT DisplayPlayersInChat( HWND hDlg )
{
    if( hDlg == NULL )
        return S_OK;

    LVITEM  lvItem;
    HWND    hListView = GetDlgItem( hDlg, IDC_PEOPLE_LIST );
    TCHAR   strStatus[32];
    TCHAR   strNumberPlayers[32];
    DWORD   dwNumPlayers = 0;

    // Remove all the players and re-add them in the player enum callback
    ListView_DeleteAllItems( hListView );

    PLAYER_LOCK(); // enter player context CS

    APP_PLAYER_INFO* pCurPlayer = g_playerHead.pNext;    

    while ( pCurPlayer != &g_playerHead )
    {
        ZeroMemory( &lvItem, sizeof(lvItem) );

        // Add the item, saving the player's name and dpnid in the listview
        lvItem.mask       = LVIF_TEXT | LVIF_PARAM;
        lvItem.iItem      = 0;
        lvItem.iSubItem   = 0;
        lvItem.pszText    = pCurPlayer->strPlayerName;
        lvItem.lParam     = (LONG) pCurPlayer->dpnidPlayer;
        lvItem.cchTextMax = _tcslen( pCurPlayer->strPlayerName );
        int nIndex = ListView_InsertItem( hListView, &lvItem );

        if( pCurPlayer->bHalfDuplex )
        {
            _tcscpy( strStatus, TEXT("Can't talk") );
        }
        else
        {
            if( g_bMixingSessionType )
            {
                // With mixing servers, you can't tell which
                // client is talking.
                _tcscpy( strStatus, TEXT("n/a") );
            }
            else
            {
                if( pCurPlayer->bTalking )
                    _tcscpy( strStatus, TEXT("Talking") );
                else
                    _tcscpy( strStatus, TEXT("Silent") );
            }
        }

        // Start the player's status off as silent.  
        lvItem.mask       = LVIF_TEXT;
        lvItem.iItem      = nIndex;
        lvItem.iSubItem   = 1;
        lvItem.pszText    = strStatus;
        lvItem.cchTextMax = _tcslen( strStatus );
        ListView_SetItem( hListView, &lvItem );

        dwNumPlayers++;
        pCurPlayer = pCurPlayer->pNext;
    }

    PLAYER_UNLOCK(); // leave player context CS

    wsprintf( strNumberPlayers, TEXT("%d"), dwNumPlayers );
    SetDlgItemText( hDlg, IDC_NUM_PLAYERS, strNumberPlayers );

    return S_OK;
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
            HRESULT hr;
            PDPNMSG_RECEIVE pReceiveMsg;
            pReceiveMsg = (PDPNMSG_RECEIVE)pMsgBuffer;

            GAMEMSG_GENERIC* pMsg = (GAMEMSG_GENERIC*) pReceiveMsg->pReceiveData;
            switch( pMsg->dwType )
            {
                case GAME_MSGID_SET_ID:
                {
                    // The host is tell us the DPNID for this client
                    GAMEMSG_SET_ID* pSetIDMsg;
                    pSetIDMsg = (GAMEMSG_SET_ID*)pReceiveMsg->pReceiveData;

                    g_dpnidLocalPlayer = pSetIDMsg->dpnidPlayer;
                    break;
                }

                case GAME_MSGID_CREATE_PLAYER:
                {
                    // The host is telling us about a new player 
                    GAMEMSG_CREATE_PLAYER* pCreatePlayerMsg;
                    pCreatePlayerMsg = (GAMEMSG_CREATE_PLAYER*)pReceiveMsg->pReceiveData;

                    // Create a new and fill in a APP_PLAYER_INFO
                    APP_PLAYER_INFO* pPlayerInfo = new APP_PLAYER_INFO;
                    ZeroMemory( pPlayerInfo, sizeof(APP_PLAYER_INFO) );
                    pPlayerInfo->lRefCount   = 1;
                    pPlayerInfo->dpnidPlayer = pCreatePlayerMsg->dpnidPlayer;
                    _tcscpy( pPlayerInfo->strPlayerName, pCreatePlayerMsg->strPlayerName );

                    PLAYER_LOCK(); // enter player struct CS
                    AddPlayerStruct( pPlayerInfo );
                    PLAYER_UNLOCK(); // leave player struct CS

                    // Update the number of active players, and 
                    // post a message to the dialog thread to update the 
                    // UI.  This keeps the DirectPlay message handler 
                    // from blocking
                    InterlockedIncrement( &g_lNumberOfActivePlayers );

                    if( g_hDlg != NULL )
                        PostMessage( g_hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );
                    break;
                };

                case GAME_MSGID_DESTROY_PLAYER:
                {
                    // The host is telling us about a player that's been destroyed
                    APP_PLAYER_INFO*        pPlayerInfo = NULL;
                    GAMEMSG_DESTROY_PLAYER* pDestroyPlayerMsg;
                    pDestroyPlayerMsg = (GAMEMSG_DESTROY_PLAYER*)pReceiveMsg->pReceiveData;

                    // Get the player struct accosicated with this DPNID
                    PLAYER_LOCK(); // enter player struct CS
                    hr = GetPlayerStruct( pDestroyPlayerMsg->dpnidPlayer, &pPlayerInfo );
                    PLAYER_UNLOCK(); // leave player struct CS

                    if( FAILED(hr) || pPlayerInfo == NULL )
                    {
                        // The player who sent this may have gone away before this 
                        // message was handled, so just ignore it
                        break;
                    }
            
                    // Release the player struct
                    PLAYER_LOCK();                  // enter player struct CS
                    PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
                    PLAYER_UNLOCK();                // leave player struct CS

                    // Update the number of active players, and 
                    // post a message to the dialog thread to update the 
                    // UI.  This keeps the DirectPlay message handler 
                    // from blocking
                    InterlockedDecrement( &g_lNumberOfActivePlayers );

                    if( g_hDlg != NULL )
                        PostMessage( g_hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );
                    break;
                };
            }

            break;
        }
    }

    // Make sure the DirectPlay MessageHandler calls the CNetConnectWizard handler, 
    // so it can be informed of messages such as DPN_MSGID_ENUM_HOSTS_RESPONSE.
    if( g_pNetClientWizard )
        g_pNetClientWizard->MessageHandler( pvUserContext, dwMessageId, pMsgBuffer );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: AddPlayerStruct()
// Desc: Add pPlayerInfo to the circular linked list, g_playerHead
//-----------------------------------------------------------------------------
VOID AddPlayerStruct( APP_PLAYER_INFO* pPlayerInfo )
{
    pPlayerInfo->pNext = g_playerHead.pNext;
    pPlayerInfo->pPrev = &g_playerHead;

    g_playerHead.pNext->pPrev = pPlayerInfo;    
    g_playerHead.pNext = pPlayerInfo;    
}




//-----------------------------------------------------------------------------
// Name: DestoryPlayerStruct() 
// Desc: Remove pPlayerInfo from the circular linked list, g_playerHead
//-----------------------------------------------------------------------------
VOID DestoryPlayerStruct( APP_PLAYER_INFO* pPlayerInfo )
{
    pPlayerInfo->pNext->pPrev = pPlayerInfo->pPrev;
    pPlayerInfo->pPrev->pNext = pPlayerInfo->pNext;

    SAFE_DELETE( pPlayerInfo );
}




//-----------------------------------------------------------------------------
// Name: GetPlayerStruct() 
// Desc: Searchs the circular linked list, g_playerHead, for dpnidPlayer
//-----------------------------------------------------------------------------
HRESULT GetPlayerStruct( DPNID dpnidPlayer, APP_PLAYER_INFO** ppPlayerInfo )
{
    if( ppPlayerInfo == NULL )
        return E_FAIL;

    APP_PLAYER_INFO* pCurPlayer = g_playerHead.pNext;
    
    *ppPlayerInfo = NULL;
    while ( pCurPlayer != &g_playerHead )
    {
        if( pCurPlayer->dpnidPlayer == dpnidPlayer )
        {
            *ppPlayerInfo = pCurPlayer;
            return S_OK;
        }
            
        pCurPlayer = pCurPlayer->pNext;
    }

    // Not found.
    return E_FAIL;
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
    if( g_pNetClientWizard )
        return g_pNetClientWizard->LobbyMessageHandler( pvUserContext, dwMessageId, 
                                                         pMsgBuffer );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DirectPlayVoiceClientMessageHandler()
// Desc: The callback for DirectPlayVoice client messages.  
//       This handles client messages and updates the UI the whenever a client 
//       starts or stops talking.  
//-----------------------------------------------------------------------------
HRESULT CALLBACK DirectPlayVoiceClientMessageHandler( LPVOID lpvUserContext, DWORD dwMessageType,
                                                      LPVOID lpMessage )
{
    // Try not to stay in this message handler for too long, otherwise
    // there will be a backlog of data.  The best solution is to 
    // queue data as it comes in, and then handle it on other threads.
    
    // This function is called by the DirectPlay message handler pool of 
    // threads, so be care of thread synchronization problems with shared memory
    HRESULT hr;
    HWND hDlg = (HWND) lpvUserContext;

    DPNID dpnidPlayer;
    BOOL bTalking;

    switch( dwMessageType )
    {
        case DVMSGID_SESSIONLOST:
            g_hrDialog = DPNERR_CONNECTIONLOST;
            EndDialog( hDlg, 0 );
            break;

        case DVMSGID_GAINFOCUS:
        case DVMSGID_LOSTFOCUS:
        {
            TCHAR strWindowName[MAX_PATH];
            wsprintf( strWindowName, TEXT("%s%s"), g_strAppName,
                (dwMessageType == DVMSGID_LOSTFOCUS) ? TEXT(" (Focus Lost)") : TEXT("") );

            SetWindowText( hDlg, strWindowName );
            break;
        }

        case DVMSGID_RECORDSTART:             
		{ 
			DVMSG_RECORDSTART* pMsg = (DVMSG_RECORDSTART*) lpMessage;
            dpnidPlayer = g_dpnidLocalPlayer;
            bTalking    = TRUE;
            break;
        }

        case DVMSGID_RECORDSTOP:             
        {
			DVMSG_RECORDSTOP* pMsg = (DVMSG_RECORDSTOP*) lpMessage;
            dpnidPlayer = g_dpnidLocalPlayer;
            bTalking    = FALSE;
            break;
        }

        case DVMSGID_PLAYERVOICESTART:
        {
            DVMSG_PLAYERVOICESTART* pMsg = (DVMSG_PLAYERVOICESTART*) lpMessage;
            dpnidPlayer = pMsg->dvidSourcePlayerID;
            bTalking    = TRUE;
            break;
        }

        case DVMSGID_PLAYERVOICESTOP:
        {
            DVMSG_PLAYERVOICESTOP* pMsg = (DVMSG_PLAYERVOICESTOP*) lpMessage;
            dpnidPlayer = pMsg->dvidSourcePlayerID;
            bTalking    = FALSE;
            break;
        }
    }

    // With a mixing sever, the client won't recieve DVMSGID_RECORDSTART/STOP msgs.  
    // It will still receieve DVMSGID_PLAYERVOICESTART/STOP messages whenever
    // audio occurs, however the messages do not identify the source.  So
    // in this sample, if the server is in mixing mode then it just reports
    // 'n/a' for the players.  This could be done differently in more complex apps.
    if( g_bMixingSessionType )
        return S_OK;

    switch( dwMessageType )
    {
        case DVMSGID_RECORDSTART:             
        case DVMSGID_RECORDSTOP:             
        case DVMSGID_PLAYERVOICESTART:
        case DVMSGID_PLAYERVOICESTOP:
        {
            APP_PLAYER_INFO* pPlayerInfo = NULL;

            PLAYER_LOCK(); // enter player context CS
            hr = GetPlayerStruct( dpnidPlayer, &pPlayerInfo );

            if( FAILED(hr) || pPlayerInfo == NULL )
            {
                // The player who sent this may have gone away before this 
                // message was handled, so just ignore it
                PLAYER_UNLOCK(); // leave player context CS
                break;
            }

            // Update the talking status
            pPlayerInfo->bTalking = bTalking; 	

            PLAYER_UNLOCK(); // leave player context CS
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: VoiceConfigDlgProc()
// Desc: Prompt the user for DirectPlayVoice setup options
//-----------------------------------------------------------------------------
INT_PTR CALLBACK VoiceConfigDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    DWORD dwSliderPos;

    switch( msg ) 
    {
        case WM_INITDIALOG:
            // Set the range on the sliders
            SendDlgItemMessage( hDlg, IDC_PLAYBACK_SLIDER,       TBM_SETRANGE, FALSE, MAKELONG( 0, 100 ) );
            SendDlgItemMessage( hDlg, IDC_RECORD_SLIDER,         TBM_SETRANGE, FALSE, MAKELONG( 0, 100 ) );
            SendDlgItemMessage( hDlg, IDC_QUALITY_SLIDER,        TBM_SETRANGE, FALSE, MAKELONG( DVBUFFERQUALITY_MIN, DVBUFFERQUALITY_MAX ) );
            SendDlgItemMessage( hDlg, IDC_THRESHOLD_SLIDER,    TBM_SETRANGE, FALSE, MAKELONG( DVTHRESHOLD_MIN,  DVTHRESHOLD_MAX ) );
            SendDlgItemMessage( hDlg, IDC_AGGRESSIVENESS_SLIDER, TBM_SETRANGE, FALSE, MAKELONG( DVBUFFERAGGRESSIVENESS_MIN, DVBUFFERAGGRESSIVENESS_MAX ) );

            // Setup the dialog based on the globals 

            // Set the playback controls
            if( g_dvClientConfig.lPlaybackVolume == DVPLAYBACKVOLUME_DEFAULT )
            {
                CheckRadioButton( hDlg, IDC_PLAYBACK_DEFAULT, IDC_PLAYBACK_SET, IDC_PLAYBACK_DEFAULT );
            }
            else
            {
                dwSliderPos = (DWORD) ( ( g_dvClientConfig.lPlaybackVolume - DSBVOLUME_MIN ) * 
                                          100.0f / (DSBVOLUME_MAX-DSBVOLUME_MIN) );
                CheckRadioButton( hDlg, IDC_PLAYBACK_DEFAULT, IDC_PLAYBACK_SET, IDC_PLAYBACK_SET );
                SendDlgItemMessage( hDlg, IDC_PLAYBACK_SLIDER, TBM_SETPOS, TRUE, dwSliderPos );
            }

            // Set the record controls
            if( g_dvClientConfig.dwFlags & DVCLIENTCONFIG_AUTORECORDVOLUME )
            {
                CheckRadioButton( hDlg, IDC_RECORD_DEFAULT, IDC_RECORD_AUTO, IDC_RECORD_AUTO );
            }
            else if( g_dvClientConfig.lRecordVolume == DVPLAYBACKVOLUME_DEFAULT )
            {
                CheckRadioButton( hDlg, IDC_RECORD_DEFAULT, IDC_RECORD_AUTO, IDC_RECORD_DEFAULT );
            }
            else
            {
                dwSliderPos = (DWORD) ( ( g_dvClientConfig.lRecordVolume - DSBVOLUME_MIN ) * 
                                          100.0f / (DSBVOLUME_MAX-DSBVOLUME_MIN) );
                CheckRadioButton( hDlg, IDC_RECORD_DEFAULT, IDC_RECORD_AUTO, IDC_RECORD_SET );
                SendDlgItemMessage( hDlg, IDC_RECORD_SLIDER, TBM_SETPOS, TRUE, dwSliderPos );
            }

            // Set the threshold controls
            if( g_dvClientConfig.dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED )
            {
                CheckRadioButton( hDlg, IDC_THRESHOLD_DEFAULT, IDC_THRESHOLD_AUTO, IDC_THRESHOLD_AUTO );
            }
            else if( g_dvClientConfig.dwThreshold == DVTHRESHOLD_DEFAULT )
            {
                CheckRadioButton( hDlg, IDC_THRESHOLD_DEFAULT, IDC_THRESHOLD_AUTO, IDC_THRESHOLD_DEFAULT );
            }
            else
            {
                CheckRadioButton( hDlg, IDC_THRESHOLD_DEFAULT, IDC_THRESHOLD_AUTO, IDC_THRESHOLD_SET );
                SendDlgItemMessage( hDlg, IDC_THRESHOLD_SLIDER, TBM_SETPOS, TRUE, g_dvClientConfig.dwThreshold );
            }

            // Set the quality controls
            if( g_dvClientConfig.dwBufferQuality == DVBUFFERQUALITY_DEFAULT )
            {
                CheckRadioButton( hDlg, IDC_QUALITY_DEFAULT, IDC_QUALITY_SET, IDC_QUALITY_DEFAULT );
            }
            else
            {
                CheckRadioButton( hDlg, IDC_QUALITY_DEFAULT, IDC_QUALITY_SET, IDC_QUALITY_SET );
                SendDlgItemMessage( hDlg, IDC_QUALITY_SLIDER, TBM_SETPOS, TRUE, g_dvClientConfig.dwBufferQuality );
            }

            // Set the aggressiveness controls
            if( g_dvClientConfig.dwBufferAggressiveness == DVBUFFERAGGRESSIVENESS_DEFAULT )
            {
                CheckRadioButton( hDlg, IDC_AGGRESSIVENESS_DEFAULT, IDC_AGGRESSIVENESS_SET, IDC_AGGRESSIVENESS_DEFAULT );
            }
            else
            {
                CheckRadioButton( hDlg, IDC_AGGRESSIVENESS_DEFAULT, IDC_AGGRESSIVENESS_SET, IDC_AGGRESSIVENESS_SET );
                SendDlgItemMessage( hDlg, IDC_AGGRESSIVENESS_SLIDER, TBM_SETPOS, TRUE, g_dvClientConfig.dwBufferAggressiveness );
            }

            return TRUE;

        case WM_NOTIFY:
            #ifndef NM_RELEASEDCAPTURE
                #define NM_RELEASEDCAPTURE (NM_FIRST-16)
            #endif
            if( ((LPNMHDR) lParam)->code == NM_RELEASEDCAPTURE )
            {
                // If this is a release capture from a slider, then automatically check 
                // its 'Set' radio button.
                switch( ((LPNMHDR) lParam)->idFrom )
                {
                case IDC_PLAYBACK_SLIDER:
                    CheckRadioButton( hDlg, IDC_PLAYBACK_DEFAULT, IDC_PLAYBACK_SET, IDC_PLAYBACK_SET );
                    break;
    
                case IDC_RECORD_SLIDER:
                    CheckRadioButton( hDlg, IDC_RECORD_DEFAULT, IDC_RECORD_AUTO, IDC_RECORD_SET );
                    break;
    
                case IDC_THRESHOLD_SLIDER:
                    CheckRadioButton( hDlg, IDC_THRESHOLD_DEFAULT, IDC_THRESHOLD_AUTO, IDC_THRESHOLD_SET );
                    break;
    
                case IDC_QUALITY_SLIDER:
                    CheckRadioButton( hDlg, IDC_QUALITY_DEFAULT, IDC_QUALITY_SET, IDC_QUALITY_SET );
                    break;
    
                case IDC_AGGRESSIVENESS_SLIDER:
                    CheckRadioButton( hDlg, IDC_AGGRESSIVENESS_DEFAULT, IDC_AGGRESSIVENESS_SET, IDC_AGGRESSIVENESS_SET );
                    break;
                }
            }
            return TRUE;            

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDOK:
                    VoiceConfigDlgOnOK( hDlg );
                    return TRUE;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    return TRUE;
            }
            break;
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: VoiceConfigDlgOnOK()
// Desc: Figure out all the DirectPlayVoice setup params from the dialog box,
//       and store them in global vars.
//-----------------------------------------------------------------------------
VOID VoiceConfigDlgOnOK( HWND hDlg )
{
    DWORD dwSliderPos;

    g_dvClientConfig.dwFlags = 0;

    // Figure out the playback params
    if( IsDlgButtonChecked( hDlg, IDC_PLAYBACK_DEFAULT ) )
    {
        g_dvClientConfig.lPlaybackVolume = DVPLAYBACKVOLUME_DEFAULT;
    }
    else 
    {
        dwSliderPos = (DWORD)SendDlgItemMessage( hDlg, IDC_PLAYBACK_SLIDER, TBM_GETPOS, 0, 0 );
        g_dvClientConfig.lPlaybackVolume = DSBVOLUME_MIN + (LONG) ( dwSliderPos / 100.0f * 
                                                                    (DSBVOLUME_MAX-DSBVOLUME_MIN) );
    }

    // Figure out the record params
    if( IsDlgButtonChecked( hDlg, IDC_RECORD_AUTO ) )
    {
        g_dvClientConfig.lRecordVolume = 0;
        g_dvClientConfig.dwFlags       |= DVCLIENTCONFIG_AUTORECORDVOLUME;
    }
    else if( IsDlgButtonChecked( hDlg, IDC_RECORD_DEFAULT ) )
    {
        g_dvClientConfig.lRecordVolume = DVPLAYBACKVOLUME_DEFAULT;
    }
    else 
    {
        dwSliderPos = (DWORD)SendDlgItemMessage( hDlg, IDC_RECORD_SLIDER, TBM_GETPOS, 0, 0 );
        g_dvClientConfig.lRecordVolume = DSBVOLUME_MIN + (LONG) ( dwSliderPos / 100.0f * 
                                                                  (DSBVOLUME_MAX-DSBVOLUME_MIN) );
    }

    // Figure out the threshold params
    if( IsDlgButtonChecked( hDlg, IDC_THRESHOLD_AUTO ) )
    {
        g_dvClientConfig.dwThreshold = DVTHRESHOLD_UNUSED;
        g_dvClientConfig.dwFlags       |= DVCLIENTCONFIG_AUTOVOICEACTIVATED;
    }
    else if( IsDlgButtonChecked( hDlg, IDC_THRESHOLD_DEFAULT ) )
    {
        g_dvClientConfig.dwThreshold = DVTHRESHOLD_DEFAULT;
        g_dvClientConfig.dwFlags       |= DVCLIENTCONFIG_MANUALVOICEACTIVATED;
    }
    else 
    {
        dwSliderPos = (DWORD)SendDlgItemMessage( hDlg, IDC_THRESHOLD_SLIDER, TBM_GETPOS, 0, 0 );
        g_dvClientConfig.dwThreshold = dwSliderPos;
        g_dvClientConfig.dwFlags       |= DVCLIENTCONFIG_MANUALVOICEACTIVATED;
    }

    // Figure out the quality params
    if( IsDlgButtonChecked( hDlg, IDC_QUALITY_DEFAULT ) )
    {
        g_dvClientConfig.dwBufferQuality = DVBUFFERQUALITY_DEFAULT;
    }
    else 
    {
        dwSliderPos = (DWORD)SendDlgItemMessage( hDlg, IDC_QUALITY_SLIDER, TBM_GETPOS, 0, 0 );
        g_dvClientConfig.dwBufferQuality = dwSliderPos;
    }

    // Figure out the aggressiveness params
    if( IsDlgButtonChecked( hDlg, IDC_AGGRESSIVENESS_DEFAULT ) )
    {
        g_dvClientConfig.dwBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT;
    }
    else 
    {
        dwSliderPos = (DWORD)SendDlgItemMessage( hDlg, IDC_AGGRESSIVENESS_SLIDER, TBM_GETPOS, 0, 0 );
        g_dvClientConfig.dwBufferAggressiveness = dwSliderPos;
    }

    EndDialog( hDlg, IDOK );
}


