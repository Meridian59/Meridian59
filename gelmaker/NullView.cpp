// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// NullView.cpp : implementation file
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

#include "NullView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CNullView, CScrollView)

/////////////////////////////////////////////////////////////////////////////

CNullView::CNullView()
{
}

CNullView::~CNullView()
{
}

BEGIN_MESSAGE_MAP(CNullView, CScrollView)
	//{{AFX_MSG_MAP(CNullView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CNullView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
}

void CNullView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	CSize szScroll = CSize(400, 2);
	SetScrollSizes(MM_TEXT, szScroll);
//	ResizeParentToFit();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CNullView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CNullView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
