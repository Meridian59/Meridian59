//------------------------------------------------------------------------------
// File: NullProp.h
//
// Desc: DirectShow sample code - definition of NullIPProperties class.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


class NullIPProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    DECLARE_IUNKNOWN;

private:

    BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();

    void SetDirty();
    void FillListBox();

    NullIPProperties(LPUNKNOWN lpunk, HRESULT *phr);

    HWND        m_hwndLB ;       // Handle of the list box
    int         m_nIndex ;       // Index of the selected media type
    IPin        *m_pPin ;        // The upstream output pin connected to us
    INullIPP    *m_pINullIPP;    // Null In Place property interface

};  // class NullIPProperties

