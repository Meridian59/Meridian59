//------------------------------------------------------------------------------
// File: DMOEnum.h
//
// Desc: DirectShow sample code - main header file for the DMOEnum application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#if !defined(AFX_DMOENUM_H__19FC7D3C_E963_41C9_AA3F_1330728D4F41__INCLUDED_)
#define AFX_DMOENUM_H__19FC7D3C_E963_41C9_AA3F_1330728D4F41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDMOEnumApp:
// See DMOEnum.cpp for the implementation of this class
//

class CDMOEnumApp : public CWinApp
{
public:
	CDMOEnumApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDMOEnumApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDMOEnumApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMOENUM_H__19FC7D3C_E963_41C9_AA3F_1330728D4F41__INCLUDED_)
