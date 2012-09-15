// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// BufferedView.cpp : implementation of the CBufferedView class
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
//   A standard CBUFFEREDVIEW_BASE which draws to an offscreen bitmap before
//   slapping the completed results to the screen.
//
//////////
//

#include "stdafx.h"

#include "BufferedView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBufferedView, CBUFFEREDVIEW_BASE)

BEGIN_MESSAGE_MAP(CBufferedView, CBUFFEREDVIEW_BASE)
	//{{AFX_MSG_MAP(CBufferedView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/* static */ CBitmap CBufferedView::sm_bmpOffscreen;
/* static */ CSize CBufferedView::sm_szBmpOffscreen;

/////////////////////////////////////////////////////////////////////////////

CBufferedView::CBufferedView()
{
}

CBufferedView::~CBufferedView()
{
}

void CBufferedView::OnInitialUpdate() 
{
	if (!sm_bmpOffscreen.m_hObject)
	{
		CWindowDC dcScreen(CWnd::GetDesktopWindow());
		sm_szBmpOffscreen = CSize(10, 10);
		sm_bmpOffscreen.CreateCompatibleBitmap(&dcScreen,
			sm_szBmpOffscreen.cx, sm_szBmpOffscreen.cy);
	}

	CBUFFEREDVIEW_BASE::OnInitialUpdate();
}

/////////////////////////////////////////////////////////////////////////////

void CBufferedView::OnDraw(CDC* pDC)
{
	// If we are not printing, we want the draw code to access a
	// memory display context instead of the screen directly.
	// When it is finished, we flip the complete image to the screen.
	//
	// Note: all CBufferedView objects share one offscreen bitmap.
	// This bitmap is reallocated as necessary to be as small and
	// as efficient as possible.
	//
	// If the given CDC is printing, the buffered OnBufferedDraw() is
	// called without any offscreen buffering.

	CDC* pDrawDC = pDC;
	CBitmap* pbmpStock = NULL;
	CDC dcOffscreen;

	CRect rcDocArea;
	if (!pDC->IsPrinting())
	{
		// Make the offscreen DC.
		//
		CWindowDC dcScreen(CWnd::GetDesktopWindow());
		dcOffscreen.CreateCompatibleDC(&dcScreen);
		pDrawDC = &dcOffscreen;

		// Determine the drawing area (the visible portion of the
		// scrolling area).  If the shared offscreen bitmap is not
		// large enough, resize it.
		//
		int iMapMode;
		CSize szScroll, szPage, szLine;
		GetDeviceScrollSizes(iMapMode, szScroll, szPage, szLine);

		GetClientRect(&rcDocArea);
		CPoint ptScrollPos = GetScrollPosition();
		rcDocArea.right = min(rcDocArea.right, szScroll.cx-ptScrollPos.x);
		rcDocArea.bottom = min(rcDocArea.bottom, szScroll.cy-ptScrollPos.y);

		if (rcDocArea.Width()  > sm_szBmpOffscreen.cx ||
		    rcDocArea.Height() > sm_szBmpOffscreen.cy)
		{
			sm_bmpOffscreen.DeleteObject();
			sm_bmpOffscreen.CreateCompatibleBitmap(&dcScreen,
				rcDocArea.Width(), rcDocArea.Height());
			sm_szBmpOffscreen = rcDocArea.Size();
		}

		// Connect the offscreen bitmap to the offscreen context, fill it
		// as if it received a WM_ERASEBKGND beforehand, and map the
		// coordinates in the same fashion as the given DC.
		//
		pbmpStock = dcOffscreen.SelectObject(&sm_bmpOffscreen);
		dcOffscreen.FillSolidRect(&rcDocArea, ::GetSysColor(COLOR_WINDOW));
		dcOffscreen.SetViewportOrg(-ptScrollPos);
	}

	// Allow the derived class to draw into our buffer
	// (or directly, for printing contexts).
	//
	OnBufferedDraw(pDrawDC);

	if (!pDC->IsPrinting())
	{
		// Flip the buffered image to the screen.
		//
		CRect rcClip;
		pDC->GetClipBox(&rcClip);
		rcDocArea.OffsetRect(rcClip.TopLeft());
		rcClip.IntersectRect(&rcClip, &rcDocArea);
		pDC->BitBlt(rcClip.left, rcClip.top,
		            rcClip.Width(), rcClip.Height(),
		            &dcOffscreen, rcClip.left, rcClip.top, SRCCOPY);

		// Disconnect the offscreen bitmap in case it must be resized/reused.
		//
		dcOffscreen.SelectObject(pbmpStock);
	}
}

void CBufferedView::OnBufferedDraw(CDC* pDC)
{
	// Override this function instead of the normal void OnDraw(CDC* pDC).
}

BOOL CBufferedView::OnEraseBkgnd(CDC* pDC) 
{
	// Since we always draw the whole visible scrollable region, we
	// don't need to erase our background.
	//
	// Erase the area outside the scrollable region, instead.
	//
	CBrush brWorkspace;
	brWorkspace.CreateSolidBrush(::GetSysColor(COLOR_APPWORKSPACE));
	FillOutsideRect(pDC, &brWorkspace);

	return TRUE; // Yes, we erased the background.
}

/////////////////////////////////////////////////////////////////////////////

void CBufferedView::OnSize(UINT nType, int cx, int cy) 
{
	// If we have an offscreen bitmap, and this sizing of the window
	// would only require a portion of it, then we shrink it down to the
	// new size.
	//
	// Note: since multiple CBufferedView windows share this bitmap, the
	// next window to be drawn may just resize it back up.  The largest
	// view will tend to win this 'tug of war'.  This method prevents the
	// bitmap from being wasteful by being larger than the biggest window.
	//
	if (sm_bmpOffscreen.m_hObject)
	{
		CRect rcClient;
		GetClientRect(&rcClient);

		if (rcClient.Width() > 0 && rcClient.Height() > 0)
		{
			// Only shrink if both axes do.
			// (If one shrinks and one grows, the OnDraw will resize.)
			//
			if (rcClient.Width() < sm_szBmpOffscreen.cx &&
			    rcClient.Height() < sm_szBmpOffscreen.cy)
			{
				CWindowDC dcScreen(CWnd::GetDesktopWindow());

				sm_bmpOffscreen.DeleteObject();
				sm_bmpOffscreen.CreateCompatibleBitmap(&dcScreen,
					rcClient.Width(), rcClient.Height());
				sm_szBmpOffscreen = rcClient.Size();
			}
		}
	}

	CBUFFEREDVIEW_BASE::OnSize(nType, cx, cy);
}

