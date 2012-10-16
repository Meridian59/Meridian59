//----------------------------------------------------------------------------
// File: StagedPeer.cpp
//
// Desc: The main game file for the StagedConnect sample.  It connects 
//       players together with two dialog boxes to prompt users on the 
//       connection settings to join or create a session. After the user 
//       connects to a sesssion, the sample displays a multiplayer stage. 
// 
//       The stage allows all players connected to the same session to chat, 
//       and start a new game at the same time when everyone is ready and the 
//       host player decides to begin.  The host player may also reject 
//       players or close player slots.  This allows host player to control 
//       who is allowed to join the game.  
//
//       After a new game has started the sample begins a very simplistic 
//       game called "The Greeting Game"), where players have the option of 
//       send a single simple DirectPlay message to all of the other players.
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <dxerr8.h>
#include <dplay8.h>
#include <dplobby8.h>
#include "StagedPeer.h"
#include "NetConnect.h"
#include "NetStage.h"
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
#define WM_APP_UPDATE_STATS     (WM_APP + 0)
#define WM_APP_DISPLAY_WAVE     (WM_APP + 1)

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {2D6D17EC-2E22-499a-B4F0-7D408864E5C8}
GUID g_guidApp = { 0x2d6d17ec, 0x2e22, 0x499a, { 0xb4, 0xf0, 0x7d, 0x40, 0x88, 0x64, 0xe5, 0xc8 } };




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Peer*  g_pDP                         = NULL;    // DirectPlay peer object
CNetConnectWizard* g_pNetConnectWizard           = NULL;    // Connection wizard
CNetStage*         g_pNetStage                   = NULL;    // Stage wizard
IDirectPlay8LobbiedApplication* g_pLobbiedApp    = NULL;    // DirectPlay lobbied app 
BOOL               g_bWasLobbyLaunched           = FALSE;   // TRUE if lobby launched
HINSTANCE          g_hInst                       = NULL;    // HINST of app
HWND               g_hDlg                        = NULL;    // HWND of main dialog
LONG               g_lNumberOfActivePlayers      = 0;       // Number of players in session
TCHAR              g_strAppName[256]             = TEXT("StagedPeer");
HRESULT            g_hrDialog;                              // Exit code for app 
TCHAR              g_strLocalPlayerName[MAX_PATH];          // Local player name
TCHAR              g_strSessionName[MAX_PATH];              // Session name
TCHAR              g_strPreferredProvider[MAX_PATH];        // Provider string




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
HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
HRESULT WINAPI DirectPlayLobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK GreetingDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT InitDirectPlay();
HRESULT WaveToAllPlayers();
VOID    AppendTextToEditControl( HWND hDlg, TCHAR* strNewLogLine );




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
    g_pNetStage         = new CNetStage( hInst, g_strAppName );

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

            bConnectSuccess = TRUE; 
        }
    }
    else
    {
        // If not lobby launched, prompt the user about the network 
        // connection and which session they would like to join or 
        // if they want to create a new one.

        // Setup connection wizard
        g_pNetConnectWizard->SetMaxPlayers( 10 );
        g_pNetConnectWizard->SetPlayerName( g_strLocalPlayerName );
        g_pNetConnectWizard->SetSessionName( g_strSessionName );
        g_pNetConnectWizard->SetPreferredProvider( g_strPreferredProvider );

        int  nStep      = 0;
        BOOL bBacktrack = FALSE;
        BOOL bHostPlayer;

        // Show the dialog boxes to start a DirectPlay connect, and loop
        // until the user completes them all successfully or quits.
        while( TRUE )
        {
            switch( nStep )
            {
                case 0:
                    // The first step is to prompt the users about the network 
                    // connection and which session they would like to join or 
                    // if they want to create a new one.
                    g_pNetStage->Init( g_pDP );
                    hr = g_pNetConnectWizard->DoConnectWizard( bBacktrack );
                    bHostPlayer = g_pNetConnectWizard->IsHostPlayer();
                    bBacktrack = TRUE; 
                    break;

                case 1:
                    if( !g_bWasLobbyLaunched )
                    {
                        // The second step is to start a multiplayer stage 
                        hr = g_pNetStage->DoStage( g_strLocalPlayerName, bHostPlayer );
                    }
                    else
                    {
                        // If lobby launched, then skip the stage
                        hr = NS_S_FORWARD;
                    }
                    break;
            }

            if( FAILED(hr) )
            {
                DXTRACE_ERR( TEXT("DoConnectWizard"), hr );
                MessageBox( NULL, TEXT("Multiplayer connect failed. ")
                            TEXT("The sample will now quit."),
                            TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
                bConnectSuccess = FALSE;
                break;
            }
            else if( hr == NCW_S_QUIT || hr == NS_S_QUIT )
            {
                // The user canceled the multiplayer connect.
                // The sample will now quit.
                bConnectSuccess = FALSE;
                break;
            }
            else if( hr == NCW_S_BACKUP || hr == NS_S_BACKUP )
            {
                // If backing up, then close down and re-create 
                // the DirectPlay interface since we'll need 
                // to connect to a new host.
                nStep--;

                hr = InitDirectPlay();
                if( FAILED( hr ) ) 
                {
                    DXTRACE_ERR( TEXT("InitDirectPlay"), hr );
                    MessageBox( NULL, TEXT("Failed initializing IDirectPlay8Peer. ")
                                TEXT("The sample will now quit."),
                                TEXT("DirectPlay Sample"), MB_OK | MB_ICONERROR );
                    return FALSE;
                }
            }
            else
            {
                nStep++;
            }

            // If we go beyond the last dialog box, then we are done
            if( nStep == 2 )
            {
                bConnectSuccess = TRUE; 

                // Read information from g_pNetConnectWizard
                _tcscpy( g_strLocalPlayerName, g_pNetConnectWizard->GetPlayerName() );
                _tcscpy( g_strSessionName, g_pNetConnectWizard->GetSessionName() );
                _tcscpy( g_strPreferredProvider, g_pNetConnectWizard->GetPreferredProvider() );

                // Write information to the registry
                DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Player Name"), g_strLocalPlayerName );
                DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), g_strSessionName );
                DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Preferred Provider"), g_strPreferredProvider );
                break;
            }
        }
    }

    if( bConnectSuccess )
    {
        // App is now connected via DirectPlay, so start the game.  

        // For this sample, we just start a simple dialog box game.
        g_hrDialog = S_OK;
        DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN_GAME), NULL, (DLGPROC) GreetingDlgProc );

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

    SAFE_DELETE( g_pNetStage );
    RegCloseKey( hDPlaySampleRegKey );
    DeleteCriticalSection( &g_csPlayerContext );
    CoUninitialize();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GreetingDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
HRESULT InitDirectPlay()
{
    DPNHANDLE hLobbyLaunchedConnection = NULL;
    HRESULT hr;

    // Cleanup previous DirectPlay instance
    if( g_pLobbiedApp )
    {
        g_pLobbiedApp->Close( 0 );
        SAFE_RELEASE( g_pLobbiedApp );
    }

    if( g_pDP )
    {
        g_pDP->Close(0);
        SAFE_RELEASE( g_pDP );
    }

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
// Name: GreetingDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK GreetingDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
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

                // Display local player's name
                SetDlgItemText( hDlg, IDC_PLAYER_NAME, g_strLocalPlayerName );

                if( g_pNetConnectWizard->IsHostPlayer() )
                    SetWindowText( hDlg, TEXT("StagedPeer (Host)") );
                else
                    SetWindowText( hDlg, TEXT("StagedPeer") );

                PostMessage( hDlg, WM_APP_UPDATE_STATS, 0, 0 );
            }
            break;

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
            DPNID            dpnidPlayer = (DPNID) wParam;
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
                    EndDialog( hDlg, 0 );
                    return TRUE;
            }
            break;
    }

    return FALSE; // Didn't handle message
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
    HRESULT hr;

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
            pPlayerInfo->dpnidPlayer = pCreatePlayerMsg->dpnidPlayer;
            pPlayerInfo->lRefCount   = 1;

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
                                                       
                    pPlayerInfo->dwFlags     = pdpPlayerInfo->dwPlayerFlags;
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
            
            // Release the pPlayerInfo struct twice, since CNetStage will 
            // still have a ref count, and its DPN_MSGID_DESTROY_PLAYER handler 
            // won't be called if the NetStage dlg has already closed.            
            PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
            if( g_pNetStage == NULL )
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
    }

    // Make sure the DirectPlay MessageHandler calls the CNetConnectWizard handler, 
    // so it can be informed of messages such as DPN_MSGID_ENUM_HOSTS_RESPONSE.
    if( g_pNetConnectWizard )
        g_pNetConnectWizard->MessageHandler( pvUserContext, dwMessageId, pMsgBuffer );
    
    // Make sure the DirectPlay MessageHandler calls the CNetStage handler, 
    // so it can be informed of messages such as DPN_MSGID_CREATE_PLAYER.
    if( g_pNetStage )
    {
        hr = g_pNetStage->MessageHandler( pvUserContext, dwMessageId, pMsgBuffer );
        if( hr == DPNSUCCESS_PENDING )
            return hr;
    }
    
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
// Name: WaveToAllPlayers()
// Desc: Send a app-defined "wave" DirectPlay message to all connected players
//-----------------------------------------------------------------------------
HRESULT WaveToAllPlayers()
{
    if( g_lNumberOfActivePlayers == 1 )
    {
        MessageBox( NULL, TEXT("No one is around to wave at! :("), 
                    TEXT("StagedPeer"), MB_OK );
    }
    else
    {
        // Send it to all of the players
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










