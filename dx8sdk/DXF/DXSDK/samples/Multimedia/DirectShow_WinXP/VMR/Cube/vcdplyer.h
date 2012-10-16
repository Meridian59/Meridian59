//------------------------------------------------------------------------------
// File: vcdplyer.h
//
// Desc: DirectShow sample code - header file for CMpegMovie class
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <ddraw.h>
#define D3D_OVERLOADS
#include <d3d.h>


/* -------------------------------------------------------------------------
** CMpegMovie - an Mpeg movie playback class.
** -------------------------------------------------------------------------
*/
enum EMpegMovieMode { MOVIE_NOTOPENED = 0x00,
                      MOVIE_OPENED = 0x01,
                      MOVIE_PLAYING = 0x02,
                      MOVIE_STOPPED = 0x03,
                      MOVIE_PAUSED = 0x04 };



#define NUM_CUBE_VERTICES (4*6)

BOOL VerifyVMR(void);

struct StreamInfo
{
    RECT SourceRect;
    BOOL bTexture;
};

struct StreamSize
{
    float cx;
    float cy;
};


class CMpegMovie :
    public CUnknown,
    public IVMRImageCompositor
{

private:
    bool DoesSupportNonPow2CondCap(LPDIRECT3DDEVICE7 pD3DDevice);
    bool DoesSupportLinearCap(LPDIRECT3DDEVICE7 pD3DDevice);
    bool DoesSupportAnisoCap(LPDIRECT3DDEVICE7 pD3DDevice);
    DWORD m_dwTexMirrorWidth;
    DWORD m_dwTexMirrorHeight;
    DWORD m_dwRegister;
    // Our state variable - records whether we are opened, playing etc.
    EMpegMovieMode  m_Mode;
    HANDLE          m_MediaEvent;
    HWND            m_hwndApp;
    int             m_iDuration;
    GUID            m_TimeFormat;
    bool            m_bInitCube;

    StreamInfo m_StreamInfo[16];

    HRESULT FrameMove(LPDIRECT3DDEVICE7 pd3dDevice, FLOAT fTimeKey);
    D3DVERTEX               m_pCubeVertices[NUM_CUBE_VERTICES];

    HRESULT AllocateTextureMirror(LPDIRECTDRAWSURFACE7 pddsVideo,
                                  DWORD* dwWidth, DWORD* dwHeight,
                                  bool bNonPow2Cond);
    LPDIRECTDRAWSURFACE7    m_pDDSTextureMirror;

    IFilterGraph*               m_Fg;
    IGraphBuilder*              m_Gb;
    IMediaControl*              m_Mc;
    IMediaSeeking*              m_Ms;
    IMediaEvent*                m_Me;
    IVMRWindowlessControl*      m_Wc;


    HRESULT AddVideoMixingRendererToFG(DWORD dwStreams);

public:
    STDMETHODIMP SetStreamMediaType(DWORD dwStrmID, AM_MEDIA_TYPE* pmt, BOOL fTexture);
     CMpegMovie(HWND hwndApplication);
    ~CMpegMovie();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    STDMETHODIMP InitCompositionTarget(
        IUnknown* pD3DDevice,
        LPDIRECTDRAWSURFACE7 pddsRenderTarget
        );

    STDMETHODIMP TermCompositionTarget(
        IUnknown* pD3DDevice,
        LPDIRECTDRAWSURFACE7 pddsRenderTarget
        )
    {
        return S_OK;
    }

    STDMETHODIMP CompositeImage(
        IUnknown* pD3DDevice,
        LPDIRECTDRAWSURFACE7 pddsRenderTarget,
        AM_MEDIA_TYPE* pmtRenderTarget,
        REFERENCE_TIME rtStart,
        REFERENCE_TIME rtEnd,
        DWORD dwMappedClrBkgnd,
        VMRVIDEOSTREAMINFO* pVideoStreamInfo,
        UINT cStreams
        );


    HRESULT         OpenMovie(TCHAR achFileName[][MAX_PATH], DWORD dwNumFiles);
    DWORD           CloseMovie();

    BOOL            PlayMovie();
    BOOL            PauseMovie();
    BOOL            StopMovie();

    OAFilterState   GetStateMovie();

    HANDLE          GetMovieEventHandle();
    long            GetMovieEventCode();

    BOOL            PutMoviePosition(LONG x, LONG y, LONG cx, LONG cy);
    BOOL            RepaintVideo(HWND hwnd, HDC hdc);

    REFTIME         GetDuration();
    REFTIME         GetCurrentPosition();
    BOOL            SeekToPosition(REFTIME rt,BOOL bFlushData);

    void            SetFullScreenMode(BOOL bMode);

    void            DisplayModeChanged() {
        if (m_Wc) {
            m_Wc->DisplayModeChanged();
        }
    }
};

RECT GetSourceRectFromMediaType(const AM_MEDIA_TYPE *pMediaType);
LPBITMAPINFOHEADER GetbmiHeader(const AM_MEDIA_TYPE *pMediaType);

