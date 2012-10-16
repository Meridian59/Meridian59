//------------------------------------------------------------------------------
// File: NullProp.cpp
//
// Desc: DirectShow sample code - implementation of NullIPProperties class.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>

// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most picky) warning level (-W4).
#pragma warning(disable: 4511 4512)

#include <windowsx.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include "resource.h"    // ids used in the dialog
#include "nulluids.h"    // public guids
#include "inull.h"       // private interface between property sheet and filter
#include "nullprop.h"    // our own class
#include "texttype.h"    // class to express media types as text


//
// CreateInstance
//
// Override CClassFactory method.
// Set lpUnk to point to an IUnknown interface on a new NullIPProperties object
// Part of the COM object instantiation mechanism
//
CUnknown * WINAPI NullIPProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new NullIPProperties(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


//
// NullIPProperties::Constructor
//
// Constructs and initialises a NullIPProperties object
//
NullIPProperties::NullIPProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("NullIP Property Page"),pUnk,
        IDD_DIALOG1, IDS_TITLE)
    , m_pPin(NULL)
    , m_nIndex(0)
    , m_pINullIPP(NULL)
{
    ASSERT(phr);

} // (constructor) NullIPProperties


//
// SetDirty
//
// Sets m_hrDirtyFlag and notifies the property page site of the change
//
void NullIPProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }

} // SetDirty


//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handles the messages for our property window
//
BOOL NullIPProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // get the hWnd of the list box
            m_hwndLB = GetDlgItem (hwnd, IDC_MEDIALIST) ;
            FillListBox();
            return (LRESULT) 1;
        }

        case WM_COMMAND:
        {
                if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                SetDirty();
            }
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


//
// OnConnect
//
// Override CBasePropertyPage method.
// Notification of which object this property page should display.
// We query the object for the INullIPP interface.
//
// If cObjects == 0 then we must release the interface.
// Set the member variable m_pPin to the upstream output pin connected
// to our input pin (or NULL if not connected).
//
HRESULT NullIPProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pINullIPP == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_INullIPP, (void **) &m_pINullIPP);
    if (FAILED(hr))
    {
        return E_NOINTERFACE;
    }

    ASSERT(m_pINullIPP);

    ASSERT(!m_pPin);
    m_pINullIPP->get_IPin(&m_pPin);

    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Override CBasePropertyPage method.
// Release the private interface, release the upstream pin.
//
HRESULT NullIPProperties::OnDisconnect()
{
    // Release of Interface

    if (m_pINullIPP == NULL)
        return E_UNEXPECTED;
    m_pINullIPP->Release();
    m_pINullIPP = NULL;

    //
    // Release the pin interface that we are holding.
    //
    if (m_pPin)
    {
        m_pPin->Release() ;
        m_pPin = NULL ;
    }
    return NOERROR;

} // OnDisconnect


//
// Activate
//
// We are being activated
//
HRESULT NullIPProperties::OnActivate()
{
    SendMessage (m_hwndLB, LB_SETCURSEL, m_nIndex, 0) ;
    return NOERROR;

} // Activate


//
// OnApplyChanges
//
// Changes made should be kept.
//
HRESULT NullIPProperties::OnApplyChanges()
{

    ASSERT(m_pINullIPP);

    //
    // get the current selection of the media type
    //
    int iIndex = SendMessage (m_hwndLB, LB_GETCURSEL, 0, 0) ;
    if (iIndex <= 0)
        iIndex = 0 ;

    m_bDirty = FALSE;            // the page is now clean
    if (iIndex == 0)
    {
        m_pINullIPP->put_MediaType (NULL) ;
        m_nIndex = 0;
        return NOERROR;
    }

    IEnumMediaTypes * pMTEnum;
    AM_MEDIA_TYPE *pMediaType;
    ULONG count;

    //
    // get the type enumerator from the pin again
    //
    HRESULT hr = m_pPin->EnumMediaTypes(&pMTEnum);
    if (FAILED(hr))
    {
        return hr;
    }

    //
    // skip to the appropriate type and read it
    //
    pMTEnum->Skip(iIndex - 1);
    pMTEnum->Next(1, &pMediaType, &count);
    if (count != 1) {
        pMTEnum->Release();
        return E_UNEXPECTED;
    }

    //
    // change the type by calling the filter
    //
    CMediaType cmt(*pMediaType);
    DeleteMediaType(pMediaType);
    hr = m_pINullIPP->put_MediaType(&cmt) ;
    if (FAILED(hr))
    {
        pMTEnum->Release();
        return NOERROR;
    }

    //
    // update the current selection and release the enumerator
    //
    m_nIndex = iIndex;
    pMTEnum->Release();
    return NOERROR;

} // OnApplyChanges


//
// FillListBox
//
// Fill the list box with an enumeration of the media type that our
//
void NullIPProperties::FillListBox()
{
    IEnumMediaTypes * pMTEnum;
    AM_MEDIA_TYPE *pMediaType;
    ULONG count;
    TCHAR szBuffer[200];
    int Loop = 0, wextent = 0 ;
    SIZE extent ;
    FILTER_STATE state ;
    CMediaType *pmtCurrent ;

    //
    // get the current media type
    //
    ASSERT(m_pINullIPP);
    m_pINullIPP->get_MediaType (&pmtCurrent);
    m_nIndex = 0 ;

    //
    // Fill the first entry of the list box with a choice to select any media.
    //
    LoadString(g_hInst, IDS_ANYTYPE, szBuffer, 200);
    SendMessage (m_hwndLB, LB_ADDSTRING, 0, (LPARAM)szBuffer) ;

    //
    // if the filter is not connected on the input, nothing more to fill
    // also return if we haven't gotten any pin interface
    //
    if (m_pPin == NULL)
        return ;

    //
    // get a dc for the control. need this to get the extent of the strings
    // that we put in the list box (so that we can get a horiz scroll bar).
    //
    HDC hdc = GetDC (m_hwndLB) ;

    //
    // now enumerate the media types from our pin.
    //
    HRESULT hr = m_pPin->EnumMediaTypes(&pMTEnum);

    if (SUCCEEDED(hr))
    {
        ASSERT(pMTEnum);
        pMTEnum->Next(1, &pMediaType, &count);
        while (count == 1)                    // While a media type retrieved OK
        {
            //
            // Count it - used to set m_nIndex.
            //
            Loop++;

            //
            // convert the media type to string.
            //
            CTextMediaType(*pMediaType).AsText(szBuffer, 200, TEXT(" - "), TEXT(" - "), TEXT("\0"));

            //
            // if the mediatype matches the current one, set m_nIndex to cur index
            //
            if (*pmtCurrent == *pMediaType) {
                m_nIndex = Loop;
            }

            //
            // get the extent of the string and save the max extent
            //
            GetTextExtentPoint (hdc, szBuffer, lstrlen(szBuffer), &extent) ;
            if (extent.cx > wextent)
                wextent = extent.cx ;

            //
            // add the string to the list box.
            //
            SendMessage (m_hwndLB, LB_ADDSTRING, 0, (LPARAM)szBuffer) ;

            //
            // enumerate the next media type and loop.
            //
            DeleteMediaType(pMediaType);
            pMTEnum->Next(1, &pMediaType, &count);
        }
        pMTEnum->Release();
        SendMessage (m_hwndLB, LB_SETHORIZONTALEXTENT, wextent, 0) ;
        SendMessage (m_hwndLB, LB_SETCURSEL, m_nIndex, 0) ;
    }
    ASSERT(m_pINullIPP);

    //
    // if the filter is in a running state, disable the list box and allow
    // no input.
    //
    m_pINullIPP->get_State(&state);
    EnableWindow (m_hwndLB, state != State_Running) ;

} // FillListBox
