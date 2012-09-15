// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// DibView.h : interface of the CDibView class
//
//////////
//
// Copyright (C) Ed Halley.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
//
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#ifndef __DIBVIEW_H__
#define __DIBVIEW_H__

/////////////////////////////////////////////////////////////////////////////

/* declare */ class CDib;

/////////////////////////////////////////////////////////////////////////////

class CDibView : public CScrollView
{
	DECLARE_DYNCREATE(CDibView)
protected:
	CDibView();

// Attributes
protected:
	CDib* m_pDib;
	BOOL m_bAutoPalette;
public:
	CDib* GetDib() const { return m_pDib; }
	void SetDib(CDib* pDib);
	//
	BOOL IsAutoChangePalette() const { return m_bAutoPalette; }
	void SetAutoChangePalette(BOOL bAuto = TRUE);

// Operations
public:

// Overrides
public:
	//{{AFX_VIRTUAL(CDibView)
	public:
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDibView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(CDibView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // __DIBVIEW_H__
