// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Modifier.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Modifier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifier dialog


CModifier::CModifier(CWnd* pParent /*=NULL*/)
	: CDialog(CModifier::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifier)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CModifier::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifier)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifier, CDialog)
	//{{AFX_MSG_MAP(CModifier)
	ON_BN_CLICKED(IDC_ALT, OnAlt)
	ON_BN_CLICKED(IDC_ANY, OnAny)
	ON_BN_CLICKED(IDC_CTRL, OnCtrl)
	ON_BN_CLICKED(IDC_SHIFT, OnShift)
	ON_BN_CLICKED(IDC_NONE, OnNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifier message handlers

void CModifier::OnAlt() 
{
  iModifier=MODIFIER_ALT;
  EndDialog(IDOK);
}

void CModifier::OnCtrl() 
{
  iModifier=MODIFIER_CTRL;
  EndDialog(IDOK);
}

void CModifier::OnShift() 
{
  iModifier=MODIFIER_SHIFT;
  EndDialog(IDOK);
}

void CModifier::OnAny() 
{
  iModifier=MODIFIER_ANY;
  EndDialog(IDOK);
}


void CModifier::OnNone() 
{
  iModifier=MODIFIER_NONE;
  EndDialog(IDOK);
}
