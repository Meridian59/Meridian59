//-----------------------------------------------------------------------------
// File: NetClient.cpp
//
// Desc: This is a class that given a IDirectPlay8Client upon DoConnectWizard()
//       will enumerate hosts, and allows the user to join a session.  The class uses
//       dialog boxes and GDI for the interactive UI.  Most games will
//       want to change the graphics to use Direct3D or another graphics
//       layer, but this simplistic sample uses dialog boxes. Feel 
//       free to use this class as a starting point for adding extra 
//       functionality.
//
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <stdio.h>
#include <mmsystem.h>
#include <dxerr8.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <dplobby8.h>
#include "NetClient.h"
#include "NetClientRes.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
CNetClientWizard* g_pNCW = NULL;           // Pointer to the net connect wizard




//-----------------------------------------------------------------------------
// Name: CNetClientWizard
// Desc: Init the class
//-----------------------------------------------------------------------------
CNetClientWizard::CNetClientWizard( HINSTANCE hInst, TCHAR* strAppName,
                                      GUID* pGuidApp )
{
    g_pNCW    = this;
    m_hInst   = hInst;
    m_guidApp = *pGuidApp;
    _tcscpy( m_strAppName, strAppName );

    m_dwEnumHostExpireInterval          = 0;
    m_hConnectCompleteEvent             = NULL;
    m_hrConnectComplete                 = 0;
    m_bConnecting                       = FALSE;
    m_bEnumListChanged                  = FALSE;
    m_bSearchingForSessions             = FALSE;
    m_hEnumAsyncOp                      = NULL;
    m_hConnectAsyncOp                   = NULL;
    m_pDPClient                         = NULL;
    m_pLobbiedApp                       = NULL;
    m_bHaveConnectionSettingsFromLobby  = FALSE;
    m_hLobbyClient                      = NULL;
    m_hDlg                              = NULL;

    InitializeCriticalSection( &m_csHostEnum );
    m_hConnectCompleteEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hLobbyConnectionEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    // Setup the m_DPHostEnumHead circular linked list
    ZeroMemory( &m_DPHostEnumHead, sizeof( DPHostEnumInfo ) );
    m_DPHostEnumHead.pNext = &m_DPHostEnumHead;
}




//-----------------------------------------------------------------------------
// Name: ~CNetClientWizard
// Desc: Cleanup the class
//-----------------------------------------------------------------------------
CNetClientWizard::~CNetClientWizard()
{
    DeleteCriticalSection( &m_csHostEnum );
    CloseHandle( m_hConnectCompleteEvent );
    CloseHandle( m_hLobbyConnectionEvent );
}



//-----------------------------------------------------------------------------
// Name: Init
// Desc:
//-----------------------------------------------------------------------------
VOID CNetClientWizard::Init( IDirectPlay8Client* pDPClient,
                             IDirectPlay8LobbiedApplication* pLobbiedApp )
{
    m_pDPClient         = pDPClient;
    m_pLobbiedApp       = pLobbiedApp;
    m_bHaveConnectionSettingsFromLobby = FALSE;
    m_hLobbyClient      = NULL;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CNetClientWizard::DoConnectWizard()
{
    m_hrDialog = S_OK;

    // Display the multiplayer games dialog box.
    DialogBox( m_hInst, MAKEINTRESOURCE(IDD_CLIENT_CONNECT), NULL, 
               (DLGPROC) StaticSessionsDlgProc );

    return m_hrDialog;
}




//-----------------------------------------------------------------------------
// Name: StaticSessionsDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetClientWizard::StaticSessionsDlgProc( HWND hDlg, UINT uMsg,
                                                          WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->SessionsDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgProc()
// Desc: Handles messages for the multiplayer games dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetClientWizard::SessionsDlgProc( HWND hDlg, UINT msg,
                                                    WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg )
    {
        case WM_INITDIALOG:
            {
                // Load and set the icon
                HICON hIcon = LoadIcon( m_hInst, MAKEINTRESOURCE( IDI_MAIN ) );
                SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
                SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

                SetDlgItemText( hDlg, IDC_PLAYER_NAME_EDIT, m_strLocalPlayerName );

                // Set the window title
                TCHAR strWindowTitle[256];
                wsprintf( strWindowTitle, TEXT("%s - Multiplayer Games"), m_strAppName );
                SetWindowText( hDlg, strWindowTitle );

                // Init the search portion of the dialog
                m_bSearchingForSessions = FALSE;
                SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Start Search") );
                SessionsDlgInitListbox( hDlg );
            }
            break;

        case WM_TIMER:
            // Upon this timer message, then refresh the list of hosts
            // by expiring old hosts, and displaying the list in the
            // dialog box
            if( wParam == TIMERID_DISPLAY_HOSTS )
            {
                // Don't refresh if we are not enumerating hosts
                if( !m_bSearchingForSessions )
                    break;

                // Expire all of the hosts that haven't
                // refreshed in a certain period of time
                SessionsDlgExpireOldHostEnums();

                // Display the list of hosts in the dialog
                if( FAILED( hr = SessionsDlgDisplayEnumList( hDlg ) ) )
                {
                    DXTRACE_ERR( TEXT("SessionsDlgEnumHosts"), hr );
                    MessageBox( hDlg, TEXT("Error enumerating DirectPlay games."),
                                m_strAppName, MB_OK | MB_ICONERROR );

                    m_bSearchingForSessions = FALSE;
                    KillTimer( hDlg, TIMERID_DISPLAY_HOSTS );
                    CheckDlgButton( hDlg, IDC_SEARCH_CHECK, BST_UNCHECKED );
                    SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Start Search") );
                    SessionsDlgInitListbox( hDlg );
                }
            }
            else if( wParam == TIMERID_CONNECT_COMPLETE )
            {
                // Check to see if the MessageHandler has set an event to tell us the
                // DPN_MSGID_CONNECT_COMPLETE has been processed.  Now m_hrConnectComplete
                // is valid.
                if( WAIT_OBJECT_0 == WaitForSingleObject( m_hConnectCompleteEvent, 0 ) )
                {
                    m_bConnecting = FALSE;

                    if( FAILED( m_hrConnectComplete ) )
                    {
                        DXTRACE_ERR( TEXT("DPN_MSGID_CONNECT_COMPLETE"), m_hrConnectComplete );
                        MessageBox( hDlg, TEXT("Unable to join game."),
                                    m_strAppName, MB_OK | MB_ICONERROR );
                    }
                    else
                    {
                        // DirectPlay connect successful, so end dialog
                        m_hrDialog = NCW_S_FORWARD;
                        EndDialog( hDlg, 0 );
                    }
                }
            }

            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SEARCH_CHECK:
                    m_bSearchingForSessions = !m_bSearchingForSessions;

                    if( m_bSearchingForSessions )
                    {
                        SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Searching...") );

                        // Start the timer to display the host list every so often
                        SetTimer( hDlg, TIMERID_DISPLAY_HOSTS, DISPLAY_REFRESH_RATE, NULL );

                        // Start the async enumeration
                        if( FAILED( hr = SessionsDlgEnumHosts( hDlg ) ) )
                        {
                            if( hr == DPNERR_ADDRESSING )
                            {
                                // This will be returned if the ip address is invalid
                                // for example something like "asdf" 
                                MessageBox( hDlg, TEXT("IP address not valid. Stopping search"),
                                            m_strAppName, MB_OK );
                            }
                            else
                            {
                                DXTRACE_ERR( TEXT("SessionsDlgEnumHosts"), hr );
                                MessageBox( hDlg, TEXT("Error enumerating DirectPlay games."),
                                            m_strAppName, MB_OK | MB_ICONERROR );
                            }

                            m_bSearchingForSessions = FALSE;
                            KillTimer( hDlg, TIMERID_DISPLAY_HOSTS );
                            CheckDlgButton( hDlg, IDC_SEARCH_CHECK, BST_UNCHECKED );
                            SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Start Search") );
                            SessionsDlgInitListbox( hDlg );
                        }
                    }
                    else
                    {
                        SetDlgItemText( hDlg, IDC_SEARCH_CHECK, TEXT("Start Search") );

                        // Stop the timer, and stop the async enumeration
                        KillTimer( hDlg, TIMERID_DISPLAY_HOSTS );

                        // Until the CancelAsyncOperation returns, it is possible
                        // to still receive host enumerations
                        if( m_hEnumAsyncOp )
                            m_pDPClient->CancelAsyncOperation( m_hEnumAsyncOp, 0 );

                        // Reset the search portion of the dialog
                        SessionsDlgInitListbox( hDlg );
                    }
                    break;

                case IDC_GAMES_LIST:
                    if( HIWORD(wParam) != LBN_DBLCLK )
                        break;
                    // Fall through

                case IDC_JOIN:
                    if( FAILED( hr = SessionsDlgJoinGame( hDlg ) ) )
                    {
                        DXTRACE_ERR( TEXT("SessionsDlgJoinGame"), hr );
                        MessageBox( hDlg, TEXT("Unable to join game."),
                                    TEXT("DirectPlay Sample"),
                                    MB_OK | MB_ICONERROR );
                    }
                    break;

                case IDCANCEL: // The close button was press
                    m_hrDialog = NCW_S_QUIT;
                    EndDialog( hDlg, 0 );
                    break;

                default:
                    return FALSE; // Message not handled
            }
            break;

        case WM_DESTROY:
        {
            KillTimer( hDlg, 1 );

            // Cancel the enum hosts search
            // if the enumeration is going on
            if( m_bSearchingForSessions && m_hEnumAsyncOp )
            {
                m_pDPClient->CancelAsyncOperation( m_hEnumAsyncOp, 0 );
                m_bSearchingForSessions = FALSE;
            }
            break;
        }

        default:
            return FALSE; // Message not handled
    }

    // Message was handled
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgInitListbox()
// Desc: Initializes the listbox
//-----------------------------------------------------------------------------
VOID CNetClientWizard::SessionsDlgInitListbox( HWND hDlg )
{
    HWND hWndListBox = GetDlgItem( hDlg, IDC_GAMES_LIST );

    // Clear the contents from the list box, and
    // display "Looking for games" text in listbox
    SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );
    if( m_bSearchingForSessions )
    {
        SendMessage( hWndListBox, LB_ADDSTRING, 0,
                     (LPARAM) TEXT("Looking for games...") );
    }
    else
    {
        SendMessage( hWndListBox, LB_ADDSTRING, 0,
                     (LPARAM) TEXT("Click Start Search to see a list of games.  ")
                              TEXT("Click Create to start a new game.") );
    }

    SendMessage( hWndListBox, LB_SETITEMDATA,  0, NULL );
    SendMessage( hWndListBox, LB_SETCURSEL,    0, 0 );

    // Disable the join button until sessions are found
    EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), FALSE );

    // Query for the enum host timeout for this SP
    DPN_SP_CAPS dpspCaps;
    ZeroMemory( &dpspCaps, sizeof(DPN_SP_CAPS) );
    dpspCaps.dwSize = sizeof(DPN_SP_CAPS);
    m_pDPClient->GetSPCaps( &CLSID_DP8SP_TCPIP, &dpspCaps, 0 );

    // Set the host expire time to around 3 times
    // length of the dwDefaultEnumRetryInterval
    m_dwEnumHostExpireInterval = dpspCaps.dwDefaultEnumRetryInterval * 3;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgEnumHosts()
// Desc: Enumerates the DirectPlay sessions, and displays them in the listbox
//-----------------------------------------------------------------------------
HRESULT CNetClientWizard::SessionsDlgEnumHosts( HWND hDlg )
{
    HRESULT hr;

    m_bEnumListChanged = TRUE;

    DPN_APPLICATION_DESC   dpnAppDesc;
    IDirectPlay8Address*   pDP8AddressHost  = NULL;
    IDirectPlay8Address*   pDP8AddressLocal = NULL;
    WCHAR*                 wszHostName      = NULL;

    // Create the local device address object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, 
                                       CLSCTX_ALL, IID_IDirectPlay8Address,
                                       (LPVOID*) &pDP8AddressLocal ) ) )
    {
        DXTRACE_ERR( TEXT("CoCreateInstance"), hr );
        goto LCleanup;
    }

    // Set IP service provider
    if( FAILED( hr = pDP8AddressLocal->SetSP( &CLSID_DP8SP_TCPIP ) ) )
    {
        DXTRACE_ERR( TEXT("SetSP"), hr );
        goto LCleanup;
    }


    // Create the remote host address object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL, 
                                       CLSCTX_ALL, IID_IDirectPlay8Address,
                                       (LPVOID*) &pDP8AddressHost ) ) )
    {
        DXTRACE_ERR( TEXT("CoCreateInstance"), hr );
        goto LCleanup;
    }

    // Set IP service provider
    if( FAILED( hr = pDP8AddressHost->SetSP( &CLSID_DP8SP_TCPIP ) ) )
    {
        DXTRACE_ERR( TEXT("SetSP"), hr );
        goto LCleanup;
    }

    // Set the remote host name (if provided)
    TCHAR strIPAddress[MAX_PATH];
    GetDlgItemText( hDlg, IDC_IP_ADDRESS, strIPAddress, MAX_PATH );

    if( strIPAddress != NULL && strIPAddress[0] != 0 )
    {
        DWORD dwPort = 0;

        // Parse out port if it exists (expected form of "xxx.xxx.xxx.xxx:port")
        TCHAR* strPort = _tcschr( strIPAddress, TEXT(':') );
        if( NULL != strPort )
        {
            // Chop off :port from end of strIPAddress
            TCHAR* strEndOfIP = strPort;
            *strEndOfIP = 0;

            // Get port number from strPort
            strPort++;
            dwPort = _ttoi( strPort );
        }

        wszHostName = new WCHAR[_tcslen(strIPAddress)+1];
        DXUtil_ConvertGenericStringToWide( wszHostName, strIPAddress );

        hr = pDP8AddressHost->AddComponent( DPNA_KEY_HOSTNAME, wszHostName, 
                                            (wcslen(wszHostName)+1)*sizeof(WCHAR), 
                                            DPNA_DATATYPE_STRING );
        if( FAILED(hr) )
        {
            DXTRACE_ERR( TEXT("AddComponent"), hr );
            goto LCleanup;
        }

        // If a port was specified in the IP string, then add it.
        // Games will typically hard code the port so the user need not know it
        if( dwPort != 0 )
        {
            hr = pDP8AddressHost->AddComponent( DPNA_KEY_PORT, 
                                                &dwPort, sizeof(dwPort),
                                                DPNA_DATATYPE_DWORD );
            if( FAILED(hr) )
            {
                DXTRACE_ERR( TEXT("AddComponent"), hr );
                goto LCleanup;
            }
        }
    }

    ZeroMemory( &dpnAppDesc, sizeof( DPN_APPLICATION_DESC ) );
    dpnAppDesc.dwSize = sizeof( DPN_APPLICATION_DESC );
    dpnAppDesc.guidApplication = m_guidApp;

    // Enumerate all StressMazeApp hosts running on IP service providers
    hr = m_pDPClient->EnumHosts( &dpnAppDesc, pDP8AddressHost, 
                                 pDP8AddressLocal, NULL, 
                                 0, INFINITE, 0, INFINITE, NULL, 
                                 &m_hEnumAsyncOp, 0 );
    if( FAILED(hr) )
    {
        if( hr != DPNERR_INVALIDDEVICEADDRESS && 
            hr != DPNERR_ADDRESSING ) // This will be returned if the ip address is is invalid. 
            DXTRACE_ERR( TEXT("EnumHosts"), hr );
        goto LCleanup;
    }

LCleanup:
    SAFE_RELEASE( pDP8AddressHost);
    SAFE_RELEASE( pDP8AddressLocal );
    SAFE_DELETE( wszHostName );

    if( hr == DPNERR_PENDING )
        hr = DPN_OK;

    return hr;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgNoteEnumResponse()
// Desc: Stores them in the linked list, m_DPHostEnumHead.  This is
//       called from the DirectPlay message handler so it could be
//       called simultaneously from multiple threads.
//-----------------------------------------------------------------------------
HRESULT CNetClientWizard::SessionsDlgNoteEnumResponse( PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg )
{
    HRESULT hr = S_OK;
    BOOL    bFound;

    // This function is called from the DirectPlay message handler so it could be
    // called simultaneously from multiple threads, so enter a critical section
    // to assure that it we don't get race conditions.  Locking the entire
    // function is crude, and could be more optimal but is effective for this
    // simple sample
    EnterCriticalSection( &m_csHostEnum );

    DPHostEnumInfo* pDPHostEnum          = m_DPHostEnumHead.pNext;
    DPHostEnumInfo* pDPHostEnumNext      = NULL;
    const DPN_APPLICATION_DESC* pResponseMsgAppDesc =
                            pEnumHostsResponseMsg->pApplicationDescription;

    // Look for a matching session instance GUID.
    bFound = FALSE;
    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        if( pResponseMsgAppDesc->guidInstance == pDPHostEnum->pAppDesc->guidInstance )
        {
            bFound = TRUE;
            break;
        }

        pDPHostEnumNext = pDPHostEnum;
        pDPHostEnum = pDPHostEnum->pNext;
    }

    if( !bFound )
    {
        m_bEnumListChanged = TRUE;

        // If there's no match, then look for invalid session and use it
        pDPHostEnum = m_DPHostEnumHead.pNext;
        while ( pDPHostEnum != &m_DPHostEnumHead )
        {
            if( !pDPHostEnum->bValid )
                break;

            pDPHostEnum = pDPHostEnum->pNext;
        }

        // If no invalid sessions are found then make a new one
        if( pDPHostEnum == &m_DPHostEnumHead )
        {
            // Found a new session, so create a new node
            pDPHostEnum = new DPHostEnumInfo;
            if( NULL == pDPHostEnum )
            {
                hr = E_OUTOFMEMORY;
                goto LCleanup;
            }

            ZeroMemory( pDPHostEnum, sizeof(DPHostEnumInfo) );

            // Add pDPHostEnum to the circular linked list, m_DPHostEnumHead
            pDPHostEnum->pNext = m_DPHostEnumHead.pNext;
            m_DPHostEnumHead.pNext = pDPHostEnum;
        }
    }

    // Update the pDPHostEnum with new information
    TCHAR strName[MAX_PATH];
    if( pResponseMsgAppDesc->pwszSessionName )
    {
        DXUtil_ConvertWideStringToGeneric( strName, pResponseMsgAppDesc->pwszSessionName );
    }

    // Cleanup any old enum
    if( pDPHostEnum->pAppDesc )
    {
        SAFE_DELETE_ARRAY( pDPHostEnum->pAppDesc->pwszSessionName );
        SAFE_DELETE_ARRAY( pDPHostEnum->pAppDesc );
    }
    SAFE_RELEASE( pDPHostEnum->pHostAddr );
    SAFE_RELEASE( pDPHostEnum->pDeviceAddr );

    //
    // Duplicate pEnumHostsResponseMsg->pAddressSender in pDPHostEnum->pHostAddr.
    // Duplicate pEnumHostsResponseMsg->pAddressDevice in pDPHostEnum->pDeviceAddr.
    //
    if( FAILED( hr = pEnumHostsResponseMsg->pAddressSender->Duplicate( &pDPHostEnum->pHostAddr ) ) )
    {
        DXTRACE_ERR( TEXT("Duplicate"), hr );
        goto LCleanup;
    }

    if( FAILED( hr = pEnumHostsResponseMsg->pAddressDevice->Duplicate( &pDPHostEnum->pDeviceAddr ) ) )
    {
        DXTRACE_ERR( TEXT("Duplicate"), hr );
        goto LCleanup;
    }

    // Deep copy the DPN_APPLICATION_DESC from
    pDPHostEnum->pAppDesc = new DPN_APPLICATION_DESC;
    ZeroMemory( pDPHostEnum->pAppDesc, sizeof(DPN_APPLICATION_DESC) );
    memcpy( pDPHostEnum->pAppDesc, pResponseMsgAppDesc, sizeof(DPN_APPLICATION_DESC) );
    if( pResponseMsgAppDesc->pwszSessionName )
    {
        pDPHostEnum->pAppDesc->pwszSessionName = new WCHAR[ wcslen(pResponseMsgAppDesc->pwszSessionName)+1 ];
        wcscpy( pDPHostEnum->pAppDesc->pwszSessionName,
                pResponseMsgAppDesc->pwszSessionName );
    }

    // Update the time this was done, so that we can expire this host
    // if it doesn't refresh w/in a certain amount of time
    pDPHostEnum->dwLastPollTime = timeGetTime();

    // Check to see if the current number of players changed
    TCHAR szSessionTemp[MAX_PATH];
    if( pResponseMsgAppDesc->dwMaxPlayers > 0 )
    {
        wsprintf( szSessionTemp, TEXT("%s (%d/%d) (%dms)"), strName,
                  pResponseMsgAppDesc->dwCurrentPlayers - 1,  // ignore the host player
                  pResponseMsgAppDesc->dwMaxPlayers - 1,      // ignore the host player
                  pEnumHostsResponseMsg->dwRoundTripLatencyMS );
    }
    else
    {
        wsprintf( szSessionTemp, TEXT("%s (%d) (%dms)"), strName,
                  pResponseMsgAppDesc->dwCurrentPlayers - 1,  // ignore the host player
                  pEnumHostsResponseMsg->dwRoundTripLatencyMS );
    }

    // if this node was previously invalidated, or the session name is now
    // different the session list in the dialog needs to be updated
    if( ( pDPHostEnum->bValid == FALSE ) ||
        ( _tcscmp( pDPHostEnum->szSession, szSessionTemp ) != 0 ) )
    {
        m_bEnumListChanged = TRUE;
    }
    _tcscpy( pDPHostEnum->szSession, szSessionTemp );

    // This host is now valid
    pDPHostEnum->bValid = TRUE;

LCleanup:
    LeaveCriticalSection( &m_csHostEnum );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgExpireOldHostEnums
// Desc: Check all nodes to see if any have expired yet.
//-----------------------------------------------------------------------------
VOID CNetClientWizard::SessionsDlgExpireOldHostEnums()
{
    DWORD dwCurrentTime = timeGetTime();

    // This is called from the dialog UI thread, SessionsDlgNoteEnumResponse
    // is called from the DirectPlay message handler threads so
    // they may also be inside it at this time, so we need to go into the
    // critical section first
    EnterCriticalSection( &m_csHostEnum );

    DPHostEnumInfo* pDPHostEnum = m_DPHostEnumHead.pNext;
    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        // Check the poll time to expire stale entries.  Also check to see if
        // the entry is already invalid.  If so, don't note that the enum list
        // changed because that causes the list in the dialog to constantly redraw.
        if( ( pDPHostEnum->bValid != FALSE ) &&
            ( pDPHostEnum->dwLastPollTime < dwCurrentTime - m_dwEnumHostExpireInterval ) )
        {
            // This node has expired, so invalidate it.
            pDPHostEnum->bValid = FALSE;
            m_bEnumListChanged  = TRUE;
        }

        pDPHostEnum = pDPHostEnum->pNext;
    }

    LeaveCriticalSection( &m_csHostEnum );
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgDisplayEnumList
// Desc: Display the list of hosts in the dialog box
//-----------------------------------------------------------------------------
HRESULT CNetClientWizard::SessionsDlgDisplayEnumList( HWND hDlg )
{
    HWND           hWndListBox   = GetDlgItem( hDlg, IDC_GAMES_LIST );
    DPHostEnumInfo* pDPHostEnumSelected = NULL;
    GUID           guidSelectedInstance;
    BOOL           bFindSelectedGUID;
    BOOL           bFoundSelectedGUID;
    int            nItemSelected;

    // This is called from the dialog UI thread, SessionsDlgNoteEnumResponse
    // is called from the DirectPlay message handler threads so
    // they may also be inside it at this time, so we need to go into the
    // critical section first
    EnterCriticalSection( &m_csHostEnum );

    // Only update the display list if it has changed since last time
    if( !m_bEnumListChanged )
    {
        LeaveCriticalSection( &m_csHostEnum );
        return S_OK;
    }

    m_bEnumListChanged = FALSE;

    bFindSelectedGUID  = FALSE;
    bFoundSelectedGUID = FALSE;

    // Try to keep the same session selected unless it goes away or
    // there is no real session currently selected
    nItemSelected = (int)SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );
    if( nItemSelected != LB_ERR )
    {
        pDPHostEnumSelected = (DPHostEnumInfo*) SendMessage( hWndListBox, LB_GETITEMDATA,
                                                             nItemSelected, 0 );
        if( pDPHostEnumSelected != NULL && pDPHostEnumSelected->bValid )
        {
            guidSelectedInstance = pDPHostEnumSelected->pAppDesc->guidInstance;
            bFindSelectedGUID = TRUE;
        }
    }

    // Tell listbox not to redraw itself since the contents are going to change
    SendMessage( hWndListBox, WM_SETREDRAW, FALSE, 0 );

    // Test to see if any sessions exist in the linked list
    DPHostEnumInfo* pDPHostEnum = m_DPHostEnumHead.pNext;
    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        if( pDPHostEnum->bValid )
            break;
        pDPHostEnum = pDPHostEnum->pNext;
    }

    // If there are any sessions in list,
    // then add them to the listbox
    if( pDPHostEnum != &m_DPHostEnumHead )
    {
        // Clear the contents from the list box and enable the join button
        SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );

        // Enable the join button only if not already connecting to a game
        if( !m_bConnecting )        
            EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), TRUE );

        pDPHostEnum = m_DPHostEnumHead.pNext;
        while ( pDPHostEnum != &m_DPHostEnumHead )
        {
            // Add host to list box if it is valid
            if( pDPHostEnum->bValid )
            {
                int nIndex = (int)SendMessage( hWndListBox, LB_ADDSTRING, 0,
                                               (LPARAM)pDPHostEnum->szSession );
                SendMessage( hWndListBox, LB_SETITEMDATA, nIndex, (LPARAM)pDPHostEnum );

                if( bFindSelectedGUID )
                {
                    // Look for the session the was selected before
                    if( pDPHostEnum->pAppDesc->guidInstance == guidSelectedInstance )
                    {
                        SendMessage( hWndListBox, LB_SETCURSEL, nIndex, 0 );
                        bFoundSelectedGUID = TRUE;
                    }
                }
            }

            pDPHostEnum = pDPHostEnum->pNext;
        }

        if( !bFindSelectedGUID || !bFoundSelectedGUID )
            SendMessage( hWndListBox, LB_SETCURSEL, 0, 0 );
    }
    else
    {
        // There are no active session, so just reset the listbox
        SessionsDlgInitListbox( hDlg );
    }

    // Tell listbox to redraw itself now since the contents have changed
    SendMessage( hWndListBox, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect( hWndListBox, NULL, FALSE );

    LeaveCriticalSection( &m_csHostEnum );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgJoinGame()
// Desc: Joins the selected DirectPlay session
//-----------------------------------------------------------------------------
HRESULT CNetClientWizard::SessionsDlgJoinGame( HWND hDlg )
{
    HRESULT         hr;
    HWND            hWndListBox = GetDlgItem( hDlg, IDC_GAMES_LIST );
    DPHostEnumInfo* pDPHostEnumSelected = NULL;
    int             nItemSelected;

    // Add status text in list box
    nItemSelected = (int)SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );

    EnterCriticalSection( &m_csHostEnum );

    pDPHostEnumSelected = (DPHostEnumInfo*) SendMessage( hWndListBox, LB_GETITEMDATA,
                                                         nItemSelected, 0 );

    if( NULL == pDPHostEnumSelected )
    {
        LeaveCriticalSection( &m_csHostEnum );
        MessageBox( hDlg, TEXT("There are no games to join."),
                    TEXT("DirectPlay Sample"), MB_OK );
        return S_OK;
    }

    m_bConnecting = TRUE;

    // Set the peer info
    WCHAR wszPeerName[MAX_PATH];
    GetDlgItemText( hDlg, IDC_PLAYER_NAME_EDIT, m_strLocalPlayerName, MAX_PATH );
    DXUtil_ConvertGenericStringToWide( wszPeerName, m_strLocalPlayerName );

    DPN_PLAYER_INFO dpPlayerInfo;
    ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
    dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
    dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
    dpPlayerInfo.pwszName = wszPeerName;

    // Set the peer info, and use the DPNOP_SYNC since by default this
    // is an async call.  If it is not DPNOP_SYNC, then the peer info may not
    // be set by the time we call Connect() below.
    if( FAILED( hr = m_pDPClient->SetClientInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
        return DXTRACE_ERR( TEXT("SetPeerInfo"), hr );

    ResetEvent( m_hConnectCompleteEvent );

    // Connect to an existing session. DPNCONNECT_OKTOQUERYFORADDRESSING allows
    // DirectPlay to prompt the user using a dialog box for any device address
    // or host address information that is missing
    // We also pass in copies of the app desc and host addr, since pDPHostEnumSelected
    // might be deleted from another thread that calls SessionsDlgExpireOldHostEnums().
    // This process could also be done using reference counting instead.
    hr = m_pDPClient->Connect( pDPHostEnumSelected->pAppDesc,       // the application desc
                               pDPHostEnumSelected->pHostAddr,      // address of the host of the session
                               pDPHostEnumSelected->pDeviceAddr,    // address of the local device the enum responses were received on
                               NULL, NULL,                          // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                               NULL, 0,                             // user data, user data size
                               NULL, &m_hConnectAsyncOp,            // async context, async handle,
                               DPNCONNECT_OKTOQUERYFORADDRESSING ); // flags
    if( hr != E_PENDING && FAILED(hr) )
        return DXTRACE_ERR( TEXT("Connect"), hr );

    LeaveCriticalSection( &m_csHostEnum );

    // Set a timer to wait for m_hConnectCompleteEvent to be signaled.
    // This will tell us when DPN_MSGID_CONNECT_COMPLETE has been processed
    // which lets us know if the connect was successful or not.
    SetTimer( hDlg, TIMERID_CONNECT_COMPLETE, 100, NULL );

    // Disable the join button until connect succeeds or fails
    EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SessionsDlgEnumListCleanup()
// Desc: Deletes the linked list, g_DPHostEnumInfoHead
//-----------------------------------------------------------------------------
VOID CNetClientWizard::SessionsDlgEnumListCleanup()
{
    DPHostEnumInfo* pDPHostEnum = m_DPHostEnumHead.pNext;
    DPHostEnumInfo* pDPHostEnumDelete;

    while ( pDPHostEnum != &m_DPHostEnumHead )
    {
        pDPHostEnumDelete = pDPHostEnum;
        pDPHostEnum = pDPHostEnum->pNext;

        if( pDPHostEnumDelete->pAppDesc )
        {
            SAFE_DELETE_ARRAY( pDPHostEnumDelete->pAppDesc->pwszSessionName );
            SAFE_DELETE_ARRAY( pDPHostEnumDelete->pAppDesc );
        }

        // Changed from array delete to Release
        SAFE_RELEASE( pDPHostEnumDelete->pHostAddr );
        SAFE_RELEASE( pDPHostEnumDelete->pDeviceAddr );
        SAFE_DELETE( pDPHostEnumDelete );
    }

    // Re-link the g_DPHostEnumInfoHead circular linked list
    m_DPHostEnumHead.pNext = &m_DPHostEnumHead;
}




//-----------------------------------------------------------------------------
// Name: MessageHandler
// Desc: Handler for DirectPlay messages.  This function is called by
//       the DirectPlay message handler pool of threads, so be careful of thread
//       synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI CNetClientWizard::MessageHandler( PVOID pvUserContext,
                                                  DWORD dwMessageId,
                                                  PVOID pMsgBuffer )
{
    // Try not to stay in this message handler for too long, otherwise
    // there will be a backlog of data.  The best solution is to
    // queue data as it comes in, and then handle it on other threads.

    // This function is called by the DirectPlay message handler pool of
    // threads, so be careful of thread synchronization problems with shared memory

    switch(dwMessageId)
    {
        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg;
            pEnumHostsResponseMsg = (PDPNMSG_ENUM_HOSTS_RESPONSE)pMsgBuffer;

            // Take note of the host response
            SessionsDlgNoteEnumResponse( pEnumHostsResponseMsg );
            break;
        }

        case DPN_MSGID_ASYNC_OP_COMPLETE:
        {
            PDPNMSG_ASYNC_OP_COMPLETE pAsyncOpCompleteMsg;
            pAsyncOpCompleteMsg = (PDPNMSG_ASYNC_OP_COMPLETE)pMsgBuffer;

            if( pAsyncOpCompleteMsg->hAsyncOp == m_hEnumAsyncOp )
            {
                SessionsDlgEnumListCleanup();

                // The user canceled the DirectPlay connection dialog,
                // so stop the search
                if( m_bSearchingForSessions )
                {
                    CheckDlgButton( m_hDlg, IDC_SEARCH_CHECK, BST_UNCHECKED );
                    SendMessage( m_hDlg, WM_COMMAND, IDC_SEARCH_CHECK, 0 );
                }

                m_hEnumAsyncOp = NULL;
                m_bSearchingForSessions = FALSE;
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
            m_hrConnectComplete = pConnectCompleteMsg->hResultCode;
            SetEvent( m_hConnectCompleteEvent );
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConnectUsingLobbySettings
// Desc: Call this after the DPL_MSGID_CONNECT has been processed to carry out
//       the connection settings received by the lobby client.  DPL_MSGID_CONNECT
//       will have already been processed if we were lobby launched, or after
//       WaitForConnection returns without timing out.
//-----------------------------------------------------------------------------
HRESULT CNetClientWizard::ConnectUsingLobbySettings()
{
    HRESULT hr;
    DPNHANDLE hAsync;

    if( m_hLobbyClient == NULL )
        return E_INVALIDARG;

    DPL_CONNECTION_SETTINGS* pSettings = NULL;
    DWORD dwSettingsSize = 0;

    // Get the connection settings from the lobby.
    hr = m_pLobbiedApp->GetConnectionSettings( m_hLobbyClient, pSettings, &dwSettingsSize, 0 );
    if( hr != DPNERR_BUFFERTOOSMALL )
        return DXTRACE_ERR( TEXT("GetConnectionSettings"), hr );
    pSettings = (DPL_CONNECTION_SETTINGS*) new BYTE[dwSettingsSize];
    if( FAILED( hr = m_pLobbiedApp->GetConnectionSettings( m_hLobbyClient, pSettings, &dwSettingsSize, 0 ) ) )
        return DXTRACE_ERR( TEXT("GetConnectionSettings"), hr );

    // Set the peer info
    WCHAR wszPeerName[MAX_PATH];
    DXUtil_ConvertGenericStringToWide( wszPeerName, m_strLocalPlayerName );
    DPN_PLAYER_INFO dpPlayerInfo;
    ZeroMemory( &dpPlayerInfo, sizeof(DPN_PLAYER_INFO) );
    dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
    dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
    dpPlayerInfo.pwszName = wszPeerName;

    // Set the peer info, and use the DPNOP_SYNC since by default this
    // is an async call.  If it is not DPNOP_SYNC, then the peer info may not
    // be set by the time we call Connect() below.
    if( FAILED( hr = m_pDPClient->SetClientInfo( &dpPlayerInfo, NULL, NULL, DPNOP_SYNC ) ) )
        return DXTRACE_ERR( TEXT("SetClientInfo"), hr );

    // Connect to an existing session. There should only be on device address in
    // the connection settings structure when connecting to a session, so just
    // pass in the first one.
    // The enumeration is automatically cancelled after Connect is called 
    hr = m_pDPClient->Connect( &pSettings->dpnAppDesc,              // the application desc
                               pSettings->pdp8HostAddress,          // address of the host of the session
                               pSettings->ppdp8DeviceAddresses[0],  // address of the local device used to connect to the host
                               NULL, NULL,                          // DPN_SECURITY_DESC, DPN_SECURITY_CREDENTIALS
                               NULL, 0,                             // user data, user data size
                               NULL, &hAsync,                       // async context, async handle,
                               0 );                                 // flags
    if( hr != E_PENDING && FAILED(hr) )
        return DXTRACE_ERR( TEXT("Connect"), hr );
    hr = S_OK; // Accept E_PENDING.

    // Wait until the MessageHandler sets an event to tell us the
    // DPN_MSGID_CONNECT_COMPLETE has been processed.  Then m_hrConnectComplete
    // will be valid.
    WaitForSingleObject( m_hConnectCompleteEvent, INFINITE );

    if( FAILED( m_hrConnectComplete ) )
    {
        DXTRACE_ERR( TEXT("DPN_MSGID_CONNECT_COMPLETE"), m_hrConnectComplete );
        MessageBox( m_hDlg, TEXT("Unable to join game."),
                    TEXT("DirectPlay Sample"),
                    MB_OK | MB_ICONERROR );
        hr = m_hrConnectComplete;
    }

    // Cleanup the addresses and memory obtained from GetConnectionSettings
    SAFE_RELEASE( pSettings->pdp8HostAddress );
    for( DWORD dwIndex=0; dwIndex < pSettings->cNumDeviceAddresses; dwIndex++ )
    {
        SAFE_RELEASE( pSettings->ppdp8DeviceAddresses[dwIndex] );
    }

    SAFE_DELETE_ARRAY( pSettings );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: LobbyMessageHandler
// Desc: Handler for DirectPlay messages.  This function is called by
//       the DirectPlay lobby message handler pool of threads, so be careful of thread
//       synchronization problems with shared memory
//-----------------------------------------------------------------------------
HRESULT WINAPI CNetClientWizard::LobbyMessageHandler( PVOID pvUserContext,
                                                       DWORD dwMessageId,
                                                       PVOID pMsgBuffer )
{
    HRESULT hr = S_OK;

    switch(dwMessageId)
    {
        case DPL_MSGID_CONNECT:
        {
            // This message will be processed when a lobby connection has been
            // established. If you were lobby launched then
            // IDirectPlay8LobbiedApplication::Initialize()
            // waits until this message has been processed before returning, so
            // take care not to deadlock by making calls that need to be handled by
            // the thread who called Initialize().  The same is true for WaitForConnection()

            PDPL_MESSAGE_CONNECT pConnectMsg;
            pConnectMsg = (PDPL_MESSAGE_CONNECT)pMsgBuffer;
            PDPL_CONNECTION_SETTINGS pSettings = pConnectMsg->pdplConnectionSettings;

            m_hLobbyClient = pConnectMsg->hConnectId;

            if( FAILED( hr = m_pDPClient->RegisterLobby( m_hLobbyClient, m_pLobbiedApp,
                                                   DPNLOBBY_REGISTER ) ) )
                return DXTRACE_ERR( TEXT("RegisterLobby"), hr );

            if( pSettings == NULL )
            {
                // There aren't connection settings from the lobby
                m_bHaveConnectionSettingsFromLobby = FALSE;
            }
            else
            {
                // Record the player name if found
                if( pSettings->pwszPlayerName != NULL )
                {
                    TCHAR strPlayerName[MAX_PATH];
                    DXUtil_ConvertWideStringToGeneric( strPlayerName, pSettings->pwszPlayerName );
                    _tcscpy( m_strLocalPlayerName, strPlayerName );
                }
                else
                {
                    _tcscpy( m_strLocalPlayerName, TEXT("Unknown player name") );
                }

                m_bHaveConnectionSettingsFromLobby = TRUE;
            }

            // Tell everyone we have a lobby connection now
            SetEvent( m_hLobbyConnectionEvent );
            break;
        }
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: StaticLobbyWaitDlgProc()
// Desc: Static msg handler which passes messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetClientWizard::StaticLobbyWaitDlgProc( HWND hDlg, UINT uMsg,
                                                                WPARAM wParam, LPARAM lParam )
{
    if( g_pNCW )
        return g_pNCW->LobbyWaitDlgProc( hDlg, uMsg, wParam, lParam );

    return FALSE; // Message not handled
}




//-----------------------------------------------------------------------------
// Name: LobbyWaitDlgProc()
// Desc: Handles messages for the lobby wait status dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CNetClientWizard::LobbyWaitDlgProc( HWND hDlg, UINT msg,
                                                      WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_INITDIALOG:
            // Set a timer to wait for m_hConnectCompleteEvent to be signaled.
            // This will tell us when DPN_MSGID_CONNECT_COMPLETE has been processed
            // which lets us know if the connect was successful or not.
            SetTimer( hDlg, TIMERID_CONNECT_COMPLETE, 100, NULL );

            SetDlgItemText( hDlg, IDC_WAIT_TEXT, TEXT("Waiting for lobby connection...") );
            return TRUE;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    return TRUE;
            }
            break;

        case WM_TIMER:
        {
            if( wParam == TIMERID_CONNECT_COMPLETE )
            {
                // Wait for a lobby connection.  If this call
                // returns WAIT_OBJECT_0 then the DPL_MSGID_CONNECT will
                // have already been processed.
                DWORD dwResult = WaitForSingleObject( m_hLobbyConnectionEvent, 100 );
                if( dwResult != WAIT_TIMEOUT )
                    EndDialog( hDlg, IDOK );
            }
            break;
        }
    }

    return FALSE; // Didn't handle message
}

