//------------------------------------------------------------------------------
// File: Main.cpp
//
// Desc: DirectShow sample code - simple movie player console application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//   This program uses the PlayCutscene() function provided in cutscene.cpp.  
//   It is only necessary to provide the name of a file and the application's 
//   instance handle.
//
//   If the file was played to the end, PlayCutscene returns S_OK.
//   If the user interrupted playback, PlayCutscene returns S_FALSE.
//   Otherwise, PlayCutscene will return an HRESULT error code.
//
//   Usage: cutscene <required file name>
//

#include <windows.h>

#include "cutscene.h"

#define USAGE \
        TEXT("Cutscene is a console application that demonstrates\r\n")      \
        TEXT("playing a movie at the beginning of your game.\r\n\r\n")       \
        TEXT("Please provide a valid filename on the command line.\r\n")     \
        TEXT("\r\n            Usage: cutscene <filename>\r\n")               \


//
// Main program code
//
int APIENTRY
WinMain (
         HINSTANCE hInstance,
         HINSTANCE hPrevInstance,
         LPSTR lpszMovie,
         int nCmdShow
         )
{
    HRESULT hr;
    TCHAR szMovieName[MAX_PATH];

    // Prevent C4100: unreferenced formal parameter
    hPrevInstance = hPrevInstance;
    nCmdShow = nCmdShow;

#ifdef UNICODE
    TCHAR szCommandLine[MAX_PATH], *pstrCommandLine=NULL;
    UNREFERENCED_PARAMETER(lpszMovie);

    // Get the UNICODE command line.  This is necessary for UNICODE apps
    // because the standard WinMain only passes in an LPSTR for command line.
    lstrcpy(szCommandLine, GetCommandLine());
    pstrCommandLine = szCommandLine;

    // Skip the first part of the command line, which is the full path 
    // to the exe.  If the path contains spaces, it will be contained in quotes,
    // so the leading and trailing quotes need to be removed.
    if (*pstrCommandLine == TEXT('\"'))
    {
        // Remove the leading quotes
        pstrCommandLine++;

        // Skip characters until we reach the trailing quotes
        while (*pstrCommandLine != TEXT('\0') && *pstrCommandLine != TEXT('\"'))
            pstrCommandLine++;

        // Strip trailing quotes from executable name
        if( *pstrCommandLine == TEXT('\"'))
            pstrCommandLine++;
    }
    else
    {
        // Executable name isn't encased in quotes, so just search for the
        // first space, which indicates the end of the executable name.
        while (*pstrCommandLine != TEXT('\0') && *pstrCommandLine != TEXT(' '))
            pstrCommandLine++;
    }

    // Strip all leading spaces on file name
    while( *pstrCommandLine == TEXT(' '))
        pstrCommandLine++;

   lstrcpy(szMovieName, pstrCommandLine);

#else
    lstrcpy(szMovieName, lpszMovie);
#endif

    // If no filename is specified, show an error message and exit
    if (szMovieName[0] == TEXT('\0'))
    {
        MessageBox(NULL, USAGE, TEXT("Cutscene Error"), MB_OK | MB_ICONERROR);
        exit(1);
    }

    // Play movie
    hr = PlayCutscene(szMovieName, hInstance);

    return hr;
}


