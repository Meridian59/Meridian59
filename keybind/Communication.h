// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_COMMUNICATION_H__705EF6E9_C0D7_46F8_A8BC_180EB8C8B4EC__INCLUDED_)
#define AFX_COMMUNICATION_H__705EF6E9_C0D7_46F8_A8BC_180EB8C8B4EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Communication.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommunication dialog

class CCommunication : public CPropertyPage
{
	DECLARE_DYNCREATE(CCommunication)

// Construction
public:
	CCommunication();
	~CCommunication();

  void UpdateDialogData();

// Dialog Data
	//{{AFX_DATA(CCommunication)
	enum { IDD = IDD_COMMUNICATION };
	CButton	m_yell_mod;
	CButton	m_who_mod;
	CButton	m_tell_mod;
	CButton	m_say_mod;
	CButton	m_emote_mod;
	CButton	m_chat_mod;
	CButton	m_broadcast_mod;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCommunication)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCommunication)
	virtual BOOL OnInitDialog();
	afx_msg void OnBroadcast();
	afx_msg void OnChat();
	afx_msg void OnEmote();
	afx_msg void OnSay();
	afx_msg void OnTell();
	afx_msg void OnWho();
	afx_msg void OnYell();
	afx_msg void OnBroadcastMod();
	afx_msg void OnChatMod();
	afx_msg void OnEmoteMod();
	afx_msg void OnSayMod();
	afx_msg void OnTellMod();
	afx_msg void OnWhoMod();
	afx_msg void OnYellMod();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMUNICATION_H__705EF6E9_C0D7_46F8_A8BC_180EB8C8B4EC__INCLUDED_)
