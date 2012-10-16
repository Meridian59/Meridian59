//------------------------------------------------------------------------------
// File: SysEnumDlg.h
//
// Desc: DirectShow sample code - header file for device enumeration dialog.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <dshow.h>


#if !defined(AFX_SYSENUMDLG_H__A96832BF_0309_4844_9B33_5E72285C3F3E__INCLUDED_)
#define AFX_SYSENUMDLG_H__A96832BF_0309_4844_9B33_5E72285C3F3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

//
//  Global data structure for storing CLSIDs and friendly strings
//
typedef struct _category_info
{
    const CLSID *pclsid;
    TCHAR szName[128];

} CATEGORY_INFO_DS;

// The DirectShow reference documentation lists a set of filter categories
// for which you can enumerate corresponding filters.  See 'Filter Categories'
// under 'DirectShow->Reference->Constants and GUIDS' in the DirectX docs.
const CATEGORY_INFO_DS categories[] = {
    
    &CLSID_AudioInputDeviceCategory, TEXT("Audio Capture Devices"),
    &CLSID_AudioCompressorCategory,  TEXT("Audio Compressors"),
    &CLSID_AudioRendererCategory,    TEXT("Audio Renderers"),
    &CLSID_LegacyAmFilterCategory,   TEXT("DirectShow Filters"),
    &CLSID_MidiRendererCategory,     TEXT("Midi Renderers"),
    &CLSID_VideoInputDeviceCategory, TEXT("Video Capture Devices"),
    &CLSID_VideoCompressorCategory,  TEXT("Video Compressors"),
};

#define NUM_CATEGORIES  (sizeof(categories) / sizeof(CATEGORY_INFO_DS))

#define STR_CLASSES     TEXT("System Device Classes")
#define STR_FILTERS     TEXT("Registered Filters")


/////////////////////////////////////////////////////////////////////////////
// CSysEnumDlg dialog

class CSysEnumDlg : public CDialog
{
// Construction
public:
	ICreateDevEnum * m_pSysDevEnum;
	CSysEnumDlg(CWnd* pParent = NULL);	// standard constructor
    HRESULT EnumFilters(IEnumMoniker *pEnumCat);
    void FillCategoryList(void);
    void DisplayFullCategorySet(void);
    void AddFilterCategory(const TCHAR *szCatDesc, const GUID *pCatGuid);
    void AddFilter(const TCHAR *szFilterName, const GUID *pCatGuid);
    void ClearDeviceList(void);
    void ClearFilterList(void);
    void SetNumClasses(int nClasses);
    void SetNumFilters(int nFilters);
    void ShowFilenameByCLSID(REFCLSID clsid);

// Dialog Data
	//{{AFX_DATA(CSysEnumDlg)
	enum { IDD = IDD_SYSENUM_DIALOG };
	CStatic	m_StrFilename;
	CStatic	m_StrFilters;
	CStatic	m_StrClasses;
	CListBox	m_FilterList;
	CListBox	m_DeviceList;
	BOOL	m_bShowAllCategories;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysEnumDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSysEnumDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeListDevices();
	afx_msg void OnClose();
	afx_msg void OnCheckShowall();
	afx_msg void OnSelchangeListFilters();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSENUMDLG_H__A96832BF_0309_4844_9B33_5E72285C3F3E__INCLUDED_)
