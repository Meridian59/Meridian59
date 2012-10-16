//-----------------------------------------------------------------------------
// File: NetConnect.h
//
// Desc:
//
// Copyright (C) 2000-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef NETCLIENT_H
#define NETCLIENT_H


#include <windows.h>
#include <dplay8.h>
#include <dpaddr.h>
#include <tchar.h>


//-----------------------------------------------------------------------------
// Defines, structures, and error codes
//-----------------------------------------------------------------------------
#define DISPLAY_REFRESH_RATE        250
#define TIMERID_DISPLAY_HOSTS       1
#define TIMERID_CONNECT_COMPLETE    2


#define NCW_S_FORWARD      0x01000001  // Dialog success, so go forward
#define NCW_S_BACKUP       0x01000002  // Dialog canceled, show previous dialog
#define NCW_S_QUIT         0x01000003  // Dialog quit, close app
#define NCW_S_LOBBYCONNECT 0x01000004  // Dialog connected from lobby, connect success


class CNetClientWizard
{
public:
    CNetClientWizard( HINSTANCE hInst, TCHAR* strAppName, GUID* pGuidApp );
    virtual ~CNetClientWizard();

    HRESULT WINAPI MessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );
    HRESULT WINAPI LobbyMessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );

    VOID    Init( IDirectPlay8Client* pDPClient, IDirectPlay8LobbiedApplication* pLobbiedApp );
    HRESULT DoConnectWizard();
    HRESULT ConnectUsingLobbySettings();

    void    SetPlayerName( TCHAR* strPlayerName )  { _tcscpy( m_strLocalPlayerName, strPlayerName ); }
    TCHAR*  GetPlayerName()                        { return m_strLocalPlayerName; }
    BOOL    HaveConnectionSettingsFromLobby()      { return m_bHaveConnectionSettingsFromLobby; }

protected:
    struct DPHostEnumInfo
    {
        DWORD                 dwRef;
        DPN_APPLICATION_DESC* pAppDesc;
        IDirectPlay8Address* pHostAddr;
        IDirectPlay8Address* pDeviceAddr;
        TCHAR                szSession[MAX_PATH];
        DWORD                dwLastPollTime;
        BOOL                 bValid;
        DPHostEnumInfo*      pNext;
    };

    static INT_PTR CALLBACK StaticSessionsDlgProc( HWND, UINT, WPARAM, LPARAM );
    static INT_PTR CALLBACK StaticLobbyWaitDlgProc( HWND, UINT, WPARAM, LPARAM );
    INT_PTR CALLBACK SessionsDlgProc( HWND, UINT, WPARAM, LPARAM );
    INT_PTR CALLBACK LobbyWaitDlgProc( HWND, UINT, WPARAM, LPARAM );

    VOID    SessionsDlgInitListbox( HWND hDlg );
    HRESULT SessionsDlgEnumHosts( HWND hDlg );
    HRESULT SessionsDlgNoteEnumResponse( PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponse );
    VOID    SessionsDlgExpireOldHostEnums();
    HRESULT SessionsDlgDisplayEnumList( HWND hDlg );
    HRESULT SessionsDlgJoinGame( HWND hDlg );
    HRESULT SessionsDlgCreateGame( HWND hDlg );
    VOID    SessionsDlgEnumListCleanup();

    IDirectPlay8Client*     m_pDPClient;
    IDirectPlay8LobbiedApplication* m_pLobbiedApp;
    CRITICAL_SECTION        m_csHostEnum;
    GUID                    m_guidApp;
    HRESULT                 m_hrDialog;
    HWND                    m_hDlg;
    HINSTANCE               m_hInst;
    DPHostEnumInfo          m_DPHostEnumHead;
    TCHAR                   m_strAppName[MAX_PATH];
    TCHAR                   m_strLocalPlayerName[MAX_PATH];
    BOOL                    m_bSearchingForSessions;
    BOOL                    m_bEnumListChanged;
    DPNHANDLE               m_hEnumAsyncOp;
    DWORD                   m_dwEnumHostExpireInterval;
    BOOL                    m_bConnecting;
    DPNHANDLE               m_hConnectAsyncOp;
    HANDLE                  m_hConnectCompleteEvent;
    HANDLE                  m_hLobbyConnectionEvent;
    HRESULT                 m_hrConnectComplete;
    BOOL                    m_bHaveConnectionSettingsFromLobby;
    DPNHANDLE               m_hLobbyClient;
};


#endif // NETCLIENT_H


