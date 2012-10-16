//------------------------------------------------------------------------------
// File: DMOEnumDlg.cpp
//
// Desc: DirectShow sample code - implementation of dialog for device
//       enumeration.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include "DMOEnum.h"
#include "DMOEnumDlg.h"

#include "mfcdmoutil.h"
#include "namedguid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// CDMOEnumDlg dialog

CDMOEnumDlg::CDMOEnumDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDMOEnumDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDMOEnumDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDMOEnumDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDMOEnumDlg)
	DDX_Control(pDX, IDC_STATIC_OUT_TYPE, m_StrOutType);
	DDX_Control(pDX, IDC_STATIC_OUT_SUBYTPE, m_StrOutSubtype);
	DDX_Control(pDX, IDC_STATIC_IN_TYPE, m_StrInType);
	DDX_Control(pDX, IDC_STATIC_IN_SUBTYPE, m_StrInSubtype);
	DDX_Control(pDX, IDC_CHECK_OUT_SUPPORTS_QC, m_CheckOutQC);
	DDX_Control(pDX, IDC_CHECK_IN_SUPPORTS_QC, m_CheckInQC);
	DDX_Control(pDX, IDC_STATIC_IN_FORMAT, m_StrInFormat);
	DDX_Control(pDX, IDC_STATIC_OUT_FORMAT, m_StrOutFormat);
	DDX_Control(pDX, IDC_CHECK_OUT_FIXED, m_CheckOutFixedSize);
	DDX_Control(pDX, IDC_CHECK_OUT_WHOLE_SAMPLES, m_CheckOutWholeSamples);
	DDX_Control(pDX, IDC_CHECK_OUT_OPTIONAL, m_CheckOutOptional);
	DDX_Control(pDX, IDC_CHECK_OUT_ONESAMPLE, m_CheckOutOneSample);
	DDX_Control(pDX, IDC_CHECK_OUT_DISCARDABLE, m_CheckOutDiscardable);
	DDX_Control(pDX, IDC_CHECK_IN_WHOLE_SAMPLES, m_CheckInWholeSamples);
	DDX_Control(pDX, IDC_CHECK_IN_ONESAMPLE, m_CheckInOneSample);
	DDX_Control(pDX, IDC_CHECK_IN_HOLDSBUFFERS, m_CheckInHoldsBuffers);
	DDX_Control(pDX, IDC_CHECK_IN_FIXED, m_CheckInFixedSize);
	DDX_Control(pDX, IDC_STATIC_OUTPUT_STREAMS, m_nOutputStreams);
	DDX_Control(pDX, IDC_STATIC_INPUT_STREAMS, m_nInputStreams);
	DDX_Control(pDX, IDC_LIST_OUTPUT_STREAMS, m_ListOutputStreams);
	DDX_Control(pDX, IDC_LIST_INPUT_STREAMS, m_ListInputStreams);
	DDX_Control(pDX, IDC_STATIC_NUM_OTYPES, m_nOutputTypes);
	DDX_Control(pDX, IDC_STATIC_NUM_ITYPES, m_nInputTypes);
	DDX_Control(pDX, IDC_LIST_OUTPUT_TYPES, m_ListOutputTypes);
	DDX_Control(pDX, IDC_LIST_INPUT_TYPES, m_ListInputTypes);
	DDX_Control(pDX, IDC_CHECK_KEYED, m_bCheckKeyed);
	DDX_Control(pDX, IDC_STATIC_FILENAME, m_StrFilename);
	DDX_Control(pDX, IDC_STATIC_FILTERS, m_StrFilters);
	DDX_Control(pDX, IDC_STATIC_CLASSES, m_StrClasses);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_FilterList);
	DDX_Control(pDX, IDC_LIST_DEVICES, m_DeviceList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDMOEnumDlg, CDialog)
	//{{AFX_MSG_MAP(CDMOEnumDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_LIST_DEVICES, OnSelchangeListDevices)
	ON_WM_CLOSE()
	ON_LBN_SELCHANGE(IDC_LIST_FILTERS, OnSelchangeListFilters)
	ON_BN_CLICKED(IDC_CHECK_KEYED, OnCheckKeyed)
	ON_LBN_SELCHANGE(IDC_LIST_INPUT_STREAMS, OnSelchangeListInputStreams)
	ON_LBN_SELCHANGE(IDC_LIST_OUTPUT_STREAMS, OnSelchangeListOutputStreams)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDMOEnumDlg message handlers

void CDMOEnumDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDMOEnumDlg::OnPaint() 
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
HCURSOR CDMOEnumDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BOOL CDMOEnumDlg::OnInitDialog()
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
	
    ////////////////////////////////////////////////////////////////////////
    //
    //  DirectShow-specific initialization code

    CoInitialize(NULL);
    m_pDMO=0;
    m_pGB=0;

    // Include keyed DMOs in our search by default
    m_bCheckKeyed.SetCheck(TRUE);

    // Enumerate and display the DMO category list
    FillCategoryList();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CDMOEnumDlg::FillCategoryList(void)
{
    // Clear listboxes
    ClearDeviceList();
    ClearFilterList();

    // Fill the category list box with the categories to display,
    // using the names stored in the DMO_CATEGORY_INFO array.
    // See DMOEnumDlg.H for a category description.
    for (int i=0; i < NUM_CATEGORIES; i++)
        m_DeviceList.AddString(dmo_categories[i].szName);

    // Update listbox title with number of classes
    SetNumClasses(NUM_CATEGORIES);

    // Select the first category to show useful information
    m_DeviceList.SetCurSel(0);
    OnSelchangeListDevices();
}

void CDMOEnumDlg::SetNumClasses(int nClasses)
{
    TCHAR szClasses[64];

    wsprintf(szClasses, TEXT("%s (%d found)\0"), STR_CLASSES, nClasses);
    m_StrClasses.SetWindowText(szClasses);
}

void CDMOEnumDlg::SetNumFilters(int nFilters)
{
    TCHAR szFilters[64];

    if (nFilters)
        wsprintf(szFilters, TEXT("%s (%d found)\0"), STR_FILTERS, nFilters);
    else
        wsprintf(szFilters, TEXT("%s\0"), STR_FILTERS);

    m_StrFilters.SetWindowText(szFilters);
}

void CDMOEnumDlg::OnCheckKeyed() 
{
    // Reenumerate the related filters, since the list may
    // have changed to add or removed keyed filters.
    OnSelchangeListDevices();
}

void CDMOEnumDlg::ClearDeviceList(void)
{
    // Clean up
    m_DeviceList.ResetContent();
    SetNumClasses(0);
}

void CDMOEnumDlg::ClearFilterList(void)
{
    CLSID *pStoredId = NULL;
    
    int nCount = m_FilterList.GetCount();

    // Delete any CLSID pointers that were stored in the listbox item data
    for (int i=0; i < nCount; i++)
    {
        pStoredId = (CLSID *) m_FilterList.GetItemDataPtr(i);
        if (pStoredId != 0)
        {
            delete pStoredId;
            pStoredId = NULL;
        }
    }

    // Clean up
    m_FilterList.ResetContent();
    SetNumFilters(0);
    m_StrFilename.SetWindowText(TEXT("<No DMO selected>"));
}

void CDMOEnumDlg::ClearTypeLists(void)
{
    // Clear the type boxes
    m_ListInputTypes.ResetContent();
    m_ListOutputTypes.ResetContent();

    m_nInputTypes.SetWindowText(TEXT("0\0"));
    m_nOutputTypes.SetWindowText(TEXT("0\0"));
}

void CDMOEnumDlg::OnClose() 
{
    // Free any stored CLSID pointers (in listbox item data ptr area)
    ReleaseDMO();
    ClearFilterList();
    ClearDeviceList();

    CoUninitialize();
	CDialog::OnClose();
}

void CDMOEnumDlg::ReleaseDMO(void)
{
    SAFE_RELEASE(m_pDMO);
    SAFE_RELEASE(m_pGB);

    // Clear any information set on the dialog for the last DMO
    ClearDMOInfo();
}

void CDMOEnumDlg::ClearDMOInfo()
{
    // Clear DMO-specific information
    m_ListInputStreams.ResetContent();
    m_ListOutputStreams.ResetContent();
    m_nInputStreams.SetWindowText(TEXT("In Streams: 0\0"));
    m_nOutputStreams.SetWindowText(TEXT("Out Streams: 0\0"));

    // Input settings
    m_CheckInWholeSamples.SetCheck(FALSE);
    m_CheckInOneSample.SetCheck(FALSE);
    m_CheckInFixedSize.SetCheck(FALSE);
    m_CheckInHoldsBuffers.SetCheck(FALSE);
    m_StrInFormat.SetWindowText(STR_UNKNOWN);
    m_StrInType.SetWindowText(STR_UNKNOWN);
    m_StrInSubtype.SetWindowText(STR_UNKNOWN);
    m_CheckInQC.SetCheck(FALSE);

    // Output settings
    m_CheckOutWholeSamples.SetCheck(FALSE);
    m_CheckOutOneSample.SetCheck(FALSE);
    m_CheckOutFixedSize.SetCheck(FALSE);
    m_CheckOutDiscardable.SetCheck(FALSE);
    m_CheckOutOptional.SetCheck(FALSE);
    m_StrOutFormat.SetWindowText(STR_UNKNOWN);
    m_StrOutType.SetWindowText(STR_UNKNOWN);
    m_StrOutSubtype.SetWindowText(STR_UNKNOWN);
    m_CheckOutQC.SetCheck(FALSE);
}


void CDMOEnumDlg::OnSelchangeListFilters() 
{
    const CLSID *clsid;

    // Get the currently selected category name
    int nItem = m_FilterList.GetCurSel();
	
    // Read the CLSID pointer from the list box's item data
    clsid = (CLSID *) m_FilterList.GetItemDataPtr(nItem);

    // Find the filter filename in the registry (by CLSID)
    if (clsid != 0)
    {
        // Display the DMO's server filename
        ShowFilenameByCLSID(*clsid);

        // Show media types/subtypes used by this DMO
        AddTypeInfo(clsid);

        // Display DMO-specific information        
        ReleaseDMO();
        ShowSelectedDMOInfo(clsid);
    }
}


void CDMOEnumDlg::OnSelchangeListDevices() 
{
    HRESULT hr;    
    IEnumDMO *pEnum = NULL;
    DWORD dwFlags = m_bCheckKeyed.GetCheck() ? DMO_ENUMF_INCLUDE_KEYED : 0;

    // Get the currently selected category name
    int nItem = m_DeviceList.GetCurSel();
    const GUID *clsid;
	
    // Read the CLSID pointer from our hard-coded array of
    // documented filter categories
    clsid = dmo_categories[nItem].pclsid;

    ClearTypeLists();
    ReleaseDMO();

    // Enumerate all DMOs of the selected category  
    hr = DMOEnum(*clsid, dwFlags, 0, NULL, 0, NULL, &pEnum);
    if (FAILED(hr))
        return;

    // Enumerate all filters using the new category enumerator
    hr = EnumDMOs(pEnum);

    // Now that the DMOs (if any) are enumerated and added 
    // to the list, go ahead and select the first one.
    m_FilterList.SetCurSel(0);
    OnSelchangeListFilters();

    SAFE_RELEASE(pEnum);
}


void CDMOEnumDlg::AddFilter(
    const TCHAR *szFilterName,
    const GUID *pCatGuid)
{
    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;

    // Add the category name and a pointer to its CLSID to the list box
    int nSuccess  = m_FilterList.AddString(szFilterName);
    int nIndexNew = m_FilterList.FindStringExact(-1, szFilterName);

    if (pclsid)
    {
        *pclsid = *pCatGuid;
        nSuccess = m_FilterList.SetItemDataPtr(nIndexNew, pclsid);
    }
}


HRESULT CDMOEnumDlg::EnumDMOs(IEnumDMO *pEnumCat)
{
    HRESULT hr=S_OK;
    ULONG cFetched;
    int nFilters=0;
    WCHAR *wszName;
    CLSID clsid;

    // Clear the current filter list
    ClearFilterList();

    // If there are no filters of a requested type, show default string
    if (!pEnumCat)
    {
        m_FilterList.AddString(TEXT("<< No entries >>"));
        SetNumFilters(nFilters);
        return S_FALSE;
    }

    // Enumerate all items associated with the moniker
    while(pEnumCat->Next(1, &clsid, &wszName, &cFetched) == S_OK)
    {
        nFilters++;
        CString str(wszName);

        // Add this DMO's name and CLSID to the listbox
        AddFilter(str, &clsid);
        CoTaskMemFree(wszName);
    }

    // If no DMOs matched the query, show a default item
    if (nFilters == 0)
        m_FilterList.AddString(TEXT("<< No entries >>"));

    // Update count of enumerated filters
    SetNumFilters(nFilters);
    return hr;
}


void CDMOEnumDlg::GetTypeSubtypeString(TCHAR *szCLSID, DMO_PARTIAL_MEDIATYPE& aList)
{
    HRESULT hr;
    CString strType, strSubtype;
    int j;

    LPOLESTR szType, szSubtype;

    // Convert binary CLSIDs to readable versions
    hr = StringFromCLSID(aList.type, &szType);
    if(FAILED(hr))
        return;
    hr = StringFromCLSID(aList.subtype, &szSubtype);
    if(FAILED(hr))
        return;

    // Set default type/subtype strings to their actual GUID values
    strType = szType;
    strSubtype = szSubtype;

    // Find type GUID's name in the named guids table
    j=0;
    while (rgng[j].pguid != 0)
    {
        if(aList.type == *(rgng[j].pguid))
        {
            strType = rgng[j].psz;  // Save type name
            break;
        }
        j++;
    }

    // Find subtype GUID's name in the named guids table
    j=0;
    while (rgng[j].pguid != 0)
    {
        if(aList.subtype == *(rgng[j].pguid))
        {
            strSubtype = rgng[j].psz; // Save subtype name
            break;
        }
        j++;
    }

    // Build a string with the type/subtype information.
    // If a friendly name was found, it will be used.
    // Otherwise, the type/subtype's raw GUID will be displayed.
    wsprintf(szCLSID, TEXT("%s\t%s"), strType, strSubtype);
}


void CDMOEnumDlg::AddTypeInfo(const GUID *pCLSID)
{
    const int NUM_PAIRS=20;
    HRESULT hr;
    DMO_PARTIAL_MEDIATYPE aInputTypes[NUM_PAIRS]={0}, 
                          aOutputTypes[NUM_PAIRS]={0};
    ULONG ulNumInputsSupplied, ulNumOutputsSupplied;
    ULONG ulNumInputTypes=NUM_PAIRS, ulNumOutputTypes=NUM_PAIRS, i;
    TCHAR szCLSID[128];

    // Clear the type/subtype GUID boxes
    ClearTypeLists();

    // Read type/subtype information
    hr = DMOGetTypes(
        *pCLSID,
        ulNumInputTypes,
        &ulNumInputsSupplied,
        aInputTypes,
        ulNumOutputTypes,
        &ulNumOutputsSupplied,
        aOutputTypes);

    if (FAILED(hr))
        return;

    // Display the number of input/output type/subtype pairs found
    TCHAR szNum[10];
    wsprintf(szNum, TEXT("%d"), ulNumInputsSupplied);
    m_nInputTypes.SetWindowText(szNum);
    wsprintf(szNum, TEXT("%d"), ulNumOutputsSupplied);
    m_nOutputTypes.SetWindowText(szNum);

    // Show input type/subtype pairs
    for (i=0; i<ulNumInputsSupplied; i++)
    {
        GetTypeSubtypeString(szCLSID, aInputTypes[i]);
        m_ListInputTypes.AddString(szCLSID);
    }

    // Show output type/subtype pairs
    for (i=0; i<ulNumOutputsSupplied; i++)
    {
        GetTypeSubtypeString(szCLSID, aOutputTypes[i]);
        m_ListOutputTypes.AddString(szCLSID);
    }
}


void CDMOEnumDlg::ShowFilenameByCLSID(REFCLSID clsid)
{
    HRESULT hr;
    LPOLESTR strCLSID;

    // Convert binary CLSID to a readable version
    hr = StringFromCLSID(clsid, &strCLSID);
    if(SUCCEEDED(hr))
    {
        TCHAR szKey[512];
        CString strQuery(strCLSID);

        // Create key name for reading filename registry
        wsprintf(szKey, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                 strQuery);

        // Free memory associated with strCLSID (allocated in StringFromCLSID)
        CoTaskMemFree(strCLSID);

        HKEY hkeyFilter=0;
        int rc=0;

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            DWORD dwSize=MAX_PATH;
            BYTE pbFilename[MAX_PATH];

            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, pbFilename, &dwSize);

            if (rc == ERROR_SUCCESS)
            {
                TCHAR szFilename[MAX_PATH];
                wsprintf(szFilename, TEXT("%s\0"), pbFilename);
                m_StrFilename.SetWindowText(szFilename);
            }
            else
                m_StrFilename.SetWindowText(TEXT("<Unknown>\0"));

            rc = RegCloseKey(hkeyFilter);
        }
    }
}


void CDMOEnumDlg::GetFormatString(TCHAR *szFormat, DMO_MEDIA_TYPE *pType)
{
    int i=0;

    // Find format GUID's name in the named guids table
    while (rgng[i].pguid != 0)
    {
        if(pType->formattype == *(rgng[i].pguid))
        {
            wsprintf(szFormat, TEXT("%s\0"), rgng[i].psz);
            return;
        }
        i++;
    }

    // If we got here, there was no match
    wsprintf(szFormat, TEXT("Format_None\0"));
}

void CDMOEnumDlg::GetGUIDString(TCHAR *szString, GUID *pGUID)
{
    int i=0;

    // Find GUID's name in the named guids table
    while (rgng[i].pguid != 0)
    {
        if(*pGUID == *(rgng[i].pguid))
        {
            wsprintf(szString, TEXT("%s\0"), rgng[i].psz);
            return;
        }
        i++;
    }

    // If we got here, there was no match
    wsprintf(szString, TEXT("GUID_NULL\0"));
}


void CDMOEnumDlg::ShowSelectedDMOInfo(const GUID *pCLSID) 
{
    HRESULT hr;
    TCHAR sz[64];

    // Create an instance of the selected DMO
    hr = CoCreateInstance(*pCLSID, NULL, 
                          CLSCTX_INPROC, IID_IMediaObject, 
                          (void **)&m_pDMO);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }
       
    // Read and display information about its input/output streams
    DWORD dwInStream=0, dwOutStream=0, dwCount;

    hr = m_pDMO->GetStreamCount(&dwInStream, &dwOutStream);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Display the number of streams used by this DMO
    wsprintf(sz, TEXT("In Streams: %d"), dwInStream);
    m_nInputStreams.SetWindowText(sz);
    wsprintf(sz, TEXT("Out Streams: %d"), dwOutStream);
    m_nOutputStreams.SetWindowText(sz);

    // Fill stream list boxes
    for (dwCount=0; dwCount < dwInStream; dwCount++)
    {
        wsprintf(sz, TEXT("In %d"), dwCount);
        m_ListInputStreams.AddString(sz);
    }
    for (dwCount=0; dwCount < dwOutStream; dwCount++)
    {
        wsprintf(sz, TEXT("Out %d"), dwCount);
        m_ListOutputStreams.AddString(sz);
    }

    // Select the first item in each list, if it exists
    if (dwInStream > 0)
    {
        m_ListInputStreams.SetCurSel(0);
        OnSelchangeListInputStreams();
    }
    if (dwOutStream > 0)
    {
        m_ListOutputStreams.SetCurSel(0);
        OnSelchangeListOutputStreams();
    }
}


void CDMOEnumDlg::OnSelchangeListInputStreams() 
{
    HRESULT hr;
    TCHAR sz[64];
    int nSel = m_ListInputStreams.GetCurSel();
    DWORD dwFlags=0;

    //
    // Display relevant information about input stream
    //
    if (!m_pDMO)
        return;

    // Read input stream information flags
    hr = m_pDMO->GetInputStreamInfo(nSel, &dwFlags);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Set stream info checkboxes
    m_CheckInWholeSamples.SetCheck(dwFlags & DMO_INPUT_STREAMF_WHOLE_SAMPLES);
    m_CheckInOneSample.SetCheck(dwFlags & DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER);
    m_CheckInFixedSize.SetCheck(dwFlags & DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE);
    m_CheckInHoldsBuffers.SetCheck(dwFlags & DMO_INPUT_STREAMF_HOLDS_BUFFERS);

    // Read preferred input type information.  The media type/subtypes
    // are arranged in order of preference, starting from zero.
    DMO_MEDIA_TYPE dmt={0};
    hr = m_pDMO->GetInputType(nSel, 0, &dmt);

    if (SUCCEEDED(hr))
    {
        GetFormatString(sz, &dmt);
        m_StrInFormat.SetWindowText(sz);
        GetGUIDString(sz, &dmt.majortype);
        m_StrInType.SetWindowText(sz);
        GetGUIDString(sz, &dmt.subtype);
        m_StrInSubtype.SetWindowText(sz);

        MoFreeMediaType(&dmt);
    }

    // Does this DMO support quality control?
    IDMOQualityControl *pQC=0;
    hr = m_pDMO->QueryInterface(IID_IDMOQualityControl, (void **) &pQC);
    if (SUCCEEDED(hr))
    {
        m_CheckInQC.SetCheck(TRUE);
        pQC->Release();
    }
}

void CDMOEnumDlg::OnSelchangeListOutputStreams() 
{
    HRESULT hr;
    TCHAR sz[64];
    int nSel = m_ListInputStreams.GetCurSel();
    DWORD dwFlags=0;

    //
    // Display relevant information about output stream
    //
    if (!m_pDMO)
        return;

    // Read output stream information flags
    hr = m_pDMO->GetOutputStreamInfo(nSel, &dwFlags);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Set stream info checkboxes
    m_CheckOutWholeSamples.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_WHOLE_SAMPLES);
    m_CheckOutOneSample.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER);
    m_CheckOutFixedSize.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE);
    m_CheckOutDiscardable.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_DISCARDABLE);
    m_CheckOutOptional.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_OPTIONAL);

    // Read preferred output type information
    DMO_MEDIA_TYPE dmt={0};
    hr = m_pDMO->GetOutputType(nSel, 0, &dmt);

    if (SUCCEEDED(hr))
    {
        GetFormatString(sz, &dmt);
        m_StrOutFormat.SetWindowText(sz);
        GetGUIDString(sz, &dmt.majortype);
        m_StrOutType.SetWindowText(sz);
        GetGUIDString(sz, &dmt.subtype);
        m_StrOutSubtype.SetWindowText(sz);

        MoFreeMediaType(&dmt);
    }

    // Does this DMO support quality control?
    IDMOQualityControl *pQC=0;
    hr = m_pDMO->QueryInterface(IID_IDMOQualityControl, (void **) &pQC);
    if (SUCCEEDED(hr))
    {
        m_CheckOutQC.SetCheck(TRUE);
        pQC->Release();
    }
}

