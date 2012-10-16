//------------------------------------------------------------------------------
// File: GargDMOProp.cpp
//
// Desc: DirectShow sample code - implementation of CGargDMOProp class.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#define FIX_LOCK_NAME
#include <param.h>
#include <dmobase.h>
#include "gargle.h"
#include "GargDMOProp.h"

/////////////////////////////////////////////////////////////////////////////
// CGargDMOProp

const CRadioChoice::ButtonEntry g_rgWaveButtons[] =
    {
        IDC_RADIO_TRIANGLE, GARGLE_FX_WAVE_TRIANGLE,
        IDC_RADIO_SQUARE, GARGLE_FX_WAVE_SQUARE,
        0
    };

CGargDMOProp::CGargDMOProp()
  : m_radioWaveform(g_rgWaveButtons),
    m_pIGargleFX( NULL )

{
	m_pUnkMarshaler = NULL;
	m_dwTitleID = IDS_TITLEGargDMOProp;
	m_dwHelpFileID = IDS_HELPFILEGargDMOProp;
	m_dwDocStringID = IDS_DOCSTRINGGargDMOProp;

    m_rgpHandlers[0] = &m_sliderRate;
    m_rgpHandlers[1] = &m_radioWaveform;
    m_rgpHandlers[2] = NULL;
}

STDMETHODIMP CGargDMOProp::SetObjects(ULONG nObjects, IUnknown **ppUnk)
{
    if (nObjects < 1 || nObjects > 1)
        return E_UNEXPECTED;

    HRESULT hr = ppUnk[0]->QueryInterface(IID_IGargleDMOSample, reinterpret_cast<void**>(&m_pIGargleFX));
    return hr;
}

STDMETHODIMP CGargDMOProp::Apply(void)
{
    if (!m_pIGargleFX)
        return E_UNEXPECTED;

    GargleFX garglefx;
    ZeroMemory(&garglefx, sizeof(GargleFX));

    garglefx.dwRateHz = static_cast<DWORD>(m_sliderRate.GetValue());
    garglefx.dwWaveShape = m_radioWaveform.GetChoice(*this);

    HRESULT hr = m_pIGargleFX->SetAllParameters(&garglefx);
    if (FAILED(hr))
        return hr;

    m_bDirty = FALSE;
    return S_OK;
}

LRESULT CGargDMOProp::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!m_pIGargleFX)
        return 1;

    GargleFX garglefx;
    ZeroMemory(&garglefx, sizeof(GargleFX));
    m_pIGargleFX->GetAllParameters(&garglefx);

    m_sliderRate.Init(GetDlgItem(IDC_SLIDER_Rate), GetDlgItem(IDC_EDIT_Rate), 1, 1000, true);
    m_sliderRate.SetValue(static_cast<float>(garglefx.dwRateHz));

    m_radioWaveform.SetChoice(*this, garglefx.dwWaveShape);

    return 1;
}

LRESULT CGargDMOProp::OnControlMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lr = MessageHandlerChain(m_rgpHandlers, uMsg, wParam, lParam, bHandled);

    if (bHandled)
        SetDirty(TRUE);
    return lr;
}


