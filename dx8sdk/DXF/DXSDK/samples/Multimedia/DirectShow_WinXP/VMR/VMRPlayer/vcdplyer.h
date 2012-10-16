//------------------------------------------------------------------------------
// File: vcdplyer.h
//
// Desc: DirectShow sample code
//       - Class header file for VMRPlayer sample
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

/* -------------------------------------------------------------------------
** CMpegMovie - an Mpeg movie playback class.
** -------------------------------------------------------------------------
*/
enum EMpegMovieMode { MOVIE_NOTOPENED = 0x00,
                      MOVIE_OPENED = 0x01,
                      MOVIE_PLAYING = 0x02,
                      MOVIE_STOPPED = 0x03,
                      MOVIE_PAUSED = 0x04 };

BOOL VerifyVMR(void);

struct IMpegAudioDecoder;
struct IMpegVideoDecoder;
struct IQualProp;

class CMpegMovie
{
private:
    // Our state variable - records whether we are opened, playing etc.
    EMpegMovieMode   m_Mode;
    HANDLE           m_MediaEvent;
    HWND             m_hwndApp;
    GUID             m_TimeFormat;

    IFilterGraph            *m_Fg;
    IGraphBuilder           *m_Gb;
    IMediaControl           *m_Mc;
    IMediaSeeking           *m_Ms;
    IMediaEvent             *m_Me;
    IVMRWindowlessControl   *m_Wc;

    HRESULT AddVideoMixingRendererToFG();
    HRESULT AddBallToFG();
    void GetPerformanceInterfaces();
    HRESULT FindInterfaceFromFilterGraph(
        REFIID iid, // interface to look for
        LPVOID *lp  // place to return interface pointer in
        );

public:
     CMpegMovie(HWND hwndApplication);
    ~CMpegMovie();

    HRESULT         OpenMovie(TCHAR *lpFileName);
    DWORD           CloseMovie();
    BOOL            PlayMovie();
    BOOL            PauseMovie();
    BOOL            StopMovie();
    OAFilterState   GetStateMovie();
    HANDLE          GetMovieEventHandle();
    long            GetMovieEventCode();
    BOOL            PutMoviePosition(LONG x, LONG y, LONG cx, LONG cy);
    BOOL            GetMoviePosition(LONG *x, LONG *y, LONG *cx, LONG *cy);
    BOOL            GetNativeMovieSize(LONG *cx, LONG *cy);
    BOOL            CanMovieFrameStep();
    BOOL            FrameStepMovie();
    REFTIME         GetDuration();
    REFTIME         GetCurrentPosition();
    BOOL            SeekToPosition(REFTIME rt,BOOL bFlushData);
    EMpegMovieMode  StatusMovie();
    BOOL            IsTimeFormatSupported(GUID Format);
    BOOL            IsTimeSupported();
    BOOL            SetTimeFormat(GUID Format);
    GUID            GetTimeFormat();
    void            SetFocus();
    BOOL            ConfigDialog(HWND hwnd);
    BOOL            RepaintVideo(HWND hwnd, HDC hdc);
    BOOL            SetAppImage(VMRALPHABITMAP* lpBmpInfo);
    BOOL            UpdateAppImage(VMRALPHABITMAP* lpBmpInfo);
    void            SetBorderClr(COLORREF clr);

    void            DisplayModeChanged() {
        m_Wc->DisplayModeChanged();
    }

    HRESULT         GetCurrentImage(LPBYTE* lplpDib)
    {
        return m_Wc->GetCurrentImage(lplpDib);
    }

    HRESULT         RenderSecondFile(TCHAR *lpFileName);

    IVMRMixerControl *m_pMixControl;
};

