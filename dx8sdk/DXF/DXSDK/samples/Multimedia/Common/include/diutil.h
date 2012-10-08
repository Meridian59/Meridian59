//-----------------------------------------------------------------------------
// File: DIUtil.h
//
// Desc: DirectInput support using action mapping
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef DIUTIL_H
#define DIUTIL_H
#include <dinput.h>




//-----------------------------------------------------------------------------
// Name: class CInputDeviceManager
// Desc: Input device manager using DX8 action mapping
//-----------------------------------------------------------------------------
class CInputDeviceManager
{
public:
    struct DeviceInfo
    {
        LPDIRECTINPUTDEVICE8 pdidDevice;
        LPVOID               pParam;
    };

    typedef HRESULT (CALLBACK *LPDIMANAGERCALLBACK)(CInputDeviceManager::DeviceInfo* pDeviceInfo, const DIDEVICEINSTANCE* pdidi, LPVOID);

private:
    BOOL                    m_bCleanupCOM;
    HWND                    m_hWnd;
    TCHAR*                  m_strUserName;

    LPDIRECTINPUT8          m_pDI;
    DeviceInfo*             m_pDevices;
    DWORD                   m_dwMaxDevices;
    DWORD                   m_dwNumDevices;
    DIACTIONFORMAT          m_diaf;

    LPDIMANAGERCALLBACK  m_AddDeviceCallback;
    LPVOID               m_AddDeviceCallbackParam;

public:
    // Device control
    HRESULT AddDevice( const DIDEVICEINSTANCE* pdidi, LPDIRECTINPUTDEVICE8 pdidDevice );
    HRESULT GetDevices( DeviceInfo** ppDeviceInfo, DWORD* pdwNumDevices );
    HRESULT ConfigureDevices( HWND hWnd, IUnknown* pSurface, VOID* pCallback, DWORD dwFlags, LPVOID pvCBParam );
    VOID UnacquireDevices();
    VOID SetFocus( HWND hWnd );

    // Construction
    HRESULT SetActionFormat( DIACTIONFORMAT& diaf, BOOL bReenumerate );
    HRESULT Create( HWND hWnd, TCHAR* strUserName, DIACTIONFORMAT& diaf, LPDIMANAGERCALLBACK AddDeviceCallback, LPVOID pCallbackParam );

    CInputDeviceManager();
    ~CInputDeviceManager();
};

#endif


