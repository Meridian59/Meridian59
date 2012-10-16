//------------------------------------------------------------------------------
// File: StillViewDlg.cpp
//
// Desc: DirectShow sample code - implementation of CStillViewDlg class.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <atlbase.h>
#include "StillView.h"
#include "StillViewDlg.h"
#include "mediatypes.h"

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
// CStillViewDlg dialog

CStillViewDlg::CStillViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStillViewDlg::IDD, pParent), m_pGB(0), m_pMC(0), m_pBV(0), m_pVW(0), m_pME(0)
{
	//{{AFX_DATA_INIT(CStillViewDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStillViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStillViewDlg)
	DDX_Control(pDX, IDC_STATIC_FILETIME, m_StrFileTime);
	DDX_Control(pDX, IDC_STATIC_IMAGESIZE, m_StrImageSize);
	DDX_Control(pDX, IDC_EDIT_MEDIADIR, m_EditMediaDir);
	DDX_Control(pDX, IDC_SPIN_FILES, m_SpinFiles);
	DDX_Control(pDX, IDC_BUTTON_PROPPAGE, m_ButtonProperties);
	DDX_Control(pDX, IDC_STATUS_DIRECTORY, m_StrMediaPath);
	DDX_Control(pDX, IDC_STATIC_FILEDATE, m_StrFileDate);
	DDX_Control(pDX, IDC_STATIC_FILESIZE, m_StrFileSize);
	DDX_Control(pDX, IDC_LIST_PINS_OUTPUT, m_ListPinsOutput);
	DDX_Control(pDX, IDC_LIST_PINS_INPUT, m_ListPinsInput);
	DDX_Control(pDX, IDC_STATIC_FILELIST, m_StrFileList);
	DDX_Control(pDX, IDC_MOVIE_SCREEN, m_Screen);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_ListFilters);
	DDX_Control(pDX, IDC_LIST_FILES, m_ListFiles);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CStillViewDlg, CDialog)
	//{{AFX_MSG_MAP(CStillViewDlg)
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LIST_FILES, OnSelectFile)
	ON_LBN_SELCHANGE(IDC_LIST_FILTERS, OnSelchangeListFilters)
	ON_LBN_DBLCLK(IDC_LIST_FILTERS, OnDblclkListFilters)
	ON_BN_CLICKED(IDC_BUTTON_PROPPAGE, OnButtonProppage)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FILES, OnDeltaposSpinFiles)
	ON_BN_CLICKED(IDC_BUTTON_SET_MEDIADIR, OnButtonSetMediadir)
	ON_BN_CLICKED(IDC_BUTTON_GRAPHEDIT, OnButtonGraphedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CStillViewDlg message handlers

void CStillViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CStillViewDlg::OnPaint() 
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
HCURSOR CStillViewDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/////////////////////////////////////////////////////////////////////////////
// CStillViewDlg DirectShow code and message handlers


BOOL CStillViewDlg::OnInitDialog()
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
	
    // Initialize COM
    CoInitialize(NULL);
	
    // Initialize DirectShow and query for needed interfaces
    HRESULT hr = InitDirectShow();
    if(FAILED(hr))
    {
        RetailOutput(TEXT("Failed to initialize DirectShow!  hr=0x%x\r\n"), hr);
        return FALSE;
    }

    // Since we're embedding video in a child window of a dialog,
    // we must set the WS_CLIPCHILDREN style to prevent the bounding
    // rectangle from drawing over our video frames.
    m_Screen.ModifyStyle(0, WS_CLIPCHILDREN);

    // Propagate the files list and select the first item
    InitMediaDirectory();
   
    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CStillViewDlg::InitMediaDirectory(void)
{
    // Fill the media file list, starting with the directory passed
    // on the command line.  If no directory is passed, then read the
    // default media path for the DirectX SDK.
    TCHAR szDir[MAX_PATH];
    LONG lResult=0;

    if (theApp.m_lpCmdLine[0] == L'\0')
    {
        lResult = GetDXMediaPath(szDir);

        // If the DirectX SDK is not installed, use the Windows
        // directory instead.
        if (lResult != 0)
        {
            GetWindowsDirectory(szDir, MAX_PATH);
            _tcscat(szDir, _T("\\") );
        }
    }
    else
        _tcscpy(szDir, theApp.m_lpCmdLine);

    TCHAR szPathMsg[MAX_PATH];
    wsprintf(szPathMsg, TEXT("Media directory: %s\0"), szDir);
    m_StrMediaPath.SetWindowText(szPathMsg);

    // Save current directory name
    wsprintf(m_szCurrentDir, TEXT("%s\0"), szDir);

    m_EditMediaDir.SetLimitText(MAX_PATH);
    m_EditMediaDir.SetWindowText(szDir);

    FillFileList(szDir);
}


LONG CStillViewDlg::GetDXMediaPath(TCHAR *szPath)
{
    HKEY  hKey;
    DWORD dwType, dwSize = MAX_PATH;

    // Open the appropriate registry key
    LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                _T("Software\\Microsoft\\DirectX SDK"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != lResult )
        return -1;

    lResult = RegQueryValueEx( hKey, _T("DX81SDK Samples Path"), NULL,
                              &dwType, (BYTE*)szPath, &dwSize );
    RegCloseKey( hKey );

    if( ERROR_SUCCESS != lResult )
        return -1;

    _tcscat( szPath, _T("\\Media\\") );
    return 0;
}


void CStillViewDlg::OnButtonSetMediadir() 
{
    TCHAR szEditPath[MAX_PATH];
    DWORD dwAttr;

    // Read the string in the media directory edit box.
    m_EditMediaDir.GetWindowText(szEditPath, MAX_PATH);

    // Is this a valid directory name?
    dwAttr = GetFileAttributes(szEditPath);
    if ((dwAttr == (DWORD) -1) || (! (dwAttr & FILE_ATTRIBUTE_DIRECTORY)))
    {
        MessageBox(TEXT("Please enter a valid directory name."), TEXT("Media error"));
        return;
    }

    // User has specified a valid media directory.  
    // Update the current path string.
    TCHAR szPathMsg[MAX_PATH];
    wsprintf(szPathMsg, TEXT("Media directory: %s\0"), szEditPath);
    m_StrMediaPath.SetWindowText(szPathMsg);

    // Save current directory name.  Append the trailing '\' to match
    // the string created by GetDXSDKMediaPath() (if not present)
    int nLength = _tcslen(szEditPath);
    if (szEditPath[nLength - 1] != TEXT('\\'))
        wsprintf(m_szCurrentDir, TEXT("%s\\\0"), szEditPath);

    // Propagate the files list and select the first item
    FillFileList(szEditPath);
}


void CStillViewDlg::FillFileList(LPTSTR pszRootDir)
{
    UINT attr = 0;

    // Clear current file list
    m_ListFiles.ResetContent();

    // Clear filter/pin/event information listboxes
    m_ListFilters.ResetContent();
    m_ListPinsInput.ResetContent();
    m_ListPinsOutput.ResetContent();

    ::SetCurrentDirectory(pszRootDir);

    // Add all of our known supported media types to the file list.
    // Add files of each type in order.
    for (int i=0; i < NUM_MEDIA_TYPES; i++)
    {
        m_ListFiles.Dir(attr, TypeInfo[i].pszType);
    }

    // Update list box title with number of items added
    int nItems  = m_ListFiles.GetCount();
    TCHAR szTitle[64];
    wsprintf(szTitle, TEXT("Media files (%d found)"), nItems);
    m_StrFileList.SetWindowText(szTitle);

    // If there are no messages, inform the user
    if (nItems == 0)
        MessageBox(TEXT("This is a valid directory, but it does not contain any ")
                   TEXT("media files expected by this application (BMP, GIF, JPG, TGA)."),
                   TEXT("No still image files!"));
    
    // Automatically select the first file in the list once
    // the dialog is displayed.
    PostMessage(WM_FIRSTFILE, 0, 0L);
    m_nCurrentFileSelection = -1;     // No selection yet
}


void CStillViewDlg::OnClose() 
{
    // Release DirectShow interfaces
    FreeDirectShow();

    // Release COM
    CoUninitialize();

	CDialog::OnClose();
}

void CStillViewDlg::OnDestroy() 
{
    FreeDirectShow();	

	CDialog::OnDestroy();	
}

void CStillViewDlg::OnSelectFile() 
{
    HRESULT hr;
    TCHAR szFilename[MAX_PATH];

    // If this is the currently selected file, do nothing
    int nItem = m_ListFiles.GetCurSel();
    if (nItem == m_nCurrentFileSelection)
        return;

    // Remember the current selection to speed double-click processing
    m_nCurrentFileSelection = nItem;

    // Read file name from list box
    m_ListFiles.GetText(nItem, szFilename);

    // First release any existing interfaces
    ResetDirectShow();

    // Clear filter/pin/event information listboxes
    m_ListFilters.ResetContent();
    m_ListPinsInput.ResetContent();
    m_ListPinsOutput.ResetContent();

    // Load the selected media file
    hr = PrepareMedia(szFilename);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Display useful information about this file
    DisplayFileInfo(szFilename);
    DisplayImageInfo();

    // Enumerate and display filters in graph
    hr = EnumFilters();

    // Select the first filter in the list to display pin info
    m_ListFilters.SetCurSel(0);
    OnSelchangeListFilters();
}


HRESULT CStillViewDlg::PrepareMedia(LPTSTR lpszMovie)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

    // Allow DirectShow to create the FilterGraph for this media file
    hr = m_pGB->RenderFile(T2W(lpszMovie), NULL);
    if (FAILED(hr)) {
        RetailOutput(TEXT("*** Failed(%08lx) in RenderFile(%s)!\r\n"),
                 hr, lpszMovie);
        return hr;
    }

    // Set the message drain of the video window to point to our main
    // application window.
    hr = m_pVW->put_MessageDrain((OAHWND) m_hWnd);

    // Have the graph signal event via window callbacks
    hr = m_pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);

    // Place video window within the bounding rectangle
    CenterVideo();

    // Finish configuring video window
    hr = m_pVW->put_WindowStyle(WS_CHILD);
    hr = m_pVW->put_Owner((OAHWND) m_Screen.GetSafeHwnd());
    hr = m_pVW->SetWindowForeground(-1);

    // Display the media file's first frame, which is the only frame
    // for a still image.
    hr = m_pMC->Pause();

    return hr;
}

HRESULT CStillViewDlg::InitDirectShow(void)
{
    HRESULT hr = S_OK;

    // Zero interfaces (sanity check)
    m_pVW = NULL;
    m_pBV = NULL;

    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&m_pGB));
    JIF(m_pGB->QueryInterface(IID_IMediaControl,  (void **)&m_pMC));
    JIF(m_pGB->QueryInterface(IID_IBasicVideo,    (void **)&m_pBV));
    JIF(m_pGB->QueryInterface(IID_IVideoWindow,   (void **)&m_pVW));
    JIF(m_pGB->QueryInterface(IID_IMediaEventEx,  (void **)&m_pME));

    return S_OK;

CLEANUP:
    FreeDirectShow();
    return(hr);
}

HRESULT CStillViewDlg::FreeDirectShow(void)
{
    HRESULT hr=S_OK;

    // Hide video window and remove owner.  This is not necessary here,
    // since we are about to destroy the filter graph, but it is included
    // for demonstration purposes.  Remember to hide the video window and
    // clear its owner when destroying a window that plays video.
    if(m_pVW)
    {
        hr = m_pVW->put_Visible(OAFALSE);
        hr = m_pVW->put_Owner(NULL);
    }

    SAFE_RELEASE(m_pMC);
    SAFE_RELEASE(m_pVW);
    SAFE_RELEASE(m_pBV);
    SAFE_RELEASE(m_pME);
    SAFE_RELEASE(m_pGB);

    return hr;
}

void CStillViewDlg::ResetDirectShow(void)
{
    // Destroy the current filter graph its filters.
    FreeDirectShow();

    // Reinitialize graph builder and query for interfaces
    InitDirectShow();
}

void CStillViewDlg::CenterVideo(void)
{
    LONG width, height;
    HRESULT hr;

    if (!m_pVW)
        return;

    // Read coordinates of video container window
    RECT rc;
    m_Screen.GetClientRect(&rc);
    width =  rc.right - rc.left;
    height = rc.bottom - rc.top;

    // Ignore the video's original size and stretch to fit bounding rectangle
    hr = m_pVW->SetWindowPosition(rc.left, rc.top, width, height);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to set window position!  hr=0x%x\r\n"), hr);
        return;
    }
}


LRESULT CStillViewDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// Field notifications from the DirectShow filter graph manager
    // and those posted by the application
    switch (message)
    {
        case WM_GRAPHNOTIFY:
            HandleGraphEvent();
            break;

        case WM_PLAYFILE:
            PlaySelectedFile();
            break;

        case WM_NEXTFILE:
            PlayNextFile();
            break;

        case WM_PREVIOUSFILE:
            PlayPreviousFile();
            break;

        case WM_FIRSTFILE:
            // Select the first item in the list
            m_ListFiles.SetCurSel(0);
            OnSelectFile();
            break;
    }

    // Pass along this message to the video window, which exists as a child
    // of the m_Screen window.  This method should be used by windows that 
    // make a renderer window a child window. It forwards significant messages 
    // to the child window that the child window would not otherwise receive. 
    if (m_pVW)
    {
        m_pVW->NotifyOwnerMessage((LONG_PTR) m_hWnd, message, wParam, lParam);
    }

	return CDialog::WindowProc(message, wParam, lParam);
}


HRESULT CStillViewDlg::HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    while(SUCCEEDED(m_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, 
                                 (LONG_PTR *) &evParam2, 0)))
    {
        // Spin through the events
        hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);
    }

    return hr;
}

void CStillViewDlg::PlaySelectedFile() 
{
    OnSelectFile();
}

void CStillViewDlg::OnDeltaposSpinFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    if (pNMUpDown->iDelta > 0)
        PostMessage(WM_NEXTFILE, 0, 0L);
    else
        PostMessage(WM_PREVIOUSFILE, 0, 0L);

	*pResult = 0;
}

void CStillViewDlg::PlayNextFile(void)
{
    int nItems  = m_ListFiles.GetCount();
    
    // Return if the list is empty
    if (!nItems)
        return;
        
    int nCurSel = m_ListFiles.GetCurSel();
    int nNewSel = (nCurSel + 1) % nItems;

    // Select the next item in the list, wrapping to top if needed
    m_ListFiles.SetCurSel(nNewSel);
    OnSelectFile();
}


void CStillViewDlg::PlayPreviousFile(void)
{
    int nItems  = m_ListFiles.GetCount();

    // Return if the list is empty
    if (!nItems)
        return;
        
    int nCurSel = m_ListFiles.GetCurSel();
    int nNewSel = nCurSel - 1;

    // If moved off top of list, select last item in list
    if (nNewSel < 0)
        nNewSel = nItems - 1;

    // Select the next item in the list, wrapping to top if needed
    m_ListFiles.SetCurSel(nNewSel);
    OnSelectFile();
}


HRESULT CStillViewDlg::EnumFilters (void) 
{
    USES_CONVERSION;

    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;

    // Clear filters list box
    m_ListFilters.ResetContent();
    
    // Get filter enumerator
    hr = m_pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
    {
        m_ListFilters.AddString(TEXT("<ERROR>"));
        return hr;
    }

    // Enumerate all filters in the graph
    while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
        
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr))
        {
            m_ListFilters.AddString(TEXT("<ERROR>"));
        }
        else
        {
            // Add the filter name to the filters listbox
            lstrcpy(szName, W2T(FilterInfo.achName));
            m_ListFilters.AddString(szName);

            FilterInfo.pGraph->Release();
        }       
        pFilter->Release();
    }
    pEnum->Release();

    return hr;
}


//
// The GraphBuilder interface provides a FindFilterByName() method,
// which provides similar functionality to the method below.
// This local method is provided for educational purposes.
//
IBaseFilter *CStillViewDlg::FindFilterFromName(LPTSTR szNameToFind)
{
    USES_CONVERSION;

    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;
    BOOL bFound = FALSE;

    // Get filter enumerator
    hr = m_pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
        return NULL;

    // Enumerate all filters in the graph
    while((pEnum->Next(1, &pFilter, &cFetched) == S_OK) && (!bFound))
    {
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
        
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr))
        {
            pFilter->Release();
            pEnum->Release();
            return NULL;
        }

        // Compare this filter's name with the one we want
        lstrcpy(szName, W2T(FilterInfo.achName));
        if (! lstrcmp(szName, szNameToFind))
        {
            bFound = TRUE;
        }

        FilterInfo.pGraph->Release();

        // If we found the right filter, don't release its interface.
        // The caller will use it and release it later.
        if (!bFound)
            pFilter->Release();
        else
            break;
    }
    pEnum->Release();

    return (bFound ? pFilter : NULL);
}


HRESULT CStillViewDlg::EnumPins(IBaseFilter *pFilter, PIN_DIRECTION PinDir,
                              CListBox& Listbox)
{
    HRESULT hr;
    IEnumPins  *pEnum = NULL;
    IPin *pPin = NULL;

    // Clear the specified listbox (input or output)
    Listbox.ResetContent();

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        Listbox.AddString(TEXT("<ERROR>"));
        return hr;
    }

    // Enumerate all pins on this filter
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;

        hr = pPin->QueryDirection(&PinDirThis);
        if (FAILED(hr))
        {
            Listbox.AddString(TEXT("<ERROR>"));
            pPin->Release();
            continue;
        }

        // Does the pin's direction match the requested direction?
        if (PinDir == PinDirThis)
        {
            PIN_INFO pininfo={0};

            // Direction matches, so add pin name to listbox
            hr = pPin->QueryPinInfo(&pininfo);
            if (SUCCEEDED(hr))
            {
                CString str(pininfo.achName);
                Listbox.AddString(str);
            }

            // The pininfo structure contains a reference to an IBaseFilter,
            // so you must release its reference to prevent resource a leak.
            pininfo.pFilter->Release();
        }
        pPin->Release();
    }
    pEnum->Release();

    return hr;
}


void CStillViewDlg::OnSelchangeListFilters() 
{
    HRESULT hr;
    IBaseFilter *pFilter = NULL;
    TCHAR szNameToFind[128];

    // Read the current filter name from the list box
    int nCurSel = m_ListFilters.GetCurSel();
    m_ListFilters.GetText(nCurSel, szNameToFind);

    // Read the current list box name and find it in the graph
    pFilter = FindFilterFromName(szNameToFind);
    if (!pFilter)
        return;

    // Now that we have filter information, enumerate pins by direction
    // and add their names to the appropriate listboxes
    hr = EnumPins(pFilter, PINDIR_INPUT,  m_ListPinsInput);
    hr = EnumPins(pFilter, PINDIR_OUTPUT, m_ListPinsOutput);

    // Find out if this filter supports a property page
    if (SupportsPropertyPage(pFilter))
        m_ButtonProperties.EnableWindow(TRUE);
    else
        m_ButtonProperties.EnableWindow(FALSE);
    
    // Must release the filter interface returned from FindFilterByName()
    pFilter->Release();
}


BOOL CStillViewDlg::DisplayImageInfo(void)
{
    HRESULT hr;
    long lWidth, lHeight;
    
    if (!m_pBV)
        return FALSE;

    hr = m_pBV->GetVideoSize(&lWidth, &lHeight);
    if (SUCCEEDED(hr))
    {
        TCHAR szSize[64];
        wsprintf(szSize, TEXT("Image size: %d x %d\0"), lWidth, lHeight);
        m_StrImageSize.SetWindowText(szSize);
    }

    return TRUE;
}

BOOL CStillViewDlg::DisplayFileInfo(LPTSTR szFile)
{
    HANDLE hFile;
    LONGLONG llSize=0;
    DWORD dwSizeLow=0, dwSizeHigh=0;
    TCHAR szScrap[64];

    // Open the specified file to read size and date information
    hFile = CreateFile(szFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
                       (DWORD) 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        RetailOutput(TEXT("*** Failed(0x%x) to open file (to read size)!\r\n"),
                     GetLastError());
        return FALSE;
    }

    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    if ((dwSizeLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    {
        RetailOutput(TEXT("*** Error(0x%x) reading file size!\r\n"),
                     GetLastError());
        CloseHandle(hFile);
        return FALSE;
    }

    // Large files will also use the upper DWORD to report size.
    // Add them together for the true size if necessary.
    if (dwSizeHigh)
        llSize = (dwSizeHigh << 16) + dwSizeLow;
    else
        llSize = dwSizeLow;

    // Read date information
    BY_HANDLE_FILE_INFORMATION fi;
    if (GetFileInformationByHandle(hFile, &fi))
    {
        CTime time(fi.ftLastWriteTime);

        wsprintf(szScrap, TEXT("Date: %02d/%02d/%d\0"), 
                 time.GetMonth(), time.GetDay(), time.GetYear());
        m_StrFileDate.SetWindowText(szScrap);

        // Set time.  Note that 12am-12:59am will show as "00" instead of 12.
        wsprintf(szScrap, TEXT("Time: %02d:%02d %s\0"), 
                (time.GetHour() % 12), time.GetMinute(), 
                ((time.GetHour() > 12) ? TEXT("pm\0") : TEXT("am\0")));
        m_StrFileTime.SetWindowText(szScrap);
    }

    CloseHandle(hFile);

    // Update size/date windows
    wsprintf(szScrap, TEXT("Size: %d bytes\0"), dwSizeLow);
    m_StrFileSize.SetWindowText(szScrap);

    return TRUE;
}


BOOL CStillViewDlg::SupportsPropertyPage(IBaseFilter *pFilter) 
{
    HRESULT hr;
    TCHAR szNameToFind[128];
    ISpecifyPropertyPages *pSpecify;

    // Read the current filter name from the list box
    int nCurSel = m_ListFilters.GetCurSel();
    m_ListFilters.GetText(nCurSel, szNameToFind);

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        pSpecify->Release();
        return TRUE;
    }
    else
        return FALSE;
}


void CStillViewDlg::OnButtonProppage() 
{
    HRESULT hr;
    IBaseFilter *pFilter = NULL;
    TCHAR szNameToFind[128];
    ISpecifyPropertyPages *pSpecify;

    // Read the current filter name from the list box
    int nCurSel = m_ListFilters.GetCurSel();
    m_ListFilters.GetText(nCurSel, szNameToFind);

    // Read the current list box name and find it in the graph
    pFilter = FindFilterFromName(szNameToFind);
    if (!pFilter)
        return;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        do 
        {
            FILTER_INFO FilterInfo;
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            if (FAILED(hr))
                break;

            CAUUID caGUID;
            hr = pSpecify->GetPages(&caGUID);
            if (FAILED(hr))
                break;

            pSpecify->Release();
        
            // Display the filter's property page
            OleCreatePropertyFrame(
                m_hWnd,                 // Parent window
                0,                      // x (Reserved)
                0,                      // y (Reserved)
                FilterInfo.achName,     // Caption for the dialog box
                1,                      // Number of filters
                (IUnknown **)&pFilter,  // Pointer to the filter 
                caGUID.cElems,          // Number of property pages
                caGUID.pElems,          // Pointer to property page CLSIDs
                0,                      // Locale identifier
                0,                      // Reserved
                NULL                    // Reserved
            );
            CoTaskMemFree(caGUID.pElems);
            FilterInfo.pGraph->Release(); 

        } while(0);
    }

    pFilter->Release();
}


void CStillViewDlg::OnDblclkListFilters() 
{
    OnButtonProppage();
}


void RetailOutput(TCHAR *tszErr, ...)
{
    TCHAR tszErrOut[MAX_PATH + 256];

    va_list valist;

    va_start(valist,tszErr);
    wvsprintf(tszErrOut, tszErr, valist);
    OutputDebugString(tszErrOut);
    va_end (valist);
}


LONG CStillViewDlg::GetGraphEditPath(TCHAR *szPath)
{
    HKEY  hKey;
    DWORD dwType, dwSize = MAX_PATH;

    // Open the appropriate registry key
    LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                _T("Software\\Microsoft\\Shared Tools\\Graphedit"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != lResult )
        return -1;

    // Read the full path (including .exe name) for the GraphEdit tool
    lResult = RegQueryValueEx( hKey, _T("Path"), NULL,
                              &dwType, (BYTE*)szPath, &dwSize );
    RegCloseKey( hKey );

    if( ERROR_SUCCESS != lResult )
        return -1;

    return 0;
}


void CStillViewDlg::OnButtonGraphedit() 
{
    TCHAR szFilename[128], szFile[MAX_PATH];

    // Read file name from list box
    int nItem = m_ListFiles.GetCurSel();
    m_ListFiles.GetText(nItem, szFilename);

    // Build the full file name with path.  The path will already have
    // a '\' appended to the end.
    wsprintf(szFile, TEXT("%s%s\0"), m_szCurrentDir, szFilename);

    // Launch GraphEdit for the selected file
    //    
    // First look for a registry key containing its full path
    TCHAR szPath[MAX_PATH];

    LONG lResult = GetGraphEditPath(szPath);

    // If the DirectX SDK is not installed, just look for GraphEdit
    // anywhere in the system path.
    if (lResult != 0)
    {
        wsprintf(szPath, TEXT("%s\0"), TEXT("graphedt\0"));
    }

    // Lauch GraphEdit using either the full tool path or just its name                               
    HINSTANCE rc = ShellExecute(m_hWnd, TEXT("open\0"), szPath, 
                                szFile, NULL, SW_SHOWNORMAL);

    if (rc < (HINSTANCE) 32)
    {
        // Failed to start the app
        if ((rc == (HINSTANCE) ERROR_FILE_NOT_FOUND) || 
            (rc == (HINSTANCE) ERROR_PATH_NOT_FOUND))
        {
            MessageBox(TEXT("Couldn't find the GraphEdit application.\r\n\r\n")
                       TEXT("Please copy graphedt.exe to a directory on your path."), 
                       TEXT("Can't find GraphEdit"));   
        }
    }
}

BOOL CStillViewDlg::OnEraseBkgnd(CDC *pDC)
{
	// Intercept background erasing for the movie window, since the
    // video renderer will keep the screen painted.  Without this code,
    // your video window might get painted over with gray (the default
    // background brush) when it is obscured by another window and redrawn.
    CRect rc;

    // Get the bounding rectangle for the movie screen
    m_Screen.GetWindowRect(&rc);
    ScreenToClient(&rc);

    // Exclude the clipping region occupied by our movie screen
    pDC->ExcludeClipRect(&rc);
    
    // Erase the remainder of the dialog as usual
	return CDialog::OnEraseBkgnd(pDC);
}
