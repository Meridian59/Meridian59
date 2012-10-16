//------------------------------------------------------------------------------
// File: SysEnumDlg.cpp
//
// Desc: DirectShow sample code - implementation of dialog for device
//       enumeration.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include <atlbase.h>
#include "SysEnum.h"
#include "SysEnumDlg.h"

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
// CSysEnumDlg dialog

CSysEnumDlg::CSysEnumDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSysEnumDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSysEnumDlg)
	m_bShowAllCategories = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSysEnumDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysEnumDlg)
	DDX_Control(pDX, IDC_STATIC_FILENAME, m_StrFilename);
	DDX_Control(pDX, IDC_STATIC_FILTERS, m_StrFilters);
	DDX_Control(pDX, IDC_STATIC_CLASSES, m_StrClasses);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_FilterList);
	DDX_Control(pDX, IDC_LIST_DEVICES, m_DeviceList);
	DDX_Check(pDX, IDC_CHECK_SHOWALL, m_bShowAllCategories);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSysEnumDlg, CDialog)
	//{{AFX_MSG_MAP(CSysEnumDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_LIST_DEVICES, OnSelchangeListDevices)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_SHOWALL, OnCheckShowall)
	ON_LBN_SELCHANGE(IDC_LIST_FILTERS, OnSelchangeListFilters)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSysEnumDlg message handlers

void CSysEnumDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSysEnumDlg::OnPaint() 
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
HCURSOR CSysEnumDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BOOL CSysEnumDlg::OnInitDialog()
{
    HRESULT hr;

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

    // Instantiate the system device enumerator
    m_pSysDevEnum = NULL;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
                          CLSCTX_INPROC, IID_ICreateDevEnum, 
                          (void **)&m_pSysDevEnum);
    if FAILED(hr)
    {
        CoUninitialize();
        return FALSE;
    }

    // By default, only enumerate subset of categories listed in docs
    m_bShowAllCategories = FALSE;
    FillCategoryList();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CSysEnumDlg::OnCheckShowall() 
{
    // Toggle category type and redraw the category list
    m_bShowAllCategories ^= 1;
    FillCategoryList();
    SetNumFilters(0);
}


void CSysEnumDlg::FillCategoryList(void)
{
    // Clear listboxes
    ClearDeviceList();
    ClearFilterList();

    if (m_bShowAllCategories)
    {
        // Emulate the behavior of GraphEdit by enumerating all 
        // categories in the system
        DisplayFullCategorySet();
    }
    else
    {
        // Fill the category list box with the categories to display,
        // using the names stored in the CATEGORY_INFO array.
        // SysEnumDlg.H for a category description.
        for (int i=0; i < NUM_CATEGORIES; i++)
        {
            m_DeviceList.AddString(categories[i].szName);
        }

        // Update listbox title with number of classes
        SetNumClasses(NUM_CATEGORIES);
    }
}


void CSysEnumDlg::SetNumClasses(int nClasses)
{
    TCHAR szClasses[64];

    wsprintf(szClasses, TEXT("%s (%d found)\0"), STR_CLASSES, nClasses);
    m_StrClasses.SetWindowText(szClasses);
}

void CSysEnumDlg::SetNumFilters(int nFilters)
{
    TCHAR szFilters[64];

    if (nFilters)
        wsprintf(szFilters, TEXT("%s (%d found)\0"), STR_FILTERS, nFilters);
    else
        wsprintf(szFilters, TEXT("%s\0"), STR_FILTERS);

    m_StrFilters.SetWindowText(szFilters);
}


void CSysEnumDlg::DisplayFullCategorySet(void)
{
    USES_CONVERSION;

    HRESULT hr;
    IEnumMoniker *pEmCat = 0;
    ICreateDevEnum *pCreateDevEnum = NULL;
    int nClasses=0;

    // Create an enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return;

    // Use the meta-category that contains a list of all categories.
    // This emulates the behavior of GraphEdit.
    hr = pCreateDevEnum->CreateClassEnumerator(
                         CLSID_ActiveMovieCategories, &pEmCat, 0);
    ASSERT(SUCCEEDED(hr));

    if(hr == S_OK)
    {
        IMoniker *pMCat;
        ULONG cFetched;

        // Enumerate over every category
        while(hr = pEmCat->Next(1, &pMCat, &cFetched),
              hr == S_OK)
        {
            IPropertyBag *pPropBag;

            // Associate moniker with a file
            hr = pMCat->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(SUCCEEDED(hr))
            {
                VARIANT varCatClsid;
                varCatClsid.vt = VT_BSTR;

                // Read CLSID string from property bag
                hr = pPropBag->Read(L"CLSID", &varCatClsid, 0);
                if(SUCCEEDED(hr))
                {
                    CLSID clsidCat;

                    if(CLSIDFromString(varCatClsid.bstrVal, &clsidCat) == S_OK)
                    {
                        // Use the guid if we can't get the name
                        WCHAR *wszCatName;
                        TCHAR szCatDesc[MAX_PATH];

                        VARIANT varCatName;
                        varCatName.vt = VT_BSTR;

                        // Read filter name
                        hr = pPropBag->Read(L"FriendlyName", &varCatName, 0);
                        if(SUCCEEDED(hr))
                            wszCatName = varCatName.bstrVal;
                        else
                            wszCatName = varCatClsid.bstrVal;

#ifndef UNICODE
                        WideCharToMultiByte(
                                CP_ACP, 0, wszCatName, -1,
                                szCatDesc, sizeof(szCatDesc), 0, 0);
#else
                        lstrcpy(szCatDesc, W2T(wszCatName));
#endif

                        if(SUCCEEDED(hr))
                            SysFreeString(varCatName.bstrVal);

                        // Add category name and CLSID to list box
                        AddFilterCategory(szCatDesc, &clsidCat);
                        nClasses++;
                    }

                    SysFreeString(varCatClsid.bstrVal);
                }

                pPropBag->Release();
            }
            else
            {
                break;
            }

            pMCat->Release();
        } // for loop

        pEmCat->Release();
    }

    pCreateDevEnum->Release();

    // Update listbox title with number of classes
    SetNumClasses(nClasses);
}


void CSysEnumDlg::AddFilterCategory(
    const TCHAR *szCatDesc,
    const GUID *pCatGuid)
{
    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;
    if (pclsid)
        *pclsid = *pCatGuid;

    // Add the category name and a pointer to its CLSID to the list box
    int nSuccess  = m_DeviceList.AddString(szCatDesc);
    int nIndexNew = m_DeviceList.FindStringExact(-1, szCatDesc);
    nSuccess = m_DeviceList.SetItemDataPtr(nIndexNew, pclsid);
}

void CSysEnumDlg::AddFilter(
    const TCHAR *szFilterName,
    const GUID *pCatGuid)
{
    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;
    if (pclsid)
        *pclsid = *pCatGuid;

    // Add the category name and a pointer to its CLSID to the list box
    int nSuccess  = m_FilterList.AddString(szFilterName);
    int nIndexNew = m_FilterList.FindStringExact(-1, szFilterName);
    nSuccess = m_FilterList.SetItemDataPtr(nIndexNew, pclsid);
}


void CSysEnumDlg::ClearDeviceList(void)
{
    CLSID *pStoredId = NULL;
    
    int nCount = m_DeviceList.GetCount();

    // Delete any CLSID pointers that were stored in the listbox item data
    for (int i=0; i < nCount; i++)
    {
        pStoredId = (CLSID *) m_DeviceList.GetItemDataPtr(i);
        if (pStoredId != 0)
        {
            delete pStoredId;
            pStoredId = NULL;
        }
    }

    // Clean up
    m_DeviceList.ResetContent();
    SetNumClasses(0);
}

void CSysEnumDlg::ClearFilterList(void)
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
    m_StrFilename.SetWindowText(TEXT("<No filter selected>"));
}


void CSysEnumDlg::OnSelchangeListDevices() 
{
    HRESULT hr;    
    IEnumMoniker *pEnumCat = NULL;

    // Get the currently selected category name
    int nItem = m_DeviceList.GetCurSel();
    const CLSID *clsid;
	
    if (m_bShowAllCategories)
    {
        // Read the CLSID pointer from the list box's item data
        clsid = (CLSID *) m_DeviceList.GetItemDataPtr(nItem);
    }
    else
    {
        // Read the CLSID pointer from our hard-coded array of
        // documented filter categories
        clsid = categories[nItem].pclsid;
    }

    // If the CLSID wasn't allocated earlier, then fail
    if (!clsid)
    {
        MessageBeep(0);
        return;
    }

    //
    // WARNING!
    //
    // Some third-party filters throw an exception (int 3) during enumeration
    // on Debug builds, often due to heap corruption in RtlFreeHeap().
    // This is not an issue on Release builds.
    //

    // Enumerate all filters of the selected category  
    hr = m_pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);
    ASSERT(SUCCEEDED(hr));
    if FAILED(hr)
        return;

    // Enumerate all filters using the category enumerator
    hr = EnumFilters(pEnumCat);

    SAFE_RELEASE(pEnumCat);
}


HRESULT CSysEnumDlg::EnumFilters(IEnumMoniker *pEnumCat)
{
    HRESULT hr=S_OK;
    IMoniker *pMoniker;
    ULONG cFetched;
    VARIANT varName={0};
    int nFilters=0;

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
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        IPropertyBag *pPropBag;
        ASSERT(pMoniker);

        // Associate moniker with a file
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

        // Read filter's CLSID from property bag.  This CLSID string will be
        // converted to a binary CLSID and passed to AddFilter(), which will
        // add the filter's name to the listbox and its CLSID to the listbox
        // item's DataPtr item.  When the user clicks on a filter name in
        // the listbox, we'll read the stored CLSID, convert it to a string,
        // and use it to find the filter's filename in the registry.
        VARIANT varFilterClsid;
        varFilterClsid.vt = VT_BSTR;

        // Read CLSID string from property bag
        hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
        if(SUCCEEDED(hr))
        {
            CLSID clsidFilter;

            // Add filter name and CLSID to listbox
            if(CLSIDFromString(varFilterClsid.bstrVal, &clsidFilter) == S_OK)
            {
                AddFilter(str, &clsidFilter);
            }

            SysFreeString(varFilterClsid.bstrVal);
        }
       
        // Cleanup interfaces
        SAFE_RELEASE(pPropBag);
        SAFE_RELEASE(pMoniker);
    }

    // Update count of enumerated filters
    SetNumFilters(nFilters);
    return hr;
}


void CSysEnumDlg::OnSelchangeListFilters() 
{
    const CLSID *clsid;

    // Get the currently selected category name
    int nItem = m_FilterList.GetCurSel();
	
    // Read the CLSID pointer from the list box's item data
    clsid = (CLSID *) m_FilterList.GetItemDataPtr(nItem);

    // Find the filter filename in the registry (by CLSID)
    if (clsid != 0)
        ShowFilenameByCLSID(*clsid);
}


void CSysEnumDlg::ShowFilenameByCLSID(REFCLSID clsid)
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
        DWORD dwSize=MAX_PATH;
        BYTE pbFilename[MAX_PATH];
        int rc=0;

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, pbFilename, &dwSize);

            if (rc == ERROR_SUCCESS)
            {
                TCHAR szFilename[MAX_PATH];
                wsprintf(szFilename, TEXT("%s\0"), pbFilename);
                m_StrFilename.SetWindowText(szFilename);
            }

            rc = RegCloseKey(hkeyFilter);
        }
    }
}


void CSysEnumDlg::OnClose() 
{
    // Free any stored CLSID pointers (in listbox item data ptr area)
    ClearFilterList();
    ClearDeviceList();

    // Release system device enumerator and close COM
    SAFE_RELEASE(m_pSysDevEnum);
    CoUninitialize();
	
	CDialog::OnClose();
}


