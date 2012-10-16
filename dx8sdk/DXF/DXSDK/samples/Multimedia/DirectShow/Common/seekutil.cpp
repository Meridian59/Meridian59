//------------------------------------------------------------------------------
// File: SeekUtil.cpp
//
// Desc: DirectShow sample code - utility functions.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include <dshow.h>

//
// Constants
//
const int TICKLEN=100, TIMERID=55;

//
// Global data
//
static REFERENCE_TIME g_rtTotalTime=0;
static UINT_PTR g_wTimerID=0;
static HWND g_hwnd=0;

HRESULT ConfigureSeekbar(IMediaSeeking *pMS, CSliderCtrl Seekbar, CStatic& strPosition);
void StartSeekTimer();
void StopSeekTimer();
void UpdatePosition(IMediaSeeking *pMS, REFERENCE_TIME rtNow, CStatic& strPosition);
void ReadMediaPosition(IMediaSeeking *pMS, CSliderCtrl& Seekbar, CStatic& strPosition);
void HandleTrackbar(IMediaControl *pMC, IMediaSeeking *pMS, 
                    CSliderCtrl& Seekbar, CStatic& strPosition, WPARAM wReq);


HRESULT ConfigureSeekbar(IMediaSeeking *pMS, CSliderCtrl Seekbar, CStatic& strPosition, HWND hwndOwner)
{
    HRESULT hr;

    // Disable seekbar for new file and reset tracker/position label
    Seekbar.SetPos(0);
    Seekbar.EnableWindow(FALSE);
    g_rtTotalTime=0;

    strPosition.SetWindowText(TEXT("Position: 00m:00s\0"));

    DWORD dwSeekCaps = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanGetDuration;

    // Can we seek this file?  If so, enable trackbar.
    if (pMS && (S_OK == pMS->CheckCapabilities(&dwSeekCaps))) 
    {
        hr = pMS->GetDuration(&g_rtTotalTime);
        Seekbar.EnableWindow(TRUE);
    }

    g_hwnd = hwndOwner;

    return hr;
}

void StartSeekTimer() 
{
    // Cancel any pending timer event
    StopSeekTimer();

    // Create a new timer
    g_wTimerID = SetTimer(g_hwnd, TIMERID, TICKLEN, NULL);
}

void StopSeekTimer() 
{
    // Cancel the timer
    if(g_wTimerID)        
    {                
        KillTimer(g_hwnd, g_wTimerID);
        g_wTimerID = 0;
    }
}


void ReadMediaPosition(IMediaSeeking *pMS, CSliderCtrl& Seekbar, CStatic& strPosition)
{
    HRESULT hr;
    REFERENCE_TIME rtNow;

    // Read the current stream position
    hr = pMS->GetCurrentPosition(&rtNow);
    if (FAILED(hr))
        return;

    // Convert position into a percentage value and update slider position
    long lTick = (long)((rtNow * 100) / g_rtTotalTime);
    Seekbar.SetPos(lTick);
	
    // Update the 'current position' string on the main dialog
    UpdatePosition(pMS, rtNow, strPosition);
}


void UpdatePosition(IMediaSeeking *pMS, REFERENCE_TIME rtNow, CStatic& strPosition) 
{
    HRESULT hr;

    // If no reference time was passed in, read the current position
    if (rtNow == 0)
    {
        // Read the current stream position
        hr = pMS->GetCurrentPosition(&rtNow);
        if (FAILED(hr))
            return;
    }

    // Convert the LONGLONG duration into human-readable format
    unsigned long nTotalMS = (unsigned long) rtNow / 10000; // 100ns -> ms
    int nSeconds = nTotalMS / 1000;
    int nMinutes = nSeconds / 60;
    nSeconds %= 60;

    // Update the display
    TCHAR szPosition[24];
    wsprintf(szPosition, _T("Position: %02dm:%02ds\0"), nMinutes, nSeconds);
    strPosition.SetWindowText(szPosition);
}


void HandleTrackbar(IMediaControl *pMC, IMediaSeeking *pMS, 
                    CSliderCtrl& Seekbar, CStatic& strPosition, WPARAM wReq)
{
    HRESULT hr;
    static OAFilterState state;
    static BOOL bStartOfScroll = TRUE;

    // If the file is not seekable, the trackbar is disabled. 
    DWORD dwPosition = Seekbar.GetPos();

    // Pause when the scroll action begins.
    if (bStartOfScroll) 
    {       
        hr = pMC->GetState(10, &state);
        bStartOfScroll = FALSE;
        hr = pMC->Pause();
    }
    
    // Update the position continuously.
    REFERENCE_TIME rtNew = (g_rtTotalTime * dwPosition) / 100;

    hr = pMS->SetPositions(&rtNew, AM_SEEKING_AbsolutePositioning,
                           NULL,   AM_SEEKING_NoPositioning);

    // Restore the state at the end.
    if (wReq == TB_ENDTRACK)
    {
        if (state == State_Stopped)
            hr = pMC->Stop();
        else if (state == State_Running) 
            hr = pMC->Run();

        bStartOfScroll = TRUE;
    }

    // Update the 'current position' string on the main dialog.
    UpdatePosition(pMS, rtNew, strPosition);
}

