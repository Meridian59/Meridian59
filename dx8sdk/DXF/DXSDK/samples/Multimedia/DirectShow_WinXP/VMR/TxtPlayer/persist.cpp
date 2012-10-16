//------------------------------------------------------------------------------
// File: persist.cpp
//
// Desc: DirectShow sample code
//       - State persistence helper functions
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <atlbase.h>
#include <atlconv.cpp>
#include <mmreg.h>

#include "project.h"

// Global data
RECENTFILES aRecentFiles[MAX_RECENT_FILES];
int         nRecentFiles;

static TCHAR cszWindow[] = TEXT("Window");
static TCHAR cszAppKey[] = TEXT("Software\\Microsoft\\Multimedia Tools\\VMRTxtPlayer");

const int CX_DEFAULT = 400;     /* Default window width */
const int CY_DEFAULT = 400;     /* Default window height */


/******************************Public*Routine******************************\
* GetAppKey
*
\**************************************************************************/
HKEY
GetAppKey(
    BOOL fCreate
    )
{
    HKEY hKey = 0;

    if(fCreate)
    {
        if(RegCreateKey(HKEY_CURRENT_USER, cszAppKey, &hKey) == ERROR_SUCCESS)
            return hKey;
    }
    else
    {
        if(RegOpenKey(HKEY_CURRENT_USER, cszAppKey, &hKey) == ERROR_SUCCESS)
            return hKey;
    }

    return NULL;
}


/******************************Public*Routine******************************\
* ProfileIntIn
*
\**************************************************************************/
int
ProfileIntIn(
    const TCHAR *szKey,
    int iDefault
    )
{
    DWORD dwType=0;
    int   iValue=0;
    BYTE  aData[20];
    DWORD cb;
    HKEY  hKey;

    if((hKey = GetAppKey(TRUE)) == 0)
    {
        return iDefault;
    }

    *(UINT *)&aData = 0;
    cb = sizeof(aData);

    if(RegQueryValueEx(hKey, szKey, NULL, &dwType, aData, &cb))
    {
        iValue = iDefault;
    }
    else
    {
        if(dwType == REG_DWORD || dwType == REG_BINARY)
        {
            iValue = *(int *)&aData;
        }
#ifdef UNICODE
        else if(dwType == REG_SZ)
        {
            iValue = atoiW((LPWSTR)aData);
        }
#else
        else if(dwType == REG_SZ)
        {
            iValue = atoiA((LPSTR)aData);
        }
#endif
    }

    RegCloseKey(hKey);
    return iValue;
}


/******************************Public*Routine******************************\
* ProfileIntOut
*
\**************************************************************************/
BOOL
ProfileIntOut(
    const TCHAR *szKey,
    int iVal
    )
{
    HKEY  hKey;
    BOOL  bRet = FALSE;

    hKey = GetAppKey(TRUE);
    if(hKey)
    {
        RegSetValueEx(hKey, szKey, 0, REG_DWORD, (LPBYTE)&iVal, sizeof(DWORD));
        RegCloseKey(hKey);
        bRet = TRUE;
    }
    return bRet;
}


/******************************Public*Routine******************************\
* ProfileString
*
\**************************************************************************/
UINT
ProfileStringIn(
    LPTSTR  szKey,
    LPTSTR  szDef,
    LPTSTR  sz,
    DWORD   cb
    )
{
    HKEY  hKey;
    DWORD dwType;

    if((hKey = GetAppKey(FALSE)) == 0)
    {
        lstrcpy(sz, szDef);
        return lstrlen(sz);
    }

    if(RegQueryValueEx(hKey, szKey, NULL, &dwType, (LPBYTE)sz, &cb) || dwType != REG_SZ)
    {
        lstrcpy(sz, szDef);
        cb = lstrlen(sz);
    }

    RegCloseKey(hKey);
    return cb;
}


/******************************Public*Routine******************************\
* ProfileStringOut
*
\**************************************************************************/
void
ProfileStringOut(
    LPTSTR  szKey,
    LPTSTR  sz
    )
{
    HKEY  hKey;

    hKey = GetAppKey(TRUE);
    if(hKey)
        RegSetValueEx(hKey, szKey, 0, REG_SZ, (LPBYTE)sz,
            sizeof(TCHAR) * (lstrlen(sz)+1));

    RegCloseKey(hKey);
}


/******************************Public*Routine******************************\
 * LoadWindowPos
 *
 * retrieve the window position information from dragn.ini
 *
\**************************************************************************/

#ifndef SPI_GETWORKAREA
 #define SPI_GETWORKAREA 48  // because NT doesnt have this define yet
#endif

BOOL
LoadWindowPos(
    LPRECT lprc
    )
{
    static RECT rcDefault = {0,0,CX_DEFAULT,CY_DEFAULT};
    RECT  rcScreen;
    RECT  rc;
    HKEY  hKey = GetAppKey(FALSE);

    // read window placement from the registry.
    //
    *lprc = rcDefault;
    if(hKey)
    {
        DWORD cb;
        DWORD dwType;

        cb = sizeof(rc);
        if(! RegQueryValueEx(hKey, cszWindow, NULL, &dwType, (LPBYTE)&rc, &cb)
            && dwType == REG_BINARY && cb == sizeof(RECT))
        {
            *lprc = rc;
        }

        RegCloseKey(hKey);
    }

    // if we fail to get the working area (screen-tray), then assume
    // the screen is 640x480
    //
    if(! SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE))
    {
        rcScreen.top = rcScreen.left = 0;
        rcScreen.right = 640;
        rcScreen.bottom = 480;
    }

    // if the proposed window position is outside the screen,
    // use the default placement
    //
    if(! IntersectRect(&rc, &rcScreen, lprc))
    {
        *lprc = rcDefault;
    }

    return ! IsRectEmpty(lprc);
}


/*****************************Private*Routine******************************\
 *  SaveWindowPos
 *
 * store the window position information in dragn.ini
 *
\**************************************************************************/
BOOL
SaveWindowPos(
    HWND hwnd
    )
{
    WINDOWPLACEMENT wpl;
    HKEY  hKey = GetAppKey(TRUE);

    if(!hKey)
    {
        return FALSE;
    }

    // save the current size and position of the window to the registry
    //
    ZeroMemory(&wpl, sizeof(wpl));
    wpl.length = sizeof(wpl);
    GetWindowPlacement(hwnd, &wpl);


    RegSetValueEx(hKey, cszWindow, 0, REG_BINARY,
        (LPBYTE)&wpl.rcNormalPosition,
        sizeof(wpl.rcNormalPosition));

    RegCloseKey(hKey);
    return TRUE;
}


/*****************************Private*Routine******************************\
* GetRecentFiles
*
* Reads at most MAX_RECENT_FILES from vcdplyer.ini. Returns the number
* of files actually read.  Updates the File menu to show the "recent" files.
*
\**************************************************************************/
int
GetRecentFiles(
    int iLastCount
    )
{
    int     i;
    TCHAR   FileName[MAX_PATH];
    TCHAR   szKey[32];
    HMENU   hSubMenu;

    //
    // Delete the files from the menu
    //
    hSubMenu = GetSubMenu(GetMenu(hwndApp), 0);

    // Delete the separator at slot 2 and all the other recent file entries

    if(iLastCount != 0)
    {
        DeleteMenu(hSubMenu, 2, MF_BYPOSITION);

        for(i = 1; i <= iLastCount; i++)
        {
            DeleteMenu(hSubMenu, ID_RECENT_FILE_BASE + i, MF_BYCOMMAND);
        }
    }


    for(i = 1; i <= MAX_RECENT_FILES; i++)
    {
        DWORD   len;
        TCHAR   szMenuName[MAX_PATH + 3];

        wsprintf(szKey, TEXT("File %d"), i);
        len = ProfileStringIn(szKey, TEXT(""), FileName, MAX_PATH * sizeof(TCHAR));
        if(len == 0)
        {
            i = i - 1;
            break;
        }

        lstrcpy(aRecentFiles[i - 1], FileName);
        wsprintf(szMenuName, TEXT("&%d %s"), i, FileName);

        if(i == 1)
        {
            InsertMenu(hSubMenu, 2, MF_SEPARATOR | MF_BYPOSITION, (UINT)-1, NULL);
        }

        InsertMenu(hSubMenu, 2 + i, MF_STRING | MF_BYPOSITION,
            ID_RECENT_FILE_BASE + i, szMenuName);
    }

    //
    // i is the number of recent files in the array.
    //
    return i;
}


/*****************************Private*Routine******************************\
* SetRecentFiles
*
* Writes the most recent files to the vcdplyer.ini file.  Purges the oldest
* file if necessary.
*
\**************************************************************************/
int
SetRecentFiles(
    TCHAR *FileName,    // File name to add
    int iCount          // Current count of files
    )
{
    TCHAR   FullPathFileName[MAX_PATH];
    TCHAR   *lpFile;
    TCHAR   szKey[32];
    int     iCountNew;
    int     i;

    //
    // Check for dupes - we don't allow them !
    //
    for(i = 0; i < iCount; i++)
    {
        if(0 == lstrcmpi(FileName, aRecentFiles[i]))
        {
            return iCount;
        }
    }

    //
    // Throw away the oldest entry
    //
    MoveMemory(&aRecentFiles[1], &aRecentFiles[0],
        sizeof(aRecentFiles) - sizeof(aRecentFiles[1]));

    //
    // Copy in the full path of the new file.
    //
    GetFullPathName(FileName, MAX_PATH, FullPathFileName, &lpFile);
    lstrcpy(aRecentFiles[0], FullPathFileName);

    //
    // Update the count of files, saturate to MAX_RECENT_FILES.
    //
    iCountNew = min(iCount + 1, MAX_RECENT_FILES);

    //
    // Clear the old stuff and the write out the recent files to disk
    //
    for(i = 1; i <= iCountNew; i++)
    {
        wsprintf(szKey, TEXT("File %d"), i);
        ProfileStringOut(szKey, aRecentFiles[i - 1]);
    }

    //
    // Update the file menu
    //
    GetRecentFiles(iCount);

    return iCountNew;  // the updated count of files.
}
