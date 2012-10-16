//------------------------------------------------------------------------------
// File: CompositeControl.cpp
//
// Desc: Implementation of the CCompositeControl
//       for the Windows XP MSVidCtl C++ sample
//
// Copyright (c) 2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "CPPVideoControl.h"
#include "CompositeControl.h"

#include <atlbase.h>
#include <msvidctl.h>
#include <tuner.h>
#include <segment.h>

#define  DEFAULT_CHANNEL 46

#define  STR_VIEW_FAILURE  TEXT("Failed IMSVidCtl::View.  You may not have ") \
             TEXT("properly installed your hardware.  Your ATSC tuner card, ") \
             TEXT("MPEG-2 decoder, or video card may be incompatible with ")   \
             TEXT("the MicrosoftTV Technologies architecture.")


/////////////////////////////////////////////////////////////////////////////
// CCompositeControl

LRESULT CCompositeControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    ITuningSpace * pTuningSpace = NULL;
    IEnumTuningSpaces * pEnumTuningSpaces = NULL;
    ITuningSpace ** pTuningSpaceArray = NULL;
    ULONG ulNumFetched = 0;
    long lCount = 0;
    ULONG i = 0;
    CComBSTR bstrATSC = L"ATSC";
    CComBSTR bstrTemp = L"";

    // Get window handle of channel label (for later update)
    m_hwndChannelID = GetDlgItem(IDC_CHANNELID);

    // Get the tuning space collection
    hr = CoCreateInstance(CLSID_SystemTuningSpaces, NULL, 
            CLSCTX_INPROC_SERVER, IID_ITuningSpaceContainer,
            reinterpret_cast<void**>(&m_pTuningSpaceContainer));
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to create system tuning space."), TEXT("Error"), MB_OK);
        return hr;
    }
  
    // Get the video control object
    hr = GetDlgControl(IDC_VIDEOCONTROL1,IID_IMSVidCtl, reinterpret_cast<void **>(&m_pMSVidCtl));
    if(m_pMSVidCtl == NULL) 
    {
        MessageBox(TEXT("Failed to get Video Control on main dialog!"),TEXT("Error"),MB_OK);
        return hr;
    }

    // Find the ATSC tuning space in the collection
    hr = m_pTuningSpaceContainer->get_EnumTuningSpaces(&pEnumTuningSpaces);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to get tuning space enumerator."), TEXT("Error"), MB_OK);
        return hr;
    }
    hr = m_pTuningSpaceContainer->get_Count(&lCount);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to enumerate tuning spaces."), TEXT("Error"), MB_OK);
        return hr;
    }

    // Read tuning space info into allocated array
    pTuningSpaceArray = new ITuningSpace*[lCount];
    hr = pEnumTuningSpaces->Next(lCount, pTuningSpaceArray, &ulNumFetched);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to read tuning spaces."), TEXT("Error"), MB_OK);
        return hr;
    }

    // Find the ATSC tuning space
    for (i = 0;i < ulNumFetched; i++)
    {
        hr = pTuningSpaceArray[i]->get_UniqueName(&bstrTemp);
        if (FAILED(hr))
        {
            MessageBox(TEXT("Failed to read tuning space unique name."), TEXT("Error"), MB_OK);
            return hr;
        }

        // Is this the ATSC tuning space?
        if (bstrTemp == bstrATSC)
        {
            hr = pTuningSpaceArray[i]->Clone(&pTuningSpace);
            break;
        }
    }

    if (pTuningSpace == NULL)
    {
        MessageBox(TEXT("Could not find ATSC tuning space."), TEXT("Error"), MB_OK);
        return hr;
    }
    
    // QI for IATSCTuningSpace
    hr = pTuningSpace->QueryInterface(IID_IATSCTuningSpace, reinterpret_cast<void**>(&m_pATSCTuningSpace));
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to QI for IATSCTuningSpace."), TEXT("Error"), MB_OK);
        return hr;
    }

    // Create ATSC Locator
    hr = CoCreateInstance(CLSID_ATSCLocator, NULL, 
            CLSCTX_INPROC_SERVER, IID_IATSCLocator,
            reinterpret_cast<void**>(&m_pATSCLocator));
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to create ATSC locator."), TEXT("Error"), MB_OK);
        return hr;
    }

    hr = SetChannel(DEFAULT_CHANNEL);
    if (FAILED(hr))
    {
        // SetChannel will give a message box indicating the error
        return hr;
    }

    // Start viewing digital TV
    hr = m_pMSVidCtl->Run();
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed IMSVidCtl::Run.  You may not have properly installed your hardware.  Your ATSC tuner card, MPEG-2 decoder, or video card may be incompatible with the MicrosoftTV Technologies architecture."), TEXT("Error"), MB_OK);
        return hr;
    }

    SAFE_RELEASE(pTuningSpace);
    SAFE_RELEASE(pEnumTuningSpaces);
    delete pTuningSpaceArray;

    return hr;
};


LRESULT CCompositeControl::OnClickedChanneldown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    long lPhysicalChannel = 0;
    
    // Get the current physical channel and decrement it
    hr =m_pATSCLocator->get_PhysicalChannel(&lPhysicalChannel);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to read physical channel."), TEXT("Error"), MB_OK);
        return hr;
    }
    lPhysicalChannel--;

    hr = SetChannel(lPhysicalChannel);
    return hr;
};

LRESULT CCompositeControl::OnClickedChannelup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    long lPhysicalChannel = 0;
    
    // Get the current physical channel and increment it
    hr =m_pATSCLocator->get_PhysicalChannel(&lPhysicalChannel);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to read physical channel."), TEXT("Error"), MB_OK);
        return hr;
    }
    lPhysicalChannel++;

    hr = SetChannel(lPhysicalChannel);
    return hr;
};

HRESULT CCompositeControl::SetChannel(long lPhysicalChannel)
{
    HRESULT hr=S_OK;
    IATSCChannelTuneRequest * pATSCTuneRequest = NULL;
    ITuneRequest * pTuneRequest = NULL;

    // Set the Physical Channel
    hr =m_pATSCLocator->put_PhysicalChannel(lPhysicalChannel);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to set physical channel."), TEXT("Error"), MB_OK);
        return hr;
    }

    // Create an ATSC tune request
    hr = m_pATSCTuningSpace->CreateTuneRequest(&pTuneRequest);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to create tune request."), TEXT("Error"), MB_OK);
        return hr;
    }

    hr = pTuneRequest->QueryInterface(IID_IATSCChannelTuneRequest, reinterpret_cast<void**>(&pATSCTuneRequest));
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to query for IATSCChannelTuneRequest."), TEXT("Error"), MB_OK);
        return hr;
    }

    // Set the Channel and MinorChannel property on the tune request to -1
    // These properties will get set by the network provider once tuned to a ATSC channel
    hr = pATSCTuneRequest->put_Channel(-1);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to put channel property."), TEXT("Error"), MB_OK);
        return hr;
    }
    hr = pATSCTuneRequest->put_MinorChannel(-1);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to put minor channel property."), TEXT("Error"), MB_OK);
        return hr;
    }
    hr = pATSCTuneRequest->put_Locator(m_pATSCLocator);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Failed to put locator."), TEXT("Error"), MB_OK);
        return hr;
    }

    // Now that the tune request is configured, pass it to the video control
    CComVariant var = pATSCTuneRequest;
    hr = m_pMSVidCtl->View(&var);
    if (FAILED(hr))
    {
        MessageBox(STR_VIEW_FAILURE, TEXT("Error"), MB_OK);
        return hr;
    }

    ShowChannelNumber(lPhysicalChannel);

    // Release interfaces
    pATSCTuneRequest->Release();
    pTuneRequest->Release();
    return hr;
}

void CCompositeControl::ShowChannelNumber(long lChannel)
{
    TCHAR szChannelNumber[8];
    wsprintf(szChannelNumber, TEXT("%d\0"), lChannel);
    SetDlgItemText(IDC_CHANNELID, szChannelNumber);
}

