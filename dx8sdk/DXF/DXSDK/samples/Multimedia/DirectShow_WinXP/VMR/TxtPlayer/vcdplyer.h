//------------------------------------------------------------------------------
// File: vcdplyer.h
//
// Desc: DirectShow sample code - header file for TxtPlayer sample
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <ddraw.h>

#define D3D_OVERLOADS
#include <d3d.h>
#include "BltAlpha.h"

/* -------------------------------------------------------------------------
** CMpegMovie - an Mpeg movie playback class.
** -------------------------------------------------------------------------
*/
enum EMpegMovieMode { MOVIE_NOTOPENED = 0x00,
                      MOVIE_OPENED    = 0x01,
                      MOVIE_PLAYING   = 0x02,
                      MOVIE_STOPPED   = 0x03,
                      MOVIE_PAUSED    = 0x04 };

#define GRID_CY 24
#define GRID_CX 40

struct IMpegAudioDecoder;
struct IMpegVideoDecoder;
struct IQualProp;

typedef struct {
    BITMAPINFOHEADER    bmiHeader;
    union {
        RGBQUAD         bmiColors[iPALETTE_COLORS];
        DWORD           dwBitMasks[iMASK_COLORS];
        TRUECOLORINFO   TrueColorInfo;
    };
} AMDISPLAYINFO;

#define NUM_CUBE_VERTICES (4*6)

BOOL VerifyVMR(void);


class CMpegMovie :
    public CUnknown,
    public IVMRSurfaceAllocator,
    public IVMRImagePresenter
{
private:
    // Our state variable - records whether we are opened, playing etc.
    EMpegMovieMode   m_Mode;
    HANDLE           m_MediaEvent;
    HWND             m_hwndApp;
    GUID             m_TimeFormat;

    RECT            m_rcSrc;
    RECT            m_rcDst;
    SIZE            m_VideoSize;
    SIZE            m_VideoAR;

    HRESULT Initialize3DEnvironment(HWND hwndApp);
    HRESULT InitDeviceObjects(LPDIRECT3DDEVICE7 pd3dDevice);
    HRESULT FrameMove(LPDIRECT3DDEVICE7 pd3dDevice,FLOAT fTimeKey);
    HRESULT Render(LPDIRECT3DDEVICE7 pd3dDevice, LPDIRECTDRAWSURFACE7 pDDSBlend, BYTE alpha);
    HRESULT RenderAppImage(LPDIRECT3DDEVICE7 pd3dDevice, LPDIRECTDRAWSURFACE7 pDDSBlend, BYTE alpha);
    HRESULT AllocateSurfaceWorker(DWORD dwFlags,
                                  LPBITMAPINFOHEADER lpHdr,
                                  LPDDPIXELFORMAT lpPixFmt,
                                  LPSIZE lpAspectRatio,
                                  DWORD dwMinBackBuffers,
                                  DWORD dwMaxBackBuffers,
                                  DWORD* lpdwBackBuffer,
                                  LPDIRECTDRAWSURFACE7* lplpSurface);
    HRESULT DDARGB32SurfaceInit(
        LPDIRECTDRAWSURFACE7* lplpDDS,
        BOOL bTexture, DWORD cx, DWORD cy);


    HRESULT CreateFontCache(int cyFont);
    CCritSec                    m_AppImageLock;
    HFONT                       m_hFont;
    int                         m_cxFont;
    int                         m_cyFont;
    int                         m_cxFontImg;
    int                         m_cyFontImg;
    LPDIRECTDRAWSURFACE7        m_lpDDSFontCache;

    HMONITOR                    m_hMonitor;
    LPDIRECTDRAW7               m_lpDDObj;
    CAlphaBlt*                  m_lpBltAlpha;

    LPDIRECTDRAWSURFACE7        m_lpPriSurf;
    LPDIRECTDRAWSURFACE7        m_lpBackBuffer;
    LPDIRECTDRAWSURFACE7        m_lpDDTexture;
    LPDIRECTDRAWSURFACE7        m_lpDDAppImage;
    D3DVERTEX                   m_pCubeVertices[NUM_CUBE_VERTICES];
    DDCAPS                      m_ddHWCaps;
    AMDISPLAYINFO               m_DispInfo;
    BOOL                        m_bRndLess;

    IFilterGraph                *m_Fg;
    IGraphBuilder               *m_Gb;
    IMediaControl               *m_Mc;
    IMediaSeeking               *m_Ms;
    IMediaEvent                 *m_Me;
    IVMRSurfaceAllocatorNotify  *m_SAN;
    IVMRWindowlessControl       *m_Wc;

    HRESULT AddVideoMixingRendererToFG();
    void GetPerformanceInterfaces();
    HRESULT FindInterfaceFromFilterGraph(
        REFIID iid, // interface to look for
        LPVOID *lp  // place to return interface pointer in
        );

public:
     CMpegMovie(HWND hwndApplication);
    ~CMpegMovie();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // IVMRSurfaceAllocator
    STDMETHODIMP AllocateSurface(DWORD_PTR w,
                                 VMRALLOCATIONINFO *lpAllocInfo,
                                 DWORD* lpdwActualBackBuffers,
                                 LPDIRECTDRAWSURFACE7* lplpSurface);
    STDMETHODIMP FreeSurface(DWORD_PTR w);
    STDMETHODIMP PrepareSurface(DWORD_PTR w, LPDIRECTDRAWSURFACE7 lplpSurface,
                                DWORD dwSurfaceFlags);
    STDMETHODIMP AdviseNotify(IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify);


    // IVMRImagePresenter
    STDMETHODIMP StartPresenting(DWORD_PTR w);
    STDMETHODIMP StopPresenting(DWORD_PTR w);
    STDMETHODIMP PresentImage(DWORD_PTR w, VMRPRESENTATIONINFO* p);

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
    BOOL            SetAppText(char* sz);

    LPDIRECTDRAWSURFACE7 GetAppImage() {
        return m_lpDDAppImage;
    }


    IMpegAudioDecoder   *pMpegAudioDecoder;
    IQualProp           *pVideoRenderer;
};

