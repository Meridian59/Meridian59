//------------------------------------------------------------------------------
// File: CPPVideoControl.cpp 
//
// Desc: Implementation of WinMain
//       for the Windows XP MSVidCtl C++ sample
//
// Copyright (c) 2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f CPPVideoControlps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <commctrl.h>

#include "CPPVideoControl.h"
#include "CPPVideoControl_i.c"
#include "CompositeControl.h"

// Constants
const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 2000;   // time to wait for threads to finish up

// Local prototypes
BOOL CheckVersion(void);


// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG lock = CComModule::Unlock();
    if (lock == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return lock;
}

//Monitors the shutdown event
void CExeModule::MonitorShutdown()
{
    DWORD dwWait=0;

    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);

        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);

        } while (dwWait == WAIT_OBJECT_0);

        // timed out
        if (!bActivity && m_nLockCnt == 0) // if no activity let's really quit
        {
#if _WIN32_WINNT >= 0x0400 && defined(_ATL_FREE_THREADED)
            CoSuspendClassObjects();
            if (!bActivity && m_nLockCnt == 0)
#endif
                break;
        }
    }

    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;

    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CompositeControl, CCompositeControl)
END_OBJECT_MAP()


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{
    // This sample requires Windows XP.  If running on a downlevel system,
    // exit gracefully.
    if (!CheckVersion())
        return FALSE;

    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

#if _WIN32_WINNT >= 0x0400 && defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif

    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, &LIBID_CPPVIDEOCONTROLLib);
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_CPPVideoControl, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_CPPVideoControl, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {
        _Module.StartMonitor();
#if _WIN32_WINNT >= 0x0400 && defined(_ATL_FREE_THREADED)
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
                    REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
        _ASSERTE(SUCCEEDED(hRes));
        hRes = CoResumeClassObjects();
#else
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE);
#endif
        _ASSERTE(SUCCEEDED(hRes));
        
        //Create the window and have our composite control (with msvidctl) appear
        AtlAxWinInit();
        InitCommonControls();

        HWND hWnd = ::CreateWindow(TEXT("AtlAxWin"), 
                    TEXT("CPPVideoControl.CompositeControl"),
                    NULL,
                    CW_USEDEFAULT, CW_USEDEFAULT,   // x,y
                    300, 350,               // width, height
                    NULL, NULL,             // parent, menu
                    ::GetModuleHandle(NULL), 
                    NULL);
       _ASSERTE(hWnd != NULL);
        hWnd = hWnd;    // Prevent C4189 (variable initialized but not used)

        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
        {
            TranslateMessage( &msg );
            DispatchMessage(&msg);
        }

        _Module.RevokeClassObjects();
        Sleep(dwPause); //wait for any threads to finish
    }

    _Module.Term();
    CoUninitialize();
    return nRet;
}

//
//  called on a WM_CLOSE & WM_SYSCOMMAND (SC_CLOSE) message.  If appropriate, save settings
//  and then destroy the window
//
LRESULT CCompositeControl::OnExit(WORD /*wNotifyCode*/, WORD /* wID */, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SAFE_RELEASE(m_pATSCTuningSpace);
    SAFE_RELEASE(m_pTuningSpaceContainer);
    SAFE_RELEASE(m_pATSCLocator);
    SAFE_RELEASE(m_pMSVidCtl);

    DestroyWindow();
    PostQuitMessage(0);
    return 0;
}

//
//  the user clicks on the system menu
//
LRESULT CCompositeControl::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE; // always do default processing

    if (wParam == SC_CLOSE)
        OnExit(NULL, NULL, NULL, bHandled);
        
    return 0;
}


BOOL CheckVersion( void ) 
{
    OSVERSIONINFO osverinfo={0};
    osverinfo.dwOSVersionInfoSize = sizeof(osverinfo);

    if(GetVersionEx(&osverinfo)) {

        // Windows XP's major version is 5 and its' minor version is 1.
        if((osverinfo.dwMajorVersion > 5) || 
            ( (osverinfo.dwMajorVersion == 5) && (osverinfo.dwMinorVersion >= 1) )) {
            return TRUE;
        }
    }

    MessageBox(NULL, 
        TEXT("This application requires the Microsoft Video Control,\r\n")
        TEXT("which is present only on Windows XP.\r\n\r\n")
        TEXT("This sample will now exit."),
        TEXT("Microsoft Video Control is required"), MB_OK);

    return FALSE;
}
