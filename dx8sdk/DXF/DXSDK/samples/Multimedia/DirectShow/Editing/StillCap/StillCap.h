//------------------------------------------------------------------------------
// File: StillCap.h
//
// Desc: DirectShow sample code - header file for Still Cap application.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#if !defined(AFX_STILLCAP_H__11EB4C7D_3284_45E8_A222_3E5EA81FC840__INCLUDED_)
#define AFX_STILLCAP_H__11EB4C7D_3284_45E8_A222_3E5EA81FC840__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CStillCapApp:
// See StillCap.cpp for the implementation of this class
//

class CStillCapApp : public CWinApp
{
public:
	CStillCapApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStillCapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CStillCapApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STILLCAP_H__11EB4C7D_3284_45E8_A222_3E5EA81FC840__INCLUDED_)
