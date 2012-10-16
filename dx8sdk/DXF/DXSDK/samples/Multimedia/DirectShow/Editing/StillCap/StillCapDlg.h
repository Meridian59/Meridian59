//------------------------------------------------------------------------------
// File: StillCapDlg.h
//
// Desc: DirectShow sample code - definition of callback and dialog
//       classes for StillCap application.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#if !defined(AFX_STILLCAPDLG_H__3067E9D2_B94C_4ED1_99AB_53034129A0DD__INCLUDED_)
#define AFX_STILLCAPDLG_H__3067E9D2_B94C_4ED1_99AB_53034129A0DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CStillCapDlg dialog

class CSampleGrabberCB;

class CStillCapDlg : public CDialog
{
    friend class CSampleGrabberCB;

protected:

    // either the capture live graph, or the capture still graph
    CComPtr< IGraphBuilder > m_pGraph;

    // the playback graph when capturing video
    CComPtr< IGraphBuilder > m_pPlayGraph;

    // the sample grabber for grabbing stills
    CComPtr< ISampleGrabber > m_pGrabber;

    // if you're in still mode or capturing video mode
    bool m_bCapStills;

    // when in video mode, whether capturing or playing back
    int m_nCapState;

    // how many times you've captured
    int m_nCapTimes;

    void GetDefaultCapDevice( IBaseFilter ** ppCap );
    HRESULT InitStillGraph( );
    HRESULT InitCaptureGraph( TCHAR * pFilename );
    HRESULT InitPlaybackGraph( TCHAR * pFilename ); 
    void ClearGraphs( );
    void UpdateStatus(TCHAR *szStatus);
    void Error( TCHAR * pText );

// Construction
public:
	CStillCapDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CStillCapDlg)
	enum { IDD = IDD_STILLCAP_DIALOG };
	CStatic	m_StrStatus;
	CStatic	m_StillScreen;
	CStatic	m_PreviewScreen;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStillCapDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CStillCapDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSnap();
	afx_msg void OnCapstills();
	afx_msg void OnCapvid();
	afx_msg void OnButtonReset();
	afx_msg void OnButtonViewstill();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STILLCAPDLG_H__3067E9D2_B94C_4ED1_99AB_53034129A0DD__INCLUDED_)
