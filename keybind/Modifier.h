// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_MODIFIER_H__550B9C1D_51CA_4F2A_A3D1_1C10E267E101__INCLUDED_)
#define AFX_MODIFIER_H__550B9C1D_51CA_4F2A_A3D1_1C10E267E101__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Modifier.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModifier dialog

class CModifier : public CDialog
{
// Construction
public:
	CModifier(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifier)
	enum { IDD = IDD_MODIFIER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifier)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifier)
	afx_msg void OnAlt();
	afx_msg void OnAny();
	afx_msg void OnCtrl();
	afx_msg void OnShift();
	afx_msg void OnNone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFIER_H__550B9C1D_51CA_4F2A_A3D1_1C10E267E101__INCLUDED_)
