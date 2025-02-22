// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Movement.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "preferences.h"
#include "preferences_movement.h"
#include "preferences_assignkey.h"
#include "preferences_modifier.h"
#include "preferences_common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovementPage property page

IMPLEMENT_DYNCREATE(CMovementPage, CPropertyPage)

CMovementPage::CMovementPage() : CPropertyPage(CMovementPage::IDD)
{
	//{{AFX_DATA_INIT(CMovementPage)
	//}}AFX_DATA_INIT
}

CMovementPage::~CMovementPage()
{
}

void CMovementPage::ProcessButtonPressed(TCHAR* TCButtonstring, CButton* modifier, int nID, HWND m_hWnd)
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

void CMovementPage::ProcessModifierPressed(TCHAR* TCString, int nID, HWND m_hWnd)
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

void CMovementPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovementPage)
	DDX_Control(pDX, IDC_SLIDERIGHT_MOD, m_slideright_mod);
	DDX_Control(pDX, IDC_SLIDELEFT_MOD, m_slideleft_mod);
	DDX_Control(pDX, IDC_RUNWALK_MOD, m_runwalk_mod);
	DDX_Control(pDX, IDC_RIGHT_MOD, m_right_mod);
	DDX_Control(pDX, IDC_MOUSELOOKTOGGLE_MOD, m_mouselooktoggle_mod);
	DDX_Control(pDX, IDC_LOOKUP_MOD, m_lookup_mod);
	DDX_Control(pDX, IDC_LOOKSTRAIGHT_MOD, m_lookstraight_mod);
	DDX_Control(pDX, IDC_LOOKDOWN_MOD, m_lookdown_mod);
	DDX_Control(pDX, IDC_LEFT_MOD, m_left_mod);
	DDX_Control(pDX, IDC_FORWARD_MOD, m_forward_mod);
	DDX_Control(pDX, IDC_FLIP_MOD, m_flip_mod);
	DDX_Control(pDX, IDC_BACKWARD_MOD, m_backward_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMovementPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMovementPage)
	ON_BN_CLICKED(IDC_BACKWARD, OnBackward)
	ON_BN_CLICKED(IDC_FLIP, OnFlip)
	ON_BN_CLICKED(IDC_FORWARD, OnForward)
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_LOOKDOWN, OnLookdown)
	ON_BN_CLICKED(IDC_LOOKSTRAIGHT, OnLookstraight)
	ON_BN_CLICKED(IDC_LOOKUP, OnLookup)
	ON_BN_CLICKED(IDC_MOUSELOOKTOGGLE, OnMouselooktoggle)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_RUNWALK, OnRunwalk)
	ON_BN_CLICKED(IDC_SLIDELEFT, OnSlideleft)
	ON_BN_CLICKED(IDC_SLIDERIGHT, OnSlideright)
	ON_BN_CLICKED(IDC_BACKWARD_MOD, OnBackwardMod)
	ON_BN_CLICKED(IDC_FLIP_MOD, OnFlipMod)
	ON_BN_CLICKED(IDC_FORWARD_MOD, OnForwardMod)
	ON_BN_CLICKED(IDC_LEFT_MOD, OnLeftMod)
	ON_BN_CLICKED(IDC_LOOKDOWN_MOD, OnLookdownMod)
	ON_BN_CLICKED(IDC_LOOKSTRAIGHT_MOD, OnLookstraightMod)
	ON_BN_CLICKED(IDC_LOOKUP_MOD, OnLookupMod)
	ON_BN_CLICKED(IDC_MOUSELOOKTOGGLE_MOD, OnMouselooktoggleMod)
	ON_BN_CLICKED(IDC_RIGHT_MOD, OnRightMod)
	ON_BN_CLICKED(IDC_RUNWALK_MOD, OnRunwalkMod)
	ON_BN_CLICKED(IDC_SLIDELEFT_MOD, OnSlideleftMod)
	ON_BN_CLICKED(IDC_SLIDERIGHT_MOD, OnSliderightMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovementPage message handlers
void CMovementPage::UpdateDialogData()
{
	SetDlgItemText(IDC_BACKWARD, TCBackward);
	SetDlgItemText(IDC_FLIP, TCFlip);
	SetDlgItemText(IDC_FORWARD, TCForward);
	SetDlgItemText(IDC_LEFT, TCLeft);
	SetDlgItemText(IDC_LOOKDOWN, TCLookdown);
	SetDlgItemText(IDC_LOOKSTRAIGHT, TCLookstraight);
	SetDlgItemText(IDC_LOOKUP, TCLookup);
	SetDlgItemText(IDC_MOUSELOOKTOGGLE, TCMouselooktoggle);
	SetDlgItemText(IDC_RIGHT, TCRight);
	SetDlgItemText(IDC_RUNWALK, TCRunwalk);
	SetDlgItemText(IDC_SLIDELEFT, TCSlideleft);
	SetDlgItemText(IDC_SLIDERIGHT, TCSlideright);

	InitModifierButton(TCBackward, &m_backward_mod);
	InitModifierButton(TCFlip, &m_flip_mod);
	InitModifierButton(TCForward, &m_forward_mod);
	InitModifierButton(TCLeft, &m_left_mod);
	InitModifierButton(TCLookdown, &m_lookdown_mod);
	InitModifierButton(TCLookstraight, &m_lookstraight_mod);
	InitModifierButton(TCLookup, &m_lookup_mod);
	InitModifierButton(TCMouselooktoggle, &m_mouselooktoggle_mod);
	InitModifierButton(TCRight, &m_right_mod);
	InitModifierButton(TCRunwalk, &m_runwalk_mod);
	InitModifierButton(TCSlideleft, &m_slideleft_mod);
	InitModifierButton(TCSlideright, &m_slideright_mod);
}

BOOL CMovementPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateDialogData();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMovementPage::OnBackward()
{
	ProcessButtonPressed(TCBackward, &m_backward_mod, IDC_BACKWARD, m_hWnd);
}

void CMovementPage::OnFlip()
{
	ProcessButtonPressed(TCFlip, &m_flip_mod, IDC_FLIP, m_hWnd);
}

void CMovementPage::OnForward()
{
	ProcessButtonPressed(TCForward, &m_forward_mod, IDC_FORWARD, m_hWnd);
}

void CMovementPage::OnLeft()
{
	ProcessButtonPressed(TCLeft, &m_left_mod, IDC_LEFT, m_hWnd);
}

void CMovementPage::OnLookdown()
{
	ProcessButtonPressed(TCLookdown, &m_lookdown_mod, IDC_LOOKDOWN, m_hWnd);
}

void CMovementPage::OnLookstraight()
{
	ProcessButtonPressed(TCLookstraight, &m_lookstraight_mod, IDC_LOOKSTRAIGHT, m_hWnd);
}

void CMovementPage::OnLookup()
{
	ProcessButtonPressed(TCLookup, &m_lookup_mod, IDC_LOOKUP, m_hWnd);
}

void CMovementPage::OnMouselooktoggle()
{
	ProcessButtonPressed(TCMouselooktoggle, &m_mouselooktoggle_mod, IDC_MOUSELOOKTOGGLE, m_hWnd);
}

void CMovementPage::OnRight()
{
	ProcessButtonPressed(TCRight, &m_right_mod, IDC_RIGHT, m_hWnd);
}

void CMovementPage::OnRunwalk()
{
	ProcessButtonPressed(TCRunwalk, &m_runwalk_mod, IDC_RUNWALK, m_hWnd);
}

void CMovementPage::OnSlideleft()
{
	ProcessButtonPressed(TCSlideleft, &m_slideleft_mod, IDC_SLIDELEFT, m_hWnd);
}

void CMovementPage::OnSlideright()
{
	ProcessButtonPressed(TCSlideright, &m_slideright_mod, IDC_SLIDERIGHT, m_hWnd);
}

void CMovementPage::OnBackwardMod()
{
	ProcessModifierPressed(TCBackward, IDC_BACKWARD, m_hWnd);
}

void CMovementPage::OnFlipMod()
{
	ProcessModifierPressed(TCFlip, IDC_FLIP, m_hWnd);
}

void CMovementPage::OnForwardMod()
{
	ProcessModifierPressed(TCForward, IDC_FORWARD, m_hWnd);
}

void CMovementPage::OnLeftMod()
{
	ProcessModifierPressed(TCLeft, IDC_LEFT, m_hWnd);
}

void CMovementPage::OnLookdownMod()
{
	ProcessModifierPressed(TCLookdown, IDC_LOOKDOWN, m_hWnd);
}

void CMovementPage::OnLookstraightMod()
{
	ProcessModifierPressed(TCLookstraight, IDC_LOOKSTRAIGHT, m_hWnd);
}

void CMovementPage::OnLookupMod()
{
	ProcessModifierPressed(TCLookup, IDC_LOOKUP, m_hWnd);
}

void CMovementPage::OnMouselooktoggleMod()
{
	ProcessModifierPressed(TCMouselooktoggle, IDC_MOUSELOOKTOGGLE, m_hWnd);
}

void CMovementPage::OnRightMod()
{
	ProcessModifierPressed(TCRight, IDC_RIGHT, m_hWnd);
}

void CMovementPage::OnRunwalkMod()
{
	ProcessModifierPressed(TCRunwalk, IDC_RUNWALK, m_hWnd);
}

void CMovementPage::OnSlideleftMod()
{
	ProcessModifierPressed(TCSlideleft, IDC_SLIDELEFT, m_hWnd);
}

void CMovementPage::OnSliderightMod()
{
	ProcessModifierPressed(TCSlideright, IDC_SLIDERIGHT, m_hWnd);
}
