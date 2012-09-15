// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelPreview.cpp : implementation file
//

#include "stdafx.h"
#include "GelMaker.h"

#include "GelPreview.h"
#include "Dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGelPreview, CDibView)

BEGIN_MESSAGE_MAP(CGelPreview, CDibView)
	//{{AFX_MSG_MAP(CGelPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CGelPreview::CGelPreview()
{
}

CGelPreview::~CGelPreview()
{
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CGelPreview::AssertValid() const
{
	CDibView::AssertValid();
}

void CGelPreview::Dump(CDumpContext& dc) const
{
	CDibView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CGelPreview::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CDibView::OnLButtonDown(nFlags, point);
	if (!m_pDib)
		return;

	point += GetScrollPosition();
	if (point.x < 0)
		point.x = 0;
	if (point.y < 0)
		point.y = 0;
	if (point.x >= m_pDib->GetSize().cx)
		point.x = m_pDib->GetSize().cx-1;
	if (point.y >= m_pDib->GetSize().cy)
		point.y = m_pDib->GetSize().cy-1;
	BYTE pixel = m_pDib->GetPixel(point);

	AfxGetMainWnd()->SendMessageToDescendants(GM_EYEDROPPER,
		(WPARAM)nFlags, (LPARAM)pixel);
}

void CGelPreview::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDibView::OnMouseMove(nFlags, point);
	if (!m_pDib)
		return;

	point += GetScrollPosition();
	if (point.x < 0)
		point.x = 0;
	if (point.y < 0)
		point.y = 0;
	if (point.x >= m_pDib->GetSize().cx)
		point.x = m_pDib->GetSize().cx-1;
	if (point.y >= m_pDib->GetSize().cy)
		point.y = m_pDib->GetSize().cy-1;
	BYTE pixel = m_pDib->GetPixel(point);

	AfxGetMainWnd()->SendMessageToDescendants(GM_EYEDROPPER,
		(WPARAM)nFlags, (LPARAM)pixel);
}
