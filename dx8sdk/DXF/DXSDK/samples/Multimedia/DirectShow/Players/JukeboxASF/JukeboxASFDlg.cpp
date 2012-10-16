//------------------------------------------------------------------------------
// File: JukeboxASFDlg.cpp
//
// Desc: DirectShow sample code - implementation of CJukeboxDlg class.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <atlbase.h>
#include "JukeboxASF.h"
#include "JukeboxASFDlg.h"
#include "playvideo.h"
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
// CJukeboxDlg dialog

CJukeboxDlg::CJukeboxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJukeboxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJukeboxDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJukeboxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJukeboxDlg)
	DDX_Control(pDX, IDC_STATIC_POSITION, m_StrPosition);
	DDX_Control(pDX, IDC_SLIDER, m_Seekbar);
	DDX_Control(pDX, IDC_STATIC_IMAGESIZE, m_StrImageSize);
	DDX_Control(pDX, IDC_STATIC_DURATION, m_StrDuration);
	DDX_Control(pDX, IDC_EDIT_MEDIADIR, m_EditMediaDir);
	DDX_Control(pDX, IDC_SPIN_FILES, m_SpinFiles);
	DDX_Control(pDX, IDC_BUTTON_FRAMESTEP, m_ButtonFrameStep);
	DDX_Control(pDX, IDC_LIST_EVENTS, m_ListEvents);
	DDX_Control(pDX, IDC_CHECK_EVENTS, m_CheckEvents);
	DDX_Control(pDX, IDC_BUTTON_PROPPAGE, m_ButtonProperties);
	DDX_Control(pDX, IDC_STATUS_DIRECTORY, m_StrMediaPath);
	DDX_Control(pDX, IDC_CHECK_MUTE, m_CheckMute);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_ButtonStop);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_ButtonPlay);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_ButtonPause);
	DDX_Control(pDX, IDC_CHECK_PLAYTHROUGH, m_CheckPlaythrough);
	DDX_Control(pDX, IDC_CHECK_LOOP, m_CheckLoop);
	DDX_Control(pDX, IDC_STATIC_FILEDATE, m_StrFileDate);
	DDX_Control(pDX, IDC_STATIC_FILESIZE, m_StrFileSize);
	DDX_Control(pDX, IDC_LIST_PINS_OUTPUT, m_ListPinsOutput);
	DDX_Control(pDX, IDC_LIST_PINS_INPUT, m_ListPinsInput);
	DDX_Control(pDX, IDC_STATIC_FILELIST, m_StrFileList);
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDC_MOVIE_SCREEN, m_Screen);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_ListFilters);
	DDX_Control(pDX, IDC_LIST_FILES, m_ListFiles);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJukeboxDlg, CDialog)
	//{{AFX_MSG_MAP(CJukeboxDlg)
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LIST_FILES, OnSelectFile)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnStop)
	ON_BN_CLICKED(IDC_CHECK_MUTE, OnCheckMute)
	ON_BN_CLICKED(IDC_CHECK_LOOP, OnCheckLoop)
	ON_BN_CLICKED(IDC_CHECK_PLAYTHROUGH, OnCheckPlaythrough)
	ON_LBN_SELCHANGE(IDC_LIST_FILTERS, OnSelchangeListFilters)
	ON_LBN_DBLCLK(IDC_LIST_FILTERS, OnDblclkListFilters)
	ON_BN_CLICKED(IDC_BUTTON_PROPPAGE, OnButtonProppage)
	ON_BN_CLICKED(IDC_CHECK_EVENTS, OnCheckEvents)
	ON_BN_CLICKED(IDC_BUTTON_FRAMESTEP, OnButtonFramestep)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_EVENTS, OnButtonClearEvents)
	ON_LBN_DBLCLK(IDC_LIST_FILES, OnDblclkListFiles)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FILES, OnDeltaposSpinFiles)
	ON_BN_CLICKED(IDC_BUTTON_SET_MEDIADIR, OnButtonSetMediadir)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_GRAPHEDIT, OnButtonGraphedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CJukeboxDlg message handlers

void CJukeboxDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CJukeboxDlg::OnPaint() 
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
HCURSOR CJukeboxDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/////////////////////////////////////////////////////////////////////////////
// CJukeboxDlg DirectShow code and message handlers


BOOL CJukeboxDlg::OnInitDialog()
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

    // IMPORTANT
    // Since we're embedding video in a child window of a dialog,
    // we must set the WS_CLIPCHILDREN style to prevent the bounding
    // rectangle from drawing over our video frames.
    //
    // Neglecting to set this style can lead to situations when the video
    // is erased and replaced with black (or the default color of the 
    // bounding rectangle).
    m_Screen.ModifyStyle(0, WS_CLIPCHILDREN);

    // Propagate the files list and select the first item
    InitMediaDirectory();
   
    // Initialize seeking trackbar range
    m_Seekbar.SetRange(0, 100, TRUE);
    m_Seekbar.SetTicFreq(10);
    
    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CJukeboxDlg::InitMediaDirectory(void)
{
    // Fill the media file list, starting with the directory passed
    // on the command line.  If no directory is passed, then read the
    // default media path for the DirectX SDK.
    TCHAR szDir[MAX_PATH];
    LONG lResult=0;

    if (theApp.m_lpCmdLine[0] == L'\0')
    {
        lResult = GetDXMediaPath(szDir);

        // If the DirectX SDK is not installed, use the Windows media
        // directory instead.
        if (lResult != 0)
        {
            GetWindowsDirectory(szDir, MAX_PATH);
            _tcscat(szDir, _T("\\Media\\") );
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


LONG CJukeboxDlg::GetDXMediaPath(TCHAR *szPath)
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


void CJukeboxDlg::FillFileList(LPTSTR pszRootDir)
{
    UINT attr = 0;

    m_ListFiles.ResetContent();

    ::SetCurrentDirectory(pszRootDir);
    Say(TEXT("Building file list..."));

    // Add all of our known supported media types to the file list.
    // Add files of each type in order.
    for (int i=0; i < NUM_MEDIA_TYPES; i++)
    {
        m_ListFiles.Dir(attr, TypeInfo[i].pszType);
    }
    Say(TEXT("File list complete."));

    // Update list box title with number of items added
    int nItems  = m_ListFiles.GetCount();
    TCHAR szTitle[64];
    wsprintf(szTitle, TEXT("Media files (%d found)"), nItems);
    m_StrFileList.SetWindowText(szTitle);
    
    // Automatically select the first file in the list once
    // the dialog is displayed.
    PostMessage(WM_FIRSTFILE, 0, 0L);
    m_nCurrentFileSelection = -1;     // No selection yet
}


void CJukeboxDlg::OnClose() 
{
    // Release DirectShow interfaces
    StopMedia();
    FreeDirectShow();

    // Release COM
    CoUninitialize();

	CDialog::OnClose();
}

void CJukeboxDlg::OnDestroy() 
{
    FreeDirectShow();	

	CDialog::OnDestroy();
}


void CJukeboxDlg::OnSelectFile() 
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

    // Remember current play state to restart playback
    int nCurrentState = g_psCurrent;

    // First release any existing interfaces
    ResetDirectShow();

    // Clear filter/pin/event information listboxes
    m_ListFilters.ResetContent();
    m_ListPinsInput.ResetContent();
    m_ListPinsOutput.ResetContent();
    m_ListEvents.ResetContent();

    // Load the selected media file
    hr = PrepareMedia(szFilename);
    if (FAILED(hr))
    {
        // Error - disable play button and give feedback
        Say(TEXT("File failed to render!"));
        m_ButtonPlay.EnableWindow(FALSE);
        MessageBeep(0);
        return;
    }
    else
    {
        m_ButtonPlay.EnableWindow(TRUE);
    }

    // Display useful information about this file
    DisplayFileInfo(szFilename);
    DisplayImageInfo();
    DisplayFileDuration();

    // Set up the seeking trackbar and read capabilities
    ConfigureSeekbar();

    // Enumerate and display filters in graph
    hr = EnumFilters();

    // Select the first filter in the list to display pin info
    m_ListFilters.SetCurSel(0);
    OnSelchangeListFilters();

    // See if the renderer supports frame stepping on this file.
    // Enable/disable frame stepping button accordingly
    m_ButtonFrameStep.EnableWindow(CanStep());

    // If the user has asked to mute audio then we
    // need to mute this new clip before continuing.
    if (g_bGlobalMute)
        MuteAudio();

    // If we were running when the user changed selection,
    // start running the newly selected clip
    if (nCurrentState == State_Running)
    {
        OnPlay();
    }
    else
    {
        // Cue the first video frame
        OnStop();
    }
}


BOOL CJukeboxDlg::IsWindowsMediaFile(LPTSTR lpszFile)
{
    if (_tcsstr(lpszFile, TEXT(".asf")) ||
        _tcsstr(lpszFile, TEXT(".ASF")) ||
        _tcsstr(lpszFile, TEXT(".wma")) ||
        _tcsstr(lpszFile, TEXT(".WMA")) ||
        _tcsstr(lpszFile, TEXT(".wmv")) ||
        _tcsstr(lpszFile, TEXT(".WMV")))
        return TRUE;
    else
        return FALSE;
}

HRESULT CJukeboxDlg::PrepareMedia(LPTSTR lpszMovie)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

    Say(TEXT("Loading..."));

    // Is this a Windows Media file (ASF, WMA, WMV)?  If so, use the new
    // ASF Reader filter, which is faster and much better at seeking than
    // the default ASF Reader filter used by default with RenderFile.
    if (IsWindowsMediaFile(lpszMovie))
    {
        hr = RenderWMFile(T2W(lpszMovie));
        if (FAILED(hr)) {
            RetailOutput(TEXT("*** Failed(%08lx) to Render WM File(%s)!\r\n"),
                     hr, lpszMovie);
            return hr;
        }
    }
    else
    {
        // Allow DirectShow to create the FilterGraph for this media file
        hr = pGB->RenderFile(T2W(lpszMovie), NULL);
        if (FAILED(hr)) {
            RetailOutput(TEXT("*** Failed(%08lx) in RenderFile(%s)!\r\n"),
                     hr, lpszMovie);
            return hr;
        }
    }

    // Set the message drain of the video window to point to our main
    // application window.  If this is an audio-only or MIDI file, 
    // then put_MessageDrain will fail.
    hr = pVW->put_MessageDrain((OAHWND) m_hWnd);
    if (FAILED(hr))
    {
        g_bAudioOnly = TRUE;
    }

    // Have the graph signal event via window callbacks
    hr = pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);

    // Configure the video window
    if (!g_bAudioOnly)
    {
        // We'll manually set the video to be visible
        hr = pVW->put_Visible(OAFALSE);

        hr = pVW->put_WindowStyle(WS_CHILD);
        hr = pVW->put_Owner((OAHWND) m_Screen.GetSafeHwnd());

        // Place video window within the bounding rectangle
        CenterVideo();

        // Make the video window visible within the screen window.
        // If this is an audio-only file, then there won't be a video interface.
        hr = pVW->put_Visible(OATRUE);
        hr = pVW->SetWindowForeground(-1);
    }

    Say(TEXT("Ready"));
    return hr;
}

HRESULT CJukeboxDlg::RenderWMFile(LPCWSTR wFile)
{
    HRESULT hr=S_OK;
    IFileSourceFilter *pFS=NULL;
    IBaseFilter *pReader=NULL;

    // Load the improved ASF reader filter by CLSID
    hr = CreateFilter(CLSID_WMAsfReader, &pReader);
    if(FAILED(hr))
	{
        RetailOutput(TEXT("Failed to create WMAsfWriter filter!  hr=0x%x\n"), hr);
		return hr;
	}

    // Add the ASF reader filter to the graph.  For ASF/WMV/WMA content,
    // this filter is NOT the default and must be added explicitly.
    hr = pGB->AddFilter(pReader, L"ASF Reader");
    if(FAILED(hr))
	{
        RetailOutput(TEXT("Failed to add ASF reader filter to graph!  hr=0x%x\n"), hr);
		return hr;
	}

    // Create the key provider that will be used to unlock the WM SDK
    JIF(AddKeyProvider(pGB));

    // Set its source filename
    JIF(pReader->QueryInterface(IID_IFileSourceFilter, (void **) &pFS));
    JIF(pFS->Load(wFile, NULL));
    pFS->Release();

    // Render the output pins of the ASF reader to build the
    // remainder of the graph automatically
    JIF(RenderOutputPins(pGB, pReader));

    // Since the graph is built and the filters are added to the graph,
    // the WM ASF reader interface can be released.
    pReader->Release();

CLEANUP:
    return hr;
}

HRESULT CJukeboxDlg::CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{
    HRESULT hr;

    hr = CoCreateInstance(clsid,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IBaseFilter,
        (void **) ppFilter);

    if(FAILED(hr))
    {
        RetailOutput(TEXT("CreateFilter: Failed to create filter!  hr=0x%x\n"), hr);
        *ppFilter = NULL;
        return hr;
    }

    return S_OK;
}


HRESULT CJukeboxDlg::RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pFilter)
{
	HRESULT			hr = S_OK;
	IEnumPins *		pEnumPin = NULL;
	IPin *			pConnectedPin = NULL, * pPin;
	PIN_DIRECTION	PinDirection;
	ULONG			ulFetched;

    // Enumerate all pins on the filter
	hr = pFilter->EnumPins( &pEnumPin );

	if(SUCCEEDED(hr))
	{
        // Step through every pin, looking for the output pins
		while (S_OK == (hr = pEnumPin->Next( 1L, &pPin, &ulFetched)))
		{
            // Is this pin connected?  We're not interested in connected pins.
			hr = pPin->ConnectedTo(&pConnectedPin);
			if (pConnectedPin)
			{
				pConnectedPin->Release();
				pConnectedPin = NULL;
			}

            // If this pin is not connected, render it.
			if (VFW_E_NOT_CONNECTED == hr)
			{
				hr = pPin->QueryDirection( &PinDirection );
				if ( ( S_OK == hr ) && ( PinDirection == PINDIR_OUTPUT ) )
				{
                    hr = pGB->Render(pPin);
				}
			}
			pPin->Release();

            // If there was an error, stop enumerating
            if (FAILED(hr))                      
                break;
		}
	}

    // Release pin enumerator
	pEnumPin->Release();
	return hr;
}


HRESULT CJukeboxDlg::AddKeyProvider(IGraphBuilder *pGraph)
{
    HRESULT hr;

    // Instantiate the key provider class, and AddRef it
    // so that COM doesn't try to free our static object.
    prov.AddRef();  // Don't let COM try to free our static object.

    // Give the graph an IObjectWithSite pointer to us for callbacks & QueryService.
    IObjectWithSite* pObjectWithSite = NULL;

    hr = pGraph->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
    if (SUCCEEDED(hr))
    {
        // Use the IObjectWithSite pointer to specify our key provider object.
        // The filter graph manager will use this pointer to call
        // QueryService to do the unlocking.
        // If the unlocking succeeds, then we can build our graph.
	        
        hr = pObjectWithSite->SetSite((IUnknown *) (IServiceProvider *) &prov);
        pObjectWithSite->Release();
    }

    return hr;
}

//
//  Displays a text string in a status line near the bottom of the dialog
//
void CJukeboxDlg::Say(LPTSTR szText)
{
    m_Status.SetWindowText(szText);
}

void CJukeboxDlg::OnPause() 
{  
    if (g_psCurrent == State_Paused)
    {
        RunMedia();
        StartSeekTimer();
        Say(TEXT("Running"));
    }
    else
    {
        StopSeekTimer();
        PauseMedia();
        Say(TEXT("PAUSED"));
    }
}

void CJukeboxDlg::OnPlay() 
{
  	RunMedia();
    StartSeekTimer();
    Say(TEXT("Running"));
}

void CJukeboxDlg::ShowState()
{
    HRESULT hr;

    OAFilterState fs;
    hr = pMC->GetState(500, &fs);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to read graph state!  hr=0x%x\r\n"), hr);
        return;
    }

    // Show debug output for current media state
    switch (fs)
    {
        case State_Stopped:
            RetailOutput(TEXT("State_Stopped\r\n"));
            break;
        case State_Paused:
            RetailOutput(TEXT("State_Paused\r\n"));
            break;
        case State_Running:
            RetailOutput(TEXT("State_Running\r\n"));
            break;
    }
}

void CJukeboxDlg::OnStop() 
{
    HRESULT hr;

    // Stop playback immediately with IMediaControl::Stop().
    StopSeekTimer();
    StopMedia();

    // Wait for the stop to propagate to all filters
    OAFilterState fs;
    hr = pMC->GetState(500, &fs);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to read graph state!  hr=0x%x\r\n"), hr);
    }

    // Reset to beginning of media clip
    LONGLONG pos=0;
    hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
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
    hr = pMC->StopWhenReady();
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed in StopWhenReady!  hr=0x%x\r\n"), hr);
    }

    Say(TEXT("Stopped"));

    // Reset slider bar and position label back to zero
    ReadMediaPosition();
}

HRESULT CJukeboxDlg::InitDirectShow(void)
{
    HRESULT hr = S_OK;

    g_bAudioOnly = FALSE;

    // Zero interfaces (sanity check)
    pVW = NULL;
    pBV = NULL;

    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pGB));
    JIF(pGB->QueryInterface(IID_IMediaControl,  (void **)&pMC));
    JIF(pGB->QueryInterface(IID_IMediaSeeking,  (void **)&pMS));
    JIF(pGB->QueryInterface(IID_IBasicVideo,    (void **)&pBV));
    JIF(pGB->QueryInterface(IID_IVideoWindow,   (void **)&pVW));
    JIF(pGB->QueryInterface(IID_IMediaEventEx,  (void **)&pME));

    return S_OK;

CLEANUP:
    FreeDirectShow();
    return(hr);
}

HRESULT CJukeboxDlg::FreeDirectShow(void)
{
    HRESULT hr=S_OK;

    StopSeekTimer();
    StopMedia();

    // Disable event callbacks
    if (pME)
        hr = pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

    // Hide video window and remove owner.  This is not necessary here,
    // since we are about to destroy the filter graph, but it is included
    // for demonstration purposes.  Remember to hide the video window and
    // clear its owner when destroying a window that plays video.
    if(pVW)
    {
        hr = pVW->put_Visible(OAFALSE);
        hr = pVW->put_Owner(NULL);
    }

    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pVW);
    SAFE_RELEASE(pBV);
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pGB);

    return hr;
}

void CJukeboxDlg::ResetDirectShow(void)
{
    // Destroy the current filter graph its filters.
    FreeDirectShow();

    // Reinitialize graph builder and query for interfaces
    InitDirectShow();
}

void CJukeboxDlg::CenterVideo(void)
{
    LONG width, height;
    HRESULT hr;

    if ((g_bAudioOnly) || (!pVW))
        return;

    // Read coordinates of video container window
    RECT rc;
    m_Screen.GetClientRect(&rc);
    width =  rc.right - rc.left;
    height = rc.bottom - rc.top;

    // Ignore the video's original size and stretch to fit bounding rectangle
    hr = pVW->SetWindowPosition(rc.left, rc.top, width, height);
    if (FAILED(hr))
    {
        RetailOutput(TEXT("Failed to set window position!  hr=0x%x\r\n"), hr);
        return;
    }
}


LRESULT CJukeboxDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// Field notifications from the DirectShow filter graph manager
    // and those posted by the application
    switch (message)
    {
        case WM_GRAPHNOTIFY:
            HandleGraphEvent();
            break;

        case WM_HSCROLL:
            HandleTrackbar(LOWORD(wParam));
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
    if (pVW)
    {
        pVW->NotifyOwnerMessage((LONG_PTR) m_hWnd, message, wParam, lParam);
    }

	return CDialog::WindowProc(message, wParam, lParam);
}


HRESULT CJukeboxDlg::HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

	// Since we may have a scenario where we're shutting down the application,
	// but events are still being generated, make sure that the event
	// interface is still valid before using it.  It's possible that
	// the interface could be freed during shutdown but later referenced in
	// this callback before the app completely exits.
	if (!pME)
		return S_OK;
    
    while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, 
                    (LONG_PTR *) &evParam2, 0)))
    {
        // Spin through the events
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);

        if(EC_COMPLETE == evCode)
        {
            // If looping, reset to beginning and continue playing
            if (g_bLooping)
            {
                LONGLONG pos=0;

                // Reset to first frame of movie
                hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                       NULL, AM_SEEKING_NoPositioning);
                if (FAILED(hr))
                {
                    // Some custom filters (like the Windows CE MIDI filter) 
                    // may not implement seeking interfaces (IMediaSeeking)
                    // to allow seeking to the start.  In that case, just stop 
                    // and restart for the same effect.  This should not be
                    // necessary in most cases.
                    StopMedia();
                    RunMedia();
                }
            }
            else if (g_bPlayThrough)
            {
                // Tell the app to select the next file in the list
                PostMessage(WM_NEXTFILE, 0, 0);
            }
            else
            {
                // Stop playback and display first frame of movie
                OnStop();
            }
        }

        //  If requested, display DirectShow events received
        if (g_bDisplayEvents)
        {
            DisplayECEvent(evCode, evParam1, evParam2);
        }
    }

    return hr;
}

void CJukeboxDlg::OnCheckMute() 
{
    // Remember global mute status for next file.  When you destroy a
    // filtergraph, you lose all of its audio settings.  Therefore, when
    // we create the next graph, we will mute the audio before running
    // the graph if this global variable is set.
    g_bGlobalMute ^= 1; 

    if (g_bGlobalMute)
        MuteAudio();
    else
        ResumeAudio();
}

void CJukeboxDlg::OnCheckLoop() 
{
    g_bLooping ^= 1;

    // Looping and play-through are mutually exclusive
    if ((g_bLooping) && (g_bPlayThrough))
    {
        // Disable play-through and uncheck button
        g_bPlayThrough = 0;
        m_CheckPlaythrough.SetCheck(0);
    }
}

void CJukeboxDlg::OnCheckPlaythrough() 
{
    g_bPlayThrough ^= 1;	

    // Looping and play-through are mutually exclusive
    if ((g_bPlayThrough) && (g_bLooping) )
    {
        // Disable play-through and uncheck button
        g_bLooping = 0;
        m_CheckLoop.SetCheck(0);
    }
}

void CJukeboxDlg::OnCheckEvents() 
{
    g_bDisplayEvents ^= 1;	
}

void CJukeboxDlg::OnButtonClearEvents() 
{
    m_ListEvents.ResetContent();
}

void CJukeboxDlg::OnButtonSetMediadir() 
{
    // Make sure that we're not playing media
    OnStop();

    // Read the string in the media directory edit box.
    TCHAR szEditPath[MAX_PATH];
    DWORD dwAttr;

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


void CJukeboxDlg::OnDblclkListFiles() 
{
    // Because it might take time to render the file and display
    // its first frame, it's better to post a message that tells
    // the app to play the selected file when ready.
    PostMessage(WM_PLAYFILE, 0, 0L);
}

void CJukeboxDlg::PlaySelectedFile() 
{
    OnPlay();
}

void CJukeboxDlg::OnDeltaposSpinFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    if (pNMUpDown->iDelta > 0)
        PostMessage(WM_NEXTFILE, 0, 0L);
    else
        PostMessage(WM_PREVIOUSFILE, 0, 0L);

	*pResult = 0;
}

void CJukeboxDlg::PlayNextFile(void)
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


void CJukeboxDlg::PlayPreviousFile(void)
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


HRESULT CJukeboxDlg::EnumFilters (void) 
{
    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;

    // Clear filters list box
    m_ListFilters.ResetContent();
    
    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
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
            USES_CONVERSION;

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
IBaseFilter *CJukeboxDlg::FindFilterFromName(LPTSTR szNameToFind)
{
    USES_CONVERSION;

    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    ULONG cFetched;
    BOOL bFound = FALSE;

    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
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


HRESULT CJukeboxDlg::EnumPins(IBaseFilter *pFilter, PIN_DIRECTION PinDir,
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


void CJukeboxDlg::OnSelchangeListFilters() 
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


BOOL CJukeboxDlg::DisplayImageInfo(void)
{
    HRESULT hr;
    long lWidth, lHeight;
    
    // If this file has no video component, clear the text field
    if ((!pBV) || (g_bAudioOnly))
    {
        m_StrImageSize.SetWindowText(TEXT("\0"));
        return FALSE;
    }

    hr = pBV->GetVideoSize(&lWidth, &lHeight);
    if (SUCCEEDED(hr))
    {
        TCHAR szSize[64];
        wsprintf(szSize, TEXT("Image size: %d x %d\0"), lWidth, lHeight);
        m_StrImageSize.SetWindowText(szSize);
    }

    return TRUE;
}


BOOL CJukeboxDlg::DisplayFileInfo(LPTSTR szFile)
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

        wsprintf(szScrap, TEXT("File date: %02d/%02d/%d\0"), 
                 time.GetMonth(), time.GetDay(), time.GetYear());
        m_StrFileDate.SetWindowText(szScrap);
    }

    CloseHandle(hFile);

    // Update size/date windows
    wsprintf(szScrap, TEXT("Size: %d bytes\0"), dwSizeLow);
    m_StrFileSize.SetWindowText(szScrap);

    return TRUE;
}


HRESULT CJukeboxDlg::DisplayFileDuration(void)
{
    HRESULT hr;

    if (!pMS)
        return E_NOINTERFACE;

    // Initialize the display in case we can't read the duration
    m_StrDuration.SetWindowText(TEXT("<00:00.000>"));

    // Is media time supported for this file?
    if (S_OK != pMS->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME))
        return E_NOINTERFACE;

    // Read the time format to restore later
    GUID guidOriginalFormat;
    hr = pMS->GetTimeFormat(&guidOriginalFormat);
    if (FAILED(hr))
        return hr;

    // Ensure media time format for easy display
    hr = pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
    if (FAILED(hr))
        return hr;

    // Read the file's duration
    LONGLONG llDuration;
    hr = pMS->GetDuration(&llDuration);
    if (FAILED(hr))
        return hr;

    // Return to the original format
    if (guidOriginalFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = pMS->SetTimeFormat(&guidOriginalFormat);
        if (FAILED(hr))
            return hr;
    }

    // Convert the LONGLONG duration into human-readable format
    unsigned long nTotalMS = (unsigned long) llDuration / 10000; // 100ns -> ms
    int nMS = nTotalMS % 1000;
    int nSeconds = nTotalMS / 1000;
    int nMinutes = nSeconds / 60;
    nSeconds %= 60;

    // Update the display
    TCHAR szDuration[24];
    wsprintf(szDuration, _T("%02dm:%02d.%03ds\0"), nMinutes, nSeconds, nMS);
    m_StrDuration.SetWindowText(szDuration);

    return hr;
}


BOOL CJukeboxDlg::SupportsPropertyPage(IBaseFilter *pFilter) 
{
    HRESULT hr;
    ISpecifyPropertyPages *pSpecify;

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

void CJukeboxDlg::OnButtonProppage() 
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

void CJukeboxDlg::OnDblclkListFilters() 
{
    OnButtonProppage();
}


void CJukeboxDlg::DisplayECEvent(long lEventCode, long lParam1, long lParam2)
{
    static TCHAR szMsg[256];
    BOOL bMatch = TRUE;

#define HANDLE_EC(c)                              \
    case c:                                       \
        wsprintf(szMsg, TEXT("%s\0"), TEXT(#c));  \
        break;

    switch (lEventCode)
    {
        HANDLE_EC(EC_ACTIVATE);
        HANDLE_EC(EC_BUFFERING_DATA);
        HANDLE_EC(EC_CLOCK_CHANGED);
        HANDLE_EC(EC_COMPLETE);
        HANDLE_EC(EC_DEVICE_LOST);
        HANDLE_EC(EC_DISPLAY_CHANGED);
        HANDLE_EC(EC_END_OF_SEGMENT);
        HANDLE_EC(EC_ERROR_STILLPLAYING);
        HANDLE_EC(EC_ERRORABORT);
        HANDLE_EC(EC_EXTDEVICE_MODE_CHANGE);
        HANDLE_EC(EC_FULLSCREEN_LOST);
        HANDLE_EC(EC_GRAPH_CHANGED);
        HANDLE_EC(EC_LENGTH_CHANGED);
        HANDLE_EC(EC_NEED_RESTART);
        HANDLE_EC(EC_NOTIFY_WINDOW);
        HANDLE_EC(EC_OLE_EVENT);
        HANDLE_EC(EC_OPENING_FILE);
        HANDLE_EC(EC_PALETTE_CHANGED);
        HANDLE_EC(EC_PAUSED);
        HANDLE_EC(EC_QUALITY_CHANGE);
        HANDLE_EC(EC_REPAINT);
        HANDLE_EC(EC_SEGMENT_STARTED);
        HANDLE_EC(EC_SHUTTING_DOWN);
        HANDLE_EC(EC_SNDDEV_IN_ERROR);
        HANDLE_EC(EC_SNDDEV_OUT_ERROR);
        HANDLE_EC(EC_STARVATION);
        HANDLE_EC(EC_STEP_COMPLETE);
        HANDLE_EC(EC_STREAM_CONTROL_STARTED);
        HANDLE_EC(EC_STREAM_CONTROL_STOPPED);
        HANDLE_EC(EC_STREAM_ERROR_STILLPLAYING);
        HANDLE_EC(EC_STREAM_ERROR_STOPPED);
        HANDLE_EC(EC_TIMECODE_AVAILABLE);
        HANDLE_EC(EC_USERABORT);
        HANDLE_EC(EC_VIDEO_SIZE_CHANGED);
        HANDLE_EC(EC_WINDOW_DESTROYED);

    default:
        bMatch = FALSE;
        RetailOutput(TEXT("  Received unknown event code (0x%x)\r\n"), lEventCode);
        break;
    }

    // If a recognized event was found, add its name to the events list box
    if (bMatch)
        m_ListEvents.AddString(szMsg);
}


void CJukeboxDlg::OnButtonFramestep() 
{
    StepFrame();
}


//
// Some hardware decoders and video renderers support stepping media
// frame by frame with the IVideoFrameStep interface.  See the interface
// documentation for more details on frame stepping.
//
BOOL CJukeboxDlg::CanStep(void)
{
    HRESULT hr;
    IVideoFrameStep* pFS;

    hr = pGB->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFS);
    if (FAILED(hr))
        return FALSE;

    // Check if this decoder can step
    hr = pFS->CanStep(0L, NULL); 

    pFS->Release();

    if (hr == S_OK)
        return TRUE;
    else
        return FALSE;
}

HRESULT CJukeboxDlg::StepFrame(void)
{
    // Get the Frame Stepping Interface
    HRESULT hr;
    IVideoFrameStep* pFS;

    hr = pGB->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFS);
    if (FAILED(hr))
        return hr;

    // The graph must be paused for frame stepping to work
    if (g_psCurrent != State_Paused)
        OnPause();

    // Step one frame
    hr = pFS->Step(1, NULL); 
    pFS->Release();

    // Since the media position has changed slightly, update the
    // slider bar and position label.
    ReadMediaPosition();
    return hr;
}


void CJukeboxDlg::ConfigureSeekbar()
{
    // Disable seekbar for new file and reset tracker/position label
    m_Seekbar.SetPos(0);
    m_StrPosition.SetWindowText(TEXT("Position: 00m:00s\0"));
    g_rtTotalTime=0;

    // If we can't read the file's duration, disable the seek bar
    if (pMS && SUCCEEDED(pMS->GetDuration(&g_rtTotalTime)))
        m_Seekbar.EnableWindow(TRUE);
    else
        m_Seekbar.EnableWindow(FALSE);
}

void CJukeboxDlg::StartSeekTimer() 
{
    // Cancel any pending timer event
    StopSeekTimer();

    // Create a new timer
    g_wTimerID = SetTimer(TIMERID, TICKLEN, NULL);
}

void CJukeboxDlg::StopSeekTimer() 
{
    // Cancel the timer
    if(g_wTimerID)        
    {                
        KillTimer(g_wTimerID);
        g_wTimerID = 0;
    }
}

void CJukeboxDlg::OnTimer(UINT nIDEvent) 
{
    ReadMediaPosition();

	CDialog::OnTimer(nIDEvent);
}

void CJukeboxDlg::ReadMediaPosition()
{
    HRESULT hr;
    REFERENCE_TIME rtNow;

    // Read the current stream position
    hr = pMS->GetCurrentPosition(&rtNow);
    if (FAILED(hr))
        return;

    // Convert position into a percentage value and update slider position
    if (g_rtTotalTime != 0)
    {
        long lTick = (long)((rtNow * 100) / g_rtTotalTime);
        m_Seekbar.SetPos(lTick);
    }
    else
        m_Seekbar.SetPos(0);
	
    // Update the 'current position' string on the main dialog
    UpdatePosition(rtNow);
}

void CJukeboxDlg::UpdatePosition(REFERENCE_TIME rtNow) 
{
    HRESULT hr;

    // If no reference time was passed in, read the current position
    if (rtNow == 0)
    {
        // Read the current stream position
        hr = pMS->GetCurrentPosition(&rtNow);
        if (FAILED(hr))
            return;
    }

    // Convert the LONGLONG duration into human-readable format
    unsigned long nTotalMS = (unsigned long) rtNow / 10000; // 100ns -> ms
    int nSeconds = nTotalMS / 1000;
    int nMinutes = nSeconds / 60;
    nSeconds %= 60;

    // Update the display
    TCHAR szPosition[24], szCurrentString[24];
    wsprintf(szPosition, _T("Position: %02dm:%02ds\0"), nMinutes, nSeconds);

    // Read current string and compare to the new string.  To prevent flicker,
    // don't update this label unless the string has changed.
    m_StrPosition.GetWindowText(szCurrentString, 24);

    if (_tcscmp(szCurrentString, szPosition))
        m_StrPosition.SetWindowText(szPosition);
}

void CJukeboxDlg::HandleTrackbar(WPARAM wReq)
{
    HRESULT hr;
    static OAFilterState state;
    static BOOL bStartOfScroll = TRUE;

    // If the file is not seekable, the trackbar is disabled. 
    DWORD dwPosition = m_Seekbar.GetPos();

    // Pause when the scroll action begins.
    if (bStartOfScroll) 
    {       
        hr = pMC->GetState(10, &state);
        bStartOfScroll = FALSE;
        hr = pMC->Pause();
    }
    
    // Update the position continuously.
    REFERENCE_TIME rtNew = (g_rtTotalTime * dwPosition) / 100;

    hr = pMS->SetPositions(&rtNew, AM_SEEKING_AbsolutePositioning,
                           NULL,   AM_SEEKING_NoPositioning);

    // Restore the state at the end.
    if (wReq == TB_ENDTRACK)
    {
        if (state == State_Stopped)
            hr = pMC->Stop();
        else if (state == State_Running) 
            hr = pMC->Run();

        bStartOfScroll = TRUE;
    }

    // Update the 'current position' string on the main dialog.
    UpdatePosition(rtNew);
}

LONG CJukeboxDlg::GetGraphEditPath(TCHAR *szPath)
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

void CJukeboxDlg::OnButtonGraphedit() 
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

BOOL CJukeboxDlg::OnEraseBkgnd(CDC *pDC)
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
