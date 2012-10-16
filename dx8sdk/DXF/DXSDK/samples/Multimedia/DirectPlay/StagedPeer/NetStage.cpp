//-----------------------------------------------------------------------------
// File: NetStage.cpp
//
// Desc: Support file for a DirectPlay stage.  The stage allows all 
//       players connected to the same session to chat, and start a new game
//       at the same time when everyone is ready and the host player decides 
//       to begin.  The host player may also reject players or close player 
//       slots.  This allows host player to control who is allowed to join 
//       the game.  
//
// Copyright (c) 1999 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define IDIRECTPLAY2_OR_GREATER
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <dxerr8.h>
#include <DPlay8.h>
#include <stdio.h>
#include <tchar.h>
#include <mmsystem.h>
#include <assert.h>
#include "NetStage.h"
#include "resource.h"
#include "DXUtil.h"



//-----------------------------------------------------------------------------
// Player context locking defines
//-----------------------------------------------------------------------------
extern CRITICAL_SECTION g_csPlayerContext;
#define PLAYER_LOCK()                   EnterCriticalSection( &g_csPlayerContext ); 
#define PLAYER_ADDREF( pPlayerInfo )    if( pPlayerInfo ) pPlayerInfo->lRefCount++;
#define PLAYER_RELEASE( pPlayerInfo )   if( pPlayerInfo ) { pPlayerInfo->lRefCount--; if( pPlayerInfo->lRefCount <= 0 ) SAFE_DELETE( pPlayerInfo ); }
#define PLAYER_UNLOCK()                 LeaveCriticalSection( &g_csPlayerContext );


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
CNetStage* g_pNS = NULL;




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CNetStage::CNetStage( HINSTANCE hInst, TCHAR* strAppName )
{
    g_pNS                = this;
    m_hInst              = hInst;
    m_pDP                = NULL;
    m_pStageDataMsg      = NULL;
    m_bActive            = FALSE;
    m_hDlg               = NULL;
    m_bStageInitialized  = FALSE;
    m_bDialogInitialized = FALSE;
    m_dpnidHostPlayer    = 0;
    m_lLocalSlotNumber   = -1; 

    InitializeCriticalSection( &m_csSlots );
    _tcscpy( m_strAppName, strAppName );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CNetStage::~CNetStage()
{
    DeleteCriticalSection( &m_csSlots );
}




//-----------------------------------------------------------------------------
// Name: Init()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CNetStage::Init( IDirectPlay8Peer* pDP )
{
    HWND    hDlg  = NULL;
    BOOL    bDone = FALSE;

    if( pDP == NULL )
        return CO_E_NOTINITIALIZED;
    m_pDP         = pDP;
    m_bActive     = TRUE;
    m_bHostPlayer = FALSE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DoStage()
// Desc: Starts the DirectPlay stage.  
//-----------------------------------------------------------------------------
HRESULT CNetStage::DoStage( TCHAR* strLocalPlayerName, BOOL bHostPlayer )
{
    HWND    hDlg  = NULL;
    BOOL    bDone = FALSE;

    if( m_pDP == NULL )
        return CO_E_NOTINITIALIZED;

    DXUtil_Trace( TEXT("\n\nNetStage: starting stage\n") );
    
    _tcsncpy( m_strLocalPlayerName, strLocalPlayerName, MAX_PLAYER_NAME );
    m_bHostPlayer = bHostPlayer;
    m_bActive     = TRUE;

    // Setup data for the player
    if( m_bHostPlayer )
    {
        m_dwNumberSlotsOpen = MAX_SLOTS;
        m_bAllowPlayerJoin  = TRUE;

        // Setup the 'is player slot open' array
        ZeroMemory( &m_Slots, sizeof(m_Slots) );
        for( int iPlayer = 0; iPlayer < MAX_SLOTS; iPlayer++ )
        {
            m_Slots[ iPlayer ].dwSlotStatus = SLOT_BUTTON_OPEN;
            m_Slots[ iPlayer ].dpnidPlayer   = 0xFFFFFFFF;  // Not valid yet
        }

        // Host player is always the first slot
        m_lLocalSlotNumber = 0;
        m_Slots[ m_lLocalSlotNumber ].dwSlotStatus = SLOT_BUTTON_FULL; 
        m_Slots[ m_lLocalSlotNumber ].dpnidPlayer   = 0; 
        _tcscpy( m_Slots[ m_lLocalSlotNumber ].strSlotName, m_strLocalPlayerName );

        // Host always has an initialized stage
        m_bStageInitialized = TRUE;
    }

    // Display the multiplayer stage dialog box.
    m_hrDialog = S_OK;

    hDlg = CreateDialog( m_hInst, MAKEINTRESOURCE(IDD_MULTIPLAYER_STAGE), NULL, 
                         (DLGPROC)StaticStageDlgProc  );
    
    ShowWindow( hDlg, SW_SHOWDEFAULT );

    MSG msg;
    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        if( msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN )
        {
            // The enter key was pressed, so send out the chat message
            if( FAILED( m_hrDialog = SendChatMessage( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("SendChatMessage"), m_hrDialog );
                PostMessage( m_hDlg, WM_QUIT, 0, 0 );
            }
        }
        else
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    DestroyWindow( hDlg );

    m_bActive = FALSE;
    m_pDP = NULL;

    // Let the user know what happened.
    if( m_hrDialog != NS_S_FORWARD )
    {
        if( m_hrDialog == NS_S_REJECTED )
        {
            MessageBox( NULL, TEXT("You were dropped from the game."), 
                        TEXT("DirectPlay Multiplayer Stage"), MB_OK );
            m_hrDialog = NS_S_BACKUP;
        }

        if( m_hrDialog == NS_S_GAMECANCELED )
        {
            MessageBox( NULL, TEXT("The game was canceled."), 
                        TEXT("DirectPlay Multiplayer Stage"), MB_OK );
            m_hrDialog = NS_S_BACKUP;
        }
    }

    return m_hrDialog;
}




//-----------------------------------------------------------------------------
// Name: StaticConnectionsDlgProc()
// Desc: Static msg handler which passes messages 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetStage::StaticStageDlgProc( HWND hDlg, UINT uMsg, 
                                                WPARAM wParam, LPARAM lParam )
{
    if( g_pNS )
        return g_pNS->StageDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: StageDlgProc()
// Desc: Handles messages for the multiplayer stage dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetStage::StageDlgProc( HWND hDlg, UINT msg, WPARAM wParam, 
                                          LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
            m_hDlg = hDlg; 
            m_bDialogInitialized = TRUE;
            DXUtil_Trace( TEXT("NetStage: initing dialog m_hDlg=%0.8x\n"), m_hDlg );

            // Set what gets notified when return is pressed 
            if( FAILED( m_hrDialog = StageDlgInit( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("StageDlgInit"), m_hrDialog );
                PostMessage( m_hDlg, WM_QUIT, 0, 0 );
            }

            break;

        case WM_APP_DISPLAY_STAGE:
        {
            // This function is called from the DirectPlay message handler so it could be 
            // called simultaneously from multiple threads, so enter a critical section
            // to assure that it we don't get race conditions.  
            EnterCriticalSection( &m_csSlots );

            // Show the stage when initialization has complete 
            UpdateStageWhenInitComplete();

            LeaveCriticalSection( &m_csSlots );
            break;
        }

        case WM_APP_CHAT:
        {
            HRESULT          hr;
            DPNID            dpnidPlayer = (DPNID) wParam;
            APP_PLAYER_INFO* pPlayerInfo = NULL;

            PLAYER_LOCK(); // enter player context CS

            // Get the player context accosicated with this DPNID
            hr = m_pDP->GetPlayerContext( dpnidPlayer, 
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
            
            STAGEMSG_CHAT* pChatMessage = (STAGEMSG_CHAT*) lParam;

            // Add the message to the local listbox
            HWND hWndChatBox = GetDlgItem( hDlg, IDC_CHAT_LISTBOX );
            int nCount = (int)SendMessage( hWndChatBox, LB_GETCOUNT, 0, 0 );
            if( nCount > MAX_CHAT_STRINGS )
                SendMessage( hWndChatBox, LB_DELETESTRING, 0, 0 );

            // Make the chat string from the player's name and the edit box string
            TCHAR strChatBuffer[MAX_PLAYER_NAME + MAX_CHAT_STRING_LENGTH + 32];
            wsprintf( strChatBuffer, TEXT("<%s> %s"), pPlayerInfo->strPlayerName, pChatMessage->strChatString );

            PLAYER_LOCK(); // leave player context CS
            PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
            PLAYER_UNLOCK(); // leave player context CS            

            // Add it, and make sure it is visible
            int nIndex = (int)SendMessage( hWndChatBox, LB_ADDSTRING, 0, (LPARAM)strChatBuffer );
            SendMessage( hWndChatBox, LB_SETTOPINDEX, nIndex, 0 );

            // Done with the buffer, so return it DirectPlay, 
            // so that the memory can be reused
            m_pDP->ReturnBuffer( pChatMessage->hBufferHandle, 0 );
            break;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_START_GAME: // 'Start game' was pressed
                    // This button should only be enabled if we are the host, and
                    // everything is ready, so we are all ready to start the game
                    if( FAILED( m_hrDialog = StartGame( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("StartGame"), m_hrDialog );
                        PostMessage( m_hDlg, WM_QUIT, 0, 0 );
                    }
                    break;
                    
                case IDCANCEL: // The close dlg button was pressed
                case IDC_BACK: // 'Cancel' was pressed
                    if( m_bHostPlayer )
                    {
                        if( FAILED( m_hrDialog = CancelGame() ) )
                        {
                            DXTRACE_ERR( TEXT("CancelGame"), m_hrDialog );
                            PostMessage( m_hDlg, WM_QUIT, 0, 0 );
                        }
                    }

                    if( LOWORD(wParam) == IDCANCEL )
                        m_hrDialog = NS_S_QUIT;
                    else
                        m_hrDialog = NS_S_BACKUP;

                    PostMessage( m_hDlg, WM_QUIT, 0, 0 );
                    break;

                case IDC_SEND_TEXT:
                    // The send button was pressed, so send out the chat message
                    if( FAILED( m_hrDialog = SendChatMessage( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("SendChatMessage"), m_hrDialog );
                        PostMessage( m_hDlg, WM_QUIT, 0, 0 );
                    }
                    break;

                case IDC_READY_P1:
                case IDC_READY_P2:
                case IDC_READY_P3:
                case IDC_READY_P4:
                case IDC_READY_P5:
                case IDC_READY_P6:
                case IDC_READY_P7:
                case IDC_READY_P8:
                case IDC_READY_P9:
                case IDC_READY_P10:
                    // Only one of these checkboxs should be enabled (the one next to 
                    // the players slot), so change the players status and tell everyone
                    if( FAILED( m_hrDialog = ChangePlayerReadyStatus( hDlg, !m_bLocalPlayerReady ) ) )
                    {
                        DXTRACE_ERR( TEXT("ChangePlayerReadyStatus"), m_hrDialog );
                        PostMessage( m_hDlg, WM_QUIT, 0, 0 );
                    }
                    break;

                case IDC_SLOT_BTN_P2:
                case IDC_SLOT_BTN_P3:
                case IDC_SLOT_BTN_P4:
                case IDC_SLOT_BTN_P5:
                case IDC_SLOT_BTN_P6:
                case IDC_SLOT_BTN_P7:
                case IDC_SLOT_BTN_P8:
                case IDC_SLOT_BTN_P9:
                case IDC_SLOT_BTN_P10:
                    // Only the host has access to use these buttons, and can not reject
                    // himself.  So figure out which slot button was pressed and 
                    // change its status
                    if( m_bHostPlayer )
                    {
                        DWORD dwSlot = LOWORD(wParam) - IDC_SLOT_BTN_P2 + 1;

                        if( FAILED( m_hrDialog = ChangePlayerSlotStatus( hDlg, dwSlot ) ) )
                        {
                            DXTRACE_ERR( TEXT("ChangePlayerSlotStatus"), m_hrDialog );
                            PostMessage( m_hDlg, WM_QUIT, 0, 0 );
                        }
                    }
                    break;

                default:
                    return FALSE; // Message not handled
            }
            break;

        default:
            return FALSE; // Message not handled
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: StageDlgInit()
// Desc: Setup the dialog for both host and players
//-----------------------------------------------------------------------------
HRESULT CNetStage::StageDlgInit( HWND hDlg )
{
    HRESULT hr;
    int nIDDlgItem; 
    TCHAR strWindowTitle[256];

    // Load and set the icon
    HICON hIcon = LoadIcon( m_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Set the window title
    wsprintf( strWindowTitle, TEXT("%s - Multiplayer Stage"), m_strAppName );
    SetWindowText( hDlg, strWindowTitle );

    m_bLocalPlayerReady = FALSE;

    // Disable all of the ready checkboxes.  
    for( nIDDlgItem = IDC_READY_P1; nIDDlgItem <= IDC_READY_P10; nIDDlgItem++ )
        EnableWindow( GetDlgItem( hDlg, nIDDlgItem ), FALSE );

    ShowWindow( hDlg, SW_HIDE );

    if( m_bHostPlayer )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_START_GAME ), FALSE );
        SetDlgItemText( hDlg, IDC_SLOT_TXT_P1, m_strLocalPlayerName );

        // Show all of the stage buttons controls, and hide the stage static text controls.  
        for( nIDDlgItem = IDC_SLOT_BTN_P2; nIDDlgItem <= IDC_SLOT_BTN_P10; nIDDlgItem++ )
            ShowWindow( GetDlgItem( hDlg, nIDDlgItem ), SW_SHOW );
        for( nIDDlgItem = IDC_SLOT_TXT_P2; nIDDlgItem <= IDC_SLOT_TXT_P10; nIDDlgItem++ )
            ShowWindow( GetDlgItem( hDlg, nIDDlgItem ), SW_HIDE );

        // Display our current state to the local player
        if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
            return DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );
    }
    else
    {
        // Non-host players can not start a game
        ShowWindow( GetDlgItem( hDlg, IDC_START_GAME ), SW_HIDE );

        // Hide all of the stage buttons controls, and show the stage static text controls.  
        for( nIDDlgItem = IDC_SLOT_BTN_P2; nIDDlgItem <= IDC_SLOT_BTN_P10; nIDDlgItem++ )
            ShowWindow( GetDlgItem( hDlg, nIDDlgItem ), SW_HIDE );
        for( nIDDlgItem = IDC_SLOT_TXT_P1; nIDDlgItem <= IDC_SLOT_TXT_P10; nIDDlgItem++ )
            ShowWindow( GetDlgItem( hDlg, nIDDlgItem ), SW_SHOW );

        if( m_pStageDataMsg )
            DisplaySlotData( hDlg, m_pStageDataMsg );
    }

    // Show the stage when initialization has complete 
    PostMessage( hDlg, WM_APP_DISPLAY_STAGE, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MessageHandler
// Desc: Handler for DirectPlay messages.
//-----------------------------------------------------------------------------
HRESULT WINAPI CNetStage::MessageHandler( PVOID pvUserContext, 
                                          DWORD dwMessageId, 
                                          PVOID pMsgBuffer )
{
    HRESULT hr;

    if( m_pDP == NULL )
        return CO_E_NOTINITIALIZED;

    if( !m_bActive )
        return S_OK;

    // Try not to stay in this message handler for too long, otherwise
    // there will be a backlog of data.  The best solution is to 
    // queue data as it comes in, and then handle it on other threads.
    
    switch(dwMessageId)
    {
        case DPN_MSGID_RECEIVE:
        {
	        PDPNMSG_RECEIVE pReceiveMsg;
            pReceiveMsg = (PDPNMSG_RECEIVE)pMsgBuffer;

            STAGEMSG_GENERIC* pStageMsg = (STAGEMSG_GENERIC*) pReceiveMsg->pReceiveData;
            return HandleAppMessages( m_hDlg, pStageMsg, pReceiveMsg );

            break;
        }

        case DPN_MSGID_CREATE_PLAYER:
        {
        	PDPNMSG_CREATE_PLAYER pCreatePlayerMsg;
            pCreatePlayerMsg = (PDPNMSG_CREATE_PLAYER)pMsgBuffer;

            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pCreatePlayerMsg->pvPlayerContext;

            PLAYER_ADDREF( pPlayerInfo ); // addref player, since we are using it now

            if( (pPlayerInfo->dwFlags & DPNPLAYER_HOST ) != 0 )
            {
                m_dpnidHostPlayer = pPlayerInfo->dpnidPlayer;
                DXUtil_Trace( TEXT("NetStage: Host ID query: Host set to %0.8x\n"), m_dpnidHostPlayer );
            }

            // If we are the host player, then add this 
            // player to the stage and tell everyone about it 
            if( m_bHostPlayer )
            {
                APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pCreatePlayerMsg->pvPlayerContext;

                DXUtil_Trace( TEXT("NetStage: adding player (name=%s, dpid=%0.8x, context=%0.8x)\n"), pPlayerInfo->strPlayerName, pCreatePlayerMsg->dpnidPlayer, pPlayerInfo );

                if( FAILED( hr = AddPlayerToStage( m_hDlg, pPlayerInfo ) ) )
                    return DXTRACE_ERR( TEXT("AddPlayerToStage"), hr );
            }  

            break;
        }

        case DPN_MSGID_DESTROY_PLAYER:
        {
        	PDPNMSG_DESTROY_PLAYER pDestroyPlayerMsg;
            pDestroyPlayerMsg = (PDPNMSG_DESTROY_PLAYER)pMsgBuffer;
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pDestroyPlayerMsg->pvPlayerContext;

            // If we are the host player, then remove this player from the stage 
            // and tell everyone about it 
            if( m_bHostPlayer )
            {
                DXUtil_Trace( TEXT("NetStage: removing player (name=%s, dpid=%0.8x, context=%0.8x)\n"), pPlayerInfo->strPlayerName, pPlayerInfo->dpnidPlayer, pPlayerInfo );

                if( FAILED( hr = RemovePlayerToStage( m_hDlg, pPlayerInfo ) ) )
                    return DXTRACE_ERR( TEXT("RemovePlayerToStage"), hr );
            }

            PLAYER_LOCK();                  // enter player context CS
            PLAYER_RELEASE( pPlayerInfo );  // Release player and cleanup if needed
            PLAYER_UNLOCK();                // leave player context CS
            break;
        }

        case DPN_MSGID_TERMINATE_SESSION:
        {
            PDPNMSG_TERMINATE_SESSION pTerminateSessionMsg;
            pTerminateSessionMsg = (PDPNMSG_TERMINATE_SESSION)pMsgBuffer;

            // if *pvTerminateData contains NS_S_REJECTED then the host 
            // rejected this player
            if( pTerminateSessionMsg->pvTerminateData != NULL &&
                pTerminateSessionMsg->dwTerminateDataSize == sizeof(DWORD) &&
                *((DWORD*) (pTerminateSessionMsg->pvTerminateData)) == NS_S_REJECTED )
            {
                // Non-host only message.  This message is sent by the host when a 
                // player is rejected from the game either because there are no
                // free slots left or the host manually rejected the player.
                m_hrDialog = NS_S_REJECTED;
            }
            else
            {
                m_hrDialog = DPNERR_CONNECTIONLOST;
            }

            PostMessage( m_hDlg, WM_QUIT, 0, 0 );
            break;
        }
    }
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: HandleAppMessages()
// Desc: Evaluates application messages and performs appropriate actions
//-----------------------------------------------------------------------------
HRESULT CNetStage::HandleAppMessages( HWND hDlg, CNetStage::STAGEMSG_GENERIC* pMsg, 
                                      PDPNMSG_RECEIVE pReceiveMsg )
{
    HRESULT hr;
    HRESULT hReturn = S_OK;

    switch( pMsg->dwMessageID )
    {
        case STAGE_MSGID_SLOTCHECK:
        {
            // Host only message.  This message is sent by players when they 
            // have marked check box as ready
            if( !m_bHostPlayer )
                return E_FAIL; // Sanity check

            // Update global slot status, and send it to everyone
            STAGEMSG_SLOT* pSlotIDMsg = (STAGEMSG_SLOT*) pMsg;

            // This function is called from the DirectPlay message handler so it could be 
            // called simultaneously from multiple threads, so enter a critical section
            // to assure that it we don't get race conditions.  
            EnterCriticalSection( &m_csSlots );
            m_Slots[ pSlotIDMsg->lSlotNumber ].dwSlotStatus |= SLOT_READY_CHECKED;
            LeaveCriticalSection( &m_csSlots );

            if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
                return DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );
            break;
        }

        case STAGE_MSGID_SLOTUNCHECK:
        {
            // Host only message.  This message is sent by players when they 
            // have marked their check box as unready
            if( !m_bHostPlayer )
                return E_FAIL; // Sanity check

            // Update global slot status, and send it to everyone
            STAGEMSG_SLOT* pSlotIDMsg = (STAGEMSG_SLOT*) pMsg;

            // This function is called from the DirectPlay message handler so it could be 
            // called simultaneously from multiple threads, so enter a critical section
            // to assure that it we don't get race conditions.  
            EnterCriticalSection( &m_csSlots );
            m_Slots[ pSlotIDMsg->lSlotNumber ].dwSlotStatus &= ~SLOT_READY_CHECKED;
            LeaveCriticalSection( &m_csSlots );

            if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
                return DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );
            break;
        }

        case STAGE_MSGID_SETSLOTID:
        {
            // Non-host only message. This message is sent by the host player
            // when a player joins the stage.  The host is telling the player 
            // what slot number this player has been assigned.
            if( m_bHostPlayer )
                return E_FAIL; // Sanity check

            // Set the local slot ID given to us by the host
            STAGEMSG_SLOT* pSlotIDMsg = (STAGEMSG_SLOT*) pMsg;
            m_lLocalSlotNumber = pSlotIDMsg->lSlotNumber;

            DXUtil_Trace( TEXT("NetStage: got slot id = %d\n"), m_lLocalSlotNumber );            
            assert( m_lLocalSlotNumber >= 0 );
            m_bStageInitialized = TRUE;

            // Show the stage when initialization has complete 
            PostMessage( hDlg, WM_APP_DISPLAY_STAGE, 0, 0 );
            break;
        }

        case STAGE_MSGID_SETSTAGEDATA:
        {
            // This message is sent by the host when the stage data changes. 
            // The players take this data and display it in the dialog.
            STAGEMSG_STAGEDATA* pStageDataMsg = (STAGEMSG_STAGEDATA*) pMsg;

            if( m_pStageDataMsg )
                m_pDP->ReturnBuffer( m_pStageDataMsg->hBufferHandle, 0 );

            DXUtil_Trace( TEXT("NetStage: got staged data\n") );
            assert( pStageDataMsg != NULL );

            // This function is called from the DirectPlay message handler so it could be 
            // called simultaneously from multiple threads, so enter a critical section
            // to assure that it we don't get race conditions.  
            EnterCriticalSection( &m_csSlots );

            m_pStageDataMsg = pStageDataMsg;
            m_pStageDataMsg->hBufferHandle = pReceiveMsg->hBufferHandle;

            LeaveCriticalSection( &m_csSlots );

            // Show the stage when initialization has complete 
            PostMessage( hDlg, WM_APP_DISPLAY_STAGE, 0, 0 );

            return DPNSUCCESS_PENDING;
        }

        case STAGE_MSGID_CHAT:
        {
            APP_PLAYER_INFO* pPlayerInfo = (APP_PLAYER_INFO*) pReceiveMsg->pvPlayerContext;

            // This message is sent when a player has send a chat message to us, so 
            // post a message to the dialog thread to update the UI.  
            // This keeps the DirectPlay threads from blocking, and also
            // serializes the recieves since DirectPlayMessageHandler can
            // be called simultaneously from a pool of DirectPlay threads.
            STAGEMSG_CHAT* pChatMessage = (STAGEMSG_CHAT*) pMsg;

            // Record the buffer handle so the buffer can be returned later 
            pChatMessage->hBufferHandle = pReceiveMsg->hBufferHandle;

            PostMessage( m_hDlg, WM_APP_CHAT, 
                         pPlayerInfo->dpnidPlayer, (LPARAM) pChatMessage );

            // Tell DirectPlay to assume that ownership of the buffer 
            // has been transferred to the application, and so it will 
            // neither free nor modify it until ownership is returned 
            // to DirectPlay through the ReturnBuffer() call.
            hReturn = DPNSUCCESS_PENDING;
            break;
        }

        case STAGE_MSGID_STARTGAME:
        {
            // Non-host only message.  This message is sent by the host when  
            // the everyone is ready, and the host has started the game.
            if( m_bHostPlayer )
                return E_FAIL; // Sanity check

            DXUtil_Trace( TEXT("NetStage: Got STAGE_MSGID_STARTGAME\n") ); 

            m_hrDialog = NS_S_FORWARD;
            PostMessage( m_hDlg, WM_QUIT, 0, 0 );
            break;
        }

        case STAGE_MSGID_CANCELGAME:
        {
            // This message is sent by the host when  
            // the host has canceled the game.
            m_hrDialog = NS_S_GAMECANCELED;
            PostMessage( m_hDlg, WM_QUIT, 0, 0 );
            break;
        }
    }

    return hReturn;
}




//-----------------------------------------------------------------------------
// Name: SendChatMessage()
// Desc: Create chat string based on the editbox and send it to everyone 
//-----------------------------------------------------------------------------
HRESULT CNetStage::SendChatMessage( HWND hDlg )
{
    // Get length of item text
    DWORD dwEditboxBufferSize = (DWORD)SendDlgItemMessage( hDlg, IDC_CHAT_EDIT, 
                                                           WM_GETTEXTLENGTH, 0, 0 );
    if( dwEditboxBufferSize == 0 )
        return S_OK;  // Don't do anything for blank messages 

    STAGEMSG_CHAT msgChat;
    msgChat.dwMessageID = STAGE_MSGID_CHAT;
    GetDlgItemText( hDlg, IDC_CHAT_EDIT, msgChat.strChatString, MAX_CHAT_STRING_LENGTH );

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(STAGEMSG_CHAT);
    bufferDesc.pBufferData  = (BYTE*) &msgChat;

    // Send it to all of the players include the local client
    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    m_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                   0, NULL, &hAsync, DPNSEND_GUARANTEED );

    // Blank out edit box
    SetDlgItemText( hDlg, IDC_CHAT_EDIT, TEXT("") );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: AddChatStringToListBox()
// Desc: Adds a string to the list box and ensures it is visible 
//-----------------------------------------------------------------------------
VOID CNetStage::AddChatStringToListBox( HWND hDlg, LPSTR strMsgText )
{
    // Add the message to the local listbox
    HWND hWndChatBox = GetDlgItem( hDlg, IDC_CHAT_LISTBOX );
    int nCount = (int)SendMessage( hWndChatBox, LB_GETCOUNT, 0, 0 );
    if( nCount > MAX_CHAT_STRINGS )
        SendMessage( hWndChatBox, LB_DELETESTRING, 0, 0 );

    // Add it, and make sure it is visible
    int nIndex = (int)SendMessage( hWndChatBox, LB_ADDSTRING, 0, (LPARAM)strMsgText );
    SendMessage( hWndChatBox, LB_SETTOPINDEX, nIndex, 0 );
}




//-----------------------------------------------------------------------------
// Name: GetFreePlayerSlot()
// Desc: Host only.  Returns the first open slot or S_FALSE if non-found
//-----------------------------------------------------------------------------
HRESULT CNetStage::GetFreePlayerSlot( DWORD* pdwPlayerSlotID )
{
    DWORD iSlot;

    if( !m_bHostPlayer )
        return E_FAIL; // Sanity check

    for( iSlot = 0; iSlot < MAX_SLOTS; iSlot++ )
    {
        if( SLOT_BUTTON_OPEN == (m_Slots[ iSlot ].dwSlotStatus & SLOT_BUTTON_MASK) ) 
        {
            *pdwPlayerSlotID = iSlot;
            return S_OK;
        }
    }

    // No open slot found
    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: SendSlotDataToPlayers()
// Desc: Host Only. Sends the slot data to all the players, and displays the 
//       same data on the local machine.  Other players will display this 
//       data when they receieve it.  
//-----------------------------------------------------------------------------
HRESULT CNetStage::SendSlotDataToPlayers( HWND hDlg )
{
    if( !m_bHostPlayer )
        return E_FAIL; // Sanity check

    STAGEMSG_STAGEDATA msgSlotData;
    msgSlotData.dwMessageID = STAGE_MSGID_SETSTAGEDATA;

    // This function is called from the DirectPlay message handler so it could be 
    // called simultaneously from multiple threads, so enter a critical section
    // to assure that it we don't get race conditions.  
    EnterCriticalSection( &m_csSlots );

    // Fill the STAGEMSG_STAGEDATA message
    memcpy( &msgSlotData.Slots, &m_Slots, sizeof(SLOT_INFO)*MAX_SLOTS );

    LeaveCriticalSection( &m_csSlots );

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(STAGEMSG_STAGEDATA);
    bufferDesc.pBufferData  = (BYTE*) &msgSlotData;

    DXUtil_Trace( TEXT("NetStage: sending STAGE_MSGID_SETSTAGEDATA to all players\n") );

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    m_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                   0, NULL, &hAsync, DPNSEND_GUARANTEED );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DisplaySlotData()
// Desc: Display the new slot data for each of the slots, updating the 
//       button's text and also the ready checkbox.  Also update the stage 
//       status message 
//-----------------------------------------------------------------------------
VOID CNetStage::DisplaySlotData( HWND hDlg, STAGEMSG_STAGEDATA* pStageDataMsg )
{
    BOOL bEveryOneReady = TRUE;
    TCHAR strStatus[256];

    m_dwNumberOfFullSlots = 0;

    for( int iSlot = 0; iSlot < MAX_SLOTS; iSlot++ )
    {
        // Update the button and checkbox status based on the 
        // STAGEMSG_STAGEDATA message.
        switch( pStageDataMsg->Slots[iSlot].dwSlotStatus & SLOT_BUTTON_MASK ) 
        {
            case SLOT_BUTTON_OPEN:
                SetDlgItemText( hDlg, IDC_SLOT_BTN_P2 - 1 + iSlot, TEXT("Open") );
                SetDlgItemText( hDlg, IDC_SLOT_TXT_P1 + iSlot, TEXT("Open") );
                CheckDlgButton( hDlg, IDC_READY_P1 + iSlot, BST_UNCHECKED );
                break;

            case SLOT_BUTTON_CLOSED:
                SetDlgItemText( hDlg, IDC_SLOT_BTN_P2 - 1 + iSlot, TEXT("Closed") );
                SetDlgItemText( hDlg, IDC_SLOT_TXT_P1 + iSlot, TEXT("Closed") );
                CheckDlgButton( hDlg, IDC_READY_P1 + iSlot, BST_UNCHECKED );
                break;

            case SLOT_BUTTON_FULL:
                if( iSlot != 0 )
                    SetDlgItemText( hDlg, IDC_SLOT_BTN_P2 - 1 + iSlot, 
                                    pStageDataMsg->Slots[iSlot].strSlotName );

                SetDlgItemText( hDlg, IDC_SLOT_TXT_P1 + iSlot, 
                                pStageDataMsg->Slots[iSlot].strSlotName );

                // Check the STAGEMSG_STAGEDATA msg to see if that player is ready
                if( ( pStageDataMsg->Slots[iSlot].dwSlotStatus & SLOT_READY_MASK ) 
                                                    == SLOT_READY_CHECKED )
                {
                    CheckDlgButton( hDlg, IDC_READY_P1 + iSlot, BST_CHECKED );
                }
                else
                {
                    CheckDlgButton( hDlg, IDC_READY_P1 + iSlot, BST_UNCHECKED );
                    bEveryOneReady = FALSE;
                }

                m_dwNumberOfFullSlots++;
                break;
        }
    }

    // Keep the start game button disabled until everything is ready
    EnableWindow( GetDlgItem( hDlg, IDC_START_GAME ), FALSE );

    // Figure out what the status message should be
    if( bEveryOneReady )
    {
        if( m_bHostPlayer )
        {
            if( m_dwNumberOfFullSlots == 1 )
            {
                _tcscpy( strStatus, TEXT("Waiting for more players to join.") );
            }
            else
            {
                _tcscpy( strStatus, TEXT("All are ready! Waiting for you to start the game.") );

                // We are all set to start the game, so enable the button
                EnableWindow( GetDlgItem( hDlg, IDC_START_GAME ), TRUE );
            }
        }
        else
        {
            _tcscpy( strStatus, TEXT("All are ready! Waiting for host to start the game.") );
        }
    }
    else
    {
        if( m_bLocalPlayerReady )
            _tcscpy( strStatus, TEXT("Waiting until all players check the Ready flag.") );
        else
            _tcscpy( strStatus, TEXT("Ready?  Flag the checkbox next to your name.") );
    }   

    // Set the status text on the dialog
    SetDlgItemText( hDlg, IDC_STATUS_TEXT, strStatus );
}




//-----------------------------------------------------------------------------
// Name: AddPlayerToStage()
// Desc: Host Only. Adds a new player to an empty slot, if one exists and 
//       tells everyone that the new player has joined the stage
//-----------------------------------------------------------------------------
HRESULT CNetStage::AddPlayerToStage( HWND hDlg, APP_PLAYER_INFO* pPlayerInfo )
{
    DWORD         dwPlayerSlot;
    HRESULT       hr;

    if( !m_bHostPlayer && pPlayerInfo != NULL )
        return E_FAIL; // Sanity check

    // This function is called from the DirectPlay message handler so it could be 
    // called simultaneously from multiple threads, so enter a critical section
    // to assure that it we don't get race conditions.  
    EnterCriticalSection( &m_csSlots );

    // Get the next free slot id if there is one
    hr = GetFreePlayerSlot( &dwPlayerSlot );

    if( hr != S_OK )
    {
        LeaveCriticalSection( &m_csSlots );

        if( FAILED(hr) ) 
            return hr;

        // Call DestroyPeer() to drop the player.  This will force the player
        // out of the connection.  We also send the player NS_S_REJECTED,
        // so the app can popup UI informing the user that they were
        // rejected.
        DWORD dwDestroyData = NS_S_REJECTED;
        m_pDP->DestroyPeer( pPlayerInfo->dpnidPlayer, &dwDestroyData, sizeof(DWORD), 0 );
        
        return S_OK;
    }

    // Update slot info
    m_Slots[ dwPlayerSlot ].dwSlotStatus = SLOT_BUTTON_FULL;
    _tcscpy( m_Slots[ dwPlayerSlot ].strSlotName, pPlayerInfo->strPlayerName );
    m_Slots[ dwPlayerSlot ].dpnidPlayer = pPlayerInfo->dpnidPlayer;

    LeaveCriticalSection( &m_csSlots );

    // Update player info
    pPlayerInfo->dwSlotIndex = dwPlayerSlot;

    // Send the new player the slot id assigned to it by the host session
    STAGEMSG_SLOT msgSlotID;
    msgSlotID.dwMessageID = STAGE_MSGID_SETSLOTID;
    msgSlotID.lSlotNumber = dwPlayerSlot;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(STAGEMSG_SLOT);
    bufferDesc.pBufferData  = (BYTE*) &msgSlotID;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    m_pDP->SendTo( pPlayerInfo->dpnidPlayer, &bufferDesc, 1,
                   0, NULL, &hAsync, DPNSEND_GUARANTEED );

    DXUtil_Trace( TEXT("NetStage: sending new player (dpid=%0.8x) a STAGE_MSGID_SETSLOTID message\n"), pPlayerInfo->dpnidPlayer );

    // Send the new slot data to everyone
    if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
        return DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RemovePlayerToStage()
// Desc: Host Only. Removes the player from the slot and 
//       tells everyone that the player has been removed from the stage
//-----------------------------------------------------------------------------
HRESULT CNetStage::RemovePlayerToStage( HWND hDlg, APP_PLAYER_INFO* pPlayerInfo )
{
    HRESULT hr = S_OK;

    if( !m_bHostPlayer && pPlayerInfo != NULL  )
        return E_FAIL; // Sanity check

    // This function is called from the DirectPlay message handler so it could be 
    // called simultaneously from multiple threads, so enter a critical section
    // to assure that it we don't get race conditions.  
    EnterCriticalSection( &m_csSlots );

    // Update the slot info to make this player's slot open.
    if( SLOT_BUTTON_FULL == (m_Slots[ pPlayerInfo->dwSlotIndex ].dwSlotStatus & SLOT_BUTTON_MASK) )
    {
        m_Slots[ pPlayerInfo->dwSlotIndex ].dwSlotStatus = SLOT_BUTTON_OPEN;
        m_Slots[ pPlayerInfo->dwSlotIndex ].dpnidPlayer = 0xFFFFFFFF;
        _tcscpy( m_Slots[ pPlayerInfo->dwSlotIndex ].strSlotName, TEXT("") );

        // Tell everyone about the change
        if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
        {
            DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );
            goto LCleanup;
        }
    }

    LeaveCriticalSection( &m_csSlots );

LCleanup:
    return hr;
}




//-----------------------------------------------------------------------------
// Name: UpdateSessionDesc()
// Desc: Host Only. Tells DirectPlay about changes the max players allowed 
//       in this session 
//-----------------------------------------------------------------------------
HRESULT CNetStage::UpdateSessionDesc()
{
    HRESULT hr;

    if( !m_bHostPlayer )
        return E_FAIL; // Sanity check

    // Get the application description
    DPN_APPLICATION_DESC* pAppDesc = NULL;
    DWORD dwSize = 0;
    hr = m_pDP->GetApplicationDesc( pAppDesc, &dwSize, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL )
        return DXTRACE_ERR( TEXT("GetApplicationDesc"), hr );
    pAppDesc = (DPN_APPLICATION_DESC*) new BYTE[dwSize];
    ZeroMemory( pAppDesc, sizeof(DPN_APPLICATION_DESC) );
    pAppDesc->dwSize = sizeof(DPN_APPLICATION_DESC); 
    hr = m_pDP->GetApplicationDesc( pAppDesc, &dwSize, 0 );
    if( FAILED( hr ) ) 
        return DXTRACE_ERR( TEXT("GetApplicationDesc"), hr );

    pAppDesc->dwSize = sizeof(DPN_APPLICATION_DESC); 
    // Change the 'max players' to what it should be
    pAppDesc->dwMaxPlayers = m_dwNumberSlotsOpen;

    // Tell DirectPlay about the change
    if( FAILED( hr = m_pDP->SetApplicationDesc( pAppDesc, 0 ) ) )
        return DXTRACE_ERR( TEXT("SetApplicationDesc"), hr );

    // Cleanup the data
    SAFE_DELETE_ARRAY( pAppDesc );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ChangePlayerReadyStatus()
// Desc: The players has clicked on the player's ready checkbox, so tell
//       everyone that his has happened.
//-----------------------------------------------------------------------------
HRESULT CNetStage::ChangePlayerReadyStatus( HWND hDlg, BOOL bPlayerReady )
{
    HRESULT hr = S_OK;

    m_bLocalPlayerReady = bPlayerReady;

    // This function is called from the dialog UI, but the DirectPlay message 
    // handler threads simultaneously change m_Slots, so enter a critical section
    // to assure that it we don't get race conditions.  
    EnterCriticalSection( &m_csSlots );

    if( m_bHostPlayer )
    {
        // Update the global slot data, and tell everyone.
        if( m_bLocalPlayerReady )
            m_Slots[ m_lLocalSlotNumber ].dwSlotStatus |= SLOT_READY_CHECKED;
        else
            m_Slots[ m_lLocalSlotNumber ].dwSlotStatus &= ~SLOT_READY_CHECKED;
    
        if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
        {
            DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );
            goto LCleanup;
        }
    }
    else
    {
        STAGEMSG_SLOT msgSlotCheck;

        // Send a message to the host player that this player has
        // changed their ready status.  The host player will in turn
        // tell everyone about the change
        msgSlotCheck.lSlotNumber = m_lLocalSlotNumber;
    
        // Send a different message based on the local ready status
        if( m_bLocalPlayerReady )
            msgSlotCheck.dwMessageID = STAGE_MSGID_SLOTCHECK;
        else
            msgSlotCheck.dwMessageID = STAGE_MSGID_SLOTUNCHECK;

        DPN_BUFFER_DESC bufferDesc;
        bufferDesc.dwBufferSize = sizeof(STAGEMSG_SLOT);
        bufferDesc.pBufferData  = (BYTE*) &msgSlotCheck;

        // DirectPlay will tell via the message handler 
        // if there are any severe errors, so ignore any errors 
        DPNHANDLE hAsync;
        m_pDP->SendTo( m_dpnidHostPlayer, &bufferDesc, 1,
                       0, NULL, &hAsync, DPNSEND_GUARANTEED );
    }

LCleanup:
    LeaveCriticalSection( &m_csSlots );
    
    return hr;
}




//-----------------------------------------------------------------------------
// Name: ChangePlayerSlotStatus()
// Desc: Host Only. The host has clicked on a player's button, so either reject 
//       that player or if the slot is empty, then flip it's status between 
//       open and closed
//-----------------------------------------------------------------------------
HRESULT CNetStage::ChangePlayerSlotStatus( HWND hDlg, DWORD dwSlot )
{
    HRESULT hr;
    int nResult;

    if( !m_bHostPlayer )
        return E_FAIL; // Sanity check

    // This function is called from the dialog UI, but the DirectPlay message 
    // handler threads simultaneously change m_Slots, so enter a critical section
    // to assure that it we don't get race conditions.  
    EnterCriticalSection( &m_csSlots );

    switch( m_Slots[ dwSlot ].dwSlotStatus & SLOT_BUTTON_MASK )
    {
        case SLOT_BUTTON_FULL:
        {
            TCHAR strQuestion[MAX_PLAYER_NAME + 256];
            wsprintf( strQuestion, TEXT("Do you want to reject %s?"), m_Slots[ dwSlot ].strSlotName );
            nResult = MessageBox( hDlg, strQuestion, TEXT("DirectPlay Multiplayer Stage"), MB_YESNO );
            if( nResult == IDNO )
            {
                hr = S_OK;
                goto LCleanup;
            }

            // Call DestroyPeer() to drop the player.  This will force the player
            // out of the connection.  We also send the player NS_S_REJECTED,
            // so the app can popup UI informing the user that they were
            // rejected.
            DWORD dwDestroyData = NS_S_REJECTED;
            m_pDP->DestroyPeer( m_Slots[dwSlot].dpnidPlayer, &dwDestroyData, sizeof(DWORD), 0 );

            // The slot status will automatically change to open when the 
            // player is dropped from the connection -- see 
            // DPN_MSGID_DESTROY_PLAYER
            break;
        }
        

        case SLOT_BUTTON_OPEN:
            // Flip the slot status, and tell DirectPlay the max number of 
            // players has changed because of this
            m_Slots[ dwSlot ].dwSlotStatus = SLOT_BUTTON_CLOSED;
            m_dwNumberSlotsOpen--;

            if( FAILED( hr = UpdateSessionDesc() ) )
            {
                DXTRACE_ERR( TEXT("UpdateSessionDesc"), hr );
                goto LCleanup;
            }
            break;

        case SLOT_BUTTON_CLOSED:
            // Flip the slot status, and tell DirectPlay the max number of 
            // players has changed because of this
            m_Slots[ dwSlot ].dwSlotStatus = SLOT_BUTTON_OPEN;
            m_dwNumberSlotsOpen++;

            if( FAILED( hr = UpdateSessionDesc() ) )
            {
                DXTRACE_ERR( TEXT("UpdateSessionDesc"), hr );
                goto LCleanup;
            }
            break;
    }

    // Tell everyone about the new slot data 
    if( FAILED( hr = SendSlotDataToPlayers( hDlg ) ) )
    {
        DXTRACE_ERR( TEXT("SendSlotDataToPlayers"), hr );
        goto LCleanup;
    }

LCleanup:
    LeaveCriticalSection( &m_csSlots );
    
    return hr;
}




//-----------------------------------------------------------------------------
// Name: StartGame()
// Desc: Host Only. The host has pressed the start game buttton.  The button
//       should only be enabled when everyone is ready, and more than 1 player
//       in the stage.  
//-----------------------------------------------------------------------------
HRESULT CNetStage::StartGame( HWND hDlg )
{
    HRESULT hr;

    if( !m_bHostPlayer )
        return E_FAIL; // Sanity check

    // Stop new players from joining the game after stage has closed,
    // and everyone has decided to begin a new game. Also set the max
    // number of players to be the number of slots that are currently full
    // since no new players can join anymore.
    m_bAllowPlayerJoin = FALSE;
    m_dwNumberSlotsOpen = m_dwNumberOfFullSlots;
    if( FAILED( hr = UpdateSessionDesc() ) )
        return hr;

    STAGEMSG_GENERIC msgStartGame;
    msgStartGame.dwMessageID = STAGE_MSGID_STARTGAME;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(STAGEMSG_GENERIC);
    bufferDesc.pBufferData  = (BYTE*) &msgStartGame;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    m_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                   0, NULL, &hAsync, DPNSEND_GUARANTEED );

    m_hrDialog = NS_S_FORWARD;
    PostMessage( m_hDlg, WM_QUIT, 0, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CancelGame()
// Desc: Host Only. The host has pressed the cancel game buttton.  So tell 
//       everyone that the game was canceled
//-----------------------------------------------------------------------------
HRESULT CNetStage::CancelGame()
{
    if( !m_bHostPlayer )
        return E_FAIL; // Sanity check

    STAGEMSG_GENERIC msgCancelGame;
    msgCancelGame.dwMessageID = STAGE_MSGID_CANCELGAME;

    DPN_BUFFER_DESC bufferDesc;
    bufferDesc.dwBufferSize = sizeof(STAGEMSG_GENERIC);
    bufferDesc.pBufferData  = (BYTE*) &msgCancelGame;

    // DirectPlay will tell via the message handler 
    // if there are any severe errors, so ignore any errors 
    DPNHANDLE hAsync;
    m_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP, &bufferDesc, 1,
                   0, NULL, &hAsync, DPNSEND_NOLOOPBACK | DPNSEND_GUARANTEED );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateStageWhenInitComplete()
// Desc: 
//-----------------------------------------------------------------------------
VOID CNetStage::UpdateStageWhenInitComplete()
{
    // Only update the stage if the initialization of both the data and the 
    // dialog has competed
    if( m_bStageInitialized && m_pStageDataMsg != NULL && m_bDialogInitialized )
    {
        assert( m_lLocalSlotNumber >= 0 );
        assert( m_hDlg != NULL );
        assert( IsWindow(m_hDlg) );

        DXUtil_Trace( TEXT("NetStage: updating stage (m_hDlg=%0.8x m_lLocalSlotNumber=%d m_pStageDataMsg=%0.8x\n)"), m_hDlg, m_lLocalSlotNumber, m_pStageDataMsg );

        // Display the slot data
        DisplaySlotData( m_hDlg, m_pStageDataMsg );

        // The first time through the window is hidden, so show it now, and 
        // and also enable our ready checkbox.
        
        // Disable all of the ready checkboxes.  
        int nIDDlgItem; 
        for( nIDDlgItem = IDC_READY_P1; nIDDlgItem <= IDC_READY_P10; nIDDlgItem++ )
            EnableWindow( GetDlgItem( m_hDlg, nIDDlgItem ), FALSE );
              
        EnableWindow( GetDlgItem( m_hDlg, IDC_READY_P1 + m_lLocalSlotNumber ), TRUE );
        ShowWindow( m_hDlg, SW_SHOW );
    }
}


