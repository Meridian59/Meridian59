//------------------------------------------------------------------------------
// File: VMRCore.h
//
// Desc: DirectShow sample code
//       Header file and class description for CVMRCore, 
//       "main" module to manage VMR and its interfaces
//       This class is called from CDemonstration.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_NEWCORE_H__9D74D6FC_F94C_45E6_A991_E38D47C3441D__INCLUDED_)
#define AFX_NEWCORE_H__9D74D6FC_F94C_45E6_A991_E38D47C3441D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

class CVMRMixDlg;

class CVMRCore  
{
public: 
    CVMRCore(CVMRMixDlg * pDlg, CMediaList *pML);

    
    CVMRCore(   CVMRMixDlg * pDlg, 
                DWORD dwVMRMode, 
                DWORD dwVMRPrefs, 
                CMediaList *pML);
                

    virtual ~CVMRCore();

    HWND GetClientHwnd();
    virtual bool ListFilters();

    HRESULT Pause();
    HRESULT Stop();
    HRESULT Play(bool bDoNotRunYet = false);

    bool IsActive();

    // functions to get information about the video playback window
    void SetAbort();
    void SetHwnd(HWND hwnd){m_hwnd = hwnd;};
    bool SetClientVideo();
    CVMRMixDlg * GetDlg(){ return m_pDlg;};
    
    // windows procedure function for the windowless control
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    char m_szEventStopTest[MAX_PATH];
    char m_szEventCloseWindow[MAX_PATH];
    char m_szEventKillCore[MAX_PATH];
    char m_szEventResumeCore[MAX_PATH];

    // access to interfaces and member variables
    IVMRWindowlessControl *     GetVMRWndless(){return m_pIWndless;};
    IGraphBuilder *             GetIGraphBuilder(){return m_pGraph;};
    IBaseFilter *               GetTestFilter(){return m_pTestFilter;};
    IVMRFilterConfig *          GetVMRConfig() { return m_pConfig; };
    IVMRMonitorConfig *         GetVMRMonitorConfig() { return m_pIMonConfig; };
    IMediaControl *             GetMediaControl(){return m_pIMC;};
    IVMRMixerControl *          GetMixerControl(){ return m_pIMixerControl;};
    IMediaSeeking *             GetMediaSeeking() { return m_pIMediaSeeking;};
    IVMRMixerBitmap *           GetMixerBitmap(){ return m_pMixerBitmap;};
    
protected:
    // protected member variables
    HWND m_hwnd;

    IGraphBuilder *         m_pGraph;     // graph 
    IBaseFilter *           m_pTestFilter;// the renderer filter
    IVMRWindowlessControl * m_pIWndless;  
    IMediaControl *         m_pIMC;       
    IVMRFilterConfig *      m_pConfig;  
    IVMRMonitorConfig *     m_pIMonConfig;
    IVideoWindow *          m_pIVidWindow;
    IVMRMixerControl *      m_pIMixerControl;
    IMediaSeeking *         m_pIMediaSeeking;
    IVMRMixerBitmap *       m_pMixerBitmap;

    DWORD m_dwVMRMode;              // VMR Mode setup flags
    DWORD m_dwVMRPrefs;             // VMR Prefs setup
    LPRECT m_lpSrcRect;             // pointer to SRC rect for painting
    LPRECT m_lpDestRect;            // pointer to Dest rect for painting

    // working functions

    virtual HRESULT CreateGraph();
    virtual HRESULT AddTestFilter();
    virtual HRESULT InitRelevantInterfaces();
    HRESULT CreateWindowlessWindow();
    virtual void ReleaseInterfaces();

    virtual LRESULT OnPaint(HWND hWnd);
    virtual LRESULT OnSize(LPARAM lParam, WPARAM wParam);

private:
    DWORD           m_tID;  // thread identifier for windless mode
    DWORD           m_dwID; // ID of this copy of CVMRCore. Generated as rand();
    CVMRMixDlg *    m_pDlg; // 'parent' dialog
    CMediaList *    m_pML;  // media list to render
    int             m_nConnectedPins;   // number of active input pins of VMR
    HANDLE m_hWinThread;    // windowless control's window thread
    
    HANDLE m_hEventStopTest;
    HANDLE m_hEventCloseWindow;
    HANDLE m_hEventKillCore;
    HANDLE m_hEventResumeCore;      // control window

    // private working functions
    bool IsRenderer(char * strFilterName);  // determines if a filter is a video renderer
    LRESULT KillWindow();                   // process closing window by user   
};

#endif // !defined(AFX_NEWCORE_H__9D74D6FC_F94C_45E6_A991_E38D47C3441D__INCLUDED_)
