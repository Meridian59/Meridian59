// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelMaker.h : main header file for the GELMAKER application
//
//////////
//
// Copyright (C) The 3DO Company
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of The 3DO Company.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////

#include "PaletteApp.h"

#define GM_EYEDROPPER (WM_APP + 0x100) // WPARAM is key flags, LPARAM is pixel index

/////////////////////////////////////////////////////////////////////////////

class CGelMakerApp : public CPaletteApp
{
public:
	CGelMakerApp();

// Overrides
public:
	//{{AFX_VIRTUAL(CGelMakerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
public:
	//{{AFX_MSG(CGelMakerApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
