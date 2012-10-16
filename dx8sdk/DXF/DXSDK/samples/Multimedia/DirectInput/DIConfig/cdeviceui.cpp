/******************************************************************************
 * File: CDeviceUI.cpp
 *
 * Desc:
 *
 * CDeviceUI is a helper that holds all the views and a bunch of
 * information for a specific device.  It has a CFlexWnd whose
 * handler it sets to the CDeviceView for the current view,
 * thus reusing one window to implement multiple pages.
 *
 * All CDeviceViews and CDeviceControls have a reference to the CDeviceUI
 * that created them (m_ui).  Thus, they also have access to the
 * CUIGlobals, since CDeviceUI has a reference to them (m_ui.m_uig).
 * CDeviceUI also provides the following read-only public variables
 * for convenience, all referring to the device this CDeviceUI
 * represents:
 * 
 * const DIDEVICEINSTANCEW &m_didi;
 * const LPDIRECTINPUTDEVICE8W &m_lpDID;
 * const DIDEVOBJSTRUCT &m_os;
 *
 * See usefuldi.h for a description of DIDEVOBJSTRUCT.
 *
 * CDeviceUI communicates to the rest of the UI via the CDeviceUINotify
 * abstract base class.  Another class (in our case CDIDeviceActionConfigPage)
 * must derive from CDeviceUINotify, and define the DeviceUINotify() and
 * IsControlMapped() virtual functions.  This derived class must be passed as
 * the last parameter to CDeviceUI's Init() function.  All the views and 
 * controls within the views notify the UI of user actions via m_ui.Notify(),
 * so that all actionformat manipulation can be done in the page class.  The
 * views and controls themselves never touch the actionformat.  See the
 * DEVICEUINOTIFY structure below for information on the parameter passed
 * through Notify()/DeviceUINotify().
 *
 * Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
 *
 ***************************************************************************/

#include "common.hpp"
#include <dinputd.h>
#include "configwnd.h"

#define DIPROP_MAPFILE MAKEDIPROP(0xFFFD)

CDeviceUI::CDeviceUI(CUIGlobals &uig, IDIConfigUIFrameWindow &uif) :
	m_uig(uig), m_UIFrame(uif),
	m_didi(m_priv_didi), m_lpDID(m_priv_lpDID), m_os(m_priv_os),
	m_pCurView(NULL),
	m_pNotify(NULL), m_hWnd(NULL), m_bInEditMode(FALSE)
{
	m_priv_lpDID = NULL;
}

CDeviceUI::~CDeviceUI()
{
	Unpopulate();
}

HRESULT CDeviceUI::Init(const DIDEVICEINSTANCEW &didi, LPDIRECTINPUTDEVICE8W lpDID, HWND hWnd, CDeviceUINotify *pNotify)
{tracescope(__ts, _T("CDeviceUI::Init()...\n"));
	// save the params
	m_priv_didi = didi;
	m_priv_lpDID = lpDID;
	m_pNotify = pNotify;
	m_hWnd = hWnd;

	// fail if we don't have lpDID
	if (m_lpDID == NULL)
	{
		etrace(_T("CDeviceUI::Init() was passed a NULL lpDID!\n"));
		return E_FAIL;
	}

	// fill the devobjstruct
	HRESULT hr = FillDIDeviceObjectStruct(m_priv_os, lpDID);
	if (FAILED(hr))
	{
		etrace1(_T("FillDIDeviceObjectStruct() failed, returning 0x%08x\n"), hr);
		return hr;
	}

	// view rect needs to be set before populating so the views are
	// created with the correct dimensions
	m_ViewRect = g_ViewRect;

	// populate
	hr = PopulateAppropriately(*this);
	if (FAILED(hr))
		return hr;

	// if there are no views, return
	if (GetNumViews() < 1)
	{
		Unpopulate();
		return E_FAIL;
	}

	// show the first view
	SetView(0);

	return hr;
}

void CDeviceUI::Unpopulate()
{
	m_pCurView = NULL;

	for (int i = 0; i < GetNumViews(); i++)
	{
		if (m_arpView[i] != NULL)
			delete m_arpView[i];
		m_arpView[i] = NULL;
	}
	m_arpView.RemoveAll();

	Invalidate();
}

void CDeviceUI::SetView(int nView)
{
	if (nView >= 0 && nView < GetNumViews())
		SetView(m_arpView[nView]);
}

void CDeviceUI::SetView(CDeviceView *pView)
{
	if (m_pCurView != NULL)
		ShowWindow(m_pCurView->m_hWnd, SW_HIDE);

	m_pCurView = pView;

	if (m_pCurView != NULL)
		ShowWindow(m_pCurView->m_hWnd, SW_SHOW);
}

CDeviceView *CDeviceUI::GetView(int nView)
{
	if (nView >= 0 && nView < GetNumViews())
		return m_arpView[nView];
	else
		return NULL;
}

CDeviceView *CDeviceUI::GetCurView()
{
	return m_pCurView;
}

int CDeviceUI::GetViewIndex(CDeviceView *pView)
{
	if (GetNumViews() == 0)
		return -1;

	for (int i = 0; i < GetNumViews(); i++)
		if (m_arpView[i] == pView)
			return i;

	return -1;
}

int CDeviceUI::GetCurViewIndex()
{
	return GetViewIndex(m_pCurView);
}

// gets the thumbnail for the specified view,
// using the selected version if the view is selected
CBitmap *CDeviceUI::GetViewThumbnail(int nView)
{
	return GetViewThumbnail(nView, GetView(nView) == GetCurView());
}

// gets the thumbnail for the specified view,
// specifiying whether or not we want the selected version
CBitmap *CDeviceUI::GetViewThumbnail(int nView, BOOL bSelected)
{
	CDeviceView *pView = GetView(nView);
	if (pView == NULL)
		return NULL;

	return pView->GetImage(bSelected ? DVI_SELTHUMB : DVI_THUMB);
}

void CDeviceUI::DoForAllControls(DEVCTRLCALLBACK callback, LPVOID pVoid, BOOL bFixed)
{
	int nv = GetNumViews();
	for (int v = 0; v < nv; v++)
	{
		CDeviceView *pView = GetView(v);
		if (pView == NULL)
			continue;

		int nc = pView->GetNumControls();
		for (int c = 0; c < nc; c++)
		{
			CDeviceControl *pControl = pView->GetControl(c);
			if (pControl == NULL)
				continue;

			callback(pControl, pVoid, bFixed);
		}
	}
}

typedef struct _DFCIAO {
	DWORD dwOffset;
	DEVCTRLCALLBACK callback;
	LPVOID pVoid;
} DFCIAO;

void DoForControlIfAtOffset(CDeviceControl *pControl, LPVOID pVoid, BOOL bFixed)
{
	DFCIAO &dfciao = *((DFCIAO *)pVoid);

	if (pControl->GetOffset() == dfciao.dwOffset)
		dfciao.callback(pControl, dfciao.pVoid, bFixed);
}

void CDeviceUI::DoForAllControlsAtOffset(DWORD dwOffset, DEVCTRLCALLBACK callback, LPVOID pVoid, BOOL bFixed)
{
	DFCIAO dfciao;
	dfciao.dwOffset = dwOffset;
	dfciao.callback = callback;
	dfciao.pVoid = pVoid;
	DoForAllControls(DoForControlIfAtOffset, &dfciao, bFixed);
}

void SetControlCaptionTo(CDeviceControl *pControl, LPVOID pVoid, BOOL bFixed)
{
	pControl->SetCaption((LPCTSTR)pVoid, bFixed);
}

void CDeviceUI::SetAllControlCaptionsTo(LPCTSTR tszCaption)
{
	DoForAllControls(SetControlCaptionTo, (LPVOID)tszCaption);
}

void CDeviceUI::SetCaptionForControlsAtOffset(DWORD dwOffset, LPCTSTR tszCaption, BOOL bFixed)
{
	DoForAllControlsAtOffset(dwOffset, SetControlCaptionTo, (LPVOID)tszCaption, bFixed);
}

void CDeviceUI::Invalidate()
{
	if (m_pCurView != NULL)
		m_pCurView->Invalidate();
}

void CDeviceUI::SetEditMode(BOOL bEdit)
{
	if (bEdit == m_bInEditMode)
		return;

	m_bInEditMode = bEdit;
	Invalidate();
}


void CDeviceUI::SetDevice(LPDIRECTINPUTDEVICE8W lpDID)
{
	m_priv_lpDID = lpDID;
}

BOOL CDeviceUI::IsControlMapped(CDeviceControl *pControl)
{
	if (pControl == NULL || m_pNotify == NULL)
		return FALSE;

	return m_pNotify->IsControlMapped(pControl);
}

void CDeviceUI::Remove(CDeviceView *pView)
{
	if (pView == NULL)
		return;

	int i = GetViewIndex(pView);
	if (i < 0 || i >= GetNumViews())
	{
		assert(0);
		return;
	}

	if (pView == m_pCurView)
		m_pCurView = NULL;

	if (m_arpView[i] != NULL)
	{
		m_arpView[i]->RemoveAll();
		delete m_arpView[i];
	}
	m_arpView[i] = NULL;

	m_arpView.RemoveAt(i);

	if (m_arpView.GetSize() < 1)
		RequireAtLeastOneView();
	else if (m_pCurView == NULL)
	{
		SetView(0);
		NumViewsChanged();
	}
}

void CDeviceUI::RemoveAll()
{
	m_pCurView = NULL;

	for (int i = 0; i < GetNumViews(); i++)
	{
		if (m_arpView[i] != NULL)
			delete m_arpView[i];
		m_arpView[i] = NULL;
	}
	m_arpView.RemoveAll();

	RequireAtLeastOneView();
}

CDeviceView *CDeviceUI::NewView()
{
	// allocate new view, continuing on if it fails
	CDeviceView *pView = new CDeviceView(*this);
	if (pView == NULL)
		return NULL;

	// add view to array
	m_arpView.SetAtGrow(m_arpView.GetSize(), pView);

	// create view
	pView->Create(m_hWnd, m_ViewRect, FALSE);

	// let the page update to indicate viewness
	NumViewsChanged();

	return pView;
}

CDeviceView *CDeviceUI::UserNewView()
{
	CDeviceView *pView = NewView();
	if (!pView)
		return NULL;

	pView->AddWrappedLineOfText(
		(HFONT)m_uig.GetFont(UIE_PICCUSTOMTEXT),
		m_uig.GetTextColor(UIE_PICCUSTOMTEXT),
		m_uig.GetBkColor(UIE_PICCUSTOMTEXT),
		_T("Customize This View"));

	pView->MakeMissingImages();

	Invalidate();

	return pView;
}

void CDeviceUI::RequireAtLeastOneView()
{
	if (GetNumViews() > 0)
		return;

	CDeviceView *pView = NewView();
	if (!pView)
		return;

	pView->AddWrappedLineOfText(
		(HFONT)m_uig.GetFont(UIE_PICCUSTOMTEXT),
		m_uig.GetTextColor(UIE_PICCUSTOMTEXT),
		m_uig.GetBkColor(UIE_PICCUSTOMTEXT),
		_T("Customize This View"));
	pView->AddWrappedLineOfText(
		(HFONT)m_uig.GetFont(UIE_PICCUSTOM2TEXT),
		m_uig.GetTextColor(UIE_PICCUSTOM2TEXT),
		m_uig.GetBkColor(UIE_PICCUSTOM2TEXT),
		_T("The UI requires at least one view per device"));

	pView->MakeMissingImages();

	SetView(pView);
}

void CDeviceUI::NumViewsChanged()
{
	DEVICEUINOTIFY uin;
	uin.msg = DEVUINM_NUMVIEWSCHANGED;
	Notify(uin);
}

