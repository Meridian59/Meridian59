// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Targeting.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Targeting.h"
#include "Assignkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTargeting property page

IMPLEMENT_DYNCREATE(CTargeting, CPropertyPage)

CTargeting::CTargeting() : CPropertyPage(CTargeting::IDD)
{
	//{{AFX_DATA_INIT(CTargeting)
	//}}AFX_DATA_INIT
}

CTargeting::~CTargeting()
{
}

void CTargeting::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargeting)
	DDX_Control(pDX, IDC_TARGETSELF_MOD, m_targetself_mod);
	DDX_Control(pDX, IDC_TARGETPREVIOUS_MOD, m_targetprevious_mod);
	DDX_Control(pDX, IDC_TARGETNEXT_MOD, m_targetnext_mod);
	DDX_Control(pDX, IDC_TARGETCLEAR_MOD, m_targetclear_mod);
	DDX_Control(pDX, IDC_TABFORWARD_MOD, m_tabforward_mod);
	DDX_Control(pDX, IDC_TABBACKWARD_MOD, m_tabbackward_mod);
	DDX_Control(pDX, IDC_SELECTTARGET_MOD, m_selecttarget_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTargeting, CPropertyPage)
	//{{AFX_MSG_MAP(CTargeting)
	ON_BN_CLICKED(IDC_TABBACKWARD, OnTabbackward)
	ON_BN_CLICKED(IDC_TABFORWARD, OnTabforward)
	ON_BN_CLICKED(IDC_TARGETCLEAR, OnTargetclear)
	ON_BN_CLICKED(IDC_TARGETNEXT, OnTargetnext)
	ON_BN_CLICKED(IDC_TARGETPREVIOUS, OnTargetprevious)
	ON_BN_CLICKED(IDC_TARGETSELF, OnTargetself)
	ON_BN_CLICKED(IDC_SELECTTARGET, OnSelecttarget)
	ON_BN_CLICKED(IDC_SELECTTARGET_MOD, OnSelecttargetMod)
	ON_BN_CLICKED(IDC_TABBACKWARD_MOD, OnTabbackwardMod)
	ON_BN_CLICKED(IDC_TABFORWARD_MOD, OnTabforwardMod)
	ON_BN_CLICKED(IDC_TARGETCLEAR_MOD, OnTargetclearMod)
	ON_BN_CLICKED(IDC_TARGETNEXT_MOD, OnTargetnextMod)
	ON_BN_CLICKED(IDC_TARGETPREVIOUS_MOD, OnTargetpreviousMod)
	ON_BN_CLICKED(IDC_TARGETSELF_MOD, OnTargetselfMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargeting message handlers
void CTargeting::UpdateDialogData()
{
  SetDlgItemText(IDC_TABBACKWARD,TCTabbackward);
  SetDlgItemText(IDC_TABFORWARD,TCTabforward);
  SetDlgItemText(IDC_TARGETCLEAR,TCTargetclear);
  SetDlgItemText(IDC_TARGETNEXT,TCTargetnext);
  SetDlgItemText(IDC_TARGETPREVIOUS,TCTargetprevious);
  SetDlgItemText(IDC_TARGETSELF,TCTargetself);
  SetDlgItemText(IDC_SELECTTARGET,TCSelecttarget);

  InitModifierButton(TCTabbackward,&m_tabbackward_mod);
  InitModifierButton(TCTabforward,&m_tabforward_mod);
  InitModifierButton(TCTargetclear,&m_targetclear_mod);
  InitModifierButton(TCTargetnext,&m_targetnext_mod);
  InitModifierButton(TCTargetprevious,&m_targetprevious_mod);
  InitModifierButton(TCTargetself,&m_targetself_mod);
  InitModifierButton(TCSelecttarget,&m_selecttarget_mod);
}

BOOL CTargeting::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  UpdateDialogData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTargeting::OnTabbackward() 
{
  ProcessButtonPressed(TCTabbackward,&m_tabbackward_mod,IDC_TABBACKWARD,m_hWnd);
}

void CTargeting::OnTabforward() 
{
  ProcessButtonPressed(TCTabforward,&m_tabforward_mod,IDC_TABFORWARD,m_hWnd);
}

void CTargeting::OnTargetclear() 
{
  ProcessButtonPressed(TCTargetclear,&m_targetclear_mod,IDC_TARGETCLEAR,m_hWnd);
}

void CTargeting::OnTargetnext() 
{
  ProcessButtonPressed(TCTargetnext,&m_targetnext_mod,IDC_TARGETNEXT,m_hWnd);
}

void CTargeting::OnTargetprevious() 
{
  ProcessButtonPressed(TCTargetprevious,&m_targetprevious_mod,IDC_TARGETPREVIOUS,m_hWnd);
}

void CTargeting::OnTargetself() 
{
  ProcessButtonPressed(TCTargetself,&m_targetself_mod,IDC_TARGETSELF,m_hWnd);
}

void CTargeting::OnSelecttarget() 
{
  ProcessButtonPressed(TCSelecttarget,&m_selecttarget_mod,IDC_SELECTTARGET,m_hWnd);
}

void CTargeting::OnSelecttargetMod() 
{
  ProcessModifierPressed(TCSelecttarget,IDC_SELECTTARGET,m_hWnd);
}

void CTargeting::OnTabbackwardMod() 
{
  ProcessModifierPressed(TCTabbackward,IDC_TABBACKWARD,m_hWnd);
}

void CTargeting::OnTabforwardMod() 
{
  ProcessModifierPressed(TCTabforward,IDC_TABFORWARD,m_hWnd);
}

void CTargeting::OnTargetclearMod() 
{
  ProcessModifierPressed(TCTargetclear,IDC_TARGETCLEAR,m_hWnd);
}

void CTargeting::OnTargetnextMod() 
{
  ProcessModifierPressed(TCTargetnext,IDC_TARGETNEXT,m_hWnd);
}

void CTargeting::OnTargetpreviousMod() 
{
  ProcessModifierPressed(TCTargetprevious,IDC_TARGETPREVIOUS,m_hWnd);
}

void CTargeting::OnTargetselfMod() 
{
  ProcessModifierPressed(TCTargetself,IDC_TARGETSELF,m_hWnd);
}
