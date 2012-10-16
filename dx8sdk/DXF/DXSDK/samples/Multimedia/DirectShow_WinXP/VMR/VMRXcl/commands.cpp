//----------------------------------------------------------------------------
//  File:   commands.cpp
//
//  Desc:   DirectShow sample code
//          Processes commands from the user.
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------
#include "project.h"
#include <mmreg.h>
#include <commctrl.h>

#include <stdio.h>
#include <io.h>
#include "resrc1.h"

// Function prototypes
void RepositionMovie(HWND hwnd);
bool FindMediaFile(TCHAR * achFileName, TCHAR * achFoundFile);

// External data
extern TCHAR       g_achFileName[];
extern CMpegMovie  * pMpegMovie;


//----------------------------------------------------------------------------
//  ProcessOpen
// 
//  Creates instance of CMpegMovie and plays it. Called from user UI functions.
//
//  Parameters:
//          achFileName - path to the file to play
//          bPlay       - start demonstration if true
//----------------------------------------------------------------------------
BOOL
ProcessOpen(
    TCHAR *achFileName,
    BOOL bPlay
    )
{
    TCHAR achFoundFile[MAX_PATH];

    if( !FindMediaFile(achFileName, achFoundFile) )
    {
        InvalidateRect( hwndApp, NULL, FALSE );
        UpdateWindow( hwndApp );
        return false;
    }

    lstrcpy(g_achFileName, achFoundFile);
    pMpegMovie = new CMpegMovie(hwndApp);

    if (pMpegMovie) {

        HRESULT hr = pMpegMovie->OpenMovie(g_achFileName);
        if (SUCCEEDED(hr)) {

            TCHAR achTmp[MAX_PATH];

            wsprintf(achTmp, IdStr(STR_APP_TITLE_LOADED), g_achFileName );
            g_State = (VCD_LOADED | VCD_STOPPED);

            RepositionMovie(hwndApp);
            InvalidateRect(hwndApp, NULL, TRUE);

            if (bPlay) {
                pMpegMovie->PlayMovie();
            }
        }
        else {
            MessageBox(hwndApp,
                       TEXT("Failed to open the movie; "),
                       IdStr(STR_APP_TITLE), MB_OK );

            pMpegMovie->CloseMovie();
            delete pMpegMovie;
            pMpegMovie = NULL;
        }
    }

    InvalidateRect( hwndApp, NULL, FALSE );
    UpdateWindow( hwndApp );
    return TRUE;
}

//----------------------------------------------------------------------------
//  FindMediaFile
// 
//  Provides FileOpen dialog to select media file or processes command line
//
//  Parameters:
//          achFileName     - command line
//          achFoundFile    - path to the file to play
//
//  Return: true if success 
//----------------------------------------------------------------------------
bool FindMediaFile(TCHAR * achFileName, TCHAR * achFoundFile)
{
    long lFindRes;
    struct _finddata_t fileinfo;

    lFindRes = _findfirst( achFileName, &fileinfo );
    if( -1 != lFindRes )
    {
        lstrcpy(achFoundFile, achFileName);
        return true;
    }

    OPENFILENAME ofn;
    TCHAR  szBuffer[MAX_PATH];

    lstrcpy(szBuffer, TEXT(""));
    static char szFilter[]  = "Video Files (.MOV, .AVI, .MPG, .VOB, .QT)\0*.AVI;*.MOV;*.MPG;*.VOB;*.QT\0" \
                              "All Files (*.*)\0*.*\0\0";
    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = NULL;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = szFilter;
    ofn.nFilterIndex        = 1;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.lpstrFile           = szBuffer;
    ofn.nMaxFile            = _MAX_PATH;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = "VMRXCL: Select a video file to play...";
    ofn.Flags               = OFN_HIDEREADONLY;
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = "mov";
    ofn.lCustData           = 0L;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName  = NULL; 
    
    if (GetOpenFileName (&ofn))  // user specified a file
    {
        lstrcpy(achFoundFile, ofn.lpstrFile);
        return true;
    }// if

    return false;
}