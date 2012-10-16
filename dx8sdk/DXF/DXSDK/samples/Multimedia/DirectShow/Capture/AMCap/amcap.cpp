//------------------------------------------------------------------------------
// File: AMCap.cpp
//
// Desc: Audio/Video Capture sample for DirectShow
//
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <dbt.h>
#include <streams.h>
#include <mmreg.h>
#include <msacm.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <commdlg.h>
#include <atlbase.h>
#include "stdafx.h"
#include "amcap.h"
#include "status.h"
#include "crossbar.h"

// you can never have too many parentheses!
#define ABS(x) (((x) > 0) ? (x) : -(x))

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#ifdef  DEBUG
#define REGISTER_FILTERGRAPH
#endif


//------------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------------
HINSTANCE ghInstApp;
HACCEL ghAccel;
HFONT  ghfontApp;
TEXTMETRIC gtm;
TCHAR gszAppName[]=TEXT("AMCAP");
HWND ghwndApp, ghwndStatus;
HDEVNOTIFY ghDevNotify;
PUnregisterDeviceNotification gpUnregisterDeviceNotification;
PRegisterDeviceNotification gpRegisterDeviceNotification;
DWORD g_dwGraphRegister=0;

struct _capstuff {
    TCHAR szCaptureFile[_MAX_PATH];
    WORD wCapFileSize;  // size in Meg
    ICaptureGraphBuilder2 *pBuilder;
    IVideoWindow *pVW;
    IMediaEventEx *pME;
    IAMDroppedFrames *pDF;
    IAMVideoCompression *pVC;
    IAMVfwCaptureDialogs *pDlg;
    IAMStreamConfig *pASC;      // for audio cap
    IAMStreamConfig *pVSC;      // for video cap
    IBaseFilter *pRender;
    IBaseFilter *pVCap, *pACap;
    IGraphBuilder *pFg;
    IFileSinkFilter *pSink;
    IConfigAviMux *pConfigAviMux;
    int  iMasterStream;
    BOOL fCaptureGraphBuilt;
    BOOL fPreviewGraphBuilt;
    BOOL fCapturing;
    BOOL fPreviewing;
    BOOL fCapAudio;
    BOOL fCapCC;
    BOOL fCCAvail;
    BOOL fCapAudioIsRelevant;
    bool fDeviceMenuPopulated;
    IMoniker *rgpmVideoMenu[10];
    IMoniker *rgpmAudioMenu[10];
    IMoniker *pmVideo;
    IMoniker *pmAudio;
    double FrameRate;
    BOOL fWantPreview;
    long lCapStartTime;
    long lCapStopTime;
    WCHAR wachFriendlyName[120];
    BOOL fUseTimeLimit;
    BOOL fUseFrameRate;
    DWORD dwTimeLimit;
    int iFormatDialogPos;
    int iSourceDialogPos;
    int iDisplayDialogPos;
    int iVCapDialogPos;
    int iVCrossbarDialogPos;
    int iTVTunerDialogPos;
    int iACapDialogPos;
    int iACrossbarDialogPos;
    int iTVAudioDialogPos;
    int iVCapCapturePinDialogPos;
    int iVCapPreviewPinDialogPos;
    int iACapCapturePinDialogPos;
    long lDroppedBase;
    long lNotBase;
    BOOL fPreviewFaked;
    CCrossbar *pCrossbar;
    int iVideoInputMenuPos;
    LONG NumberOfVideoInputs;
    HMENU hMenuPopup;
    int iNumVCapDevices;
} gcap;


// implements IAMCopyCaptureFileProgress
//
class CProgress : public CUnknown, public IAMCopyCaptureFileProgress {
    public:

    CProgress(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    CUnknown(pName, pUnk, phr) {
    };
    ~CProgress() {
    };

    STDMETHODIMP_(ULONG) AddRef() {
        return 1;
    };
    STDMETHODIMP_(ULONG) Release() {
        return 0;
    };

    STDMETHODIMP QueryInterface(REFIID iid, void **p) {
        CheckPointer(p, E_POINTER);
        if(iid == IID_IAMCopyCaptureFileProgress) {
            return GetInterface((IAMCopyCaptureFileProgress *)this, p);
        }
        else {
            return E_NOINTERFACE;
        }
    };
    STDMETHODIMP Progress(int i) {
        TCHAR tach[80];
        wsprintf(tach, TEXT("Save File Progress: %d%%"), i);
        statusUpdateStatus(ghwndStatus, tach);
        return S_OK;
    };
};

CComModule _Module;

//------------------------------------------------------------------------------
// Funciton Prototypes
//------------------------------------------------------------------------------
typedef LONG(PASCAL *LPWNDPROC)(HWND, UINT, WPARAM, LPARAM); // pointer to a window procedure
LONG WINAPI AppWndProc(HWND hwnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);
LONG PASCAL AppCommand(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int ErrMsg(LPTSTR sz,...);
BOOL SetCaptureFile(HWND hWnd);
BOOL SaveCaptureFile(HWND hWnd);
BOOL AllocCaptureFile(HWND hWnd);
int DoDialog(HWND hwndParent, int DialogID, DLGPROC fnDialog, long lParam);
int FAR PASCAL AllocCapFileProc(HWND hDlg, UINT Message, UINT wParam, LONG lParam);
int FAR PASCAL FrameRateProc(HWND hDlg, UINT Message, UINT wParam, LONG lParam);
int FAR PASCAL TimeLimitProc(HWND hDlg, UINT Message, UINT wParam, LONG lParam);
int FAR PASCAL PressAKeyProc(HWND hDlg, UINT Message, UINT wParam, LONG lParam);
void TearDownGraph(void);
BOOL BuildCaptureGraph();
BOOL BuildPreviewGraph();
void UpdateStatus(BOOL fAllStats);
void AddDevicesToMenu();
void ChooseDevices(TCHAR *szVideo, TCHAR *szAudio);
void ChooseDevices(IMoniker *pmVideo, IMoniker *pmAudio);
void ChooseFrameRate();
BOOL InitCapFilters();
void FreeCapFilters();
BOOL StopPreview();
BOOL StartPreview();
BOOL StopCapture();
DWORDLONG GetSize(LPCTSTR tach);
void MakeMenuOptions();
void OnClose();

// Adds/removes a DirectShow filter graph from the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph if enabled.
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void RemoveGraphFromRot(DWORD pdwRegister);


//------------------------------------------------------------------------------
// Name: SetAppCaption()
// Desc: Set the caption to be the application name followed by the capture file
//------------------------------------------------------------------------------
void SetAppCaption() 
{
    TCHAR tach[_MAX_PATH + 80];
    lstrcpy(tach, gszAppName);
    if(gcap.szCaptureFile[0] != 0) {
        lstrcat(tach, TEXT(" - "));
        lstrcat(tach, gcap.szCaptureFile);
    }
    SetWindowText(ghwndApp, tach);
}


/*----------------------------------------------------------------------------*\
|   AppInit( hInst, hPrev)                                                     |
|                                                                              |
|   Description:                                                               |
|       This is called when the application is first loaded into               |
|       memory.  It performs all initialization that doesn't need to be done   |
|       once per instance.                                                     |
|                                                                              |
|   Arguments:                                                                 |
|       hInstance       instance handle of current instance                    |
|       hPrev           instance handle of previous instance                   |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if successful, FALSE if not                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL AppInit(HINSTANCE hInst, HINSTANCE hPrev, int sw) 
{
    WNDCLASS    cls;
    HDC         hdc;

    const DWORD  dwExStyle = 0;

    CoInitialize(NULL);
    DbgInitialise(hInst);

    /* Save instance handle for DialogBoxs */
    ghInstApp = hInst;

    ghAccel = LoadAccelerators(hInst, MAKEINTATOM(ID_APP));

    if(!hPrev) {
        /*
        *  Register a class for the main application window
        */
        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = LoadIcon(hInst, TEXT("AMCapIcon"));
        cls.lpszMenuName   = MAKEINTATOM(ID_APP);
        cls.lpszClassName  = MAKEINTATOM(ID_APP);
        cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance      = hInst;
        cls.style          = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        cls.lpfnWndProc    = (WNDPROC) AppWndProc;
        cls.cbWndExtra     = 0;
        cls.cbClsExtra     = 0;

        if(!RegisterClass(&cls))
            return FALSE;
    }

    // Is this necessary?
    ghfontApp = (HFONT)GetStockObject(ANSI_VAR_FONT);
    hdc = GetDC(NULL);
    SelectObject(hdc, ghfontApp);
    GetTextMetrics(hdc, &gtm);
    ReleaseDC(NULL, hdc);

    ghwndApp=CreateWindowEx(dwExStyle,
        MAKEINTATOM(ID_APP),    // Class name
        gszAppName,             // Caption
        // Style bits
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0,       // Position
        320,300,                // Size
        (HWND)NULL,             // Parent window (no parent)
        (HMENU)NULL,            // use class menu
        hInst,                  // handle to window instance
        (LPSTR)NULL             // no params to pass on
        );

    // create the status bar
    statusInit(hInst, hPrev);
    ghwndStatus = CreateWindowEx(0,
        szStatusClass,
        NULL,
        WS_CHILD|WS_BORDER|WS_VISIBLE|WS_CLIPSIBLINGS,
        0, 0,
        0, 0,
        ghwndApp,
        NULL,
        hInst,
        NULL);
    if(ghwndStatus == NULL) {
        return(FALSE);
    }
    ShowWindow(ghwndApp,sw);

    // get the capture file name from win.ini
    GetProfileString(TEXT("annie"), TEXT("CaptureFile"), TEXT(""),
        gcap.szCaptureFile,
        sizeof(gcap.szCaptureFile));

    // get which devices to use from win.ini
    ZeroMemory(gcap.rgpmAudioMenu, sizeof(gcap.rgpmAudioMenu));
    ZeroMemory(gcap.rgpmVideoMenu, sizeof(gcap.rgpmVideoMenu));
    gcap.pmVideo = 0;
    gcap.pmAudio = 0;

    TCHAR szVideoDisplayName[1024], szAudioDisplayName[1024];
    *szAudioDisplayName = *szVideoDisplayName = 0; // null terminate

    GetProfileString(TEXT("annie"), TEXT("VideoDevice2"), TEXT(""),
        szVideoDisplayName,
        sizeof(szVideoDisplayName)/sizeof(szVideoDisplayName[0]));
    GetProfileString(TEXT("annie"), TEXT("AudioDevice2"), TEXT(""),
        szAudioDisplayName,
        sizeof(szAudioDisplayName)/sizeof(szAudioDisplayName[0]));

    gcap.fDeviceMenuPopulated = false;
    AddDevicesToMenu();

    // do we want audio?
    gcap.fCapAudio = GetProfileInt(TEXT("annie"), TEXT("CaptureAudio"), TRUE);
    gcap.fCapCC = GetProfileInt(TEXT("annie"), TEXT("CaptureCC"), FALSE);

    // do we want preview?
    gcap.fWantPreview = GetProfileInt(TEXT("annie"), TEXT("WantPreview"), FALSE);

    // which stream should be the master? NONE(-1) means nothing special happens
    // AUDIO(1) means the video frame rate is changed before written out to keep
    // the movie in sync when the audio and video capture crystals are not the
    // same (and therefore drift out of sync after a few minutes).  VIDEO(0)
    // means the audio sample rate is changed before written out
    gcap.iMasterStream = GetProfileInt(TEXT("annie"), TEXT("MasterStream"), 1);

    // get the frame rate from win.ini before making the graph
    gcap.fUseFrameRate = GetProfileInt(TEXT("annie"), TEXT("UseFrameRate"), 1);
    int units_per_frame = GetProfileInt(TEXT("annie"), TEXT("FrameRate"), 666667);  // 15fps
    gcap.FrameRate = 10000000. / units_per_frame;
    gcap.FrameRate = (int)(gcap.FrameRate * 100) / 100.;

    // reasonable default
    if(gcap.FrameRate <= 0.)
        gcap.FrameRate = 15.0;

    gcap.fUseTimeLimit = GetProfileInt(TEXT("annie"), TEXT("UseTimeLimit"), 0);
    gcap.dwTimeLimit = GetProfileInt(TEXT("annie"), TEXT("TimeLimit"), 0);

    // instantiate the capture filters we need to do the menu items
    // this will start previewing, if wanted
    // 
    // make these the official devices we're using

    ChooseDevices(szVideoDisplayName, szAudioDisplayName);
    // and builds a partial filtergraph.

    // Register for device add/remove notifications.
    DEV_BROADCAST_DEVICEINTERFACE filterData;
    ZeroMemory(&filterData, sizeof(DEV_BROADCAST_DEVICEINTERFACE));

    filterData.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    filterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filterData.dbcc_classguid = AM_KSCATEGORY_CAPTURE;

    gpUnregisterDeviceNotification = NULL;
    gpRegisterDeviceNotification = NULL;
    // dynload device removal APIs
    {
        HMODULE hmodUser = GetModuleHandle(TEXT("user32.dll"));
        ASSERT(hmodUser);       // we link to user32
        gpUnregisterDeviceNotification = (PUnregisterDeviceNotification)
        GetProcAddress(hmodUser, "UnregisterDeviceNotification");

        // m_pRegisterDeviceNotification is prototyped differently in unicode
        gpRegisterDeviceNotification = (PRegisterDeviceNotification)
            GetProcAddress(hmodUser,
#ifdef UNICODE
            "RegisterDeviceNotificationW"
#else
            "RegisterDeviceNotificationA"
#endif
            );
        // failures expected on older platforms.
        ASSERT(gpRegisterDeviceNotification && gpUnregisterDeviceNotification ||
            !gpRegisterDeviceNotification && !gpUnregisterDeviceNotification);
    }

    ghDevNotify = NULL;

    if(gpRegisterDeviceNotification) {
        ghDevNotify = gpRegisterDeviceNotification(ghwndApp, &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);
        ASSERT(ghDevNotify != NULL);
    }

    SetAppCaption();
    return TRUE;
}

void IMonRelease(IMoniker *&pm) 
{
    if(pm) {
        pm->Release();
        pm = 0;
    }
}

/*----------------------------------------------------------------------------*\
|   WinMain( hInst, hPrev, lpszCmdLine, cmdShow )                              |
|                                                                              |
|   Description:                                                               |
|       The main procedure for the App.  After initializing, it just goes      |
|       into a message-processing loop until it gets a WM_QUIT message         |
|       (meaning the app was closed).                                          |
|                                                                              |
|   Arguments:                                                                 |
|       hInst           instance handle of this instance of the app            |
|       hPrev           instance handle of previous instance, NULL if first    |
|       szCmdLine       ->null-terminated command line                         |
|       cmdShow         specifies how the window is initially displayed        |
|                                                                              |
|   Returns:                                                                   |
|       The exit code as specified in the WM_QUIT message.                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw) 
{
    MSG     msg;

    /* Call initialization procedure */
    if(!AppInit(hInst,hPrev,sw))
        return FALSE;

    /*
    * Polling messages from event queue
    */
    for(;;) {
        while(PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
            if(msg.message == WM_QUIT)
                return ((int) msg.wParam);

            if(TranslateAccelerator(ghwndApp, ghAccel, &msg))
                continue;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        WaitMessage();
    }

    // not reached
    return ((int) msg.wParam);
}


/*----------------------------------------------------------------------------*\
|   AppWndProc( hwnd, uiMessage, wParam, lParam )                              |
|                                                                              |
|   Description:                                                               |
|       The window proc for the app's main (tiled) window.  This processes all |
|       of the parent window's messages.                                       |
|                                                                              |
|   Arguments:                                                                 |
|       hwnd            window handle for the window                           |
|       msg             message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
LONG WINAPI  AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    int cxBorder, cyBorder, cy;

    switch(msg) {
        case WM_CREATE:
            break;

        case WM_COMMAND:
            return AppCommand(hwnd,msg,wParam,lParam);

        case WM_INITMENU:
            // we can start capture if not capturing already
            EnableMenuItem((HMENU)wParam, MENU_START_CAP, 
                (!gcap.fCapturing) ? MF_ENABLED :
            MF_GRAYED);
            // we can stop capture if it's currently capturing
            EnableMenuItem((HMENU)wParam, MENU_STOP_CAP, 
                (gcap.fCapturing) ? MF_ENABLED : MF_GRAYED);

            // We can bring up a dialog if the graph is stopped
            EnableMenuItem((HMENU)wParam, MENU_DIALOG0, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG1, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG2, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG3, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG4, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG5, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG6, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG7, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG8, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOG9, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOGA, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOGB, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOGC, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOGD, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOGE, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_DIALOGF, !gcap.fCapturing ?
                MF_ENABLED : MF_GRAYED);

            // toggles capturing audio or not - can't be capturing right now
            // and we must have an audio capture device created
            EnableMenuItem((HMENU)wParam, MENU_CAP_AUDIO, 
                (!gcap.fCapturing && gcap.pACap) ? MF_ENABLED : MF_GRAYED);
            // are we capturing audio?
            CheckMenuItem((HMENU)wParam, MENU_CAP_AUDIO, 
                (gcap.fCapAudio) ? MF_CHECKED : MF_UNCHECKED);
            // are we doing closed captioning?
            CheckMenuItem((HMENU)wParam, MENU_CAP_CC, 
                (gcap.fCapCC) ? MF_CHECKED : MF_UNCHECKED);
            EnableMenuItem((HMENU)wParam, MENU_CAP_CC, 
                (gcap.fCCAvail) ? MF_ENABLED : MF_GRAYED);
            // change audio formats when not capturing
            EnableMenuItem((HMENU)wParam, MENU_AUDIOFORMAT, (gcap.fCapAudio &&
                !gcap.fCapturing) ? MF_ENABLED : MF_GRAYED);
            // change frame rate when not capturing, and only if the video
            // filter captures a VIDEOINFO type format
            EnableMenuItem((HMENU)wParam, MENU_FRAMERATE,
                (!gcap.fCapturing && gcap.fCapAudioIsRelevant) ?
                MF_ENABLED : MF_GRAYED);
            // change time limit when not capturing
            EnableMenuItem((HMENU)wParam, MENU_TIMELIMIT,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            // change capture file name when not capturing
            EnableMenuItem((HMENU)wParam, MENU_SET_CAP_FILE,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            // pre-allocate capture file when not capturing
            EnableMenuItem((HMENU)wParam, MENU_ALLOC_CAP_FILE,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            // can save capture file when not capturing
            EnableMenuItem((HMENU)wParam, MENU_SAVE_CAP_FILE,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            // do we want preview?
            CheckMenuItem((HMENU)wParam, MENU_PREVIEW, 
                (gcap.fWantPreview) ? MF_CHECKED : MF_UNCHECKED);
            // can toggle preview if not capturing
            EnableMenuItem((HMENU)wParam, MENU_PREVIEW,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);

            // which is the master stream? Not applicable unless we're also
            // capturing audio
            EnableMenuItem((HMENU)wParam, MENU_NOMASTER,
                (!gcap.fCapturing && gcap.fCapAudio) ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem((HMENU)wParam, MENU_NOMASTER, 
                (gcap.iMasterStream == -1) ? MF_CHECKED : MF_UNCHECKED);
            EnableMenuItem((HMENU)wParam, MENU_AUDIOMASTER,
                (!gcap.fCapturing && gcap.fCapAudio) ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem((HMENU)wParam, MENU_AUDIOMASTER, 
                (gcap.iMasterStream == 1) ? MF_CHECKED : MF_UNCHECKED);
            EnableMenuItem((HMENU)wParam, MENU_VIDEOMASTER,
                (!gcap.fCapturing && gcap.fCapAudio) ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem((HMENU)wParam, MENU_VIDEOMASTER, 
                (gcap.iMasterStream == 0) ? MF_CHECKED : MF_UNCHECKED);

            // can't select a new capture device when capturing
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE0,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE1,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE2,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE3,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE4,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE5,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE6,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE7,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE8,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_VDEVICE9,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE0,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE1,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE2,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE3,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE4,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE5,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE6,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE7,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE8,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, MENU_ADEVICE9,
                !gcap.fCapturing ? MF_ENABLED : MF_GRAYED);

            break;


        case WM_INITMENUPOPUP:
            if(GetSubMenu(GetMenu(ghwndApp), 1) == (HMENU)wParam) {
                AddDevicesToMenu();
            }

            break;

        //
        // We're out of here!
        //
        case WM_DESTROY:
            DbgTerminate();

            IMonRelease(gcap.pmVideo);
            IMonRelease(gcap.pmAudio); {
                    for(int i = 0; i < NUMELMS(gcap.rgpmVideoMenu); i++) {
                        IMonRelease(gcap.rgpmVideoMenu[i]);
                    }
                    for(i = 0; i < NUMELMS(gcap.rgpmAudioMenu); i++) {
                        IMonRelease(gcap.rgpmAudioMenu[i]);
                    }
            }

            CoUninitialize();
            PostQuitMessage(0);
            break;


        case WM_CLOSE:
            OnClose();
            break;

        case WM_ENDSESSION:
            if(wParam || (lParam & ENDSESSION_LOGOFF)) {
                OnClose();
            }
            break;

        case WM_ERASEBKGND:
            break;

        // ESC will stop capture
        case WM_KEYDOWN:
            if((GetAsyncKeyState(VK_ESCAPE) & 0x01) && gcap.fCapturing) {
                StopCapture();
                if(gcap.fWantPreview) {
                    BuildPreviewGraph();
                    StartPreview();
                }
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd,&ps);

            // nothing to do
            EndPaint(hwnd,&ps);
            break;

        case WM_TIMER:
            // update our status bar with #captured, #dropped
            // if we've stopped capturing, don't do it anymore.  Some WM_TIMER
            // messages may come late, after we've destroyed the graph and
            // we'll get invalid numbers.
            if(gcap.fCapturing)
                UpdateStatus(FALSE);

            // is our time limit up?
            if(gcap.fUseTimeLimit) {
                    if((timeGetTime() - gcap.lCapStartTime) / 1000 >=
                        gcap.dwTimeLimit) {
                        StopCapture();
                        if(gcap.fWantPreview) {
                            BuildPreviewGraph();
                            StartPreview();
                        }
                    }
            }
            break;

        case WM_SIZE:
            // make the preview window fit inside our window, taking up
            // all of our client area except for the status window at the
            // bottom
            GetClientRect(ghwndApp, &rc);
            cxBorder = GetSystemMetrics(SM_CXBORDER);
            cyBorder = GetSystemMetrics(SM_CYBORDER);
            cy = statusGetHeight() + cyBorder;
            MoveWindow(ghwndStatus, -cxBorder, rc.bottom - cy,
                rc.right + (2 * cxBorder), cy + cyBorder, TRUE);
            rc.bottom -= cy;
            // this is the video renderer window showing the preview
            if(gcap.pVW)
                gcap.pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
            break;

        case WM_FGNOTIFY:
            // uh-oh, something went wrong while capturing - the filtergraph
            // will send us events like EC_COMPLETE, EC_USERABORT and the one
            // we care about, EC_ERRORABORT.
            if(gcap.pME) {
                    LONG event, l1, l2;
                    BOOL bAbort = FALSE;
                    while(gcap.pME->GetEvent(&event, (LONG_PTR *) &l1, 
                        (LONG_PTR *) &l2, 0) == S_OK) {
                        gcap.pME->FreeEventParams(event, l1, l2);
                        if(event == EC_ERRORABORT) {
                            StopCapture();
                            bAbort = TRUE;
                            continue;
                        }
                        else if(event == EC_DEVICE_LOST) {
                                // Check if we have lost a capture filter being used.
                                // lParam2 of EC_DEVICE_LOST event == 1 indicates device added
                                //                                 == 0 indicates device removed
                                if(l2 == 0) {
                                    IBaseFilter *pf;
                                    IUnknown *punk = (IUnknown *) (LONG_PTR) l1;
                                    if(S_OK == punk->QueryInterface(IID_IBaseFilter, (void **) &pf)) {
                                        if(::IsEqualObject(gcap.pVCap, pf)) {
                                            pf->Release();
                                            bAbort = FALSE;
                                            StopCapture();
                                            TCHAR szError[100];
                                            wsprintf(szError, 
                                                TEXT("Stopping Capture (Device Lost). Select New Capture Device"), l1);
                                            ErrMsg(szError);
                                            break;
                                        }
                                        pf->Release();
                                    }
                                }
                        }
                    } // end while
                    if(bAbort) {
                            if(gcap.fWantPreview) {
                                BuildPreviewGraph();
                                StartPreview();
                            }
                            TCHAR szError[100];
                            wsprintf(szError, TEXT("ERROR during capture, error code=%08x"), l1);
                            ErrMsg(szError);
                    }
            }
            break;

        case WM_DEVICECHANGE:
            // We are interested in only device arrival & removal events
            if(DBT_DEVICEARRIVAL != wParam && DBT_DEVICEREMOVECOMPLETE != wParam)
                break;
            PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR) lParam;
            if(pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE) {
                break;
            }
            PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
            // Check for capture devices.
            if(pdbi->dbcc_classguid != AM_KSCATEGORY_CAPTURE) {
                break;
            }

            // Check for device arrival/removal.
            if(DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) {
                gcap.fDeviceMenuPopulated = false;
            }
            break;

    }

    return (LONG) DefWindowProc(hwnd,msg,wParam,lParam);
}


// Make a graph builder object we can use for capture graph building
//
BOOL MakeBuilder() 
{
    // we have one already
    if(gcap.pBuilder)
        return TRUE;

    HRESULT hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2,
        NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder2,
        (void **)&gcap.pBuilder);

    return (hr == NOERROR) ? TRUE : FALSE;
}


// Make a graph object we can use for capture graph building
//
BOOL MakeGraph() 
{
    // we have one already
    if(gcap.pFg)
        return TRUE;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
        IID_IGraphBuilder, (LPVOID *)&gcap.pFg);

    return (hr == NOERROR) ? TRUE : FALSE;
}


// make sure the preview window inside our window is as big as the
// dimensions of captured video, or some capture cards won't show a preview.
// (Also, it helps people tell what size video they're capturing)
// We will resize our app's window big enough so that once the status bar
// is positioned at the bottom there will be enough room for the preview
// window to be w x h
//
int gnRecurse = 0;


void ResizeWindow(int w, int h) 
{
    RECT rcW, rcC;
    int xExtra, yExtra;
    int cyBorder = GetSystemMetrics(SM_CYBORDER);

    gnRecurse++;

    GetWindowRect(ghwndApp, &rcW);
    GetClientRect(ghwndApp, &rcC);
    xExtra = rcW.right - rcW.left - rcC.right;
    yExtra = rcW.bottom - rcW.top - rcC.bottom + cyBorder + statusGetHeight();

    rcC.right = w;
    rcC.bottom = h;
    SetWindowPos(ghwndApp, NULL, 0, 0, rcC.right + xExtra,
        rcC.bottom + yExtra, SWP_NOZORDER | SWP_NOMOVE);

    // we may need to recurse once.  But more than that means the window cannot
    // be made the size we want, trying will just stack fault.
    //
    if(gnRecurse == 1 && ((rcC.right + xExtra != rcW.right - rcW.left && w > GetSystemMetrics(SM_CXMIN)) ||
        (rcC.bottom + yExtra != rcW.bottom - rcW.top)))
        ResizeWindow(w,h);

    gnRecurse--;
}


// Tear down everything downstream of a given filter
void NukeDownstream(IBaseFilter *pf) 
{
    //DbgLog((LOG_TRACE,1,TEXT("Nuking...")));

    IPin *pP, *pTo;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;
    HRESULT hr = pf->EnumPins(&pins);
    pins->Reset();
    while(hr == NOERROR) {
        hr = pins->Next(1, &pP, &u);
        if(hr == S_OK && pP) {
            pP->ConnectedTo(&pTo);
            if(pTo) {
                hr = pTo->QueryPinInfo(&pininfo);
                if(hr == NOERROR) {
                    if(pininfo.dir == PINDIR_INPUT) {
                        NukeDownstream(pininfo.pFilter);
                        gcap.pFg->Disconnect(pTo);
                        gcap.pFg->Disconnect(pP);
                        gcap.pFg->RemoveFilter(pininfo.pFilter);
                    }
                    pininfo.pFilter->Release();
                }
                pTo->Release();
            }
            pP->Release();
        }
    }
    if(pins)
        pins->Release();
}


// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
void TearDownGraph() 
{
    if(gcap.pSink)
        gcap.pSink->Release();
    gcap.pSink = NULL;
    if(gcap.pConfigAviMux)
        gcap.pConfigAviMux->Release();
    gcap.pConfigAviMux = NULL;
    if(gcap.pRender)
        gcap.pRender->Release();
    gcap.pRender = NULL;
    if(gcap.pVW) {
        // stop drawing in our window, or we may get wierd repaint effects
        gcap.pVW->put_Owner(NULL);
        gcap.pVW->put_Visible(OAFALSE);
        gcap.pVW->Release();
    }
    gcap.pVW = NULL;
    if(gcap.pME)
        gcap.pME->Release();
    gcap.pME = NULL;
    if(gcap.pDF)
        gcap.pDF->Release();
    gcap.pDF = NULL;

    // destroy the graph downstream of our capture filters
    if(gcap.pVCap)
        NukeDownstream(gcap.pVCap);
    if(gcap.pACap)
        NukeDownstream(gcap.pACap);

    // potential debug output - what the graph looks like
    // if (gcap.pFg) DumpGraph(gcap.pFg, 1);

#ifdef REGISTER_FILTERGRAPH
    // Remove filter graph from the running object table   
    if (g_dwGraphRegister)
    {
        RemoveGraphFromRot(g_dwGraphRegister);
        g_dwGraphRegister = 0;
    }
#endif

    gcap.fCaptureGraphBuilt = FALSE;
    gcap.fPreviewGraphBuilt = FALSE;
    gcap.fPreviewFaked = FALSE;
}


// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
//
BOOL InitCapFilters() 
{
    HRESULT hr=S_OK;
    BOOL f;

    gcap.fCCAvail = FALSE;  // assume no closed captioning support

    f = MakeBuilder();
    if(!f) {
        ErrMsg(TEXT("Cannot instantiate graph builder"));
        return FALSE;
    }

    //
    // First, we need a Video Capture filter, and some interfaces
    //

    gcap.pVCap = NULL;
    if(gcap.pmVideo != 0) {
        IPropertyBag *pBag;
        gcap.wachFriendlyName[0] = 0;
        hr = gcap.pmVideo->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR) {
                lstrcpyW(gcap.wachFriendlyName, var.bstrVal);
                SysFreeString(var.bstrVal);
            }
            pBag->Release();
        }
        hr = gcap.pmVideo->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pVCap);
    }

    if(gcap.pVCap == NULL) {
        ErrMsg(TEXT("Error %x: Cannot create video capture filter"), hr);
        goto InitCapFiltersFail;
    }

    //
    // make a filtergraph, give it to the graph builder and put the video
    // capture filter in the graph
    //

    f = MakeGraph();
    if(!f) {
        ErrMsg(TEXT("Cannot instantiate filtergraph"));
        goto InitCapFiltersFail;
    }
    hr = gcap.pBuilder->SetFiltergraph(gcap.pFg);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Cannot give graph to builder"));
        goto InitCapFiltersFail;
    }

    hr = gcap.pFg->AddFilter(gcap.pVCap, NULL);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Error %x: Cannot add vidcap to filtergraph"), hr);
        goto InitCapFiltersFail;
    }

    // Calling FindInterface below will result in building the upstream
    // section of the capture graph (any WDM TVTuners or Crossbars we might
    // need).

    // we use this interface to get the name of the driver
    // Don't worry if it doesn't work:  This interface may not be available
    // until the pin is connected, or it may not be available at all.
    // (eg: interface may not be available for some DV capture)
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved,
        gcap.pVCap, IID_IAMVideoCompression, (void **)&gcap.pVC);
    if(hr != S_OK) {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video,
            gcap.pVCap, IID_IAMVideoCompression, (void **)&gcap.pVC);
    }

    // !!! What if this interface isn't supported?
    // we use this interface to set the frame rate and get the capture size
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved,
        gcap.pVCap, IID_IAMStreamConfig, (void **)&gcap.pVSC);
    if(hr != NOERROR) {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, gcap.pVCap,
            IID_IAMStreamConfig, (void **)&gcap.pVSC);
        if(hr != NOERROR) {
            // this means we can't set frame rate (non-DV only)
            ErrMsg(TEXT("Error %x: Cannot find VCapture:IAMStreamConfig"), hr);
        }
    }

    gcap.fCapAudioIsRelevant = TRUE;

    AM_MEDIA_TYPE *pmt;

    // default capture format
    if(gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK) {
        // DV capture does not use a VIDEOINFOHEADER
        if(pmt->formattype == FORMAT_VideoInfo) {
            // resize our window to the default capture size
            ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
                ABS(HEADER(pmt->pbFormat)->biHeight));
        }
        if(pmt->majortype != MEDIATYPE_Video) {
            // This capture filter captures something other that pure video.
            // Maybe it's DV or something?  Anyway, chances are we shouldn't
            // allow capturing audio separately, since our video capture    
            // filter may have audio combined in it already!
            gcap.fCapAudioIsRelevant = FALSE;
            gcap.fCapAudio = FALSE;
        }
        DeleteMediaType(pmt);
    }

    // we use this interface to bring up the 3 dialogs
    // NOTE:  Only the VfW capture filter supports this.  This app only brings
    // up dialogs for legacy VfW capture drivers, since only those have dialogs
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video, gcap.pVCap,
        IID_IAMVfwCaptureDialogs, (void **)&gcap.pDlg);


    // Use the crossbar class to help us sort out all the possible video inputs
    // The class needs to be given the capture filters ANALOGVIDEO input pin
    {
        IPin        *pP = 0;
        IEnumPins   *pins;
        ULONG        n;
        PIN_INFO     pinInfo;
        BOOL         Found = FALSE;
        IKsPropertySet *pKs;
        GUID guid;
        DWORD dw;
        BOOL fMatch = FALSE;

        gcap.pCrossbar = NULL;

        if(SUCCEEDED(gcap.pVCap->EnumPins(&pins))) {            
            while(!Found && (S_OK == pins->Next(1, &pP, &n))) {
                if(S_OK == pP->QueryPinInfo(&pinInfo)) {
                    if(pinInfo.dir == PINDIR_INPUT) {
                        // is this pin an ANALOGVIDEOIN input pin?
                        if(pP->QueryInterface(IID_IKsPropertySet,
                            (void **)&pKs) == S_OK) {
                            if(pKs->Get(AMPROPSETID_Pin,
                                AMPROPERTY_PIN_CATEGORY, NULL, 0,
                                &guid, sizeof(GUID), &dw) == S_OK) {
                                if(guid == PIN_CATEGORY_ANALOGVIDEOIN)
                                    fMatch = TRUE;
                            }
                            pKs->Release();
                        }

                        if(fMatch) {
                            gcap.pCrossbar = new CCrossbar(pP);
                            hr = gcap.pCrossbar->GetInputCount
                                (&gcap.NumberOfVideoInputs);
                            Found = TRUE;
                        }
                    }
                    pinInfo.pFilter->Release();
                }
                pP->Release();
            }
            pins->Release();
        }
    }

    // there's no point making an audio capture filter
    if(gcap.fCapAudioIsRelevant == FALSE)
        goto SkipAudio;

    // create the audio capture filter, even if we are not capturing audio right
    // now, so we have all the filters around all the time.

    //
    // We want an audio capture filter and some interfaces
    //

    if(gcap.pmAudio == 0) {
        // there are no audio capture devices. We'll only allow video capture
        gcap.fCapAudio = FALSE;
        goto SkipAudio;
    }
    gcap.pACap = NULL;


    hr = gcap.pmAudio->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pACap);

    if(gcap.pACap == NULL) {
        // there are no audio capture devices. We'll only allow video capture
        gcap.fCapAudio = FALSE;
        ErrMsg(TEXT("Cannot create audio capture filter"));
        goto SkipAudio;
    }

    //
    // put the audio capture filter in the graph
    //

    // We'll need this in the graph to get audio property pages
    hr = gcap.pFg->AddFilter(gcap.pACap, NULL);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Error %x: Cannot add audcap to filtergraph"), hr);
        goto InitCapFiltersFail;
    }

    // Calling FindInterface below will result in building the upstream
    // section of the capture graph (any WDM TVAudio's or Crossbars we might
    // need).

    // !!! What if this interface isn't supported?
    // we use this interface to set the captured wave format
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
        gcap.pACap, IID_IAMStreamConfig, (void **)&gcap.pASC);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Cannot find ACapture:IAMStreamConfig"));
    }

SkipAudio:

    // Can this filter do closed captioning?
    IPin *pPin;
    hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_VBI,
        NULL, FALSE, 0, &pPin);
    if(hr != S_OK)
        hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_CC,
            NULL, FALSE, 0, &pPin);
    if(hr == S_OK) {
        pPin->Release();
        gcap.fCCAvail = TRUE;
    }
    else {
        gcap.fCapCC = FALSE;    // can't capture it, then
    }

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    return TRUE;

InitCapFiltersFail:
    FreeCapFilters();
    return FALSE;
}


// all done with the capture filters and the graph builder
//
void FreeCapFilters() 
{
    if(gcap.pFg)
        gcap.pFg->Release();
    gcap.pFg = NULL;
    if(gcap.pBuilder)
        gcap.pBuilder->Release();
    gcap.pBuilder = NULL;
    if(gcap.pVCap)
        gcap.pVCap->Release();
    gcap.pVCap = NULL;
    if(gcap.pACap)
        gcap.pACap->Release();
    gcap.pACap = NULL;
    if(gcap.pASC)
        gcap.pASC->Release();
    gcap.pASC = NULL;
    if(gcap.pVSC)
        gcap.pVSC->Release();
    gcap.pVSC = NULL;
    if(gcap.pVC)
        gcap.pVC->Release();
    gcap.pVC = NULL;
    if(gcap.pDlg)
        gcap.pDlg->Release();
    gcap.pDlg = NULL;

    if(gcap.pCrossbar) {
        delete gcap.pCrossbar;
        gcap.pCrossbar = NULL;
    }
}


// build the capture graph
//
BOOL BuildCaptureGraph() 
{
    USES_CONVERSION;
    int cy, cyBorder;
    HRESULT hr;
    BOOL f;
    AM_MEDIA_TYPE *pmt;

    // we have one already
    if(gcap.fCaptureGraphBuilt)
        return TRUE;

    // No rebuilding while we're running
    if(gcap.fCapturing || gcap.fPreviewing)
        return FALSE;

    // We don't have the necessary capture filters
    if(gcap.pVCap == NULL)
        return FALSE;
    if(gcap.pACap == NULL && gcap.fCapAudio)
        return FALSE;

    // no capture file name yet... we need one first
    if(gcap.szCaptureFile[0] == 0) {
        f = SetCaptureFile(ghwndApp);
        if(!f)
            return f;
    }

    // we already have another graph built... tear down the old one
    if(gcap.fPreviewGraphBuilt)
        TearDownGraph();

    //
    // We need a rendering section that will write the capture file out in AVI
    // file format
    //

    GUID guid = MEDIASUBTYPE_Avi;
    hr = gcap.pBuilder->SetOutputFileName(&guid, T2W(gcap.szCaptureFile),
        &gcap.pRender, &gcap.pSink);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Cannot set output file"));
        goto SetupCaptureFail;
    }

    // Now tell the AVIMUX to write out AVI files that old apps can read properly.
    // If we don't, most apps won't be able to tell where the keyframes are,
    // slowing down editing considerably
    // Doing this will cause one seek (over the area the index will go) when
    // you capture past 1 Gig, but that's no big deal.
    // NOTE: This is on by default, so it's not necessary to turn it on

    // Also, set the proper MASTER STREAM

    hr = gcap.pRender->QueryInterface(IID_IConfigAviMux,
        (void **)&gcap.pConfigAviMux);
    if(hr == NOERROR && gcap.pConfigAviMux) {
        gcap.pConfigAviMux->SetOutputCompatibilityIndex(TRUE);
        if(gcap.fCapAudio) {
            hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
            if(hr != NOERROR)
                ErrMsg(TEXT("SetMasterStream failed!"));
        }
    }

    //
    // Render the video capture and preview pins - even if the capture filter only
    // has a capture pin (and no preview pin) this should work... because the
    // capture graph builder will use a smart tee filter to provide both capture
    // and preview.  We don't have to worry.  It will just work.
    //

    // NOTE that we try to render the interleaved pin before the video pin, because
    // if BOTH exist, it's a DV filter and the only way to get the audio is to use
    // the interleaved pin.  Using the Video pin on a DV filter is only useful if
    // you don't want the audio.

    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved,
        gcap.pVCap, NULL, gcap.pRender);
    if(hr != NOERROR) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video,
            gcap.pVCap, NULL, gcap.pRender);
        if(hr != NOERROR) {
            ErrMsg(TEXT("Cannot render video capture stream"));
            goto SetupCaptureFail;
        }
    }

    if(gcap.fWantPreview) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
            &MEDIATYPE_Interleaved, gcap.pVCap, NULL, NULL);
        if(hr == VFW_S_NOPREVIEWPIN) {
            // preview was faked up for us using the (only) capture pin
            gcap.fPreviewFaked = TRUE;
        }
        else if(hr != S_OK) {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
                &MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
            if(hr == VFW_S_NOPREVIEWPIN) {
                // preview was faked up for us using the (only) capture pin
                gcap.fPreviewFaked = TRUE;
            }
            else if(hr != S_OK) {
                ErrMsg(TEXT("Cannot render video preview stream"));
                goto SetupCaptureFail;
            }
        }
    }

    //
    // Render the audio capture pin?
    //

    if(gcap.fCapAudio) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Audio, gcap.pACap, NULL, gcap.pRender);
        if(hr != NOERROR) {
            ErrMsg(TEXT("Cannot render audio capture stream"));
            goto SetupCaptureFail;
        }
    }

    //
    // Render the closed captioning pin? It could be a CC or a VBI category pin,
    // depending on the capture driver
    //

    if(gcap.fCapCC) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CC, NULL,
            gcap.pVCap, NULL, gcap.pRender);
        if(hr != NOERROR) {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
                gcap.pVCap, NULL, gcap.pRender);
            if(hr != NOERROR) {
                ErrMsg(TEXT("Cannot render closed captioning"));
                // so what? goto SetupCaptureFail;
            }
        }
        // To preview and capture VBI at the same time, we can call this twice
        if(gcap.fWantPreview) {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
                gcap.pVCap, NULL, NULL);
        }
    }

    //
    // Get the preview window to be a child of our app's window
    //

    // This will find the IVideoWindow interface on the renderer.  It is 
    // important to ask the filtergraph for this interface... do NOT use
    // ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
    // know we own the window so it can give us display changed messages, etc.

    if(gcap.fWantPreview) {
        hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
        if(hr != NOERROR && gcap.fWantPreview) {
            ErrMsg(TEXT("This graph cannot preview"));
        }
        else if(hr == NOERROR) {
            RECT rc;
            gcap.pVW->put_Owner((OAHWND)ghwndApp);    // We own the window now
            gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child
            // give the preview window all our space but where the status bar is
            GetClientRect(ghwndApp, &rc);
            cyBorder = GetSystemMetrics(SM_CYBORDER);
            cy = statusGetHeight() + cyBorder;
            rc.bottom -= cy;
            gcap.pVW->SetWindowPosition(0, 0, rc.right, rc.bottom); // be this big
            gcap.pVW->put_Visible(OATRUE);
        }
    }

    // now tell it what frame rate to capture at.  Just find the format it
    // is capturing with, and leave everything alone but change the frame rate
    hr = gcap.fUseFrameRate ? E_FAIL : NOERROR;
    if(gcap.pVSC && gcap.fUseFrameRate) {
        hr = gcap.pVSC->GetFormat(&pmt);
        // DV capture does not use a VIDEOINFOHEADER
        if(hr == NOERROR) {
            if(pmt->formattype == FORMAT_VideoInfo) {
                VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
                pvi->AvgTimePerFrame = (LONGLONG)(10000000 / gcap.FrameRate);
                hr = gcap.pVSC->SetFormat(pmt);
            }
            DeleteMediaType(pmt);
        }
    }
    if(hr != NOERROR)
        ErrMsg(TEXT("Cannot set frame rate for capture"));

    // now ask the filtergraph to tell us when something is completed or aborted
    // (EC_COMPLETE, EC_USERABORT, EC_ERRORABORT).  This is how we will find out
    // if the disk gets full while capturing
    hr = gcap.pFg->QueryInterface(IID_IMediaEventEx, (void **)&gcap.pME);
    if(hr == NOERROR) {
        gcap.pME->SetNotifyWindow((OAHWND)ghwndApp, WM_FGNOTIFY, 0);
    }

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
    hr = AddGraphToRot(gcap.pFg, &g_dwGraphRegister);
    if (FAILED(hr))
    {
        ErrMsg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        g_dwGraphRegister = 0;
    }
#endif

    // All done.
    gcap.fCaptureGraphBuilt = TRUE;
    return TRUE;

SetupCaptureFail:
    TearDownGraph();
    return FALSE;
}



// build the preview graph!
//
// !!! PLEASE NOTE !!!  Some new WDM devices have totally separate capture
// and preview settings.  An application that wishes to preview and then 
// capture may have to set the preview pin format using IAMStreamConfig on the
// preview pin, and then again on the capture pin to capture with that format.
// In this sample app, there is a separate page to set the settings on the 
// capture pin and one for the preview pin.  To avoid the user
// having to enter the same settings in 2 dialog boxes, an app can have its own
// UI for choosing a format (the possible formats can be enumerated using
// IAMStreamConfig) and then the app can programmatically call IAMStreamConfig
// to set the format on both pins.
//
BOOL BuildPreviewGraph() 
{
    int cy, cyBorder;
    HRESULT hr;
    AM_MEDIA_TYPE *pmt;

    // we have one already
    if(gcap.fPreviewGraphBuilt)
        return TRUE;

    // No rebuilding while we're running
    if(gcap.fCapturing || gcap.fPreviewing)
        return FALSE;

    // We don't have the necessary capture filters
    if(gcap.pVCap == NULL)
        return FALSE;
    if(gcap.pACap == NULL && gcap.fCapAudio)
        return FALSE;

    // we already have another graph built... tear down the old one
    if(gcap.fCaptureGraphBuilt)
        TearDownGraph();

    //
    // Render the preview pin - even if there is not preview pin, the capture
    // graph builder will use a smart tee filter and provide a preview.
    //
    // !!! what about latency/buffer issues?

    // NOTE that we try to render the interleaved pin before the video pin, because
    // if BOTH exist, it's a DV filter and the only way to get the audio is to use
    // the interleaved pin.  Using the Video pin on a DV filter is only useful if
    // you don't want the audio.

    hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
        &MEDIATYPE_Interleaved, gcap.pVCap, NULL, NULL);
    if(hr == VFW_S_NOPREVIEWPIN) {
        // preview was faked up for us using the (only) capture pin
        gcap.fPreviewFaked = TRUE;
    }
    else if(hr != S_OK) {
        // maybe it's DV?
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
            &MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
        if(hr == VFW_S_NOPREVIEWPIN) {
            // preview was faked up for us using the (only) capture pin
            gcap.fPreviewFaked = TRUE;
        }
        else if(hr != S_OK) {
            ErrMsg(TEXT("This graph cannot preview!"));
        }
    }

    //
    // Render the closed captioning pin? It could be a CC or a VBI category pin,
    // depending on the capture driver
    //

    if(gcap.fCapCC) {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CC, NULL,
            gcap.pVCap, NULL, NULL);
        if(hr != NOERROR) {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
                gcap.pVCap, NULL, NULL);
            if(hr != NOERROR) {
                ErrMsg(TEXT("Cannot render closed captioning"));
                // so what? goto SetupCaptureFail;
            }
        }
    }

    //
    // Get the preview window to be a child of our app's window
    //

    // This will find the IVideoWindow interface on the renderer.  It is 
    // important to ask the filtergraph for this interface... do NOT use
    // ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
    // know we own the window so it can give us display changed messages, etc.

    hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
    if(hr != NOERROR) {
        ErrMsg(TEXT("This graph cannot preview properly"));
    }
    else {

        //Find out if this is a DV stream
        AM_MEDIA_TYPE * pmtDV;


        if(gcap.pVSC && SUCCEEDED(gcap.pVSC->GetFormat(&pmtDV))) {
            if(pmtDV->formattype == FORMAT_DvInfo) {
                // in this case we want to set the size of the parent window to that of 
                // current DV resolution.
                // We get that resolution from the IVideoWindow.
                CComQIPtr <IBasicVideo, &IID_IBasicVideo> pBV(gcap.pVW);

                if(pBV != NULL) {   
                    HRESULT hr1, hr2;
                    long lWidth, lHeight;
                    hr1 = pBV->get_VideoHeight(&lHeight);
                    hr2 = pBV->get_VideoWidth(&lWidth);
                    if(SUCCEEDED(hr1) && SUCCEEDED(hr2)) {
                        ResizeWindow(lWidth, abs(lHeight));
                    }
                }   
            }
        }

        RECT rc;
        gcap.pVW->put_Owner((OAHWND)ghwndApp);    // We own the window now
        gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child


        // give the preview window all our space but where the status bar is
        GetClientRect(ghwndApp, &rc);
        cyBorder = GetSystemMetrics(SM_CYBORDER);
        cy = statusGetHeight() + cyBorder;
        rc.bottom -= cy;
        gcap.pVW->SetWindowPosition(0, 0, rc.right, rc.bottom); // be this big
        gcap.pVW->put_Visible(OATRUE);
    }

    // now tell it what frame rate to capture at.  Just find the format it
    // is capturing with, and leave everything alone but change the frame rate
    // No big deal if it fails.  It's just for preview
    // !!! Should we then talk to the preview pin?
    if(gcap.pVSC && gcap.fUseFrameRate) {
        hr = gcap.pVSC->GetFormat(&pmt);
        // DV capture does not use a VIDEOINFOHEADER
        if(hr == NOERROR) {
            if(pmt->formattype == FORMAT_VideoInfo) {
                VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
                pvi->AvgTimePerFrame = (LONGLONG)(10000000 / gcap.FrameRate);
                hr = gcap.pVSC->SetFormat(pmt);
                if(hr != NOERROR)
                    ErrMsg(TEXT("%x: Cannot set frame rate for preview"), hr);
            }
            DeleteMediaType(pmt);
        }
    }

    // make sure we process events while we're previewing!
    hr = gcap.pFg->QueryInterface(IID_IMediaEventEx, (void **)&gcap.pME);
    if(hr == NOERROR) {
        gcap.pME->SetNotifyWindow((OAHWND)ghwndApp, WM_FGNOTIFY, 0);
    }

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
    hr = AddGraphToRot(gcap.pFg, &g_dwGraphRegister);
    if (FAILED(hr))
    {
        ErrMsg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        g_dwGraphRegister = 0;
    }
#endif

    // All done.
    gcap.fPreviewGraphBuilt = TRUE;
    return TRUE;
}


// Start previewing
//
BOOL StartPreview() 
{
    // way ahead of you
    if(gcap.fPreviewing)
        return TRUE;

    if(!gcap.fPreviewGraphBuilt)
        return FALSE;

    // run the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr)) {
        hr = pMC->Run();
        if(FAILED(hr)) {
            // stop parts that ran
            pMC->Stop();
        }
        pMC->Release();
    }
    if(FAILED(hr)) {
        ErrMsg(TEXT("Error %x: Cannot run preview graph"), hr);
        return FALSE;
    }

    gcap.fPreviewing = TRUE;
    return TRUE;
}


// stop the preview graph
//
BOOL StopPreview() 
{
    // way ahead of you
    if(!gcap.fPreviewing) {
        return FALSE;
    }

    // stop the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr)) {
        hr = pMC->Stop();
        pMC->Release();
    }
    if(FAILED(hr)) {
        ErrMsg(TEXT("Error %x: Cannot stop preview graph"), hr);
        return FALSE;
    }

    gcap.fPreviewing = FALSE;

    // !!! get rid of menu garbage
    InvalidateRect(ghwndApp, NULL, TRUE);

    return TRUE;
}


// start the capture graph
//
BOOL StartCapture() 
{
    BOOL f, fHasStreamControl;
    HRESULT hr;

    // way ahead of you
    if(gcap.fCapturing)
        return TRUE;

    // or we'll get confused
    if(gcap.fPreviewing)
        StopPreview();

    // or we'll crash
    if(!gcap.fCaptureGraphBuilt)
        return FALSE;

    // This amount will be subtracted from the number of dropped and not 
    // dropped frames reported by the filter.  Since we might be having the
    // filter running while the pin is turned off, we don't want any of the
    // frame statistics from the time the pin is off interfering with the
    // statistics we gather while the pin is on
    gcap.lDroppedBase = 0;
    gcap.lNotBase = 0;

    REFERENCE_TIME start = MAX_TIME, stop = MAX_TIME;

    // don't capture quite yet...
    hr = gcap.pBuilder->ControlStream(&PIN_CATEGORY_CAPTURE, NULL,
        NULL, &start, NULL, 0, 0);
    //DbgLog((LOG_TRACE,1,TEXT("Capture OFF returns %x"), hr));

    // Do we have the ability to control capture and preview separately?
    fHasStreamControl = SUCCEEDED(hr);

    // prepare to run the graph
    IMediaControl *pMC = NULL;
    hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(FAILED(hr)) {
        ErrMsg(TEXT("Error %x: Cannot get IMediaControl"), hr);
        return FALSE;
    }

    // If we were able to keep capture off, then we can
    // run the graph now for frame accurate start later yet still showing a
    // preview.   Otherwise, we can't run the graph yet without capture
    // starting too, so we'll pause it so the latency between when they
    // press a key and when capture begins is still small (but they won't have
    // a preview while they wait to press a key)

    if(fHasStreamControl)
        hr = pMC->Run();
    else
        hr = pMC->Pause();
    if(FAILED(hr)) {
        // stop parts that started
        pMC->Stop();
        pMC->Release();
        ErrMsg(TEXT("Error %x: Cannot start graph"), hr);
        return FALSE;
    }

    // press a key to start capture
    f = DoDialog(ghwndApp, IDD_PressAKeyDialog, (DLGPROC)PressAKeyProc, 0);
    if(!f) {
        pMC->Stop();
        pMC->Release();
        if(gcap.fWantPreview) {
            BuildPreviewGraph();
            StartPreview();
        }
        return f;
    }

    // Start capture NOW!
    if(fHasStreamControl) {
        // we may not have this yet
        if(!gcap.pDF) {
            hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                &MEDIATYPE_Interleaved, gcap.pVCap,
                IID_IAMDroppedFrames, (void **)&gcap.pDF);
            if(hr != NOERROR)
                hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                    &MEDIATYPE_Video, gcap.pVCap,
                    IID_IAMDroppedFrames, (void **)&gcap.pDF);
        }

        // turn the capture pin on now!
        hr = gcap.pBuilder->ControlStream(&PIN_CATEGORY_CAPTURE, NULL,
            NULL, NULL, &stop, 0, 0);
        //DbgLog((LOG_TRACE,0,TEXT("Capture ON returns %x"), hr));
        // make note of the current dropped frame counts
        if(gcap.pDF) {
            gcap.pDF->GetNumDropped(&gcap.lDroppedBase);
            gcap.pDF->GetNumNotDropped(&gcap.lNotBase);
            //DbgLog((LOG_TRACE,0,TEXT("Dropped counts are %ld and %ld"),
            //      gcap.lDroppedBase, gcap.lNotBase));
        } 
    }
    else {
        hr = pMC->Run();
        if(FAILED(hr)) {
            // stop parts that started
            pMC->Stop();
            pMC->Release();
            ErrMsg(TEXT("Error %x: Cannot run graph"), hr);
            return FALSE;
        }
    }

    pMC->Release();

    // when did we start capture?
    gcap.lCapStartTime = timeGetTime();

    // update status bar 30 times per second - #captured, #dropped
    SetTimer(ghwndApp, 1, 33, NULL);

    gcap.fCapturing = TRUE;
    return TRUE;
}


// stop the capture graph
//
BOOL StopCapture() 
{
    // way ahead of you
    if(!gcap.fCapturing) {
        return FALSE;
    }

    // stop the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr)) {
        hr = pMC->Stop();
        pMC->Release();
    }
    if(FAILED(hr)) {
        ErrMsg(TEXT("Error %x: Cannot stop graph"), hr);
        return FALSE;
    }

    // when the graph was stopped
    gcap.lCapStopTime = timeGetTime();

    // no more status bar updates
    KillTimer(ghwndApp, 1);

    // one last time for the final count and all the stats
    UpdateStatus(TRUE);

    gcap.fCapturing = FALSE;

    // !!! get rid of menu garbage
    InvalidateRect(ghwndApp, NULL, TRUE);

    return TRUE;
}


// Let's talk about UI for a minute.  There are many programmatic interfaces
// you can use to program a capture filter or related filter to capture the
// way you want it to.... eg:  IAMStreamConfig, IAMVideoCompression,
// IAMCrossbar, IAMTVTuner, IAMTVAudio, IAMAnalogVideoDecoder, IAMCameraControl,
// IAMVideoProcAmp, etc.
//
// But you probably want some UI to let the user play with all these settings.
// For new WDM-style capture devices, we offer some default UI you can use.
// The code below shows how to bring up all of the dialog boxes supported 
// by any capture filters.
//
// The following code shows you how you can bring up all of the
// dialogs supported by a particular object at once on a big page with lots
// of thumb tabs.  You do this by starting with an interface on the object that
// you want, and using ISpecifyPropertyPages to get the whole list, and
// OleCreatePropertyFrame to bring them all up.  This way you will get custom
// property pages a filter has, too, that are not one of the standard pages that
// you know about.  There are at least 9 objects that may have property pages.
// Your app already has 2 of the object pointers, the video capture filter and
// the audio capture filter (let's call them pVCap and pACap)
// 1.  The video capture filter - pVCap
// 2.  The video capture filter's capture pin - get this by calling
//     FindInterface(&PIN_CATEGORY_CAPTURE, pVCap, IID_IPin, &pX);
// 3.  The video capture filter's preview pin - get this by calling
//     FindInterface(&PIN_CATEGORY_PREVIEW, pVCap, IID_IPin, &pX);
// 4.  The audio capture filter - pACap
// 5.  The audio capture filter's capture pin - get this by calling
//     FindInterface(&PIN_CATEGORY_CAPTURE, pACap, IID_IPin, &pX);
// 6.  The crossbar connected to the video capture filter - get this by calling
//     FindInterface(NULL, pVCap, IID_IAMCrossbar, &pX);
// 7.  There is a possible second crossbar to control audio - get this by 
//     looking upstream of the first crossbar like this:
//     FindInterface(&LOOK_UPSTREAM_ONLY, pX, IID_IAMCrossbar, &pX2);
// 8.  The TV Tuner connected to the video capture filter - get this by calling
//     FindInterface(NULL, pVCap, IID_IAMTVTuner, &pX);
// 9.  The TV Audio connected to the audio capture filter - get this by calling
//     FindInterface(NULL, pACap, IID_IAMTVAudio, &pX);
// 10. We have a helper class, CCrossbar, which makes the crossbar issue less
//     confusing.  In fact, although not supported here, there may be more than
//     two crossbars, arranged in many different ways.  An application may not
//     wish to have separate dialogs for each crossbar, but instead hide the
//     complexity and simply offer the user a list of inputs that can be chosen.
//     This list represents all the unique inputs from all the crossbars.
//     The crossbar helper class does this and offers that list as #10.  It is
//     expected that an application will either provide the crossbar dialogs
//     above (#6 and #7) OR provide the input list (this #10), but not both.
//     That would be confusing because if you select an input using dialog 6 or
//     7 the input list here in #10 won't know about your choice.
//
// Your last choice for UI is to make your own pages, and use the results of 
// your custom page to call the interfaces programmatically.


void MakeMenuOptions() 
{
    HRESULT hr;
    HMENU hMenuSub = GetSubMenu(GetMenu(ghwndApp), 2); // Options menu

    // remove any old choices from the last device
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);
    RemoveMenu(hMenuSub, 4, MF_BYPOSITION);

    int zz = 0;
    gcap.iFormatDialogPos = -1;
    gcap.iSourceDialogPos = -1;
    gcap.iDisplayDialogPos = -1;
    gcap.iVCapDialogPos = -1;
    gcap.iVCrossbarDialogPos = -1;
    gcap.iTVTunerDialogPos = -1;
    gcap.iACapDialogPos = -1;
    gcap.iACrossbarDialogPos = -1;
    gcap.iTVAudioDialogPos = -1;
    gcap.iVCapCapturePinDialogPos = -1;
    gcap.iVCapPreviewPinDialogPos = -1;
    gcap.iACapCapturePinDialogPos = -1;

    // If this device supports the old legacy UI dialogs, offer them

    if(gcap.pDlg && !gcap.pDlg->HasDialog(VfwCaptureDialog_Format)) {
        AppendMenu(hMenuSub, MF_STRING, MENU_DIALOG0 + zz, TEXT("Video Format..."));
        gcap.iFormatDialogPos = zz++;
    }
    if(gcap.pDlg && !gcap.pDlg->HasDialog(VfwCaptureDialog_Source)) {
        AppendMenu(hMenuSub, MF_STRING, MENU_DIALOG0 + zz, TEXT("Video Source..."));
        gcap.iSourceDialogPos = zz++;
    }
    if(gcap.pDlg && !gcap.pDlg->HasDialog(VfwCaptureDialog_Display)) {
        AppendMenu(hMenuSub, MF_STRING, MENU_DIALOG0 + zz, TEXT("Video Display..."));
        gcap.iDisplayDialogPos = zz++;
    }

    // Also check the audio capture filter at this point, since even non wdm devices
    // may support an IAMAudioInputMixer property page (we'll also get any wdm filter
    // properties here as well). We'll get any audio capture pin property pages just
    // a bit later.
    if(gcap.pACap != NULL) {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;

        hr = gcap.pACap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz, TEXT("Audio Capture Filter..."));
                gcap.iACapDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
    }

    // don't bother looking for new property pages if the old ones are supported
    // or if we don't have a capture filter
    if(gcap.pVCap == NULL || gcap.iFormatDialogPos != -1)
        return;

    // New WDM devices support new UI and new interfaces.
    // Your app can use some default property
    // pages for UI if you'd like (like we do here) or if you don't like our
    // dialog boxes, feel free to make your own and programmatically set 
    // the capture options through interfaces like IAMCrossbar, IAMCameraControl
    // etc.

    // There are 9 objects that might support property pages.  Let's go through
    // them.

    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;

    // 1. the video capture filter itself

    hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
    if(hr == S_OK) {
        hr = pSpec->GetPages(&cauuid);
        if(hr == S_OK && cauuid.cElems > 0) {
            AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz, TEXT("Video Capture Filter..."));
            gcap.iVCapDialogPos = zz++;
            CoTaskMemFree(cauuid.pElems);
        }
        pSpec->Release();
    }

    // 2.  The video capture capture pin

    IAMStreamConfig *pSC;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved,
        gcap.pVCap, IID_IAMStreamConfig, (void **)&pSC);
    if(hr != S_OK)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, gcap.pVCap,
            IID_IAMStreamConfig, (void **)&pSC);

    if(hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz, TEXT("Video Capture Pin..."));
                gcap.iVCapCapturePinDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 3.  The video capture preview pin.
    // This basically sets the format being previewed.  Typically, you
    // want to capture and preview using the SAME format, instead of having to
    // enter the same value in 2 dialog boxes.  For a discussion on this, see
    // the comment above the MakePreviewGraph function.

    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
        &MEDIATYPE_Interleaved, gcap.pVCap,
        IID_IAMStreamConfig, (void **)&pSC);
    if(hr != NOERROR)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
            &MEDIATYPE_Video, gcap.pVCap,
            IID_IAMStreamConfig, (void **)&pSC);
    if(hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz,TEXT("Video Preview Pin..."));
                gcap.iVCapPreviewPinDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 4 & 5.  The video crossbar, and a possible second crossbar

    IAMCrossbar *pX, *pX2;
    IBaseFilter *pXF;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved, gcap.pVCap,
        IID_IAMCrossbar, (void **)&pX);
    if(hr != S_OK)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, gcap.pVCap,
            IID_IAMCrossbar, (void **)&pX);

    if(hr == S_OK) {
        hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
        if(hr == S_OK) {
            hr = pX->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
            if(hr == S_OK) {
                hr = pSpec->GetPages(&cauuid);
                if(hr == S_OK && cauuid.cElems > 0) {
                    AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz,
                        TEXT("Video Crossbar..."));
                    gcap.iVCrossbarDialogPos = zz++;
                    CoTaskMemFree(cauuid.pElems);
                }
                pSpec->Release();
            }
            hr = gcap.pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,
                IID_IAMCrossbar, (void **)&pX2);
            if(hr == S_OK) {
                hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK) {
                    hr = pSpec->GetPages(&cauuid);
                    if(hr == S_OK && cauuid.cElems > 0) {
                        AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz,
                            TEXT("Second Crossbar..."));
                        gcap.iACrossbarDialogPos = zz++;
                        CoTaskMemFree(cauuid.pElems);
                    }
                    pSpec->Release();
                }
                pX2->Release();
            }
            pXF->Release();
        }
        pX->Release();
    }

    // 6.  The TVTuner

    IAMTVTuner *pTV;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved, gcap.pVCap,
        IID_IAMTVTuner, (void **)&pTV);
    if(hr != S_OK)
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, gcap.pVCap,
            IID_IAMTVTuner, (void **)&pTV);
    if(hr == S_OK) {
        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz, TEXT("TV Tuner..."));
                gcap.iTVTunerDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pTV->Release();
    }

    // no audio capture, we're done
    if(gcap.pACap == NULL)
        return;

    // 7.  The Audio capture filter itself... Thanks anyway, but we got these already

    // 8.  The Audio capture pin

    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Audio, gcap.pACap,
        IID_IAMStreamConfig, (void **)&pSC);
    if(hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz, TEXT("Audio Capture Pin..."));
                gcap.iACapCapturePinDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 9.  The TV Audio filter

    IAMTVAudio *pTVA;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, 
        &MEDIATYPE_Audio, gcap.pACap,
        IID_IAMTVAudio, (void **)&pTVA);
    if(hr == S_OK) {
        hr = pTVA->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+zz, TEXT("TV Audio..."));
                gcap.iTVAudioDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pTVA->Release();
    }

    // 10.  Crossbar class helper menu item, to let you choose an input

    if(gcap.pCrossbar && gcap.NumberOfVideoInputs) {
        gcap.hMenuPopup = CreatePopupMenu();
        LONG j;
        LONG  PhysicalType;
        TCHAR buf[MAX_PATH];
        LONG InputToEnable = -1;

        gcap.iVideoInputMenuPos = zz++;
        AppendMenu(hMenuSub, MF_SEPARATOR, 0, NULL);

        for(j = 0; j < gcap.NumberOfVideoInputs; j++) {
            EXECUTE_ASSERT(S_OK == gcap.pCrossbar->GetInputType(j, &PhysicalType));
            EXECUTE_ASSERT(S_OK == gcap.pCrossbar->GetInputName(j, buf, sizeof (buf)));
            AppendMenu(gcap.hMenuPopup,MF_STRING,MENU_DIALOG0+zz, buf);
            zz++;

            // Route the first TVTuner by default
            if((PhysicalType == PhysConn_Video_Tuner) && InputToEnable == -1) {
                InputToEnable = j;
            }
        }

        AppendMenu(hMenuSub, MF_STRING | MF_POPUP, (UINT_PTR)gcap.hMenuPopup, TEXT("Video Input"));

        if(InputToEnable == -1) {
            InputToEnable = 0;
        }
        CheckMenuItem(gcap.hMenuPopup, InputToEnable, MF_BYPOSITION | MF_CHECKED); 

        gcap.pCrossbar->SetInputIndex(InputToEnable);
    }
    // !!! anything needed to delete the popup when selecting a new input?
}


// how many captured/dropped so far
//
void UpdateStatus(BOOL fAllStats) 
{
    HRESULT hr;
    LONG lDropped, lNot=0, lAvgFrameSize;
    TCHAR tach[160];

    // we use this interface to get the number of captured and dropped frames
    // NOTE:  We cannot query for this interface earlier, as it may not be
    // available until the pin is connected
    if(!gcap.pDF) {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Interleaved, gcap.pVCap,
            IID_IAMDroppedFrames, (void **)&gcap.pDF);
        if(hr != S_OK)
            hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                &MEDIATYPE_Video, gcap.pVCap,
                IID_IAMDroppedFrames, (void **)&gcap.pDF);
    }

    // this filter can't tell us dropped frame info.
    if(!gcap.pDF) {
        statusUpdateStatus(ghwndStatus,
            TEXT("Filter cannot report capture information"));
        return;
    }

    hr = gcap.pDF->GetNumDropped(&lDropped);
    if(hr == S_OK)
        hr = gcap.pDF->GetNumNotDropped(&lNot);
    if(hr != S_OK)
        return;

    lDropped -= gcap.lDroppedBase;
    lNot -= gcap.lNotBase;

    if(!fAllStats) {
        LONG lTime = timeGetTime() - gcap.lCapStartTime;
        wsprintf(tach, TEXT("Captured %d frames (%d dropped) %d.%dsec"), lNot,
            lDropped, lTime / 1000, 
            lTime / 100 - lTime / 1000 * 10);
        statusUpdateStatus(ghwndStatus, tach);
        return;
    }

    // we want all possible stats, including capture time and actual acheived
    // frame rate and data rate (as opposed to what we tried to get).  These
    // numbers are an indication that though we dropped frames just now, if we
    // chose a data rate and frame rate equal to the numbers I'm about to
    // print, we probably wouldn't drop any frames.

    // average size of frame captured
    hr = gcap.pDF->GetAverageFrameSize(&lAvgFrameSize);
    if(hr != S_OK)
        return;

    // how long capture lasted
    LONG lDurMS = gcap.lCapStopTime - gcap.lCapStartTime;
    double flFrame;     // acheived frame rate
    LONG lData;         // acheived data rate

    if(lDurMS > 0) {
        flFrame = (double)(LONGLONG)lNot * 1000. /
            (double)(LONGLONG)lDurMS;
        lData = (LONG)(LONGLONG)(lNot / (double)(LONGLONG)lDurMS *
            1000. * (double)(LONGLONG)lAvgFrameSize);
    }
    else {
        flFrame = 0.;
        lData = 0;
    }

    wsprintf(tach, TEXT("Captured %d frames in %d.%d sec (%d dropped): %d.%d fps %d.%d Meg/sec"),
        lNot, lDurMS / 1000, lDurMS / 100 - lDurMS / 1000 * 10,
        lDropped, (int)flFrame,
        (int)(flFrame * 10.) - (int)flFrame * 10,
        lData / 1000000,
        lData / 1000 - (lData / 1000000 * 1000));
    statusUpdateStatus(ghwndStatus, tach);
}


// Check the devices we're currently using and make filters for them
//
void ChooseDevices(IMoniker *pmVideo, IMoniker *pmAudio) 
{
    USES_CONVERSION;
#define VERSIZE 40
#define DESCSIZE 80
    int versize = VERSIZE;
    int descsize = DESCSIZE;
    WCHAR wachVer[VERSIZE]={0}, wachDesc[DESCSIZE]={0};
    TCHAR tachStatus[VERSIZE + DESCSIZE + 5]={0};


    // they chose a new device. rebuild the graphs
    if(gcap.pmVideo != pmVideo || gcap.pmAudio != pmAudio) {
        if(pmVideo) {
            pmVideo->AddRef();
        }
        if(pmAudio) {
            pmAudio->AddRef();
        }
        IMonRelease(gcap.pmVideo);
        IMonRelease(gcap.pmAudio);
        gcap.pmVideo = pmVideo;
        gcap.pmAudio = pmAudio;
        if(gcap.fPreviewing)
            StopPreview();
        if(gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
            TearDownGraph();
        FreeCapFilters();
        InitCapFilters();
        if(gcap.fWantPreview) { // were we previewing?
            BuildPreviewGraph();
            StartPreview();
        }
        MakeMenuOptions();  // the UI choices change per device
    }

    // Set the check marks for the devices menu.
    int i;
    for(i = 0; i < NUMELMS(gcap.rgpmVideoMenu); i++) {
        if(gcap.rgpmVideoMenu[i] == NULL)
            break;
        CheckMenuItem(GetMenu(ghwndApp), 
            MENU_VDEVICE0 + i, 
            (S_OK == gcap.rgpmVideoMenu[i]->IsEqual(gcap.pmVideo)) ? MF_CHECKED : MF_UNCHECKED); 
    }

    for(i = 0; i < NUMELMS(gcap.rgpmAudioMenu); i++) {
        if(gcap.rgpmAudioMenu[i] == NULL)
            break;
        CheckMenuItem(GetMenu(ghwndApp), 
            MENU_ADEVICE0 + i, 
            (S_OK == gcap.rgpmAudioMenu[i]->IsEqual(gcap.pmAudio)) ? MF_CHECKED : MF_UNCHECKED); 
    }

    // Put the video driver name in the status bar - if the filter supports
    // IAMVideoCompression::GetInfo, that's the best way to get the name and
    // the version.  Otherwise use the name we got from device enumeration
    // as a fallback.
    if(gcap.pVC) {
        HRESULT hr = gcap.pVC->GetInfo(wachVer, &versize, wachDesc, &descsize,
            NULL, NULL, NULL, NULL);
        if(hr == S_OK) {
            // It's possible that the call succeeded without actually filling
            // in information for description and version.  If these strings
            // are empty, just display the device's friendly name.
            if(wcslen(wachDesc) && wcslen(wachVer)) {
                wsprintf(tachStatus, TEXT("%s - %s"), W2T(wachDesc), W2T(wachVer));
                statusUpdateStatus(ghwndStatus, tachStatus);
                return;
            }
        }
    }

    // Since the GetInfo method failed (or the interface did not exist),
    // display the device's friendly name.
    statusUpdateStatus(ghwndStatus, W2T(gcap.wachFriendlyName));
}

void ChooseDevices(TCHAR *szVideo, TCHAR *szAudio) 
{
    WCHAR wszVideo[1024],  wszAudio[1024];

#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, szVideo, -1, wszVideo, NUMELMS(wszVideo));
    MultiByteToWideChar(CP_ACP, 0, szAudio, -1, wszAudio, NUMELMS(wszAudio));
#else
    wcscpy(wszVideo, T2W(szVideo));
    wcscpy(wszAudio, T2W(szAudio));
#endif

    IBindCtx *lpBC;
    HRESULT hr = CreateBindCtx(0, &lpBC);
    IMoniker *pmVideo = 0, *pmAudio = 0;
    if(SUCCEEDED(hr)) {
        DWORD dwEaten;
        hr = MkParseDisplayName(lpBC, wszVideo, &dwEaten,
            &pmVideo);
        hr = MkParseDisplayName(lpBC, wszAudio, &dwEaten,
            &pmAudio);

        lpBC->Release();
    }

    // Handle the case where the video capture device used for the previous session
    // is not available now.
    BOOL bFound = FALSE;
    if(pmVideo != NULL) {
        for(int i = 0; i < NUMELMS(gcap.rgpmVideoMenu); i++) {
            if(gcap.rgpmVideoMenu[i] != NULL &&
                S_OK == gcap.rgpmVideoMenu[i]->IsEqual(pmVideo)) {
                bFound = TRUE;
                break;
            }
        }
    }

    if(!bFound) {
        if(gcap.iNumVCapDevices > 0) {
            IMonRelease(pmVideo);
            ASSERT(gcap.rgpmVideoMenu[0] != NULL);
            pmVideo = gcap.rgpmVideoMenu[0];
            pmVideo->AddRef();
        }
        else
            goto CleanUp;
    }

    ChooseDevices(pmVideo, pmAudio);

CleanUp:
    IMonRelease(pmVideo);
    IMonRelease(pmAudio);
}


// put all installed video and audio devices in the menus
//
void AddDevicesToMenu() 
{
    USES_CONVERSION;

    if(gcap.fDeviceMenuPopulated) {
        return;
    }
    gcap.fDeviceMenuPopulated = true;
    gcap.iNumVCapDevices = 0;

    UINT    uIndex = 0;
    HMENU   hMenuSub;
    HRESULT hr;
    BOOL bCheck = FALSE;

    hMenuSub = GetSubMenu(GetMenu(ghwndApp), 1);        // Devices menu

    // Clean the sub menu
    int iMenuItems = GetMenuItemCount(hMenuSub);
    if(iMenuItems == -1) {
        ErrMsg(TEXT("Error Cleaning Devices Menu"));
        return;
    }
    else if(iMenuItems > 0) {
        for(int i = 0; i < iMenuItems; i++) {
            RemoveMenu(hMenuSub, 0, MF_BYPOSITION);
        }

    } {
        for(int i = 0; i < NUMELMS(gcap.rgpmVideoMenu); i++) {
            IMonRelease(gcap.rgpmVideoMenu[i]);
        }
        for(i = 0; i < NUMELMS(gcap.rgpmAudioMenu); i++) {
            IMonRelease(gcap.rgpmAudioMenu[i]);
        }
    }


    // enumerate all video capture devices
    ICreateDevEnum *pCreateDevEnum;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Error Creating Device Enumerator"));
        return;
    }

    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
        &pEm, 0);
    if(hr != NOERROR) {
        ErrMsg(TEXT("Sorry, you have no video capture hardware"));
        goto EnumAudio;
    }
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK) {
        IPropertyBag *pBag;
        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR) {
                AppendMenu(hMenuSub, MF_STRING, MENU_VDEVICE0 + uIndex,
                    W2T(var.bstrVal));

                if(gcap.pmVideo != 0 && (S_OK == gcap.pmVideo->IsEqual(pM)))
                    bCheck = TRUE;
                CheckMenuItem(hMenuSub, 
                    MENU_VDEVICE0 + uIndex, 
                    (bCheck ? MF_CHECKED : MF_UNCHECKED));
                bCheck = FALSE;
                EnableMenuItem(hMenuSub,
                    MENU_VDEVICE0 + uIndex,
                    (gcap.fCapturing ? MF_DISABLED : MF_ENABLED));


                SysFreeString(var.bstrVal);

                ASSERT(gcap.rgpmVideoMenu[uIndex] == 0);
                gcap.rgpmVideoMenu[uIndex] = pM;
                pM->AddRef();
            }
            pBag->Release();
        }
        pM->Release();
        uIndex++;
    }
    pEm->Release();

    gcap.iNumVCapDevices = uIndex;

    // separate the video and audio devices
    AppendMenu(hMenuSub, MF_SEPARATOR, 0, NULL);

EnumAudio:

    // enumerate all audio capture devices
    uIndex = 0;
    bCheck = FALSE;

    ASSERT(pCreateDevEnum != NULL);

    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,
        &pEm, 0);
    pCreateDevEnum->Release();
    if(hr != NOERROR)
        return;
    pEm->Reset();
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK) {
        IPropertyBag *pBag;
        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR) {
                AppendMenu(hMenuSub, MF_STRING, MENU_ADEVICE0 + uIndex,
                    W2T(var.bstrVal));

                if(gcap.pmAudio != 0 && (S_OK == gcap.pmAudio->IsEqual(pM)))
                    bCheck = TRUE;
                CheckMenuItem(hMenuSub, 
                    MENU_ADEVICE0 + uIndex, 
                    (bCheck ? MF_CHECKED : MF_UNCHECKED));
                bCheck = FALSE;
                EnableMenuItem(hMenuSub,
                    MENU_ADEVICE0 + uIndex,
                    (gcap.fCapturing ? MF_DISABLED : MF_ENABLED));

                SysFreeString(var.bstrVal);

                ASSERT(gcap.rgpmAudioMenu[uIndex] == 0);
                gcap.rgpmAudioMenu[uIndex] = pM;
                pM->AddRef();                
            }
            pBag->Release();
        }
        pM->Release();
        uIndex++;
    }
    pEm->Release();
}



// let them pick a frame rate
//
void ChooseFrameRate() 
{
    double rate = gcap.FrameRate;

    DoDialog(ghwndApp, IDD_FrameRateDialog, (DLGPROC)FrameRateProc, 0);

    HRESULT hr = E_FAIL;

    // If somebody unchecks "use frame rate" it means we will no longer
    // tell the filter what frame rate to use... it will either continue
    // using the last one, or use some default, or if you bring up a dialog
    // box that has frame rate choices, it will obey them.

    // new frame rate?
    if(gcap.fUseFrameRate && gcap.FrameRate != rate) {
        if(gcap.fPreviewing)
            StopPreview();
        // now tell it what frame rate to capture at.  Just find the format it
        // is capturing with, and leave everything else alone
        if(gcap.pVSC) {
            AM_MEDIA_TYPE *pmt;
            hr = gcap.pVSC->GetFormat(&pmt);
            // DV capture does not use a VIDEOINFOHEADER
            if(hr == NOERROR) {
                if(pmt->formattype == FORMAT_VideoInfo) {
                    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
                    pvi->AvgTimePerFrame =(LONGLONG)(10000000 / gcap.FrameRate);
                    hr = gcap.pVSC->SetFormat(pmt);
                    if(hr != S_OK)
                        ErrMsg(TEXT("%x: Cannot set new frame rate"), hr);
                }
                DeleteMediaType(pmt);
            }
        }
        if(hr != NOERROR)
            ErrMsg(TEXT("Cannot set frame rate for capture"));
        if(gcap.fWantPreview)  // we were previewing
            StartPreview();
    }
}


// let them set a capture time limit
//
void ChooseTimeLimit() 
{
    DoDialog(ghwndApp, IDD_TimeLimitDialog, (DLGPROC)TimeLimitProc, 0);
}


// choose an audio capture format using ACM
//
void ChooseAudioFormat() 
{
    ACMFORMATCHOOSE cfmt;
    DWORD dwSize;
    LPWAVEFORMATEX lpwfx;
    AM_MEDIA_TYPE *pmt;

    // there's no point if we can't set a new format
    if(gcap.pASC == NULL)
        return;

    // What's the largest format size around?
    acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &dwSize);
    HRESULT hr = gcap.pASC->GetFormat(&pmt);
    if(hr != NOERROR)
        return;
    lpwfx = (LPWAVEFORMATEX)pmt->pbFormat;
    dwSize = max(dwSize, lpwfx->cbSize + sizeof(WAVEFORMATEX));

    // !!! This doesn't really map to the supported formats of the filter.
    // We should be using a property page based on IAMStreamConfig

    // Put up a dialog box initialized with the current format
    lpwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, dwSize);
    if(lpwfx) {
        CopyMemory(lpwfx, pmt->pbFormat, pmt->cbFormat);
        _fmemset(&cfmt, 0, sizeof(ACMFORMATCHOOSE));
        cfmt.cbStruct = sizeof(ACMFORMATCHOOSE);
        cfmt.fdwStyle = ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
        // show only formats we can capture
        cfmt.fdwEnum = ACM_FORMATENUMF_HARDWARE | ACM_FORMATENUMF_INPUT;
        cfmt.hwndOwner = ghwndApp;
        cfmt.pwfx = lpwfx;
        cfmt.cbwfx = dwSize;

        // we chose a new format... so give it to the capture filter
        if(!acmFormatChoose(&cfmt)) {
            if(gcap.fPreviewing)
                StopPreview();  // can't call IAMStreamConfig::SetFormat

            // while streaming
            ((CMediaType *)pmt)->SetFormat((LPBYTE)lpwfx,
                lpwfx->cbSize + sizeof(WAVEFORMATEX));
            gcap.pASC->SetFormat(pmt);  // filter will reconnect
            if(gcap.fWantPreview)
                StartPreview();
        }
        GlobalFreePtr(lpwfx) ;
    }
    DeleteMediaType(pmt);
}


/*----------------------------------------------------------------------------*\
|    AppCommand()
|
|    Process all of our WM_COMMAND messages.
\*----------------------------------------------------------------------------*/
LONG PASCAL AppCommand(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam) 
{
    HRESULT hr;
    int id = GET_WM_COMMAND_ID(wParam, lParam);
    switch(id) {
        //
        // Our about box
        //
        case MENU_ABOUT:
            DialogBox(ghInstApp, MAKEINTRESOURCE(IDD_ABOUT), hwnd, 
                (DLGPROC)AboutDlgProc);
            break;

        //
        // We want out of here!
        //
        case MENU_EXIT:
            PostMessage(hwnd,WM_CLOSE,0,0L);
            break;

        // choose a capture file
        //
        case MENU_SET_CAP_FILE:
            SetCaptureFile(hwnd);
            break;

        // pre-allocate the capture file
        //
        case MENU_ALLOC_CAP_FILE:
            AllocCaptureFile(hwnd);
            break;

        // save the capture file
        //
        case MENU_SAVE_CAP_FILE:
            SaveCaptureFile(hwnd);
            break;

        // start capturing
        //
        case MENU_START_CAP:
            if(gcap.fPreviewing)
                StopPreview();
            if(gcap.fPreviewGraphBuilt)
                TearDownGraph();
            BuildCaptureGraph();
            StartCapture();
            break;

        // toggle preview
        // 
        case MENU_PREVIEW:
            gcap.fWantPreview = !gcap.fWantPreview;
            if(gcap.fWantPreview) {
                BuildPreviewGraph();
                StartPreview();
            }
            else
                StopPreview();
            break;

        // stop capture
        //
        case MENU_STOP_CAP:
            StopCapture();
            if(gcap.fWantPreview) {
                    BuildPreviewGraph();
                    StartPreview();
            }
            break;

        // select the master stream
        //
        case MENU_NOMASTER:
            gcap.iMasterStream = -1;
            if(gcap.pConfigAviMux) {
                    hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
                    if(hr != NOERROR)
                        ErrMsg(TEXT("SetMasterStream failed!"));
            }
            break;

        case MENU_AUDIOMASTER:
            gcap.iMasterStream = 1;
            if(gcap.pConfigAviMux) {
                    hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
                    if(hr != NOERROR)
                        ErrMsg(TEXT("SetMasterStream failed!"));
            }
            break;

        case MENU_VIDEOMASTER:
            gcap.iMasterStream = 0;
            if(gcap.pConfigAviMux) {
                    hr = gcap.pConfigAviMux->SetMasterStream(gcap.iMasterStream);
                    if(hr != NOERROR)
                        ErrMsg(TEXT("SetMasterStream failed!"));
            }
            break;

        // toggle capturing audio
        case MENU_CAP_AUDIO:
            if(gcap.fPreviewing)
                StopPreview();
            gcap.fCapAudio = !gcap.fCapAudio;
            // when we capture we'll need a different graph now
            if(gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
                TearDownGraph();
            if(gcap.fWantPreview) {
                    BuildPreviewGraph();
                    StartPreview();
            }
            break;

        // toggle closed captioning
        case MENU_CAP_CC:
            if(gcap.fPreviewing)
                StopPreview();
            gcap.fCapCC = !gcap.fCapCC;
            // when we capture we'll need a different graph now
            if(gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
                TearDownGraph();
            if(gcap.fWantPreview) {
                    BuildPreviewGraph();
                    StartPreview();
            }
            break;

        // choose the audio capture format
        //
        case MENU_AUDIOFORMAT:
            ChooseAudioFormat();
            break;

        // pick a frame rate
        //
        case MENU_FRAMERATE:
            ChooseFrameRate();
            break;

        // pick a time limit
        //
        case MENU_TIMELIMIT:
            ChooseTimeLimit();
            break;

        // pick which video capture device to use
        // pick which video capture device to use
        //
        case MENU_VDEVICE0:
        case MENU_VDEVICE1:
        case MENU_VDEVICE2:
        case MENU_VDEVICE3:
        case MENU_VDEVICE4:
        case MENU_VDEVICE5:
        case MENU_VDEVICE6:
        case MENU_VDEVICE7:
        case MENU_VDEVICE8:
        case MENU_VDEVICE9:
            ChooseDevices(gcap.rgpmVideoMenu[id - MENU_VDEVICE0], gcap.pmAudio);
            break;

        // pick which audio capture device to use
        //
        case MENU_ADEVICE0:
        case MENU_ADEVICE1:
        case MENU_ADEVICE2:
        case MENU_ADEVICE3:
        case MENU_ADEVICE4:
        case MENU_ADEVICE5:
        case MENU_ADEVICE6:
        case MENU_ADEVICE7:
        case MENU_ADEVICE8:
        case MENU_ADEVICE9:
            ChooseDevices(gcap.pmVideo, gcap.rgpmAudioMenu[id - MENU_ADEVICE0]);
            break;

        // video format dialog
        //
        case MENU_DIALOG0:
        case MENU_DIALOG1:
        case MENU_DIALOG2:
        case MENU_DIALOG3:
        case MENU_DIALOG4:
        case MENU_DIALOG5:
        case MENU_DIALOG6:
        case MENU_DIALOG7:
        case MENU_DIALOG8:
        case MENU_DIALOG9:
        case MENU_DIALOGA:
        case MENU_DIALOGB:
        case MENU_DIALOGC:
        case MENU_DIALOGD:
        case MENU_DIALOGE:
        case MENU_DIALOGF:

            // they want the VfW format dialog
            if(id - MENU_DIALOG0 == gcap.iFormatDialogPos) {
                    // this dialog will not work while previewing
                    if(gcap.fWantPreview)
                        StopPreview();
                    HRESULT hrD;
                    hrD = gcap.pDlg->ShowDialog(VfwCaptureDialog_Format, ghwndApp);
                    // Oh uh!  Sometimes bringing up the FORMAT dialog can result
                    // in changing to a capture format that the current graph 
                    // can't handle.  It looks like that has happened and we'll
                    // have to rebuild the graph.
                    if(hrD == VFW_E_CANNOT_CONNECT) {
                        DbgLog((LOG_TRACE,1,TEXT("DIALOG CORRUPTED GRAPH!")));
                        TearDownGraph();    // now we need to rebuild
                        // !!! This won't work if we've left a stranded h/w codec
                    }

                    // Resize our window to be the same size that we're capturing
                    if(gcap.pVSC) {
                            AM_MEDIA_TYPE *pmt;
                            // get format being used NOW
                            hr = gcap.pVSC->GetFormat(&pmt);
                            // DV capture does not use a VIDEOINFOHEADER
                            if(hr == NOERROR) {
                                if(pmt->formattype == FORMAT_VideoInfo) {
                                    // resize our window to the new capture size
                                    ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
                                        abs(HEADER(pmt->pbFormat)->biHeight));
                                }
                                DeleteMediaType(pmt);
                            }
                    }

                    if(gcap.fWantPreview) {
                            BuildPreviewGraph();
                            StartPreview();
                    }
            }
            else if(id - MENU_DIALOG0 == gcap.iSourceDialogPos) {
                    // this dialog will not work while previewing
                    if(gcap.fWantPreview)
                        StopPreview();
                    gcap.pDlg->ShowDialog(VfwCaptureDialog_Source, ghwndApp);
                    if(gcap.fWantPreview)
                        StartPreview();
            }
            else if(id - MENU_DIALOG0 == gcap.iDisplayDialogPos) {
                    // this dialog will not work while previewing
                    if(gcap.fWantPreview)
                        StopPreview();
                    gcap.pDlg->ShowDialog(VfwCaptureDialog_Display, ghwndApp);
                    if(gcap.fWantPreview)
                        StartPreview();

                    // now the code for the new dialogs
            }
            else if(id - MENU_DIALOG0 == gcap.iVCapDialogPos) {
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&gcap.pVCap, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
            }
            else if(id - MENU_DIALOG0 == gcap.iVCapCapturePinDialogPos) {
                    // You can change this pin's output format in these dialogs.
                    // If the capture pin is already connected to somebody who's 
                    // fussy about the connection type, that may prevent using 
                    // this dialog(!) because the filter it's connected to might not
                    // allow reconnecting to a new format. (EG: you switch from RGB
                    // to some compressed type, and need to pull in a decoder)
                    // I need to tear down the graph downstream of the
                    // capture filter before bringing up these dialogs.
                    // In any case, the graph must be STOPPED when calling them.
                    if(gcap.fWantPreview)
                        StopPreview();  // make sure graph is stopped

                    // The capture pin that we are trying to set the format on is connected if
                    // one of these variable is set to TRUE. The pin should be disconnected for
                    // the dialog to work properly.
                    if(gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt) {
                        DbgLog((LOG_TRACE,1,TEXT("Tear down graph for dialog")));
                        TearDownGraph();    // graph could prevent dialog working
                    }
                    IAMStreamConfig *pSC;
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Interleaved, gcap.pVCap,
                        IID_IAMStreamConfig, (void **)&pSC);
                    if(hr != NOERROR)
                        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                            &MEDIATYPE_Video, gcap.pVCap,
                            IID_IAMStreamConfig, (void **)&pSC);
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                            hr = pSpec->GetPages(&cauuid);
                            hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                                (IUnknown **)&pSC, cauuid.cElems,
                                (GUID *)cauuid.pElems, 0, 0, NULL);

                            // !!! What if changing output formats couldn't reconnect
                            // and the graph is broken?  Shouldn't be possible...

                            if(gcap.pVSC) {
                                AM_MEDIA_TYPE *pmt;
                                // get format being used NOW
                                hr = gcap.pVSC->GetFormat(&pmt);
                                // DV capture does not use a VIDEOINFOHEADER
                                if(hr == NOERROR) {
                                    if(pmt->formattype == FORMAT_VideoInfo) {
                                        // resize our window to the new capture size
                                        ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
                                            abs(HEADER(pmt->pbFormat)->biHeight));
                                    }
                                    DeleteMediaType(pmt);
                                }
                            }

                            CoTaskMemFree(cauuid.pElems);
                            pSpec->Release();
                    }
                    pSC->Release();
                    if(gcap.fWantPreview) {
                            BuildPreviewGraph();
                            StartPreview();
                    }
            }
            else if(id - MENU_DIALOG0 == gcap.iVCapPreviewPinDialogPos) {
                    // this dialog may not work if the preview pin is connected
                    // already, because the downstream filter may reject a format
                    // change, so we better kill the graph. (EG: We switch from 
                    // capturing RGB to some compressed fmt, and need to pull in
                    // a decompressor)
                    if(gcap.fWantPreview) {
                        StopPreview();
                        TearDownGraph();
                    }

                    IAMStreamConfig *pSC;

                    // This dialog changes the preview format, so it might affect
                    // the format being drawn.  Our app's window size is taken
                    // from the size of the capture pin's video, not the preview
                    // pin, so changing that here won't have any effect. All in all,
                    // this probably won't be a terribly useful dialog in this app.
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
                        &MEDIATYPE_Interleaved, gcap.pVCap,
                        IID_IAMStreamConfig, (void **)&pSC);
                    if(hr != NOERROR)
                        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
                            &MEDIATYPE_Video, gcap.pVCap,
                            IID_IAMStreamConfig, (void **)&pSC);

                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                            hr = pSpec->GetPages(&cauuid);
                            hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                                (IUnknown **)&pSC, cauuid.cElems,
                                (GUID *)cauuid.pElems, 0, 0, NULL);
                            CoTaskMemFree(cauuid.pElems);
                            pSpec->Release();
                    }
                    pSC->Release();
                    if(gcap.fWantPreview) {
                            BuildPreviewGraph();
                            StartPreview();
                    }
            }
            else if(id - MENU_DIALOG0 == gcap.iVCrossbarDialogPos) {
                    IAMCrossbar *pX;
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Interleaved, gcap.pVCap,
                        IID_IAMCrossbar, (void **)&pX);
                    if(hr != NOERROR)
                        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                            &MEDIATYPE_Video, gcap.pVCap,
                            IID_IAMCrossbar, (void **)&pX);
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pX->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&pX, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
                    pX->Release();
            }
            else if(id - MENU_DIALOG0 == gcap.iTVTunerDialogPos) {
                    IAMTVTuner *pTV;
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Interleaved, gcap.pVCap,
                        IID_IAMTVTuner, (void **)&pTV);
                    if(hr != NOERROR)
                        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                            &MEDIATYPE_Video, gcap.pVCap,
                            IID_IAMTVTuner, (void **)&pTV);
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pTV->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&pTV, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
                    pTV->Release();
            }
            else if(id - MENU_DIALOG0 == gcap.iACapDialogPos) {
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = gcap.pACap->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&gcap.pACap, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
            }
            else if(id - MENU_DIALOG0 == gcap.iACapCapturePinDialogPos) {
                    // this dialog will not work while previewing - it might change
                    // the output format!
                    if(gcap.fWantPreview)
                        StopPreview();
                    IAMStreamConfig *pSC;
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Audio, gcap.pACap,
                        IID_IAMStreamConfig, (void **)&pSC);
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&pSC, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
                    pSC->Release();
                    if(gcap.fWantPreview)
                        StartPreview();
            }
            else if(id - MENU_DIALOG0 == gcap.iACrossbarDialogPos) {
                    IAMCrossbar *pX, *pX2;
                    IBaseFilter *pXF;
                    // we could use better error checking here... I'm assuming
                    // this won't fail
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Interleaved, gcap.pVCap,
                        IID_IAMCrossbar, (void **)&pX);
                    if(hr != NOERROR)
                        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                            &MEDIATYPE_Video, gcap.pVCap,
                            IID_IAMCrossbar, (void **)&pX);
                    hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
                    hr = gcap.pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL,
                        pXF, IID_IAMCrossbar, (void **)&pX2);
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&pX2, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
                    pX2->Release();
                    pXF->Release();
                    pX->Release();
            }
            else if(id - MENU_DIALOG0 == gcap.iTVAudioDialogPos) {
                    IAMTVAudio *pTVA;
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Audio, gcap.pACap,
                        IID_IAMTVAudio, (void **)&pTVA);
                    ISpecifyPropertyPages *pSpec;
                    CAUUID cauuid;
                    hr = pTVA->QueryInterface(IID_ISpecifyPropertyPages,
                        (void **)&pSpec);
                    if(hr == S_OK) {
                        hr = pSpec->GetPages(&cauuid);
                        hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                            (IUnknown **)&pTVA, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                        pSpec->Release();
                    }
                    pTVA->Release();
            }
            else if(((id - MENU_DIALOG0) >  gcap.iVideoInputMenuPos) && 
                (id - MENU_DIALOG0) <= gcap.iVideoInputMenuPos + gcap.NumberOfVideoInputs) {
                    // Remove existing checks
                    for(int j = 0; j < gcap.NumberOfVideoInputs; j++) {
                        CheckMenuItem(gcap.hMenuPopup, j, MF_BYPOSITION | 
                            ((j == (id - MENU_DIALOG0) - gcap.iVideoInputMenuPos - 1) ?
                            MF_CHECKED : MF_UNCHECKED )); 
                    }

                    if(gcap.pCrossbar) {
                            EXECUTE_ASSERT(S_OK == gcap.pCrossbar->SetInputIndex((id - MENU_DIALOG0) - gcap.iVideoInputMenuPos - 1));
                    }
            }

            break;

    }
    return 0L;
}


/*----------------------------------------------------------------------------*\
|   ErrMsg - Opens a Message box with a error message in it.  The user can     |
|            select the OK button to continue                                  |
\*----------------------------------------------------------------------------*/
int ErrMsg(LPTSTR sz,...) 
{
    static TCHAR tach[2000];
    va_list va;

    va_start(va, sz);
    wvsprintf(tach, sz, va);
    va_end(va);
    MessageBox(ghwndApp, tach, NULL, MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);
    return FALSE;
}


/* AboutDlgProc()
 *
 * Dialog Procedure for the "about" dialog box.
 *
 */

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch(msg) {
        case WM_COMMAND:
            EndDialog(hwnd, TRUE);
            return TRUE;
        case WM_INITDIALOG:
            return TRUE;
    }
    return FALSE;
}


// pre-allocate the capture file
//
BOOL AllocCaptureFile(HWND hWnd) 
{
    USES_CONVERSION;

    // we'll get into an infinite loop in the dlg proc setting a value
    if(gcap.szCaptureFile[0] == 0)
        return FALSE;

    /*
    * show the allocate file space dialog to encourage
    * the user to pre-allocate space
    */
    if(DoDialog(hWnd, IDD_AllocCapFileSpace, (DLGPROC)AllocCapFileProc, 0)) {

        // ensure repaint after dismissing dialog before
        // possibly lengthy operation
        UpdateWindow(ghwndApp);

        // User has hit OK. Alloc requested capture file space
        BOOL f = MakeBuilder();
        if(!f)
            return FALSE;
        if(gcap.pBuilder->AllocCapFile(T2W(gcap.szCaptureFile),
            (DWORDLONG)gcap.wCapFileSize * 1024L * 1024L) != NOERROR) {
            MessageBox(ghwndApp, TEXT("Error"),
                TEXT("Failed to pre-allocate capture file space"),
                MB_OK | MB_ICONEXCLAMATION);
            return FALSE;
        }
        return TRUE;
    }
    else {
        return FALSE;
    }
}


/*
 * Put up the open file dialog
 */
BOOL OpenFileDialog(HWND hWnd, LPTSTR pszName, int cb) 
{
    OPENFILENAME ofn;
    LPTSTR p;
    TCHAR        szFileName[_MAX_PATH];
    TCHAR        szBuffer[_MAX_PATH] ;

    if(pszName == NULL || cb <= 0)
        return FALSE;

    // start with capture file as current file name
    szFileName[0] = 0;
    lstrcpy(szFileName, gcap.szCaptureFile);

    // Get just the path info
    // Terminate the full path at the last backslash
    lstrcpy(szBuffer, szFileName);
    for(p = szBuffer + lstrlen(szBuffer); p > szBuffer; p--) {
        if(*p == '\\') {
            *(p+1) = '\0';
            break;
        }
    }

    _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = hWnd ;
    ofn.lpstrFilter = TEXT("Microsoft AVI\0*.avi\0\0");
    ofn.nFilterIndex = 0 ;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) ;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrTitle = TEXT("Set Capture File");
    ofn.nMaxFileTitle = 0 ;
    ofn.lpstrInitialDir = szBuffer;
    ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST ;

    if(GetOpenFileName(&ofn)) {
        // We have a capture file name
        lstrcpyn(pszName, szFileName, cb);
        return TRUE;
    }
    else {
        return FALSE;
    }
}


/*
 * Put up a dialog to allow the user to select a capture file.
 */
BOOL SetCaptureFile(HWND hWnd) 
{
    USES_CONVERSION;

    if(OpenFileDialog(hWnd, gcap.szCaptureFile, _MAX_PATH)) {
        OFSTRUCT os;

        // We have a capture file name

        /*
        * if this is a new file, then invite the user to
        * allocate some space
        */
#ifdef UNICODE
        // Convert Multibyte string to ANSI
        char szCaptureFile[STR_MAX_LENGTH];
        int rc = WideCharToMultiByte(CP_ACP, 0, gcap.szCaptureFile, -1, 
            szCaptureFile, STR_MAX_LENGTH, NULL, NULL);
#else
        TCHAR *szCaptureFile = gcap.szCaptureFile;
#endif

        if(OpenFile(szCaptureFile, &os, OF_EXIST) == HFILE_ERROR) {

            // bring up dialog, and set new file size
            BOOL f = AllocCaptureFile(hWnd);
            if(!f)
                return FALSE;
        }
    }
    else {
        return FALSE;
    }

    SetAppCaption();    // new a new app caption

    // tell the file writer to use the new filename
    if(gcap.pSink) {
        gcap.pSink->SetFileName(T2W(gcap.szCaptureFile), NULL);
    }

    return TRUE;
}


/*
 * Put up a dialog to allow the user to save the contents of the capture file
 * elsewhere
 */
BOOL SaveCaptureFile(HWND hWnd) 
{
    USES_CONVERSION;
    HRESULT hr;
    TCHAR tachDstFile[_MAX_PATH];

    if(gcap.pBuilder == NULL)
        return FALSE;

    if(OpenFileDialog(hWnd, tachDstFile, _MAX_PATH)) {

        // We have a capture file name
        statusUpdateStatus(ghwndStatus, TEXT("Saving capture file - please wait..."));

        // we need our own graph builder because the main one might not exist
        ICaptureGraphBuilder2 *pBuilder;
        hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2,
            NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder2,
            (void **)&pBuilder);

        if(hr == NOERROR) {
            // allow the user to press ESC to abort... ask for progress
            CProgress *pProg = new CProgress(TEXT(""), NULL, &hr);
            IAMCopyCaptureFileProgress *pIProg = NULL;
            if(pProg) {
                hr = pProg->QueryInterface(IID_IAMCopyCaptureFileProgress,
                    (void **)&pIProg);
            }
            hr = pBuilder->CopyCaptureFile(T2W(gcap.szCaptureFile),
                T2W(tachDstFile), TRUE, pIProg);
            if(pIProg)
                pIProg->Release();
            pBuilder->Release();
        }

        if(hr == S_OK)
            statusUpdateStatus(ghwndStatus, TEXT("Capture file saved"));
        else if(hr == S_FALSE)
            statusUpdateStatus(ghwndStatus, TEXT("Capture file save aborted"));
        else
            statusUpdateStatus(ghwndStatus, TEXT("Capture file save ERROR"));
        return (hr == NOERROR ? TRUE : FALSE); 

    }
    else {
        return TRUE;    // they cancelled or something
    }
}

// brings up a dialog box
//
int DoDialog(HWND hwndParent, int DialogID, DLGPROC fnDialog, long lParam) 
{
    DLGPROC fn;
    int result;

    fn = (DLGPROC)MakeProcInstance(fnDialog, ghInstApp);
    result = (int) DialogBoxParam(ghInstApp,
        MAKEINTRESOURCE(DialogID),
        hwndParent,
        fn,
        lParam);
    FreeProcInstance(fn);

    return result;
}


//
// GetFreeDiskSpace: Function to Measure Available Disk Space
//
static long GetFreeDiskSpaceInKB(LPTSTR pFile) 
{
    DWORD dwFreeClusters, dwBytesPerSector, dwSectorsPerCluster, dwClusters;
    TCHAR RootName[MAX_PATH];
    LPTSTR ptmp;    //required arg
    ULARGE_INTEGER ulA, ulB, ulFreeBytes;

    // need to find path for root directory on drive containing
    // this file.

    GetFullPathName(pFile, sizeof(RootName)/sizeof(RootName[0]), RootName, &ptmp);

    // truncate this to the name of the root directory (how tedious)
    if(RootName[0] == '\\' && RootName[1] == '\\') {

        // path begins with  \\server\share\path so skip the first
        // three backslashes
        ptmp = &RootName[2];
        while(*ptmp && (*ptmp != '\\')) {
            ptmp++;
        }
        if(*ptmp) {
            // advance past the third backslash
            ptmp++;
        }
    }
    else {
        // path must be drv:\path
        ptmp = RootName;
    }

    // find next backslash and put a null after it
    while(*ptmp && (*ptmp != '\\')) {
        ptmp++;
    }
    // found a backslash ?
    if(*ptmp) {
        // skip it and insert null
        ptmp++;
        *ptmp = '\0';
    }

    // the only real way of finding out free disk space is calling
    // GetDiskFreeSpaceExA, but it doesn't exist on Win95

    HINSTANCE h = LoadLibrary(TEXT("kernel32.dll"));
    if(h) {
        typedef BOOL(WINAPI *MyFunc)(LPCTSTR RootName, PULARGE_INTEGER pulA, PULARGE_INTEGER pulB, PULARGE_INTEGER pulFreeBytes);

#ifdef UNICODE
        MyFunc pfnGetDiskFreeSpaceEx = (MyFunc)GetProcAddress(h, "GetDiskFreeSpaceExW");
#else
        MyFunc pfnGetDiskFreeSpaceEx = (MyFunc)GetProcAddress(h, "GetDiskFreeSpaceExA");
#endif
        FreeLibrary(h);

        if(pfnGetDiskFreeSpaceEx) {
            if(!pfnGetDiskFreeSpaceEx(RootName, &ulA, &ulB, &ulFreeBytes))
                return -1;
            else
                return (long)(ulFreeBytes.QuadPart / 1024);
        }
    }

    if(!GetDiskFreeSpace(RootName, &dwSectorsPerCluster, &dwBytesPerSector,
        &dwFreeClusters, &dwClusters))
        return (-1);
    else
        return(MulDiv(dwSectorsPerCluster * dwBytesPerSector,
            dwFreeClusters,
            1024));
}



// AllocCapFileProc: Capture file Space Allocation Dialog Box Procedure
//
int FAR PASCAL AllocCapFileProc(HWND hDlg, UINT Message, UINT wParam, LONG lParam) 
{
    static int nFreeMBs = 0 ;

    switch(Message) {
        case WM_INITDIALOG: {
                DWORDLONG        dwlFileSize = 0;
                long             lFreeSpaceInKB;

                // Get current capture file name and measure its size
                dwlFileSize = GetSize(gcap.szCaptureFile);

                // Get free disk space and add current capture file size to that.
                // Convert the available space to MBs.
                if((lFreeSpaceInKB = GetFreeDiskSpaceInKB(gcap.szCaptureFile)) != -1L) {
                    lFreeSpaceInKB += (long)(dwlFileSize / 1024);
                    nFreeMBs = lFreeSpaceInKB / 1024 ;
                    SetDlgItemInt(hDlg, IDD_SetCapFileFree, nFreeMBs, TRUE) ;
                }
                else {
                        EnableWindow(GetDlgItem(hDlg, IDD_SetCapFileFree), FALSE);
                }

                gcap.wCapFileSize = (WORD) (dwlFileSize / (1024L * 1024L));

                SetDlgItemInt(hDlg, IDD_SetCapFileSize, gcap.wCapFileSize, TRUE) ;
                return TRUE ;
            }

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK : {
                        int         iCapFileSize ;

                        iCapFileSize = (int) GetDlgItemInt(hDlg, IDD_SetCapFileSize, NULL, TRUE) ;
                        if(iCapFileSize <= 0 || iCapFileSize > nFreeMBs) {
                            // You are asking for more than we have !! Sorry, ...
                            SetDlgItemInt(hDlg, IDD_SetCapFileSize, iCapFileSize, TRUE) ;
                            SetFocus(GetDlgItem(hDlg, IDD_SetCapFileSize)) ;
                            MessageBeep(MB_ICONEXCLAMATION) ;
                            return FALSE ;
                        }
                        gcap.wCapFileSize = (WORD)iCapFileSize ;

                        EndDialog(hDlg, TRUE) ;
                        return TRUE ;
                    }

                case IDCANCEL :
                    EndDialog(hDlg, FALSE) ;
                    return TRUE ;

                case IDD_SetCapFileSize: {
                            long l;
                            BOOL bchanged;
                            TCHAR tachBuffer[21];

                            // check that entered size is a valid number
                            GetDlgItemText(hDlg, IDD_SetCapFileSize, tachBuffer,
                                sizeof(tachBuffer)/sizeof(tachBuffer[0]));
                            l = _ttol(tachBuffer);
                            bchanged = FALSE;
                            if(l < 1) {
                                l = 1;
                                bchanged = TRUE;
                                // don't infinite loop if there's < 1 Meg free
                            }
                            else if(l > nFreeMBs && nFreeMBs > 0) {
                                    l = nFreeMBs;
                                    bchanged = TRUE;
                            }
                            else {
                                    // make sure there are no non-digit chars
                                    // atol() will ignore trailing non-digit characters
                                    int c = 0;
                                    while(tachBuffer[c]) {
                                        if(IsCharAlpha(tachBuffer[c]) ||
                                            !IsCharAlphaNumeric(tachBuffer[c])) {

                                            // string contains non-digit chars - reset
                                            l = 1;
                                            bchanged = TRUE;
                                            break;
                                        }
                                        c++;
                                    }
                            }
                            if(bchanged) {
                                    wsprintf(tachBuffer, TEXT("%ld"), l);
                                    SetDlgItemText(hDlg, IDD_SetCapFileSize, tachBuffer);
                            }
                            break;
                    }
            }
            break;
    }

    return FALSE ;
}


//
// FrameRateProc: Choose a frame rate
//
int FAR PASCAL FrameRateProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam) 
{
    TCHAR  tach[32];
    USES_CONVERSION;

    switch(msg) {
        case WM_INITDIALOG:
            /* put the current frame rate in the box */
            wsprintf(tach, TEXT("%d"), (int) gcap.FrameRate);
            SetDlgItemText(hwnd, IDC_FRAMERATE, tach);
            CheckDlgButton(hwnd, IDC_USEFRAMERATE, gcap.fUseFrameRate);
            break;

        case WM_COMMAND:
            switch(wParam) {
            case IDCANCEL:
                EndDialog(hwnd, FALSE);
                break;

            case IDOK:
                /* get the new frame rate */
                GetDlgItemText(hwnd, IDC_FRAMERATE, tach, sizeof(tach)/sizeof(tach[0]));

#ifdef UNICODE
                int rc;

                // Convert Multibyte string to ANSI
                char szANSI[STR_MAX_LENGTH];
                rc = WideCharToMultiByte(CP_ACP, 0, tach, -1, szANSI, 
                    STR_MAX_LENGTH, NULL, NULL);
                double frameRate = atof(szANSI);
#else
                double frameRate = atof(tach);
#endif

                if(frameRate <= 0.) {
                    ErrMsg(TEXT("Invalid frame rate."));
                    break;
                }
                else
                    gcap.FrameRate = frameRate;

                gcap.fUseFrameRate = IsDlgButtonChecked(hwnd, IDC_USEFRAMERATE);
                EndDialog(hwnd, TRUE);
                break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


//
// TimeLimitProc: Choose a capture time limit
//
int FAR PASCAL TimeLimitProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam) 
{
    TCHAR   tach[32];
    DWORD   dwTimeLimit;

    switch(msg) {
        case WM_INITDIALOG:
            /* put the current time limit info in the boxes */
            wsprintf(tach, TEXT("%d"), gcap.dwTimeLimit);
            SetDlgItemText(hwnd, IDC_TIMELIMIT, tach);
            CheckDlgButton(hwnd, IDC_USETIMELIMIT, gcap.fUseTimeLimit);
            break;

        case WM_COMMAND:
            switch(wParam) {
            case IDCANCEL:
                EndDialog(hwnd, FALSE);
                break;

            case IDOK:
                /* get the new time limit */
                dwTimeLimit = GetDlgItemInt(hwnd, IDC_TIMELIMIT, NULL, FALSE);
                gcap.dwTimeLimit = dwTimeLimit;
                gcap.fUseTimeLimit = IsDlgButtonChecked(hwnd, IDC_USETIMELIMIT);
                EndDialog(hwnd, TRUE);
                break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


//
// PressAKeyProc: Press OK to capture
//
int FAR PASCAL PressAKeyProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam) 
{
    TCHAR tach[_MAX_PATH];

    switch(msg) {
        case WM_INITDIALOG:
            /* set the current file name in the box */
            wsprintf(tach, TEXT("%s"), gcap.szCaptureFile);
            SetDlgItemText(hwnd, IDC_CAPFILENAME, tach);
            break;

        case WM_COMMAND:
            switch(wParam) {
            case IDCANCEL:
                EndDialog(hwnd, FALSE);
                break;

            case IDOK:
                EndDialog(hwnd, TRUE);
                break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

DWORDLONG GetSize(LPCTSTR tach) 
{
    HANDLE hFile = CreateFile(tach, GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    DWORD dwSizeHigh;
    DWORD dwSizeLow = GetFileSize(hFile, &dwSizeHigh);

    DWORDLONG dwlSize = dwSizeLow + ((DWORDLONG)dwSizeHigh << 32);

    if(!CloseHandle(hFile)) {
        dwlSize = 0;
    }

    return dwlSize;
}

void OnClose() 
{
    // Unregister device notifications
    if(ghDevNotify != NULL) {
        ASSERT(gpUnregisterDeviceNotification);
        gpUnregisterDeviceNotification(ghDevNotify);
        ghDevNotify = NULL;
    }

    StopPreview();
    StopCapture();
    TearDownGraph();
    FreeCapFilters();

    // store current settings in win.ini for next time
    WriteProfileString(TEXT("annie"), TEXT("CaptureFile"),
        gcap.szCaptureFile);

    TCHAR *tach = NULL;
    WCHAR *szDisplayName = NULL;
    szDisplayName = 0;
    if(gcap.pmVideo) {
        if(SUCCEEDED(gcap.pmVideo->GetDisplayName(0, 0, &szDisplayName))) {   
            if(szDisplayName) {
                tach = new TCHAR [wcslen(szDisplayName)+1];
                if(tach == NULL)
                    return;

                wsprintf(tach, TEXT("%S"), szDisplayName);
                CoTaskMemFree(szDisplayName);
            }

        }
    }
    WriteProfileString(TEXT("annie"), TEXT("VideoDevice2"), tach ? tach : TEXT("") );
    delete(tach);
    tach = NULL;

    szDisplayName = 0;
    if(gcap.pmAudio) {
        if(SUCCEEDED(gcap.pmAudio->GetDisplayName(0, 0, &szDisplayName))) {
            if(szDisplayName) {
                tach = new TCHAR [wcslen(szDisplayName)+1];
                if(tach == NULL)
                    return;

                wsprintf(tach, TEXT("%S"), szDisplayName);
                CoTaskMemFree(szDisplayName);
            }

        }
    }

    if(tach == NULL) {
        tach = new TCHAR [120];
        wsprintf(tach, TEXT("%S"), L"");

    }

    WriteProfileString(TEXT("annie"), TEXT("AudioDevice2"), tach);

    wsprintf(tach, TEXT("%d"), (int)(10000000 / gcap.FrameRate));
    WriteProfileString(TEXT("annie"), TEXT("FrameRate"), tach);
    wsprintf(tach, TEXT("%d"), gcap.fUseFrameRate);
    WriteProfileString(TEXT("annie"), TEXT("UseFrameRate"), tach);
    wsprintf(tach, TEXT("%d"), gcap.fCapAudio);
    WriteProfileString(TEXT("annie"), TEXT("CaptureAudio"), tach);
    wsprintf(tach, TEXT("%d"), gcap.fCapCC);
    WriteProfileString(TEXT("annie"), TEXT("CaptureCC"), tach);
    wsprintf(tach, TEXT("%d"), gcap.fWantPreview);
    WriteProfileString(TEXT("annie"), TEXT("WantPreview"), tach);
    wsprintf(tach, TEXT("%d"), gcap.iMasterStream);
    WriteProfileString(TEXT("annie"), TEXT("MasterStream"), tach);
    wsprintf(tach, TEXT("%d"), gcap.fUseTimeLimit);
    WriteProfileString(TEXT("annie"), TEXT("UseTimeLimit"), tach);
    wsprintf(tach, TEXT("%d"), gcap.dwTimeLimit);
    WriteProfileString(TEXT("annie"), TEXT("TimeLimit"), tach);
    delete(tach);
}



// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}

// Removes a filter graph from the Running Object Table
void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}
