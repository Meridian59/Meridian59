//-----------------------------------------------------------------------------
// File: DXInstall.h
//
// Desc: Header file for example code showing how to use DirectXSetup.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
VOID    SetButtons( HWND hDlg, DWORD wMsgType );
DLGPROC DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL    DirectXInstall( HWND hWnd );
VOID    DirectXGetVersion();




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define DSETUP_REINSTALL	0x00000080

#define SHOW_ALL		1
#define SHOW_UPGRADES	2
#define SHOW_PROBLEMS	3
#define SHOW_NONE		4
