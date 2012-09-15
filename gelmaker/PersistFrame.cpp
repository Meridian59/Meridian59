// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// PersistFrame.cpp : implementation of the CPersistFrame class
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

#include "StdAfx.h"

// header defines CPERSISTFRAME_BASE to CFrameWnd or CMDIFrameWnd

#include "PersistFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPersistFrame, CPERSISTFRAME_BASE)

BEGIN_MESSAGE_MAP(CPersistFrame, CPERSISTFRAME_BASE)
	//{{AFX_MSG_MAP(CPersistFrame)
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static char* _toolbars = "-Bars";

/////////////////////////////////////////////////////////////////////////////

CPersistFrame::CPersistFrame()
{
	m_bPreActivation = TRUE;
	m_bAutoPersist = TRUE;
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CPersistFrame::AssertValid() const
{
	CPERSISTFRAME_BASE::AssertValid();
	// m_sPersistName
}

void CPersistFrame::Dump(CDumpContext& dc) const
{
	CPERSISTFRAME_BASE::Dump(dc);
	dc << "\nm_sPersistName = \"" << m_sPersistName << "\"";
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CPersistFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	if (IsAutoPersist() && m_bPreActivation)
	{
		RestoreFramePlacement();
		ASSERT(m_bPreActivation == FALSE);
	}

	CPERSISTFRAME_BASE::OnActivate(nState, pWndOther, bMinimized);
}

/////////////////////////////////////////////////////////////////////////////

void CPersistFrame::OnDestroy() 
{
	if (IsAutoPersist() && !m_bPreActivation)
		WriteFramePlacement();

	CPERSISTFRAME_BASE::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////

void CPersistFrame::_DataToHexString(CString& sHex, LPVOID pvData, int cbLength)
{
	LPBYTE pbData = (LPBYTE)pvData;

	LPSTR pHex = sHex.GetBuffer(2*cbLength+1);

	while (cbLength > 0)
	{
		wsprintf(pHex, "%02X", *pbData++);
		pHex += 2;
		cbLength--;
	}

	sHex.ReleaseBuffer();
}

BOOL CPersistFrame::_HexStringToData(CString& sHex, LPVOID pvData, int cbLength)
{
	LPBYTE pbData = (LPBYTE)pvData;
	LPCSTR pHex = sHex;

	if (sHex.GetLength() != 2*cbLength)
	{
		//ASSERT(0);
		return FALSE;
	}

	while (cbLength)
	{
		BYTE b = 0;

		// input and convert two ascii hex digits into b

		static char* _hex = "0123456789ABCDEF0123456789abcdef";
		char* pDigit;

		pDigit = strchr(_hex, *pHex++);
		if (pDigit)
			b = ((pDigit - _hex) & 0x0F) << 4;

		pDigit = strchr(_hex, *pHex++);
		if (pDigit)
			b |= ((pDigit - _hex) & 0x0F);

		*pbData++ = b;
		cbLength--;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPersistFrame::LoadToolBar(CToolBar* pBar, UINT uID)
{
	if (!pBar->Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP, uID) ||
		!pBar->LoadToolBar(uID))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;
	}

	LONG l = pBar->GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;
	pBar->SetBarStyle(l);

	CString sResource;
	sResource.LoadString(uID);
	CString sCaption;
	AfxExtractSubString(sCaption, sResource, 2); // menuflyby\ntooltip\ncaption
	pBar->SetWindowText(sCaption);

	pBar->EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(pBar);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

#if 0

//TODO: future convenience routines take a CSplitterWnd* and save the
//      split dimensions for later.
//      need to save #rows, #cols, cy-currideal/row, cx-currideal/col

static LPCTSTR _splitter = "Split";
static LPCTSTR _rows = "Rows";
static LPCTSTR _cols = "Columns";

void CPersistFrame::SaveSplitterState(CSplitterWnd* pSplit, LPCTSTR pszName /* = NULL */)
{
	if (!_FindSafePersistName())
		return;

	ASSERT(pSplit && pSplit->m_hWnd);
	if (!pSplit || !pSplit->m_hWnd)
		return;

	CString sName = m_sPersistName + _splitter;
	if (pszName && *pszName)
		sName += pszName;

	int iRows = pSplit->GetRowCount();
	int iCols = pSplit->GetColumnCount();
	AfxGetApp()->WriteProfileString(_rows, sName);
	AfxGetApp()->WriteProfileString(_cols, sName);
}

void CPersistFrame::RestoreSplitterState(CSplitterWnd* pSplit, LPCTSTR pszName /* = NULL */)
{
	if (!_FindSafePersistName())
		return;

	ASSERT(pSplit && pSplit->m_hWnd);
	if (!pSplit || !pSplit->m_hWnd)
		return;

	//TODO: 
}

#endif

/////////////////////////////////////////////////////////////////////////////

static char* _frameplacement = "FramePlacement";

void CPersistFrame::SetPersistName(LPCSTR pszName)
{
	m_sPersistName = pszName;

	//BUGBUG: Win31 INI handling fails if frame name contains '=', '[', ']', or ';'
}

LPCSTR CPersistFrame::GetPersistName()
{
	if (!_FindSafePersistName())
		return NULL;

	return m_sPersistName;
}

BOOL CPersistFrame::_FindSafePersistName()
{
	// Default frame name in case it wasn't set.
	//
	if (m_sPersistName.IsEmpty())
	{
		GetWindowText(m_sPersistName);
		if (m_sPersistName.IsEmpty())
		{
			m_sPersistName = AfxGetAppName();
			if (m_sPersistName.IsEmpty())
			{
				TRACE("warning CPersistFrame::_FindSafePersistName: no name\n");
				return FALSE;
			}
		}
	}

	//BUGBUG: Win31 INI handling fails if frame name contains '=', '[', ']', or ';'

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CPersistFrame::RestoreFramePlacement()
{
	m_bPreActivation = FALSE;

	if (!_FindSafePersistName())
		return;

	// Read placement from profile string (hex digits).
	//
	CString sStruct =
		AfxGetApp()->GetProfileString(_frameplacement, m_sPersistName);
	if (sStruct.IsEmpty())
		return;

	// Restore from loaded placement.
	//
	WINDOWPLACEMENT wndpl;
	_HexStringToData(sStruct, &wndpl, sizeof(wndpl));
	SetWindowPlacement(&wndpl);

	// Restore toolbar state, too.
	CString s = m_sPersistName + _toolbars;
	LoadBarState(s);

	// Call the overridable for additional frame state.
	//
	OnRestoreFrameInfo();
}

void CPersistFrame::WriteFramePlacement()
{
	if (!_FindSafePersistName())
		return;

	// Get the window placement; this covers minimized, restore size, etc.
	//
	WINDOWPLACEMENT wndpl;
	if (!GetWindowPlacement(&wndpl))
		return;

	// Save placement as hex digits in a profile string.
	//
	CString sStruct;
	_DataToHexString(sStruct, &wndpl, sizeof(wndpl));
	AfxGetApp()->WriteProfileString(_frameplacement, m_sPersistName, sStruct);

	// Save toolbar state.
	CString s = m_sPersistName + _toolbars;
	SaveBarState(s);

	// Call the overridable for additional frame state.
	//
	OnWriteFrameInfo();
}

/////////////////////////////////////////////////////////////////////////////

void CPersistFrame::ShoveFrameOntoDesktop(int iBuffer /* = 0 */)
{
	if (!m_hWnd || IsIconic() || IsZoomed())
		return;

	CRect rcWindow;
	GetWindowRect(&rcWindow);

	// Get the desktop area.
	//
	CRect rcDesktop;
	if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0))
		CWnd::GetDesktopWindow()->GetWindowRect(&rcDesktop);

	// Allow a border area around the edge of the desktop to peek through.
	//
	iBuffer *= ::GetSystemMetrics(SM_CXBORDER);
	rcDesktop.InflateRect(-iBuffer, -iBuffer);

	// Shove the window around if it reaches out that safe area.
	// Favor topleft visibility over bottomright for large windows.
	//
	if (rcWindow.right > rcDesktop.right)
		rcWindow.OffsetRect(rcDesktop.right - rcWindow.right, 0);
	if (rcWindow.bottom > rcDesktop.bottom)
		rcWindow.OffsetRect(0, rcDesktop.bottom - rcWindow.bottom);
	if (rcWindow.left < rcDesktop.left)
		rcWindow.OffsetRect(rcDesktop.left - rcWindow.left, 0);
	if (rcWindow.top < rcDesktop.top)
		rcWindow.OffsetRect(0, rcDesktop.top - rcWindow.top);

	MoveWindow(&rcWindow);
}
