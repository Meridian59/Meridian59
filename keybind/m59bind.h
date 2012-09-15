// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// m59bind.h : main header file for the M59BIND application
//

#if !defined(AFX_M59BIND_H__0FD9EA3D_FDFC_4ACB_831B_821737DC7907__INCLUDED_)
#define AFX_M59BIND_H__0FD9EA3D_FDFC_4ACB_831B_821737DC7907__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CM59bindApp:
// See m59bind.cpp for the implementation of this class
//

class CM59bindApp : public CWinApp
{
public:
	CM59bindApp();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CM59bindApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CM59bindApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#define MAX_KEYVALUELEN 128

#define MODIFIER_NONE 0
#define MODIFIER_ALT 1
#define MODIFIER_CTRL 2
#define MODIFIER_SHIFT 3
#define MODIFIER_ANY 4

extern CString strINIFile;
extern BOOL bUpdateINI;

extern TCHAR TCNewkey[MAX_KEYVALUELEN];

extern int iModifier;

extern BOOL bInvert;
extern int iMouselookXscale;
extern int iMouselookYscale;

extern TCHAR TCBroadcast[MAX_KEYVALUELEN];
extern TCHAR TCChat[MAX_KEYVALUELEN];
extern TCHAR TCEmote[MAX_KEYVALUELEN];
extern TCHAR TCSay[MAX_KEYVALUELEN];
extern TCHAR TCTell[MAX_KEYVALUELEN];
extern TCHAR TCWho[MAX_KEYVALUELEN];
extern TCHAR TCYell[MAX_KEYVALUELEN];

extern TCHAR TCAttack[MAX_KEYVALUELEN];
extern TCHAR TCBuy[MAX_KEYVALUELEN];
extern TCHAR TCDeposit[MAX_KEYVALUELEN];
extern TCHAR TCExamine[MAX_KEYVALUELEN];
extern TCHAR TCLook[MAX_KEYVALUELEN];
extern TCHAR TCOffer[MAX_KEYVALUELEN];
extern TCHAR TCOpen[MAX_KEYVALUELEN];
extern TCHAR TCPickup[MAX_KEYVALUELEN];
extern TCHAR TCWithdraw[MAX_KEYVALUELEN];

extern TCHAR TCBackward[MAX_KEYVALUELEN];
extern TCHAR TCFlip[MAX_KEYVALUELEN];
extern TCHAR TCForward[MAX_KEYVALUELEN];
extern TCHAR TCLeft[MAX_KEYVALUELEN];
extern TCHAR TCLookdown[MAX_KEYVALUELEN];
extern TCHAR TCLookstraight[MAX_KEYVALUELEN];
extern TCHAR TCLookup[MAX_KEYVALUELEN];
extern TCHAR TCMouselooktoggle[MAX_KEYVALUELEN];
extern TCHAR TCRight[MAX_KEYVALUELEN];
extern TCHAR TCRunwalk[MAX_KEYVALUELEN];
extern TCHAR TCSlideleft[MAX_KEYVALUELEN];
extern TCHAR TCSlideright[MAX_KEYVALUELEN];

extern TCHAR TCTabbackward[MAX_KEYVALUELEN];
extern TCHAR TCTabforward[MAX_KEYVALUELEN];
extern TCHAR TCTargetclear[MAX_KEYVALUELEN];
extern TCHAR TCTargetnext[MAX_KEYVALUELEN];
extern TCHAR TCTargetprevious[MAX_KEYVALUELEN];
extern TCHAR TCTargetself[MAX_KEYVALUELEN];
extern TCHAR TCSelecttarget[MAX_KEYVALUELEN];

extern TCHAR TCMap[MAX_KEYVALUELEN];
extern TCHAR TCMapzoomin[MAX_KEYVALUELEN];
extern TCHAR TCMapzoomout[MAX_KEYVALUELEN];


extern void StripOffModifier(TCHAR *TCValue);
extern void ExcludeModifier(TCHAR *TCValue, TCHAR *TCExcluded);
extern void AppendModifier(TCHAR *TCString, int iAppend);
extern int ModifierNum(TCHAR *TCValue);
extern BOOL IsModifier(TCHAR *TCValue);
extern void ProcessButtonPressed(TCHAR *TCButtonstring, CButton *modifier, int nID, HWND m_hWnd);
extern void InitModifierButton(TCHAR *TCButtonstring, CButton *modifier);
extern void ProcessModifierPressed(TCHAR *TCString, int nID, HWND m_hWnd);
extern void BooltoString(BOOL bValue, TCHAR *TCValue);
extern BOOL StringtoBool(TCHAR *TCValue);


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_M59BIND_H__0FD9EA3D_FDFC_4ACB_831B_821737DC7907__INCLUDED_)

