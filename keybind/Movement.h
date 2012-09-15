// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_MOVEMENT_H__364A6737_BBE6_45AC_8F74_C909B692D2DB__INCLUDED_)
#define AFX_MOVEMENT_H__364A6737_BBE6_45AC_8F74_C909B692D2DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Movement.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMovement dialog

class CMovement : public CPropertyPage
{
	DECLARE_DYNCREATE(CMovement)

// Construction
public:
	CMovement();
	~CMovement();

  void UpdateDialogData();

// Dialog Data
	//{{AFX_DATA(CMovement)
	enum { IDD = IDD_MOVEMENT };
	CButton	m_slideright_mod;
	CButton	m_slideleft_mod;
	CButton	m_runwalk_mod;
	CButton	m_right_mod;
	CButton	m_mouselooktoggle_mod;
	CButton	m_lookup_mod;
	CButton	m_lookstraight_mod;
	CButton	m_lookdown_mod;
	CButton	m_left_mod;
	CButton	m_forward_mod;
	CButton	m_flip_mod;
	CButton	m_backward_mod;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMovement)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMovement)
	virtual BOOL OnInitDialog();
	afx_msg void OnBackward();
	afx_msg void OnFlip();
	afx_msg void OnForward();
	afx_msg void OnLeft();
	afx_msg void OnLookdown();
	afx_msg void OnLookstraight();
	afx_msg void OnLookup();
	afx_msg void OnMouselooktoggle();
	afx_msg void OnRight();
	afx_msg void OnRunwalk();
	afx_msg void OnSlideleft();
	afx_msg void OnSlideright();
	afx_msg void OnBackwardMod();
	afx_msg void OnFlipMod();
	afx_msg void OnForwardMod();
	afx_msg void OnLeftMod();
	afx_msg void OnLookdownMod();
	afx_msg void OnLookstraightMod();
	afx_msg void OnLookupMod();
	afx_msg void OnMouselooktoggleMod();
	afx_msg void OnRightMod();
	afx_msg void OnRunwalkMod();
	afx_msg void OnSlideleftMod();
	afx_msg void OnSliderightMod();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVEMENT_H__364A6737_BBE6_45AC_8F74_C909B692D2DB__INCLUDED_)
