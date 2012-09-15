// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_TARGETING_H__F08BB69E_2733_4863_8DD5_D003DC1C1C18__INCLUDED_)
#define AFX_TARGETING_H__F08BB69E_2733_4863_8DD5_D003DC1C1C18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Targeting.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTargeting dialog

class CTargeting : public CPropertyPage
{
	DECLARE_DYNCREATE(CTargeting)

// Construction
public:
	CTargeting();
	~CTargeting();

  void UpdateDialogData();

// Dialog Data
	//{{AFX_DATA(CTargeting)
	enum { IDD = IDD_TARGETING };
	CButton	m_targetself_mod;
	CButton	m_targetprevious_mod;
	CButton	m_targetnext_mod;
	CButton	m_targetclear_mod;
	CButton	m_tabforward_mod;
	CButton	m_tabbackward_mod;
	CButton	m_selecttarget_mod;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTargeting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTargeting)
	virtual BOOL OnInitDialog();
	afx_msg void OnTabbackward();
	afx_msg void OnTabforward();
	afx_msg void OnTargetclear();
	afx_msg void OnTargetnext();
	afx_msg void OnTargetprevious();
	afx_msg void OnTargetself();
	afx_msg void OnSelecttarget();
	afx_msg void OnSelecttargetMod();
	afx_msg void OnTabbackwardMod();
	afx_msg void OnTabforwardMod();
	afx_msg void OnTargetclearMod();
	afx_msg void OnTargetnextMod();
	afx_msg void OnTargetpreviousMod();
	afx_msg void OnTargetselfMod();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TARGETING_H__F08BB69E_2733_4863_8DD5_D003DC1C1C18__INCLUDED_)
