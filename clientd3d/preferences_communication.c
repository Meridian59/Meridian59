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
#include "resource.h"
#include "preferences_communication.h"
#include "preferences_assignkey.h"
#include "preferences_modifier.h"
#include "preferences_common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void ActivateIt(void);
/////////////////////////////////////////////////////////////////////////////
// CCommunicationPage property page

IMPLEMENT_DYNCREATE(CCommunicationPage, CPropertyPage)

CCommunicationPage::CCommunicationPage() : CPropertyPage(CCommunicationPage::IDD)
{
	//{{AFX_DATA_INIT(CCommunicationPage)
	//}}AFX_DATA_INIT
}

CCommunicationPage::~CCommunicationPage()
{
}

void CCommunicationPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommunicationPage)
	DDX_Control(pDX, IDC_YELL_MOD, m_yell_mod);
	DDX_Control(pDX, IDC_WHO_MOD, m_who_mod);
	DDX_Control(pDX, IDC_TELL_MOD, m_tell_mod);
	DDX_Control(pDX, IDC_SAY_MOD, m_say_mod);
	DDX_Control(pDX, IDC_EMOTE_MOD, m_emote_mod);
	DDX_Control(pDX, IDC_CHAT_MOD, m_chat_mod);
	DDX_Control(pDX, IDC_BROADCAST_MOD, m_broadcast_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommunicationPage, CPropertyPage)
	//{{AFX_MSG_MAP(CCommunicationPage)
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
// CCommunicationPage message handlers
void CCommunicationPage::UpdateDialogData()
{
	SetDlgItemText(IDC_BROADCAST, TCBroadcast);
	SetDlgItemText(IDC_CHAT, TCChat);
	SetDlgItemText(IDC_EMOTE, TCEmote);
	SetDlgItemText(IDC_SAY, TCSay);
	SetDlgItemText(IDC_TELL, TCTell);
	SetDlgItemText(IDC_WHO, TCWho);
	SetDlgItemText(IDC_YELL, TCYell);

	InitModifierButton(TCBroadcast, &m_broadcast_mod);
	InitModifierButton(TCBroadcast, &m_chat_mod);
	InitModifierButton(TCBroadcast, &m_emote_mod);
	InitModifierButton(TCBroadcast, &m_say_mod);
	InitModifierButton(TCBroadcast, &m_tell_mod);
	InitModifierButton(TCBroadcast, &m_who_mod);
	InitModifierButton(TCBroadcast, &m_yell_mod);
}

BOOL CCommunicationPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateDialogData();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCommunicationPage::ProcessButtonPressed(TCHAR* TCButtonstring, CButton* modifier, int nID, HWND m_hWnd)
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

void CCommunicationPage::ProcessModifierPressed(TCHAR* TCString, int nID, HWND m_hWnd)
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

void CCommunicationPage::OnBroadcast()
{
	ProcessButtonPressed(TCBroadcast, &m_broadcast_mod, IDC_BROADCAST, m_hWnd);
}

void CCommunicationPage::OnChat()
{
	ProcessButtonPressed(TCChat, &m_chat_mod, IDC_CHAT, m_hWnd);
}

void CCommunicationPage::OnEmote()
{
	ProcessButtonPressed(TCEmote, &m_emote_mod, IDC_EMOTE, m_hWnd);
}

void CCommunicationPage::OnSay()
{
	ProcessButtonPressed(TCSay, &m_say_mod, IDC_SAY, m_hWnd);
}

void CCommunicationPage::OnTell()
{
	ProcessButtonPressed(TCTell, &m_tell_mod, IDC_TELL, m_hWnd);
}

void CCommunicationPage::OnWho()
{
	ProcessButtonPressed(TCWho, &m_who_mod, IDC_WHO, m_hWnd);
}

void CCommunicationPage::OnYell()
{
	ProcessButtonPressed(TCYell, &m_yell_mod, IDC_YELL, m_hWnd);
}

void CCommunicationPage::OnBroadcastMod()
{
	ProcessModifierPressed(TCBroadcast, IDC_BROADCAST, m_hWnd);
}

void CCommunicationPage::OnChatMod()
{
	ProcessModifierPressed(TCChat, IDC_CHAT, m_hWnd);
}

void CCommunicationPage::OnEmoteMod()
{
	ProcessModifierPressed(TCEmote, IDC_EMOTE, m_hWnd);
}

void CCommunicationPage::OnSayMod()
{
	ProcessModifierPressed(TCSay, IDC_SAY, m_hWnd);
}

void CCommunicationPage::OnTellMod()
{
	ProcessModifierPressed(TCTell, IDC_TELL, m_hWnd);
}

void CCommunicationPage::OnWhoMod()
{
	ProcessModifierPressed(TCWho, IDC_WHO, m_hWnd);
}

void CCommunicationPage::OnYellMod()
{
	ProcessModifierPressed(TCYell, IDC_YELL, m_hWnd);
}

