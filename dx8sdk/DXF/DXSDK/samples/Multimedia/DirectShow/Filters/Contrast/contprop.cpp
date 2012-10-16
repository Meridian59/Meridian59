//------------------------------------------------------------------------------
// File: ContProp.cpp
//
// Desc: DirectShow sample code - implementation of property page class
//       for the contrast filter.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include "resource.h"
#include "contuids.h"
#include "icontrst.h"
#include "contrast.h"
#include "contprop.h"


//
// CreateInstance
//
// This goes in the factory template table to create new filter instances
//
CUnknown * WINAPI CContrastProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CContrastProperties(lpunk, phr);
    if (punk == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return punk;

} // CreateInstance


//
// Constructor
//
CContrastProperties::CContrastProperties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("Contrast Property Page"),pUnk,
                      IDD_CONTRASTPROP,
                      IDS_TITLE),
    m_pContrast(NULL)
{
    InitCommonControls();

} // (Constructor)


//
// SetDirty
//
// Sets m_bDirty and notifies the property page site of the change
//
void CContrastProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite) {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }

} // SetDirty


//
// OnReceiveMessage
//
// Virtual method called by base class with Window messages
//
BOOL CContrastProperties::OnReceiveMessage(HWND hwnd,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
	    m_hwndSlider = CreateSlider(hwnd);
	    ASSERT(m_hwndSlider);
            return (LRESULT) 1;
        }
        case WM_VSCROLL:
        {
	    ASSERT(m_hwndSlider);
	    OnSliderNotification(wParam);
            return (LRESULT) 1;
        }

        case WM_COMMAND:
        {
	    if (LOWORD(wParam) == IDB_DEFAULT)
            {
	        pIContrast()->put_DefaultContrastLevel();
	        SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, 0L);
                SetDirty();
	    }
            return (LRESULT) 1;
        }

        case WM_DESTROY:
        {
	    DestroyWindow(m_hwndSlider);
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


//
// OnConnect
//
// Called when the property page connects to a filter
//
HRESULT CContrastProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pContrast == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IContrast, (void **) &m_pContrast);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pContrast);

    // Get the initial contrast value
    m_pContrast->get_ContrastLevel(&m_cContrastLevel);
    m_cContrastOnExit = m_cContrastLevel;
    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Called when we're disconnected from a filter
//
HRESULT CContrastProperties::OnDisconnect()
{
    // Release of Interface after setting the appropriate contrast value

    if (m_pContrast == NULL) {
        return E_UNEXPECTED;
    }

    m_pContrast->put_ContrastLevel(m_cContrastOnExit, 0);
    m_pContrast->Release();
    m_pContrast = NULL;
    return NOERROR;

} // OnDisconnect


//
// OnDeactivate
//
// We are being deactivated
//
HRESULT CContrastProperties::OnDeactivate(void)
{
    // Remember the present contrast level for the next activate

    pIContrast()->get_ContrastLevel(&m_cContrastLevel);
    return NOERROR;

} // OnDeactivate


//
// OnApplyChanges
//
// Changes made should be kept. Change the m_cContrastOnExit variable
//
HRESULT CContrastProperties::OnApplyChanges()
{
    pIContrast()->get_ContrastLevel(&m_cContrastOnExit);
    m_bDirty = FALSE;
    return(NOERROR);

} // OnApplyChanges


//
// CreateSlider
//
// Create the slider (common control) to allow the user to adjust contrast
//
HWND CContrastProperties::CreateSlider(HWND hwndParent)
{
    ULONG Styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_VERT | TBS_BOTH | WS_GROUP;
    HWND hwndSlider = CreateWindow( TRACKBAR_CLASS,
				    TEXT(""),
				    Styles,
				    62, 35,
				    39, 100,
				    hwndParent,
				    NULL,
				    g_hInst,
				    NULL);
    if (hwndSlider == NULL) {
	    // DWORD dwErr = GetLastError();
	    return NULL;
    }

    // Set the initial range for the slider
    SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(MinContrastLevel, MaxContrastLevel) );

    // Set a tick at zero
    SendMessage(hwndSlider, TBM_SETTIC, 0, 0L);

    // Set the initial slider position
    SendMessage(hwndSlider, TBM_SETPOS, TRUE, m_cContrastLevel);

    return hwndSlider;

} // CreateSlider


//
// OnSliderNotification
//
// Handle the notification messages from the slider control
//
void CContrastProperties::OnSliderNotification(WPARAM wParam)
{
    switch (wParam) {
        case TB_BOTTOM:
            SetDirty();
	    SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, (LPARAM) MinContrastLevel);
	    break;

        case TB_TOP:
            SetDirty();
    	    SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, (LPARAM) MaxContrastLevel);
            break;

        case TB_PAGEDOWN:
        case TB_PAGEUP:
            break;

        case TB_THUMBPOSITION:
        case TB_ENDTRACK:
        {
                SetDirty();
                signed char Level = (signed char) SendMessage(m_hwndSlider, TBM_GETPOS, 0, 0L);
	        pIContrast()->put_ContrastLevel(Level, 0);
        }
	break;

        // Default handling of these messages is ok
        case TB_THUMBTRACK:
        case TB_LINEDOWN:
        case TB_LINEUP:
	    break;
    }

} // OnSliderNotification
