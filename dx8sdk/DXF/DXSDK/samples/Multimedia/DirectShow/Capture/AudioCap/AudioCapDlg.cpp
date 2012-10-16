//------------------------------------------------------------------------------
// File: AudioCapDlg.cpp
//
// Desc: DirectShow sample code - Implementation of CAudioCapDlg class for
//       audio capture sample application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AudioCap.h"
#include "AudioCapDlg.h"
#include <atlbase.h>

#include <mtype.h>
#include "mfcutil.h"
#include "dshowutil.h"

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
// CAudioCapDlg dialog

CAudioCapDlg::CAudioCapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioCapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioCapDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAudioCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioCapDlg)
	DDX_Control(pDX, IDC_STATIC_STATUS, m_strStatus);
	DDX_Control(pDX, IDC_RADIO_MONO, m_btnMono);
	DDX_Control(pDX, IDC_RADIO_8BIT, m_btn8BIT);
	DDX_Control(pDX, IDC_RADIO_11KHZ, m_btn11KHZ);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_LIST_FILTER_OUTPUTS, m_ListFilterOutputs);
	DDX_Control(pDX, IDC_LIST_FILTER_INPUTS, m_ListFilterInputs);
	DDX_Control(pDX, IDC_BUTTON_RECORD, m_btnRecord);
	DDX_Control(pDX, IDC_BUTTON_PROPERTIES, m_btnProperties);
	DDX_Control(pDX, IDC_LIST_INPUT_DEVICES, m_ListInputs);
	DDX_Control(pDX, IDC_LIST_INPUT_PINS, m_ListInputPins);
	DDX_Control(pDX, IDC_LIST_FILTERS, m_ListFilters);
	DDX_Control(pDX, IDC_EDIT_FILENAME, m_StrFilename);
	DDX_Control(pDX, IDC_CHECK_WRITE, m_CheckWriteFile);
	DDX_Control(pDX, IDC_CHECK_AUDITION, m_CheckAudition);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAudioCapDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioCapDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LIST_INPUT_DEVICES, OnSelchangeListInputDevices)
	ON_LBN_SELCHANGE(IDC_LIST_FILTERS, OnSelchangeListFilters)
	ON_LBN_SELCHANGE(IDC_LIST_INPUT_PINS, OnSelchangeListInputPins)
	ON_BN_CLICKED(IDC_BUTTON_PROPERTIES, OnButtonProperties)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_RECORD, OnButtonRecord)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioCapDlg message handlers

void CAudioCapDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAudioCapDlg::OnPaint() 
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
HCURSOR CAudioCapDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CAudioCapDlg::OnInitDialog()
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

    if (FAILED(InitializeCapture()))
    {
        ReleaseCapture();
        return FALSE;
    }

    CheckDlgButton(IDC_RADIO_16BIT, TRUE);
    CheckDlgButton(IDC_RADIO_STEREO, TRUE);
    CheckDlgButton(IDC_RADIO_44KHZ, TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

HRESULT CAudioCapDlg::InitializeCapture()
{
    HRESULT hr;

    SetDefaults();
	
    if (FAILED(hr = GetInterfaces()))
        return hr;

    if (FAILED(hr = FillLists()))
        return hr;

    return hr;
}

HRESULT CAudioCapDlg::FillLists() 
{
    HRESULT hr;
    
    // Enumerate and display the audio input devices installed in the system
    hr = EnumFiltersWithMonikerToList(NULL, &CLSID_AudioInputDeviceCategory, m_ListInputs);
    if (FAILED(hr))
        return hr;

    // Select the first audio device in the list for the default
    m_ListInputs.SetCurSel(0);
    OnSelchangeListInputDevices();

    return hr;
}

HRESULT CAudioCapDlg::GetInterfaces()
{
    HRESULT hr;

    // Create the filter graph.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                          IID_IGraphBuilder, (void **)&m_pGB);
    if (!m_pGB)
        return E_NOINTERFACE;

    // Create the capture graph builder.
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
                          IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
    if (!m_pCapture)
        return E_NOINTERFACE;

    // Associate the filter graph with the capture graph builder
    hr = m_pCapture->SetFiltergraph(m_pGB);    
    if (FAILED(hr))
        return hr;

    // Get useful interfaces from the graph builder
    JIF(hr = m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC));
    JIF(hr = m_pGB->QueryInterface(IID_IMediaEventEx, (void **)&m_pME));

    // Have the graph signal events via window callbacks
    hr = m_pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, RECORD_EVENT);

CLEANUP:
    return hr;
}

HRESULT CAudioCapDlg::GetPlaybackInterfaces()
{
    HRESULT hr;

    // Release any existing interface pointers
    FreePlaybackInterfaces();

    // Create the filter graph.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                          IID_IGraphBuilder, (void **)&m_pGBPlayback);
    if (!m_pGBPlayback)
        return E_NOINTERFACE;

    // Get useful interfaces from the graph builder
    JIF(hr = m_pGBPlayback->QueryInterface(IID_IMediaControl, (void **)&m_pMCPlayback));
    JIF(hr = m_pGBPlayback->QueryInterface(IID_IMediaEventEx, (void **)&m_pMEPlayback));

    // Have the graph signal events via window callbacks
    hr = m_pMEPlayback->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, PLAYBACK_EVENT);

CLEANUP:
    return hr;
}

void CAudioCapDlg::ClearAllocatedLists()
{
    // Clear the list and delete its allocated CLSID bytes
    ClearFilterListWithMoniker(m_ListInputs);
}

void CAudioCapDlg::ClearLists()
{
    // Clear the lists that don't contain extra data
    m_ListInputPins.ResetContent();

    m_ListFilters.ResetContent();
    m_ListFilterInputs.ResetContent();
    m_ListFilterOutputs.ResetContent();
}

void CAudioCapDlg::FreeInterfaces()
{
    // Disable event callbacks
    if (m_pME)
        m_pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

    // Release and zero DirectShow interface pointers
    SAFE_RELEASE(m_pMC);
    SAFE_RELEASE(m_pME);
    SAFE_RELEASE(m_pSplitter);
    SAFE_RELEASE(m_pFileWriter);
    SAFE_RELEASE(m_pWAVDest);
    SAFE_RELEASE(m_pInputDevice);
    SAFE_RELEASE(m_pGB);
    SAFE_RELEASE(m_pCapture);
}

void CAudioCapDlg::FreePlaybackInterfaces()
{
    HRESULT hr;

    // Disable event callbacks
    if (m_pMEPlayback)
        hr = m_pMEPlayback->SetNotifyWindow((OAHWND)NULL, 0, 0);

    // Release and zero DirectShow interface pointers
    SAFE_RELEASE(m_pMEPlayback);
    SAFE_RELEASE(m_pMCPlayback);
    SAFE_RELEASE(m_pGBPlayback);
}

void CAudioCapDlg::ReleaseCapture()
{
    ClearAllocatedLists();
    ClearLists();
    DestroyPreviewStream();
    DestroyCaptureStream();
    FreeInterfaces();
    FreePlaybackInterfaces();
}

void CAudioCapDlg::ResetCapture()
{
    ClearLists();
    FreeInterfaces();
    GetInterfaces();
}

void CAudioCapDlg::SetDefaults()
{
    // Zero DirectShow interface pointers (sanity check)
    m_pGB = m_pGBPlayback = 0;
    m_pMC = m_pMCPlayback = 0;
    m_pME = m_pMEPlayback = 0;
    m_pCapture = 0;
    m_pInputDevice =0;
    m_pSplitter = 0;
    m_pRenderer =0;
    m_pWAVDest=0;
    m_pFileWriter=0;
    m_bPausedRecording=FALSE;

    // Set default values for controls on the dialog
    m_CheckAudition.SetCheck(TRUE);
    m_btnProperties.EnableWindow(FALSE);
    m_StrFilename.SetWindowText(DEFAULT_FILENAME);
}

void CAudioCapDlg::EnableButtons(BOOL bEnable)
{
    m_btnRecord.EnableWindow(bEnable);
    m_btnPause.EnableWindow(bEnable);
    m_btnStop.EnableWindow(bEnable);
    m_btnPlay.EnableWindow(bEnable);
}

void CAudioCapDlg::EnableRadioButtons(BOOL bEnable)
{
    GetDlgItem(IDC_RADIO_11KHZ)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_22KHZ)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_44KHZ)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_8BIT)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_16BIT)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_MONO)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_STEREO)->EnableWindow(bEnable);
}

void CAudioCapDlg::OnClose() 
{
    if (m_pMC)
        m_pMC->Stop();

    ReleaseCapture();
    CoUninitialize();

	CDialog::OnClose();
}

void CAudioCapDlg::OnDestroy() 
{
    ReleaseCapture();
	CDialog::OnDestroy();
}

HRESULT CAudioCapDlg::HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    if (!m_pME)
        return S_OK;

    // Read all events currently on the event queue
    while(SUCCEEDED(m_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, 
                                    (LONG_PTR *) &evParam2, 0)))
    {
        if(EC_DEVICE_LOST == evCode)
        {
            // Device was removed
            if (evParam2 == 0)
            {
                OnButtonStop();
                ReleaseCapture();
                SetDefaults();
                MessageBeep(0);
                MessageBox(TEXT("The selected input device was removed!\0"),
                           TEXT("DirectShow Audio Capture Sample\0"), 
                           MB_OK | MB_ICONEXCLAMATION);
            }
            // Device was reattached
            else
            {
                // Restart from scratch
                InitializeCapture();
                MessageBeep(0);
                MessageBox(TEXT("The audio input device has been reattached!\0"),
                           TEXT("DirectShow Audio Capture Sample\0"), 
                           MB_OK | MB_ICONINFORMATION);
            }
        }

        // Free event parameters to prevent memory leaks
        hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);
    }

    return hr;
}

HRESULT CAudioCapDlg::HandlePlaybackGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    if (!m_pMEPlayback)
        return S_OK;

    // Read all events currently on the event queue
    while(SUCCEEDED(m_pMEPlayback->GetEvent(&evCode, (LONG_PTR *) &evParam1, 
                                    (LONG_PTR *) &evParam2, 0)))
    {
        // Free event parameters to prevent memory leaks
        hr = m_pMEPlayback->FreeEventParams(evCode, evParam1, evParam2);

        if (evCode == EC_COMPLETE)
        {
            OnButtonStop();
            break;
        }
    }

    return hr;
}

LRESULT CAudioCapDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// Field notifications from the DirectShow filter graph manager
    // and those posted by the application
    switch (message)
    {
        case WM_GRAPHNOTIFY:
            if (lParam == RECORD_EVENT)
                HandleGraphEvent();
            else
                HandlePlaybackGraphEvent();
            break;
    }
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CAudioCapDlg::OnButtonProperties() 
{
    // Use helper function to display property page for this device
    ShowFilterPropertyPage(m_pInputDevice, m_hWnd);

    // Increment the device's reference count
    m_pInputDevice->AddRef();
}

void CAudioCapDlg::OnSelchangeListInputDevices() 
{
    HRESULT hr;
    IMoniker *pMoniker=0;

    // Release and delete any previous capture filter graph
    if (m_pInputDevice)
        ResetCapture();

    // Get the currently selected category name
    int nItem = m_ListInputs.GetCurSel();

    // Read the stored moniker pointer from the list box's item data
    pMoniker = (IMoniker *) m_ListInputs.GetItemDataPtr(nItem);

    // Use the moniker to create the specified audio capture device
    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pInputDevice);   
    if (FAILED(hr))
        return;

    // Add the capture filter to the filter graph
    hr = m_pGB->AddFilter(m_pInputDevice, L"Audio Capture");
    if (FAILED(hr))
        return;

    // List and display the available input pins on the capture filter.
    // Many devices offer multiple pins for input (microphone, CD, LineIn, etc.)
    hr = EnumPinsOnFilter(m_pInputDevice, PINDIR_INPUT, m_ListInputPins);
    if (FAILED(hr))
        return;

    // Enable the properties button, if supported by the selected input device
    m_btnProperties.EnableWindow(SupportsPropertyPage(m_pInputDevice));

    // Select the first available input pin by default
    m_ListInputPins.SetCurSel(0);
    OnSelchangeListInputPins();

    // Now display the existing filters in the graph for diagnostic help
    UpdateFilterLists(m_pGB);
}

void CAudioCapDlg::OnSelchangeListFilters() 
{
    // Use a helper function to display the filter's pins in listboxes
    if (m_pGBPlayback)
    {
        AddFilterPinsToLists(m_pGBPlayback, m_ListFilters, 
                             m_ListFilterInputs, m_ListFilterOutputs);
    }
    else
    {
        AddFilterPinsToLists(m_pGB, m_ListFilters, 
                             m_ListFilterInputs, m_ListFilterOutputs);
    }
}

void CAudioCapDlg::UpdateFilterLists(IGraphBuilder *pUpdateGB)
{
    // Now display the existing filters in the graph for diagnostic help
    AddGraphFiltersToList(pUpdateGB, m_ListFilters);

    // Refresh the filter/pin display lists
    m_ListFilters.SetCurSel(0);
    OnSelchangeListFilters();
}

void CAudioCapDlg::OnButtonPlay() 
{
    USES_CONVERSION;
    TCHAR szPlayFile[MAX_PATH];

    Say(TEXT("Playing specified file..."));
    m_StrFilename.GetWindowText(szPlayFile, MAX_PATH);

    // Play the media file using DirectShow methods
    if (SUCCEEDED(GetPlaybackInterfaces()))
    {
        HRESULT hr = m_pGBPlayback->RenderFile(T2W(szPlayFile), NULL);
        if (SUCCEEDED(hr))
        {
            // Update the graph filter display to show rendered graph
            UpdateFilterLists(m_pGBPlayback);

            hr = m_pMCPlayback->Run();
        }
    }

    // Disable pause button to remove need for extra pause/play handling code
    m_btnPause.EnableWindow(FALSE);
}

void CAudioCapDlg::OnButtonRecord() 
{
    HRESULT hr;

    // Don't rebuild the graphs if we're returning from paused state
    if (m_bPausedRecording)
    {
        if (m_pMC)
            hr = m_pMC->Run();

        m_bPausedRecording = FALSE;
        Say(TEXT("Auditioning/recording audio..."));
        return;
    }

    // If we're currently playing back the recorded file, stop playback
    // and clear the interfaces before beginning recording.
    FreePlaybackInterfaces();

    if (m_CheckAudition.GetCheck())
        RenderPreviewStream();

    if (m_CheckWriteFile.GetCheck())
        RenderCaptureStream();

    // Update the graph filter display to show rendered graph
    UpdateFilterLists(m_pGB);

    if (m_pMC)
    {
        hr = m_pMC->Run();
        Say(TEXT("Auditioning/recording audio..."));
    }

    m_btnRecord.EnableWindow(FALSE);
    m_btnPlay.EnableWindow(FALSE);
    m_btnPause.EnableWindow(TRUE);
    EnableRadioButtons(FALSE);
}

void CAudioCapDlg::OnButtonPause() 
{
    HRESULT hr;

    // We share the functionality of the Pause button with both
    // Record and Playback.  If the Playback interfaces aren't NULL,
    // then we're in the middle of playback, so pause playback.
    // Otherwise, pause recording.
    if (m_pMCPlayback)
    {
        hr = m_pMCPlayback->Pause();        
        Say(TEXT("Paused audio playback"));
    }
    else
    {
        if (m_pMC)
        {
            hr = m_pMC->Pause();
            Say(TEXT("Paused audio recording"));
            m_bPausedRecording = TRUE;
        }
    }

    m_btnRecord.EnableWindow(TRUE);
}

void CAudioCapDlg::OnButtonStop() 
{
    HRESULT hr;

    // We share the functionality of the Stop button with both
    // Record and Playback.  If the Playback interfaces aren't NULL,
    // then we're in the middle of playback, so stop playback.
    // Otherwise, stop recording.
    if (m_pMCPlayback)
    {
        // Stop playback
        if (m_pMCPlayback)
            hr = m_pMCPlayback->Stop();

        FreePlaybackInterfaces();

        // Clear filter/pins list boxes, since there is no active filter graph
        m_ListFilters.ResetContent();
        m_ListFilterInputs.ResetContent();
        m_ListFilterOutputs.ResetContent();
    }
    else
    {
        // Stop recording
        if (m_pMC)
            hr = m_pMC->StopWhenReady();

        // Destroy the existing filter graph
        DestroyPreviewStream();
        DestroyCaptureStream();
        m_bPausedRecording = FALSE;

        // Update the graph filter display to show rendered graph
        UpdateFilterLists(m_pGB);

        EnableRadioButtons(TRUE);
    }

    EnableButtons(TRUE);
    Say(TEXT("Ready"));
}

HRESULT CAudioCapDlg::RenderCaptureStream()
{
    USES_CONVERSION;
    HRESULT hr=0;
    IFileSinkFilter2 *pFileSink;

    // Create the WAVDest filter
    hr = CoCreateInstance(CLSID_WavDest, NULL, CLSCTX_INPROC,
                          IID_IBaseFilter, (void **)&m_pWAVDest);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Couldn't create a WAVDest filter.  This sample filter is installed ")
                   TEXT("with the DirectX 8.0 SDK in the DirectShow\\Filters\\WavDest ")
                   TEXT("directory.\n\nPlease build this filter (with Visual C++) and ")
                   TEXT("register it by running 'Tools->Register Control' once the ")
                   TEXT("filter finishes building.\nThis helper filter is used ")
                   TEXT("when writing .WAV files to disk, and this sample requires it.\n\n")
                   TEXT("You may still audition the audio input, but you will not be ")
                   TEXT("able to write the data to a file."),
                   TEXT("Error loading WAVDest helper filter!"));
        return E_NOINTERFACE; 
    }

    // Create the FileWriter filter
    hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC,
                          IID_IFileSinkFilter2, (void **)&pFileSink);
    if (FAILED(hr))
        return E_NOINTERFACE; 

    // Get the file sink interface from the File Writer
    hr = pFileSink->QueryInterface(IID_IBaseFilter, (void **)&m_pFileWriter);

    // Add the WAVDest filter to the graph
    hr = m_pGB->AddFilter(m_pWAVDest, L"WAV Dest");
    if (FAILED(hr))
        return hr;

    // Add the FileWriter filter to the graph
    hr = m_pGB->AddFilter((IBaseFilter *)m_pFileWriter, L"File Writer");
    if (FAILED(hr))
        return hr;

    // Set filter to always overwrite the file
    hr = pFileSink->SetMode(AM_FILE_OVERWRITE);

    // Set the output filename, which must be a wide string
    TCHAR szEditFile[MAX_PATH];
    WCHAR wszFilename[MAX_PATH];

    m_StrFilename.GetWindowText(szEditFile, MAX_PATH);
    if (szEditFile[0] == TEXT('\0'))
        wcscpy(wszFilename, T2W(DEFAULT_FILENAME));
    else
        wcscpy(wszFilename, T2W(szEditFile));

    hr = pFileSink->SetFileName(wszFilename, NULL);
    if (FAILED(hr))
        return hr; 

    // Get the pin interface for the capture pin
    IPin *pPin=0;
    if (m_pSplitter)
        hr = GetPin(m_pSplitter, PINDIR_OUTPUT, 0, &pPin);
    else
        hr = GetPin(m_pInputDevice, PINDIR_OUTPUT, 0, &pPin);

    // Connect the new filters
    if (pPin)
    {
        hr = m_pGB->Render(pPin);
        pPin->Release();
    }

    // Release the FileSinkFilter2 interface, since it's no longer needed
    SAFE_RELEASE(pFileSink);
    return hr;
}

HRESULT CAudioCapDlg::DestroyCaptureStream()
{
    HRESULT hr=0;

    // Destroy Audio renderer filter, if it exists
    if (m_pWAVDest)
    {
        hr = m_pGB->RemoveFilter(m_pWAVDest);
        SAFE_RELEASE(m_pWAVDest);
    }

    // Destroy Smart Tee filter, if it exists
    if (m_pFileWriter)
    {
        hr = m_pGB->RemoveFilter(m_pFileWriter);
        SAFE_RELEASE(m_pFileWriter);
    }

    return hr;
}

HRESULT CAudioCapDlg::RenderPreviewStream()
{
    USES_CONVERSION;
    HRESULT hr=0;
    WCHAR wszFilter[64];

    // If we've already configured the stream, just exit
    if (m_pRenderer)
        return S_OK;

    // Set the requested audio properties - buffer sizes, channels, freq, etc.
    hr = SetAudioProperties();

    // Render the preview stream
    hr = m_pCapture->RenderStream(
            &PIN_CATEGORY_PREVIEW, 
            &MEDIATYPE_Audio, 
            m_pInputDevice, 
            NULL,   // No compression filter.
            NULL    // Default renderer.
        );

    // Some capture sources will have only a Capture pin (no Preview pin).
    // In that case, the Capture Graph builder will automatically insert
    // a SmartTee filter, which will split the stream into a Capture stream
    // and a Preview stream.  In that case, it's not an error.
    if (hr == VFW_S_NOPREVIEWPIN)
    {
        wcscpy(wszFilter, T2W(TEXT("Smart Tee")));

        // Get the interface for the Splitter filter for later use
        hr = m_pGB->FindFilterByName(wszFilter, &m_pSplitter);
    }

    // Get the interface for the audio renderer filter for later use
    wcscpy(wszFilter, T2W(TEXT("Audio Renderer")));
    hr = m_pGB->FindFilterByName(wszFilter, &m_pRenderer);   

    return hr;
}

HRESULT CAudioCapDlg::DestroyPreviewStream()
{
    HRESULT hr=0;

    // Destroy Smart Tee filter, if it exists
    if (m_pSplitter)
    {
        hr = m_pGB->RemoveFilter(m_pSplitter);
        SAFE_RELEASE(m_pSplitter);
    }

    // Destroy Audio renderer filter, if it exists
    if (m_pRenderer)
    {
        hr = m_pGB->RemoveFilter(m_pRenderer);
        SAFE_RELEASE(m_pRenderer);
    }

    return hr;
}

void CAudioCapDlg::OnSelchangeListInputPins() 
{
    HRESULT hr;

    // Make sure that playback is stopped
    OnButtonStop();

    // Select the capture device's input pin
    hr = ActivateSelectedInputPin();
}

HRESULT CAudioCapDlg::ActivateSelectedInputPin() 
{
    HRESULT hr=S_OK;
    IPin *pPin=0;
    IAMAudioInputMixer *pPinMixer;

    // How many pins are in the input pin list?
    int nPins = m_ListInputPins.GetCount();
    int nActivePin = m_ListInputPins.GetCurSel();

    // Activate the selected input pin and deactivate all others
    for (int i=0; i<nPins; i++)
    {
        // Get this pin's interface
        hr = GetPin(m_pInputDevice, PINDIR_INPUT, i, &pPin);
        if (SUCCEEDED(hr))
        {
            hr = pPin->QueryInterface(IID_IAMAudioInputMixer, (void **)&pPinMixer);
            if (SUCCEEDED(hr))
            {
                // If this is our selected pin, enable it
                if (i == nActivePin)
                {
                    // Set any other audio properties on this pin
                    hr = SetInputPinProperties(pPinMixer);

                    // If there is only one input pin, this method
                    // might return E_NOTIMPL.
                    hr = pPinMixer->put_Enable(TRUE);
                }
                // Otherwise, disable it
                else
                {
                    hr = pPinMixer->put_Enable(FALSE);
                }

                pPinMixer->Release();
            }

            // Release pin interfaces
            pPin->Release();
        }
    }

    return hr;
}

HRESULT CAudioCapDlg::SetInputPinProperties(IAMAudioInputMixer *pPinMixer) 
{
    HRESULT hr=0;
    BOOL bLoudness=0, bMono=0;
    double dblBass=0, dblBassRange=0, dblMixLevel=0, 
           dblPan=0, dblTreble=0, dblTrebleRange=0;

    // Read current settings for debugging purposes.  Many of these interfaces
    // will not be implemented by the input device's driver, so they will
    // return E_NOTIMPL.  In that case, just ignore the values.
    hr = pPinMixer->get_Bass(&dblBass);
    hr = pPinMixer->get_BassRange(&dblBassRange);
    hr = pPinMixer->get_Loudness(&bLoudness);
    hr = pPinMixer->get_MixLevel(&dblMixLevel);
    hr = pPinMixer->get_Mono(&bMono);
    hr = pPinMixer->get_Pan(&dblPan);
    hr = pPinMixer->get_Treble(&dblTreble);
    hr = pPinMixer->get_TrebleRange(&dblTrebleRange);

    // Manipulate desired values here (mono/stereo, pan, etc.)

    return hr;    
}

void CAudioCapDlg::Say(TCHAR *szMsg) 
{
    // Display status information on the main dialog
    m_strStatus.SetWindowText(szMsg);
}

HRESULT CAudioCapDlg::SetAudioProperties()
{
    HRESULT hr=0;
    IPin *pPin=0;
    IAMBufferNegotiation *pNeg=0;
    IAMStreamConfig *pCfg=0;
    int nFrequency;

    // Determine audio properties
    int nChannels = m_btnMono.GetCheck() ? 1 : 2;
    int nBytesPerSample = m_btn8BIT.GetCheck() ? 1 : 2;

    if (IsDlgButtonChecked(IDC_RADIO_11KHZ))      nFrequency = 11025;
    else if (IsDlgButtonChecked(IDC_RADIO_22KHZ)) nFrequency = 22050;
    else nFrequency = 44100;

    // Find number of bytes in one second
    long lBytesPerSecond = (long) (nBytesPerSample * nFrequency * nChannels);

    // Set to 50ms worth of data    
    long lBufferSize = (long) ((float) lBytesPerSecond * DEFAULT_BUFFER_TIME);

    // Get the buffer negotiation interface for each pin,
    // since there could be both a Capture and a Preview pin.
    for (int i=0; i<2; i++)
    {
        hr = GetPin(m_pInputDevice, PINDIR_OUTPUT, i, &pPin);
        if (SUCCEEDED(hr))
        {
            // Get buffer negotiation interface
            hr = pPin->QueryInterface(IID_IAMBufferNegotiation, (void **)&pNeg);
            if (FAILED(hr))
            {
                pPin->Release();
                break;
            }

            // Set the buffer size based on selected settings
            ALLOCATOR_PROPERTIES prop={0};
            prop.cbBuffer = lBufferSize;
            prop.cBuffers = 6;
            prop.cbAlign = nBytesPerSample * nChannels;
            hr = pNeg->SuggestAllocatorProperties(&prop);
            pNeg->Release();

            // Now set the actual format of the audio data
            hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pCfg);
            if (FAILED(hr))
            {
                pPin->Release();
                break;
            }            

            // Read current media type/format
            AM_MEDIA_TYPE *pmt={0};
            hr = pCfg->GetFormat(&pmt);

            if (SUCCEEDED(hr))
            {
                // Fill in values for the new format
                WAVEFORMATEX *pWF = (WAVEFORMATEX *) pmt->pbFormat;
                pWF->nChannels = (WORD) nChannels;
                pWF->nSamplesPerSec = nFrequency;
                pWF->nAvgBytesPerSec = lBytesPerSecond;
                pWF->wBitsPerSample = (WORD) (nBytesPerSample * 8);
                pWF->nBlockAlign = (WORD) (nBytesPerSample * nChannels);

                // Set the new formattype for the output pin
                hr = pCfg->SetFormat(pmt);
                DeleteMediaType(pmt);
            }

            // Release interfaces
            pCfg->Release();
            pPin->Release();
        }
        // No more output pins on this filter
        else
            break;
    }

    return hr;
}
