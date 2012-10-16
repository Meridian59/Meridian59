//------------------------------------------------------------------------------
// File: PlayDMO.cpp
//
// Desc: DirectShow sample code - main header for PlayDMO application
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "PlayDMO.h"
#include "PlayDMODlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayDMOApp

BEGIN_MESSAGE_MAP(CPlayDMOApp, CWinApp)
	//{{AFX_MSG_MAP(CPlayDMOApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayDMOApp construction

CPlayDMOApp::CPlayDMOApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPlayDMOApp object

CPlayDMOApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPlayDMOApp initialization

BOOL CPlayDMOApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	// In MFC 5.0, Enable3dControls and Enable3dControlsStatic are obsolete because
	// their functionality is incorporated into Microsoft's 32-bit operating systems.
#if (_MSC_VER <= 1200)
	Enable3dControls();			// Call this when using MFC in a shared DLL
#endif
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CPlayDMODlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = (int) dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
