//------------------------------------------------------------------------------
// File: DMOEnumDlg.h
//
// Desc: DirectShow sample code - header file for device enumeration dialog.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <dshow.h>
#include <dmo.h>

#if !defined(AFX_DMOENUMDLG_H__A96832BF_0309_4844_9B33_5E72285C3F3E__INCLUDED_)
#define AFX_DMOENUMDLG_H__A96832BF_0309_4844_9B33_5E72285C3F3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }


/////////////////////////////////////////////////////////////////////////////
// CDMOEnumDlg dialog

class CDMOEnumDlg : public CDialog
{
// Construction
public:
	CDMOEnumDlg(CWnd* pParent = NULL);	// standard constructor
    HRESULT EnumDMOs(IEnumDMO *pEnumDMO);
    void FillCategoryList(void);
    void DisplayFullCategorySet(void);
    void AddFilterCategory(const TCHAR *szCatDesc, const GUID *pCatGuid);
    void AddFilter(const TCHAR *szFilterName, const GUID *pCatGuid);
    void AddTypeInfo(const GUID *pCLSID);
    void ShowSelectedDMOInfo(const GUID *pCLSID);
    void ClearDMOInfo(void);
    void ClearDeviceList(void);
    void ClearFilterList(void);
    void ClearTypeLists(void);
    void SetNumClasses(int nClasses);
    void SetNumFilters(int nFilters);
    void ShowFilenameByCLSID(REFCLSID clsid);
    void ReleaseDMO(void);
    void GetFormatString(TCHAR *szFormat, DMO_MEDIA_TYPE *pType);
    void GetGUIDString(TCHAR *szString, GUID *pGUID);
    void GetTypeSubtypeString(TCHAR *szCLSID, DMO_PARTIAL_MEDIATYPE& aList);

// Dialog Data
	//{{AFX_DATA(CDMOEnumDlg)
	enum { IDD = IDD_DMOENUM_DIALOG };
	CStatic	m_StrOutType;
	CStatic	m_StrOutSubtype;
	CStatic	m_StrInType;
	CStatic	m_StrInSubtype;
	CButton	m_CheckOutQC;
	CButton	m_CheckInQC;
	CStatic	m_StrInFormat;
	CStatic	m_StrOutFormat;
	CButton	m_CheckOutFixedSize;
	CButton	m_CheckOutWholeSamples;
	CButton	m_CheckOutOptional;
	CButton	m_CheckOutOneSample;
	CButton	m_CheckOutDiscardable;
	CButton	m_CheckInWholeSamples;
	CButton	m_CheckInOneSample;
	CButton	m_CheckInHoldsBuffers;
	CButton	m_CheckInFixedSize;
	CStatic	m_nOutputStreams;
	CStatic	m_nInputStreams;
	CListBox	m_ListOutputStreams;
	CListBox	m_ListInputStreams;
	CStatic	m_nOutputTypes;
	CStatic	m_nInputTypes;
	CListBox	m_ListOutputTypes;
	CListBox	m_ListInputTypes;
	CButton	m_bCheckKeyed;
	CStatic	m_StrFilename;
	CStatic	m_StrFilters;
	CStatic	m_StrClasses;
	CListBox	m_FilterList;
	CListBox	m_DeviceList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDMOEnumDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    IMediaObject  *m_pDMO;
    IGraphBuilder *m_pGB;

	// Generated message map functions
	//{{AFX_MSG(CDMOEnumDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeListDevices();
	afx_msg void OnClose();
	afx_msg void OnCheckShowall();
	afx_msg void OnSelchangeListFilters();
	afx_msg void OnCheckKeyed();
	afx_msg void OnSelchangeListInputStreams();
	afx_msg void OnSelchangeListOutputStreams();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DMOENUMDLG_H__A96832BF_0309_4844_9B33_5E72285C3F3E__INCLUDED_)
