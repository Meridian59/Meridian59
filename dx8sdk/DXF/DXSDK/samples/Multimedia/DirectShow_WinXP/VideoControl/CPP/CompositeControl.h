//------------------------------------------------------------------------------
// File: CompositeControl.h
//
// Desc: Declaration of the CCompositeControl
//       for the Windows XP MSVidCtl C++ sample
//
// Copyright (c) 2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef __COMPOSITECONTROL_H_
#define __COMPOSITECONTROL_H_

#include "resource.h"       // main symbols

#include <atlctl.h>
#include <msvidctl.h>

#pragma warning(disable:4100)  /* Disable 'unused parameter' warning */

#define SAFE_RELEASE(x) { if (x) { (x)->Release();  x=NULL; }}

/////////////////////////////////////////////////////////////////////////////
// CCompositeControl
class ATL_NO_VTABLE CCompositeControl : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ICompositeControl, &IID_ICompositeControl, &LIBID_CPPVIDEOCONTROLLib>,
    public CComCompositeControl<CCompositeControl>,
    public IPersistStreamInitImpl<CCompositeControl>,
    public IOleControlImpl<CCompositeControl>,
    public IOleObjectImpl<CCompositeControl>,
    public IOleInPlaceActiveObjectImpl<CCompositeControl>,
    public IViewObjectExImpl<CCompositeControl>,
    public IOleInPlaceObjectWindowlessImpl<CCompositeControl>,
    public IPersistStorageImpl<CCompositeControl>,
    public ISpecifyPropertyPagesImpl<CCompositeControl>,
    public IQuickActivateImpl<CCompositeControl>,
    public IDataObjectImpl<CCompositeControl>,
    public IProvideClassInfo2Impl<&CLSID_CompositeControl, NULL, &LIBID_CPPVIDEOCONTROLLib>,
    public CComCoClass<CCompositeControl, &CLSID_CompositeControl>
{
public:
    CCompositeControl()
    {
        m_bWindowOnly = TRUE;
        CalcExtent(m_sizeExtent);

        // Initialize internal data
        m_pTuningSpaceContainer = NULL;
        m_pATSCTuningSpace = NULL;
        m_pATSCLocator = NULL;
        m_pMSVidCtl = NULL;
        m_hwndChannelID = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_COMPOSITECONTROL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCompositeControl)
    COM_INTERFACE_ENTRY(ICompositeControl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CCompositeControl)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
END_PROP_MAP()

BEGIN_MSG_MAP(CCompositeControl)
    CHAIN_MSG_MAP(CComCompositeControl<CCompositeControl>)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_HANDLER(IDC_CHANNELDOWN, BN_CLICKED, OnClickedChanneldown)
    COMMAND_HANDLER(IDC_CHANNELUP, BN_CLICKED, OnClickedChannelup)
    COMMAND_ID_HANDLER(WM_CLOSE, OnExit)
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
END_MSG_MAP()

// Handler prototypes:
    LRESULT OnExit(WORD /*wNotifyCode*/, WORD /* wID */, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

BEGIN_SINK_MAP(CCompositeControl)
    //Make sure the Event Handlers have __stdcall calling convention
END_SINK_MAP()

    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
    {
        if (dispid == DISPID_AMBIENT_BACKCOLOR)
        {
            SetBackgroundColorFromAmbient();
            FireViewChange();
        }
        return IOleControlImpl<CCompositeControl>::OnAmbientPropertyChange(dispid);
    }


// IViewObjectEx
    DECLARE_VIEW_STATUS(0)

// ICompositeControl
public:

    enum { IDD = IDD_COMPOSITECONTROL };
    IMSVidCtl * m_pMSVidCtl;
    ITuningSpaceContainer * m_pTuningSpaceContainer;
    IATSCTuningSpace * m_pATSCTuningSpace;
    IATSCLocator * m_pATSCLocator;
    HWND m_hwndChannelID;
    
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClickedChannelup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedChanneldown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    HRESULT SetChannel(long lChannel);
    void ShowChannelNumber(long lChannel);
    
};
#endif //__COMPOSITECONTROL_H_
