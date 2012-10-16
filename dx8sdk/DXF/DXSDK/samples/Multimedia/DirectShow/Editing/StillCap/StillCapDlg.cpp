//------------------------------------------------------------------------------
// File: StillCapDlg.cpp
//
// Desc: DirectShow sample code - implementation of callback and dialog
//       objects for StillCap application.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include "StillCap.h"
#include "StillCapDlg.h"
#include "..\..\common\dshowutil.cpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#ifdef DEBUG
#define REGISTER_FILTERGRAPH
#endif

// Constants
#define WM_CAPTURE_BITMAP   WM_APP + 1

// Global data
BOOL g_bOneShot = FALSE;
DWORD g_dwGraphRegister=0;  // For running object table
HWND g_hwnd;

// Structures
typedef struct _callbackinfo 
{
    double dblSampleTime;
    long lBufferSize;
    BYTE *pBuffer;
    BITMAPINFOHEADER bih;

} CALLBACKINFO;

CALLBACKINFO cb={0};


// Note: this object is a SEMI-COM object, and can only be created statically.
// We use this little semi-com object to handle the sample-grab-callback,
// since the callback must provide a COM interface. We could have had an interface
// where you provided a function-call callback, but that's really messy, so we
// did it this way. You can put anything you want into this C++ object, even
// a pointer to a CDialog. Be aware of multi-thread issues though.
//
class CSampleGrabberCB : public ISampleGrabberCB 
{
public:
    // these will get set by the main thread below. We need to
    // know this in order to write out the bmp
    long lWidth;
    long lHeight;
    CStillCapDlg * pOwner;
    TCHAR m_szCapDir[MAX_PATH]; // the directory we want to capture to
    TCHAR m_szSnappedName[MAX_PATH];
    BOOL bFileWritten;

    CSampleGrabberCB( )
    {
        pOwner = NULL;
        m_szCapDir[0] = 0;
        bFileWritten = FALSE;
    }   

    // fake out any COM ref counting
    //
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    // fake out any COM QI'ing
    //
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
        {
            *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
            return NOERROR;
        }    
        return E_NOINTERFACE;
    }

    // we don't implement this interface for this example
    //
    STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
    {
        return 0;
    }

    // The sample grabber is calling us back on its deliver thread.
    // This is NOT the main app thread!
    //
    //           !!!!! WARNING WARNING WARNING !!!!!
    //
    // On Windows 9x systems, you are not allowed to call most of the 
    // Windows API functions in this callback.  Why not?  Because the
    // video renderer might hold the global Win16 lock so that the video
    // surface can be locked while you copy its data.  This is not an
    // issue on Windows 2000, but is a limitation on Win95,98,98SE, and ME.
    // Calling a 16-bit legacy function could lock the system, because 
    // it would wait forever for the Win16 lock, which would be forever
    // held by the video renderer.
    //
    // As a workaround, copy the bitmap data during the callback,
    // post a message to our app, and write the data later.
    //
    STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
    {
        // this flag will get set to true in order to take a picture
        //
        if( !g_bOneShot )
            return 0;

        // Since we can't access Windows API functions in this callback, just
        // copy the bitmap data to a global structure for later reference.
        cb.dblSampleTime = dblSampleTime;
        cb.lBufferSize   = lBufferSize;

        // If we haven't yet allocated the data buffer, do it now.
        // Just allocate what we need to store the new bitmap.
        if (!cb.pBuffer)
            cb.pBuffer = new BYTE[lBufferSize];

        // Copy the bitmap data into our global buffer
        if (cb.pBuffer)
            memcpy(cb.pBuffer, pBuffer, lBufferSize);

        // Post a message to our application, telling it to come back
        // and write the saved data to a bitmap file on the user's disk.
        PostMessage(g_hwnd, WM_CAPTURE_BITMAP, 0, 0L);
        return 0;
    }

    // This function will be called whenever a captured still needs to be
    // displayed in the preview window.  It is called initially within
    // CopyBitmap() to display the captured still, but it is also called
    // whenever the main dialog needs to repaint and when we transition
    // from video capture mode back into still capture mode.
    //
    BOOL DisplayCapturedBits(BYTE *pBuffer, BITMAPINFOHEADER *pbih)
    {
        // If we haven't yet snapped a still, return
        if (!bFileWritten || !pOwner || !pBuffer)
            return FALSE;

        // put bits into the preview window with StretchDIBits
        //
        HWND hwndStill = NULL;
        pOwner->GetDlgItem( IDC_STILL, &hwndStill );

        RECT rc;
        ::GetWindowRect( hwndStill, &rc );
        long lStillWidth = rc.right - rc.left;
        long lStillHeight = rc.bottom - rc.top;
        
        HDC hdcStill = GetDC( hwndStill );
        PAINTSTRUCT ps;
        BeginPaint(hwndStill, &ps);

        SetStretchBltMode(hdcStill, COLORONCOLOR);
        StretchDIBits( 
            hdcStill, 0, 0, 
            lStillWidth, lStillHeight, 
            0, 0, lWidth, lHeight, 
            pBuffer, 
            (BITMAPINFO*) pbih, 
            DIB_RGB_COLORS, 
            SRCCOPY );

        EndPaint(hwndStill, &ps);
        ReleaseDC( hwndStill, hdcStill );    

        return TRUE;
    }

    // This is the implementation function that writes the captured video
    // data onto a bitmap on the user's disk.
    //
    BOOL CopyBitmap( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
    {
        if( !g_bOneShot )
            return 0;

        // we only take one at a time
        //
        g_bOneShot = FALSE;

        // figure out where to capture to
        //
        TCHAR m_ShortName[MAX_PATH];
        wsprintf( m_szSnappedName, TEXT("%sStillCap%4.4ld.bmp"), 
                  m_szCapDir, pOwner->m_nCapTimes );
        wsprintf( m_ShortName, TEXT("StillCap%4.4ld.bmp"), 
                  pOwner->m_nCapTimes );

        // increment bitmap number if user requested it
        // otherwise, we'll reuse the filename next time
        if( pOwner->IsDlgButtonChecked( IDC_AUTOBUMP ) )
            pOwner->m_nCapTimes++;

        // write out a BMP file
        //
        HANDLE hf = CreateFile(
            m_szSnappedName, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, NULL, NULL );

        if( hf == INVALID_HANDLE_VALUE )
            return 0;

        // write out the file header
        //
        BITMAPFILEHEADER bfh;
        memset( &bfh, 0, sizeof( bfh ) );
        bfh.bfType = 'MB';
        bfh.bfSize = sizeof( bfh ) + lBufferSize + sizeof( BITMAPINFOHEADER );
        bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

        DWORD dwWritten = 0;
        WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );

        // and the bitmap format
        //
        BITMAPINFOHEADER bih;
        memset( &bih, 0, sizeof( bih ) );
        bih.biSize = sizeof( bih );
        bih.biWidth = lWidth;
        bih.biHeight = lHeight;
        bih.biPlanes = 1;
        bih.biBitCount = 24;

        dwWritten = 0;
        WriteFile( hf, &bih, sizeof( bih ), &dwWritten, NULL );

        // and the bits themselves
        //
        dwWritten = 0;
        WriteFile( hf, pBuffer, lBufferSize, &dwWritten, NULL );
        CloseHandle( hf );
        bFileWritten = TRUE;

        // Display the bitmap bits on the dialog's preview window
        DisplayCapturedBits(pBuffer, &bih);

        // Save bitmap header for later use when repainting the window
        memcpy(&(cb.bih), &bih, sizeof(bih));        

        // show where it captured
        //
        pOwner->SetDlgItemText( IDC_SNAPNAME, m_ShortName );

        // Enable the 'View Still' button
        HWND hwndButton = NULL;
        pOwner->GetDlgItem( IDC_BUTTON_VIEWSTILL, &hwndButton );
        ::EnableWindow(hwndButton, TRUE);

        // play a snap sound
        if (pOwner->IsDlgButtonChecked(IDC_PLAYSOUND))
        {
            TCHAR szSound[128];
            GetWindowsDirectory(szSound, 128);
            _tcscat(szSound, TEXT("\\media\\click.wav\0"));
            sndPlaySound(szSound, SND_ASYNC);
        }

        return 0;
    }

};

// this semi-COM object will receive sample callbacks for us
//
CSampleGrabberCB mCB;

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
// CStillCapDlg dialog

CStillCapDlg::CStillCapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStillCapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStillCapDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStillCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStillCapDlg)
	DDX_Control(pDX, IDC_STATUS, m_StrStatus);
	DDX_Control(pDX, IDC_STILL, m_StillScreen);
	DDX_Control(pDX, IDC_PREVIEW, m_PreviewScreen);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CStillCapDlg, CDialog)
	//{{AFX_MSG_MAP(CStillCapDlg)
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SNAP, OnSnap)
	ON_BN_CLICKED(IDC_CAPSTILLS, OnCapstills)
	ON_BN_CLICKED(IDC_CAPVID, OnCapvid)
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_VIEWSTILL, OnButtonViewstill)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStillCapDlg message handlers

void CStillCapDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CStillCapDlg::OnPaint() 
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

        // Update the bitmap preview window, if we have
        // already captured bitmap data
        mCB.DisplayCapturedBits(cb.pBuffer, &(cb.bih));
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStillCapDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CStillCapDlg::OnInitDialog()
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
	
    // StillCap-specific initialization
    CoInitialize( NULL );

    // default to this capture directory
    //
    SetDlgItemText( IDC_CAPDIR, TEXT("c:\\") );

    // default to capturing stills
    //
    CheckDlgButton( IDC_CAPSTILLS, 1 );
    m_bCapStills = true;
    m_nCapState = 0;
    m_nCapTimes = 0;
    g_hwnd = GetSafeHwnd();

    // start up the still image capture graph
    //
    HRESULT hr = InitStillGraph( );
    if (FAILED(hr))
        Error( TEXT("Failed to initialize StillGraph!"));

    // Modify the window style of the capture and still windows
    // to prevent excessive repainting
    m_PreviewScreen.ModifyStyle(0, WS_CLIPCHILDREN);
    m_StillScreen.ModifyStyle(0, WS_CLIPCHILDREN);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStillCapDlg::ClearGraphs( )
{
    // Destroy capture graph
    if( m_pGraph )
    {
        // have to wait for the graphs to stop first
        //
        CComQIPtr< IMediaControl, &IID_IMediaControl > pControl = m_pGraph;
        if( pControl ) 
            pControl->Stop( );

        // make the window go away before we release graph
        // or we'll leak memory/resources
        // 
        CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pGraph;
        if( pWindow )
        {
            pWindow->put_Visible( OAFALSE );
            pWindow->put_Owner( NULL );
        }

#ifdef REGISTER_FILTERGRAPH
        // Remove filter graph from the running object table   
        if (g_dwGraphRegister)
            RemoveGraphFromRot(g_dwGraphRegister);
#endif

        m_pGraph.Release( );
        m_pGrabber.Release( );
    }

    // Destroy playback graph, if it exists
    if( m_pPlayGraph )
    {
        CComQIPtr< IMediaControl, &IID_IMediaControl > pControl = m_pPlayGraph;
        if( pControl ) 
            pControl->Stop( );

        CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pPlayGraph;
        if( pWindow )
        {
            pWindow->put_Visible( OAFALSE );
            pWindow->put_Owner( NULL );
        }

        m_pPlayGraph.Release( );
    }
}

HRESULT CStillCapDlg::InitStillGraph( )
{
    HRESULT hr;

    // create a filter graph
    //
    hr = m_pGraph.CoCreateInstance( CLSID_FilterGraph );
    if( !m_pGraph )
    {
        Error( TEXT("Could not create filter graph") );
        return E_FAIL;
    }

    // get whatever capture device exists
    //
    CComPtr< IBaseFilter > pCap;
    GetDefaultCapDevice( &pCap );
    if( !pCap )
    {
        Error( TEXT("No video capture device was detected on your system.\r\n\r\n")
               TEXT("This sample requires a functional video capture device, such\r\n")
               TEXT("as a USB web camera.") );
        return E_FAIL;
    }

    // add the capture filter to the graph
    //
    hr = m_pGraph->AddFilter( pCap, L"Cap" );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not put capture device in graph"));
        return E_FAIL;
    }

    // create a sample grabber
    //
    hr = m_pGrabber.CoCreateInstance( CLSID_SampleGrabber );
    if( !m_pGrabber )
    {
        Error( TEXT("Could not create SampleGrabber (is qedit.dll registered?)"));
        return hr;
    }
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabBase( m_pGrabber );

    // force it to connect to video, 24 bit
    //
    CMediaType VideoType;
    VideoType.SetType( &MEDIATYPE_Video );
    VideoType.SetSubtype( &MEDIASUBTYPE_RGB24 );
    hr = m_pGrabber->SetMediaType( &VideoType ); // shouldn't fail
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not set media type"));
        return hr;
    }

    // add the grabber to the graph
    //
    hr = m_pGraph->AddFilter( pGrabBase, L"Grabber" );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not put sample grabber in graph"));
        return hr;
    }

    // find the two pins and connect them
    //
    IPin * pCapOut = GetOutPin( pCap, 0 );
    IPin * pGrabIn = GetInPin( pGrabBase, 0 );
    hr = m_pGraph->Connect( pCapOut, pGrabIn );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not connect capture pin #0 to grabber.\r\n")
               TEXT("Is the capture device being used by another application?"));
        return hr;
    }

    // render the sample grabber output pin, so we get a preview window
    //
    IPin * pGrabOut = GetOutPin( pGrabBase, 0 );
    hr = m_pGraph->Render( pGrabOut );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not render sample grabber output pin"));
        return hr;
    }

    // ask for the connection media type so we know how big
    // it is, so we can write out bitmaps
    //
    AM_MEDIA_TYPE mt;
    hr = m_pGrabber->GetConnectedMediaType( &mt );
    if ( FAILED( hr) )
    {
        Error( TEXT("Could not read the connected media type"));
        return hr;
    }
    
    VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mt.pbFormat;
    mCB.pOwner = this;
    mCB.lWidth  = vih->bmiHeader.biWidth;
    mCB.lHeight = vih->bmiHeader.biHeight;
    FreeMediaType( mt );

    // don't buffer the samples as they pass through
    //
    m_pGrabber->SetBufferSamples( FALSE );

    // only grab one at a time, stop stream after
    // grabbing one sample
    //
    m_pGrabber->SetOneShot( FALSE );

    // set the callback, so we can grab the one sample
    //
    m_pGrabber->SetCallback( &mCB, 1 );

    // find the video window and stuff it in our window
    //
    CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pGraph;
    if( !pWindow )
    {
        Error( TEXT("Could not get video window interface"));
        return E_FAIL;
    }

    // set up the preview window to be in our dialog
    // instead of floating popup
    //
    HWND hwndPreview = NULL;
    GetDlgItem( IDC_PREVIEW, &hwndPreview );
    RECT rc;
    ::GetWindowRect( hwndPreview, &rc );
    pWindow->put_Owner( (OAHWND) hwndPreview );
    pWindow->put_Left( 0 );
    pWindow->put_Top( 0 );
    pWindow->put_Width( rc.right - rc.left );
    pWindow->put_Height( rc.bottom - rc.top );
    pWindow->put_Visible( OATRUE );
    pWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS );
    
    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
    hr = AddGraphToRot(m_pGraph, &g_dwGraphRegister);
    if (FAILED(hr))
    {
        Error(TEXT("Failed to register filter graph with ROT!"));
        g_dwGraphRegister = 0;
    }
#endif

    // run the graph
    //
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl = m_pGraph;
    hr = pControl->Run( );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not run graph"));
        return hr;
    }

    UpdateStatus(_T("Previewing Live Video"));
    return 0;
}

HRESULT CStillCapDlg::InitCaptureGraph( TCHAR * pFilename )
{
    HRESULT hr;

    // make a filter graph
    //
    m_pGraph.CoCreateInstance( CLSID_FilterGraph );
    if( !m_pGraph )
    {
        Error(TEXT("Could not create filter graph"));
        return E_FAIL;
    }

    // get whatever capture device exists
    //
    CComPtr< IBaseFilter > pCap;
    GetDefaultCapDevice( &pCap );
    if( !pCap )
    {
        Error( TEXT("No video capture device was detected on your system.\r\n\r\n")
               TEXT("This sample requires a functional video capture device, such\r\n")
               TEXT("as a USB web camera.") );
        return E_FAIL;
    }

    // add the capture filter to the graph
    //
    hr = m_pGraph->AddFilter( pCap, L"Cap" );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not put capture device in graph"));
        return hr;
    }

    // make a capture builder graph (for connecting help)
    //
    CComPtr< ICaptureGraphBuilder2 > pBuilder;
    hr = pBuilder.CoCreateInstance( CLSID_CaptureGraphBuilder2 );
    if( !pBuilder )
    {
        Error( TEXT("Could not create capture graph builder2"));
        return hr;
    }

    hr = pBuilder->SetFiltergraph( m_pGraph );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not set filtergraph on graphbuilder2"));
        return hr;
    }

    CComPtr< IBaseFilter > pMux;
    CComPtr< IFileSinkFilter > pSink;
    USES_CONVERSION;

    hr = pBuilder->SetOutputFileName( &MEDIASUBTYPE_Avi,
        T2W( pFilename ),
        &pMux,
        &pSink );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not create/hookup mux and writer"));
        return hr;
    }

    hr = pBuilder->RenderStream( &PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video,
        pCap,
        NULL,
        pMux );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not connect capture pin"));
        return hr;
    }

    hr = pBuilder->RenderStream( &PIN_CATEGORY_PREVIEW,
        &MEDIATYPE_Video,
        pCap,
        NULL,
        NULL );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not render capture pin"));
        return hr;
    }
    if( hr == VFW_S_NOPREVIEWPIN )
    {
        // preview was faked up using the capture pin, so we can't
        // turn capture on and off at will.
        hr = 0;
    }

    // find the video window and stuff it in our window
    //
    CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pGraph;
    if( !pWindow )
    {
        Error( TEXT("Could not get video window interface"));
        return hr;
    }

    // set up the preview window to be in our dialog
    // instead of floating popup
    //
    HWND hwndPreview = NULL;
    GetDlgItem( IDC_PREVIEW, &hwndPreview );
    RECT rc;
    ::GetWindowRect( hwndPreview, &rc );
    pWindow->put_Owner( (OAHWND) hwndPreview );
    pWindow->put_Left( 0 );
    pWindow->put_Top( 0 );
    pWindow->put_Width( rc.right - rc.left );
    pWindow->put_Height( rc.bottom - rc.top );
    pWindow->put_Visible( OATRUE );
    pWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS );
    
    // run the graph
    //
    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl = m_pGraph;
    hr = pControl->Run( );
    if( FAILED( hr ) )
    {
        Error( TEXT("Could not run graph"));
        return hr;
    }

    UpdateStatus(_T("Capturing Video To Disk"));
    return 0;
}

HRESULT CStillCapDlg::InitPlaybackGraph( TCHAR * pFilename )
{
    m_pPlayGraph.CoCreateInstance( CLSID_FilterGraph );
    USES_CONVERSION;

    HRESULT hr = m_pPlayGraph->RenderFile( T2W( pFilename ), NULL );
    if (FAILED(hr))
        return hr;

    // find the video window and stuff it in our window
    //
    CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pPlayGraph;
    if( !pWindow )
    {
        Error( TEXT("Could not get video window interface"));
        return E_FAIL;
    }

    // set up the preview window to be in our dialog
    // instead of floating popup
    //
    HWND hwndPreview = NULL;
    GetDlgItem( IDC_STILL, &hwndPreview );
    RECT rc;
    ::GetWindowRect( hwndPreview, &rc );
    pWindow->put_Owner( (OAHWND) hwndPreview );
    pWindow->put_Left( 0 );
    pWindow->put_Top( 0 );
    pWindow->put_Width( rc.right - rc.left );
    pWindow->put_Height( rc.bottom - rc.top );
    pWindow->put_Visible( OATRUE );
    pWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS );

    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl;
    pControl = m_pPlayGraph;

    // Play back the recorded video
    pControl->Run( );
    UpdateStatus(_T("Playing Back Recorded Video"));
    return 0;
}

void CStillCapDlg::GetDefaultCapDevice( IBaseFilter ** ppCap )
{
    HRESULT hr;

    *ppCap = NULL;

    // create an enumerator
    //
    CComPtr< ICreateDevEnum > pCreateDevEnum;
    pCreateDevEnum.CoCreateInstance( CLSID_SystemDeviceEnum );
    if( !pCreateDevEnum )
        return;

    // enumerate video capture devices
    //
    CComPtr< IEnumMoniker > pEm;
    pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );
    if( !pEm )
        return;

    pEm->Reset( );
 
    // go through and find first video capture device
    //
    while( 1 )
    {
        ULONG ulFetched = 0;
        CComPtr< IMoniker > pM;
        hr = pEm->Next( 1, &pM, &ulFetched );
        if( hr != S_OK )
            break;

        // get the property bag interface from the moniker
        //
        CComPtr< IPropertyBag > pBag;
        hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
        if( hr != S_OK )
            continue;

        // ask for the english-readable name
        //
        CComVariant var;
        var.vt = VT_BSTR;
        hr = pBag->Read( L"FriendlyName", &var, NULL );
        if( hr != S_OK )
            continue;

        // set it in our UI
        //
        USES_CONVERSION;
        SetDlgItemText( IDC_CAPOBJ, W2T( var.bstrVal ) );

        // ask for the actual filter
        //
        hr = pM->BindToObject( 0, 0, IID_IBaseFilter, (void**) ppCap );
        if( *ppCap )
            break;
    }

    return;
}

void CStillCapDlg::OnSnap() 
{
    CString CapDir;
    GetDlgItemText( IDC_CAPDIR, CapDir );

    // Snap a still picture?
    if( m_bCapStills )
    {
        _tcscpy( mCB.m_szCapDir, CapDir );
        g_bOneShot = TRUE;
    }

    // Start capturing video
    else
    {
        if( m_nCapState == 0 )
        {
            if( IsDlgButtonChecked( IDC_AUTOBUMP ) )
                m_nCapTimes++;
        }

        // Determine AVI filename
        TCHAR szFilename[MAX_PATH], szFile[MAX_PATH];
        wsprintf( szFilename, TEXT("%sStillCap%04d.avi"), CapDir, m_nCapTimes );
        wsprintf( szFile, TEXT("StillCap%04d.avi"), m_nCapTimes );

        // start capturing, show playing button
        //
        if( m_nCapState == 0 )
        {
            ClearGraphs( );
            InitCaptureGraph( szFilename );
            SetDlgItemText( IDC_SNAP, TEXT("&Start Playback"));
            m_nCapState = 1;
        }
        else if( m_nCapState == 1 )
        {
            // show us where it captured to
            //
            SetDlgItemText( IDC_SNAPNAME, szFile );

            ClearGraphs( );
            InitPlaybackGraph( szFilename );
            SetDlgItemText( IDC_SNAP, TEXT("&Start Capture"));
            m_nCapState = 0;
        }
    }
}

BOOL CStillCapDlg::DestroyWindow() 
{
    ClearGraphs( );

    return CDialog::DestroyWindow();
}

void CStillCapDlg::Error( TCHAR * pText )
{
    GetDlgItem( IDC_SNAP )->EnableWindow( FALSE );
    ::MessageBox( NULL, pText, TEXT("Error!"), MB_OK | MB_TASKMODAL | MB_SETFOREGROUND );
}

void CStillCapDlg::OnCapstills() 
{
    if( m_bCapStills )
        return;

    SetDlgItemText( IDC_SNAP, TEXT("&Snap Still"));
    m_bCapStills = true;

    ClearGraphs( );
    InitStillGraph( );

    // Update the bitmap preview window, if we have
    // already captured bitmap data
    mCB.DisplayCapturedBits(cb.pBuffer, &(cb.bih));
}

void CStillCapDlg::OnCapvid() 
{
    if( !m_bCapStills )
        return;

    ClearGraphs( );
    m_bCapStills = false;
    m_nCapState = 0;

    // use OnSnap to set the UI state and the graphs
    //
    OnSnap( );
}

void CStillCapDlg::OnButtonReset() 
{
    // Reset bitmap counter to reset at zero
    m_nCapTimes = 0;
}

void CStillCapDlg::OnButtonViewstill() 
{
    // Open the bitmap with the system-default application
    ShellExecute(this->GetSafeHwnd(), TEXT("open\0"), mCB.m_szSnappedName, 
                 NULL, NULL, SW_SHOWNORMAL);
}

void CStillCapDlg::UpdateStatus(TCHAR *szStatus)
{
    m_StrStatus.SetWindowText(szStatus);
}

LRESULT CStillCapDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    // Field the message posted by our SampleGrabber callback function.
    if (message == WM_CAPTURE_BITMAP)
        mCB.CopyBitmap(cb.dblSampleTime, cb.pBuffer, cb.lBufferSize);        
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CStillCapDlg::OnClose() 
{
    // Free the memory allocated for our bitmap data buffer
    if (cb.pBuffer != 0)
    {
        delete cb.pBuffer;
        cb.pBuffer = 0;
    }
    	
	CDialog::OnClose();
}
