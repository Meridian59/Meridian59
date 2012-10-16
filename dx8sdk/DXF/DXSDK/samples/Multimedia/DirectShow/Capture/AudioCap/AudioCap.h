//------------------------------------------------------------------------------
// File: AudioCapDlg.cpp
//
// Desc: DirectShow sample code - main header file for the 
//       audio capture sample application
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_AUDIOCAP_H__EC6C2C49_154B_480D_B494_B3DF7F1748F0__INCLUDED_)
#define AFX_AUDIOCAP_H__EC6C2C49_154B_480D_B494_B3DF7F1748F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAudioCapApp:
// See AudioCap.cpp for the implementation of this class
//

class CAudioCapApp : public CWinApp
{
public:
	CAudioCapApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioCapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAudioCapApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOCAP_H__EC6C2C49_154B_480D_B494_B3DF7F1748F0__INCLUDED_)
