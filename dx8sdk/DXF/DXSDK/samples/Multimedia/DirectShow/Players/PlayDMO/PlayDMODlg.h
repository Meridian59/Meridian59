//------------------------------------------------------------------------------
// File: PlayDMODlg.h
//
// Desc: DirectShow sample code - main header for CPlayDMODlg
//
// Copyright (c) 1996-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_PLAYDMODLG_H__AD2B59C5_9A0D_438F_A912_5441F8FC8B79__INCLUDED_)
#define AFX_PLAYDMODLG_H__AD2B59C5_9A0D_438F_A912_5441F8FC8B79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dshow.h>
#include <streams.h>
#include <qedit.h>
#include <atlbase.h>
#include <dmo.h>
#include <dmodshow.h>

// Include utility headers
#include "mfcutil.h"
#include "mfcdmoutil.h"
#include "dshowutil.h"

// Constants
#define MAX_DMOS    10


/////////////////////////////////////////////////////////////////////////////
// CPlayDMODlg dialog

class CPlayDMODlg : public CDialog
{
// Construction
public:
	CPlayDMODlg(CWnd* pParent = NULL);	// standard constructor

    HRESULT FillLists(void);
    void ClearLists(void);
    void SetDefaults(void);
    void EnableButtons(BOOL bEnable);

    HRESULT GetInterfaces(void);
    void FreeInterfaces(void);

    HRESULT HandleGraphEvent(void);
    HRESULT SetInputPinProperties(IAMAudioInputMixer *pPinMixer);
    HRESULT SetAudioProperties(void);
    HRESULT AddDMOsToGraph(void);
    HRESULT RemoveDMOsFromGraph(void);

    void ClearAllocatedLists(void);
    void Say(TCHAR *szMsg);
    void ShowInputBufferInfo(IMediaObject *pDMO, int nSel);
    void ShowOutputBufferInfo(IMediaObject *pDMO, int nSel);

    HRESULT PrepareMedia(LPTSTR lpszMovie);
    HRESULT InitDirectShow(void);
    HRESULT FreeDirectShow(void);

    void ResetDirectShow(void);
    void CenterVideo(void);

    HRESULT RunMedia(void);
    HRESULT StopMedia(void);
    HRESULT PauseMedia(void);
    HRESULT OnSelectFile();
    HRESULT ConnectDMOsToRenderer() ;

    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Dialog Data
	//{{AFX_DATA(CPlayDMODlg)
	enum { IDD = IDD_PLAYDMO_DIALOG };
	CButton	m_btnProperties;
	CStatic	m_strStatus;
	CStatic	m_nOutBufferSize;
	CStatic	m_nOutAlignment;
	CStatic	m_nMaxLatency;
	CStatic	m_nInBufferSize;
	CStatic	m_nInLookahead;
	CStatic	m_nInAlignment;
	CListBox	m_ListFilters;
	CListBox	m_ListFilterOutputs;
	CListBox	m_ListFilterInputs;
	CListBox	m_ListAudioDMO;
	CEdit	m_StrFilename;
	CButton	m_btnStop;
	CButton	m_btnPlay;
	CButton	m_btnPause;
	CStatic	m_Screen;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayDMODlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    DWORD m_dwRegister;     // Used by debug ROT functions

    unsigned long m_ulDataBuffered;
    int m_nDMOCount;
    int m_nLoadedDMOs;

    // DirectShow interfaces
    IGraphBuilder *m_pGB;
    IMediaSeeking *m_pMS;
    IMediaControl *m_pMC;
    IMediaEventEx *m_pME;
    IVideoWindow  *m_pVW;

    // List of DMO interfaces
    IBaseFilter *m_pDMOList[MAX_DMOS];

    BOOL m_bAudioOnly;
    FILTER_STATE g_psCurrent;

	// Generated message map functions
	//{{AFX_MSG(CPlayDMODlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
    afx_msg void OnDestroy();
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonPause();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonFile();
	afx_msg void OnSelchangeListFilters();
	afx_msg void OnButtonAddDmo();
	afx_msg void OnButtonClear();
	afx_msg void OnButtonProppage();
	afx_msg void OnDblclkListFilters();
	afx_msg BOOL OnEraseBkgnd(CDC *);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYDMODLG_H__AD2B59C5_9A0D_438F_A912_5441F8FC8B79__INCLUDED_)
