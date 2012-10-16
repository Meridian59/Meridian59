//------------------------------------------------------------------------------
// File: MapperDlg.cpp
//
// Desc: DirectShow sample code - an MFC based C++ filter mapper application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
// MapperDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Mapper.h"
#include "MapperDlg.h"

#include "types.h"
#include "wxdebug.h"
#include "fil_data.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ID for unpublished filter data helper method
const IID IID_IAMFilterData = {0x97f7c4d4,0x547b,0x4a5f,{0x83,0x32,0x53,0x64,0x30,0xad,0x2e,0x4d}};


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapperDlg dialog

CMapperDlg::CMapperDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapperDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapperDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMapperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapperDlg)
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_bSearch);
	DDX_Control(pDX, IDC_BUTTON_CLEAR, m_bClear);
	DDX_Control(pDX, IDC_STATIC_NUMFILTERS, m_StrNumFilters);
	DDX_Control(pDX, IDC_CHECK_RENDERER, m_bIsRenderer);
	DDX_Control(pDX, IDC_CHECK_OUTPUT_PIN, m_bAtLeastOneOutputPin);
	DDX_Control(pDX, IDC_CHECK_INPUT_PIN, m_bAtLeastOneInputPin);
	DDX_Control(pDX, IDC_CHECK_EXACT_MATCH, m_bExactMatch);
	DDX_Control(pDX, IDC_LIST_OUTPUT_MINOR2, m_ListOutputMinor2);
	DDX_Control(pDX, IDC_LIST_OUTPUT_MINOR, m_ListOutputMinor);
	DDX_Control(pDX, IDC_LIST_OUTPUT_MAJOR2, m_ListOutputMajor2);
	DDX_Control(pDX, IDC_LIST_OUTPUT_MAJOR, m_ListOutputMajor);
	DDX_Control(pDX, IDC_LIST_INPUT_MINOR2, m_ListInputMinor2);
	DDX_Control(pDX, IDC_LIST_INPUT_MINOR, m_ListInputMinor);
	DDX_Control(pDX, IDC_LIST_INPUT_MAJOR2, m_ListInputMajor2);
	DDX_Control(pDX, IDC_LIST_INPUT_MAJOR, m_ListInputMajor);
	DDX_Control(pDX, IDC_COMBO_MERIT, m_ComboMerit);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_CATEGORY, m_ComboOutputCat);
	DDX_Control(pDX, IDC_COMBO_INPUT_CATEGORY, m_ComboInputCat);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_ListFilters);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMapperDlg, CDialog)
	//{{AFX_MSG_MAP(CMapperDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
    ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonSearch)
	ON_LBN_SELCHANGE(IDC_LIST_INPUT_MAJOR, OnSelchangeListInputMajor)
	ON_LBN_SELCHANGE(IDC_LIST_OUTPUT_MAJOR, OnSelchangeListOutputMajor)
	ON_LBN_SELCHANGE(IDC_LIST_INPUT_MAJOR2, OnSelchangeListInputMajor2)
	ON_LBN_SELCHANGE(IDC_LIST_OUTPUT_MAJOR2, OnSelchangeListOutputMajor2)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnButtonClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapperDlg message handlers

void CMapperDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMapperDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMapperDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BOOL CMapperDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	

    //////////////////////////////////////////////////////////////////////////
    //
    // DirectShow initialization code

    CoInitialize(NULL);

    // Create the filter mapper that will be used for all queries    
    HRESULT hr = CoCreateInstance(CLSID_FilterMapper2, 
                          NULL, CLSCTX_INPROC, IID_IFilterMapper2, 
                          (void **) &m_pMapper);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return FALSE;

    // Initialize some application settings
    InitializeTooltips();
    SetDefaults();

    // Customize list box tab stops for good formatting
    int nTabStops[2]={140,230};
    m_ListFilters.SetTabStops(2, nTabStops);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CMapperDlg::InitializeTooltips()
{
    int rc;

    // Create the tooltip control
    m_pToolTip = new CToolTipCtrl;
    if(!m_pToolTip->Create(this))
    {
       TRACE(TEXT("Unable To create ToolTip\n"));
       return FALSE;
    }

    // Set some tooltip defaults
    m_pToolTip->SetMaxTipWidth(250);
    m_pToolTip->SetDelayTime(TTDT_AUTOPOP, 10000);  /* 10s  */
    m_pToolTip->SetDelayTime(TTDT_INITIAL, 2000);   /* 2s */

    // Add tooltip strings for the relevant controls
    rc = m_pToolTip->AddTool(&m_bExactMatch,          TTSTR_EXACT_MATCH);
    rc = m_pToolTip->AddTool(&m_bAtLeastOneInputPin,  TTSTR_ALOIP);
    rc = m_pToolTip->AddTool(&m_bAtLeastOneOutputPin, TTSTR_ALOOP);
    rc = m_pToolTip->AddTool(&m_bIsRenderer,          TTSTR_IS_RENDERER);
    rc = m_pToolTip->AddTool(&m_bClear,               TTSTR_CLEAR);
    rc = m_pToolTip->AddTool(&m_bSearch,              TTSTR_SEARCH);
    rc = m_pToolTip->AddTool(&m_ListFilters,          TTSTR_FILTERS);
    rc = m_pToolTip->AddTool(&m_ComboMerit,           TTSTR_MERIT);
    rc = m_pToolTip->AddTool(&m_ComboInputCat,        TTSTR_PIN_CAT);
    rc = m_pToolTip->AddTool(&m_ComboOutputCat,       TTSTR_PIN_CAT);
    rc = m_pToolTip->AddTool(&m_ListInputMajor,       TTSTR_TYPE);
    rc = m_pToolTip->AddTool(&m_ListInputMajor2,      TTSTR_TYPE);
    rc = m_pToolTip->AddTool(&m_ListOutputMajor,      TTSTR_TYPE);
    rc = m_pToolTip->AddTool(&m_ListOutputMajor2,     TTSTR_TYPE);
    rc = m_pToolTip->AddTool(&m_ListInputMinor,       TTSTR_SUBTYPE);
    rc = m_pToolTip->AddTool(&m_ListInputMinor2,      TTSTR_SUBTYPE);
    rc = m_pToolTip->AddTool(&m_ListOutputMinor,      TTSTR_SUBTYPE);
    rc = m_pToolTip->AddTool(&m_ListOutputMinor2,     TTSTR_SUBTYPE);

    // Activate the tooltip control
    m_pToolTip->Activate(TRUE);
    return TRUE;
}

BOOL CMapperDlg::PreTranslateMessage(MSG* pMsg) 
{
    // Let the tooltip process the message, if appropriate
    if (m_pToolTip)
        m_pToolTip->RelayEvent(pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CMapperDlg::SetDefaults()
{
    int i;

    // The app allows you to specify up to two major/minor type pairs
    FillMajorTypes(m_ListInputMajor);
    FillMajorTypes(m_ListInputMajor2);
    FillMajorTypes(m_ListOutputMajor);
    FillMajorTypes(m_ListOutputMajor2);

    // Initialize subtype list boxes
    m_ListInputMinor.ResetContent();
    m_ListInputMinor2.ResetContent();
    m_ListOutputMinor.ResetContent();
    m_ListOutputMinor2.ResetContent();

    // Set checkboxes to preferred default values
    m_bAtLeastOneInputPin.SetCheck(1);
    m_bAtLeastOneOutputPin.SetCheck(1);

    // Initialize merit box
    m_ComboMerit.ResetContent();
    for (i=0; i < NUM_MERIT_TYPES; i++)
    {
        m_ComboMerit.InsertString(i, merittypes[i].szName);
        m_ComboMerit.SetItemData(i, merittypes[i].dwMerit);
    }
    m_ComboMerit.SetCurSel(4);  // "Normal" merit
    
    // Initialize pin categories
    m_ComboInputCat.ResetContent();
    m_ComboOutputCat.ResetContent();
    m_ComboInputCat.AddString(TEXT("<Don't care>\0"));
    m_ComboOutputCat.AddString(TEXT("<Don't care>\0"));

    // Fill pin category lists
    for (i=1; i < NUM_PIN_TYPES; i++)
    {
        m_ComboInputCat.InsertString(i, pintypes[i].szName);
        m_ComboInputCat.SetItemDataPtr(i, (void *) pintypes[i].pGUID);
        m_ComboOutputCat.InsertString(i, pintypes[i].szName);
        m_ComboOutputCat.SetItemDataPtr(i, (void *) pintypes[i].pGUID);
    }
    m_ComboInputCat.SetCurSel(0);
    m_ComboOutputCat.SetCurSel(0);  
    
    // Clear filter list
    m_ListFilters.ResetContent();  
    m_StrNumFilters.SetWindowText(TEXT("Filters Matching Query : 0"));

    // Select the first item in each type list
    FillSubType(m_ListInputMajor, m_ListInputMinor);
    FillSubType(m_ListInputMajor2, m_ListInputMinor2);
    FillSubType(m_ListOutputMajor, m_ListOutputMinor);
    FillSubType(m_ListOutputMajor2, m_ListOutputMinor2);

    // Disable the second type/subtype listboxes until the user selects
    // something other than "don't care" for the first type/subtype pair.
    OnSelchangeListInputMajor();
    OnSelchangeListOutputMajor();
}

void CMapperDlg::OnButtonClear() 
{
    // Clear all values and reset dialog
    SetDefaults();	
}

void CMapperDlg::OnClose() 
{
    // Disable and destroy the tooltip control
    if (m_pToolTip)
    {
        m_pToolTip->Activate(FALSE);
        delete m_pToolTip;
        m_pToolTip = 0;
    }

    // Release FilterMapper interface
    SAFE_RELEASE(m_pMapper);

    // Release COM
    CoUninitialize();
    
    CDialog::OnClose();
}

void CMapperDlg::OnDestroy()
{
    // Disable and destroy the tooltip control
    if (m_pToolTip)
    {
        delete m_pToolTip;
        m_pToolTip = 0;
    }

    // Release FilterMapper interface
    SAFE_RELEASE(m_pMapper);

    CDialog::OnDestroy();
}

void CMapperDlg::FillMajorTypes(CListBox& m_List)
{
    m_List.ResetContent();

    // Fill the specified list box with major type name/GUID
    for (int i=0; i < NUM_MAJOR_TYPES; i++)
    {
        m_List.InsertString(i, majortypes[i].szName);
        m_List.SetItemDataPtr(i, (void *) majortypes[i].pGUID);
    }

    m_List.SetCurSel(0);
}

void CMapperDlg::FillSubType(CListBox& m_List, CListBox& m_ListMinor)
{
    const GUIDINFO *pSubtype;
    int nSelection = m_List.GetCurSel();
    int nMajorType;

    // First clear the subtype list
    m_ListMinor.ResetContent();

    // If the "don't care" item was selected, clear and exit
    if (nSelection == 0)
    {
        m_ListMinor.AddString(TEXT("<No subtypes>\0"));
        m_ListMinor.SetCurSel(0);
        return;
    }
    else
        nMajorType = nSelection - 1;

    // Determine how to fill the minor type list, based on the
    // currently selected major type.
    pSubtype = pSubTypes[nMajorType];

    // If there's no associated subtype, just add a default
    if (!pSubtype)
    {
        m_ListMinor.AddString(TEXT("<No subtypes>\0"));
        m_ListMinor.SetCurSel(0);
        return;
    }
    else
    {
        // Set a default item for "don't care"
        m_ListMinor.AddString(TEXT("<Don't care>\0"));

        int i=0;

        // Fill the subtype list box.  Enter N item data to the N+1 list slot.
        while (pSubtype[i].pGUID != NULL)
        {
            m_ListMinor.AddString(pSubtype[i].szName);
            m_ListMinor.SetItemDataPtr(i+1, (void *) pSubtype[i].pGUID);
            i++;
        }

        m_ListMinor.SetCurSel(0);
    }
}

void CMapperDlg::OnSelchangeListInputMajor() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListInputMajor, m_ListInputMinor);

    // Since the second type/subtype pair is irrelevant if the first
    // is a don't care, disable the second set if appropriate.
    EnableSecondTypePair(m_ListInputMajor, m_ListInputMajor2, m_ListInputMinor2);
}

void CMapperDlg::OnSelchangeListInputMajor2() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListInputMajor2, m_ListInputMinor2);
}

void CMapperDlg::OnSelchangeListOutputMajor() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListOutputMajor, m_ListOutputMinor);

    // Since the second type/subtype pair is irrelevant if the first
    // is a don't care, disable the second set if appropriate.
    EnableSecondTypePair(m_ListOutputMajor, m_ListOutputMajor2, m_ListOutputMinor2);
}

void CMapperDlg::OnSelchangeListOutputMajor2() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListOutputMajor2, m_ListOutputMinor2);
}

void CMapperDlg::EnableSecondTypePair(CListBox& m_ListMajor, 
                 CListBox& m_ListMajor2, CListBox& m_ListMinor2)
{
    // If there is no selection in the first major type listbox,
    // clear and disable the second major/minor type listboxes.
    if (m_ListMajor.GetCurSel() == 0)
    {
        m_ListMajor2.SetCurSel(0);
        FillSubType(m_ListMajor2, m_ListMinor2);

        m_ListMajor2.EnableWindow(FALSE);
        m_ListMinor2.EnableWindow(FALSE);
    }
    else
    {
        m_ListMajor2.EnableWindow(TRUE);
        m_ListMinor2.EnableWindow(TRUE);
    }
}

void FillTypeArray(CListBox& m_ListMajor, CListBox& m_ListMinor,
                   GUID *atypes, int nIndex, DWORD *pdwPairs)
{
    int nMajorSel = m_ListMajor.GetCurSel();
    int nMinorSel = m_ListMinor.GetCurSel();

    // If no selection ("don't care"), just exit without modifying the array
    if (nMajorSel <= 0)
        return;

    // Get GUID for major type
    const GUID *p1 = (const GUID *)m_ListMajor.GetItemDataPtr(nMajorSel);

    // Since the FilterMapper interface requires GUIDs (instead of GUID *),
    // copy the specified GUID data into its array slot.
    memcpy(&atypes[nIndex], p1, sizeof(GUID));

    // Increment number of type/subtype pairs
    (*pdwPairs)++;

    // If no selection ("don't care"), or uninitialized (returning -1),
    // just exit without modifying the array
    if (nMinorSel <= 0)
        return;
    
    // Get GUID for subtype
    const GUID *p2 = (const GUID *)m_ListMinor.GetItemDataPtr(nMinorSel);

    if (p2)
        memcpy(&atypes[nIndex+1], p2, sizeof(GUID));
}

IEnumMoniker *CMapperDlg::GetFilterEnumerator(void)
{
    // First set up the boolean values
    BOOL bExactMatch = m_bExactMatch.GetCheck();
    BOOL bAtLeastOneInputPin = m_bAtLeastOneInputPin.GetCheck();
    BOOL bAtLeastOneOutputPin = m_bAtLeastOneOutputPin.GetCheck();
    BOOL bIsRenderer = m_bIsRenderer.GetCheck();

    // Merit
    DWORD dwMerit = (DWORD) m_ComboMerit.GetItemData(m_ComboMerit.GetCurSel());

    // Pin categories
    const GUID *pInCat  = (const GUID *)
        m_ComboInputCat.GetItemDataPtr(m_ComboInputCat.GetCurSel());
    const GUID *pOutCat = (const GUID *)
        m_ComboOutputCat.GetItemDataPtr(m_ComboOutputCat.GetCurSel());

    // Major type/subtype pairs
    DWORD dwInputTypes=0, dwOutputTypes=0;
    GUID arrayInput[4], arrayOutput[4];

    // Initialize GUID type/subtype arrays
    for (int i=0; i<4; i++)
    {
        arrayInput[i]  = GUID_NULL;
        arrayOutput[i] = GUID_NULL;
    }

    // Fill each of the four GUID arrays (input/output, major/minor).
    // If the first call doesn't set anything (primary), don't bother to
    // read the secondary value, since it would be invalid anyway due to
    // its position within the array (2) but with a count of only 1.
    FillTypeArray(m_ListInputMajor, m_ListInputMinor, arrayInput, 0, &dwInputTypes);
    if (dwInputTypes)
        FillTypeArray(m_ListInputMajor2, m_ListInputMinor2, arrayInput, 2, &dwInputTypes);

    FillTypeArray(m_ListOutputMajor, m_ListOutputMinor, arrayOutput, 0, &dwOutputTypes);
    if (dwOutputTypes)
        FillTypeArray(m_ListOutputMajor2, m_ListOutputMinor2, arrayOutput, 2, &dwOutputTypes);

	// Enumerate all filters that match the specified criteria
    IEnumMoniker *pEnum;
    HRESULT hr=S_OK;

    hr = m_pMapper->EnumMatchingFilters(
            &pEnum,
            0,                  // Reserved
            bExactMatch,        // Use exact match?
            dwMerit,            // Minimum merit
            bAtLeastOneInputPin,
            dwInputTypes,       // Number of major type/subtype pairs for input
            arrayInput,         // Array of major type/subtype pairs for input
            NULL,               // Input medium
            pInCat,             // Input pin category
            bIsRenderer,        // Must be a renderer?
            bAtLeastOneOutputPin,
            dwOutputTypes,      // Number of major type/subtype pairs for output
            arrayOutput,        // Array of major type/subtype pairs for output
            NULL,               // Output medium
            pOutCat);           // Output pin category

    if (FAILED(hr))
        return 0;
    else
        return pEnum;
}

int CMapperDlg::GetFilenameByCLSID(REFCLSID clsid, TCHAR *szFile)
{
    HRESULT hr;
    LPOLESTR strCLSID;
    int rc=0;

    // Convert binary CLSID to a readable version
    hr = StringFromCLSID(clsid, &strCLSID);
    if(SUCCEEDED(hr))
    {
        TCHAR szKey[512];
        CString strQuery(strCLSID);

        // Create key name for reading filename from registry
        wsprintf(szKey, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                 strQuery);

        // Free memory associated with strCLSID (allocated in StringFromCLSID)
        CoTaskMemFree(strCLSID);

        HKEY hkeyFilter=0;
        DWORD dwSize=MAX_PATH;
        BYTE szFilename[MAX_PATH];

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, szFilename, &dwSize);
           
            if (rc == ERROR_SUCCESS)
                wsprintf(szFile, TEXT("%s"), szFilename);

            RegCloseKey(hkeyFilter);
        }
    }
    else
        return -1;

    return rc;
}

// Read merit and version information
HRESULT CMapperDlg::GetMerit(IPropertyBag *pPropBag, DWORD *pdwMerit)
{
    HRESULT hr;
    IAMFilterData *pData=0;

    *pdwMerit = 0;

    VARIANT varFilData={0};
    varFilData.vt = VT_UI1 | VT_ARRAY;
    varFilData.parray = 0;     // docs say to zero this

    BYTE *pbFilterData = 0;    // 0 if not read
    DWORD dwcbFilterDAta = 0;  // 0 if not read

    // Read compressed filter data from the property bag with a variant
    hr = pPropBag->Read(L"FilterData", &varFilData, 0);
    if(SUCCEEDED(hr))
    {
        ASSERT(varFilData.vt == (VT_UI1 | VT_ARRAY));
        dwcbFilterDAta = varFilData.parray->rgsabound[0].cElements;

        // Access the filter data
        HRESULT hrTmp = SafeArrayAccessData(varFilData.parray, (void **)&pbFilterData);
        ASSERT(hrTmp == S_OK);
        ASSERT(pbFilterData);
    }
    else
    {
        ASSERT(dwcbFilterDAta == 0 && pbFilterData == 0);
        return E_FAIL;
    }

    // Get the IAMFilterData interface for parsing the filter data
    hr = m_pMapper->QueryInterface(IID_IAMFilterData, (void **)&pData);

    if(SUCCEEDED(hr))
    {
        BYTE *pb=0;

        // Use a helper method to parse the binary filter data.  Pass in
        // the pointer to the filter data, its size, and a buffer to fill with
        // the resulting data.  The "pb" buffer is allocated with CoTaskMemAlloc,
        // so it must be correspondingly freed by the caller.
        hr = pData->ParseFilterData(pbFilterData, dwcbFilterDAta, &pb);
        if(SUCCEEDED(hr))
        {
            REGFILTER2 *pFil = ((REGFILTER2 **)pb)[0];

            if(pbFilterData)
            {
                HRESULT hrTmp = SafeArrayUnaccessData(varFilData.parray);
                ASSERT(hrTmp == S_OK);

                hrTmp = VariantClear(&varFilData);
                ASSERT(hrTmp == S_OK);
            }

            // Assign the merit value from the REGFILTER2 structure
            if (pFil)
                *pdwMerit = pFil->dwMerit;

            // Free the memory allocated by ParseFilterData
            if (pb)
                CoTaskMemFree(pb);
        }

        // Release the IAMFilterData interface
        pData->Release();
    }

    return hr;
}

void CMapperDlg::AddMerit(TCHAR *szInfo, DWORD dwMerit)
{
    TCHAR szMerit[32];

    switch (dwMerit)
    {
        case MERIT_NORMAL:
             wsprintf(szMerit, TEXT("MERIT_NORMAL\0"));
             break;     // Assume normal
        case MERIT_PREFERRED:
             wsprintf(szMerit, TEXT("MERIT_PREFERRED\0"));
             break;
        case MERIT_UNLIKELY:
             wsprintf(szMerit, TEXT("MERIT_UNLIKELY\0"));
             break;
        case MERIT_DO_NOT_USE:
             wsprintf(szMerit, TEXT("MERIT_DO_NOT_USE\0"));
             break;
        case MERIT_SW_COMPRESSOR:
             wsprintf(szMerit, TEXT("MERIT_SW_COMPRESSOR\0"));
             break;
        case MERIT_HW_COMPRESSOR:
             wsprintf(szMerit, TEXT("MERIT_HW_COMPRESSOR\0"));
             break;
        
        default:
             wsprintf(szMerit, TEXT("0x%08x\0"), dwMerit);
             break;
    }

    // Add new merit information to string
    _tcscat(szInfo, szMerit);
}


void CMapperDlg::AddFilter(const TCHAR *szFilterName, const GUID *pCatGuid, DWORD dwMerit)
{
    TCHAR szInfo[300], szFilename[255];

    // First add the friendly name of the filter
    wsprintf(szInfo, TEXT("%s\t\0"), szFilterName);

    // Add the filter's merit value
    AddMerit(szInfo, dwMerit);

    // Get the filter's server name from the registry
    int rc = GetFilenameByCLSID(*pCatGuid, szFilename);

    // Add the filter's server name
    if (rc == ERROR_SUCCESS)
    {
        _tcscat(szInfo, TEXT("\t\0"));
        _tcscat(szInfo, szFilename);
    }

    // Add the filter name and filename to the listbox
    m_ListFilters.AddString(szInfo);
}

void CMapperDlg::OnButtonSearch() 
{
    HRESULT hr=S_OK;
    IEnumMoniker *pEnum;
    IMoniker *pMoniker=0;
    ULONG cFetched=0;
    VARIANT varName={0};
    int nFilters=0;    

    // Clear any previous results
    m_ListFilters.ResetContent();

    // Enumerate filters based on the current dialog box selections
    pEnum = GetFilterEnumerator();
    if (!pEnum)
    {
        m_ListFilters.AddString(TEXT("<No filters matched query>"));
        return;
    }
    hr = pEnum->Reset();

    // Enumerate all filters that match the search criteria.  The enumerator
    // that is returned from GetFilterEnumerator() will only list filters
    // that matched the user's search request.
    while((hr = pEnum->Next(1, &pMoniker, &cFetched)) == S_OK)
    {
        IPropertyBag *pPropBag;
        ASSERT(pMoniker);

        // Associate the moniker with a file
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                                    (void **)&pPropBag);
        ASSERT(SUCCEEDED(hr));
        ASSERT(pPropBag);
        if (FAILED(hr))
            continue;

        // Read filter name from property bag
        varName.vt = VT_BSTR;
        hr = pPropBag->Read(L"FriendlyName", &varName, 0);
        if (FAILED(hr))
            continue;

        // Get filter name (converting BSTR name to a CString)
        CString str(varName.bstrVal);
        SysFreeString(varName.bstrVal);
        nFilters++;
        
        // Read Merit value from property bag
        DWORD dwMerit;
        GetMerit(pPropBag, &dwMerit);

        // Read filter's CLSID from property bag.  This CLSID string will be
        // converted to a binary CLSID and passed to AddFilter(), which will
        // add the filter's name to the listbox and its CLSID to the listbox
        // item's DataPtr item.
        VARIANT varFilterClsid;
        varFilterClsid.vt = VT_BSTR;

        // Read CLSID string from property bag
        hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
        if(SUCCEEDED(hr))
        {
            CLSID clsidFilter;

            // Add filter name and filename to listbox
            if(CLSIDFromString(varFilterClsid.bstrVal, &clsidFilter) == S_OK)
            {
                AddFilter(str, &clsidFilter, dwMerit);
            }

            SysFreeString(varFilterClsid.bstrVal);
        }

        // Clean up interfaces
        SAFE_RELEASE(pPropBag);
        SAFE_RELEASE(pMoniker);
    }   

    // Display number of filters matching query
    TCHAR szNumFilters[48];
    wsprintf(szNumFilters, TEXT("Filters Matching Query : %d"), nFilters);
    m_StrNumFilters.SetWindowText(szNumFilters);

    // Clean up enumerator
    if (pEnum)
        pEnum->Release();
}



