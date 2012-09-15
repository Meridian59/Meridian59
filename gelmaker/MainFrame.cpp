// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// MainFrame.cpp : implementation of the CMainFrame class
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

#include "MainFrame.h"
#include "GelPreview.h"
#include "GelView.h"
#include "GelDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMainFrame, CPersistFrame)

BEGIN_MESSAGE_MAP(CMainFrame, CPersistFrame)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPersistFrame::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	BOOL bSuccess;

	bSuccess = m_wndSplitter.CreateStatic(this, 1, 4);

	bSuccess = bSuccess &&
		m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CGelPreview),
			CSize(175, 100), pContext);

	bSuccess = bSuccess &&
		m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CGelPreview),
			CSize(175, 100), pContext);

	bSuccess = bSuccess &&
		m_wndSplitter.CreateView(0, 2, RUNTIME_CLASS(CGelView),
			CSize(150, 100), pContext);

	bSuccess = bSuccess &&
		m_wndSplitter.CreateView(0, 3, RUNTIME_CLASS(CGelPreview),
			CSize(175, 100), pContext);

	if (pContext->m_pCurrentDoc)
		UpdateSample((CGelDoc*)pContext->m_pCurrentDoc);

	return bSuccess;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	return CPersistFrame::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CPersistFrame::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CPersistFrame::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnDestroy() 
{
	CPersistFrame::OnDestroy();
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	CGelDoc* pDoc = (CGelDoc*)GetActiveDocument();
	if (!pDoc)
		return;
	ASSERT_DYNAMIC(pDoc, CGelDoc);

	CString sFilename;
	::DragQueryFile(hDropInfo, 0, sFilename.GetBuffer(2048), 2048);
	sFilename.ReleaseBuffer();

	if (!sFilename.IsEmpty() && pDoc->LoadSample(sFilename))
	{
		::DragFinish(hDropInfo);
	}
	else
	{
		CPersistFrame::OnDropFiles(hDropInfo);
	}
}

void CMainFrame::UpdateSample(CGelDoc* pDoc /* = NULL */)
{
	if (!pDoc)
		pDoc = (CGelDoc*)GetActiveDocument();
	if (!pDoc)
		return;
	ASSERT_DYNAMIC(pDoc, CGelDoc);

	CDibView* pDibView;

	pDibView = (CDibView*)m_wndSplitter.GetPane(0, 0);
	ASSERT_DYNAMIC(pDibView, CDibView);
	pDibView->SetAutoChangePalette(FALSE);
	pDibView->SetDib(pDoc->m_pUntransformed);

	pDibView = (CDibView*)m_wndSplitter.GetPane(0, 1);
	ASSERT_DYNAMIC(pDibView, CDibView);
	pDibView->SetAutoChangePalette(FALSE);
	pDibView->SetDib(pDoc->m_pSelection);

	pDibView = (CDibView*)m_wndSplitter.GetPane(0, 3);
	ASSERT_DYNAMIC(pDibView, CDibView);
	pDibView->SetAutoChangePalette(FALSE);
	pDibView->SetDib(pDoc->m_pTransformed);
}
