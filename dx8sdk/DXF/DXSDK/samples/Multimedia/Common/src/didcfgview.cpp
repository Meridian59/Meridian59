//-----------------------------------------------------------------------------
// File: didcfgview.cpp
//
// Desc: Interface for retrieving and rendering DirectInput Device
//       configuration view( implemenation ). Feel free to use 
//       this class as a starting point for adding extra functionality.
//
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <d3d8.h>
#include <d3dx8tex.h>
#include <assert.h>
#include <tchar.h>
#include "didcfgview.h"
#include "dxutil.h"




//-----------------------------------------------------------------------------
// Name: DIDCfgView()
// Desc: constructor
//-----------------------------------------------------------------------------
DIDCfgView::DIDCfgView()
    :   m_lpViewManager( NULL ),
        m_lpCalloutManager( NULL ),
        m_bIsInit( FALSE ),
        m_nView( DIDCV_INVALID_ID ),
        m_lpDidImgHeader( NULL ),
        m_lpCustomViewInfo( NULL ),
        m_lpDIDevice( NULL ),
        m_bCanBeCollapsed( FALSE ),
        m_bIsCollapsed( FALSE ),
        m_lpDiaf( NULL ),
        m_crFore( RGB( 0xff, 0xff, 0xff ) ),
        m_crBack( RGB( 0x0, 0x0, 0x0 ) ),
        m_crHighlight( RGB( 0x33, 0x33, 0x33 ) ),
        m_crHighlightLine( RGB( 0x99, 0xff, 0x0 ) ),
        m_hFont( NULL )
{
    _tcscpy( m_tszDefaultText, TEXT( "- - -" ) );
    ZeroMemory( &m_ptOrigin, sizeof( POINT ) );
}




//-----------------------------------------------------------------------------
// Name: ~DIDCfgView()
// Desc: destructor
//-----------------------------------------------------------------------------
DIDCfgView::~DIDCfgView()
{
    CleanUp();
    
    if( m_hFont )
    {
        // free the font
        DeleteObject( m_hFont );
        m_hFont = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: Init()
// Desc: initializes the DIDCfgView object with a DirectInputDevice8
//       reference.  upon successful completion of this functions, image
//       and callout data for the device should be successfully retrieved,
//       loaded, and processed, ready to be used.( ultimately, after optionally 
//       setting the action format, callout state, performing hit tests, etc -
//       the image can be drawn by calling RenderView(). )  this Init()
//       must be called successfully before anything else can be used.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::Init( LPDIRECTINPUTDEVICE8 lpDidev )
{
    HRESULT hr;

    // always start clean
    CleanUp();

    // sanity check
    if( NULL == lpDidev )
    {
        return DIERR_INVALIDPARAM;
    }

    // allocate objects
    hr = InitAlloc();
    if( FAILED( hr ) )
    {
        CleanUp();
        return hr;
    }

    // retrieve the image info from DirectInput
    hr = InitImageInfoRetrieve( lpDidev );
    if( FAILED( hr ) )
    {
        // no device image on system, let's build our own
        hr = InitCustomViews( lpDidev );
        if( FAILED( hr ) )
        {
            CleanUp();
            return hr;
        }

        // keep a reference for custom views, since we might
        // need the device to reconstruct collapsed views
        m_lpDIDevice = lpDidev;
        m_lpDIDevice->AddRef();
    }
    else
    {
        // process the image info from DirectInput
        hr = InitImageInfoProcess();
        if( FAILED( hr ) )
        {
            CleanUp();
            return hr;
        }
    }

    m_bIsInit = TRUE;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: resets the cfgview object, freeing memory and other resources
//       this method is idempotent.
//-----------------------------------------------------------------------------
VOID DIDCfgView::CleanUp()
{
    m_bIsInit = FALSE;
    m_nView = -1;
    m_bCanBeCollapsed = FALSE;
    m_bIsCollapsed = FALSE;
    
    if( m_lpDidImgHeader )
    {
        // free array
        SAFE_DELETE_ARRAY( m_lpDidImgHeader->lprgImageInfoArray );

        // free the imginfoheader
        delete m_lpDidImgHeader;
        m_lpDidImgHeader = NULL;
    }

    // free the view manager
    SAFE_DELETE( m_lpViewManager );

    // free callout manager
    SAFE_DELETE( m_lpCalloutManager );

    // free the custom view info
    SAFE_DELETE( m_lpCustomViewInfo );

    // release the DirectInputDevice
    SAFE_RELEASE( m_lpDIDevice );

    if( m_lpDiaf )
    {
        // free the DIACTION array
        SAFE_DELETE_ARRAY( m_lpDiaf->rgoAction );

        // free the DIACTIONFORMAT
        delete m_lpDiaf;
        m_lpDiaf = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: SetActionFormat()
// Desc: sets the action format mapping for this cfgview object.  the action
//       format should be relevant to the same DirectInputDevice that was
//       used to initialize this object.  this information will be used when
//       the configuration view is rendered.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::SetActionFormat( LPDIACTIONFORMAT lpDiaf )
{
    UINT i;

    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // clear any actions currently mapped
    m_lpCalloutManager->ClearAllActionMaps();

    // no DIACTIONFORMAT passed in, error
    if( NULL == lpDiaf )
        return DIERR_INVALIDPARAM;

    // set each action mapping
    for( i = 0; i < lpDiaf->dwNumActions; i++ )
    {
        m_lpCalloutManager->SetActionMap( &( lpDiaf->rgoAction[i] ), 
                                          lpDiaf->rgoAction[i].dwObjID );
    }

    // if we build custom views
    if( m_lpCustomViewInfo )
    {
        // determine if views can be compacted
        m_bCanBeCollapsed = m_lpCalloutManager->CalcCanBeCollapsed();

        // keep copy of the action map
        if( m_bCanBeCollapsed )
            if( FALSE == CopyActionMap( lpDiaf ) )
                return DIERR_OUTOFMEMORY;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RebuildViews()
// Desc: collapses or exands the views
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::RebuildViews( BOOL bCompact )
{
    UINT i;
    INT  iNumViews;

    // make sure this has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    if( NULL == m_lpDiaf )
        return DIERR_NOTINITIALIZED;

    if( FALSE == m_bCanBeCollapsed )
        return DIERR_INVALIDPARAM;

    // build the custom views, expand or compact
    InitCustomViews( m_lpDIDevice, bCompact );

    // set the action map
    for( i = 0; i < m_lpDiaf->dwNumActions; i++ )
    {
        m_lpCalloutManager->SetActionMap( &( m_lpDiaf->rgoAction[i] ), 
                                          m_lpDiaf->rgoAction[i].dwObjID );
    }

    // get the new number of views;
    iNumViews = m_lpViewManager->GetNumViews();

    // bound the current view id
    if( iNumViews == 0 )
        m_nView = DIDCV_INVALID_ID;

    if( m_nView >= iNumViews )
        m_nView  = iNumViews - 1;

    // keep track of whether the views are compact or not
    m_bIsCollapsed = bCompact;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetCalloutState()
// Desc: sets the state for a given callout.  the flags are:
//
//       DIDCV_DRAWCALLOUT - draws the callout
//       DIDCV_DRAWOVERLAY - draws the overlay
//       DIDCV_DRAWHIGHLIGHT - draws highlight
//       DIDCV_DRAWEMPTYCALLOUT - draws the callout even if an action mapping
//          is not present
//       DIDCV_DRAWFULLNAME - draws the full name of action, if the
//          callout clips part of the string
//
//       the dwFlags argument should be a bitwise AND of none, any, or all of
//       the values.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::SetCalloutState( DWORD dwFlags, DWORD dwObjID )
{
    DidcvCalloutState calloutState;
    BOOL bResult;
    
    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // set the state structure to match the dword
    calloutState.SetFlag( dwFlags );

    // set the state for the object id( if exist )
    bResult = m_lpCalloutManager->SetCalloutState( &calloutState, dwObjID );

    // could be an invalid object id
    if( FALSE == bResult )
        return DIERR_INVALIDPARAM;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetAllCalloutState()
// Desc: sets the state for all callouts.  see SetCalloutState for flag value
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::SetAllCalloutState( DWORD dwFlags )
{
    DidcvCalloutState calloutState;
    BOOL bResult;
    
    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // set the state structure to match the dword
    calloutState.SetFlag( dwFlags );

    // set the state for the object id( if exist )
    bResult = m_lpCalloutManager->SetAllCalloutState( &calloutState );

    // could be an invalid object id
    if( FALSE == bResult )
        return DIERR_INVALIDPARAM;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutState()
// Desc: returns the state for a given callout.  see SetCalloutState comments
//       for explanation of the flags values
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::GetCalloutState( LPDWORD lpdwFlags, DWORD dwObjID )
{
    DidcvCalloutState calloutState;
    BOOL bResult;

    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // sanity check
    if( NULL == lpdwFlags )
        return DIERR_INVALIDPARAM;

    // retrieve callout state from the callout manager
    bResult = m_lpCalloutManager->GetCalloutState( &calloutState, dwObjID );

    // did not find state info
    if( FALSE == bResult )
        return DIERR_INVALIDPARAM;

    // construct flag from the state
    *lpdwFlags = calloutState.MakeFlag();

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetObjectIDByLocation()
// Desc: for a given point in image coordinate, returns a callout with which
//       the point intersects.  if not found, the functions return success,
//       and the return argument will be set to NULL.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::GetObjectIDByLocation( LPDWORD lpdwObjID, LPPOINT pPt )
{
    POINT ptTransform;

    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // sanity check
    if( NULL == lpdwObjID )
        return DIERR_INVALIDPARAM;

    // transform by new origin
    ptTransform.x = pPt->x - m_ptOrigin.x;
    ptTransform.y = pPt->y - m_ptOrigin.y;

    // pass pt into the callout manager with the current view for hit test
    *lpdwObjID = m_lpCalloutManager->GetObjectIDByLocation( &ptTransform, m_nView );

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetCurrentView()
// Desc: set the current view for the DIDCfgView object.  all further
//       operations will be done in the context of this new view.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::SetCurrentView( INT nView )
{
    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // sanity check
    if( nView < 0 || nView >=( INT ) m_lpViewManager->GetNumViews() )
        return DIERR_INVALIDPARAM;

    // set the view
    m_nView = nView;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetCurrentView()
// Desc: returns the current view identifier
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::GetCurrentView( LPINT lpnView  )
{
    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // sanity check
    if( NULL == lpnView )
        return DIERR_INVALIDPARAM;

    // copy the view
    *lpnView = m_nView;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: GetInfo()
// Desc: retrieves information from DIDCfgView
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::GetInfo( DIDCfgViewInfo* pCfgViewInfo )
{
    // sanity checks
    if( NULL == pCfgViewInfo )
        return DIERR_INVALIDPARAM;

    ZeroMemory( pCfgViewInfo, sizeof( DIDCfgViewInfo ) );

    // always return whether the Config Viewer is initialized or not
    pCfgViewInfo->bIsInit = m_bIsInit;

    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // set the information to be returned
    pCfgViewInfo->iCurrentViewID = m_nView;
    pCfgViewInfo->iNumTotalViews =( INT )m_lpViewManager->GetNumViews();
    pCfgViewInfo->dwNumUniqueCallouts = m_lpCalloutManager->GetNumUniqueCallouts();
    pCfgViewInfo->bCanBeCollapsed = m_bCanBeCollapsed;
    pCfgViewInfo->bIsCollapsed = m_bIsCollapsed;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: SetColors()
// Desc: sets the foreground( text ) color, background and highlight color,
//       the highlighted callout color
//-----------------------------------------------------------------------------
VOID DIDCfgView::SetColors( COLORREF crFore, COLORREF crBack, 
                            COLORREF crHighlight, COLORREF crHighlightLine )
{
    m_crFore = crFore;
    m_crBack = crBack;
    m_crHighlight = crHighlight;
    m_crHighlightLine = crHighlightLine;
}




//-----------------------------------------------------------------------------
// Name: GetColor()
// Desc: returns the colors currently being using in rendering
//-----------------------------------------------------------------------------
VOID DIDCfgView::GetColors( COLORREF* pCrFore, COLORREF* pCrBack, 
                            COLORREF* pCrHighlight, COLORREF* pCrHighlightLine )
{
    if( pCrFore ) *pCrFore = m_crFore;
    if( pCrBack ) *pCrBack = m_crBack;
    if( pCrHighlight ) *pCrHighlight = m_crHighlight;
    if( pCrHighlightLine ) *pCrHighlightLine = m_crHighlightLine;
}




//-----------------------------------------------------------------------------
// Name: SetFont()
// Desc: sets the font used to draw the text
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::SetFont( const TCHAR* pctszFontName, DWORD dwAttribs )
{
    HDC hDCMem;
    HFONT hFont;
    HRESULT hr = DI_OK;

    if( NULL == pctszFontName )
    {
        if( m_hFont )
            DeleteObject( m_hFont );
    }
    else
    {
        // create a memory device context
        hDCMem = CreateCompatibleDC( NULL );

        // create font
        hFont = DidcvCreateFont( hDCMem, pctszFontName, DIDCV_DEFAULT_FONT_SIZE, 
                                 0, dwAttribs, FALSE );

        if( NULL == hFont )
            hr = DIERR_INVALIDPARAM;
        else
        {
            if( m_hFont )
                DeleteObject( m_hFont );

            m_hFont = hFont;
        }

        // clean up
        DeleteDC( hDCMem );
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: GetFont()
// Desc: sets the font used to draw the text
//-----------------------------------------------------------------------------
HFONT   DIDCfgView::GetFont()
{
    return m_hFont;
}




//-----------------------------------------------------------------------------
// Name: SetViewOrigin
// Desc: tells DIDCfgView where to start drawing and performing hit tests
//       on the surface being rendered on
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::SetViewOrigin( const POINT* pPtOrg, POINT* pPtOldOrg )
{
    if( NULL == pPtOrg )
        return DIERR_INVALIDPARAM;

    // return the current origin
    if( pPtOldOrg )
    {
        pPtOldOrg->x = m_ptOrigin.x;
        pPtOldOrg->y = m_ptOrigin.y;
    }

    // set the current origin to new origin
    m_ptOrigin.x = pPtOrg->x;
    m_ptOrigin.y = pPtOrg->y;

    return DI_OK;
}




//-----------------------------------------------------------------------------
// Name: CalcCenterOrgForCurrentView()
// Desc: calculate the coordinate where to set the origin, in order for the
//       current view to be centered on the drawing surface, lpImgSize should
//       contain the dimensions of the drawing surface.  if the view is
//       larger than the drawing area, then( 0, 0 ) will be returned
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::CalcCenterOrgForCurrentView( const SIZE* pSurfaceSize, 
                                                 POINT* pCenterOrg, BOOL bSetOrigin )
{
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // sanity check
    if( NULL == pSurfaceSize || NULL == pCenterOrg )
        return DIERR_INVALIDPARAM;

    INT x = 0, y = 0;
    DidcvView* pView;
    SIZE imgSize;

    // get the current view
    pView = m_lpViewManager->GetImage( m_nView );

    // get the size and find the coordinate at which to center the image
    // note that x == 0 and y == 0 if surface width and height
    // is not as big as the image width and height, respectively
    if( pView && pView->GetViewSize( &imgSize ) )
    {
        if( imgSize.cx < pSurfaceSize->cx )
            x =( pSurfaceSize->cx - imgSize.cx ) / 2;
        if( imgSize.cy < pSurfaceSize->cy )
            y =( pSurfaceSize->cy - imgSize.cy ) / 2;
    }

    // set the values to be returns
    pCenterOrg->x = x;
    pCenterOrg->y = y;

    // if bSetOrigin is TRUE, then set the view origin for this object
    if( bSetOrigin )
        SetViewOrigin( pCenterOrg, NULL );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetDefaultText()
// Desc: sets the default text for callouts that doesn't have action mapped
//-----------------------------------------------------------------------------
VOID DIDCfgView::SetDefaultText( const TCHAR* pctszDefaultText, DWORD length )
{
    // copy up to MAX_PATH characters
    size_t numToCopy =( length < MAX_PATH ? length : MAX_PATH );
    _tcsncpy( m_tszDefaultText, pctszDefaultText, numToCopy );

    // if we copied MAX_PATH, make sure it is NULL terminated
    if( numToCopy == MAX_PATH )
        m_tszDefaultText [MAX_PATH - 1] = 0;
}




//-----------------------------------------------------------------------------
// Name: RenderView()
// Desc: draws the configuration view to HBITMAP, a DDB or a DIB
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::RenderView( HBITMAP hBitmap, BOOL bIsDib )
{
    HDC hDCMem;
    HGDIOBJ hOldBitmap;
    DIBSECTION dibs;
    HRESULT hr;

    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    // sanity check
    if( NULL == hBitmap )
        return DIERR_INVALIDPARAM;

    // create a memory DC compatible with the current display
    hDCMem = CreateCompatibleDC( NULL );
    // select the surface to drawn on into the HDC
    hOldBitmap = SelectObject( hDCMem, hBitmap );

    // render, passing in as much data as we can safely acquire
    if( bIsDib && GetObject( hBitmap, sizeof( DIBSECTION ), &dibs ) )
        hr = RenderView( hDCMem, dibs.dsBm.bmBits, dibs.dsBm.bmWidth, dibs.dsBm.bmHeight );
    else
        hr = RenderView( hDCMem );

    // clean up
    SelectObject( hDCMem, hOldBitmap );
    DeleteDC( hDCMem );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: RenderView()
// Desc: draws the configuration view to an HDC
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::RenderView( HDC hdc )
{
    return RenderView( hdc, NULL, 0, 0 );
}




//-----------------------------------------------------------------------------
// Name: RenderView()
// Desc: draws the configuration view to an HDC
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::RenderView( HDC hdc, VOID* pBits, INT width, INT height )
{
    DidcvView* pView;
    const DidcvCalloutSet* pSet;
    int iDCStackNum;
    
    // make sure this object has been properly initialized
    if( FALSE == m_bIsInit )
        return DIERR_NOTINITIALIZED;

    if( NULL == hdc )
        return DIERR_INVALIDPARAM;

    pView = m_lpViewManager->GetImage( m_nView );
    
    // no valid view set
    if( NULL == pView )
        return DIERR_NOTINITIALIZED;


    // set rendering options for HDC
    HBRUSH hBkBrush = CreateSolidBrush( m_crHighlight );
    HPEN hLinePen = CreatePen( PS_SOLID, 0, m_crFore );
    HPEN hHighlightPen = CreatePen( PS_SOLID, 0, m_crHighlight );
    HPEN hHighlightLinePen = CreatePen( PS_SOLID, 0, m_crHighlightLine );

    // save the state of the DC on the context stack
    iDCStackNum = SaveDC( hdc );

    // select the new highlight background color brush
    SelectObject( hdc, hBkBrush );
    // select the callout line color
    SelectObject( hdc, hLinePen );
    // set text color
    SetTextColor( hdc, m_crFore );
    // set background color
    SetBkColor( hdc, m_crBack );
    // shift the origin
    SetViewportOrgEx( hdc, m_ptOrigin.x, m_ptOrigin.y, NULL );
    // the bk mode for drawing text
    SetBkMode( hdc, TRANSPARENT );

    // draw the current view
    pView->Render( hdc, pBits, width, height, &m_ptOrigin );

    // get the set of callouts associated with the current view
    pSet = m_lpCalloutManager->GetCalloutSetByView( m_nView );

    if( pSet )
    {
        // Get the list of callout in the set.
        // A callout is the line drawn from each axis/button to a label
        const GwArray <DidcvCallout *> & arr = pSet->GetInternalArrayRef();
        const DidcvCalloutData* pData;

        UINT i;
        BOOL bCalloutEmpty;
        DWORD dwNumPts = 0;
        const POINT* lpPts;
        TCHAR szBuffer[MAX_PATH];
        TCHAR szCopy[MAX_PATH];
        DidcvActionMap* lpActionMap;
        DWORD dwTextFlags, dwTextAlign;

        if( m_hFont )
        {
            // set the font
            SelectObject( hdc, m_hFont );
        }

        // for each callout (the line drawn from each axis/button to a label)
        for( i = 0; i < arr.Size(); i++ )
        {
            // the data associated
            pData = arr[i]->GetDataRef();
            const RECT* rect = &( arr[i]->GetCalloutRect() );

            // get the action map, if one exists
            lpActionMap = pData->lpActionMap;
            if( lpActionMap && lpActionMap->dia.lptszActionName )
                bCalloutEmpty = FALSE;
            else
                bCalloutEmpty = TRUE;

            // draw the callout for the control
            if( !bCalloutEmpty || pData->lpState->bDrawEmptyCallout )
            {
                // draw the highlight first
                if( pData->lpState->bDrawHighlight )
                {
                    // draw the highlight
                    SelectObject( hdc, hHighlightPen );
                    Rectangle( hdc, rect->left, rect->top, rect->right, rect->bottom );

                    // set pen to highlight line
                    SelectObject( hdc, hHighlightLinePen );

                    // set back ground color to highlight color
                    SetBkColor( hdc, m_crHighlight );

                    // set the text color to highlightline color
                    // SetTextColor( hdc, m_crHighlightLine );
                }

                // draw the overlay image
                if( pData->lpState->bDrawOverlay )
                {
                    arr[i]->DrawOverlay( hdc, pBits, width, height, &m_ptOrigin );
                }

                // draw the callout line and text
                if( pData->lpState->bDrawCallout )
                {
                    // draw the callout line
                    lpPts = arr[i]->GetCalloutLine( &dwNumPts );
                    DidcvPolyLineArrow( hdc, lpPts,( INT ) dwNumPts, FALSE );

                    // set the draw text options
                    dwTextFlags = DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS 
                                | DT_EDITCONTROL | DT_MODIFYSTRING;

                    if( !bCalloutEmpty )
                    {
                        // copy the text
                        _tcscpy( szBuffer, lpActionMap->dia.lptszActionName );

                        // reset the text flags
                        dwTextAlign = arr[i]->GetTextAlign();

                        // check horizontal alignment
                        if( dwTextAlign & DIDAL_LEFTALIGNED )
                            dwTextFlags |= DT_LEFT;
                        else if( dwTextAlign & DIDAL_RIGHTALIGNED )
                            dwTextFlags |= DT_RIGHT;
                        else //( dwTextAlign & DIDAL_CENTERED )
                            dwTextFlags |= DT_CENTER;

                        // check vertical alignment
                        if( dwTextAlign & DIDAL_TOPALIGNED )
                            dwTextFlags |= DT_TOP;
                        else if( dwTextAlign & DIDAL_BOTTOMALIGNED )
                            dwTextFlags |= DT_SINGLELINE | DT_BOTTOM;
                        else //( dwTextAlign & DIDAL_MIDDLE )
                            dwTextFlags |= DT_SINGLELINE | DT_VCENTER;

                    }
                    else
                    {
                        // copy and draw the default text
                        _tcscpy( szBuffer, m_tszDefaultText );
                        dwTextFlags |= DT_VCENTER | DT_SINGLELINE;
                    }

                    // make a copy
                    _tcscpy( szCopy, szBuffer );

                    // draw the text
                    DrawText( hdc, szCopy, -1,( LPRECT ) rect, dwTextFlags );

                    // rendered string truncated
                    if( pData->lpState->bDrawFullname && _tcscmp( szCopy, szBuffer ) )
                    {
                        RenderFullname( hdc, szBuffer, rect );
                    }

                    if( pData->lpState->bDrawHighlight )
                    {
                        // reset the pen
                        SelectObject( hdc, hLinePen );

                        // reset the back ground color
                        SetBkColor( hdc, m_crBack );

                        // reset the text color
                        // SetTextColor( hdc, m_crFore );
                    }
                }
            }
        }
    }

    // restore DC state
    RestoreDC( hdc, iDCStackNum );

    // clean up
    DeleteObject( hBkBrush );
    DeleteObject( hLinePen );
    DeleteObject( hHighlightPen );
    DeleteObject( hHighlightLinePen );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderFullname()
// Desc: renders a box with the full name inside
//-----------------------------------------------------------------------------
VOID DIDCfgView::RenderFullname( HDC hdc, const TCHAR* pctszFullname, 
                                 const RECT* pCalloutRect )
{
    HFONT hFont, hOldFont;
    HBRUSH hBrush, hOldBrush;
    COLORREF crOldTextColor;
    TEXTMETRIC tm;
    RECT rect;
    DWORD dwTextFlags;
    int len, width, height;

    // check the length
    len = _tcslen( pctszFullname );
    if( 0 == len )
        return;

    // create the font to use in the box to display the full name
    // we hard-code Courier to ensure we have a fix-width font
    hFont = DidcvCreateFont( hdc, TEXT( "Courier" ), 
                             DIDCV_DEFAULT_FONT_SIZE, 0, 0, FALSE );

    // use the foreground color as the background
    hBrush = CreateSolidBrush( m_crFore );
    // and vice versa
    crOldTextColor = SetTextColor( hdc, m_crBack );

    hOldFont =( HFONT ) SelectObject( hdc, hFont );
    hOldBrush =( HBRUSH ) SelectObject( hdc, hBrush );

    // get the width and height of the text
    GetTextMetrics( hdc, &tm );
    width = tm.tmAveCharWidth * len + 8;
    height = tm.tmHeight + tm.tmExternalLeading + 4;

    // for now draw over the callout rect
    rect.left = pCalloutRect->left;
    rect.top = pCalloutRect->top;
    rect.right = rect.left + width;
    rect.bottom = pCalloutRect->bottom;

    // draw the rectangle
    Rectangle( hdc, rect.left, rect.top, rect.right, rect.bottom );

    // draw the text
    dwTextFlags = DT_NOPREFIX | DT_EDITCONTROL | DT_CENTER | 
                  DT_VCENTER | DT_SINGLELINE;
    DrawText( hdc, pctszFullname, -1, &rect, dwTextFlags );

    // restore HDC objects
    SelectObject( hdc, hOldFont );
    SelectObject( hdc, hOldBrush );
    SetTextColor( hdc, crOldTextColor );

    // clean up
    DeleteObject( hFont );
    DeleteObject( hBrush );
}




//-----------------------------------------------------------------------------
// Name: InitAlloc()
// Desc: helper function to dynamically allocate memory for internal data
//       structures.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::InitAlloc()
{
    // allocate a view manager, callout manager
    m_lpViewManager = new DidcvViewManager;
    m_lpCalloutManager = new DidcvCalloutManager;

    // ensure that memory has been allocated
    if( NULL == m_lpViewManager || NULL == m_lpCalloutManager )
    {
        CleanUp();
        return DIERR_OUTOFMEMORY;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitImageInfoRetrieve()
// Desc: helper function to retrieve callout / image data from
//       DirectInput
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::InitImageInfoRetrieve( LPDIRECTINPUTDEVICE8 lpDidev )
{
    HRESULT hr;

    // allocate a DIDEVICEIMAGEINFOHEADER
    m_lpDidImgHeader = new DIDEVICEIMAGEINFOHEADER;

    // make sure memory has been allocated
    if( NULL == m_lpDidImgHeader )
    {
        return DIERR_OUTOFMEMORY;
    }

    // zero out the DIDEVICEIMAGEINFOHEADER
    ZeroMemory( m_lpDidImgHeader, sizeof( DIDEVICEIMAGEINFOHEADER ) );

    // properly initialize the structure before it can be used
    m_lpDidImgHeader->dwSize = sizeof( DIDEVICEIMAGEINFOHEADER );
    m_lpDidImgHeader->dwSizeImageInfo = sizeof( DIDEVICEIMAGEINFO );

    // since m_lpDidImgHeader->dwBufferSize is 0, this call serves to determine
    // the minimum buffer size required to hold information for all the images
    hr = lpDidev->GetImageInfo( m_lpDidImgHeader );
    if( FAILED( hr ) )
    {
        return DIERR_INVALIDPARAM; 
    }

    // at this point, m_lpDidImgHeader->dwBufferUsed has been set by
    // the GetImageInfo method to minimum buffer size needed, so allocate.
    m_lpDidImgHeader->dwBufferSize = m_lpDidImgHeader->dwBufferUsed;
    m_lpDidImgHeader->lprgImageInfoArray = new DIDEVICEIMAGEINFO[ m_lpDidImgHeader->dwBufferSize / 
                                                                  sizeof( DIDEVICEIMAGEINFO ) ];

    // make sure memory has been allocated
    if( m_lpDidImgHeader->lprgImageInfoArray == NULL )
    {
        return DIERR_OUTOFMEMORY;
    }

    // now that the dwBufferSize has been filled, and lprgImageArray allocated,
    // we call GetImageInfo again to get the image data
    hr = lpDidev->GetImageInfo( m_lpDidImgHeader );
    if( FAILED( hr ) )
    {
        return DIERR_INVALIDPARAM;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitImageInfoProcess()
// Desc: helper function to process the image/callout information to
//       get them ready to be used.
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::InitImageInfoProcess()
{
    DWORD num_elements, i;
    DidcvView* pView = NULL;
    BOOL bLoadComplete = TRUE;
    DidcvCallout* pCallout = NULL;
    DWORD dwNumViews, dwNumCallouts, dwNumUniqueObjIDs;

    // a pre-condition of this method is that m_lpDidImgHeader != NULL
    assert( m_lpDidImgHeader != NULL );

    // calcuate the number of elements in the image info array
    num_elements = m_lpDidImgHeader->dwBufferUsed / m_lpDidImgHeader->dwSizeImageInfo;

    // determines sizes ahead of time
    CalcSizes( m_lpDidImgHeader->lprgImageInfoArray, num_elements, &dwNumViews, 
               &dwNumCallouts, &dwNumUniqueObjIDs );

    // set the capacity of the managers ahead of time
    m_lpViewManager->SetCapacity( dwNumViews );
    m_lpCalloutManager->SetCapacity( dwNumCallouts, dwNumUniqueObjIDs, dwNumViews );

    // first pass to process all configuration image, in order
    for( i = 0; i < num_elements; i++ )
    {
        // check to see if the element is a configuration image
        if( m_lpDidImgHeader->lprgImageInfoArray[i].dwFlags & DIDIFT_CONFIGURATION )
        {
            pView = DidcvBitmapView::Create( m_lpDidImgHeader->lprgImageInfoArray[i].tszImagePath,
                                             m_lpDidImgHeader->lprgImageInfoArray[i].dwFlags );

            if( NULL == pView )
            {
                bLoadComplete = FALSE;
                continue;
            }
            
            // add new image to the ImageManager, along with the array index, 
            // which is referred by callout entries
            m_lpViewManager->AddImage( pView,( INT )i );
        }
    }

    // second pass to process all overlays
    for( i = 0; i < num_elements; i++ )
    {
        // check to see if the element is a control overlay entry
        if( m_lpDidImgHeader->lprgImageInfoArray[i].dwFlags & DIDIFT_OVERLAY )
        {
            pCallout = DidcvCallout::Create( &( m_lpDidImgHeader->lprgImageInfoArray[i] ) );

            if( NULL == pCallout )
            {
                bLoadComplete = FALSE;
                continue;
            }
            
            // try to get the appropriate view, by the offset in the image array
            pView = m_lpViewManager->GetImageByOffset( m_lpDidImgHeader->lprgImageInfoArray[i].dwViewID );

            if( NULL == pView )
            {
                bLoadComplete = FALSE;
                continue;
            }

            // add new image to the ImageManager, along with the array index, 
            // which is referred by callout entries
            m_lpCalloutManager->AddCallout( pCallout, pView->GetID() );
        }
    }

    // free unused dynamic memory
    m_lpCalloutManager->TrimArrays();

    // appropriate error code for bLoadComplete != TRUE is not returned at this time
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CalcSizes()
// Desc: helper function to determine the number of views and callouts in
//       in the image info array.
//-----------------------------------------------------------------------------
VOID DIDCfgView::CalcSizes( const DIDEVICEIMAGEINFO* lprgImageInfoArray, 
                            DWORD dwNumElements, LPDWORD lpNumViews, 
                            LPDWORD lpNumCallouts, LPDWORD lpNumDistinctObjID )
{
    DWORD i, j;

    // set everything to 0
    *lpNumViews = *lpNumCallouts = *lpNumDistinctObjID = 0;

    for( i = 0; i < dwNumElements; i++ )
    {
        // figure out how many of each
        if( lprgImageInfoArray[i].dwFlags & DIDIFT_CONFIGURATION )
          ( *lpNumViews ) ++;
        else if( lprgImageInfoArray[i].dwFlags & DIDIFT_OVERLAY )
        {
          ( *lpNumCallouts ) ++;

            // find to see if dwObjID unique so far
            for( j = 0; j < i; j++ )
                if( lprgImageInfoArray[j].dwObjID == lprgImageInfoArray[i].dwObjID )
                    break;

            if( j == i ) // current id unique
              ( *lpNumDistinctObjID )++;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: enum DidcvEnumDataType
// Desc: constants for the dwType member of the DidcvEnumDeviceData
//-----------------------------------------------------------------------------
enum DidcvEnumDataType
{
    DIDCV_EDT_COUNT = 1,    // count
    DIDCV_EDT_SAVE  = 2     // save each item
};




//-----------------------------------------------------------------------------
// Name: IsObjectOnExcludeList()
// Desc: Exclude these objects from the view
//-----------------------------------------------------------------------------
BOOL IsObjectOnExcludeList( DWORD dwOfs )
{
	if (dwOfs == DIK_PREVTRACK  ||
	    dwOfs == DIK_NEXTTRACK  ||
	    dwOfs == DIK_MUTE       ||
	    dwOfs == DIK_CALCULATOR ||
	    dwOfs == DIK_PLAYPAUSE  ||
	    dwOfs == DIK_MEDIASTOP  ||
	    dwOfs == DIK_VOLUMEDOWN ||
	    dwOfs == DIK_VOLUMEUP   ||
	    dwOfs == DIK_WEBHOME    ||
	    dwOfs == DIK_SLEEP      ||
	    dwOfs == DIK_WEBSEARCH  ||
	    dwOfs == DIK_WEBFAVORITES ||
	    dwOfs == DIK_WEBREFRESH ||
	    dwOfs == DIK_WEBSTOP    ||
	    dwOfs == DIK_WEBFORWARD ||
	    dwOfs == DIK_WEBBACK    ||
	    dwOfs == DIK_MYCOMPUTER ||
	    dwOfs == DIK_MAIL       ||
	    dwOfs == DIK_MEDIASELECT)
		return TRUE;

	return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DidcvEnumDeviceObjectsCallback()
// Desc: enumerates the objects( keys) on an device
//-----------------------------------------------------------------------------
BOOL CALLBACK DidcvEnumDeviceObjectsCallback( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef )
{
    // get the struct that is passed in
    DidcvCustomViewInfo* lpData =( DidcvCustomViewInfo* ) pvRef;
    if( NULL == lpData )
        return DIENUM_STOP;

    // Exclude certain keys on advanced keyboards
    if( IsObjectOnExcludeList( lpddoi->dwOfs ) )
        return DIENUM_CONTINUE;

    if( lpData->dwType == DIDCV_EDT_COUNT )
    {
        // simply count
        lpData->dwCount++;
    }
    else if( lpData->dwType == DIDCV_EDT_SAVE )
    {
        if( lpData->dwSize < lpData->dwCount )
        {
            // instantiate new object instance
            lpData->rgObjData[lpData->dwSize] = new DIDEVICEOBJECTINSTANCE;
            memcpy( lpData->rgObjData[lpData->dwSize], lpddoi, sizeof( DIDEVICEOBJECTINSTANCE ) );
            lpData->dwSize++;
        }
        else
        {
            // this should not happen
            return DIENUM_STOP;
        }
    }
    else
        return DIENUM_STOP;

    return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: InitCustomViews()
// Desc: initializes custom views by retrieving device data, and calling
//       ConstructCustomView() to make views using bitmap images for
//       the device controls 
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::InitCustomViews( LPDIRECTINPUTDEVICE8 lpDevice, BOOL bUseInternal )
{
    HRESULT hr;

    // reset the info
    if( m_lpCustomViewInfo )
        m_lpCustomViewInfo->CleanUp();
    else
    {
        m_lpCustomViewInfo = new DidcvCustomViewInfo;
        if( NULL == m_lpCustomViewInfo )
            return DIERR_OUTOFMEMORY;
    }

    // get a count of how many controls there are
    m_lpCustomViewInfo->dwType = DIDCV_EDT_COUNT;

    if( bUseInternal )
    {
        if( m_lpCalloutManager == NULL )
            return DIERR_INVALIDPARAM;

        // count internal objects that have action mapping
        m_lpCalloutManager->EnumObjects( lpDevice, DidcvEnumDeviceObjectsCallback, 
                                         m_lpCustomViewInfo, TRUE );
    }
    else
    {
        // count from device
        lpDevice->EnumObjects( DidcvEnumDeviceObjectsCallback, m_lpCustomViewInfo, DIDFT_ALL );
    }

    if( m_lpCustomViewInfo->dwCount == 0 )
    {
        m_lpCustomViewInfo->CleanUp();
        return DIERR_INVALIDPARAM;
    }

    // do a second time to get the objects
    m_lpCustomViewInfo->dwType = DIDCV_EDT_SAVE;
    m_lpCustomViewInfo->rgObjData = new LPDIDEVICEOBJECTINSTANCE[m_lpCustomViewInfo->dwCount];
    if( NULL == m_lpCustomViewInfo->rgObjData )
    {
        m_lpCustomViewInfo->CleanUp();
        return DIERR_OUTOFMEMORY;
    }

    if( bUseInternal )
    {
        if( m_lpCalloutManager == NULL )
            return DIERR_INVALIDPARAM;

        // get the internal objects that have action mapping
        m_lpCalloutManager->EnumObjects( lpDevice, DidcvEnumDeviceObjectsCallback, 
                                         m_lpCustomViewInfo, TRUE );
    }
    else
    {
        // get a list of all objects for the device
        lpDevice->EnumObjects( DidcvEnumDeviceObjectsCallback, 
                               m_lpCustomViewInfo, DIDFT_ALL );
    }

    if( m_lpCustomViewInfo->dwSize == 0 )
    {
        m_lpCustomViewInfo->CleanUp();
        return DIERR_INVALIDPARAM;
    }

    // build the custom views
    if( FAILED( hr = BuildCustomViews() ) )
    {
        m_lpCustomViewInfo->CleanUp();
        return hr;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BuildCustomViews()
// Desc: makes custom views
//-----------------------------------------------------------------------------
HRESULT DIDCfgView::BuildCustomViews()
{
    DidcvView* pView;
    DidcvCallout* pCallout;
    DWORD dwNumViews, dwStartIndex, dwFinishIndex;
    DWORD i, j;
    DIDEVICEIMAGEINFO devImageInfo;

    // pre-condition of this method - m_lpCustomViewInfo must be loaded
    assert( m_lpCustomViewInfo && m_lpCustomViewInfo->rgObjData );

    // calculate the number of views needed
    dwNumViews = DidcvCustomView::CalcNumViews( m_lpCustomViewInfo->dwSize );
    assert( dwNumViews );

    // reset
    m_lpViewManager->CleanUp();
    m_lpCalloutManager->CleanUp();

    // set the initial capacity
    if( FALSE == m_lpViewManager->SetCapacity( dwNumViews ) )
        return DIERR_OUTOFMEMORY;

    if( FALSE == m_lpCalloutManager->SetCapacity( m_lpCustomViewInfo->dwSize, 
                                                  m_lpCustomViewInfo->dwSize, dwNumViews ) )
        return DIERR_OUTOFMEMORY;

    dwStartIndex = 0;
    for( i = 0; i < dwNumViews; i++ )
    {
        // create views, each for a section of object array
        pView = DidcvCustomView::Create( m_lpCustomViewInfo, dwStartIndex, 
                                         &dwFinishIndex );
        if( NULL == pView )
            return DIERR_OUTOFMEMORY;

        // add the view to the manager
        if( FAILED( m_lpViewManager->AddImage( pView ) ) )
        {
            delete pView;
            continue;
        }

        for( j = dwStartIndex; j <= dwFinishIndex; j++ )
        {
            ZeroMemory( &devImageInfo, sizeof( DIDEVICEIMAGEINFO ) );

            // construct our own image info
            devImageInfo.dwFlags = DIDIFT_OVERLAY;
            // figure out the callout information for each object
            DidcvCustomView::CalcImageInfo( j, &( devImageInfo.rcOverlay ), 
                                            &( devImageInfo.dwcValidPts ),
                                            devImageInfo.rgptCalloutLine, 
                                            &( devImageInfo.rcCalloutRect ) );
            devImageInfo.dwTextAlign = DIDAL_LEFTALIGNED;
            devImageInfo.dwObjID = m_lpCustomViewInfo->rgObjData[j]->dwType;

            // create the callout and add
            pCallout = DidcvCallout::Create( &devImageInfo );
            m_lpCalloutManager->AddCallout( pCallout, pView->GetID() );
        }

        dwStartIndex = dwFinishIndex + 1;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CopyActionMap()
// Desc: copies a DIACTIONFORMAT
//-----------------------------------------------------------------------------
BOOL DIDCfgView::CopyActionMap( LPDIACTIONFORMAT lpDiaf )
{
    if( m_lpDiaf )
    {
        // free the DIACTIONFORMAT
        if( m_lpDiaf->rgoAction )
            delete [] m_lpDiaf->rgoAction;
        delete m_lpDiaf;
        m_lpDiaf = NULL;
    }

    if( lpDiaf == NULL )
        return TRUE;

    m_lpDiaf = new DIACTIONFORMAT;
    if( m_lpDiaf == NULL )
        return FALSE;

    // copy the DIACTIONFORMAT
    memcpy( m_lpDiaf, lpDiaf, sizeof( DIACTIONFORMAT ) );
    if( lpDiaf->rgoAction )
    {
        // copy the internal array
        m_lpDiaf->rgoAction = new DIACTION[lpDiaf->dwNumActions];
        if( m_lpDiaf->rgoAction == NULL )
            return FALSE;

        memcpy( m_lpDiaf->rgoAction, lpDiaf->rgoAction, 
                sizeof( DIACTION ) * lpDiaf->dwNumActions );
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: release the resource for the CustomViewInfo
//-----------------------------------------------------------------------------
void DidcvCustomViewInfo::CleanUp()
{
    // free the custom view data
    if( rgObjData )
    {
        DWORD i;
        // free each item
        for( i = 0; i < dwSize; i++)
            if( rgObjData[i] )
                delete rgObjData[i];

        // free the array
        delete [] rgObjData;
        rgObjData = NULL;
    }

    dwCount = 0;
    dwSize = 0;
}




//-----------------------------------------------------------------------------
// Name: DidcvBitmap()
// Desc: Constructor
//-----------------------------------------------------------------------------
DidcvBitmap::DidcvBitmap()
    : m_hbitmap( NULL ),
      m_lpBits( NULL )
{
    ZeroMemory( &m_size, sizeof( SIZE ) );

    // register with the alpha blend module
    DidcvAlphaBlend::AddClient();
}




//-----------------------------------------------------------------------------
// Name: ~DidcvBitmap()
// Desc: Destructor
//-----------------------------------------------------------------------------
DidcvBitmap::~DidcvBitmap()
{
    CleanUp();

    // release the alpha blending module
    DidcvAlphaBlend::ReleaseClient();
}




//-----------------------------------------------------------------------------
// Name: GetSize()
// Desc: retrieves the size of the bitmap - return FALSE if there is no bitmap
//-----------------------------------------------------------------------------
BOOL DidcvBitmap::GetSize( SIZE* lpSize ) const
{
    if( NULL == m_hbitmap )
        return FALSE;

    memcpy( lpSize, &m_size, sizeof( SIZE ) );
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: Draw()
// Desc: Draws onto a HDC, at( xStart, yStart )
//-----------------------------------------------------------------------------
BOOL DidcvBitmap::Draw( HDC hDC, INT xStart, INT yStart )
{
    if( NULL == m_hbitmap )
        return FALSE;

    BITMAP bm;
    HDC hdcMem;
    POINT ptSize, ptOrg;

    // create memory DC
    hdcMem = CreateCompatibleDC( hDC );
    SelectObject( hdcMem, m_hbitmap );
    SetMapMode( hdcMem, GetMapMode( hDC ) );

    // get bitmap info
    GetObject( m_hbitmap, sizeof( BITMAP ),( LPVOID ) &bm );

    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP( hDC, &ptSize, 1 );

    ptOrg.x = 0;
    ptOrg.y = 0;
    DPtoLP( hdcMem, &ptOrg, 1 );

    // blit
    BitBlt( hDC, xStart, yStart, ptSize.x, ptSize.y, 
            hdcMem, ptOrg.x, ptOrg.y, SRCCOPY );

    // free the memory DC
    DeleteDC( hdcMem );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: Blend()
// Desc: draw the bitmap into HDC with alpha blending( xStart, yStart )
//-----------------------------------------------------------------------------
BOOL DidcvBitmap::Blend( HDC hDC, INT xStart, INT yStart )
{
    return DidcvAlphaBlend::Blend( hDC, xStart, yStart, 
                                   m_size.cx, m_size.cy, m_hbitmap, &m_size );
}




//-----------------------------------------------------------------------------
// Name: Blend()
// Desc: blend this bitmap unto the image
//-----------------------------------------------------------------------------
BOOL DidcvBitmap::Blend( VOID* lpBits, INT xStart, INT yStart, INT width, INT height )
{
    return DidcvAlphaBlend::Blend( lpBits, xStart, yStart, width, height, 
                                   m_lpBits, m_size.cx, m_size.cy );
}




//-----------------------------------------------------------------------------
// Name: GetHandle()
// Desc: return the GDI HBITMAP handle
//-----------------------------------------------------------------------------
HBITMAP DidcvBitmap::GetHandle()
{
    return m_hbitmap;
}




//-----------------------------------------------------------------------------
// Name: GetBits()
// Desc: returns a pointer to the bits
//-----------------------------------------------------------------------------
LPVOID DidcvBitmap::GetBits()
{
    return m_lpBits;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: static function to instantiate a bitmap object from a pathname
//-----------------------------------------------------------------------------
DidcvBitmap* DidcvBitmap::Create( LPCTSTR tszFilename )
{
    return CreateViaD3dx( tszFilename );
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: static function to instantiate a bitmap object from dimensions
//-----------------------------------------------------------------------------
DidcvBitmap* DidcvBitmap::Create( INT width, INT height )
{
    BITMAPINFO bmi;
    DidcvBitmap* bm;
    HBITMAP hbm;
    VOID* lpBits;

    // sanity check
    if( width <= 0 || height <= 0 )
        return NULL;

    // make the header
    ZeroMemory( &bmi, sizeof( BITMAPINFO ) );
    bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    // create a DIBSection
    hbm = CreateDIBSection( NULL, &bmi, DIB_RGB_COLORS, &lpBits, NULL, 0 );
    if( hbm == NULL )
        return NULL;

    // allocate an object
    bm = new DidcvBitmap;
    if( NULL == bm )
        return NULL;

    // set the members
    bm->m_hbitmap;
    bm->m_lpBits = lpBits;
    bm->m_size.cx = width;
    bm->m_size.cy = height;

    return bm;
}





//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: helper function to free resources
//-----------------------------------------------------------------------------
void DidcvBitmap::CleanUp()
{
    if( m_hbitmap )
    {
        DeleteObject( m_hbitmap );
        m_hbitmap = NULL;
    }

    m_lpBits = NULL;
    ZeroMemory( &m_size, sizeof( SIZE ) );
}




//-----------------------------------------------------------------------------
// Name: FigureSize()
// Desc: determines the size of the bitmap
//-----------------------------------------------------------------------------
void DidcvBitmap::FigureSize()
{
    BITMAP bm;

    if( NULL == m_hbitmap )
        return;

    if( 0 == GetObject( ( HGDIOBJ ) m_hbitmap, sizeof( BITMAP ),( LPVOID ) &bm ) )
        return;

    m_size.cx = abs( bm.bmWidth );
    m_size.cy = abs( bm.bmHeight );
}




//-----------------------------------------------------------------------------
// static members of DidcvAlphaBlend
//-----------------------------------------------------------------------------
DIDCV_ALPHABLEND DidcvAlphaBlend::s_alphaBlendProc = NULL;
HMODULE          DidcvAlphaBlend::s_hDll = NULL;
DWORD            DidcvAlphaBlend::s_dwNumClients = 0;




//-----------------------------------------------------------------------------
// Name: AddClient()
// Desc: static alpha blending initialization function
//       this bumps the reference count by one for each time it is called
//-----------------------------------------------------------------------------
BOOL DidcvAlphaBlend::AddClient()
{
    // try to the AlphaBlend function
    if( s_alphaBlendProc == NULL )
    {
        s_hDll = LoadLibrary( DIDCV_ALPHABLEND_DLL_NAME );

        if( s_hDll )
        {
            s_alphaBlendProc =( DIDCV_ALPHABLEND ) GetProcAddress( s_hDll, 
                                DIDCV_ALPHABLEND_PROC_NAME );
            if( !s_alphaBlendProc )
            { 
                FreeLibrary( s_hDll );
                s_hDll = NULL;
            }
        }
    }

    // bump the reference count
    s_dwNumClients ++;

    return( s_alphaBlendProc != NULL );
}




//-----------------------------------------------------------------------------
// Name: ReleaseClient()
// Desc: static alpha blend function called when a client detach
//-----------------------------------------------------------------------------
BOOL DidcvAlphaBlend::ReleaseClient()
{
    if( s_dwNumClients )
        s_dwNumClients --;

    // if reference count 0, then free resources
    if( s_dwNumClients == 0 )
    {
        s_alphaBlendProc = NULL;

        if( s_hDll )
        {
            FreeLibrary( s_hDll );
            s_hDll = NULL;
        }
    }

    return TRUE;
}



//-----------------------------------------------------------------------------
// Name: Blend()
// Desc: blends images by hand
//-----------------------------------------------------------------------------
BOOL DidcvAlphaBlend::Blend( VOID* lpDestBits, INT xStart, INT yStart, 
                             INT destWidth, INT destHeight, VOID* lpSrcBits, 
                             INT srcWidth, INT srcHeight )
{
    if( !lpDestBits || !lpSrcBits )
        return FALSE;

    // calculate the strides
    INT destStride = destWidth * 4;
    INT destOffset = xStart * 4;
    INT srcStride = srcWidth * 4;
    INT i, j;

    // clip the rectangle to draw to if necessary
    if( ( xStart + srcWidth ) >( destWidth ) )
        srcWidth = destWidth - xStart;
    if( ( yStart + srcHeight ) >( destHeight ) )
        srcHeight = destHeight - yStart;

    // the DIB bits are is indexed from bottom, left corner, so need to reverse horizontally
    BYTE* pDestStart = ( BYTE* ) lpDestBits + 
                       ( ( destHeight - yStart - 1 ) * destStride ) + destOffset;
    BYTE* pSrcStart =( BYTE* ) lpSrcBits + ( srcHeight - 1 ) * srcStride;
    BYTE destAlpha, srcAlpha;

    for( i = 0; i < srcHeight; i++ )
    {
        for( j = 0; j < srcStride; j += 4 )
        {
            // calculate src and dest alpha
            destAlpha = 0xff - pSrcStart[j + 3];
            srcAlpha = pSrcStart[j + 3];

            if( !srcAlpha ) continue;
            if( !destAlpha )
                *(( DWORD* )&pDestStart[j]) = *((DWORD *)&pSrcStart[j]);
            else
            {
                // set the R, G, B components
                pDestStart [j] =( pDestStart[j] * destAlpha + pSrcStart[j] * srcAlpha ) / 0xff;
                pDestStart [j+1] =( pDestStart[j+1] * destAlpha + pSrcStart[j+1] * srcAlpha ) / 0xff;
                pDestStart [j+2] =( pDestStart[j+2] * destAlpha + pSrcStart[j+2] * srcAlpha ) / 0xff;
            }
        }

        // advance the pointers
        pDestStart -= destStride;
        pSrcStart -= srcStride;
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: Blend()
// Desc: blends a bitmap into the HDC
//-----------------------------------------------------------------------------
BOOL DidcvAlphaBlend::Blend( HDC hDC, INT xStart, INT yStart, INT width, 
                             INT height, HBITMAP hbitmap, const SIZE* lpSize )
{
    if( hDC == NULL || hbitmap == NULL || lpSize == NULL )
        return FALSE;

#if( WINVER >= 0x400 )

    HDC hDCbm = CreateCompatibleDC( hDC );
    if( hDCbm == NULL )
        return FALSE;

// this is done to resolve differences in header files
#ifdef AC_SRC_ALPHA
    BLENDFUNCTION blendfn = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
#else
    BLENDFUNCTION blendfn = { AC_SRC_OVER, 0, 255, 0x1 };
#endif

    HGDIOBJ hOldBitmap = SelectObject( hDCbm, hbitmap );
    BOOL bRet = s_alphaBlendProc( hDC, xStart, yStart, width, height, 
                                  hDCbm, 0, 0, lpSize->cx, lpSize->cy, blendfn );
    SelectObject( hDCbm, hOldBitmap ); 
    DeleteDC( hDCbm );

    return TRUE;

#else

    return FALSE;

#endif

}




//-----------------------------------------------------------------------------
// Name: DidcvCallout()
// Desc: Constructor
//-----------------------------------------------------------------------------
DidcvCallout::DidcvCallout()
    : m_nViewRef( DIDCV_INVALID_ID ),
      m_lpDataRef( NULL ),
      m_lpOverlayBitmap( NULL )
{
    ZeroMemory( &m_devImgInfo, sizeof( DIDEVICEIMAGEINFO ) );
}




//-----------------------------------------------------------------------------
// Name: ~DidcvCallout()
// Desc: destructor
//-----------------------------------------------------------------------------
DidcvCallout::~DidcvCallout()
{
    if( m_lpOverlayBitmap )
    {
        delete m_lpOverlayBitmap;
        m_lpOverlayBitmap = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: GetObjID()
// Desc: returns the object ID
//-----------------------------------------------------------------------------
DWORD DidcvCallout::GetObjID() const
{
    return m_devImgInfo.dwObjID;
}




//-----------------------------------------------------------------------------
// Name: GetOverlayRect()
// Desc: returns the overlay rect for the callout
//-----------------------------------------------------------------------------
const RECT & DidcvCallout::GetOverlayRect() const
{
    return m_devImgInfo.rcOverlay;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutRect()
// Desc: returns the callout rect, where the action text is displayed
//-----------------------------------------------------------------------------
const RECT & DidcvCallout::GetCalloutRect() const
{
    return m_devImgInfo.rcCalloutRect;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutLine()
// Desc: returns the callout line segments as a const array
//-----------------------------------------------------------------------------
const POINT* DidcvCallout::GetCalloutLine( DWORD* lpNumPts ) const
{
    if( NULL != lpNumPts )
        *lpNumPts = m_devImgInfo.dwcValidPts;

    return m_devImgInfo.rgptCalloutLine;
}




//-----------------------------------------------------------------------------
// Name: GetTextAlign()
// Desc: return the text alignment for the callout
//-----------------------------------------------------------------------------
DWORD DidcvCallout::GetTextAlign() const
{
    return m_devImgInfo.dwTextAlign;
}




//-----------------------------------------------------------------------------
// Name: GetOverlayOffset()
// Desc: returns the index of the callout in the original image info array
//       returned by DirectInputDevice8::GetImage()
//-----------------------------------------------------------------------------
DWORD DidcvCallout::GetOverlayOffset() const
{
    return m_devImgInfo.dwViewID;
}




//-----------------------------------------------------------------------------
// Name: GetAssociatedViewID()
// Desc: returns the internally assigned( by DIDCfgView ) identifier of a
//       view that this callout is assicated with
//-----------------------------------------------------------------------------
INT DidcvCallout::GetAssociatedViewID() const
{
    return m_nViewRef;
}




//-----------------------------------------------------------------------------
// Name: SetAssociatedViewID()
// Desc: sets the view with which this callout is associated with
//-----------------------------------------------------------------------------
void DidcvCallout::SetAssociatedViewID( INT nViewRef )
{
    m_nViewRef = nViewRef;
}




//-----------------------------------------------------------------------------
// Name: HitTest()
// Desc: tests to determines whether a point is contained the associated
//       regions of this callout.  coordinates are relative to the left top
//       corner of the image
//-----------------------------------------------------------------------------
DWORD DidcvCallout::HitTest( LPPOINT lpPt ) const
{
    if( NULL == lpPt )
        return FALSE;

    // if in overlay rectangle, then hit
    //    if( PtInRect( &( m_devImgInfo.rcOverlay ), *lpPt ) )
    //        return TRUE;

    // if in callout rectangle, then hit
    if( PtInRect( &( m_devImgInfo.rcCalloutRect ), *lpPt ) )
        return TRUE;

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DrawOverlay()
// Desc: draws the overlay image
//-----------------------------------------------------------------------------
DWORD DidcvCallout::DrawOverlay( HDC hDC, VOID* lpBits, INT width, 
                                 INT height, const POINT* ptOrigin )
{
    BOOL res = FALSE;

    if( m_lpOverlayBitmap )
    {
        // alpha blend with pointers only
        if( lpBits )
            res = m_lpOverlayBitmap->Blend( lpBits, 
                        m_devImgInfo.rcOverlay.left + ptOrigin->x, 
                        m_devImgInfo.rcOverlay.top  + ptOrigin->y, 
                        width, height );

        if( !res )
        {
            // try alpha blending with hDC
            res = m_lpOverlayBitmap->Blend( hDC, m_devImgInfo.rcOverlay.left, 
                                            m_devImgInfo.rcOverlay.top );
        }
    }

    return res;
}




//-----------------------------------------------------------------------------
// Name: SetDataRef()
// Desc: attach a pointer to the callout data relevant to this callout
//-----------------------------------------------------------------------------
void DidcvCallout::SetDataRef( const DidcvCalloutData* lpData )
{
    m_lpDataRef = lpData;
}




//-----------------------------------------------------------------------------
// Name: GetDataRef()
// Desc: retrieved the the data reference stored with this callout
//-----------------------------------------------------------------------------
const DidcvCalloutData* DidcvCallout::GetDataRef() const
{
    return m_lpDataRef;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: static function for instantiating a DidcvCallout
//-----------------------------------------------------------------------------
DidcvCallout* DidcvCallout::Create( LPDIDEVICEIMAGEINFO lpDevImgInfo )
{
    DidcvCallout* lpCallout = NULL;

    if( NULL == lpDevImgInfo )
        return NULL;

    // allocate
    lpCallout = new DidcvCallout;

    if( NULL == lpCallout )
        return NULL;

    // copy the device image info
    memcpy( &( lpCallout->m_devImgInfo ), lpDevImgInfo, sizeof( DIDEVICEIMAGEINFO ) );

    if( lpDevImgInfo->tszImagePath )
    {
        // load the overlay
        lpCallout->m_lpOverlayBitmap = DidcvBitmap::Create( lpDevImgInfo->tszImagePath );
    }

    return lpCallout;
}




//-----------------------------------------------------------------------------
// Name: DidcvCalloutManager()
// Desc: Constructor
//-----------------------------------------------------------------------------
DidcvCalloutManager::DidcvCalloutManager() 
    : m_calloutList( DIDCV_DEFAULT_CALLOUT_MANAGER_ARRAY_SIZE ),
      m_calloutSetListByObjID( DIDCV_DEFAULT_CALLOUT_MANAGER_ARRAY_SIZE ),
      m_calloutSetListByView( DIDCV_DEFAULT_CALLOUT_MANAGER_ARRAY_SIZE )
{ }




//-----------------------------------------------------------------------------
// Name: ~DidcvCalloutManager()
// Desc: destructor
//-----------------------------------------------------------------------------
DidcvCalloutManager::~DidcvCalloutManager()
{
    CleanUp();
}




//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: reset the callout manager, freeing resources
//-----------------------------------------------------------------------------
void DidcvCalloutManager::CleanUp()
{
    UINT i;

    // free the entries
    for( i = 0; i < m_calloutList.Size(); i++ )
        if( m_calloutList[i] )
            delete m_calloutList[i];

    // free the view callout set
    for( i = 0; i < m_calloutSetListByView.Size(); i++ )
        if( m_calloutSetListByView[i] )
            delete m_calloutSetListByView[i];

    // free the obj id callout set
    for( i = 0; i < m_calloutSetListByObjID.Size(); i++ )
    {
        if( m_calloutSetListByObjID[i] )
        {
            DidcvCalloutData* lpCalloutData = 
              ( DidcvCalloutData* ) m_calloutSetListByObjID[i]->GetData();

            // is there is data attached, deallocate it
            if( lpCalloutData )
            {
                if( lpCalloutData->lpState )
                    delete lpCalloutData->lpState;

                if( lpCalloutData->lpActionMap )
                    delete lpCalloutData->lpActionMap;

                delete lpCalloutData;
            }

            delete m_calloutSetListByObjID[i];
        }
    }

    // set the array size to 0
    m_calloutList.SetSize( 0 );
    m_calloutSetListByObjID.SetSize( 0 );
    m_calloutSetListByView.SetSize( 0 );
}




//-----------------------------------------------------------------------------
// Name: SetCapacity()
// Desc: sets the capacity in advance
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::SetCapacity( DWORD dwNumCallouts, DWORD dwNumUniqueObjID, 
                                       DWORD dwNumViews, BOOL bDeleteContent )
{
    if( bDeleteContent )
        CleanUp();

    m_calloutList.SetSize( dwNumCallouts );
    m_calloutSetListByObjID.SetSize( dwNumUniqueObjID );
    m_calloutSetListByView.SetSize( dwNumViews );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: AddCallout()
// Desc: adds a callout object to be stored and managed by this object
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::AddCallout( DidcvCallout* lpCallout, INT nView )
{
    DidcvCalloutSet*  lpViewSet  = NULL;
    DidcvCalloutSet*  lpObjIDSet = NULL;
    DidcvCalloutData* lpData     = NULL;

    // sanity check
    if( NULL == lpCallout || lpCallout->GetObjID() == 0 || nView < 0 )
        return FALSE;

    // append to complete list
    m_calloutList.PushBack( lpCallout );

    // ----------- organize by view

    // look to see if CalloutSet exist for the view
    lpViewSet = Find( m_calloutSetListByView,( DWORD ) nView );

    // no set exist - new view
    if( NULL == lpViewSet )
    {
        lpViewSet = new DidcvCalloutSet;
        lpViewSet->SetIdentifier( ( DWORD ) nView );
        m_calloutSetListByView.PushBack( lpViewSet );
    }

    // append callout to the view set
    lpViewSet->AddCallout( lpCallout );

    // ----------- organize by object id

    // look to see if CalloutSet exist for the ObjID
    lpObjIDSet = Find( m_calloutSetListByObjID, lpCallout->GetObjID() );

    if( NULL == lpObjIDSet )
    {
        // new set of callout
        lpObjIDSet = new DidcvCalloutSet;
        lpObjIDSet->SetIdentifier( lpCallout->GetObjID() );
        // allocate new data to be associated
        lpObjIDSet->SetData( lpData = new DidcvCalloutData( 
            new DidcvCalloutState( DIDCV_ALL_CALLOUT_STATE_SET ),
            NULL ) );
        m_calloutSetListByObjID.PushBack( lpObjIDSet );
    }
    else
    {
        lpData =( DidcvCalloutData* ) lpObjIDSet->GetData();
    }

    // append callout
    lpObjIDSet->AddCallout( lpCallout );

    // set data reference
    lpCallout->SetDataRef( lpData );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetObjectIDByLocation()
// Desc: given a point( in image coordinate ) and a view, returns a callout 
//     ( if any ) that contains the point.
//-----------------------------------------------------------------------------
DWORD DidcvCalloutManager::GetObjectIDByLocation( LPPOINT lpPt, INT nView )
{
    UINT i, size;
    DidcvCalloutSet* lpViewSet = NULL;

    // sanity check
    if( NULL == lpPt || nView < 0 )
        return 0;

    // look for the the callout set by view
    lpViewSet = Find( m_calloutSetListByView,( DWORD )nView );

    // if not found, return
    if( NULL == lpViewSet )
        return 0;

    // obtain a const reference, fast + safe
    const GwArray <DidcvCallout *> & arr = lpViewSet->GetInternalArrayRef();

    size = arr.Size();

    // loop through the callouts
    for( i = 0; i < size; i++ )
    {
        // return the first hit
        if( arr[i]->HitTest( lpPt ) != 0 )
            return arr[i]->GetObjID();
    }

    // no hits
    return 0;
}




//-----------------------------------------------------------------------------
// Name: SetCalloutState()
// Desc: set the state for a particular callout
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::SetCalloutState( const DidcvCalloutState* lpCalloutState, 
                                           DWORD dwObjID )
{
    DidcvCalloutData* lpInternalCalloutData;

    // sanity check
    if( NULL == lpCalloutState )
        return FALSE;

    // get the data associated with the object
    lpInternalCalloutData = GetCalloutData( dwObjID );

    // not found
    if( NULL == lpInternalCalloutData )
        return FALSE;

    // set the internal state to the new state
    lpInternalCalloutData->lpState->SmartSet( lpCalloutState );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: SetCalloutState()
// Desc: set the state for a particular callout
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::SetAllCalloutState( const DidcvCalloutState* lpCalloutState )
{
    DidcvCalloutData* lpInternalCalloutData;
    UINT i;

    // sanity check
    if( NULL == lpCalloutState )
        return FALSE;

    for( i = 0; i < m_calloutSetListByObjID.Size(); i++ )
    {
        // get the data associated with the object
        lpInternalCalloutData =( DidcvCalloutData* ) m_calloutSetListByObjID[i]->GetData();

        // not found
        if( NULL == lpInternalCalloutData )
        {
            // internal error
            continue;
        }

        // set the internal state to the new state
        lpInternalCalloutData->lpState->SmartSet( lpCalloutState );
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutState()
// Desc: retrieves the state for a given callout
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::GetCalloutState( DidcvCalloutState* lpCalloutState, 
                                           DWORD dwObjID )
{
    DidcvCalloutData* lpInternalCalloutData;

    if( NULL == lpCalloutState )
        return FALSE;

    // get the data associated with the object
    lpInternalCalloutData = GetCalloutData( dwObjID );

    // not found
    if( NULL == lpInternalCalloutData )
        return FALSE;

    // copy the internal state
    lpCalloutState->Copy( lpInternalCalloutData->lpState );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: SetActionMap()
// Desc: sets the action mapping for a particular callout
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::SetActionMap( const LPDIACTION lpAction, DWORD dwObjID )
{
    DidcvCalloutSet* lpCalloutSet = NULL;
    DidcvCalloutData* lpInternalCalloutData = NULL;
    DidcvActionMap* lpActionMap = NULL;

    // find the set associated with the control
    lpCalloutSet = Find( m_calloutSetListByObjID, dwObjID );

    // not found
    if( NULL == lpCalloutSet )
        return FALSE;

    // get the data associated with the object
    lpInternalCalloutData =( DidcvCalloutData* ) lpCalloutSet->GetData();

    // every set should have a callout data reference, if not - internal error
    assert ( lpInternalCalloutData != NULL );

    if( NULL == lpAction )
    {
        // if lpAction is NULL, then reset the action map
        if( lpInternalCalloutData->lpActionMap )
        {
            delete lpInternalCalloutData->lpActionMap;
            lpInternalCalloutData->lpActionMap = NULL;
        }

        lpActionMap;
    }
    else
    {
        // allocate, if necessary
        if( NULL == lpInternalCalloutData->lpActionMap )
            lpActionMap = lpInternalCalloutData->lpActionMap = new DidcvActionMap;

        // store the information
        memcpy( &( lpInternalCalloutData->lpActionMap->dia ), 
                lpAction, sizeof( DIACTION ) );
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetActionMap()
// Desc: retrieves the action map for a particular callout
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::GetActionMap( DidcvActionMap* lpActionMap, DWORD dwObjID )
{
    DidcvCalloutData* lpInternalCalloutData;

    // get the data associated with the object
    lpInternalCalloutData = GetCalloutData( dwObjID );

    // not found
    if( NULL == lpInternalCalloutData || NULL == lpInternalCalloutData->lpActionMap )
        return FALSE;

    // copy internal data to be returned
    lpActionMap->Copy( lpInternalCalloutData->lpActionMap );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetNumUniqueCallouts()
// Desc: returns the number of unique callouts
//-----------------------------------------------------------------------------
UINT DidcvCalloutManager::GetNumUniqueCallouts() const
{
    return m_calloutSetListByObjID.Size();
}




//-----------------------------------------------------------------------------
// Name: ClearAllActionMaps()
// Desc: resets( and frees ) all action maps associated with all callouts in
//       the manager
//-----------------------------------------------------------------------------
void DidcvCalloutManager::ClearAllActionMaps()
{
    UINT i;
    DidcvCalloutData* lpInternalCalloutData;

    // run through callout set list
    for( i = 0; i < m_calloutSetListByObjID.Size(); i++ )
    {
        // get the data associated with each
        lpInternalCalloutData =( DidcvCalloutData* ) m_calloutSetListByObjID[i]->GetData();
        
        if( lpInternalCalloutData->lpActionMap )
        {
            // free and reset action maps
            delete lpInternalCalloutData->lpActionMap;
            lpInternalCalloutData->lpActionMap = NULL;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: EnumObjects()
// Desc: enumerates the list of objects - the dwMapOnly specifies if only
//       objects with action assign
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::EnumObjects( LPDIRECTINPUTDEVICE8 lpDevice, 
                                       LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, 
                                       LPVOID pvRef, DWORD dwMapOnly )
{
    UINT i;
    DidcvCalloutData* lpData;
    DIDEVICEOBJECTINSTANCE objInst;

    if( lpDevice == NULL || lpCallback == NULL )
        return FALSE;

    // go through list of unique callouts
    for( i = 0; i < m_calloutSetListByObjID.Size(); i++ )
    {
        // get the data associated with the callout set
        lpData =( DidcvCalloutData* ) m_calloutSetListByObjID[i]->GetData();

        // if do all or if action mapped
        if( !dwMapOnly || lpData->lpActionMap )
        {
            ZeroMemory( &objInst, sizeof( DIDEVICEOBJECTINSTANCE ) );
            objInst.dwSize = sizeof( DIDEVICEOBJECTINSTANCE );

            // get the info about the object from the device
            lpDevice->GetObjectInfo( &objInst, 
                        m_calloutSetListByObjID[i]->GetIdentifier(), DIPH_BYID );
            lpCallback( &objInst, pvRef );
        }
    }
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: CalcCanBeCollapsed()
// Desc: figures out if there are any unmapped callouts
//-----------------------------------------------------------------------------
BOOL DidcvCalloutManager::CalcCanBeCollapsed()
{
    UINT i;
    DidcvCalloutData* lpData;

    for( i = 0; i < m_calloutSetListByObjID.Size(); i ++ )
    {
        lpData =( DidcvCalloutData* ) m_calloutSetListByObjID[i]->GetData();

        // if we find any non-mapped callouts
        if( lpData->lpActionMap == NULL )
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: TrimArrays()
// Desc: trims the internal array( and frees unnecessary memory )
//-----------------------------------------------------------------------------
void DidcvCalloutManager::TrimArrays()
{
    // this shouldn't have to do anything if the sizes were preset
    m_calloutList.Trim();
    m_calloutSetListByObjID.Trim();
    m_calloutSetListByView.Trim();

    UINT i;

    for( i = 0; i < m_calloutSetListByObjID.Size(); i ++ )
        m_calloutSetListByObjID[i]->TrimArrays();

    for( i = 0; i < m_calloutSetListByView.Size(); i ++ )
        m_calloutSetListByView[i]->TrimArrays();
}




//-----------------------------------------------------------------------------
// Name: GetCalloutDataRef()
// Desc: returns the( const ) data associated with a particular callout
//-----------------------------------------------------------------------------
const DidcvCalloutData* DidcvCalloutManager::GetCalloutDataRef( DWORD dwObjID ) const
{
    return GetCalloutData( dwObjID );
}




//-----------------------------------------------------------------------------
// Name: GetCalloutSetByView()
// Desc: return the callout set for a given view, if any
//-----------------------------------------------------------------------------
const DidcvCalloutSet* DidcvCalloutManager::GetCalloutSetByView( INT nView ) const
{
    return Find( m_calloutSetListByView,( DWORD ) nView );
}




//-----------------------------------------------------------------------------
// Name: GetCalloutSetByObjID()
// Desc: returns the callout set for a given callout, if any
//-----------------------------------------------------------------------------
const DidcvCalloutSet* DidcvCalloutManager::GetCalloutSetByObjID( DWORD dwObjID ) const
{
    return Find( m_calloutSetListByView, dwObjID );
}




//-----------------------------------------------------------------------------
// Name: Find()
// Desc: helper function to find a callout set from an array, using 
//       an identifier
//-----------------------------------------------------------------------------
DidcvCalloutSet* DidcvCalloutManager::Find( const GwArray <DidcvCalloutSet *> & arr, 
                                            DWORD dwIdentifier ) const
{
    // TODO: this function can be optimized if needed
    UINT i, size;

    size = arr.Size();

    for( i = 0; i < size; i++ )
    {
        if( arr[i]->GetIdentifier() == dwIdentifier )
            return arr[i];
    }

    return NULL;
}




//-----------------------------------------------------------------------------
// Name: GetCalloutData()
// Desc: helper function that returns the callout data for a given callout
//-----------------------------------------------------------------------------
DidcvCalloutData* DidcvCalloutManager::GetCalloutData( DWORD dwObjID ) const
{
    DidcvCalloutData* lpData;
    DidcvCalloutSet* lpCalloutSet;

    // find the set of callout with the id
    lpCalloutSet = Find( m_calloutSetListByObjID, dwObjID );

    // if not found
    if( NULL == lpCalloutSet )
        return NULL;

    // return the data associated with the object
    lpData =( DidcvCalloutData* ) lpCalloutSet->GetData();

    if( NULL == lpData || NULL == lpData->lpState )
    {
        // internal error, must handle differently
        assert( 0 );
        return NULL;
    }

    return lpData;
}




//-----------------------------------------------------------------------------
// Name: DidcvCalloutSet()
// Desc: Constructor
//-----------------------------------------------------------------------------
DidcvCalloutSet::DidcvCalloutSet()
    : m_calloutList( DIDCV_DEFAULT_CALLOUT_SET_ARRAY_SIZE ),
      m_dwSetID( 0 ),
      m_lpData( NULL )
{ }




//-----------------------------------------------------------------------------
// Name: ~DidcvCalloutSet()
// Desc: destructor
//-----------------------------------------------------------------------------
DidcvCalloutSet::~DidcvCalloutSet()
{
    CleanUp();
}




//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: resets the object, and frees any resources
//-----------------------------------------------------------------------------
void DidcvCalloutSet::CleanUp()
{
    // set the array back to default size
    m_calloutList.SetSize( 0 );

    // reset the set id
    m_dwSetID = 0;

    // reset the data pointer
    m_lpData = NULL;
}




//-----------------------------------------------------------------------------
// Name: TrimArrays()
// Desc: resize arrays to free unused memory
//-----------------------------------------------------------------------------
void DidcvCalloutSet::TrimArrays()
{
    m_calloutList.Trim();
}



//-----------------------------------------------------------------------------
// Name: AddCallout()
// Desc: adds a callout reference to the set
//-----------------------------------------------------------------------------
BOOL DidcvCalloutSet::AddCallout( DidcvCallout* lpCallout )
{
    // if NULL, return FALSE
    if( NULL == lpCallout )
        return FALSE;

    // append to list
    m_calloutList.PushBack( lpCallout );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: Apply()
// Desc: using a instantiation of DidcvCalloutApplicant, applies it to every
//       every callout in the group
//-----------------------------------------------------------------------------
void DidcvCalloutSet::Apply( DidcvCalloutApplicant* lpCalloutApp )
{
    UINT i, size;

    size = m_calloutList.Size();

    // loop through entries, and apply each
    for( i = 0; i < size; i++ )
    {
        // if FALSE is returned, then stop
        if( FALSE == lpCalloutApp->Apply( m_calloutList[i] ) )
            break;
    }
}




//-----------------------------------------------------------------------------
// Name: GetInternalArray()
// Desc: returns a const reference to the array that stores the callout
//       references
//-----------------------------------------------------------------------------
const GwArray <DidcvCallout *> & DidcvCalloutSet::GetInternalArrayRef() const
{
    return m_calloutList;
}




//-----------------------------------------------------------------------------
// Name: SetIdentifier()
// Desc: assigned a unique identifier to the set
//-----------------------------------------------------------------------------
void DidcvCalloutSet::SetIdentifier( DWORD dwID )
{
    m_dwSetID = dwID;
}




//-----------------------------------------------------------------------------
// Name: GetIdentifier()
// Desc: returns the identifier associated with this set
//-----------------------------------------------------------------------------
DWORD DidcvCalloutSet::GetIdentifier() const
{
    return m_dwSetID;
}




//-----------------------------------------------------------------------------
// Name: SetData()
// Desc: attach a void pointer with this set
//-----------------------------------------------------------------------------
void DidcvCalloutSet::SetData( void* lpData )
{
    m_lpData = lpData;
}




//-----------------------------------------------------------------------------
// Name: GetData()
// Desc: returns the data associated with this set
//-----------------------------------------------------------------------------
void* DidcvCalloutSet::GetData() const
{
    return m_lpData;
}




//-----------------------------------------------------------------------------
// Name: PutLinePoint()
// Desc: draws a line to a point
//-----------------------------------------------------------------------------
void PutLinePoint( HDC hDC, POINT p )
{
    MoveToEx( hDC, p.x, p.y, NULL );
    LineTo( hDC, p.x + 1, p.y );
}




//-----------------------------------------------------------------------------
// Name: SmartSet()
// Desc: copies valid members of other
//-----------------------------------------------------------------------------
void DidcvCalloutState::SmartSet( const DidcvCalloutState* other )
{
    if( other->dwFlags & DIDCV_DRAWCALLOUT )
        bDrawCallout = other->bDrawCallout;

    if( other->dwFlags & DIDCV_DRAWOVERLAY )
        bDrawOverlay = other->bDrawOverlay;

    if( other->dwFlags & DIDCV_DRAWHIGHLIGHT )
        bDrawHighlight = other->bDrawHighlight;

    if( other->dwFlags & DIDCV_DRAWEMPTYCALLOUT )
        bDrawEmptyCallout = other->bDrawEmptyCallout;

    if( other->dwFlags & DIDCV_DRAWFULLNAME )
        bDrawFullname = other->bDrawFullname;
}




//-----------------------------------------------------------------------------
// Name: MakeFlag()
// Desc: makes a DWORD flag
//-----------------------------------------------------------------------------
DWORD DidcvCalloutState::MakeFlag() const
{
    DWORD dwExtFlags = 0;

    if( ( dwFlags & DIDCV_DRAWCALLOUT ) && bDrawCallout )
        dwExtFlags |= DIDCV_DRAWCALLOUT;

    if( ( dwFlags & DIDCV_DRAWOVERLAY ) && bDrawOverlay )
        dwExtFlags |= DIDCV_DRAWOVERLAY;

    if( ( dwFlags & DIDCV_DRAWHIGHLIGHT ) && bDrawHighlight )
        dwExtFlags |= DIDCV_DRAWHIGHLIGHT;

    if( ( dwFlags & DIDCV_DRAWEMPTYCALLOUT ) && bDrawEmptyCallout )
        dwExtFlags |= DIDCV_DRAWEMPTYCALLOUT;

    if( ( dwFlags & DIDCV_DRAWFULLNAME ) && bDrawFullname )
        dwExtFlags |= DIDCV_DRAWFULLNAME;

    return dwExtFlags;
}




//-----------------------------------------------------------------------------
// Name: SetFlag()
// Desc: given a flag, sets the members
//-----------------------------------------------------------------------------
void DidcvCalloutState::SetFlag( DWORD dwExtFlags )
{
    dwFlags = DIDCV_ALL_CALLOUT_STATE_SET;

    bDrawCallout = ( dwExtFlags & DIDCV_DRAWCALLOUT ) != 0;
    bDrawOverlay = ( dwExtFlags & DIDCV_DRAWOVERLAY ) != 0;
    bDrawHighlight = ( dwExtFlags & DIDCV_DRAWHIGHLIGHT ) != 0;
    bDrawEmptyCallout = ( dwExtFlags & DIDCV_DRAWEMPTYCALLOUT ) != 0;
    bDrawFullname = ( dwExtFlags & DIDCV_DRAWFULLNAME ) != 0;
}




//-----------------------------------------------------------------------------
// Name: DidcvPolyLineArrow()
// Desc: draws a line using an array of line segments, with optional shadows
//-----------------------------------------------------------------------------
void DidcvPolyLineArrow( HDC hDC, const POINT *rgpt, int nPoints, BOOL bDoShadow )
{
    int i;

    if( rgpt == NULL || nPoints < 1 )
        return;

    if( nPoints > 1 )
        for( i = 0; i < nPoints - 1; i++ )
        {
            SPOINT a = rgpt[i], b = rgpt[i + 1];

            if( bDoShadow )
            {
                int rise = abs( b.y - a.y ), run = abs( b.x - a.x );
                bool vert = rise > run;
                int ord = vert ? 1 : 0;
                int nord = vert ? 0 : 1;
                
                for( int o = -1; o <= 1; o += 2 )
                {
                    SPOINT c( a ), d( b );
                    c.a[nord] += o;
                    d.a[nord] += o;
                    MoveToEx( hDC, c.x, c.y, NULL );
                    LineTo( hDC, d.x, d.y );
                }

                bool reverse = a.a[ord] > b.a[ord];
                SPOINT e( reverse ? b : a ), f( reverse ? a : b );
                e.a[ord] -= 1;
                f.a[ord] += 1;
                PutLinePoint( hDC, e );
                PutLinePoint( hDC, f );
            }
            else
            {
                MoveToEx( hDC, a.x, a.y, NULL );
                LineTo( hDC, b.x, b.y );
            }
        }

    POINT z = rgpt[nPoints - 1];

    if( bDoShadow )
    {
        POINT pt[5] = {
            { z.x, z.y + 2 },
            { z.x + 2, z.y },
            { z.x, z.y - 2 },
            { z.x - 2, z.y }, };
        pt[4] = pt[0];
        Polyline( hDC, pt, 5 );
    }
    else
    {
        MoveToEx( hDC, z.x - 1, z.y, NULL );
        LineTo( hDC, z.x + 2, z.y );
        MoveToEx( hDC, z.x, z.y - 1, NULL );
        LineTo( hDC, z.x, z.y + 2 );
    }
}




//-----------------------------------------------------------------------------
// Name: DidcvCreateFont()
// Desc: Creates a GDI font object( code adapted from Charles Petzold )
//-----------------------------------------------------------------------------
HFONT DidcvCreateFont( HDC hdc, const TCHAR* szFaceName, int iDeciPtHeight, 
                       int iDeciPtWidth, int iAttributes, BOOL fLogRes)
{
     FLOAT      cxDpi, cyDpi ;
     HFONT      hFont ;
     LOGFONT    lf ;
     POINT      pt ;
     TEXTMETRIC tm ;

     SaveDC( hdc ) ;

     SetGraphicsMode( hdc, GM_ADVANCED ) ;
     ModifyWorldTransform( hdc, NULL, MWT_IDENTITY ) ;
     SetViewportOrgEx( hdc, 0, 0, NULL ) ;
     SetWindowOrgEx  ( hdc, 0, 0, NULL ) ;

     if( fLogRes)
     {
          cxDpi =( FLOAT) GetDeviceCaps( hdc, LOGPIXELSX ) ;
          cyDpi =( FLOAT) GetDeviceCaps( hdc, LOGPIXELSY ) ;
     }
     else
     {
          cxDpi =( FLOAT)( 25.4 * GetDeviceCaps( hdc, HORZRES ) /
                                  GetDeviceCaps( hdc, HORZSIZE ) ) ;

          cyDpi =( FLOAT)( 25.4 * GetDeviceCaps( hdc, VERTRES ) /
                                  GetDeviceCaps( hdc, VERTSIZE ) ) ;
     }

     pt.x =( int)( iDeciPtWidth  * cxDpi / 72 ) ;
     pt.y =( int)( iDeciPtHeight * cyDpi / 72 ) ;

     DPtoLP( hdc, &pt, 1) ;

     lf.lfHeight         = -( int)( fabs( pt.y) / 10.0 + 0.5 ) ;
     lf.lfWidth          = 0 ;
     lf.lfEscapement     = 0 ;
     lf.lfOrientation    = 0 ;
     lf.lfWeight         = iAttributes & DIDCV_FONT_BOLD      ? 700 : 0 ;
     lf.lfItalic         = iAttributes & DIDCV_FONT_ITALIC    ?   1 : 0 ;
     lf.lfUnderline      = iAttributes & DIDCV_FONT_UNDERLINE ?   1 : 0 ;
     lf.lfStrikeOut      = iAttributes & DIDCV_FONT_STRIKEOUT ?   1 : 0 ;
     lf.lfCharSet        = 0 ;
     lf.lfOutPrecision   = 0 ;
     lf.lfClipPrecision  = 0 ;
     lf.lfQuality        = 0 ;
     lf.lfPitchAndFamily = 0 ;

     _tcscpy( lf.lfFaceName, szFaceName ) ;

     hFont = CreateFontIndirect( &lf ) ;

     if( iDeciPtWidth != 0 )
     {
          hFont =( HFONT ) SelectObject( hdc, hFont );
          GetTextMetrics( hdc, &tm );
          DeleteObject( SelectObject( hdc, hFont ) ) ;
          lf.lfWidth =( int)( tm.tmAveCharWidth * fabs( pt.x ) / 
                              fabs( pt.y ) + 0.5 ) ;

          hFont = CreateFontIndirect( &lf ) ;
     }

     RestoreDC( hdc, -1) ;

     return hFont ;
}




//-----------------------------------------------------------------------------
// Name: DidcvView()
// Desc: Constructor
//-----------------------------------------------------------------------------
DidcvView::DidcvView()
    : m_nOffset( DIDCV_INVALID_ID ),
      m_nID( DIDCV_INVALID_ID )
{ }




//-----------------------------------------------------------------------------
// Name: SetOffset()
// Desc: sets the offset from the original index returned by GetImage
//-----------------------------------------------------------------------------
void DidcvView::SetOffset( INT nOffset )
{
    m_nOffset = nOffset;
}




//-----------------------------------------------------------------------------
// Name: GetOffset()
// Desc: returns the original GetImage offset
//-----------------------------------------------------------------------------
INT DidcvView::GetOffset() const
{
    return m_nOffset;
}




//-----------------------------------------------------------------------------
// Name: SetID()
// Desc: sets the internally assigned unique identifier
//-----------------------------------------------------------------------------
void DidcvView::SetID( INT nID )
{
    m_nID = nID;
}




//-----------------------------------------------------------------------------
// Name: GetID()
// Desc: returns the internally assigned unique identifier
//-----------------------------------------------------------------------------
INT DidcvView::GetID() const
{
    return m_nID;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: static function to the instantiate a DidcvView object from filename
//-----------------------------------------------------------------------------
DidcvBitmapView* DidcvBitmapView::Create( LPCTSTR tszFilename, DWORD dwFlags )
{
    DidcvBitmapView* lpView;
    DidcvBitmap* lpBm;

    lpView = new DidcvBitmapView();
    if( NULL == lpView )
        return NULL;

    lpBm = DidcvBitmap::Create( tszFilename );
    if( NULL == lpBm )
    {
        delete lpView;
        return NULL;
    }

    lpView->m_lpBitmap = lpBm ;

    return lpView;
}




//-----------------------------------------------------------------------------
// Name: DidcvBitmapView()
// Desc: constructor
//-----------------------------------------------------------------------------
DidcvBitmapView::DidcvBitmapView()
    : DidcvView(),
      m_lpBitmap( NULL )
{ }




//-----------------------------------------------------------------------------
// Name: ~DidcvBitmapView()
// Desc: Destructor
//-----------------------------------------------------------------------------
DidcvBitmapView::~DidcvBitmapView()
{
    SAFE_DELETE( m_lpBitmap );
}




//-----------------------------------------------------------------------------
// Name: GetViewSize()
// Desc: returns the size of the view image
//-----------------------------------------------------------------------------
BOOL DidcvBitmapView::GetViewSize( SIZE* lpSize ) const
{
    if( NULL == lpSize )
        return FALSE;

    if( NULL == m_lpBitmap )
    {
        lpSize->cx = 0;
        lpSize->cy = 0;

        return FALSE;
    }

    return m_lpBitmap->GetSize( lpSize );
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: renders the view to a HDC
//-----------------------------------------------------------------------------
void DidcvBitmapView::Render( HDC hdc, VOID* lpBits, INT width, INT height, 
                              const POINT* ptOrigin )
{
    BOOL res = FALSE;

    // nothing to draw
    if( NULL == m_lpBitmap )
        return;

    // alpha blend by hand
    if( lpBits )
        res = m_lpBitmap->Blend( lpBits, ptOrigin->x, ptOrigin->y, width, height );

    if( !res )
    {
        // try alpha blending with hDC
        res = m_lpBitmap->Blend( hdc, 0, 0 );

        if( !res )
            // no alpha blending available - just draw
            m_lpBitmap->Draw( hdc, 0 , 0 );
    }
}




//-----------------------------------------------------------------------------
// Name: DidcvCustomView()
// Desc: constructor
//-----------------------------------------------------------------------------
DidcvCustomView::DidcvCustomView( const DidcvCustomViewInfo* lpInfo, 
                                  DWORD dwStartIndex, DWORD dwEndIndex )
    : m_lpCustomViewInfoRef( lpInfo ),
      m_dwStartIndex( dwStartIndex ),
      m_dwEndIndex( dwEndIndex )
{ }




//-----------------------------------------------------------------------------
// Name: GetViewSize()
// Desc: retrieves the size of the view
//-----------------------------------------------------------------------------
BOOL DidcvCustomView::GetViewSize( SIZE* lpSize ) const
{
    if( NULL == lpSize )
        return FALSE;

    lpSize->cx = DIDCV_CUSTOM_VIEW_WIDTH;
    lpSize->cy = DIDCV_CUSTOM_VIEW_HEIGHT;

    return TRUE;
}



#define DIDCV_CUSTOM_BORDER_WIDTH       8
#define DIDCV_CUSTOM_BOX_WIDTH          90
#define DIDCV_CUSTOM_BOX_CONTROL_WIDTH  70
#define DIDCV_CUSTOM_BOX_HEIGHT         24
#define DIDCV_CUSTOM_LINE_BORDER        8
#define DIDCV_CUSTOM_LINE_BOX_WIDTH     32
#define DIDCV_CUSTOM_VIEWS_PER_COL     ( ( DIDCV_CUSTOM_VIEW_HEIGHT -( 2 * DIDCV_CUSTOM_BORDER_WIDTH ) ) / DIDCV_CUSTOM_BOX_HEIGHT )
//-----------------------------------------------------------------------------
// Name: Render()
// Desc: draws the custom view
//-----------------------------------------------------------------------------
VOID DidcvCustomView::Render( HDC hdc, VOID* lpBits, INT width, INT height, 
                              const POINT* pptOrigin )
{
    TCHAR tszBuffer[MAX_PATH];
    DWORD dwTextFlags = DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS 
                      | DT_EDITCONTROL | DT_RIGHT | DT_VCENTER | DT_SINGLELINE;
    RECT rect;
    DWORD i;

    if( NULL == m_lpCustomViewInfoRef )
        return;

    for( i = m_dwStartIndex; i <= m_dwEndIndex; i++ )
    {
        // calculate coordinates based on the index
        CalcCoordinates( i, &rect );
        // copy the callout name into the buffer
        _tcscpy( tszBuffer, m_lpCustomViewInfoRef->rgObjData[i]->tszName );
        // draw the text
        DrawText( hdc, tszBuffer, -1,( LPRECT ) &rect, dwTextFlags );
    }
}




//-----------------------------------------------------------------------------
// Name: CalcCoordinates()
// Desc: calculate coordinates for control string
//-----------------------------------------------------------------------------
void DidcvCustomView::CalcCoordinates( DWORD dwIndex, LPRECT lpRect )
{
    POINT ptStart;

    // mod the index
    dwIndex = dwIndex % ( DIDCV_CUSTOM_VIEWS_PER_COL* 2 );

    // start at the top left
    ptStart.x = DIDCV_CUSTOM_BORDER_WIDTH;
    if( dwIndex >= DIDCV_CUSTOM_VIEWS_PER_COL )
    {
        // shift into the right column
        ptStart.x += DIDCV_CUSTOM_VIEW_WIDTH / 2;
        dwIndex -= DIDCV_CUSTOM_VIEWS_PER_COL;
    }
    ptStart.y = DIDCV_CUSTOM_BORDER_WIDTH + ( dwIndex* DIDCV_CUSTOM_BOX_HEIGHT );

    // set the coordinate
    lpRect->left = ptStart.x;
    lpRect->top = ptStart.y;
    lpRect->right = lpRect->left + DIDCV_CUSTOM_BOX_CONTROL_WIDTH;
    lpRect->bottom = lpRect->top + DIDCV_CUSTOM_BOX_HEIGHT;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: static function to instantiate a view
//-----------------------------------------------------------------------------
DidcvCustomView* DidcvCustomView::Create( const DidcvCustomViewInfo* lpInfo, 
                                          DWORD dwStartIndex, DWORD* lpFinishIndex )
{
    DWORD dwIndex;

    if( dwStartIndex >= lpInfo->dwSize )
        return NULL;

    // calculate how many can fit
    dwIndex = DIDCV_CUSTOM_VIEWS_PER_COL;
    dwIndex *= 2;
    dwIndex += dwStartIndex;
    dwIndex --;
    
    if( dwIndex >= lpInfo->dwSize )
        dwIndex = lpInfo->dwSize - 1;

    if( lpFinishIndex )
        *lpFinishIndex = dwIndex;

    return new DidcvCustomView( lpInfo, dwStartIndex, dwIndex );
}




//-----------------------------------------------------------------------------
// Name: CalcNumViews()
// Desc: calculates the number of views from a total count of controls
//-----------------------------------------------------------------------------
DWORD DidcvCustomView::CalcNumViews( DWORD dwCount )
{
    DWORD dwIndex;
    DWORD result;

    dwIndex = DIDCV_CUSTOM_VIEWS_PER_COL;
    dwIndex *= 2;

    result = dwCount / dwIndex +( dwCount % dwIndex ? 1 : 0 );

    return result;
}




//-----------------------------------------------------------------------------
// Name: CalcImageInfo()
// Desc: calculates the image info for a custom control
//-----------------------------------------------------------------------------
BOOL DidcvCustomView::CalcImageInfo( DWORD index, LPRECT rcOverlay, 
                                     LPDWORD dwNumPoints, LPPOINT rgptCalloutLine, 
                                     LPRECT rcCalloutRect )
{
    POINT ptStart;

    // mod the index
    index = index %( DIDCV_CUSTOM_VIEWS_PER_COL* 2 );

    ptStart.x = DIDCV_CUSTOM_BORDER_WIDTH + DIDCV_CUSTOM_BOX_CONTROL_WIDTH;
    if( index >= DIDCV_CUSTOM_VIEWS_PER_COL )
    {
        ptStart.x += DIDCV_CUSTOM_VIEW_WIDTH / 2;
        index -= DIDCV_CUSTOM_VIEWS_PER_COL;
    }
    ptStart.y = DIDCV_CUSTOM_BORDER_WIDTH +( index * DIDCV_CUSTOM_BOX_HEIGHT );

    // do nothing with the overlay rect at this point
    ZeroMemory( rcOverlay, sizeof( RECT ) );
    
    *dwNumPoints = 2;
    rgptCalloutLine[1].x = ptStart.x + DIDCV_CUSTOM_LINE_BORDER;
    rgptCalloutLine[1].y = ptStart.y +( DIDCV_CUSTOM_BOX_HEIGHT / 2 );
    rgptCalloutLine[0].x = rgptCalloutLine[1].x + DIDCV_CUSTOM_LINE_BOX_WIDTH - 
                           2 * DIDCV_CUSTOM_LINE_BORDER;
    rgptCalloutLine[0].y = rgptCalloutLine[1].y;
    
    rcCalloutRect->left = ptStart.x + DIDCV_CUSTOM_LINE_BOX_WIDTH;
    rcCalloutRect->top = ptStart.y;
    rcCalloutRect->right = rcCalloutRect->left + DIDCV_CUSTOM_BOX_WIDTH;
    rcCalloutRect->bottom = rcCalloutRect->top + DIDCV_CUSTOM_BOX_HEIGHT;

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: DidcvViewManager()
// Desc: constructor
//-----------------------------------------------------------------------------
DidcvViewManager::DidcvViewManager()
    : m_ppViewTable( NULL ),
      m_capacity( 0 ),
      m_size( 0 )
{
    // initialized
}




//-----------------------------------------------------------------------------
// Name: ~DidcvViewManager()
// Desc: destructor
//-----------------------------------------------------------------------------
DidcvViewManager::~DidcvViewManager()
{
    CleanUp();
}




//-----------------------------------------------------------------------------
// Name: SetSize()
// Desc: sets the capacity of the view manager
//-----------------------------------------------------------------------------
BOOL DidcvViewManager::SetCapacity( UINT uCapacity, BOOL bDeleteContent )
{
    if( m_ppViewTable )
    {
        if( bDeleteContent )
            CleanUp();
        else
            free( m_ppViewTable );
    }

    m_capacity = uCapacity;
    m_size = 0;
    m_ppViewTable =( DidcvView ** ) malloc( uCapacity * sizeof( DidcvView* ) );

    if( NULL == m_ppViewTable )
    {
        m_capacity = 0;
        return FALSE;
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: frees the table and the contents of the table
//-----------------------------------------------------------------------------
void DidcvViewManager::CleanUp()
{
    UINT i;

    if( m_ppViewTable )
    {
        // free the entries
        for( i = 0; i < m_size ; i++ )
            if( m_ppViewTable [i] )
                delete m_ppViewTable [i];

        // free the table
        free( m_ppViewTable );
        m_ppViewTable = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: AddImage()
// Desc: adds a DidcvView to the to be stored and managed
//-----------------------------------------------------------------------------
BOOL DidcvViewManager::AddImage( DidcvView* lpView, INT nOffset )
{
    // save the offset value in the View object
    lpView->SetOffset( nOffset );

    // try to allocate the first free entry
    if( m_size >= m_capacity )
    {
        lpView->SetID( DIDCV_INVALID_ID );
        return FALSE;
    }

    m_ppViewTable [m_size] = lpView;

    // save the table entry as the unique view id
    lpView->SetID( ( INT )m_size );

    m_size ++;

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: GetImage()
// Desc: returns the DidcvView for a given internal identifier
//-----------------------------------------------------------------------------
DidcvView* DidcvViewManager::GetImage( INT nID )
{
    // make sure is within bounds
    if( nID < 0 ||( UINT ) nID >= m_capacity )
        return NULL;

    // return the view object in the table
    return m_ppViewTable[ nID ];
}




//-----------------------------------------------------------------------------
// Name: GetImageByOffset()
// Desc: returns the DidcvView for a given offset from the original GetImage
//       image info array
//-----------------------------------------------------------------------------
DidcvView* DidcvViewManager::GetImageByOffset( INT nOffset )
{
    DWORD i;
    if( nOffset < 0 )
        return NULL;

    // look for the offset in the entry
    for( i = 0; i < m_size; i++ )
        if( m_ppViewTable[i] &&( m_ppViewTable[i]->GetOffset() == nOffset ) )
            return m_ppViewTable[i];
 
    return NULL;
}




//-----------------------------------------------------------------------------
// Name: GetNumViews()
// Desc: returns the number of images currently stored in the view manager
//-----------------------------------------------------------------------------
UINT DidcvViewManager::GetNumViews() const
{
    return m_size;
}




//-----------------------------------------------------------------------------
// Name: DidcvDirect3DSurface8Clone
// Desc: a clone surface
//-----------------------------------------------------------------------------
class DidcvDirect3DSurface8Clone : public IUnknown
{
private:
    int m_iRefCount;
    BYTE *m_pData;
    D3DSURFACE_DESC m_Desc;

public:
    DidcvDirect3DSurface8Clone() : m_pData( NULL ), m_iRefCount( 1 ) { }
    ~DidcvDirect3DSurface8Clone() { delete[] m_pData; }

public:     
    // IUnknown methods
    STDMETHOD( QueryInterface )( REFIID  riid, VOID  **ppvObj ) { return E_NOINTERFACE; }
    STDMETHOD_( ULONG,AddRef )() { return ++m_iRefCount; }
    STDMETHOD_( ULONG,Release )()
    {
        if( !--m_iRefCount )
        {
            delete this;
            return 0;
        }
        return m_iRefCount;
    }

    // IBuffer methods
    STDMETHOD( SetPrivateData )( REFGUID riid, CONST VOID *pvData, DWORD cbData, DWORD   dwFlags ) { return S_OK; }
    STDMETHOD( GetPrivateData )( REFGUID riid, VOID* pvData, DWORD  *pcbData ) { return S_OK; }
    STDMETHOD( FreePrivateData )( REFGUID riid ) { return S_OK; }
    STDMETHOD( GetContainer )( REFIID riid, void **ppContainer ) { return S_OK; }
    STDMETHOD( GetDevice )( IDirect3DDevice8 **ppDevice ) { return S_OK; }

    // IDirect3DSurface8 methods
    STDMETHOD_( D3DSURFACE_DESC, GetDesc )() { return m_Desc; }
    STDMETHOD( LockRect )( D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD dwFlags )
    {
        // Assume the entire surface is being locked.
        pLockedRect->Pitch = m_Desc.Width * 4;
        pLockedRect->pBits = m_pData;
        return S_OK;
    }
    STDMETHOD( UnlockRect )() { return S_OK; }
    BOOL Create( int iWidth, int iHeight )
    {
        m_pData = new BYTE[iWidth * iHeight * 4];
        if( !m_pData ) return FALSE;

        m_Desc.Format = D3DFMT_A8R8G8B8;
        m_Desc.Type = D3DRTYPE_SURFACE;
        m_Desc.Usage = 0;
        m_Desc.Pool = D3DPOOL_SYSTEMMEM;
        m_Desc.Size = iWidth * iHeight * 4;
        m_Desc.MultiSampleType = D3DMULTISAMPLE_NONE;
        m_Desc.Width = iWidth;
        m_Desc.Height = iHeight;
        return TRUE;
    }
};




//-----------------------------------------------------------------------------
// Name: GetCloneSurface()
// Desc: 
//-----------------------------------------------------------------------------
IDirect3DSurface8* GetCloneSurface( int iWidth, int iHeight )
{
    DidcvDirect3DSurface8Clone *pSurf = new DidcvDirect3DSurface8Clone;

    if( !pSurf ) return NULL;
    if( !pSurf->Create( iWidth, iHeight ) )
    {
        delete pSurf;
        return NULL;
    }

    return( IDirect3DSurface8* )pSurf;
}




//-----------------------------------------------------------------------------
// Name: CreateViaD3Dx()
// Desc: loads a bitmap from .BMP or .PNG using D3Dx
//-----------------------------------------------------------------------------
DidcvBitmap* DidcvBitmap::CreateViaD3dx( LPCTSTR tszFileName )
{
    LPDIRECT3DSURFACE8 lpSurf = NULL;
    HRESULT hr;
    D3DXIMAGE_INFO d3dxImgInfo;
    DidcvBitmap* pbm = NULL;
    BYTE* pbData = NULL;
    UINT y, i;
    UINT width, height, pitch;

    lpSurf = GetCloneSurface( DIDCV_MAX_IMAGE_WIDTH, DIDCV_MAX_IMAGE_HEIGHT );
    if( lpSurf )
    {
        if( FAILED( D3DXLoadSurfaceFromFile( lpSurf, NULL, NULL, tszFileName, 
                                NULL, D3DX_FILTER_NONE, 0, &d3dxImgInfo ) ) )
            goto label_release;
    }

    width = d3dxImgInfo.Width;
    height = d3dxImgInfo.Height;
    pitch = width * 4;

    // Create a bitmap and copy the texture content onto it.
    LPBYTE pDIBBits;
    HBITMAP hDIB;
    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;
    hDIB = CreateDIBSection( NULL, &bmi, DIB_RGB_COLORS,( LPVOID* )&pDIBBits, NULL, 0 );
    if( !hDIB )
         goto label_release;

    // Lock the surface
    D3DLOCKED_RECT D3DRect;
    hr = lpSurf->LockRect( &D3DRect, NULL, 0 );
    if( FAILED( hr ) )
         goto label_release;
    
    // Copy the bits
    // Note that the image is reversed in Y direction, so we need to re-reverse it.
    for( y = 0; y < height; ++y )
        CopyMemory( pDIBBits +( y * pitch ),( LPBYTE ) D3DRect.pBits + 
                    ( ( height - y ) * D3DRect.Pitch ), width * 4 );  // 512 pixels per line, 32 bits per pixel

    // process the pixel data based on alpha for blending
    pbData =( LPBYTE ) pDIBBits;
    for( i = 0; i < width * height; ++i )
    {
        BYTE bAlpha = pbData[i * 4 + 3];
        pbData[i * 4] = pbData[i * 4] * bAlpha / 255;
        pbData[i * 4 + 1] = pbData[i * 4 + 1] * bAlpha / 255;
        pbData[i * 4 + 2] = pbData[i * 4 + 2] * bAlpha / 255;
    }

    // Unlock
    lpSurf->UnlockRect();

    pbm = new DidcvBitmap;
    if( NULL == pbm )
        goto label_release;

    pbm->m_hbitmap = hDIB;
    pbm->m_lpBits = pDIBBits;
    pbm->FigureSize();


label_release:

    if( lpSurf ) lpSurf->Release();
    if( !pbm && hDIB )
        DeleteObject( hDIB );

    return pbm;
}
