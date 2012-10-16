//------------------------------------------------------------------------------
// File: PlayDMODlg.cpp
//
// Desc: DirectShow sample code - Implementation for CPlayDMODlg
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "PlayDMO.h"
#include "PlayDMODlg.h"

#include <mtype.h>

#define DEFAULT_FILENAME    TEXT("c:\\")

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
// CPlayDMODlg dialog

CPlayDMODlg::CPlayDMODlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayDMODlg::IDD, pParent), m_pVW(0), m_pGB(0), m_pMC(0)
{
	//{{AFX_DATA_INIT(CPlayDMODlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlayDMODlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlayDMODlg)
	DDX_Control(pDX, IDC_BUTTON_PROPPAGE, m_btnProperties);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_strStatus);
	DDX_Control(pDX, IDC_STATIC_OUT_MINBUFFERSIZE, m_nOutBufferSize);
	DDX_Control(pDX, IDC_STATIC_OUT_ALIGNMENT, m_nOutAlignment);
	DDX_Control(pDX, IDC_STATIC_MAXLATENCY, m_nMaxLatency);
	DDX_Control(pDX, IDC_STATIC_IN_MINBUFFERSIZE, m_nInBufferSize);
	DDX_Control(pDX, IDC_STATIC_IN_MAXLOOKAHEAD, m_nInLookahead);
	DDX_Control(pDX, IDC_STATIC_IN_ALIGNMENT, m_nInAlignment);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_ListFilters);
	DDX_Control(pDX, IDC_LIST_FILTER_OUTPUTS, m_ListFilterOutputs);
	DDX_Control(pDX, IDC_LIST_FILTER_INPUTS, m_ListFilterInputs);
	DDX_Control(pDX, IDC_LIST_AUDIO_DMOS, m_ListAudioDMO);
	DDX_Control(pDX, IDC_EDIT_FILENAME, m_StrFilename);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_SCREEN, m_Screen);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPlayDMODlg, CDialog)
	//{{AFX_MSG_MAP(CPlayDMODlg)
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
    ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_FILE, OnButtonFile)
	ON_LBN_SELCHANGE(IDC_LIST_FILTERS, OnSelchangeListFilters)
	ON_BN_CLICKED(IDC_BUTTON_ADD_DMO, OnButtonAddDmo)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_PROPPAGE, OnButtonProppage)
	ON_LBN_DBLCLK(IDC_LIST_FILTERS, OnDblclkListFilters)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayDMODlg message handlers

void CPlayDMODlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPlayDMODlg::OnPaint() 
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
HCURSOR CPlayDMODlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



BOOL CPlayDMODlg::OnInitDialog()
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
    SetDefaults();

    // Initialize DirectShow and query for needed interfaces
    HRESULT hr = InitDirectShow();
    if(FAILED(hr))
    {
        RetailOutput(TEXT("Failed to initialize DirectShow!  hr=0x%x\r\n"), hr);
        return FALSE;
    }

    // IMPORTANT
    // Since we're embedding video in a child window of a dialog,
    // we must set the WS_CLIPCHILDREN style to prevent the bounding
    // rectangle from drawing over our video frames.
    //
    // Neglecting to set this style can lead to situations when the video
    // is erased and replaced with black (or the default color of the 
    // bounding rectangle).
    m_Screen.ModifyStyle(0, WS_CLIPCHILDREN);

    hr = FillLists();
    if(FAILED(hr))
    {
        RetailOutput(TEXT("Failed to fill DMO list!  hr=0x%x\r\n"), hr);
        return FALSE;
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPlayDMODlg::SetDefaults()
{
    // Zero DirectShow interface pointers (sanity check)
    m_pGB=0;
    m_pMS=0;
    m_pMC=0;
    m_pME=0;
    m_pVW=0;

    // Zero DMO list
    for (int i=0; i<MAX_DMOS; i++)
        m_pDMOList[i] = 0;

    // Set default values for controls on the dialog
    m_StrFilename.SetWindowText(DEFAULT_FILENAME);
    g_psCurrent = State_Stopped;
}


HRESULT CPlayDMODlg::FillLists() 
{
    HRESULT hr;

    // Enumerate and display the audio DMOs installed in the system
    hr = AddDMOsToList(&DMOCATEGORY_AUDIO_EFFECT, m_ListAudioDMO, TRUE);
    if (FAILED(hr))
        return hr;

    // Set internal variables
    m_nLoadedDMOs = 0;
    m_nDMOCount = m_ListAudioDMO.GetCount();
    
    return hr;
}

//
//  Displays a text string in a status line near the bottom of the dialog
//
void CPlayDMODlg::Say(LPTSTR szText)
{
    m_strStatus.SetWindowText(szText);
}

HRESULT CPlayDMODlg::InitDirectShow(void)
{
    HRESULT hr = S_OK;

    m_bAudioOnly = FALSE;

    // Get interfaces
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
                         IID_IGraphBuilder, (void **)&m_pGB));

    JIF(m_pGB->QueryInterface(IID_IMediaControl,  (void **)&m_pMC));
    JIF(m_pGB->QueryInterface(IID_IMediaSeeking,  (void **)&m_pMS));
    JIF(m_pGB->QueryInterface(IID_IVideoWindow,   (void **)&m_pVW));
    JIF(m_pGB->QueryInterface(IID_IMediaEventEx,  (void **)&m_pME));

    return S_OK;

CLEANUP:
    FreeDirectShow();
    return(hr);
}

HRESULT CPlayDMODlg::FreeDirectShow(void)
{
    HRESULT hr=S_OK;

    StopMedia();

    // Disable event callbacks
    if (m_pME)
        hr = m_pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

    // Hide video window and remove owner.  This is not necessary here,
    // since we are about to destroy the filter graph, but it is included
    // for demonstration purposes.  Remember to hide the video window and
    // clear its owner when destroying a window that plays video.
    if(m_pVW)
    {
        hr = m_pVW->put_Visible(OAFALSE);
        hr = m_pVW->put_Owner(NULL);
    }

#ifdef DEBUG
    RemoveGraphFromRot(m_dwRegister);
#endif

    RemoveDMOsFromGraph();

    SAFE_RELEASE(m_pMC);
    SAFE_RELEASE(m_pMS);
    SAFE_RELEASE(m_pVW);
    SAFE_RELEASE(m_pME);
    SAFE_RELEASE(m_pGB);

    ClearLists();
    return hr;
}

void CPlayDMODlg::ResetDirectShow(void)
{
    // Destroy the current filter graph its filters.
    FreeDirectShow();

    // Reinitialize graph builder and query for interfaces
    InitDirectShow();
}

void CPlayDMODlg::ClearAllocatedLists()
{
    // Clear the lists and delete the allocated CLSID bytes
    ClearFilterListWithCLSID(m_ListAudioDMO);
}

void CPlayDMODlg::ClearLists()
{
    // Clear the lists that don't contain extra data
    m_ListFilters.ResetContent();
    m_ListFilterInputs.ResetContent();
    m_ListFilterOutputs.ResetContent();
}

void CPlayDMODlg::OnClose() 
{
    // Release DirectShow interfaces
    StopMedia();
    FreeDirectShow();
    ClearAllocatedLists();

    // Release COM
    CoUninitialize();

	CDialog::OnClose();
}

void CPlayDMODlg::OnDestroy() 
{
    // Release DirectShow interfaces
    FreeDirectShow();
    ClearAllocatedLists();

	CDialog::OnDestroy();
}

void CPlayDMODlg::OnButtonPlay() 
{
    if (g_psCurrent == State_Paused)
    {
        RunMedia();
    }
    else
    {
        if (SUCCEEDED(OnSelectFile()))
        {
            // Add any selected DMOs to the graph
            OnButtonAddDmo();

            // Disconnect audio renderer and insert DMOs
            if (m_nLoadedDMOs != 0)
            {
                if (FAILED(ConnectDMOsToRenderer()))
                {
                    MessageBeep(0);
                    EnableButtons(FALSE);
                    return;
                }
            }

            EnableButtons(FALSE);
  	        RunMedia();
        }
        else
        {
            MessageBox(TEXT("Please enter a valid media file name."), 
                       TEXT("Error opening media file!"));
            return;
        }
    }

    Say(TEXT("Running"));
}

void CPlayDMODlg::OnButtonPause() 
{
    PauseMedia();
    Say(TEXT("Paused"));
}

void CPlayDMODlg::OnButtonStop() 
{
    HRESULT hr;

    // Stop playback immediately with IMediaControl::Stop().
    StopMedia();
    EnableButtons(TRUE);

    // Reset to beginning of media clip
    LONGLONG pos=0;
    hr = m_pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                           NULL, AM_SEEKING_NoPositioning);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to seek to beginning of media!  hr=0x%x\r\n"), hr);
    }
  
    // Display the first frame of the media clip, if it contains video.
    // StopWhenReady() pauses all filters internally (which allows the video
    // renderer to queue and display the first video frame), after which
    // it sets the filters to the stopped state.  This enables easy preview
    // of the video's poster frame.
    hr = m_pMC->StopWhenReady();
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed in StopWhenReady!  hr=0x%x\r\n"), hr);
    }

    Say(TEXT("Stopped"));
}

void CPlayDMODlg::OnButtonFile() 
{
    CFileDialog dlgFile(TRUE);
    CString title;
    CString strFilter, strDefault, strFilename;

    VERIFY(title.LoadString(AFX_IDS_OPENFILE));

    // Initialize the file extensions and descriptions
    strFilter += "Media Files (*.avi, *.mpg, *.wav, *.mid)";
    strFilter += (TCHAR)'\0';
    strFilter += _T("*.avi;*.mpg;*.wav;*.mid");
    strFilter += (TCHAR)'\0';
    dlgFile.m_ofn.nMaxCustFilter++;

    CString allFilter;
    VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));

    // Append the "*.*" all files filter
    strFilter += allFilter;
    strFilter += (TCHAR)'\0';     // next string
    strFilter += _T("*.*");
    strFilter += (TCHAR)'\0\0';   // last string
    dlgFile.m_ofn.nMaxCustFilter++;

    dlgFile.m_ofn.lpstrFilter = strFilter;
    dlgFile.m_ofn.lpstrTitle = title;
    dlgFile.m_ofn.lpstrFile = strFilename.GetBuffer(_MAX_PATH);

    // Display the file open dialog
    INT_PTR nResult = dlgFile.DoModal();

    // If a file was selected, update the main dialog
    if (nResult == IDOK)
    {
        m_StrFilename.SetWindowText(strFilename);

        // Render this file and show the first video frame, if present
        OnSelectFile();
    }

    strFilename.ReleaseBuffer();
}


void CPlayDMODlg::OnButtonAddDmo() 
{
    // Release any currently loaded DMOs
    RemoveDMOsFromGraph();

    // Add the currently selected DMOs to the graph
    AddDMOsToGraph();

    // Update the graph filter display to show rendered graph
    AddGraphFiltersToList(m_pGB, m_ListFilters);
}

void CPlayDMODlg::OnButtonClear() 
{
    // Release any currently loaded DMOs
    RemoveDMOsFromGraph();

    // Clear the list of active DMOs
    m_ListAudioDMO.SelItemRange(FALSE, 0, m_nDMOCount);

    // Update the graph filter display to show rendered graph
    AddGraphFiltersToList(m_pGB, m_ListFilters);
}

HRESULT CPlayDMODlg::RemoveDMOsFromGraph(void)
{
    HRESULT hr=S_OK;

    // Remove and release each loaded DMO
    for (int i=0; i < m_nLoadedDMOs; i++)
    {
        if (m_pDMOList[i])
        {
            // Remove filter from graph.
            hr = m_pGB->RemoveFilter(m_pDMOList[i]);

            // Since we're not holding a reference to the filter, 
            // there's no need to Release() it, but do clear the variable.
            m_pDMOList[i]=0;
        }
    }

    m_nLoadedDMOs = 0;
    return hr;
}

HRESULT CPlayDMODlg::AddDMOsToGraph(void)
{
    USES_CONVERSION;
    HRESULT hr=S_OK;
    IBaseFilter *pDMOFilter=0;
    IDMOWrapperFilter *pWrap;
    CLSID *pStoredCLSID = NULL;
    int rnItems[MAX_DMOS];
    WCHAR wszDMOName[64];
    TCHAR szItem[64];

    // Clear status
    m_nLoadedDMOs = 0;

    // Get a list of the DMO entries selected in the multiple-select listbox
    int nSelected = m_ListAudioDMO.GetSelItems(MAX_DMOS, rnItems);

    // Load each selected DMO and add it to the graph
    for (int i=0; i < nSelected; i++)
    {
        // Read this DMO's CLSID from the list box
        pStoredCLSID = (CLSID *) m_ListAudioDMO.GetItemDataPtr(rnItems[i]);

        // Create the DMO Wrapper filter.
        hr = CoCreateInstance(CLSID_DMOWrapperFilter, NULL, CLSCTX_INPROC, 
                              IID_IBaseFilter, (void **)&pDMOFilter);
        if (FAILED(hr))
            continue;

        // Get its DMOWrapperFilter interface so that it can be added to
        // the DirectShow filter graph
        hr = pDMOFilter->QueryInterface(IID_IDMOWrapperFilter, (void **)&pWrap);

        if (SUCCEEDED(hr)) 
        {     
            // Initialize the wrapper filter with our DMO's CLSID and category
            hr = pWrap->Init(*pStoredCLSID, DMOCATEGORY_AUDIO_EFFECT); 
            if (FAILED(hr))
            {
                pWrap->Release();
                pDMOFilter->Release();
                continue;
            }

            pWrap->Release();

            // Read the DMO's name and convert to a wide string
            m_ListAudioDMO.GetText(rnItems[i], szItem);
            wcscpy(wszDMOName, T2W(szItem));

            // Add the DMO to the graph
            hr = m_pGB->AddFilter(pDMOFilter, wszDMOName);
            if (FAILED(hr))
                return hr;

            // If this filter was added to the graph, save its reference
            // and increment the count of loaded DMOs
            m_pDMOList[i] = pDMOFilter;
            m_nLoadedDMOs++;
        }

        pDMOFilter->Release();
    }

    return hr;
}


void CPlayDMODlg::OnSelchangeListFilters() 
{
    IBaseFilter *pFilter = NULL;
    TCHAR szNameToFind[128];

    // Use a helper function to display the filter's pins in listboxes
    AddFilterPinsToLists(m_pGB, m_ListFilters, m_ListFilterInputs, m_ListFilterOutputs);

    // Read the current filter name from the list box
    m_ListFilters.GetText(m_ListFilters.GetCurSel(), szNameToFind);

    // Read the current list box name and find it in the graph.
    // Display DMO-specific information if present in the graph.
    pFilter = FindFilterFromName(m_pGB, szNameToFind);
    if (pFilter)
    {
        HRESULT hr;
        IMediaObject *pDMO=NULL;

        // Find out if this filter supports a property page
        if (SupportsPropertyPage(pFilter))
            m_btnProperties.EnableWindow(TRUE);
        else
            m_btnProperties.EnableWindow(FALSE);

        // If this is a DMO, it will support IMediaObject, so show more info.
        hr = pFilter->QueryInterface(IID_IMediaObject, (void**) &pDMO);

        // Show information or default values
        ShowInputBufferInfo(pDMO, 0);
        ShowOutputBufferInfo(pDMO, 0);

        if (SUCCEEDED(hr))
            pDMO->Release();

        pFilter->Release();
    }
}


HRESULT CPlayDMODlg::OnSelectFile() 
{
    HRESULT hr;
    TCHAR szFilename[MAX_PATH];

    // Read file name from list box
    m_StrFilename.GetWindowText(szFilename, MAX_PATH);

    // First release any existing interfaces
    ResetDirectShow();

    // Clear filter/pin/event information listboxes
    m_ListFilters.ResetContent();
    m_ListFilterInputs.ResetContent();
    m_ListFilterOutputs.ResetContent();

    // Load the selected media file
    hr = PrepareMedia(szFilename);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return hr;
    }
    else
    {
        m_btnPlay.EnableWindow(TRUE);
    }

    // Enumerate and display filters in graph
    hr = AddGraphFiltersToList(m_pGB, m_ListFilters);

    // Select the first filter in the list to display pin info
    m_ListFilters.SetCurSel(0);
    OnSelchangeListFilters();

    // Cue the first video frame
    m_pMC->StopWhenReady();
    return hr;
}


HRESULT CPlayDMODlg::PrepareMedia(LPTSTR lpszMovie)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

    Say(TEXT("Loading..."));

    // Allow DirectShow to create the FilterGraph for this media file
    hr = m_pGB->RenderFile(T2W(lpszMovie), NULL);
    if (FAILED(hr)) {
        RetailOutput(TEXT("*** Failed(%08lx) in RenderFile(%s)!\r\n"),
                 hr, lpszMovie);
        return hr;
    }

    // Add our filter graph to the running object table for debugging
#ifdef DEBUG
    AddGraphToRot(m_pGB, &m_dwRegister);
#endif

    // Set the message drain of the video window to point to our main
    // application window.  If this is an audio-only or MIDI file, 
    // then put_MessageDrain will fail.
    hr = m_pVW->put_MessageDrain((OAHWND) m_hWnd);
    if (FAILED(hr))
    {
        m_bAudioOnly = TRUE;
    }

    // Have the graph signal events via window callbacks
    hr = m_pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);

    // Configure the video window
    if (!m_bAudioOnly)
    {
        hr = m_pVW->put_Owner((OAHWND) m_Screen.GetSafeHwnd());
        hr = m_pVW->put_WindowStyle(WS_CHILD);

        // We'll manually set the video to be visible
        hr = m_pVW->put_Visible(OAFALSE);

        // Place video window within the bounding rectangle
        CenterVideo();

        // Make the video window visible within the screen window.
        // If this is an audio-only file, then there won't be a video interface.
        hr = m_pVW->put_Visible(OATRUE);
        hr = m_pVW->SetWindowForeground(-1);
    }

    Say(TEXT("Ready"));
    return hr;
}

void CPlayDMODlg::CenterVideo(void)
{
    LONG width, height;
    HRESULT hr;

    if ((m_bAudioOnly) || (!m_pVW))
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

LRESULT CPlayDMODlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// Field notifications from the DirectShow filter graph manager
    // and those posted by the application
    switch (message)
    {
        case WM_GRAPHNOTIFY:
            HandleGraphEvent();
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


HRESULT CPlayDMODlg::HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    // Check that event interface is still valid
	if (!m_pME)
		return S_OK;
    
    // Spin through the events
    while(SUCCEEDED(hr = m_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, 
    									  (LONG_PTR *) &evParam2, 0)))
    {
        if(EC_COMPLETE == evCode)
        {
            // Since we're looping, reset to beginning and continue playing
            LONGLONG pos=0;
            hr = m_pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                     NULL, AM_SEEKING_NoPositioning);
        }

        // Free memory associated with this event
        hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);
    }

    return hr;
}


HRESULT CPlayDMODlg::RunMedia()
{
    HRESULT hr=S_OK;

    if (!m_pMC)
        return S_OK;

    // Start playback
    hr = m_pMC->Run();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Run()!\r\n"), hr);
        return hr;
    }

    // Remember play state
    g_psCurrent = State_Running;
    return hr;
}


HRESULT CPlayDMODlg::StopMedia()
{
    HRESULT hr=S_OK;

    if (!m_pMC)
        return S_OK;

    // If we're already stopped, don't check again
    if (g_psCurrent == State_Stopped)
        return hr;

    // Stop playback
    hr = m_pMC->Stop();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Stop()!\r\n"), hr);
        return hr;
    }

    // Wait for the stop to propagate to all filters
    OAFilterState fs;
    hr = m_pMC->GetState(500, &fs);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to read graph state!  hr=0x%x\r\n"), hr);
    }

    // Remember play state
    g_psCurrent = State_Stopped;
    return hr;
}


HRESULT CPlayDMODlg::PauseMedia(void)
{
    HRESULT hr=S_OK;

    if (!m_pMC)
        return S_OK;

    // Don't pause unless we're in the running state
    if(g_psCurrent != State_Running)
        return S_OK;

    // Pause playback
    hr = m_pMC->Pause();
    if (FAILED(hr)) {
        RetailOutput(TEXT("\r\n*** Failed(%08lx) in Pause()!\r\n"), hr);
        return hr;
    }

    // Remember play state
    g_psCurrent = State_Paused;
    return hr;
}


void CPlayDMODlg::OnButtonProppage() 
{
    IBaseFilter *pFilter = NULL;
    TCHAR szNameToFind[128];

    // Read the current filter name from the list box
    int nCurSel = m_ListFilters.GetCurSel();
    m_ListFilters.GetText(nCurSel, szNameToFind);

    // Find this filter in the graph
    pFilter = FindFilterFromName(m_pGB, szNameToFind);
    if (!pFilter)
        return;

    // Show the property page for the selected DMO or filter
    ShowFilterPropertyPage(pFilter, m_hWnd);
}

void CPlayDMODlg::OnDblclkListFilters() 
{
    // Show the property page for the selected DMO or filter
    OnButtonProppage();
}


void CPlayDMODlg::ShowInputBufferInfo(IMediaObject *pDMO, int nSel)
{
    HRESULT hr;
    TCHAR sz[32];
    DWORD dwSize=0, dwLookahead=0, dwAlign=0;

    // If the selected item is not a DMO, display default information
    if (!pDMO)
    {
        m_nInBufferSize.SetWindowText(STR_NOTDMO);
        m_nInLookahead.SetWindowText(STR_NOTDMO);
        m_nInAlignment.SetWindowText(STR_NOTDMO);
        m_nMaxLatency.SetWindowText(STR_NOTDMO);
        return;
    }

    // Read/display maximum latency
    REFERENCE_TIME reftime=0;
    hr = pDMO->GetInputMaxLatency(nSel, &reftime);
    if (SUCCEEDED(hr))
    {
        DWORD dwLatency = (DWORD) (reftime * 100000);
        wsprintf(sz, TEXT("%ld"), dwLatency);
        m_nMaxLatency.SetWindowText(sz);
    }
    else if (hr == E_NOTIMPL)
        m_nMaxLatency.SetWindowText(TEXT("0"));
    else
        m_nMaxLatency.SetWindowText(STR_UNKNOWN);

    // Read/display input buffer size information
    hr = pDMO->GetInputSizeInfo(nSel, &dwSize, &dwLookahead, &dwAlign);
    if (SUCCEEDED(hr))
    {
        wsprintf(sz, TEXT("%ld"), dwSize);
        m_nInBufferSize.SetWindowText(sz);

        wsprintf(sz, TEXT("%ld"), dwLookahead);
        m_nInLookahead.SetWindowText(sz);

        wsprintf(sz, TEXT("%ld"), dwAlign);
        m_nInAlignment.SetWindowText(sz);
    }
    else
    {
        m_nInBufferSize.SetWindowText(STR_UNKNOWN);
        m_nInLookahead.SetWindowText(STR_UNKNOWN);
        m_nInAlignment.SetWindowText(STR_UNKNOWN);
    }
}

void CPlayDMODlg::ShowOutputBufferInfo(IMediaObject *pDMO, int nSel)
{
    HRESULT hr;
    TCHAR sz[32];
    DWORD dwSize=0, dwAlign=0;

    // If the selected item is not a DMO, display default information
    if (!pDMO)
    {
        m_nOutBufferSize.SetWindowText(STR_NOTDMO);
        m_nOutAlignment.SetWindowText(STR_NOTDMO);
        return;
    }

    // Read/display output buffer size information
    hr = pDMO->GetOutputSizeInfo(nSel, &dwSize, &dwAlign);

    if (SUCCEEDED(hr))
    {
        wsprintf(sz, TEXT("%ld"), dwSize);
        m_nOutBufferSize.SetWindowText(sz);

        wsprintf(sz, TEXT("%ld"), dwAlign);
        m_nOutAlignment.SetWindowText(sz);
    }
    else
    {
        m_nOutBufferSize.SetWindowText(STR_UNKNOWN);
        m_nOutAlignment.SetWindowText(STR_UNKNOWN);
    }
}


void CPlayDMODlg::EnableButtons(BOOL bEnable)
{
    // Enable or disable buttons and other dialog items
    m_StrFilename.EnableWindow(bEnable);
    m_ListAudioDMO.EnableWindow(bEnable);

    GetDlgItem(IDC_BUTTON_ADD_DMO)->EnableWindow(bEnable);
    GetDlgItem(IDC_BUTTON_CLEAR)->EnableWindow(bEnable);
    GetDlgItem(IDC_BUTTON_FILE)->EnableWindow(bEnable);
}

HRESULT CPlayDMODlg::ConnectDMOsToRenderer() 
{
    USES_CONVERSION;
    HRESULT hr=0;
    IPin *pInputPin=0, *pUpstreamOutputPin=0;
    IBaseFilter *pRenderer=0;

    // Make sure that the graph is stopped before manipulating its filters
    hr = m_pMC->Stop();

    // Find the audio renderer in the graph
#if 0
    // It's not recommended to search for Microsoft-provided filters by text
    // name, since the name could change in the future.  Instead, use 
    // the more reliable method outlined below.
    WCHAR wszFilter[64];

    wcscpy(wszFilter, T2W(TEXT("Default DirectSound Device")));
    hr = m_pGB->FindFilterByName(wszFilter, &pRenderer);   
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to find audio renderer!\r\n"));
        goto err;
    }
#else
    //
    // Enumerate ALL filters in the graph, find one that has one input and 
    // no outputs, and check to see if it is configured to receive audio data.
    //
    hr = FindAudioRenderer(m_pGB, &pRenderer);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to find audio renderer!\r\n"));
        goto err;
    }
#endif

    // Verify that an audio renderer is present
    if (!pRenderer)
    {
        MessageBox(TEXT("This file contains no audio component.\r\n\r\n")
                   TEXT("Please select an audio file or a video file with audio."),
                   TEXT("No audio renderer!"));
        goto err;
    }

    // Get the pin interface for its input pin
    hr = GetPin(pRenderer, PINDIR_INPUT, 0, &pInputPin);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to find audio input pin!\r\n"));
        goto err;
    }

    // Find its upstream connection
    hr = pInputPin->ConnectedTo(&pUpstreamOutputPin);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to find audio renderer upstream output pin!\r\n"));
        goto err;
    }

    // Break the connection between these pins
    hr = pUpstreamOutputPin->Disconnect();
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to disconnect upstream output pin!\r\n"));
        goto err;
    }
    hr = pInputPin->Disconnect();
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to disconnect renderer input pin!\r\n"));
        goto err;
    }

    // Render the upstream filter's output pin again.  This will cause
    // the filter graph manager to connect our newly loaded DMOs into
    // the renderer's input pin.
    hr = m_pGB->Render(pUpstreamOutputPin);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to render upstream output pin!\r\n"));
        goto err;
    }

err:
    SAFE_RELEASE(pInputPin);
    SAFE_RELEASE(pUpstreamOutputPin);
    SAFE_RELEASE(pRenderer);

    return hr;
}


BOOL CPlayDMODlg::OnEraseBkgnd(CDC *pDC)
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
