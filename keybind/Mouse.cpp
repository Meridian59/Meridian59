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
#include "m59bind.h"

#include "defaults.h"
#include "Mouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMouse property page

IMPLEMENT_DYNCREATE(CMouse, CPropertyPage)

CMouse::CMouse() : CPropertyPage(CMouse::IDD)
{
	//{{AFX_DATA_INIT(CMouse)
	m_invert = FALSE;
	//}}AFX_DATA_INIT
}

CMouse::~CMouse()
{
}

void CMouse::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMouse)
	DDX_Control(pDX, IDC_MOUSEYSCALE, m_mouseyscale);
	DDX_Control(pDX, IDC_MOUSEXSCALE, m_mousexscale);
	DDX_Check(pDX, IDC_INVERT, m_invert);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMouse, CPropertyPage)
	//{{AFX_MSG_MAP(CMouse)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMouse message handlers
void CMouse::UpdateDialogData()
{
  TCHAR TCValue[MAX_KEYVALUELEN];

  m_invert=bInvert;

//  UpdateData(FALSE);

  CButton* pCheck;
	pCheck = (CButton*) GetDlgItem(IDC_INVERT);

  if(pCheck!=NULL)
  {
    if(bInvert)
      pCheck->SetCheck(1);
    else
      pCheck->SetCheck(0);
  }

	m_mousexscale.SetPos(iMouselookXscale);
	m_mouseyscale.SetPos(iMouselookYscale);

  sprintf(TCValue,"%d",iMouselookXscale);
  SetDlgItemText(IDC_MOUSEXSCALEVALUE,TCValue);
  sprintf(TCValue,"%d",iMouselookYscale);
  SetDlgItemText(IDC_MOUSEYSCALEVALUE,TCValue);
}

BOOL CMouse::OnInitDialog() 
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

void CMouse::OnInvert() 
{
  UpdateData(TRUE);
  bInvert=m_invert;
}

void CMouse::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  TCHAR TCValue[MAX_KEYVALUELEN];

  iMouselookXscale=m_mousexscale.GetPos();
  iMouselookYscale=m_mouseyscale.GetPos();
  
  sprintf(TCValue,"%d",iMouselookXscale);
  SetDlgItemText(IDC_MOUSEXSCALEVALUE,TCValue);
  sprintf(TCValue,"%d",iMouselookYscale);
  SetDlgItemText(IDC_MOUSEYSCALEVALUE,TCValue);
	
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}
