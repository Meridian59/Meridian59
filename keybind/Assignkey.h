// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_ASSIGNKEY_H__77A87ED4_4400_4D3A_B5E9_6B74FF8AC707__INCLUDED_)
#define AFX_ASSIGNKEY_H__77A87ED4_4400_4D3A_B5E9_6B74FF8AC707__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Assignkey.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAssignkey dialog

class CAssignkey : public CDialog
{
// Construction
public:
	CAssignkey(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAssignkey)
	enum { IDD = IDD_ASSIGNKEY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAssignkey)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAssignkey)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASSIGNKEY_H__77A87ED4_4400_4D3A_B5E9_6B74FF8AC707__INCLUDED_)
