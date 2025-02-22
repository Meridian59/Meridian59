// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Mouse.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "preferences_mouse.h"
#include "preferences_assignkey.h"
#include "preferences_modifier.h"
#include "preferences_common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMousePage property page

IMPLEMENT_DYNCREATE(CMousePage, CPropertyPage)

CMousePage::CMousePage() : CPropertyPage(CMousePage::IDD)
{
	//{{AFX_DATA_INIT(CMousePage)
	m_invert = FALSE;
	//}}AFX_DATA_INIT
}

CMousePage::~CMousePage()
{
}

void CMousePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMousePage)
	DDX_Control(pDX, IDC_MOUSEYSCALE, m_mouseyscale);
	DDX_Control(pDX, IDC_MOUSEXSCALE, m_mousexscale);
	DDX_Check(pDX, IDC_INVERT, m_invert);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMousePage, CPropertyPage)
	//{{AFX_MSG_MAP(CMousePage)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMousePage message handlers
void CMousePage::UpdateDialogData()
{
	TCHAR TCValue[MAX_KEYVALUELEN];

	m_invert = bInvert;

	//  UpdateData(FALSE);

	CButton* pCheck;
	pCheck = (CButton*)GetDlgItem(IDC_INVERT);

	if (pCheck != NULL)
	{
		if (bInvert)
			pCheck->SetCheck(1);
		else
			pCheck->SetCheck(0);
	}

	m_mousexscale.SetPos(iMouselookXscale);
	m_mouseyscale.SetPos(iMouselookYscale);

	sprintf(TCValue, "%d", iMouselookXscale);
	SetDlgItemText(IDC_MOUSEXSCALEVALUE, TCValue);
	sprintf(TCValue, "%d", iMouselookYscale);
	SetDlgItemText(IDC_MOUSEYSCALEVALUE, TCValue);
}

BOOL CMousePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_mousexscale.SetRange(MIN_MOUSEXSCALE, MAX_MOUSEXSCALE);
	m_mousexscale.SetTicFreq(TIC_MOUSEXSCALE);

	m_mouseyscale.SetRange(MIN_MOUSEYSCALE, MAX_MOUSEYSCALE);
	m_mouseyscale.SetTicFreq(TIC_MOUSEYSCALE);

	UpdateDialogData();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMousePage::OnInvert()
{
	UpdateData(TRUE);
	bInvert = m_invert;
}

void CMousePage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	TCHAR TCValue[MAX_KEYVALUELEN];

	iMouselookXscale = m_mousexscale.GetPos();
	iMouselookYscale = m_mouseyscale.GetPos();

	sprintf(TCValue, "%d", iMouselookXscale);
	SetDlgItemText(IDC_MOUSEXSCALEVALUE, TCValue);
	sprintf(TCValue, "%d", iMouselookYscale);
	SetDlgItemText(IDC_MOUSEYSCALEVALUE, TCValue);
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}
