//-----------------------------------------------------------------------------
// File: Wizard.h
//
// Desc: UI routines
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#define IDIRECTPLAY2_OR_GREATER
#include <commctrl.h>
#include <dplay.h>


// Structure for the tree control
struct TREEDATA
{
    GUID  guid;      // Session GUID
    DWORD dwRefresh; // Used to detect when sessions go away
};


BOOL    SetupFonts();
VOID    CleanupFonts();
LONG    RegSet( const TCHAR* strName, const BYTE* pData, DWORD dwSize );
LONG    RegGet( const TCHAR* strName, BYTE* pData, DWORD* pdwDataSize );

DWORD WINAPI    DoWizard( VOID* pv );
BOOL CALLBACK   DlgProcChooseProvider( HWND hDlg, UINT msg, WPARAM wParam,
                                       LPARAM lParam);
BOOL FAR PASCAL DPEnumConnectionsCallback( const GUID* pguidSP,
                                           VOID* pConnection, DWORD dwSize, 
                                           const DPNAME* pName, DWORD dwFlags,
                                           VOID* pContext );
BOOL WINAPI     EnumSession( const DPSESSIONDESC2* pDPSessionDesc,
                             DWORD* pdwTimeOut, DWORD dwFlags,  VOID* pContext );
BOOL CALLBACK   DlgProcGameSetup( HWND hDlg, UINT msg, WPARAM wParam,
                                  LPARAM lParam );
BOOL WINAPI     EnumPlayer( DPID pidID, DWORD dwPlayerType,
                            const DPNAME* pName, DWORD dwFlags,
                            VOID* pContext );
BOOL CALLBACK   DlgProcJoinSession( HWND hDlg, UINT msg, WPARAM wParam,
                                    LPARAM lParam );
BOOL CALLBACK   DlgProcHostSession( HWND hDlg, UINT msg, WPARAM wParam,
                                    LPARAM lParam );

BOOL      InitTreeViewImageLists( HWND hwndTV );
HTREEITEM AddItemToTree( HWND hwndTV, TCHAR* strItem, DWORD dwData, int nLevel );
VOID      ReleaseSessionData( HWND hWndCtl );
VOID      ReleaseSPData();



