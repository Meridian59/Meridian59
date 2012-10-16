//------------------------------------------------------------------------------
// File: JukeboxASFDlg.h
//
// Desc: DirectShow sample code - main dialog header file for the Jukebox
//       application.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_JUKEBOXDLG_H__04AD8433_DF22_4491_9611_260EDAE17B96__INCLUDED_)
#define AFX_JUKEBOXDLG_H__04AD8433_DF22_4491_9611_260EDAE17B96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dshow.h>
#include "keyprovider.h"

//
// Constants
//
const int TICKLEN=100, TIMERID=55;


/////////////////////////////////////////////////////////////////////////////
// CJukeboxDlg dialog

class CJukeboxDlg : public CDialog
{
// Construction
public:
	CJukeboxDlg(CWnd* pParent = NULL);	// standard constructor
    void FillFileList(LPTSTR pszCmdLine);

    HRESULT PrepareMedia(LPTSTR lpszMovie);
    BOOL DisplayFileInfo(LPTSTR szFile);
    HRESULT DisplayFileDuration(void);
    BOOL DisplayImageInfo(void);
    void Say(LPTSTR szText);
    BOOL IsWindowsMediaFile(LPTSTR lpszFile);

    LONG GetDXMediaPath(TCHAR *strPath);
    LONG GetGraphEditPath(TCHAR *szPath);
    void InitMediaDirectory(void);

    HRESULT RenderWMFile(LPCWSTR wFile);
    HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter);
    HRESULT AddKeyProvider(IGraphBuilder *pGraph);
    HRESULT RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pReader);

    HRESULT InitDirectShow(void);
    HRESULT FreeDirectShow(void);
    HRESULT HandleGraphEvent(void);

    void ResetDirectShow(void);
    void DisplayECEvent(long lEventCode, long lParam1, long lParam2);
    void CenterVideo(void);
    void PlayNextFile(void);
    void PlayPreviousFile(void);
    void PlaySelectedFile(void);
    void ShowState(void);
    void ConfigureSeekbar(void);
    void StartSeekTimer(void);
    void StopSeekTimer(void);
    void HandleTrackbar(WPARAM wReq);
    void UpdatePosition(REFERENCE_TIME rtNow);
    void ReadMediaPosition(void);

    BOOL CanStep(void);
    HRESULT StepFrame(void);
    HRESULT EnumFilters(void);
    HRESULT EnumPins(IBaseFilter *pFilter, PIN_DIRECTION PinDir, CListBox& Listbox);
    IBaseFilter * FindFilterFromName(LPTSTR szName);
    BOOL SupportsPropertyPage(IBaseFilter *pFilter);

    void CALLBACK MediaTimer(UINT wTimerID, UINT msg, ULONG dwUser, ULONG dw1, ULONG dw2);

    // Dialog Data
	//{{AFX_DATA(CJukeboxDlg)
	enum { IDD = IDD_JUKEBOX_DIALOG };
	CStatic	m_StrPosition;
	CSliderCtrl	m_Seekbar;
	CStatic	m_StrImageSize;
	CStatic	m_StrDuration;
	CEdit	m_EditMediaDir;
	CSpinButtonCtrl	m_SpinFiles;
	CButton	m_ButtonFrameStep;
	CListBox	m_ListEvents;
	CButton	m_CheckEvents;
	CButton	m_ButtonProperties;
	CStatic	m_StrMediaPath;
	CButton	m_CheckMute;
	CButton	m_ButtonStop;
	CButton	m_ButtonPlay;
	CButton	m_ButtonPause;
	CButton	m_CheckPlaythrough;
	CButton	m_CheckLoop;
	CStatic	m_StrFileDate;
	CStatic	m_StrFileSize;
	CListBox	m_ListPinsOutput;
	CListBox	m_ListPinsInput;
	CStatic	m_StrFileList;
	CStatic	m_Status;
	CStatic	m_Screen;
	CListBox	m_ListInfo;
	CListBox	m_ListFilters;
	CListBox	m_ListFiles;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJukeboxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    int   m_nCurrentFileSelection;
    REFERENCE_TIME g_rtTotalTime;
    UINT_PTR g_wTimerID;
    TCHAR m_szCurrentDir[MAX_PATH];

    // Global key provider object created/released during the
    // Windows Media graph-building stage.
    CKeyProvider prov;

	// Generated message map functions
	//{{AFX_MSG(CJukeboxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnSelectFile();
	afx_msg void OnPause();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnCheckMute();
	afx_msg void OnCheckLoop();
	afx_msg void OnCheckPlaythrough();
	afx_msg void OnSelchangeListFilters();
	afx_msg void OnDblclkListFilters();
	afx_msg void OnButtonProppage();
	afx_msg void OnCheckEvents();
	afx_msg void OnButtonFramestep();
	afx_msg void OnButtonClearEvents();
	afx_msg void OnDblclkListFiles();
	afx_msg void OnDeltaposSpinFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonSetMediadir();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonGraphedit();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC *);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JUKEBOXDLG_H__04AD8433_DF22_4491_9611_260EDAE17B96__INCLUDED_)
