// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#if !defined(AFX_KEYBINDSPS_H__CD73D767_8203_461F_9533_03B6B3A01952__INCLUDED_)
#define AFX_KEYBINDSPS_H__CD73D767_8203_461F_9533_03B6B3A01952__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Keybindsps.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeybindsps

class CKeybindsps : public CPropertySheet
{
	DECLARE_DYNAMIC(CKeybindsps)

// Construction
public:
	CKeybindsps(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CKeybindsps(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeybindsps)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CKeybindsps();

	// Generated message map functions
protected:
	//{{AFX_MSG(CKeybindsps)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYBINDSPS_H__CD73D767_8203_461F_9533_03B6B3A01952__INCLUDED_)
