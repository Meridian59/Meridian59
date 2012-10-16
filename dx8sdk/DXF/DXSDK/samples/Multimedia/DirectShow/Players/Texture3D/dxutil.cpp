//-----------------------------------------------------------------------------
// File: DXUtil.cpp
//
// Desc: Shortcut macros and functions for using DX objects
//
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#include <stdio.h> 
#include <stdarg.h>
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath()
// Desc: Returns the DirectX SDK media path
//-----------------------------------------------------------------------------
const TCHAR* DXUtil_GetDXSDKMediaPath()
{
    static TCHAR strNull[2] = _T("");
    static TCHAR strPath[MAX_PATH];
    DWORD dwType;
    DWORD dwSize = MAX_PATH;
    HKEY  hKey;

    // Open the appropriate registry key
    LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                _T("Software\\Microsoft\\DirectX SDK"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != lResult )
        return strNull;

    lResult = RegQueryValueEx( hKey, _T("DX81SDK Samples Path"), NULL,
                              &dwType, (BYTE*)strPath, &dwSize );
    RegCloseKey( hKey );

    if( ERROR_SUCCESS != lResult )
        return strNull;

    _tcscat( strPath, _T("\\Media\\") );

    return strPath;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_FindMediaFile()
// Desc: Returns a valid path to a DXSDK media file
//-----------------------------------------------------------------------------
HRESULT DXUtil_FindMediaFile( TCHAR* strPath, TCHAR* strFilename )
{
    HANDLE file;

    if( NULL==strFilename || NULL==strPath )
        return E_INVALIDARG;

    // Check if the file exists in the current directory
    _tcscpy( strPath, strFilename );

    file = CreateFile( strPath, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, 0, NULL );
    if( INVALID_HANDLE_VALUE != file )
    {
        CloseHandle( file );
        return S_OK;
    }
    
    // Check if the file exists in the current directory
    _stprintf( strPath, _T("%s%s"), DXUtil_GetDXSDKMediaPath(), strFilename );

    file = CreateFile( strPath, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, 0, NULL );
    if( INVALID_HANDLE_VALUE != file )
    {
        CloseHandle( file );
        return S_OK;
    }

    // On failure, just return the file as the path
    _tcscpy( strPath, strFilename );
    return E_FAIL;
}

