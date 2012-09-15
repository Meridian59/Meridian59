// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Communication.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Communication.h"
#include "m59bindDlg.h"
#include "Assignkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void ActivateIt(void);
/////////////////////////////////////////////////////////////////////////////
// CCommunication property page

IMPLEMENT_DYNCREATE(CCommunication, CPropertyPage)

CCommunication::CCommunication() : CPropertyPage(CCommunication::IDD)
{
	//{{AFX_DATA_INIT(CCommunication)
	//}}AFX_DATA_INIT
}

CCommunication::~CCommunication()
{
}

void CCommunication::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommunication)
	DDX_Control(pDX, IDC_YELL_MOD, m_yell_mod);
	DDX_Control(pDX, IDC_WHO_MOD, m_who_mod);
	DDX_Control(pDX, IDC_TELL_MOD, m_tell_mod);
	DDX_Control(pDX, IDC_SAY_MOD, m_say_mod);
	DDX_Control(pDX, IDC_EMOTE_MOD, m_emote_mod);
	DDX_Control(pDX, IDC_CHAT_MOD, m_chat_mod);
	DDX_Control(pDX, IDC_BROADCAST_MOD, m_broadcast_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommunication, CPropertyPage)
	//{{AFX_MSG_MAP(CCommunication)
	ON_BN_CLICKED(IDC_BROADCAST, OnBroadcast)
	ON_BN_CLICKED(IDC_CHAT, OnChat)
	ON_BN_CLICKED(IDC_EMOTE, OnEmote)
	ON_BN_CLICKED(IDC_SAY, OnSay)
	ON_BN_CLICKED(IDC_TELL, OnTell)
	ON_BN_CLICKED(IDC_WHO, OnWho)
	ON_BN_CLICKED(IDC_YELL, OnYell)
	ON_BN_CLICKED(IDC_BROADCAST_MOD, OnBroadcastMod)
	ON_BN_CLICKED(IDC_CHAT_MOD, OnChatMod)
	ON_BN_CLICKED(IDC_EMOTE_MOD, OnEmoteMod)
	ON_BN_CLICKED(IDC_SAY_MOD, OnSayMod)
	ON_BN_CLICKED(IDC_TELL_MOD, OnTellMod)
	ON_BN_CLICKED(IDC_WHO_MOD, OnWhoMod)
	ON_BN_CLICKED(IDC_YELL_MOD, OnYellMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommunication message handlers
void CCommunication::UpdateDialogData()
{
  SetDlgItemText(IDC_BROADCAST,TCBroadcast);
  SetDlgItemText(IDC_CHAT,TCChat);
  SetDlgItemText(IDC_EMOTE,TCEmote);
  SetDlgItemText(IDC_SAY,TCSay);
  SetDlgItemText(IDC_TELL,TCTell);
  SetDlgItemText(IDC_WHO,TCWho);
  SetDlgItemText(IDC_YELL,TCYell);

  InitModifierButton(TCBroadcast,&m_broadcast_mod);
  InitModifierButton(TCBroadcast,&m_chat_mod);
  InitModifierButton(TCBroadcast,&m_emote_mod);
  InitModifierButton(TCBroadcast,&m_say_mod);
  InitModifierButton(TCBroadcast,&m_tell_mod);
  InitModifierButton(TCBroadcast,&m_who_mod);
  InitModifierButton(TCBroadcast,&m_yell_mod);
}


BOOL CCommunication::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  UpdateDialogData();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCommunication::OnBroadcast() 
{
  ProcessButtonPressed(TCBroadcast,&m_broadcast_mod,IDC_BROADCAST,m_hWnd);
}

void CCommunication::OnChat() 
{
  ProcessButtonPressed(TCChat,&m_chat_mod,IDC_CHAT,m_hWnd);
}

void CCommunication::OnEmote() 
{
  ProcessButtonPressed(TCEmote,&m_emote_mod,IDC_EMOTE,m_hWnd);
}

void CCommunication::OnSay() 
{
  ProcessButtonPressed(TCSay,&m_say_mod,IDC_SAY,m_hWnd);
}

void CCommunication::OnTell() 
{
  ProcessButtonPressed(TCTell,&m_tell_mod,IDC_TELL,m_hWnd);
}

void CCommunication::OnWho() 
{
  ProcessButtonPressed(TCWho,&m_who_mod,IDC_WHO,m_hWnd);
}

void CCommunication::OnYell() 
{
  ProcessButtonPressed(TCYell,&m_yell_mod,IDC_YELL,m_hWnd);
}


void CCommunication::OnBroadcastMod() 
{
  ProcessModifierPressed(TCBroadcast,IDC_BROADCAST,m_hWnd);
}

void CCommunication::OnChatMod() 
{
  ProcessModifierPressed(TCChat,IDC_CHAT,m_hWnd);
}

void CCommunication::OnEmoteMod() 
{
  ProcessModifierPressed(TCEmote,IDC_EMOTE,m_hWnd);
}

void CCommunication::OnSayMod() 
{
  ProcessModifierPressed(TCSay,IDC_SAY,m_hWnd);
}

void CCommunication::OnTellMod() 
{
  ProcessModifierPressed(TCTell,IDC_TELL,m_hWnd);
}

void CCommunication::OnWhoMod() 
{
  ProcessModifierPressed(TCWho,IDC_WHO,m_hWnd);
}

void CCommunication::OnYellMod() 
{
  ProcessModifierPressed(TCYell,IDC_YELL,m_hWnd);
}

