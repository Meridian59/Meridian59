//------------------------------------------------------------------------------
// File: StillViewDlg.h
//
// Desc: DirectShow sample code - dialog header file for the StillView
//       application.
//
// Copyright (c) 1998 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_StillViewDLG_H__04AD8433_DF22_4491_9611_260EDAE17B96__INCLUDED_)
#define AFX_StillViewDLG_H__04AD8433_DF22_4491_9611_260EDAE17B96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dshow.h>


// Application-defined messages
#define WM_GRAPHNOTIFY  WM_APP + 1
#define WM_FIRSTFILE    WM_APP + 2
#define WM_PLAYFILE     WM_APP + 3
#define WM_NEXTFILE     WM_APP + 4
#define WM_PREVIOUSFILE WM_APP + 5

//
// Macros
//
#define SAFE_RELEASE(i) {if (i) i->Release(); i = NULL;}

#define JIF(x) if (FAILED(hr=(x))) \
    {RetailOutput(TEXT("FAILED(0x%x) ") TEXT(#x) TEXT("\n"), hr); goto CLEANUP;}

// Function prototypes
void RetailOutput(TCHAR *tszErr, ...);


/////////////////////////////////////////////////////////////////////////////
// CStillViewDlg dialog

class CStillViewDlg : public CDialog
{
// Construction
public:
	CStillViewDlg(CWnd* pParent = NULL);	// standard constructor

    HRESULT PrepareMedia(LPTSTR lpszMovie);
    HRESULT InitDirectShow(void);
    HRESULT FreeDirectShow(void);
    HRESULT HandleGraphEvent(void);
    HRESULT EnumFilters(void);
    HRESULT EnumPins(IBaseFilter *pFilter, PIN_DIRECTION PinDir, CListBox& Listbox);

    IBaseFilter * FindFilterFromName(LPTSTR szName);

    BOOL DisplayFileInfo(LPTSTR szFile);
    BOOL DisplayImageInfo(void);
    BOOL SupportsPropertyPage(IBaseFilter *pFilter);

    LONG GetDXMediaPath(TCHAR *strPath);
    LONG GetGraphEditPath(TCHAR *szPath);
    void InitMediaDirectory(void);
    void FillFileList(LPTSTR pszCmdLine);

    void ResetDirectShow(void);
    void CenterVideo(void);
    void PlayNextFile(void);
    void PlayPreviousFile(void);
    void PlaySelectedFile(void);

    // Dialog Data
	//{{AFX_DATA(CStillViewDlg)
	enum { IDD = IDD_STILLVIEW_DIALOG };
	CStatic	m_StrFileTime;
	CStatic	m_StrImageSize;
	CEdit	m_EditMediaDir;
	CSpinButtonCtrl	m_SpinFiles;
	CButton	m_ButtonProperties;
	CStatic	m_StrMediaPath;
	CStatic	m_StrFileDate;
	CStatic	m_StrFileSize;
	CListBox    m_ListPinsOutput;
	CListBox	m_ListPinsInput;
	CStatic	m_StrFileList;
	CStatic	m_Screen;
	CListBox	m_ListInfo;
	CListBox	m_ListFilters;
	CListBox	m_ListFiles;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStillViewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    int   m_nCurrentFileSelection;
    TCHAR m_szCurrentDir[MAX_PATH];

    IGraphBuilder *m_pGB;
    IMediaControl *m_pMC;
    IBasicVideo   *m_pBV;
    IVideoWindow  *m_pVW;
    IMediaEventEx *m_pME;

	// Generated message map functions
	//{{AFX_MSG(CStillViewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSelectFile();
	afx_msg void OnSelchangeListFilters();
	afx_msg void OnDblclkListFilters();
	afx_msg void OnButtonProppage();
	afx_msg void OnDeltaposSpinFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonSetMediadir();
	afx_msg void OnButtonGraphedit();
	afx_msg BOOL OnEraseBkgnd(CDC *);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_StillViewDLG_H__04AD8433_DF22_4491_9611_260EDAE17B96__INCLUDED_)
