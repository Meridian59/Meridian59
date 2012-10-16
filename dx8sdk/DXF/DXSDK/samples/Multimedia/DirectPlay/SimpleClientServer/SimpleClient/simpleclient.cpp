//----------------------------------------------------------------------------
// File: SimpleClient.cpp
//
// Desc: The SimpleClientServer sample is a simple client/server application. 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <dplobby8.h>
#include <dxerr8.h>
#include <tchar.h>
#include "DXUtil.h"
#include "SimpleClientServer.h"
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
BOOL                    g_bWasLobbyLaunched           = FALSE;   // TRUE if lobby launched
HINSTANCE               g_hInst                       = NULL;    // HINST of app
HWND                    g_hDlg                        = NULL;    // HWND of main dialog
LONG                    g_lNumberOfActivePlayers      = 0;       // Number of players currently in game
DPNID                   g_dpnidLocalPlayer            = 0;
APP_PLAYER_INFO         g_playerHead;
TCHAR                   g_strAppName[256]             = TEXT("SimpleClient");
HRESULT                 g_hrDialog;                              // Exit code for app 
TCHAR                   g_strLocalPlayerName[MAX_PATH];          // Local player name




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI   DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI   DirectPlayLobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK GreetingDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT  InitDirectPlay();
HRESULT  WaveToAllPlayers();
VOID     AppendTextToEditControl( HWND hDlg, TCHAR* strNewLogLine );
HRESULT  GetPlayerStruct( DPNID dpnidPlayer, APP_PLAYER_INFO** ppPlayerInfo );
VOID     DestoryPlayerStruct( APP_PLAYER_INFO* pPlayerInfo );
VOID     AddPlayerStruct( APP_PLAYER_INFO* pPlayerInfo );




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

    if( FAILED( hr = InitDirectPlay() ) )
    {
        DXTRACE_ERR( TEXT("InitDirectPlay"), hr );
        MessageBox( NULL, TEXT("Failed initializing IDirectPlay8Peer. ")
                    TEXT("The sample will now quit."),
                    g_strAppName, MB_OK | MB_ICONERROR );
        return FALSE;
    }

    // If we were launched from a lobby client, then we may have connection settings
    // that we can use to connect to the host.  If not, then we'll need to prompt 
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

    // Cleanup DirectPlay and helper classes
    if( g_pDPClient )
    {
        g_pDPClient->Close(0);
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

            SetWindowText( hDlg, g_strAppName );

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
            
            // Get the player struct accosicated with this DPNID
            PLAYER_LOCK(); // enter player struct CS
            hr = GetPlayerStruct( dpnidPlayer, &pPlayerInfo );
            PLAYER_UNLOCK(); // leave player struct CS

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
                        PostMessage( g_hDlg, WM_APP_UPDATE_STATS, 0, 0 );
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
                        PostMessage( g_hDlg, WM_APP_UPDATE_STATS, 0, 0 );
                    break;
                };

                case GAME_MSGID_WAVE:
                {
                    // The host is telling us that someone waved to this client
                    APP_PLAYER_INFO* pPlayerInfo = NULL;
                    GAMEMSG_WAVE* pWaveMsg;
                    pWaveMsg = (GAMEMSG_WAVE*)pReceiveMsg->pReceiveData;

                    // Ignore wave messages set by the local player
                    if( pWaveMsg->dpnidPlayer == g_dpnidLocalPlayer )
                        break;

                    // Get the player struct accosicated with this DPNID
                    PLAYER_LOCK(); // enter player struct CS
                    hr = GetPlayerStruct( pWaveMsg->dpnidPlayer, &pPlayerInfo );
                    PLAYER_ADDREF( pPlayerInfo ); // addref player, since we are using it now
                    PLAYER_UNLOCK(); // leave player struct CS

                    if( FAILED(hr) || pPlayerInfo == NULL )
                    {
                        // The player who sent this may have gone away before this 
                        // message was handled, so just ignore it
                        break;
                    }

                    // This message is sent when a player has waved to us, so 
                    // post a message to the dialog thread to update the UI.  
                    // This keeps the DirectPlay threads from blocking, and also
                    // serializes the recieves since DirectPlayMessageHandler can
                    // be called simultaneously from a pool of DirectPlay threads.
                    PostMessage( g_hDlg, WM_APP_DISPLAY_WAVE, pPlayerInfo->dpnidPlayer, 0 );
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
                    TEXT("SimpleClient"), MB_OK );
    }
    else
    {
        // Send a message to all of the players
        GAMEMSG_GENERIC msgWave;
        msgWave.dwType = GAME_MSGID_WAVE;

        DPN_BUFFER_DESC bufferDesc;
        bufferDesc.dwBufferSize = sizeof(GAMEMSG_GENERIC);
        bufferDesc.pBufferData  = (BYTE*) &msgWave;

        DPNHANDLE hAsync;
        // DirectPlay will tell via the message handler 
        // if there are any severe errors, so ignore any errors 
        g_pDPClient->Send( &bufferDesc, 1, 0, NULL, &hAsync, DPNSEND_GUARANTEED );
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


