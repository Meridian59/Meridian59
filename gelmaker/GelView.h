// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelView.h : interface of the CGelView class
//
//////////
//
// Copyright (C) The 3DO Company
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of The 3DO Company.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

/////////////////////////////////////////////////////////////////////////////

#include "BufferedView.h"

class CGelView : public CBufferedView
{
	DECLARE_DYNCREATE(CGelView)
protected:
	CGelView();

// Attributes
public:
	class CGelDoc* GetDocument();
	BYTE m_iFocus;
	HICON m_hiSelSwatch;

// Operations
public:
	int IndexFromPoint(CPoint point);
	void InvalidateIndex(int i);

// Overrides
public:
	virtual void OnBufferedDraw(CDC* pDC);
	//{{AFX_VIRTUAL(CGelView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGelView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	afx_msg LRESULT OnGelEyedropper(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
