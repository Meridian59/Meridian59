//------------------------------------------------------------------------------
// File: Jukebox.h
//
// Desc: DirectShow sample code - main header file for the Jukebox
//       application.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#if !defined(AFX_JUKEBOX_H__A0CFADEB_2EC4_463F_947A_D0552C0D1CC1__INCLUDED_)
#define AFX_JUKEBOX_H__A0CFADEB_2EC4_463F_947A_D0552C0D1CC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJukeboxApp:
// See Jukebox.cpp for the implementation of this class
//

class CJukeboxApp : public CWinApp
{
public:
	CJukeboxApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJukeboxApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJukeboxApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CJukeboxApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JUKEBOX_H__A0CFADEB_2EC4_463F_947A_D0552C0D1CC1__INCLUDED_)
