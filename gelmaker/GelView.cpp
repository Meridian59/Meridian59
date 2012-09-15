// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelView.cpp : implementation of the CGelView class
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

#include "StdAfx.h"
#include "GelMaker.h"

#include "PaletteApp.h"
#include "GelDoc.h"
#include "GelView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGelView, CBufferedView)

BEGIN_MESSAGE_MAP(CGelView, CBufferedView)
	//{{AFX_MSG_MAP(CGelView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(GM_EYEDROPPER, OnGelEyedropper)
END_MESSAGE_MAP()

#define CXSWATCH 30
#define CYSWATCH  8
#define CXMARGIN 20
#define CYMARGIN 20
#define CXGUTTER 10

/////////////////////////////////////////////////////////////////////////////

CGelView::CGelView()
{
	m_iFocus = 0;
	m_hiSelSwatch = AfxGetApp()->LoadIcon(IDI_SELSWATCH);
}

CGelView::~CGelView()
{
}

/////////////////////////////////////////////////////////////////////////////

CGelDoc* CGelView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGelDoc)));
	return (CGelDoc*)m_pDocument;
}

BOOL CGelView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CBufferedView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////

void CGelView::OnBufferedDraw(CDC* pDC)
{
	CGelDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	AfxGetPaletteApp()->RealizeAppPalette(pDC);

	CRect rcLeft = CRect(CPoint(CXMARGIN, CYMARGIN), CSize(CXSWATCH, CYSWATCH));
	CRect rcRight = rcLeft;
	rcRight.OffsetRect(CXSWATCH + CXGUTTER, 0);

	CRect rcFocus = rcLeft;
	rcFocus.OffsetRect(0, CYSWATCH * m_iFocus);
	rcFocus.InflateRect(1, 0);
	pDC->DrawFocusRect(rcFocus);
	rcFocus.InflateRect(1, 0);
	pDC->DrawFocusRect(rcFocus);

	int i;
	for (i = 0; i < 256; i++)
	{
		pDC->FillSolidRect(rcLeft, PALETTEINDEX(i));
		pDC->FillSolidRect(rcRight, PALETTEINDEX(pDoc->m_abyTransform[i]));

		if (pDoc->m_abySelection[i])
			pDC->DrawIcon(rcLeft.TopLeft() + CSize(0, CYSWATCH/2) - CSize(10, 4),
				m_hiSelSwatch);

		rcLeft.OffsetRect(0, CYSWATCH);
		rcRight.OffsetRect(0, CYSWATCH);
	}
}

void CGelView::OnInitialUpdate()
{
	CBufferedView::OnInitialUpdate();
	CSize sizeTotal;
	sizeTotal.cx = CXSWATCH * 2 + CXMARGIN * 2 + CXGUTTER;
	sizeTotal.cy = CYSWATCH * 256 + CYMARGIN * 2;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CGelView::AssertValid() const
{
	CBufferedView::AssertValid();
}

void CGelView::Dump(CDumpContext& dc) const
{
	CBufferedView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

int CGelView::IndexFromPoint(CPoint point)
{
	if (point.y < CYMARGIN || point.y > (CYSWATCH * 256 + CYMARGIN))
		return -1;

	return (point.y - CYMARGIN) / CYSWATCH;
}

void CGelView::InvalidateIndex(int i)
{
	ASSERT(i >= 0 && i <= 255);

	CSize szTotal = GetTotalSize();
	CRect rcItem = CRect(CPoint(0, CYMARGIN), CSize(szTotal.cx, CYSWATCH));
	rcItem.OffsetRect(0, CYSWATCH * i);
	rcItem.OffsetRect(-GetScrollPosition());

	InvalidateRect(rcItem, TRUE);
}

/////////////////////////////////////////////////////////////////////////////

void CGelView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CBufferedView::OnLButtonDown(nFlags, point);
	CGelDoc* pDoc = GetDocument();
	if (!pDoc)
		return;
	ASSERT_VALID(pDoc);

	SetCapture();

	point += GetScrollPosition();

	int i = IndexFromPoint(point);
	if (i < 0)
		return;

	ASSERT(i >= 0 && i <= 255);

	if (nFlags & MK_CONTROL)
	{
		// Controlled click inverts i's select.
		//
		pDoc->m_abySelection[i] = ~pDoc->m_abySelection[i];
		pDoc->ApplySelection();
	}
	else if (nFlags & MK_SHIFT)
	{
		// Shifted click selects/deselects all items between focus and i.
		// New selection state is opposite that of clicked item.
		// If user wanted other state, just shift-click the old focused item
		// one more time.
		//
		int j;
		BYTE bSel = !pDoc->m_abySelection[i];
		for (j = min(i, m_iFocus); j <= max(i, m_iFocus); j++)
		{
			pDoc->m_abySelection[j] = bSel;
		}
		pDoc->ApplySelection();
	}
	else
	{
		// Unshifted, uncontrolled click *should* deselect all but i.
		// However, this is rarely useful in this application where complex
		// selection sets may be used for many operations in a row.
		// So we just set the focus to i and leave the selection alone.
		// There's a "select none" menu, toolbar choice for those rare cases.
	}

	if (i != m_iFocus)
	{
		InvalidateIndex(m_iFocus);
		m_iFocus = i;
		InvalidateIndex(i);
	}
}

void CGelView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CBufferedView::OnLButtonUp(nFlags, point);
	CGelDoc* pDoc = GetDocument();
	if (!pDoc)
		return;
	ASSERT_VALID(pDoc);

	point += GetScrollPosition();

	if (CWnd::GetCapture() == this)
		ReleaseCapture();
}

void CGelView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBufferedView::OnMouseMove(nFlags, point);
	CGelDoc* pDoc = GetDocument();
	if (!pDoc)
		return;
	ASSERT_VALID(pDoc);

	point += GetScrollPosition();

	if ((nFlags & MK_LBUTTON) && CWnd::GetCapture() == this)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CGelView::OnGelEyedropper(WPARAM wParam, LPARAM lParam)
{
	CGelDoc* pDoc = GetDocument();
	if (!pDoc)
		return 0;
	ASSERT_VALID(pDoc);

	UINT nFlags = (UINT)wParam; // key flags
	BYTE byPixel = (BYTE)lParam; // eyedroppered pixel
	int i = byPixel;

	if (nFlags & MK_LBUTTON)
	{
		if (nFlags & MK_CONTROL)
		{
			// Controlled click removes i's select.
			//
			pDoc->m_abySelection[i] = 0x00;
			pDoc->ApplySelection();
		}
		else if (nFlags & MK_SHIFT)
		{
			// Shifted click adds i's select.
			//
			pDoc->m_abySelection[i] = 0xFF;
			pDoc->ApplySelection();
		}
	}

	if (i != m_iFocus)
	{
		InvalidateIndex(m_iFocus);
		m_iFocus = i;
		InvalidateIndex(i);
	}

	return 0L;
}

/////////////////////////////////////////////////////////////////////////////

void CGelView::OnCancelMode() 
{
	CBufferedView::OnCancelMode();
	if (CWnd::GetCapture() == this)
		ReleaseCapture();
}
