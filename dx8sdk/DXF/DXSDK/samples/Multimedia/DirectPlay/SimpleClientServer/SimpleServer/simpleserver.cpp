//----------------------------------------------------------------------------
// File: SimpleServer.cpp
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
#include <dxerr8.h>
#include <tchar.h>
#include "SimpleClientServer.h"
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
#define SIMPLESERVER_DEFAULT_PORT           0x6501 // arbitrary port number for this app

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
IDirectPlay8Server* g_pDPServer                  = NULL;    // DirectPlay server object
HINSTANCE          g_hInst                       = NULL;    // HINST of app
HWND               g_hDlg                        = NULL;    // HWND of main dialog
LONG               g_lNumberOfActivePlayers      = 0;       // Number of players currently in game
TCHAR              g_strAppName[256]             = TEXT("SimpleServer");
TCHAR              g_strSessionName[MAX_PATH];              // Session name
DWORD              g_dwPort;                                // Port
HRESULT            g_hrDialog;                              // Exit code for app 
BOOL               g_bServerStarted              = FALSE;   // TRUE if the server has started




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI   DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
INT_PTR CALLBACK ServerDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT  StartServer( HWND hDlg );
VOID     StopServer( HWND hDlg );
VOID     DisplayPlayers( HWND hDlg );
HRESULT  SendCreatePlayerMsg( APP_PLAYER_INFO* pPlayerInfo, DPNID dpnidTarget );
HRESULT  SendWorldStateToNewPlayer( DPNID dpnidPlayer );
HRESULT  SendDestroyPlayerMsgToAll( APP_PLAYER_INFO* pPlayerInfo );
HRESULT  SendWaveMessageToAll( DPNID dpnidFrom );





//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, 
                      LPSTR pCmdLine, INT nCmdShow )
{
    HKEY    hDPlaySampleRegKey;
    BOOL    bConnectSuccess = FALSE;

    g_hInst = hInst; 
    InitializeCriticalSection( &g_csPlayerContext );

    // Read persistent state information from registry
    RegCreateKeyEx( HKEY_CURRENT_USER, DPLAY_SAMPLE_KEY, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    &hDPlaySampleRegKey, NULL );
    DXUtil_ReadStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), 
                             g_strSessionName, MAX_PATH, TEXT("TestGame") );
    DXUtil_ReadIntRegKey( hDPlaySampleRegKey, TEXT("SimpleServer Port"), 
                          &g_dwPort, SIMPLESERVER_DEFAULT_PORT );

    // Init COM so we can use CoCreateInstance
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    // For this sample, we just start a simple dialog box server
    g_hrDialog = S_OK;
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC) ServerDlgProc );

    if( FAILED( g_hrDialog ) )
    {
        if( g_hrDialog == DPNERR_CONNECTIONLOST )
        {
            MessageBox( NULL, TEXT("The DirectPlay session was lost. ")
                        TEXT("The server will now quit."),
                        g_strAppName, MB_OK | MB_ICONERROR );
        }
        else
        {
            DXTRACE_ERR( TEXT("DialogBox"), g_hrDialog );
            MessageBox( NULL, TEXT("An error occured. ")
                        TEXT("The server will now quit."),
                        g_strAppName, MB_OK | MB_ICONERROR );
        }
    }

    DXUtil_WriteStringRegKey( hDPlaySampleRegKey, TEXT("Session Name"), g_strSessionName );
    DXUtil_WriteIntRegKey( hDPlaySampleRegKey, TEXT("SimpleServer Port"), g_dwPort );

    StopServer( NULL );

    RegCloseKey( hDPlaySampleRegKey );
    DeleteCriticalSection( &g_csPlayerContext );
    CoUninitialize();

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: ServerDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK ServerDlgProc( HWND hDlg, UINT msg, 
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

            SetWindowText( hDlg, TEXT("SimpleServer") );
            SetDlgItemText( hDlg, IDC_SESSION_NAME, g_strSessionName );

            // Set the port to either a number or blank
            if( g_dwPort != 0 )
                SetDlgItemInt( hDlg, IDC_PORT, g_dwPort, FALSE );
            else
                SetDlgItemText( hDlg, IDC_PORT, TEXT("") );

            SetDlgItemText( hDlg, IDC_STATUS, TEXT("Server stoped.") );

            PostMessage( hDlg, WM_APP_UPDATE_STATS, 0, 0 );
            break;
        }

        case WM_APP_UPDATE_STATS:
        {
            // Update the number of players in the game
            TCHAR strNumberPlayers[32];

            wsprintf( strNumberPlayers, TEXT("%d"), g_lNumberOfActivePlayers );
            SetDlgItemText( hDlg, IDC_NUM_PLAYERS, strNumberPlayers );
            DisplayPlayers( hDlg );
            break;
        }

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case IDC_START:
                    if( !g_bServerStarted )
                    {
                        if( FAILED( g_hrDialog = StartServer( hDlg ) ) )
                        {
                            DXTRACE_ERR( TEXT("StartServer"), g_hrDialog );
                            EndDialog( hDlg, 0 );
                        }
                    }
                    else
                    {
                        StopServer( hDlg );
                    }

                    if( g_bServerStarted )
                    {
                        SetDlgItemText( hDlg, IDC_START, TEXT("Stop Server") );
                        EnableWindow( GetDlgItem( hDlg, IDC_SESSION_NAME ), FALSE );
                        EnableWindow( GetDlgItem( hDlg, IDC_PORT ), FALSE );
                    }
                    else
                    {
                        SetDlgItemText( hDlg, IDC_START, TEXT("Start Server") );
                        EnableWindow( GetDlgItem( hDlg, IDC_SESSION_NAME ), TRUE );
                        EnableWindow( GetDlgItem( hDlg, IDC_PORT ), TRUE );
                    }

                    break;

                case IDCANCEL:
                    StopServer( hDlg );
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
        case DPN_MSGID_CREATE_PLAYER:
        {
            HRESULT hr;
            PDPNMSG_CREATE_PLAYER pCreatePlayerMsg;
            pCreatePlayerMsg = (PDPNMSG_CREATE_PLAYER)pMsgBuffer;

            // Get the peer info and extract its name
            DWORD dwSize = 0;
            DPN_PLAYER_INFO* pdpPlayerInfo = NULL;
            hr = g_pDPServer->GetClientInfo( pCreatePlayerMsg->dpnidPlayer, 
                                             pdpPlayerInfo, &dwSize, 0 );
            if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL )
            {
                if( hr == DPNERR_INVALIDPLAYER )
                {
                    // Ignore this message if this is for the host
                    break;
                }

                return DXTRACE_ERR( TEXT("GetClientInfo"), hr );
            }
            pdpPlayerInfo = (DPN_PLAYER_INFO*) new BYTE[ dwSize ];
            ZeroMemory( pdpPlayerInfo, dwSize );
            pdpPlayerInfo->dwSize = sizeof(DPN_PLAYER_INFO);
            hr = g_pDPServer->GetClientInfo( pCreatePlayerMsg->dpnidPlayer, 
                                       pdpPlayerInfo, &dwSize, 0 );
            if( FAILED(hr) )
                return DXTRACE_ERR( TEXT("GetClientInfo"), hr );

            // Create a new and fill in a APP_PLAYER_INFO
            APP_PLAYER_INFO* pPlayerInfo = new APP_PLAYER_INFO;
            ZeroMemory( pPlayerInfo, sizeof(APP_PLAYER_INFO) );
            pPlayerInfo->lRefCount   = 1;
            pPlayerInfo->dpnidPlayer = pCreatePlayerMsg->dpnidPlayer;

            // This stores a extra TCHAR copy of the player name for 
            // easier access.  This will be redundent copy since DPlay 
            // also keeps a copy of the player name in GetClientInfo()
            DXUtil_ConvertWideStringToGeneric( pPlayerInfo->strPlayerName, 
                                               pdpPlayerInfo->pwszName, MAX_PLAYER_NAME );

            SAFE_DELETE_ARRAY( pdpPlayerInfo );

            // Tell DirectPlay to store this pPlayerInfo 
            // pointer in the pvPlayerContext.
            pCreatePlayerMsg->pvPlayerContext = pPlayerInfo;

            // Send all connected players a message telling about this new player
            SendCreatePlayerMsg( pPlayerInfo, DPNID_ALL_PLAYERS_GROUP );

            // Tell this new player about the world state
            SendWorldStateToNewPlayer( pCreatePlayerMsg->dpnidPlayer );

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

            // Ignore this message if this is the host player
            if( pPlayerInfo == NULL )
                break; 

            // Send all connected players a message telling about this destroyed player
            SendDestroyPlayerMsgToAll( pPlayerInfo );

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

            GAMEMSG_GENERIC* pMsg = (GAMEMSG_GENERIC*) pReceiveMsg->pReceiveData;
            if( pMsg->dwType == GAME_MSGID_WAVE )
                SendWaveMessageToAll( pPlayerInfo->dpnidPlayer );
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: StartServer
// Desc: 
//-----------------------------------------------------------------------------
HRESULT StartServer( HWND hDlg )
{
    HRESULT hr;
    PDIRECTPLAY8ADDRESS pDP8AddrLocal = NULL;

    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Starting server...") );
    SetCursor( LoadCursor(NULL, IDC_WAIT) );

    WCHAR wstrSessionName[MAX_PATH];
    GetDlgItemText( hDlg, IDC_SESSION_NAME, g_strSessionName, MAX_PATH );
    DXUtil_ConvertGenericStringToWide( wstrSessionName, g_strSessionName );

    BOOL bPortTranslated;
    g_dwPort = GetDlgItemInt( hDlg, IDC_PORT, &bPortTranslated, FALSE );
    if( !bPortTranslated )
        g_dwPort = 0;

    // Create IDirectPlay8Server
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Server, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Server, 
                                       (LPVOID*) &g_pDPServer ) ) )
        return DXTRACE_ERR( TEXT("CoCreateInstance"), hr );

    // Init IDirectPlay8Server
    if( FAILED( hr = g_pDPServer->Initialize( NULL, DirectPlayMessageHandler, 0 ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

    hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, 
                           CLSCTX_ALL, IID_IDirectPlay8Address, 
                           (LPVOID*) &pDP8AddrLocal );
    if( FAILED(hr) )
    {
        DXTRACE_ERR( TEXT("CoCreateInstance"), hr );
        goto LCleanup;
    }

    hr = pDP8AddrLocal->SetSP( &CLSID_DP8SP_TCPIP );
    if( FAILED(hr) )
    {
        DXTRACE_ERR( TEXT("SetSP"), hr );
        goto LCleanup;
    }

    // Add the port to pDP8AddrLocal, if the port is non-zero.
    // If the port is 0, then DirectPlay will pick a port, 
    // Games will typically hard code the port so the 
    // user need not know it
    if( g_dwPort != 0 )
    {
        if( FAILED( hr = pDP8AddrLocal->AddComponent( DPNA_KEY_PORT, 
                                                      &g_dwPort, sizeof(g_dwPort),
                                                      DPNA_DATATYPE_DWORD ) ) )
            return DXTRACE_ERR( TEXT("AddComponent"), hr );
    }

    DPN_APPLICATION_DESC dpnAppDesc;
    ZeroMemory( &dpnAppDesc, sizeof(DPN_APPLICATION_DESC) );
    dpnAppDesc.dwSize           = sizeof( DPN_APPLICATION_DESC );
    dpnAppDesc.dwFlags          = DPNSESSION_CLIENT_SERVER;
    dpnAppDesc.guidApplication  = g_guidApp;
    dpnAppDesc.pwszSessionName  = wstrSessionName;

    hr = g_pDPServer->Host( &dpnAppDesc, &pDP8AddrLocal, 1, NULL, NULL, NULL, 0  );
    if( FAILED(hr) )
    {
        DXTRACE_ERR( TEXT("Host"), hr );
        goto LCleanup;
    }

    SetCursor( LoadCursor(NULL, IDC_ARROW) );
    g_bServerStarted = TRUE;
    SetDlgItemText( hDlg, IDC_STATUS, TEXT("Server started.") );

LCleanup:
    SAFE_RELEASE( pDP8AddrLocal );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: StopServer
// Desc: 
//-----------------------------------------------------------------------------
VOID StopServer( HWND hDlg )
{
    if( hDlg )
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Stopping server...") );
    SetCursor( LoadCursor(NULL, IDC_WAIT) );

    if( g_pDPServer )
    {
        g_pDPServer->Close(0);
        SAFE_RELEASE( g_pDPServer );
    }
    g_bServerStarted = FALSE;

    SetCursor( LoadCursor(NULL, IDC_ARROW) );
    if( hDlg )
        SetDlgItemText( hDlg, IDC_STATUS, TEXT("Server stoped.") );
}



    
//-----------------------------------------------------------------------------
// Name: DisplayPlayers
// Desc: 
//-----------------------------------------------------------------------------
VOID DisplayPlayers( HWND hDlg )
{
    HRESULT hr;
    DWORD dwNumPlayers = 0;
    DPNID* aPlayers = NULL;

    SendMessage( GetDlgItem(hDlg, IDC_PLAYER_LIST), LB_RESETCONTENT, 0, 0 );

    if( NULL == g_pDPServer )
        return;

    // Enumerate all the connected players
    while( TRUE )
    {
        hr = g_pDPServer->EnumPlayersAndGroups( aPlayers, &dwNumPlayers, DPNENUM_PLAYERS );
        if( SUCCEEDED(hr) )
            break;

        if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL )
            return;

        SAFE_DELETE_ARRAY( aPlayers );
        aPlayers = new DPNID[ dwNumPlayers ];
    }

    // For each player, send a "create player" message to the new player
    for( DWORD i = 0; i<dwNumPlayers; i++ )
    {
        APP_PLAYER_INFO* pPlayerInfo = NULL;

        do
        {
            // Get the player context accosicated with this DPNID
            // Call GetPlayerContext() until it returns something other than DPNERR_NOTREADY
            // DPNERR_NOTREADY will be returned if the callback thread has not 
            // yet returned from DPN_MSGID_CREATE_PLAYER, which sets the player's context
            hr = g_pDPServer->GetPlayerContext( aPlayers[i], (LPVOID*) &pPlayerInfo, 0 );
        } 
        while( hr == DPNERR_NOTREADY ); 
                
        // Ignore this player if we can't get the context
        if( pPlayerInfo == NULL || FAILED(hr) )
            continue; 
        
        TCHAR strTemp[MAX_PATH];
        wsprintf( strTemp, TEXT("DPNID: 0x%0.8x (%s)"), pPlayerInfo->dpnidPlayer, pPlayerInfo->strPlayerName );
        int nIndex = (int)SendMessage( GetDlgItem(hDlg, IDC_PLAYER_LIST), LB_ADDSTRING, 
                                       0, (LPARAM)strTemp );
    }

    SAFE_DELETE_ARRAY( aPlayers );
}



    
//-----------------------------------------------------------------------------
// Name: SendCreatePlayerMsg
// Desc: Send the target player a creation message about the player identified
//       in the APP_PLAYER_INFO struct.
//-----------------------------------------------------------------------------
HRESULT SendCreatePlayerMsg( APP_PLAYER_INFO* pPlayerAbout, DPNID dpnidTarget )
{
    GAMEMSG_CREATE_PLAYER msgCreatePlayer;
    msgCreatePlayer.dwType = GAME_MSGID_CREATE_PLAYER;
    msgCreatePlayer.dpnidPlayer = pPlayerAbout->dpnidPlayer;
    _tcscpy( msgCreatePlayer.strPlayerName, pPlayerAbout->strPlayerName );

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(GAMEMSG_CREATE_PLAYER);
    bufferDesc.pBufferData  = (BYTE*) &msgCreatePlayer;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    g_pDPServer->SendTo( dpnidTarget, &bufferDesc, 1,
                         0, NULL, &hAsync, DPNSEND_NOLOOPBACK | DPNSEND_GUARANTEED );

    return S_OK;
}



    
//-----------------------------------------------------------------------------
// Name: SendWorldStateToNewPlayer
// Desc: Send the world state to the new player.  For this sample, it is just
//       "create player" message for every connected player
//-----------------------------------------------------------------------------
HRESULT SendWorldStateToNewPlayer( DPNID dpnidNewPlayer )
{
    HRESULT hr;
    DWORD dwNumPlayers = 0;
    DPNID* aPlayers = NULL;

    // Tell this player the dpnid of itself
    GAMEMSG_SET_ID msgSetID;
    msgSetID.dwType      = GAME_MSGID_SET_ID;
    msgSetID.dpnidPlayer = dpnidNewPlayer;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(GAMEMSG_SET_ID);
    bufferDesc.pBufferData  = (BYTE*) &msgSetID;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    g_pDPServer->SendTo( dpnidNewPlayer, &bufferDesc, 1,
                         0, NULL, &hAsync, DPNSEND_NOLOOPBACK | DPNSEND_GUARANTEED );

    // Enumerate all the connected players
    while( TRUE )
    {
        hr = g_pDPServer->EnumPlayersAndGroups( aPlayers, &dwNumPlayers, DPNENUM_PLAYERS );
        if( SUCCEEDED(hr) )
            break;

        if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL )
            return DXTRACE_ERR( TEXT("EnumPlayersAndGroups"), hr );

        SAFE_DELETE_ARRAY( aPlayers );
        aPlayers = new DPNID[ dwNumPlayers ];
    }

    // For each player, send a "create player" message to the new player
    for( DWORD i = 0; i<dwNumPlayers; i++ )
    {
        APP_PLAYER_INFO* pPlayerInfo = NULL;

        // Don't send a create msg to the new player about itself.  This will 
        // be already done when we sent one to DPNID_ALL_PLAYERS_GROUP
        if( aPlayers[i] == dpnidNewPlayer )
            continue;  

        // Get the player context accosicated with this DPNID
        hr = g_pDPServer->GetPlayerContext( aPlayers[i], (LPVOID*) &pPlayerInfo, 0 );

        // Ignore this player if we can't get the context
        if( pPlayerInfo == NULL || FAILED(hr) )
            continue; 

        SendCreatePlayerMsg( pPlayerInfo, dpnidNewPlayer );
    }

    SAFE_DELETE_ARRAY( aPlayers );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SendDestroyPlayerMsgToAll
// Desc: 
//-----------------------------------------------------------------------------
HRESULT SendDestroyPlayerMsgToAll( APP_PLAYER_INFO* pPlayerInfo )
{
    GAMEMSG_DESTROY_PLAYER msgDestroyPlayer;
    msgDestroyPlayer.dwType = GAME_MSGID_DESTROY_PLAYER;
    msgDestroyPlayer.dpnidPlayer = pPlayerInfo->dpnidPlayer;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(GAMEMSG_CREATE_PLAYER);
    bufferDesc.pBufferData  = (BYTE*) &msgDestroyPlayer;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    g_pDPServer->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                         0, NULL, &hAsync, DPNSEND_NOLOOPBACK | DPNSEND_GUARANTEED );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SendWaveMessageToAll
// Desc: 
//-----------------------------------------------------------------------------
HRESULT SendWaveMessageToAll( DPNID dpnidFrom )
{
    GAMEMSG_WAVE msgWave;
    msgWave.dwType = GAME_MSGID_WAVE;
    msgWave.dpnidPlayer = dpnidFrom;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(GAMEMSG_WAVE);
    bufferDesc.pBufferData  = (BYTE*) &msgWave;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    g_pDPServer->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                         0, NULL, &hAsync, DPNSEND_NOLOOPBACK | DPNSEND_GUARANTEED );

    return S_OK;
}



