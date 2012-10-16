//------------------------------------------------------------------------------
// File: GargDMOProp.h
//
// Desc: DirectShow sample code - definition of the CGargDMOProp class.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------


#ifndef __GARGDMOPROP_H_
#define __GARGDMOPROP_H_

#include "resource.h"       // main symbols
#include "controlhelp.h"

EXTERN_C const CLSID CLSID_GargDMOProp;

/////////////////////////////////////////////////////////////////////////////
// CGargDMOProp
class ATL_NO_VTABLE CGargDMOProp :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGargDMOProp, &CLSID_GargDMOProp>,
	public IPropertyPageImpl<CGargDMOProp>,
	public CDialogImpl<CGargDMOProp>
{
public:

	enum {IDD = IDD_GARGDMOPROP};

DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_REGISTRY_RESOURCEID(IDR_GARGDMOPROP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGargDMOProp) 
	COM_INTERFACE_ENTRY(IPropertyPage)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

BEGIN_MSG_MAP(CGargDMOProp)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
	CHAIN_MSG_MAP(IPropertyPageImpl<CGargDMOProp>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    CGargDMOProp();

    HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

    STDMETHOD(SetObjects)(ULONG nObjects, IUnknown **ppUnk);
    STDMETHOD(Apply)(void);

    // Message handlers
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnControlMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    // Member variables
    CSliderValue m_sliderRate;
    CRadioChoice m_radioWaveform;
    Handler *m_rgpHandlers[3];
    CComPtr<IGargleDMOSample> m_pIGargleFX;

};

#endif //__GARGDMOPROP_H_
