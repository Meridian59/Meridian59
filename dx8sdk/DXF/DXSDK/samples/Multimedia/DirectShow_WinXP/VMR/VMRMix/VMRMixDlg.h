//------------------------------------------------------------------------------
// File: VMRMixDlg.h
//
// Desc: DirectShow sample code
//       Headers and class description for the settings dialog
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_VMRMixDLG_H__023F795A_822F_482F_8EC9_00EC8D8AA54F__INCLUDED_)
#define AFX_VMRMixDLG_H__023F795A_822F_482F_8EC9_00EC8D8AA54F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVMRMixDlg dialog

class CVMRMixDlg : public CDialog
{
// Construction
public:
    HRESULT RunDemonstration();
    bool GetMediaSettings();
    bool IsFullScreen(){ return m_bFullScreen;};
    bool IsBitmapToUse(){ return m_bUseBitmap;};
    CVMRMixDlg(CWnd* pParent = NULL);   // standard constructor

    typedef enum eState
    {
        eStop,
        ePlay
    } eState;

    CMediaList m_MediaList;

// Dialog Data
    //{{AFX_DATA(CVMRMixDlg)
    enum { IDD = IDD_VMRMIX_DIALOG };
    CButton m_chkFullScreen;
    CButton m_chkBitmap;
    CSliderCtrl m_Slider;
    CButton m_btnOK;
    CButton m_btnStop;
    CButton m_btnPlay;
    CButton m_btnPause;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CVMRMixDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void PostNcDestroy();
    //}}AFX_VIRTUAL

    bool SetNumberOfStreams( int n);
    bool SwitchStateTo( eState eNewState);

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    //{{AFX_MSG(CVMRMixDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnButtonPlay();
    afx_msg void SelectFolder();
    afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCheckApplybitmap();
    afx_msg void OnCheckFullscreen();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
private:
    bool m_bFullScreen;
    bool m_bUseBitmap;
    int m_nStreams;
    eState m_eState;
    char m_szFolder[MAX_PATH];
    int m_nMaxSources;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VMRMixDLG_H__023F795A_822F_482F_8EC9_00EC8D8AA54F__INCLUDED_)
