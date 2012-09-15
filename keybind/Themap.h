// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_THEMAP_H__2A84949B_5C39_4ACE_A4CD_1836935075EF__INCLUDED_)
#define AFX_THEMAP_H__2A84949B_5C39_4ACE_A4CD_1836935075EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Themap.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThemap dialog

class CThemap : public CPropertyPage
{
	DECLARE_DYNCREATE(CThemap)

// Construction
public:
	CThemap();
	~CThemap();

  void UpdateDialogData();

// Dialog Data
	//{{AFX_DATA(CThemap)
	enum { IDD = IDD_MAP };
	CButton	m_mapzoomout_mod;
	CButton	m_mapzoomin_mod;
	CButton	m_map_mod;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CThemap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CThemap)
	virtual BOOL OnInitDialog();
	afx_msg void OnMap();
	afx_msg void OnMapzoomin();
	afx_msg void OnMapzoomout();
	afx_msg void OnMapMod();
	afx_msg void OnMapzoominMod();
	afx_msg void OnMapzoomoutMod();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THEMAP_H__2A84949B_5C39_4ACE_A4CD_1836935075EF__INCLUDED_)
