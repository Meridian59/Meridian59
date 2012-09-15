// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_MOUSE_H__02372E34_BE0B_4871_8168_4E66A3E9981B__INCLUDED_)
#define AFX_MOUSE_H__02372E34_BE0B_4871_8168_4E66A3E9981B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Mouse.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMouse dialog

class CMouse : public CPropertyPage
{
	DECLARE_DYNCREATE(CMouse)

// Construction
public:
	CMouse();
	~CMouse();

  void UpdateDialogData();

// Dialog Data
	//{{AFX_DATA(CMouse)
	enum { IDD = IDD_MOUSE };
	CSliderCtrl	m_mouseyscale;
	CSliderCtrl	m_mousexscale;
	BOOL	m_invert;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMouse)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMouse)
	virtual BOOL OnInitDialog();
	afx_msg void OnInvert();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOUSE_H__02372E34_BE0B_4871_8168_4E66A3E9981B__INCLUDED_)
