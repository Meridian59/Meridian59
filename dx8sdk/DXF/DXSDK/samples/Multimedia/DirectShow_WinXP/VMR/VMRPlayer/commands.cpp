//------------------------------------------------------------------------------
// File: commands.cpp
//
// Desc: DirectShow sample code
//       - Processes commands from the user.
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
#include <streams.h>
#include "project.h"

typedef LPBITMAPINFOHEADER PDIB;

extern CMpegMovie *pMpegMovie;
extern void RepositionMovie(HWND hwnd);

// Constants
#define BFT_BITMAP 0x4d42   /* 'BM' */

// Macros
#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))

#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))


/******************************Public*Routine******************************\
* VcdPlyerCaptureImage
*
\**************************************************************************/
BOOL
VcdPlyerCaptureImage(
    LPCSTR szFile
    )
{
    if(pMpegMovie)
    {
        BYTE* lpCurrImage = NULL;

        if(pMpegMovie->GetCurrentImage(&lpCurrImage) == S_OK)
        {
            BITMAPFILEHEADER    hdr;
            HFILE               fh;
            OFSTRUCT            of;
            DWORD               dwSize;
            PDIB                pdib = (PDIB)lpCurrImage;

            fh = OpenFile(szFile,&of,OF_CREATE|OF_READWRITE);

            if(fh == -1)
                return FALSE;

            dwSize = DibSize(pdib);

            hdr.bfType          = BFT_BITMAP;
            hdr.bfSize          = dwSize + sizeof(BITMAPFILEHEADER);
            hdr.bfReserved1     = 0;
            hdr.bfReserved2     = 0;
            hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + pdib->biSize +
                DibPaletteSize(pdib);

            _lwrite(fh,(LPCSTR)(LPVOID)&hdr,sizeof(BITMAPFILEHEADER));
            _hwrite(fh,(LPCSTR)(LPVOID)pdib,dwSize);

            _lclose(fh);

            CoTaskMemFree(lpCurrImage);

            TCHAR szText[128], szDir[MAX_PATH];
            GetCurrentDirectory(MAX_PATH, szDir);
            wsprintf(szText, TEXT("Captured current image to %s\\%s."), szDir, szFile);
            MessageBox(hwndApp, szText, TEXT("Captured bitmap"), MB_OK);
            return TRUE;
        }
    }

    return FALSE;
}


/******************************Public*Routine******************************\
* VcdPlyerDisplayCapturedImage
*
\**************************************************************************/
BOOL
VcdPlyerDisplayCapturedImage(
    LPCSTR szFile
    )
{
    // Open the bitmap with the system-default application
    ShellExecute(hwndApp, "open\0", szFile, NULL, NULL, SW_SHOWNORMAL);

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerOpenCmd
*
\**************************************************************************/
BOOL
VcdPlayerOpenCmd(
    int strmID
    )
{
    static BOOL fFirstTime = TRUE;
    BOOL fRet;
    TCHAR achFileName[MAX_PATH];
    TCHAR achFilter[MAX_PATH];
    LPTSTR lp;

    if(fFirstTime)
    {
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndApp;
        ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
            OFN_SHAREAWARE | OFN_PATHMUSTEXIST;
    }

    lstrcpy(achFilter, IdStr(STR_FILE_FILTER));
    ofn.lpstrFilter = achFilter;

    /*
    ** Convert the resource string into to something suitable for
    ** GetOpenFileName ie.  replace '#' characters with '\0' characters.
    */
    for(lp = achFilter; *lp; lp++)
    {
        if(*lp == TEXT('#'))
        {
            *lp = TEXT('\0');
        }
    }

    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName) / sizeof(TCHAR);
    ZeroMemory(achFileName, sizeof(achFileName));

    fRet = GetOpenFileName(&ofn);
    if(fRet)
    {
        if(strmID == 0)
        {
            fFirstTime = FALSE;
            ProcessOpen(achFileName);
        }
        else
        {
            if(pMpegMovie)
            {
                pMpegMovie->RenderSecondFile(achFileName);
            }
        }

        InitStreamParams(strmID);
    }

    return fRet;
}


/******************************Public*Routine******************************\
* VcdPlayerCloseCmd
*
\**************************************************************************/
BOOL
VcdPlayerCloseCmd(
    void
    )
{
    if(pMpegMovie)
    {
        LONG cx, cy;

        g_State = VCD_NO_CD;
        pMpegMovie->GetMoviePosition(&lMovieOrgX, &lMovieOrgY, &cx, &cy);
        pMpegMovie->StopMovie();
        pMpegMovie->CloseMovie();

        SetDurationLength((REFTIME)0);
        SetCurrentPosition((REFTIME)0);

        delete pMpegMovie;
        pMpegMovie = NULL;
    }

    g_bSecondFileLoaded = FALSE;
    InvalidateRect(hwndApp, NULL, FALSE);
    UpdateWindow(hwndApp);
    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerPlayCmd
*
\**************************************************************************/
BOOL
VcdPlayerPlayCmd(
    void
    )
{
    BOOL fStopped = (g_State & VCD_STOPPED);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if((fStopped || fPaused))
    {
        if(pMpegMovie)
        {
            pMpegMovie->PlayMovie();
        }

        g_State &= ~(fStopped ? VCD_STOPPED : VCD_PAUSED);
        g_State |= VCD_PLAYING;
    }

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerPlayCmd
*
\**************************************************************************/
BOOL
VcdPlayerStopCmd(
    void
    )
{
    BOOL fPlaying = (g_State & VCD_PLAYING);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if((fPlaying || fPaused))
    {
        if(pMpegMovie)
        {
            pMpegMovie->StopMovie();
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
        }

        g_State &= ~(fPlaying ? VCD_PLAYING : VCD_PAUSED);
        g_State |= VCD_STOPPED;
    }
    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerStepCmd
*
\**************************************************************************/
BOOL
VcdPlayerStepCmd(
    void
    )
{
    if(pMpegMovie)
    {
        // Ensure that the video is paused to update toolbar buttons
        if(g_State & VCD_PLAYING)
            VcdPlayerPauseCmd();

		if(pMpegMovie->FrameStepMovie())
        {
	        g_State |= VCD_STEPPING;
            return TRUE;
        }
    }
    return FALSE;
}


/******************************Public*Routine******************************\
* VcdPlayerPauseCmd
*
\**************************************************************************/
BOOL
VcdPlayerPauseCmd(
    void
    )
{
    BOOL fPlaying = (g_State & VCD_PLAYING);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if(fPlaying)
    {
        if(pMpegMovie)
        {
            pMpegMovie->PauseMovie();
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
        }

        g_State &= ~VCD_PLAYING;
        g_State |= VCD_PAUSED;
    }
    else if(fPaused)
    {
        if(pMpegMovie)
        {
            pMpegMovie->PlayMovie();
        }

        g_State &= ~VCD_PAUSED;
        g_State |= VCD_PLAYING;
    }

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerSeekCmd
*
\**************************************************************************/
void
VcdPlayerSeekCmd(
    REFTIME rtSeekBy
    )
{
    REFTIME rt;
    REFTIME rtDur;

    rtDur = pMpegMovie->GetDuration();
    rt = pMpegMovie->GetCurrentPosition() + rtSeekBy;

    rt = max(0, min(rt, rtDur));

    pMpegMovie->SeekToPosition(rt,TRUE);
    SetCurrentPosition(pMpegMovie->GetCurrentPosition());
}


/******************************Public*Routine******************************\
* ProcessOpen
*
\**************************************************************************/
void
ProcessOpen(
    TCHAR *achFileName,
    BOOL bPlay
    )
{
    /*
    ** If we currently have a video loaded we need to discard it here.
    */
    if(g_State & VCD_LOADED)
    {
        VcdPlayerCloseCmd();
    }

    lstrcpy(g_achFileName, achFileName);

    pMpegMovie = new CMpegMovie(hwndApp);
    if(pMpegMovie)
    {
        HRESULT hr = pMpegMovie->OpenMovie(g_achFileName);
        if(SUCCEEDED(hr))
        {
            TCHAR achTmp[MAX_PATH];

            nRecentFiles = SetRecentFiles(achFileName, nRecentFiles);

            wsprintf(achTmp, IdStr(STR_APP_TITLE_LOADED),
                g_achFileName);
            g_State = (VCD_LOADED | VCD_STOPPED);

            // SetDurationLength(pMpegMovie->GetDuration());
            g_TimeFormat = VcdPlayerChangeTimeFormat(g_TimeFormat);

            RepositionMovie(hwndApp);
            pMpegMovie->SetBorderClr(RGB(0x00, 0x80, 0x80));

            //  If play
            if(bPlay)
            {
                pMpegMovie->PlayMovie();
            }
        }
        else
        {
            TCHAR Buffer[MAX_ERROR_TEXT_LEN];

            if(AMGetErrorText(hr, Buffer, MAX_ERROR_TEXT_LEN))
            {
                MessageBox(hwndApp, Buffer,
                    IdStr(STR_APP_TITLE), MB_OK);
            }
            else
            {
                MessageBox(hwndApp,
                    TEXT("Failed to open the movie.  Either the file was ")
                    TEXT("not found or the wave device is in use."),
                    IdStr(STR_APP_TITLE), MB_OK);
            }

            pMpegMovie->CloseMovie();
            delete pMpegMovie;
            pMpegMovie = NULL;
        }
    }

    InitStreamParams(0);

    InvalidateRect(hwndApp, NULL, FALSE);
    UpdateWindow(hwndApp);
}


/******************************Public*Routine******************************\
* VcdPlayerChangeTimeFormat
*
* Tries to change the time format to id.  Returns the time format that
* actually got set.  This may differ from id if the graph does not support
* the requested time format.
*
\**************************************************************************/
int
VcdPlayerChangeTimeFormat(
    int id
    )
{
    // Menu items are disabled while we are playing
    BOOL    bRet = FALSE;
    int     idActual = id;

    ASSERT(pMpegMovie);
    ASSERT(pMpegMovie->StatusMovie() != MOVIE_NOTOPENED);

    // Change the time format with the filtergraph
    switch(id)
    {
        case IDM_FRAME:
            bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_FRAME);
            break;

        case IDM_FIELD:
            bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_FIELD);
            break;

        case IDM_SAMPLE:
            bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_SAMPLE);
            break;

        case IDM_BYTES:
            bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_BYTE);
            break;
    }

    if(!bRet)
    {
        // IDM_TIME and all other cases,  everyone should support IDM_TIME
        bRet = pMpegMovie->SetTimeFormat(TIME_FORMAT_MEDIA_TIME);
        ASSERT(bRet);
        idActual = IDM_TIME;
    }

    // Pause the movie to get a current position

    SetDurationLength(pMpegMovie->GetDuration());
    SetCurrentPosition(pMpegMovie->GetCurrentPosition());

    return idActual;
}


/******************************Public*Routine******************************\
* VcdPlayerRewindCmd
*
\**************************************************************************/
BOOL
VcdPlayerRewindCmd(
    void
    )
{
    if(pMpegMovie)
    {
        pMpegMovie->SeekToPosition((REFTIME)0,FALSE);
    }

    return TRUE;
}


