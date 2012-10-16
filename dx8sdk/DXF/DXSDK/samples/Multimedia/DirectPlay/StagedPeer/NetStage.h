//-----------------------------------------------------------------------------
// File: NetStage.h
//
// Desc: see stagedpeer.cpp
//
// Copyright (C) 2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef NETSTAGE_H
#define NETSTAGE_H


#include <windows.h>
#include <dplay8.h>
#include "stagedpeer.h"


//-----------------------------------------------------------------------------
// Defines, structures, and error codes
//-----------------------------------------------------------------------------
#define WM_APP_DISPLAY_STAGE    (WM_APP + 0)
#define WM_APP_CHAT             (WM_APP + 1)

#define NS_S_FORWARD      0x01000001  // Dialog success, so go forward
#define NS_S_BACKUP       0x01000002  // Dialog canceled, show previous dialog
#define NS_S_QUIT         0x01000003  // Dialog quit, close app
#define NS_S_LOBBYCONNECT 0x01000004  // Dialog connected from lobby, connect success
#define NS_S_REJECTED     0x01000005  // The player was rejected from the game, so 
                                      // the previous connection dialog should be shown
#define NS_S_GAMECANCELED 0x01000006  // The host canceled the game, so quit.

#define MAX_SLOTS            10
#define MAX_CHAT_STRINGS     50
#define MAX_CHAT_STRING_LENGTH  508

#define SLOT_BUTTON_MASK     0x0F
#define SLOT_BUTTON_OPEN     0x00
#define SLOT_BUTTON_CLOSED   0x01
#define SLOT_BUTTON_FULL     0x02

#define SLOT_READY_MASK      0xF0
#define SLOT_READY_CHECKED   0x10



//-----------------------------------------------------------------------------
// App specific DirectPlay messages and structures 
//-----------------------------------------------------------------------------
#define STAGE_MSGID_SETSLOTID     0x12001
#define STAGE_MSGID_SETSTAGEDATA  0x12002
#define STAGE_MSGID_SLOTCHECK     0x12004
#define STAGE_MSGID_SLOTUNCHECK   0x12005
#define STAGE_MSGID_STARTGAME     0x12006
#define STAGE_MSGID_CANCELGAME    0x12007
#define STAGE_MSGID_CHAT          0x12008


class CNetStage
{
public:
	CNetStage( HINSTANCE hInst, TCHAR* strAppName );
	virtual ~CNetStage();

    HRESULT Init( IDirectPlay8Peer* pDP );
    HRESULT DoStage( TCHAR* strLocalPlayerName, BOOL bHostPlayer );
    HRESULT WINAPI MessageHandler( PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer );

protected:
    struct SLOT_INFO
    {
        DWORD dwSlotStatus;                // Status of this slot (SLOT_BUTTON_FULL, etc)
        TCHAR strSlotName[MAX_PLAYER_NAME]; // String name associated with this slot
        DWORD dpnidPlayer;                 // dpnid of the player in the slot
    };

    struct STAGEMSG_GENERIC
    {
        DWORD dwMessageID;
    };

    struct STAGEMSG_SLOT : public STAGEMSG_GENERIC
    {
        LONG lSlotNumber;
    };

    struct STAGEMSG_CHAT : public STAGEMSG_GENERIC
    {
        TCHAR strChatString[MAX_CHAT_STRING_LENGTH];
        DPNHANDLE hBufferHandle;
    };

    struct STAGEMSG_STAGEDATA : public STAGEMSG_GENERIC
    {
        DPNHANDLE hBufferHandle;
        SLOT_INFO Slots[MAX_SLOTS]; 
    };

    static INT_PTR CALLBACK StaticStageDlgProc( HWND, UINT, WPARAM, LPARAM );
    INT_PTR CALLBACK StageDlgProc( HWND, UINT, WPARAM, LPARAM );

    HRESULT StageDlgInit( HWND hDlg );
    HRESULT HandleAppMessages( HWND hDlg, STAGEMSG_GENERIC* pMsg, PDPNMSG_RECEIVE pReceiveMsg );
    HRESULT SendChatMessage( HWND hDlg );
    HRESULT GetFreePlayerSlot( DWORD* pdwPlayerSlotID );
    HRESULT SendSlotDataToPlayers( HWND hDlg );
    VOID    DisplaySlotData( HWND hDlg, STAGEMSG_STAGEDATA* pStageDataMsg );
    HRESULT AddPlayerToStage( HWND hDlg, APP_PLAYER_INFO* pPlayerInfo );
    HRESULT RemovePlayerToStage( HWND hDlg, APP_PLAYER_INFO* pPlayerInfo );
    VOID    AddChatStringToListBox( HWND hDlg, LPSTR strMsgText );
    HRESULT ChangePlayerReadyStatus( HWND hDlg, BOOL bPlayerReady );
    HRESULT ChangePlayerSlotStatus( HWND hDlg, DWORD dwSlot );
    HRESULT UpdateSessionDesc();
    HRESULT StartGame( HWND hDlg );
    HRESULT CancelGame();
    VOID    UpdateStageWhenInitComplete();

    IDirectPlay8Peer*   m_pDP;
    HINSTANCE           m_hInst;
    HRESULT             m_hrDialog;
    HWND                m_hDlg;
    BOOL                m_bActive;
    BOOL                m_bHostPlayer;
    BOOL                m_bStageInitialized;
    BOOL                m_bDialogInitialized;
    LONG                m_lLocalSlotNumber;
    BOOL                m_bLocalPlayerReady;
    BOOL                m_bAllowPlayerJoin;
    DWORD               m_dwNumberOfFullSlots;
    TCHAR               m_strAppName[MAX_PATH];
    TCHAR               m_strLocalPlayerName[MAX_PLAYER_NAME];
    STAGEMSG_STAGEDATA* m_pStageDataMsg;
    CRITICAL_SECTION    m_csSlots;
    SLOT_INFO           m_Slots[MAX_SLOTS]; 
    DWORD               m_dwNumberSlotsOpen;
    DPNID               m_dpnidHostPlayer;
};


#endif // NETSTAGE_H

