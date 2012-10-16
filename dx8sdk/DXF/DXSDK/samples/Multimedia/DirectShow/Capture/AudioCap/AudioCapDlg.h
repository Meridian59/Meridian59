//------------------------------------------------------------------------------
// File: AudioCapDlg.h
//
// Desc: DirectShow sample code - Main header for CAudioCapDlg class
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_AUDIOCAPDLG_H__CC9BF075_154D_430A_9E58_7311EC641C61__INCLUDED_)
#define AFX_AUDIOCAPDLG_H__CC9BF075_154D_430A_9E58_7311EC641C61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dshow.h>

//
//  Constants
//
#define DEFAULT_BUFFER_TIME ((float) 0.05)  /* 50 milliseconds*/
#define DEFAULT_FILENAME    TEXT("c:\\test.wav\0")
#define RECORD_EVENT        0
#define PLAYBACK_EVENT      1

static const GUID CLSID_WavDest =
{ 0x3c78b8e2, 0x6c4d, 0x11d1, { 0xad, 0xe2, 0x0, 0x0, 0xf8, 0x75, 0x4b, 0x99 } };


/////////////////////////////////////////////////////////////////////////////
// CAudioCapDlg dialog

class CAudioCapDlg : public CDialog
{
// Construction
public:
	CAudioCapDlg(CWnd* pParent = NULL);	// standard constructor

    HRESULT FillLists(void);
    void ClearLists(void);
    void SetDefaults(void);
    void EnableButtons(BOOL bEnable);
    void EnableRadioButtons(BOOL bEnable);
    HRESULT InitializeCapture(void);
    HRESULT GetInterfaces(void);
    HRESULT GetPlaybackInterfaces();
    void FreeInterfaces(void);
    void FreePlaybackInterfaces(void);
    void ReleaseCapture(void);
    void ResetCapture(void);
    void UpdateFilterLists(IGraphBuilder *pGB);
    HRESULT HandleGraphEvent(void);
    HRESULT HandlePlaybackGraphEvent(void);
    HRESULT SetInputPinProperties(IAMAudioInputMixer *pPinMixer);
    HRESULT SetAudioProperties();
    HRESULT ActivateSelectedInputPin(void);
    HRESULT RenderPreviewStream(void);
    HRESULT RenderCaptureStream(void);
    HRESULT DestroyPreviewStream(void);
    HRESULT DestroyCaptureStream(void);
    void ClearAllocatedLists(void);
    void Say(TCHAR *szMsg);

// Dialog Data
	//{{AFX_DATA(CAudioCapDlg)
	enum { IDD = IDD_AUDIOCAP_DIALOG };
	CStatic	m_strStatus;
	CButton	m_btnMono;
	CButton	m_btn8BIT;
	CButton	m_btn11KHZ;
	CButton	m_btnPause;
	CButton	m_btnStop;
	CButton	m_btnPlay;
	CListBox	m_ListFilterOutputs;
	CListBox	m_ListFilterInputs;
	CButton	m_btnRecord;
	CButton	m_btnProperties;
	CListBox	m_ListInputs;
	CListBox	m_ListInputPins;
	CListBox	m_ListFilters;
	CEdit	m_StrFilename;
	CButton	m_CheckWriteFile;
	CButton	m_CheckAudition;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioCapDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    BOOL m_bPausedRecording;
    IGraphBuilder *m_pGB, *m_pGBPlayback;
    IMediaControl *m_pMC, *m_pMCPlayback;
    IMediaEventEx *m_pME, *m_pMEPlayback;

    ICaptureGraphBuilder2 *m_pCapture;

    IBaseFilter *m_pInputDevice, *m_pRenderer, *m_pFileWriter,
                *m_pSplitter,    *m_pWAVDest;

	// Generated message map functions
	//{{AFX_MSG(CAudioCapDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnButtonFile();
	afx_msg void OnSelchangeListInputDevices();
	afx_msg void OnSelchangeListFilters();
	afx_msg void OnButtonClear();
	afx_msg void OnSelchangeListInputPins();
	afx_msg void OnButtonProperties();
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonPause();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonRecord();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOCAPDLG_H__CC9BF075_154D_430A_9E58_7311EC641C61__INCLUDED_)
