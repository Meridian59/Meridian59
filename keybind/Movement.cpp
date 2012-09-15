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
#include "m59bind.h"
#include "Movement.h"
#include "Assignkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovement property page

IMPLEMENT_DYNCREATE(CMovement, CPropertyPage)

CMovement::CMovement() : CPropertyPage(CMovement::IDD)
{
	//{{AFX_DATA_INIT(CMovement)
	//}}AFX_DATA_INIT
}

CMovement::~CMovement()
{
}

void CMovement::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovement)
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


BEGIN_MESSAGE_MAP(CMovement, CPropertyPage)
	//{{AFX_MSG_MAP(CMovement)
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
// CMovement message handlers
void CMovement::UpdateDialogData()
{
  SetDlgItemText(IDC_BACKWARD,TCBackward);
  SetDlgItemText(IDC_FLIP,TCFlip);
  SetDlgItemText(IDC_FORWARD,TCForward);
  SetDlgItemText(IDC_LEFT,TCLeft);
  SetDlgItemText(IDC_LOOKDOWN,TCLookdown);
  SetDlgItemText(IDC_LOOKSTRAIGHT,TCLookstraight);
  SetDlgItemText(IDC_LOOKUP,TCLookup);
  SetDlgItemText(IDC_MOUSELOOKTOGGLE,TCMouselooktoggle);
  SetDlgItemText(IDC_RIGHT,TCRight);
  SetDlgItemText(IDC_RUNWALK,TCRunwalk);
  SetDlgItemText(IDC_SLIDELEFT,TCSlideleft);
  SetDlgItemText(IDC_SLIDERIGHT,TCSlideright);

  InitModifierButton(TCBackward,&m_backward_mod);
  InitModifierButton(TCFlip,&m_flip_mod);
  InitModifierButton(TCForward,&m_forward_mod);
  InitModifierButton(TCLeft,&m_left_mod);
  InitModifierButton(TCLookdown,&m_lookdown_mod);
  InitModifierButton(TCLookstraight,&m_lookstraight_mod);
  InitModifierButton(TCLookup,&m_lookup_mod);
  InitModifierButton(TCMouselooktoggle,&m_mouselooktoggle_mod);
  InitModifierButton(TCRight,&m_right_mod);
  InitModifierButton(TCRunwalk,&m_runwalk_mod);
  InitModifierButton(TCSlideleft,&m_slideleft_mod);
  InitModifierButton(TCSlideright,&m_slideright_mod);
}

BOOL CMovement::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  UpdateDialogData();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMovement::OnBackward() 
{
  ProcessButtonPressed(TCBackward,&m_backward_mod,IDC_BACKWARD,m_hWnd);
}

void CMovement::OnFlip() 
{
  ProcessButtonPressed(TCFlip,&m_flip_mod,IDC_FLIP,m_hWnd);
}

void CMovement::OnForward() 
{
  ProcessButtonPressed(TCForward,&m_forward_mod,IDC_FORWARD,m_hWnd);
}

void CMovement::OnLeft() 
{
  ProcessButtonPressed(TCLeft,&m_left_mod,IDC_LEFT,m_hWnd);
}

void CMovement::OnLookdown() 
{
  ProcessButtonPressed(TCLookdown,&m_lookdown_mod,IDC_LOOKDOWN,m_hWnd);
}

void CMovement::OnLookstraight() 
{
  ProcessButtonPressed(TCLookstraight,&m_lookstraight_mod,IDC_LOOKSTRAIGHT,m_hWnd);
}

void CMovement::OnLookup() 
{
  ProcessButtonPressed(TCLookup,&m_lookup_mod,IDC_LOOKUP,m_hWnd);
}

void CMovement::OnMouselooktoggle() 
{
  ProcessButtonPressed(TCMouselooktoggle,&m_mouselooktoggle_mod,IDC_MOUSELOOKTOGGLE,m_hWnd);
}

void CMovement::OnRight() 
{
  ProcessButtonPressed(TCRight,&m_right_mod,IDC_RIGHT,m_hWnd);
}

void CMovement::OnRunwalk() 
{
  ProcessButtonPressed(TCRunwalk,&m_runwalk_mod,IDC_RUNWALK,m_hWnd);
}

void CMovement::OnSlideleft() 
{
  ProcessButtonPressed(TCSlideleft,&m_slideleft_mod,IDC_SLIDELEFT,m_hWnd);
}

void CMovement::OnSlideright() 
{
  ProcessButtonPressed(TCSlideright,&m_slideright_mod,IDC_SLIDERIGHT,m_hWnd);
}

void CMovement::OnBackwardMod() 
{
  ProcessModifierPressed(TCBackward,IDC_BACKWARD,m_hWnd);
}

void CMovement::OnFlipMod() 
{
  ProcessModifierPressed(TCFlip,IDC_FLIP,m_hWnd);
}

void CMovement::OnForwardMod() 
{
  ProcessModifierPressed(TCForward,IDC_FORWARD,m_hWnd);
}

void CMovement::OnLeftMod() 
{
  ProcessModifierPressed(TCLeft,IDC_LEFT,m_hWnd);
}

void CMovement::OnLookdownMod() 
{
  ProcessModifierPressed(TCLookdown,IDC_LOOKDOWN,m_hWnd);
}

void CMovement::OnLookstraightMod() 
{
  ProcessModifierPressed(TCLookstraight,IDC_LOOKSTRAIGHT,m_hWnd);
}

void CMovement::OnLookupMod() 
{
  ProcessModifierPressed(TCLookup,IDC_LOOKUP,m_hWnd);
}

void CMovement::OnMouselooktoggleMod() 
{
  ProcessModifierPressed(TCMouselooktoggle,IDC_MOUSELOOKTOGGLE,m_hWnd);
}

void CMovement::OnRightMod() 
{
  ProcessModifierPressed(TCRight,IDC_RIGHT,m_hWnd);
}

void CMovement::OnRunwalkMod() 
{
  ProcessModifierPressed(TCRunwalk,IDC_RUNWALK,m_hWnd);
}

void CMovement::OnSlideleftMod() 
{
  ProcessModifierPressed(TCSlideleft,IDC_SLIDELEFT,m_hWnd);
}

void CMovement::OnSliderightMod() 
{
  ProcessModifierPressed(TCSlideright,IDC_SLIDERIGHT,m_hWnd);
}
