// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// DibView.cpp : implementation of the CDibView class
//
//////////
//
// Copyright (C) Mindscape, Inc.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Mindscape, Inc.
//
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#include "StdAfx.h"

#include "DibView.h"

#include "PaletteApp.h"
#include "Dib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CDibView::CDibView()
{
	m_pDib = NULL;
	m_bAutoPalette = TRUE;
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////

void CDibView::SetDib(CDib* pDib)
{
	ASSERT_DYNAMIC_OR_NULL(pDib, CDib);
	m_pDib = pDib;

	if (m_hWnd)
	{
		CSize szScroll = CSize(10, 10);
		if (m_pDib && m_pDib->m_pInfo)
		{
			szScroll = pDib->GetSize();
			if (m_bAutoPalette)
				AfxGetPaletteApp()->SetAppPalette(m_pDib);
		}

		SetScrollSizes(MM_TEXT, szScroll);
	}
}

void CDibView::SetAutoChangePalette(BOOL bAuto /* = TRUE */)
{
	m_bAutoPalette = bAuto;
	if (!bAuto || !m_hWnd || !m_pDib)
		return;

	AfxGetPaletteApp()->SetAppPalette(m_pDib);
}

/////////////////////////////////////////////////////////////////////////////

void CDibView::OnDraw(CDC* pDC)
{
	if (m_pDib && m_pDib->m_pInfo)
	{
		ASSERT_DYNAMIC(m_pDib, CDib);

		AfxGetPaletteApp()->RealizeAppPalette(pDC);
		CRect rcDib;
		m_pDib->GetRect(&rcDib);
		m_pDib->DibBlt(pDC,
		               rcDib,
		               CPoint(0, 0),
		               SRCCOPY);
	}
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CDibView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDibView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CDibView::OnActivateView(BOOL bActivate,
                              CView* pActivateView,
                              CView* pDeactiveView) 
{
	if (bActivate)
	{
		if (m_pDib && m_pDib->m_pInfo)
		{
 			ASSERT_DYNAMIC(m_pDib, CDib);
			if (m_bAutoPalette)
				AfxGetPaletteApp()->SetAppPalette(m_pDib);
		}
	}
	InvalidateRect(NULL, FALSE);

	if (pActivateView || pDeactiveView)
		CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CDibView::OnInitialUpdate() 
{
	// This will refresh any scroll ranges, etc.
	// which we base off the dib that may already be loaded.
	//
	CDib* pDib = m_pDib; m_pDib = NULL;
	SetDib(pDib);

	CScrollView::OnInitialUpdate();
}

BOOL CDibView::OnEraseBkgnd(CDC* pDC) 
{
	if (!m_pDib || !m_pDib->m_pInfo)
		return CScrollView::OnEraseBkgnd(pDC);

	CBrush br(::GetSysColor(COLOR_APPWORKSPACE));
	FillOutsideRect(pDC, &br);

	return TRUE; // yes, we painted all that needs painting
}
