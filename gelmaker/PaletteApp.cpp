// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// PaletteApp.cpp : Defines the class behaviors for the CPaletteApp class.
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

#include "StdAfx.h"

// define CPALETTEAPP_BASE to CWinApp or derivative

#include "PaletteApp.h"

#include "Dib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPaletteApp, CPALETTEAPP_BASE)

#ifdef _DEBUGMEM
#define new DEBUG_NEW
#endif // _DEBUGMEM

BEGIN_MESSAGE_MAP(CPaletteApp, CPALETTEAPP_BASE)
	//{{AFX_MSG_MAP(CPaletteApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CPaletteApp::CPaletteApp()
	: CPALETTEAPP_BASE()
{
	m_pAppActivePalette = NULL;
	m_pAppInactivePalette = NULL;
}

CPaletteApp::~CPaletteApp()
{
	if (m_pAppActivePalette)
		delete m_pAppActivePalette;

	if (m_pAppInactivePalette)
		delete m_pAppInactivePalette;
}

/////////////////////////////////////////////////////////////////////////////

CPalette* CPaletteApp::GetAppPalette()
{
	return AfxIsAppActive()? m_pAppActivePalette : m_pAppInactivePalette;
}

/////////////////////////////////////////////////////////////////////////////

void CPaletteApp::SetAppPalette(UINT uIdDib)
{
	if (m_pAppActivePalette)
		delete m_pAppActivePalette;

	if (m_pAppInactivePalette)
		delete m_pAppInactivePalette;

	CDib dib(uIdDib);
	m_pAppActivePalette = dib.ClonePalette(PC_NOCOLLAPSE, TRUE);
	m_pAppInactivePalette = dib.ClonePalette(0, TRUE);

	//REVIEW: doesn't seem to happen immediately.
	CWindowDC dc(CWnd::GetDesktopWindow());
	RealizeAppPalette(&dc);
}

void CPaletteApp::SetAppPalette(CDib* pDib)
{
	if (m_pAppActivePalette)
	{
		delete m_pAppActivePalette;
		m_pAppActivePalette = NULL;
	}

	if (m_pAppInactivePalette)
	{
		delete m_pAppInactivePalette;
		m_pAppInactivePalette = NULL;
	}

	if (pDib && pDib->m_pInfo)
	{
		m_pAppActivePalette = pDib->ClonePalette(PC_NOCOLLAPSE, TRUE);
		m_pAppInactivePalette = pDib->ClonePalette(0, TRUE);
	}

	//REVIEW: doesn't seem to happen immediately.
	CWindowDC dc(CWnd::GetDesktopWindow());
	RealizeAppPalette(&dc);
}

/////////////////////////////////////////////////////////////////////////////

UINT CPaletteApp::RealizeAppPalette(CDC* pDC)
{
	// PERFORMANCE

	static CPalette* pLastApp = NULL;
	ASSERT(pDC);

#ifdef _DEBUGRARE
	// Note that this may be called often since every OnPaint()
	// or other DC drawing done by the app should realize this palette
	// into the DC before drawing.
	//
	TRACE("CPaletteApp::RealizeAppPalette() called\n");
#endif // _DEBUGRARE

	CPalette* pAppPalette = GetAppPalette();
	UINT uChanged = 0;
	if (pAppPalette)
	{
		pDC->SelectPalette(pAppPalette, FALSE);
		uChanged = pDC->RealizePalette();
		if (uChanged || pAppPalette != pLastApp)
		{
			// Cause all windows to repaint with new palette.
			// REVIEW: This only affects views of the current document.
			//
			if (m_pMainWnd && m_pMainWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
			{
				CDocument* pDoc = ((CFrameWnd*)m_pMainWnd)->GetActiveDocument();
				if (pDoc)
					pDoc->UpdateAllViews(NULL);
			}
			pLastApp = pAppPalette;
			uChanged = 256;
		}
	}

	return uChanged;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPaletteApp::PreTranslateMessage(MSG* pMsg)
{

#if 0
	// STILL UNDER DEVELOPMENT

	if (pMsg->message == WM_PALETTECHANGED)
	{
		HWND hChangedPalette = (HWND)pMsg->wParam;

		// This message is sent to all top-level windows (no owner or parent)
		// to get them to realize the new palette for their own drawing.
		// Since this is caused by one of the top-level windows realizing
		// a palette in the first place, avoid recursion here.
		// 
		if (pMsg->hwnd == hChangedPalette)
			return FALSE;
	}

	if (pMsg->message == WM_PALETTECHANGED ||
		pMsg->message == WM_QUERYNEWPALETTE)
	{
		// We should realize any palette we need into the system palette
		// so that our appearance can be matched as well as possible after
		// palette changes.
		//
		// REVIEW: if a window were handling this, they'd return an LRESULT
		// of TRUE to indicate that they manipulated the palette.  Not
		// sure how to do this if we wanted to supply that TRUE result.
		// The return-value of a message is handled by DispatchMessage().
		//
		CClientDC dc(CWnd::FromHandle(pMsg->hwnd));
		UINT u = RealizeAppPalette(&dc);

		return FALSE;
	}
#endif

	return CPALETTEAPP_BASE::PreTranslateMessage(pMsg);
}
