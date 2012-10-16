//------------------------------------------------------------------------------
// File: Mapper.h
//
// Desc: DirectShow sample code - an MFC based C++ filter mapper application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
// Mapper.h : main header file for the MAPPER application
//

#if !defined(AFX_MAPPER_H__8F990BFF_27ED_4CE2_AF97_7C2E416356FB__INCLUDED_)
#define AFX_MAPPER_H__8F990BFF_27ED_4CE2_AF97_7C2E416356FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMapperApp:
// See Mapper.cpp for the implementation of this class
//

class CMapperApp : public CWinApp
{
public:
	CMapperApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapperApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMapperApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPPER_H__8F990BFF_27ED_4CE2_AF97_7C2E416356FB__INCLUDED_)
