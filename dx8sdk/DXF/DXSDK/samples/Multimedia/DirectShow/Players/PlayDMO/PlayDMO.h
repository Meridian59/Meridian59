//------------------------------------------------------------------------------
// File: PlayDMO.h
//
// Desc: DirectShow sample code - main header file for PlayDMO
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_PLAYDMO_H__123E1806_4ECC_487E_884B_EBF648ADB914__INCLUDED_)
#define AFX_PLAYDMO_H__123E1806_4ECC_487E_884B_EBF648ADB914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPlayDMOApp:
// See PlayDMO.cpp for the implementation of this class
//

class CPlayDMOApp : public CWinApp
{
public:
	CPlayDMOApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayDMOApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPlayDMOApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYDMO_H__123E1806_4ECC_487E_884B_EBF648ADB914__INCLUDED_)
