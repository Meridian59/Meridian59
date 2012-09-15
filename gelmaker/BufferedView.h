// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// BufferedView.h : interface of the CBufferedView class
//
//////////
//
// Copyright (C) Ed Halley, Inc.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//
// CBufferedView:
//   A standard CScrollView which draws to an offscreen bitmap before
//   slapping the completed results to the screen.
//
//////////
//

#ifndef __BUFFEREDVIEW_H__
#define __BUFFEREDVIEW_H__

/////////////////////////////////////////////////////////////////////////////

// Define CBUFFEREDVIEW_BASE to CScrollView.
// Override this default definition by adding a #define to your stdafx.h.
//
#ifndef CBUFFEREDVIEW_BASE
#define CBUFFEREDVIEW_BASE CScrollView
#endif

//////////

#ifndef __AFXEDH_H__
#error Must include AfxEdh.h (in your stdafx.h) to support this module.
#endif

/////////////////////////////////////////////////////////////////////////////

//
//////////
//
// Using a CBufferedView:
//
// * Just like a CScrollView.
//   Set scroll sizes in your override of void OnInitialUpdate().
//
// * Override void OnBufferedDraw(CDC* pDC) instead of void OnDraw(CDC* pDC).
//   When drawing, do not rely on any clip region or clip box information.
//   The buffer in the display context has been filled with COLOR_WINDOW.
//
// * No palette-management is done.
//   Your class must set and realize a palette if desired.
//
// * A simple BOOL OnEraseBkgnd(CDC* pDC) is provided.
//   It will fill areas outside scrolling region with COLOR_APPWORKSPACE.
//
//////////
//

/////////////////////////////////////////////////////////////////////////////

class CBufferedView : public CBUFFEREDVIEW_BASE
{
	DECLARE_DYNCREATE(CBufferedView)
protected:
	CBufferedView();

// Attributes
public:

// Overridables
public:
	virtual void OnBufferedDraw(CDC* pDC);

// Overrides
public:
	//{{AFX_VIRTUAL(CBufferedView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	static CBitmap sm_bmpOffscreen;
	static CSize sm_szBmpOffscreen;
public:
	virtual ~CBufferedView();

protected:
	//{{AFX_MSG(CBufferedView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // __BUFFEREDVIEW_H__
