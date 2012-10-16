//------------------------------------------------------------------------------
// File: TextOut.cpp
//
// Desc: DirectShow sample code - illustrates a simple text renderer.  It
//       draws samples from a text stream into a window.  The CBaseRenderer
//       class is used to manage a filter with a single input pin, and
//       CBaseWindow looks after the construction, management and destruction
//       of a window.  The window object itself creates a separate worker
//       thread.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Summary
//
// This is a text renderer - we have authored a special AVI file (called
// CLOCKTXT.AVI) which has an additional stream containing buffers of null
// termimated text. The AVI parser reads these and sends them packaged
// up as DirectShow media samples. We get the buffer from these and cast
// them to strings ready to be passed to ExtTextOut (from DoRenderSample).
//
// (NOTE: The CLOCKTXT.AVI file is installed to the SDK's Media directory.)
//
//
// Implementation
//
// We create a simple window (the default size of 320x240) which is displayed
// each time the filter is paused. Into this window we will ExtTextOut each
// strings that are sent to us. The renderer is based on the CBaseRenderer
// SDK base class which handles all the synchronisation and seeking although
// it doesn't have any quality management implementation (to get the quality
// management we would inherit from CBaseVideoRenderer base class instead).
//
// From the base renderer class we override the virtual OnReceiveFirstSample
// method so that when we are paused we will show a poster frame. The filter
// also supports IVideoWindow, which is done fairly simply by inheriting a
// class from CBaseControlWindow.  This base class implements most of that
// interface, although we also override OnClose, OnReceiveMessage (which is
// a generic message handler) and GetClassWindowStyles. GetClassWindowStyles
// is called by the base window class (CBaseWindow) during window creation
// when it wants to know what window and class styles it should register.
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. 
// Drag and drop the CLOCKTXT.AVI movie into the tool and it will be rendered.
// Notice that the AVI splitter filter will display three output pins:
//      - A video output pin
//      - An audio waveform output pin
//      - A text output pin ("Lyrics)
//
// GraphEdit may automatically render the "Lyrics" pin to another filter,
// such as the "Internal Script Command Renderer".  If so, just select the
// filter that connects the Lyrics pin and press the delete key to remove 
// the connection.  Next, choose Graph->Insert Filters, expand the 
// DirectShow Filters node, select Text Display, and close the dialog box.
// Connect the Lyrics pin to the input of this Text Display filter using
// the mouse or by right-clicking on the Lyrics pin and selecting Render.
//
//
// Click on Pause and then Run on the GraphEdit frame and you will see the video 
// in one window on the desktop and another video with text being drawn into it.
// The words being displayed are "One", "Two". "Three", etc., and the point at
// which they are displayed should match with the images in the other window.
//
//
// Files
//
// textout.cpp          The filter and window class implementations
// textout.def          What we import and export from this DLL
// textout.h            Window and filter class definitions
// textout.rc           Version and title resource information
// makefile             How we build it...
//
//
// Base classes we use
//
// CBaseWindow          Manages a window on a separate worker thread
// CBaseRenderer        The simple renderer filter base class
//
//

#include <streams.h>
#include <initguid.h>
#include "textout.h"


// Setup data

const AMOVIESETUP_MEDIATYPE sudIpPinTypes =
{
    &MEDIATYPE_Text,              // MajorType
    &MEDIASUBTYPE_NULL            // MinorType
};

const AMOVIESETUP_PIN sudIpPin =
{
    L"Input",                     // The Pins name
    FALSE,                        // Is rendered
    FALSE,                        // Is an output pin
    FALSE,                        // Allowed none
    FALSE,                        // Allowed many
    &CLSID_NULL,                  // Connects to filter
    NULL,                         // Connects to pin
    1,                            // Number of types
    &sudIpPinTypes                // Pin details
};

const AMOVIESETUP_FILTER sudTextoutAx =
{
    &CLSID_TextRender,            // Filter CLSID
    L"Text Display",              // String name
    MERIT_NORMAL,                 // Filter merit
    1,                            // Number of pins
    &sudIpPin                     // Pin details
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance
// function when it is asked to create a CLSID_VideoRenderer object

CFactoryTemplate g_Templates[] = {
    { L"Text Display"
    , &CLSID_TextRender
    , CTextOutFilter::CreateInstance
    , NULL
    , &sudTextoutAx },
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// DllRegisterServer
//
// Used to register and unregister the filter
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


//
// Constructor
//
// Constructor for the text out renderer filter. After initialising the base
// renderer class and our nested window handling class we have to pass our
// input pin we have to the window class. The base class uses this to check
// that the filter has a valid pin connection before allowing IVideoWindow
// methods to be called (this is a stipulation of the interface set mainly
// because most filters can't do anything before they know what data they
// will be dealing with - an example being video renderers who can't really
// support IVideoWindow fully until they know the size/format of the video)
//
#pragma warning(disable:4355)
//
CTextOutFilter::CTextOutFilter(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseRenderer(CLSID_TextRender, NAME("Text Display Filter"), pUnk, phr),
    m_TextWindow(NAME("Text properties"),GetOwner(),phr,&m_InterfaceLock,this)
{
    m_TextWindow.SetControlWindowPin( GetPin(0) );

} // (Constructor)


//
// Destructor
//
CTextOutFilter::~CTextOutFilter()
{
}


//
// CreateInstance
//
// This goes in the factory template table to create new instances
//
CUnknown * WINAPI CTextOutFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CTextOutFilter *pTextOutFilter = new CTextOutFilter(pUnk,phr);
    if (pTextOutFilter == NULL) {
        return NULL;
    }
    return (CBaseMediaFilter *) pTextOutFilter;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Overriden to say what interfaces we support and where
//
STDMETHODIMP
CTextOutFilter::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    if (riid == IID_IVideoWindow) {
        return m_TextWindow.NonDelegatingQueryInterface(riid,ppv);
    }
    return CBaseRenderer::NonDelegatingQueryInterface(riid,ppv);

} // NonDelegatingQueryInterface


//
// Pause
//
// Overriden to show the text renderer window
//
STDMETHODIMP CTextOutFilter::Pause()
{
    BOOL fStopToPause = (m_State == State_Stopped);

    HRESULT hr = CBaseRenderer::Pause();
    if(FAILED(hr)) {
        return hr;
    }

    if(fStopToPause)
    {
        m_TextWindow.ActivateWindow();
        m_TextWindow.DoShowWindow(SW_SHOWNORMAL);
    }
    return hr;

} // Pause


//
// BreakConnect
//
// Deactivate the text out rendering window
//
HRESULT CTextOutFilter::BreakConnect()
{
    m_TextWindow.InactivateWindow();
    m_TextWindow.DoShowWindow(SW_HIDE);
    return NOERROR;

} // BreakConnect


//
// CheckMediaType
//
// Check that we can support a given proposed type
//
HRESULT CTextOutFilter::CheckMediaType(const CMediaType *pmt)
{
    // Reject non-Text type

    if (pmt->majortype != MEDIATYPE_Text) {
    	return E_INVALIDARG;
    }

    return NOERROR;

} // CheckMediaType


//
// OnPaint
//
// This is called when the window thread receives a WM_PAINT message
//
BOOL CTextOutFilter::OnPaint(COLORREF WindowColor)
{
    CAutoLock cAutoLock(&m_RendererLock);
    RECT ClientRect;
    PAINTSTRUCT ps;

    BeginPaint(m_TextWindow.GetWindowHWND(),&ps);
    EndPaint(m_TextWindow.GetWindowHWND(),&ps);

    // Display the text if we have a sample

    if (m_pMediaSample) {
        DrawText(m_pMediaSample);
        return TRUE;
    }

    // Create a coloured brush to paint the window

    HBRUSH hBrush = CreateSolidBrush(WindowColor);
    EXECUTE_ASSERT(GetClientRect(m_TextWindow.GetWindowHWND(),&ClientRect));
    EXECUTE_ASSERT(FillRect(m_TextWindow.GetWindowHDC(),&ClientRect,hBrush));
    EXECUTE_ASSERT(DeleteObject(hBrush));
    return TRUE;

} // OnPaint


//
// DoRenderSample
//
// This is called when a sample is ready for rendering
//
HRESULT CTextOutFilter::DoRenderSample(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);
    DrawText(pMediaSample);
    return NOERROR;

} // DoRenderSample


//
// OnReceiveFirstSample
//
// Display an image if not streaming
//
void CTextOutFilter::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    if(IsStreaming() == FALSE)
    {
        ASSERT(pMediaSample);
        DrawText(pMediaSample);
    }

} // OnReceiveFirstSample


//
// DrawText
//
// This is called with an IMediaSample interface on the image to be drawn. We
// are called from two separate code paths. The first is when we're signalled
// that an image has become due for rendering, the second is when we need to
// refresh a static window image. NOTE: it is safe to check the type of buffer
// allocator as to change it we must be inactive, which by definition means
// we cannot have any samples available to render so we cannot be here
//
void CTextOutFilter::DrawText(IMediaSample *pMediaSample)
{
    BYTE *pText;        // Pointer to image data
    RECT rcClip;        // window rectangle

    SetRect(&rcClip, (LONG) 0, (LONG) 0,
            m_TextWindow.GetWindowWidth(),
            m_TextWindow.GetWindowHeight());

    pMediaSample->GetPointer(&pText);
    ASSERT(pText != NULL);

    #ifdef UNICODE
        USES_CONVERSION;

        // Convert BYTE string to wide character string
        WCHAR strNumber[16];
        wcscpy(strNumber, A2T((char *) pText));
    #else
        char strNumber[16];
        strcpy(strNumber, (char *) pText);
    #endif

    // Ignore zero length samples

    if (pMediaSample->GetActualDataLength() == 0) {
        return;
    }

    // Remove trailing NULL from the text data

    ExtTextOut(m_TextWindow.GetWindowHDC(),
	       0, 0,
	       ETO_OPAQUE | ETO_CLIPPED,
	       &rcClip,
	       strNumber,
	       pMediaSample->GetActualDataLength() - 1,
	       NULL);

    GdiFlush();

} // DrawText


//
// Constructor
//
// Derived class handling window interactions. We did have the main renderer
// object inheriting from CBaseControlWindow so that we didn't have to have
// a separate class but that means there are two many classes derived from
// CUnknown, so when in the final text out filter class you call something
// like GetOwner it gets really confusing to know who is actually going to
// be called. So in the end we made it a separate class for the window. We
// have to specialise the base class to provide the PURE virtual method that
// returns the class and window information (GetClassWindowStyles). We are
// also interested in certain window messages like WM_PAINT and WM_NCHITTEST
//
CTextOutWindow::CTextOutWindow(TCHAR *pName,                // Object string
                               LPUNKNOWN pUnk,              // COM ownership
                               HRESULT *phr,                // OLE code
                               CCritSec *pLock,             // Interface lock
                               CTextOutFilter *pRenderer) : // Main filter

    CBaseControlWindow(pRenderer,pLock,pName,pUnk,phr),
    m_pRenderer(pRenderer)
{
    PrepareWindow();

} // Constructor


//
// Destructor
//
CTextOutWindow::~CTextOutWindow()
{
    DoneWithWindow();

} // Destructor


//
// OnReceiveMessage
//
// This is a virtual method that does our derived class message handling
// We should process the messages we are interested in and then call the
// base class as appropriate - some messages we may not pass forward
//
LRESULT CTextOutWindow::OnReceiveMessage(HWND hwnd,         // Window handle
                                         UINT uMsg,         // Message ID
                                         WPARAM wParam,     // First parameter
                                         LPARAM lParam)     // Other parameter
{
    switch (uMsg) {

        // This tells us some of the window's client area has become exposed
        // If our connected filter is doing overlay work then we repaint the
        // background so that it will pick up the window clipping changes

        case WM_PAINT:
            m_pRenderer->OnPaint(RGB(0,0,0));
            return (LRESULT) 1;

    }

    return CBaseControlWindow::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


//
// OnClose
//
// Overriding the WM_CLOSE handling to also signal EC_USERABORT
//
BOOL CTextOutWindow::OnClose()
{
    CBaseControlWindow::OnClose();
    m_pRenderer->NotifyEvent(EC_USERABORT,0,0);
    return TRUE;

} // OnClose


//
// GetClassWindowStyles
//
// When we call PrepareWindow in our constructor it will call this method as
// it is going to create the window to get our window and class styles. The
// return code is the class name and must be allocated in static storage. We
// specify a normal window during creation although the window styles as well
// as the extended styles may be changed by the application via IVideoWindow
//
LPTSTR CTextOutWindow::GetClassWindowStyles(DWORD *pClassStyles,
                                            DWORD *pWindowStyles,
                                            DWORD *pWindowStylesEx)
{
    *pClassStyles = TextClassStyles;
    *pWindowStyles = TextWindowStyles;
    *pWindowStylesEx = (DWORD) 0;
    return TextClassName;

} // GetClassWindowStyles

