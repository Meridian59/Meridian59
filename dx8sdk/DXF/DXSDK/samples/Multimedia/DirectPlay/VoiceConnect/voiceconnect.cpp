//----------------------------------------------------------------------------
// File: VoiceConnect.cpp
//
// Desc: The main file for VoiceConnect that shows how use DirectPlay along 
//       with DirectPlayVoice to allow talking in a conference situation. 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <dxerr8.h>
#include <dplay8.h>
#include <dplobby8.h>
#include <dvoice.h>
#include <commctrl.h>
#include <cguid.h>
#include "NetConnect.h"
#include "NetVoice.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Player context locking defines
//-----------------------------------------------------------------------------
CRITICAL_SECTION g_csPlayerContext;
#define PLAYER_LOCK()                   EnterCriticalSection( &g_csPlayerContext ); 
#define PLAYER_ADDREF( pPlayerInfo )    if( pPlayerInfo ) pPlayerInfo->lRefCount++;
#define PLAYER_RELEASE( pPlayerInfo )   if( pPlayerInfo ) { pPlayerInfo->lRefCount--; if( pPlayerInfo->lRefCount <= 0 ) SAFE_DELETE( pPlayerInfo ); }
#define PLAYER_UNLOCK()                 LeaveCriticalSection( &g_csPlayerContext );


//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#define DPLAY_SAMPLE_KEY        TEXT("Software\\Microsoft\\DirectX DirectPlay Samples")
#define MAX_PLAYER_NAME         14
#define WM_APP_DISPLAY_PLAYERS  (WM_APP + 0)

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {FCD4761E-2B9C-4478-B7EA-D9E18E6E2AAC}
GUID g_guidApp = { 0xfcd4761e, 0x2b9c, 0x4478, { 0xb7, 0xea, 0xd9, 0xe1, 0x8e, 0x6e, 0x2a, 0xac } };




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
struct APP_PLAYER_INFO
{
    LONG  lRefCount;                        // Ref count so we can cleanup when all threads 
                                            // are done w/ this object
    DPNID dpnidPlayer;                      // DPNID of player
    BOOL  bTalking;                         // Is the player talking
    BOOL  bHalfDuplex;                      // If true, then player cannot talk
    TCHAR strPlayerName[MAX_PLAYER_NAME];   // Player name
};


IDirectPlay8Peer*  g_pDP                         = NULL;    // DirectPlay peer object
CNetConnectWizard* g_pNetConnectWizard           = NULL;    // Connection wizard
IDirectPlay8LobbiedApplication* g_pLobbiedApp    = NULL;    // DirectPlay lobbied app 
BOOL               g_bWasLobbyLaunched           = FALSE;   // TRUE if lobby launched
CNetVoice*         g_pNetVoice                   = NULL;    // DirectPlay voice helper class
HINSTANCE          g_hInst                       = NULL;    // HINST of app
HWND               g_hDlg                        = NULL;    // HWND of main dialog
TCHAR              g_strAppName[256]             = TEXT("VoiceConnect");
BOOL               g_bVoiceSessionInProgress     = FALSE;   // True if voice has been init'ed
DPNID*             g_pPlayers                    = NULL;    // Array of DPNIDs
DWORD              g_dwPlayersArraySize          = 0;       // Size of g_pPlayers
HRESULT            g_hrDialog;                              // Exit code for app 
TCHAR              g_strLocalPlayerName[MAX_PATH];          // Local player name
TCHAR              g_strSessionName[MAX_PATH];              // Session name
TCHAR              g_strPreferredProvider[MAX_PATH];        // Provider string
BOOL               g_bHostPlayer;                           // TRUE if local player is host
DPNID              g_LocalPlayerDPNID;                      // DPNID of local player
GUID               g_guidDVSessionCT;                       // GUID for choosen voice compression
DVCLIENTCONFIG     g_dvClientConfig;                        // Voice client config




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK VoiceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT          InitDirectPlay();
HRESULT          OnInitDialog( HWND hDlg );
HRESULT          DisplayPlayersInChat( HWND hDlg );
HRESULT WINAPI   DirectPlayMessageHandler( LPVOID lpvUserContext, DWORD dwMessageType, LPVOID lpMessage );
HRESULT WINAPI   DirectPlayLobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayVoiceClientMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayVoiceServerMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
void             SetPlayerTalking( APP_PLAYER_INFO *pPlayerInfo, BOOL bTalking );
INT_PTR CALLBACK VoiceConfigDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT          VoiceConfigEnumCompressionCodecs( HWND hDlg );
VOID             VoiceConfigDlgOnOK( HWND hDlg );




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    HRESULT hr;
    HKEY    hDPlaySampleRegKey;
    BOOL    bConnectSuccess = FALSE;

    g_hInst = hInst;
    InitializeCriticalSection( &g_csPlayerContext );

    // Init player ID array 
    g_dwPlayersArraySize = 10;
    g_pPlayers = (DPNID*) malloc( sizeof(DPNID)*g_dwPlayersArraySize );

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

    InitCommonControls();

    // Init COM so we can use CoCreateInstance
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    // Create helper class
    g_pNetConnectWizard = new CNetConnectWizard( hInst, NULL, g_strAppName, &g_guidApp );
    g_pNetVoice         = new CNetVoice( DirectPlayVoiceClientMessageHandler, DirectPlayVoiceServerMessageHandler );

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
            _tcscpy( g_strLocalPlayerName, g_pNetConnectWizard->GetPlayerName() );
            g_bHostPlayer = g_pNetConnectWizard->IsHostPlayer();

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
            _tcscpy( g_strLocalPlayerName, g_pNetConnectWizard->GetPlayerName() );
            _tcscpy( g_strSessionName, g_pNetConnectWizard->GetSessionName() );
            _tcscpy( g_strPreferredProvider, g_pNetConnectWizard->GetPreferredProvider() );
            g_bHostPlayer = g_pNetConnectWizard->IsHostPlayer();

            // Write information to the registry
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), g_strLocalPlayerName );
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), g_strSessionName );
            DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), g_strPreferredProvider );
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
        g_dvClientConfig.dwThreshold          = DVTHRESHOLD_UNUSED;
        g_dvClientConfig.lRecordVolume          = DVRECORDVOLUME_LAST;
        g_dvClientConfig.dwNotifyPeriod         = 0;

        g_guidDVSessionCT                       = DPVCTGUID_DEFAULT;

        // Ask user to change g_dvClientConfig, g_guidDVSessionCT
        DWORD dwResult = (DWORD)DialogBox( hInst, MAKEINTRESOURCE(IDD_VOICE_SETUP), 
                                           NULL, (DLGPROC) VoiceConfigDlgProc );
    }

    if( bConnectSuccess )
    {
        // App is now connected via DirectPlay, so start the game.  

        // For this sample, we just start a simple dialog box game.
        g_hrDialog = S_OK;
        DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN_GAME), NULL, (DLGPROC) VoiceDlgProc );

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
                DXTRACE_ERR( TEXT("DialogBox"), hr );
                MessageBox( NULL, TEXT("An error occured during the game. ")
                            TEXT("The sample will now quit."),
                            TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
            }
        }
    }

    // Cleanup player ID array 
    free( g_pPlayers );

    // Disconnect from the DirectPlayVoice session, 
    // and destory it if we are the host player.
    SAFE_DELETE( g_pNetVoice ); 

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
// Name: VoiceDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK VoiceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
            {
                g_hDlg = hDlg;

                if( FAILED( g_hrDialog = g_pNetVoice->Init( hDlg, g_bHostPlayer, TRUE,
                                                            g_pDP, DVSESSIONTYPE_PEER, 
                                                            &g_guidDVSessionCT, &g_dvClientConfig ) ) )
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
                    break;
                }

                if( g_pNetVoice->IsHalfDuplex() ) 
                {
                    MessageBox( hDlg, TEXT("You are running in half duplex mode. ")
                                TEXT("In half duplex mode no recording takes place."), 
                                TEXT("DirectPlay Sample"), MB_OK );
                }

                g_bVoiceSessionInProgress = TRUE;

                if( FAILED( g_hrDialog = OnInitDialog( hDlg ) ) )
                {
                    DXTRACE_ERR( TEXT("OnInitDialog"), g_hrDialog );
                    EndDialog( hDlg, 0 );
                    break;
                }
            }
            break;

        case WM_COMMAND:
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
                    continue;
                }

                TCHAR strStatus[255];

                if( pPlayerInfo->bHalfDuplex )
                {
                    _tcscpy( strStatus, TEXT("Can't talk") );
                }
                else
                {
                    if( pPlayerInfo->bTalking )
                        _tcscpy( strStatus, TEXT("Talking") );
                    else
                        _tcscpy( strStatus, TEXT("Silent") );
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
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Inits the dialog 
//-----------------------------------------------------------------------------
HRESULT OnInitDialog( HWND hDlg )
{
    LVCOLUMN column;
    RECT     rctListView;
    TCHAR    strHeader[255];
    DWORD    dwVertScrollBar;
    DWORD    dwListViewWidth;

    // Load and set the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    if( g_bHostPlayer )
        SetWindowText( hDlg, TEXT("VoiceConnect (Session Host)") );
    else
        SetWindowText( hDlg, TEXT("VoiceConnect (Session Client)") );

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

    // Update the listbox 
    PostMessage( g_hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );

    // Make a timer to update the listbox 
    // 'Status' column every so often 
    SetTimer( hDlg, 0, 250, NULL );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DisplayPlayersInChat()
// Desc: Displays the active players in the listview
//-----------------------------------------------------------------------------
HRESULT DisplayPlayersInChat( HWND hDlg )
{
    if( hDlg == NULL )
        return S_OK;

    HRESULT hr;
    LVITEM  lvItem;
    HWND    hListView = GetDlgItem( hDlg, IDC_PEOPLE_LIST );
    TCHAR   strStatus[32];
    TCHAR   strNumberPlayers[32];
    DWORD   dwNumPlayers;
    APP_PLAYER_INFO* pPlayerInfo = NULL;

    // Remove all the players and re-add them in the player enum callback
    ListView_DeleteAllItems( hListView );

    do
    {
        // Enum all players in the player ID array
        dwNumPlayers = g_dwPlayersArraySize;
        hr = g_pDP->EnumPlayersAndGroups( g_pPlayers, &dwNumPlayers, DPNENUM_PLAYERS );
        if( SUCCEEDED(hr) )
            break;

        if( hr == DPNERR_BUFFERTOOSMALL )
        {
            // Resize player pointer array     
            g_dwPlayersArraySize += 10;
            g_pPlayers   = (DPNID*) realloc( g_pPlayers, sizeof(DPNID)*g_dwPlayersArraySize );
        }                
    }         
    while( hr == DPNERR_BUFFERTOOSMALL );

    if( FAILED(hr) )                    
        return DXTRACE_ERR( TEXT("EnumPlayersAndGroups"), hr );

    for( DWORD i = 0; i<dwNumPlayers; i++ )
    {
        PLAYER_LOCK(); // enter player context CS

        do
        {
            // Get the player context accosicated with this DPNID
            // Call GetPlayerContext() until it returns something other than DPNERR_NOTREADY
            // DPNERR_NOTREADY will be returned if the callback thread has not 
            // yet returned from DPN_MSGID_CREATE_PLAYER, which sets the player's context
            hr = g_pDP->GetPlayerContext( g_pPlayers[i], (LPVOID*) &pPlayerInfo, 0);
        } 
        while( hr == DPNERR_NOTREADY ); 

        PLAYER_ADDREF( pPlayerInfo ); // addref player, since we are using it now
        PLAYER_UNLOCK(); // leave player context CS

        if( FAILED(hr) || pPlayerInfo == NULL )
        {
            // The player who sent this may have gone away before this 
            // message was handled, so just ignore it
            continue;
        }

        ZeroMemory( &lvItem, sizeof(lvItem) );

        // Add the item, saving the player's name and dpnid in the listview
        lvItem.mask       = LVIF_TEXT | LVIF_PARAM;
        lvItem.iItem      = 0;
        lvItem.iSubItem   = 0;
        lvItem.pszText    = pPlayerInfo->strPlayerName;
        lvItem.lParam     = g_pPlayers[i];
        lvItem.cchTextMax = _tcslen( pPlayerInfo->strPlayerName );
        int nIndex = ListView_InsertItem( hListView, &lvItem );

        if( pPlayerInfo->bHalfDuplex )
        {
            _tcscpy( strStatus, TEXT("Can't talk") );
        }
        else
        {
            if( pPlayerInfo->bTalking )
                _tcscpy( strStatus, TEXT("Talking") );
            else
                _tcscpy( strStatus, TEXT("Silent") );
        }

        PLAYER_LOCK();
        PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
        PLAYER_UNLOCK();

        // Start the player's status off as silent.  
        lvItem.mask       = LVIF_TEXT;
        lvItem.iItem      = nIndex;
        lvItem.iSubItem   = 1;
        lvItem.pszText    = strStatus;
        lvItem.cchTextMax = _tcslen( strStatus );
        ListView_SetItem( hListView, &lvItem );
    }

    wsprintf( strNumberPlayers, TEXT("%d"), dwNumPlayers );
    SetDlgItemText( hDlg, IDC_NUM_PLAYERS, strNumberPlayers );

    return S_OK;
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
    // queue data as it comes in, and then handle it on other threads.
    
    // This function is called by the DirectPlay message handler pool of 
    // threads, so be care of thread synchronization problems with shared memory

    switch( dwMessageId )
    {
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
                                                       
                    if( (pdpPlayerInfo->dwPlayerFlags & DPNPLAYER_LOCAL) != 0 )
                        g_LocalPlayerDPNID = pCreatePlayerMsg->dpnidPlayer;
                }
                
                SAFE_DELETE_ARRAY( pdpPlayerInfo );
            }

            // Tell DirectPlay to store this pPlayerInfo 
            // pointer in the pvPlayerContext.
            pCreatePlayerMsg->pvPlayerContext = pPlayerInfo;

            // Post a message to the dialog thread to update the 
            // UI.  This keeps the DirectPlay message handler 
            // from blocking
            if( g_hDlg != NULL )
                PostMessage( g_hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );
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

            // Post a message to the dialog thread to update the 
            // UI.  This keeps the DirectPlay message handler 
            // from blocking
            if( g_hDlg != NULL )
                PostMessage( g_hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );
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
    }

    // Make sure the DirectPlay MessageHandler calls the CNetConnectWizard handler, 
    // so it can be informed of messages such as DPN_MSGID_ENUM_HOSTS_RESPONSE.
    if( g_pNetConnectWizard )
        return g_pNetConnectWizard->MessageHandler( pvUserContext, dwMessageId, pMsgBuffer );
    
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
// Name: DirectPlayVoiceServerMessageHandler()
// Desc: The callback for DirectPlayVoice server messages.  
//-----------------------------------------------------------------------------
HRESULT CALLBACK DirectPlayVoiceServerMessageHandler( LPVOID lpvUserContext, DWORD dwMessageType,
                                                      LPVOID lpMessage )
{
    // This simple sample doesn't respond to any server messages
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

    switch( dwMessageType )
    {
        case DVMSGID_CREATEVOICEPLAYER:
            {
                DVMSG_CREATEVOICEPLAYER* pCreateVoicePlayerMsg = (DVMSG_CREATEVOICEPLAYER*) lpMessage;
                APP_PLAYER_INFO* pPlayerInfo = NULL;

                PLAYER_LOCK(); // enter player context CS

                // Get the player context accosicated with this DPNID
                hr = g_pDP->GetPlayerContext( pCreateVoicePlayerMsg->dvidPlayer, 
                                              (LPVOID* const) &pPlayerInfo,
											  0);

                if( FAILED(hr) || pPlayerInfo == NULL )
                {
                    // The player who sent this may have gone away before this 
                    // message was handled, so just ignore it
                    PLAYER_UNLOCK();
                    break;
                }

                PLAYER_ADDREF( pPlayerInfo ); // addref player, to be used by the voice layer
                PLAYER_UNLOCK(); // leave player context CS

                pPlayerInfo->bHalfDuplex = ((pCreateVoicePlayerMsg->dwFlags & DVPLAYERCAPS_HALFDUPLEX) != 0);

                // Set voice context value
				pCreateVoicePlayerMsg->pvPlayerContext = pPlayerInfo;

                // Post a message to the dialog thread to update the 
                // UI.  This keeps the DirectPlay message handler 
                // from blocking
                if( g_hDlg != NULL )
                    PostMessage( g_hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );

                // We're leaving the extra reference, voice will own that reference
            }
            break;

        case DVMSGID_DELETEVOICEPLAYER:
        {
            DVMSG_DELETEVOICEPLAYER* pMsg = (DVMSG_DELETEVOICEPLAYER*) lpMessage;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pMsg->pvPlayerContext;

            // Don't update the dlg if this message is for the local 
            // client since the dlg will be gone.
            if( pMsg->dvidPlayer != g_LocalPlayerDPNID )
            {
                // Post a message to the dialog thread to update the 
                // UI.  This keeps the DirectPlay message handler 
                // from blocking
                if( hDlg != NULL )
                    PostMessage( hDlg, WM_APP_DISPLAY_PLAYERS, 0, 0 );
			}

			// Release our extra reference on the player info that we have for the voice
			// context value.  
			//
            PLAYER_LOCK();
            PLAYER_RELEASE( pPlayerInfo );  
            PLAYER_UNLOCK();
            break;
        }            


        case DVMSGID_SESSIONLOST:
            g_hrDialog = DPNERR_CONNECTIONLOST;
            EndDialog( hDlg, 0 );
            break;

        case DVMSGID_HOSTMIGRATED:
        {
            DVMSG_HOSTMIGRATED* pMsg = (DVMSG_HOSTMIGRATED*) lpMessage;

            if( pMsg->pdvServerInterface != NULL )
            {           
                // If we keep the pMsg->pdvServerInterface pointer around, then
                // we must AddRef() it.  The CNetVoice::HostMigrate() automatically
                // does this for us.
                g_pNetVoice->HostMigrate( pMsg->pdvServerInterface );

                g_bHostPlayer = TRUE;
                SetWindowText( hDlg, TEXT("VoiceConnect (Session Host)") );
            }
            break;
        }

        case DVMSGID_GAINFOCUS:
        case DVMSGID_LOSTFOCUS:
        {
            TCHAR strWindowName[MAX_PATH];
            wsprintf( strWindowName, TEXT("%s%s%s"), g_strAppName,
                (g_bHostPlayer)                      ? TEXT(" (Session Host)") : TEXT(""),
                (dwMessageType == DVMSGID_LOSTFOCUS) ? TEXT(" (Focus Lost)") : TEXT("") );

            SetWindowText( hDlg, strWindowName );
            break;
        }

        case DVMSGID_RECORDSTART:             
		{ 
			DVMSG_RECORDSTART* pMsg = (DVMSG_RECORDSTART*) lpMessage;
			SetPlayerTalking( (APP_PLAYER_INFO*) pMsg->pvLocalPlayerContext, TRUE );
            break;
        }
        case DVMSGID_RECORDSTOP:             
        {
			DVMSG_RECORDSTOP* pMsg = (DVMSG_RECORDSTOP*) lpMessage;
			SetPlayerTalking( (APP_PLAYER_INFO*) pMsg->pvLocalPlayerContext, FALSE );
            break;
        }
        case DVMSGID_PLAYERVOICESTART:
        {
            DVMSG_PLAYERVOICESTART* pMsg = (DVMSG_PLAYERVOICESTART*) lpMessage;
            SetPlayerTalking( (APP_PLAYER_INFO*) pMsg->pvPlayerContext, TRUE );
            break;
        }

        case DVMSGID_PLAYERVOICESTOP:
        {
            DVMSG_PLAYERVOICESTOP* pMsg = (DVMSG_PLAYERVOICESTOP*) lpMessage;
            SetPlayerTalking( (APP_PLAYER_INFO*) pMsg->pvPlayerContext, FALSE );        	
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetPlayerTalking()
// Desc: Set player talking flag
//-----------------------------------------------------------------------------
void SetPlayerTalking( APP_PLAYER_INFO* pPlayerInfo, BOOL bTalking )
{
    if( pPlayerInfo )
        pPlayerInfo->bTalking = bTalking; 	
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

            if( !g_bHostPlayer || g_bVoiceSessionInProgress )
            {
                // We are are not the host player then disable all the server only options 
                EnableWindow( GetDlgItem( hDlg, IDC_COMPRESSION_COMBO ), FALSE );
                EnableWindow( GetDlgItem( hDlg, IDC_SESSIONCOMPRESION_GROUP ), FALSE );
            }
            else
            {
                VoiceConfigEnumCompressionCodecs( hDlg );   
                EnableWindow( GetDlgItem( hDlg, IDCANCEL ), FALSE );
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

        case WM_DESTROY:
        {
            GUID* pGuid;
            int nCount = (int)SendDlgItemMessage( hDlg, IDC_COMPRESSION_COMBO, CB_GETCOUNT, 0, 0 );
            for( int i=0; i<nCount; i++ )
            {
                pGuid = (LPGUID) SendDlgItemMessage( hDlg, IDC_COMPRESSION_COMBO, CB_GETITEMDATA, i, 0 );
                SAFE_DELETE( pGuid );
            }
            break;
        }
    }

    return FALSE; // Didn't handle message
}




//-----------------------------------------------------------------------------
// Name: VoiceConfigEnumCompressionCodecs()
// Desc: Asks DirectPlayVoice what voice compression codecs are availible
//       and fills the combo box thier names and GUIDs.
//-----------------------------------------------------------------------------
HRESULT VoiceConfigEnumCompressionCodecs( HWND hDlg )
{
    LPDIRECTPLAYVOICECLIENT pVoiceClient        = NULL;
    LPDVCOMPRESSIONINFO     pdvCompressionInfo  = NULL;
    LPGUID  pGuid         = NULL;
    LPBYTE  pBuffer       = NULL;
    DWORD   dwSize        = 0;
    DWORD   dwNumElements = 0;
    HWND    hPulldown     = GetDlgItem( hDlg, IDC_COMPRESSION_COMBO );
    HRESULT hr;
    LONG    lIndex;
    LONG    lFirst;

    CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlayVoiceClient, NULL, 
                                       CLSCTX_INPROC_SERVER, IID_IDirectPlayVoiceClient, 
                                       (VOID**) &pVoiceClient ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    hr = pVoiceClient->GetCompressionTypes( pBuffer, &dwSize, &dwNumElements, 0 );
    if( hr != DVERR_BUFFERTOOSMALL && FAILED(hr) )
        return DXTRACE_ERR( TEXT("GetCompressionTypes"), hr );

    pBuffer = new BYTE[dwSize];
    if( FAILED( hr = pVoiceClient->GetCompressionTypes( pBuffer, &dwSize, 
                                                        &dwNumElements, 0 ) ) )
        return DXTRACE_ERR( TEXT("GetCompressionTypes"), hr );

    SAFE_RELEASE( pVoiceClient );
    CoUninitialize();

    pdvCompressionInfo = (LPDVCOMPRESSIONINFO) pBuffer;
    for( DWORD dwIndex = 0; dwIndex < dwNumElements; dwIndex++ )
    {
        TCHAR strName[MAX_PATH];
        DXUtil_ConvertWideStringToGeneric( strName, 
                                           pdvCompressionInfo[dwIndex].lpszName );

        lIndex = (LONG)SendMessage( hPulldown, CB_ADDSTRING, 0, (LPARAM) strName );

        pGuid = new GUID;
        (*pGuid) = pdvCompressionInfo[dwIndex].guidType;
        SendMessage( hPulldown, CB_SETITEMDATA, lIndex, (LPARAM) pGuid );

        if( pdvCompressionInfo[dwIndex].guidType == DPVCTGUID_SC03 )
            lFirst = lIndex;
    }

    SAFE_DELETE_ARRAY( pBuffer );
    SendMessage( hPulldown, CB_SETCURSEL, lFirst, 0 );

    return S_OK;
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

    if( g_bHostPlayer )
    {
        // Figure out the compression codec
        LONG lCurSelection;
        LPGUID pGuidCT;

        lCurSelection = (LONG)SendDlgItemMessage( hDlg, IDC_COMPRESSION_COMBO, CB_GETCURSEL, 0, 0 );
        if( lCurSelection != CB_ERR )
        {
            pGuidCT = (LPGUID) SendDlgItemMessage( hDlg, IDC_COMPRESSION_COMBO, 
                                                   CB_GETITEMDATA, lCurSelection, 0 );
            if( pGuidCT != NULL )
                g_guidDVSessionCT = (*pGuidCT);
        }
    }

    EndDialog( hDlg, IDOK );
}




