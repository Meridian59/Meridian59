// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_INTERACTION_H__EAE0BBA6_2D63_407F_BF38_DE07F5535BC5__INCLUDED_)
#define AFX_INTERACTION_H__EAE0BBA6_2D63_407F_BF38_DE07F5535BC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Interaction.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInteraction dialog

class CInteraction : public CPropertyPage
{
	DECLARE_DYNCREATE(CInteraction)

// Construction
public:
	CInteraction();
	~CInteraction();

  void UpdateDialogData();

// Dialog Data
	//{{AFX_DATA(CInteraction)
	enum { IDD = IDD_INTERACTION };
	CButton	m_withdraw_mod;
	CButton	m_pickup_mod;
	CButton	m_open_mod;
	CButton	m_offer_mod;
	CButton	m_look_mod;
	CButton	m_examine_mod;
	CButton	m_deposit_mod;
	CButton	m_buy_mod;
	CButton	m_attack_mod;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CInteraction)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CInteraction)
	virtual BOOL OnInitDialog();
	afx_msg void OnAttack();
	afx_msg void OnBuy();
	afx_msg void OnDeposit();
	afx_msg void OnExamine();
	afx_msg void OnLook();
	afx_msg void OnOffer();
	afx_msg void OnOpen();
	afx_msg void OnPickup();
	afx_msg void OnWithdraw();
	afx_msg void OnAttackMod();
	afx_msg void OnBuyMod();
	afx_msg void OnDepositMod();
	afx_msg void OnExamineMod();
	afx_msg void OnLookMod();
	afx_msg void OnOfferMod();
	afx_msg void OnOpenMod();
	afx_msg void OnPickupMod();
	afx_msg void OnWithdrawMod();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTERACTION_H__EAE0BBA6_2D63_407F_BF38_DE07F5535BC5__INCLUDED_)
