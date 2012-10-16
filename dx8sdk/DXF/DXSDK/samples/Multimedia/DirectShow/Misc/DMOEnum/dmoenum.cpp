//------------------------------------------------------------------------------
// File: DMOEnum.cpp
//
// Desc: DirectShow sample code - an MFC based application for device 
//       enumeration.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include "DMOEnum.h"
#include "DMOEnumDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDMOEnumApp

BEGIN_MESSAGE_MAP(CDMOEnumApp, CWinApp)
	//{{AFX_MSG_MAP(CDMOEnumApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDMOEnumApp construction

CDMOEnumApp::CDMOEnumApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDMOEnumApp object

CDMOEnumApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDMOEnumApp initialization

BOOL CDMOEnumApp::InitInstance()
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

	CDMOEnumDlg dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
