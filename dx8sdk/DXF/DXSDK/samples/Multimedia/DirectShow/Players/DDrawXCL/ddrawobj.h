//------------------------------------------------------------------------------
// File: DDrawObj.h
//
// Desc: DirectShow sample code - DDraw Object class header file.
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// #define NOFLIP  1  /* for debugging */


// Define ball radius
#define BALL_RADIUS   40
#define BALL_STEP      4

//
// Some macros
//
#define RECTWIDTH(rect)   ((rect).right  - (rect).left)
#define RECTHEIGHT(rect)  ((rect).bottom - (rect).top)

// forward declaration
class COverlayCallback ;

//
// DDraw object class to paint color key, flip etc etc.
//
class CDDrawObject {
public:   // public methods for Windows structure to call
    CDDrawObject(HWND hWndApp) ;
    ~CDDrawObject(void) ;
    
    BOOL    Initialize(HWND hWndApp) ;
    HRESULT StartExclusiveMode(HWND hWndApp) ;
    HRESULT StopExclusiveMode(HWND hWndApp) ;
    HRESULT UpdateAndFlipSurfaces(void) ;
    void    SetVideoPosition(DWORD dwVideoLeft, DWORD dwVideoTop, 
                             DWORD dwVideoWidth, DWORD dwVideoHeight) ;
    void    SetBallPosition(DWORD dwVideoLeft, DWORD dwVideoTop, 
                            DWORD dwVideoWidth, DWORD dwVideoHeight) ;
    void    MoveBallPosition(int iDirX, int iDirY) ;
    inline  void    SetColorKey(DWORD dwColorKey)   { m_dwVideoKeyColor = dwColorKey ; } ;
    inline  void    GetScreenRect(RECT *pRect)      { *pRect = m_RectScrn ; } ;
    inline  BOOL    IsInExclusiveMode(void)         { return m_bInExclMode ; } ;
    inline  LPDIRECTDRAW         GetDDObject(void)  { return m_pDDObject ; } ;
    inline  LPDIRECTDRAWSURFACE  GetDDPrimary(void) { return m_pPrimary ; } ;
    inline  void    SetOverlayState(BOOL bState)    { m_bOverlayVisible = bState ; } ;
    inline  IDDrawExclModeVideoCallback * GetCallbackInterface(void) { return m_pOverlayCallback ; } ;
    
private:  // private helper methods for the class' own use
    HRESULT FillSurface(IDirectDrawSurface *pDDSurface) ;
    void    DrawOnSurface(LPDIRECTDRAWSURFACE pSurface) ;
    HRESULT ConvertColorRefToPhysColor(COLORREF rgb, DWORD *pdwPhysColor) ;
    inline  DWORD   GetColorKey(DWORD dwColorKey)   { return m_dwVideoKeyColor ; } ;
    inline  void    IncCount(void)                  { m_iCount++ ; } ;
    inline  int     GetCount(void)                  { return m_iCount ; } ;
    
private:  // internal state info
    LPDIRECTDRAW         m_pDDObject ;   // DirectDraw interface
    LPDIRECTDRAWSURFACE  m_pPrimary ;    // primary surface
    LPDIRECTDRAWSURFACE  m_pBackBuff ;   // back buffer attached to primary
    
    BOOL     m_bInExclMode ;     // Are we in exclusive mode now?
    RECT     m_RectScrn ;        // whole screen as a rect
    RECT     m_RectVideo ;       // current video position as rect
    DWORD    m_dwScrnColor ;     // physical color for surface filling
    DWORD    m_dwVideoKeyColor ; // physical color for color keying video area
    int      m_iCount ;          // flip count
    int      m_iBallCenterX ;    // X-coord of ball's center
    int      m_iBallCenterY ;    // Y-coord of ball's center
    BOOL     m_bFrontBuff ;      // draw on front (or back) buffer?
    LPTSTR   m_szFrontMsg ;      // front surface string ("Front Buffer")
    LPTSTR   m_szBackMsg ;       // back surface string ("Back Buffer")
    LPTSTR   m_szDirection ;     // Direction string for users
    HPEN     m_hPen ;            // pen for drawing outline of the ball
    HBRUSH   m_hBrush ;          // brush for filling the ball
    BOOL     m_bOverlayVisible ; // is overlay visible?
    IDDrawExclModeVideoCallback *m_pOverlayCallback ;  // overlay callback handler interface
} ;



//
// Overlay callback handler object class
//
class COverlayCallback : public CUnknown, public IDDrawExclModeVideoCallback
{
public:
    COverlayCallback(CDDrawObject *pDDrawObj, HWND hWndApp, HRESULT *phr) ;
    ~COverlayCallback() ;

    DECLARE_IUNKNOWN
    //
    // IDDrawExclModeVideoCallback interface methods
    //
    STDMETHODIMP OnUpdateOverlay(BOOL  bBefore,
                                 DWORD dwFlags,
                                 BOOL  bOldVisible,
                                 const RECT *prcSrcOld,
                                 const RECT *prcDestOld,
                                 BOOL  bNewVisible,
                                 const RECT *prcSrcNew,
                                 const RECT *prcDestNew) ;

    STDMETHODIMP OnUpdateColorKey(COLORKEY const *pKey,
                                  DWORD    dwColor) ;

    STDMETHODIMP OnUpdateSize(DWORD dwWidth, DWORD dwHeight, 
                              DWORD dwARWidth, DWORD dwARHeight) ;

private:
    CDDrawObject *m_pDDrawObj ;
    HWND          m_hWndApp ;
} ;
