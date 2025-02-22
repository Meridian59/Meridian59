// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Targeting.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "preferences_targeting.h"
#include "preferences_assignkey.h"
#include "preferences_modifier.h"
#include "preferences_common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTargetingPage property page

IMPLEMENT_DYNCREATE(CTargetingPage, CPropertyPage)

CTargetingPage::CTargetingPage() : CPropertyPage(CTargetingPage::IDD)
{
	//{{AFX_DATA_INIT(CTargetingPage)
	//}}AFX_DATA_INIT
}

CTargetingPage::~CTargetingPage()
{
}

void CTargetingPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetingPage)
	DDX_Control(pDX, IDC_TARGETSELF_MOD, m_targetself_mod);
	DDX_Control(pDX, IDC_TARGETPREVIOUS_MOD, m_targetprevious_mod);
	DDX_Control(pDX, IDC_TARGETNEXT_MOD, m_targetnext_mod);
	DDX_Control(pDX, IDC_TARGETCLEAR_MOD, m_targetclear_mod);
	DDX_Control(pDX, IDC_TABFORWARD_MOD, m_tabforward_mod);
	DDX_Control(pDX, IDC_TABBACKWARD_MOD, m_tabbackward_mod);
	DDX_Control(pDX, IDC_SELECTTARGET_MOD, m_selecttarget_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTargetingPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTargetingPage)
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
// CTargetingPage message handlers
void CTargetingPage::UpdateDialogData()
{
	SetDlgItemText(IDC_TABBACKWARD, TCTabbackward);
	SetDlgItemText(IDC_TABFORWARD, TCTabforward);
	SetDlgItemText(IDC_TARGETCLEAR, TCTargetclear);
	SetDlgItemText(IDC_TARGETNEXT, TCTargetnext);
	SetDlgItemText(IDC_TARGETPREVIOUS, TCTargetprevious);
	SetDlgItemText(IDC_TARGETSELF, TCTargetself);
	SetDlgItemText(IDC_SELECTTARGET, TCSelecttarget);

	InitModifierButton(TCTabbackward, &m_tabbackward_mod);
	InitModifierButton(TCTabforward, &m_tabforward_mod);
	InitModifierButton(TCTargetclear, &m_targetclear_mod);
	InitModifierButton(TCTargetnext, &m_targetnext_mod);
	InitModifierButton(TCTargetprevious, &m_targetprevious_mod);
	InitModifierButton(TCTargetself, &m_targetself_mod);
	InitModifierButton(TCSelecttarget, &m_selecttarget_mod);
}


BOOL CTargetingPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateDialogData();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTargetingPage::ProcessButtonPressed(TCHAR* TCButtonstring, CButton* modifier, int nID, HWND m_hWnd)
{
	CAssignkey dlg;
	dlg.DoModal();

	strcpy(TCButtonstring, TCNewkey);

	if (IsModifier(TCButtonstring))
	{
		// Disable Modifier button
		modifier->EnableWindow(FALSE);
	}
	else
	{
		AppendModifier(TCButtonstring, iModifier);
		// Enable Modifier button
		modifier->EnableWindow(TRUE);
	}

	SetDlgItemText(nID, TCButtonstring);

	CheckforDuplicateBindM(TCButtonstring);
}

void CTargetingPage::ProcessModifierPressed(TCHAR* TCString, int nID, HWND m_hWnd)
{
	CModifier dlg;

	iModifier = MODIFIER_NONE;
	auto nResponse = dlg.DoModal();
	if (nResponse == IDCANCEL)
	{
		return;
	}

	StripOffModifier(TCString);
	AppendModifier(TCString, iModifier);

	SetDlgItemText(nID, TCString);

	CheckforDuplicateBindM(TCString);
}

void CTargetingPage::OnTabbackward()
{
	ProcessButtonPressed(TCTabbackward, &m_tabbackward_mod, IDC_TABBACKWARD, m_hWnd);
}

void CTargetingPage::OnTabforward()
{
	ProcessButtonPressed(TCTabforward, &m_tabforward_mod, IDC_TABFORWARD, m_hWnd);
}

void CTargetingPage::OnTargetclear()
{
	ProcessButtonPressed(TCTargetclear, &m_targetclear_mod, IDC_TARGETCLEAR, m_hWnd);
}

void CTargetingPage::OnTargetnext()
{
	ProcessButtonPressed(TCTargetnext, &m_targetnext_mod, IDC_TARGETNEXT, m_hWnd);
}

void CTargetingPage::OnTargetprevious()
{
	ProcessButtonPressed(TCTargetprevious, &m_targetprevious_mod, IDC_TARGETPREVIOUS, m_hWnd);
}

void CTargetingPage::OnTargetself()
{
	ProcessButtonPressed(TCTargetself, &m_targetself_mod, IDC_TARGETSELF, m_hWnd);
}

void CTargetingPage::OnSelecttarget()
{
	ProcessButtonPressed(TCSelecttarget, &m_selecttarget_mod, IDC_SELECTTARGET, m_hWnd);
}

void CTargetingPage::OnSelecttargetMod()
{
	ProcessModifierPressed(TCSelecttarget, IDC_SELECTTARGET, m_hWnd);
}

void CTargetingPage::OnTabbackwardMod()
{
	ProcessModifierPressed(TCTabbackward, IDC_TABBACKWARD, m_hWnd);
}

void CTargetingPage::OnTabforwardMod()
{
	ProcessModifierPressed(TCTabforward, IDC_TABFORWARD, m_hWnd);
}

void CTargetingPage::OnTargetclearMod()
{
	ProcessModifierPressed(TCTargetclear, IDC_TARGETCLEAR, m_hWnd);
}

void CTargetingPage::OnTargetnextMod()
{
	ProcessModifierPressed(TCTargetnext, IDC_TARGETNEXT, m_hWnd);
}

void CTargetingPage::OnTargetpreviousMod()
{
	ProcessModifierPressed(TCTargetprevious, IDC_TARGETPREVIOUS, m_hWnd);
}

void CTargetingPage::OnTargetselfMod()
{
	ProcessModifierPressed(TCTargetself, IDC_TARGETSELF, m_hWnd);
}
