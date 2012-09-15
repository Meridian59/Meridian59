// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Interaction.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Interaction.h"
#include "Assignkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInteraction property page

IMPLEMENT_DYNCREATE(CInteraction, CPropertyPage)

CInteraction::CInteraction() : CPropertyPage(CInteraction::IDD)
{
	//{{AFX_DATA_INIT(CInteraction)
	//}}AFX_DATA_INIT
}

CInteraction::~CInteraction()
{
}

void CInteraction::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInteraction)
	DDX_Control(pDX, IDC_WITHDRAW_MOD, m_withdraw_mod);
	DDX_Control(pDX, IDC_PICKUP_MOD, m_pickup_mod);
	DDX_Control(pDX, IDC_OPEN_MOD, m_open_mod);
	DDX_Control(pDX, IDC_OFFER_MOD, m_offer_mod);
	DDX_Control(pDX, IDC_LOOK_MOD, m_look_mod);
	DDX_Control(pDX, IDC_EXAMINE_MOD, m_examine_mod);
	DDX_Control(pDX, IDC_DEPOSIT_MOD, m_deposit_mod);
	DDX_Control(pDX, IDC_BUY_MOD, m_buy_mod);
	DDX_Control(pDX, IDC_ATTACK_MOD, m_attack_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInteraction, CPropertyPage)
	//{{AFX_MSG_MAP(CInteraction)
	ON_BN_CLICKED(IDC_ATTACK, OnAttack)
	ON_BN_CLICKED(IDC_BUY, OnBuy)
	ON_BN_CLICKED(IDC_DEPOSIT, OnDeposit)
	ON_BN_CLICKED(IDC_EXAMINE, OnExamine)
	ON_BN_CLICKED(IDC_LOOK, OnLook)
	ON_BN_CLICKED(IDC_OFFER, OnOffer)
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_PICKUP, OnPickup)
	ON_BN_CLICKED(IDC_WITHDRAW, OnWithdraw)
	ON_BN_CLICKED(IDC_ATTACK_MOD, OnAttackMod)
	ON_BN_CLICKED(IDC_BUY_MOD, OnBuyMod)
	ON_BN_CLICKED(IDC_DEPOSIT_MOD, OnDepositMod)
	ON_BN_CLICKED(IDC_EXAMINE_MOD, OnExamineMod)
	ON_BN_CLICKED(IDC_LOOK_MOD, OnLookMod)
	ON_BN_CLICKED(IDC_OFFER_MOD, OnOfferMod)
	ON_BN_CLICKED(IDC_OPEN_MOD, OnOpenMod)
	ON_BN_CLICKED(IDC_PICKUP_MOD, OnPickupMod)
	ON_BN_CLICKED(IDC_WITHDRAW_MOD, OnWithdrawMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInteraction message handlers
void CInteraction::UpdateDialogData()
{
  SetDlgItemText(IDC_ATTACK,TCAttack);
  SetDlgItemText(IDC_BUY,TCBuy);
  SetDlgItemText(IDC_DEPOSIT,TCDeposit);
  SetDlgItemText(IDC_EXAMINE,TCExamine);
  SetDlgItemText(IDC_LOOK,TCLook);
  SetDlgItemText(IDC_OFFER,TCOffer);
  SetDlgItemText(IDC_OPEN,TCOpen);
  SetDlgItemText(IDC_PICKUP,TCPickup);
  SetDlgItemText(IDC_WITHDRAW,TCWithdraw);

  InitModifierButton(TCAttack,&m_attack_mod);
  InitModifierButton(TCDeposit,&m_deposit_mod);
  InitModifierButton(TCExamine,&m_examine_mod);
  InitModifierButton(TCLook,&m_look_mod);
  InitModifierButton(TCOffer,&m_offer_mod);
  InitModifierButton(TCOpen,&m_open_mod);
  InitModifierButton(TCPickup,&m_pickup_mod);
  InitModifierButton(TCWithdraw,&m_withdraw_mod);
}

BOOL CInteraction::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  UpdateDialogData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInteraction::OnAttack() 
{
  ProcessButtonPressed(TCAttack,&m_attack_mod,IDC_ATTACK,m_hWnd);
}

void CInteraction::OnBuy() 
{
  ProcessButtonPressed(TCBuy,&m_buy_mod,IDC_BUY,m_hWnd);
}

void CInteraction::OnDeposit() 
{
  ProcessButtonPressed(TCDeposit,&m_deposit_mod,IDC_DEPOSIT,m_hWnd);
}

void CInteraction::OnExamine() 
{
  ProcessButtonPressed(TCExamine,&m_examine_mod,IDC_EXAMINE,m_hWnd);
}

void CInteraction::OnLook() 
{
  ProcessButtonPressed(TCLook,&m_look_mod,IDC_LOOK,m_hWnd);
}

void CInteraction::OnOffer() 
{
  ProcessButtonPressed(TCOffer,&m_offer_mod,IDC_OFFER,m_hWnd);
}

void CInteraction::OnOpen() 
{
  ProcessButtonPressed(TCOpen,&m_open_mod,IDC_OPEN,m_hWnd);
}

void CInteraction::OnPickup() 
{
  ProcessButtonPressed(TCPickup,&m_pickup_mod,IDC_PICKUP,m_hWnd);
}

void CInteraction::OnWithdraw() 
{
  ProcessButtonPressed(TCWithdraw,&m_withdraw_mod,IDC_WITHDRAW,m_hWnd);
}


void CInteraction::OnAttackMod() 
{
  ProcessModifierPressed(TCAttack,IDC_ATTACK,m_hWnd);
}

void CInteraction::OnBuyMod() 
{
  ProcessModifierPressed(TCBuy,IDC_BUY,m_hWnd);
}

void CInteraction::OnDepositMod() 
{
  ProcessModifierPressed(TCDeposit,IDC_DEPOSIT,m_hWnd);
}

void CInteraction::OnExamineMod() 
{
  ProcessModifierPressed(TCExamine,IDC_EXAMINE,m_hWnd);
}

void CInteraction::OnLookMod() 
{
  ProcessModifierPressed(TCLook,IDC_LOOK,m_hWnd);
}

void CInteraction::OnOfferMod() 
{
  ProcessModifierPressed(TCOffer,IDC_OFFER,m_hWnd);
}

void CInteraction::OnOpenMod() 
{
  ProcessModifierPressed(TCOpen,IDC_OPEN,m_hWnd);
}

void CInteraction::OnPickupMod() 
{
  ProcessModifierPressed(TCPickup,IDC_PICKUP,m_hWnd);
}

void CInteraction::OnWithdrawMod() 
{
  ProcessModifierPressed(TCWithdraw,IDC_WITHDRAW,m_hWnd);
}
