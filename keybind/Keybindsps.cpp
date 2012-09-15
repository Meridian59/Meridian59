// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Keybindsps.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Keybindsps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeybindsps

IMPLEMENT_DYNAMIC(CKeybindsps, CPropertySheet)

CKeybindsps::CKeybindsps(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CKeybindsps::CKeybindsps(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CKeybindsps::~CKeybindsps()
{
}


BEGIN_MESSAGE_MAP(CKeybindsps, CPropertySheet)
	//{{AFX_MSG_MAP(CKeybindsps)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeybindsps message handlers

BOOL CKeybindsps::PreTranslateMessage(MSG* pMsg) 
{
/*	
	// since CPropertySheet::PreTranslateMessage will eat frame window
	// accelerators, we call all frame windows' PreTranslateMessage first.
	CFrameWnd* pFrameWnd = GetParentFrame();   // start with first parent frame
	while (pFrameWnd != NULL)
	{
		// allow owner & frames to translate before IsDialogMessage does
		if (pFrameWnd->PreTranslateMessage(pMsg))
			return TRUE;

		// try parent frames until there are no parent frames
		pFrameWnd = pFrameWnd->GetParentFrame();
	}
*/
	// otherwise, just let the sheet translate the message
	return CPropertySheet::PreTranslateMessage(pMsg);
}
