//------------------------------------------------------------------------------
// File: MapperDlg.h
//
// Desc: DirectShow sample code - an MFC based C++ filter mapper application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_MAPPERDLG_H__B56EF95A_58D3_498A_B0FD_11BD1B1CADF3__INCLUDED_)
#define AFX_MAPPERDLG_H__B56EF95A_58D3_498A_B0FD_11BD1B1CADF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <dshow.h>

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

//
// Tooltip strings
//
#define TTSTR_EXACT_MATCH TEXT("If the value of the bExactMatch parameter is TRUE, this method ")\
        TEXT("looks for filters that exactly match the values you specify for media type, pin category, ")\
        TEXT("and pin medium. If the value is FALSE, filters that register a value of NULL for any ")\
        TEXT("of these items are considered a match. (In effect, a NULL value in the registry acts as a ")\
        TEXT("wildcard.)")

#define TTSTR_ALOIP       TEXT("Boolean value indicating whether the filter must have an input pin. ")\
        TEXT("If the value is TRUE, the filter must have at least one input pin.")

#define TTSTR_ALOOP       TEXT("Boolean value specifying whether the filter must have an output pin. ")\
        TEXT("If TRUE, the filter must have at least one output pin.")

#define TTSTR_IS_RENDERER TEXT("Boolean value that specifies whether the filter must render its ")\
        TEXT("input. If TRUE, the specified filter must render input.")

#define TTSTR_CLEAR       TEXT("Clear all selections and return them to default values.")

#define TTSTR_SEARCH      TEXT("Search the registry for filters that match the current selections, ")\
        TEXT("using the IFilterMapper2::EnumMatchingFilters() method.")

#define TTSTR_FILTERS     TEXT("This area displays the names and filenames of filters that match ")\
        TEXT("your query.")

#define TTSTR_MERIT       TEXT("Minimum merit value.  Each filter is registered with a merit value. ")\
        TEXT("When the filter graph manager builds a graph, it enumerates all the filters registered ")\
        TEXT("with the correct media type. Then it tries them in order of merit, from highest to lowest. ")\
        TEXT("(It uses additional criteria to choose between filters with equal merit.) It never tries ")\
        TEXT("filters with a merit value less than or equal to MERIT_DO_NOT_USE.")

#define TTSTR_PIN_CAT     TEXT("The pin property set enables you to retrieve the category to which ")\
        TEXT("a pin belongs, if any. The category is set by the filter when it creates the pin; the ")\
        TEXT("category indicates what type of data the pin is streaming. The most commonly used ")\
        TEXT("categories are for the capture and preview pins on a capture filter.  Leave as <Don't Care> ")\
        TEXT("if not needed.")

#define TTSTR_TYPE        TEXT("Select a media type, if desired.  If you specify <Don't care> for ")\
        TEXT("media type or pin category, any filter is considered a match for that parameter.  ")\
        TEXT("If a pin did not register any media types, this method will not consider it a match for ")\
        TEXT("the media type.")

#define TTSTR_SUBTYPE     TEXT("Select a corresponding media subtype, if desired.  If you specify ")\
        TEXT("<Don't care> for the subtype, then the subtype is ignored (set to GUID_NULL) so that all ")\
        TEXT("subtypes are considered a match within the related type.")

/////////////////////////////////////////////////////////////////////////////
// CMapperDlg dialog

class CMapperDlg : public CDialog
{
// Construction
public:
	CMapperDlg(CWnd* pParent = NULL);	// standard constructor

    void FillMajorTypes(CListBox& m_List);
    void FillSubType(CListBox& m_List, CListBox& m_ListMinor);
    void EnableSecondTypePair(CListBox& m_ListMajor, 
                              CListBox& m_ListMajor2, CListBox& m_ListMinor2);
    void SetDefaults();

    BOOL InitializeTooltips(void);

    IEnumMoniker *GetFilterEnumerator(void);

    int GetFilenameByCLSID(REFCLSID clsid, TCHAR *szFile);

    void AddFilter(const TCHAR *szFilterName, const GUID *pCatGuid, DWORD dwMerit);
    void AddMerit(TCHAR *szInfo, DWORD dwMerit);
    HRESULT GetMerit(IPropertyBag *pPropBag, DWORD *pdwMerit);


// Dialog Data
	//{{AFX_DATA(CMapperDlg)
	enum { IDD = IDD_MAPPER_DIALOG };
	CButton	m_bSearch;
	CButton	m_bClear;
	CStatic	m_StrNumFilters;
	CButton	m_bIsRenderer;
	CButton	m_bAtLeastOneOutputPin;
	CButton	m_bAtLeastOneInputPin;
	CButton	m_bExactMatch;
	CListBox	m_ListOutputMinor2;
	CListBox	m_ListOutputMinor;
	CListBox	m_ListOutputMajor2;
	CListBox	m_ListOutputMajor;
	CListBox	m_ListInputMinor2;
	CListBox	m_ListInputMinor;
	CListBox	m_ListInputMajor2;
	CListBox	m_ListInputMajor;
	CComboBox	m_ComboMerit;
	CComboBox	m_ComboOutputCat;
	CComboBox	m_ComboInputCat;
	CListBox	m_ListFilters;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapperDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

    IFilterMapper2 *m_pMapper;
    CToolTipCtrl   *m_pToolTip;

	// Generated message map functions
	//{{AFX_MSG(CMapperDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnButtonSearch();
	afx_msg void OnSelchangeListInputMajor();
	afx_msg void OnSelchangeListOutputMajor();
	afx_msg void OnSelchangeListInputMajor2();
	afx_msg void OnSelchangeListOutputMajor2();
	afx_msg void OnButtonClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPPERDLG_H__B56EF95A_58D3_498A_B0FD_11BD1B1CADF3__INCLUDED_)
