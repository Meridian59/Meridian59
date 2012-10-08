//-----------------------------------------------------------------------------
// File: didcfgview.h
//
// Desc: Header file for DIDCfgView, a class that encapsulates a view of a
//       DirectInput device.  The DIDCfgView class exists to make it easier 
//       to make custom interfaces to view or configure action mappings for 
//       input devices(instead of using IDirectInput8::ConfigureDevices).
//
//       To use the DIDCfgView class, you initialize it for a particular
//       DirectInput device.  You then set up state information for how the
//       image should be drawn: colors, fonts, and details for callouts(the
//       lines drawn from each axis/button to a label).  Finally, you can 
//       call RenderView, passing in a bitmap or HDC for DIDCfgView to draw
//       the image to.
//
//       DIDCfgView is the only class in this file that you need to understand 
//       or interface to.  The other classes shown here are only used to 
//       implement the DIDCfgView class.
//
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __DIDCV_H__
#define __DIDCV_H__




#include <windows.h>
#include <dinput.h>




// invalid id, used for unique, signed identifiers
#define DIDCV_INVALID_ID                            -1

// flags for callout states
#define DIDCV_DRAWCALLOUT                           0x1
#define DIDCV_DRAWOVERLAY                           0x2
#define DIDCV_DRAWHIGHLIGHT                         0x4
#define DIDCV_DRAWEMPTYCALLOUT                      0x8
#define DIDCV_DRAWFULLNAME                          0x10

// all flags set
#define DIDCV_ALL_CALLOUT_STATE_SET                 0xFFFFFFFF

// default dimensions
#define DIDCV_MAX_IMAGE_WIDTH                       512
#define DIDCV_MAX_IMAGE_HEIGHT                      512
#define DIDCV_CUSTOM_VIEW_WIDTH                     400
#define DIDCV_CUSTOM_VIEW_HEIGHT                    300

// DidcvCreateFont attribute flags
#define DIDCV_FONT_BOLD                             0x1
#define DIDCV_FONT_ITALIC                           0x2
#define DIDCV_FONT_UNDERLINE                        0x4
#define DIDCV_FONT_STRIKEOUT                        0x8

// default font point width
#define DIDCV_DEFAULT_FONT_SIZE                     90

// default size of callout array in DidcvCalloutManager
#define DIDCV_DEFAULT_CALLOUT_MANAGER_ARRAY_SIZE    0

// default size of callout array in DidcvCalloutSet
#define DIDCV_DEFAULT_CALLOUT_SET_ARRAY_SIZE        8




// forward declarations
class DidcvViewManager;
class DidcvCalloutManager;
class DidcvBitmap;
class DidcvCallout;
class DidcvCalloutSet;
struct DIDCfgViewInfo;
struct DidcvCalloutData;
struct DidcvCalloutState;
struct DidcvActionMap;
struct DidcvCustomViewInfo;




//-----------------------------------------------------------------------------
// Name: class DIDCfgView
// Desc: main interface for retrieving and rendering device configuration views
//
//       Init() - initializes the object for a particular DirectInputDevice
//       SetActionFormat() - sets the action mapping for the device
//       SetCalloutState() - specifies state for a given callout
//       SetAllCalloutState() - sets the state for all callouts
//       GetObjectIDByLocation() - gets object id at location of current view
//       SetCurrentView() - sets the current view to be rendered
//       RebuildViews() - collapses or expand the views for this device
//       RenderView() - renders device configuration view with image,
//                       callout information, action mapping.
//-----------------------------------------------------------------------------
class DIDCfgView
{
public:
    DIDCfgView();
    ~DIDCfgView();

public:
    // main device configuration view interface
    HRESULT Init( LPDIRECTINPUTDEVICE8 pDevice );
    HRESULT SetActionFormat( LPDIACTIONFORMAT pDiaf );
    HRESULT SetCalloutState( DWORD dwFlags, DWORD dwObjID );
    HRESULT SetAllCalloutState( DWORD dwFlags );
    HRESULT GetObjectIDByLocation( LPDWORD pdwObjID, LPPOINT pPt );
    HRESULT SetCurrentView( INT nView );
    HRESULT RebuildViews( BOOL bCompact );
    HRESULT RenderView( HBITMAP hBitmap, BOOL bIsDib );
    HRESULT RenderView( HDC hdc );

    // rendering options
    HRESULT SetViewOrigin( const POINT* pPtOrg, POINT* pPtOldOrg );
    HRESULT CalcCenterOrgForCurrentView( const SIZE* pImgSize, POINT* pPtOrg, BOOL bSetOrigin = FALSE );
    VOID    SetDefaultText( const TCHAR* pctszDefaultText, DWORD length );
    VOID    SetColors( COLORREF crFore, COLORREF crBack, COLORREF crHighlight, COLORREF crHighlightLine );
    VOID    GetColors( COLORREF* pCrFore, COLORREF* pCrBack, COLORREF* pCrHighlight, COLORREF* pCrHighlightLine );
    HRESULT SetFont( const TCHAR*  pctszFontName, DWORD dwAttributes );
    HFONT   GetFont();

    // information access functios
    HRESULT GetInfo( DIDCfgViewInfo* pCfgViewInfo );
    HRESULT GetCurrentView( LPINT lpnView );
    HRESULT GetCalloutState( LPDWORD lpdwFlags, DWORD dwObjID );

protected:
    // protected helper functions
    VOID    CleanUp();
    HRESULT InitAlloc();
    HRESULT InitImageInfoRetrieve( LPDIRECTINPUTDEVICE8 pDevice );
    HRESULT InitImageInfoProcess();
    VOID    CalcSizes( const DIDEVICEIMAGEINFO* prgImageInfoArray, DWORD dwNumElements, const LPDWORD pNumViews, LPDWORD pNumCallouts, LPDWORD pNumDistinctObjID );
    HRESULT RenderView( HDC hdc, VOID* pBits, INT width, INT height );

    HRESULT InitCustomViews( LPDIRECTINPUTDEVICE8 pDevice, BOOL bUseInternal = FALSE );
    HRESULT BuildCustomViews();
    VOID    RenderFullname( HDC hdc, const TCHAR* pctszFullname, const RECT* pRect );
    BOOL    CopyActionMap( LPDIACTIONFORMAT pDiaf );

protected:
    // data structures for managing views and callouts
    DidcvViewManager* m_lpViewManager;
    DidcvCalloutManager* m_lpCalloutManager;

    // keeps track of whether this has been initialized
    BOOL m_bIsInit;
    // the index of the next view to render
    INT m_nView;
    // pointer to the DIDEVICEIMAGEINFOHEADER
    LPDIDEVICEIMAGEINFOHEADER m_lpDidImgHeader;
    // custom device view data
    DidcvCustomViewInfo* m_lpCustomViewInfo;

    // device reference
    LPDIRECTINPUTDEVICE8 m_lpDIDevice;
    // action mapping copy
    LPDIACTIONFORMAT m_lpDiaf;
    // can this view be collapsed
    BOOL m_bCanBeCollapsed;
    // is this view compacted
    BOOL m_bIsCollapsed;

    // rendering options
    COLORREF m_crFore;
    COLORREF m_crBack;
    COLORREF m_crHighlight;
    COLORREF m_crHighlightLine;
    POINT    m_ptOrigin;
    HFONT    m_hFont;
    TCHAR m_tszDefaultText [MAX_PATH];
};




//-----------------------------------------------------------------------------
// Name: struct DIDCfgViewInfo
// Desc: struct containing current information about DIDCfgView states
//-----------------------------------------------------------------------------
struct DIDCfgViewInfo
{
    BOOL  bIsInit;              // is the CfgView object initialized
    INT   iCurrentViewID;       // the ID of the current view
    INT   iNumTotalViews;       // total number of views
    DWORD dwNumUniqueCallouts;  // number of unique
    BOOL  bCanBeCollapsed;      // can views for this CfgView object be compacted
    BOOL  bIsCollapsed;         // is the CfgView currently using collapsed views


    DIDCfgViewInfo()
        : bIsInit( FALSE ),
          iCurrentViewID( DIDCV_INVALID_ID ),
          iNumTotalViews( 0 ),
          dwNumUniqueCallouts( 0 ),
          bCanBeCollapsed( FALSE ),
          bIsCollapsed( FALSE )
    { }
};




//-----------------------------------------------------------------------------
// Name: struct DidcvCustomViewInfo
// Desc: object containing information about custom views
//-----------------------------------------------------------------------------
struct DidcvCustomViewInfo
{
    DWORD dwType;               // type of the operation(count or save)
    DWORD dwCount;              // the total count of items
    DWORD dwSize;               // number of items actually in array
    LPDIDEVICEOBJECTINSTANCE* rgObjData;   // array of control info

    DidcvCustomViewInfo()
        : dwType( 0 ),
          dwCount( 0 ),
          dwSize( 0 ),
          rgObjData( NULL )
    { }

    ~DidcvCustomViewInfo() { CleanUp(); }
    
    VOID CleanUp();
};




//-----------------------------------------------------------------------------
// Name: class DidcvView
// Desc: data structure for representing one view of a device
//-----------------------------------------------------------------------------
class DidcvView
{
public:
    virtual ~DidcvView() { };

    // DidcvView interface
    void SetOffset( INT nOffset );
    void SetID( INT nID );
    INT  GetOffset() const;
    INT  GetID() const;

    // rendering
    virtual BOOL GetViewSize( SIZE* pSize ) const = 0;
    virtual void Render( HDC hdc, VOID* pBits, INT width, INT height, const POINT* pPtOrigin ) = 0;

protected:
    // constructor
    DidcvView();
    // the original offset in the array returned by DirectInputDevice8::GetImage
    INT m_nOffset;
    // the internally assigned unique identifier
    INT m_nID;
};




//-----------------------------------------------------------------------------
// Name: class DidcvBitmapView
// Desc: subclass of DidcvView that renders a bitmap
//-----------------------------------------------------------------------------
class DidcvBitmapView : public DidcvView
{
public:
    ~DidcvBitmapView();

    // interface
    virtual BOOL GetViewSize( SIZE* pSize ) const;
    virtual void Render( HDC hdc, VOID* pBits, INT width, INT height, const POINT* pPtOrigin );

public:
    // function to instantiate a DidcvBitmapView
    static DidcvBitmapView* Create( LPCTSTR ptszImagePath, DWORD dwFlags );

private:
    // constructor
    DidcvBitmapView();

protected:
    // bitmap of the view
    DidcvBitmap* m_lpBitmap;
};




//-----------------------------------------------------------------------------
// Name: class DidcvCustomView
// Desc: subclass of DidcvView that renders a view from custom data
//-----------------------------------------------------------------------------
class DidcvCustomView : public DidcvView
{
public:
    ~DidcvCustomView() { }

    // interface
    virtual BOOL GetViewSize( SIZE* pSize ) const;
    virtual void Render( HDC hdc, VOID* pBits, INT width, INT height, const POINT* pPtOrigin );

public:
    // static function to instantiate a DidcvCustomView
    static DidcvCustomView* Create( const DidcvCustomViewInfo* pInfo, DWORD dwStartIndex, DWORD* pFinishIndex );
    static BOOL CalcImageInfo( DWORD index, LPRECT rcOverlay, LPDWORD pNumPoints, LPPOINT rgptCalloutLine, LPRECT rcCalloutRect );
    static DWORD CalcNumViews( DWORD dwCount );

private:
    // constructor
    DidcvCustomView( const DidcvCustomViewInfo* pInfo, DWORD dwStartIndex, DWORD dwEndIndex );

protected:
    // returns coordinates for a given index
    void CalcCoordinates( DWORD dwIndex, LPRECT lpRect );

    // custom data from which to render the view
    const DidcvCustomViewInfo* m_lpCustomViewInfoRef;
    DWORD m_dwStartIndex;
    DWORD m_dwEndIndex;
};




//-----------------------------------------------------------------------------
// Name: class DidcvViewManager
// Desc: stores and manages all the views for particular device
//-----------------------------------------------------------------------------
class DidcvViewManager
{
public:
    DidcvViewManager();
    ~DidcvViewManager();

    // interface
    BOOL SetCapacity( UINT uCapacity, BOOL bDeleteContent = TRUE );
    BOOL AddImage( DidcvView* pView, INT nOffset = DIDCV_INVALID_ID );
    DidcvView* GetImage( INT nID );
    DidcvView* GetImageByOffset( INT nOffset );
    UINT GetNumViews() const;
    void CleanUp();

protected:
    // table holding references to views, indexed by the internal unique identifier
    DidcvView ** m_ppViewTable;
    UINT m_capacity;
    UINT m_size;
};




//-----------------------------------------------------------------------------
// Name: class DidcvCallout
// Desc: data structure for representing one callout on a particular view
//       A callout is the line drawn from each axis/button to a label
//-----------------------------------------------------------------------------
class DidcvCallout
{
public:
    ~DidcvCallout();

    // accessor functions to information retrieved from DirectInput
    DWORD GetObjID() const;
    const RECT & GetOverlayRect() const;
    const RECT & GetCalloutRect() const;
    const POINT* GetCalloutLine( DWORD* lpNumPts ) const;
    DWORD GetTextAlign() const;
    DWORD GetOverlayOffset() const;

    // accessor functions to internal data
    void  SetAssociatedViewID( INT nViewRef );
    INT   GetAssociatedViewID() const;
    void  SetDataRef( const DidcvCalloutData* lpData );
    const DidcvCalloutData* GetDataRef() const;

    // hit test for a given point
    DWORD HitTest( LPPOINT lpPt ) const;

    // draw the overlay
    DWORD DrawOverlay( HDC hDC, VOID* lpBits, INT width, INT height, const POINT* pptOrigin );

public:
    // static functions to instantiate a callout object
    static DidcvCallout* Create( LPDIDEVICEIMAGEINFO devImgInfo );

private:
    // private constructor
    DidcvCallout();

protected:
    // copy of device information
    DIDEVICEIMAGEINFO m_devImgInfo;
    // the view that this particular callout is associated with
    INT m_nViewRef;
    // pointer to callout state/data
    const DidcvCalloutData* m_lpDataRef;
    // the bitmap of the overlay
    DidcvBitmap* m_lpOverlayBitmap;
};




// default array size
#define GW_ARRAY_DEFAULT_SIZE       4

//-----------------------------------------------------------------------------
// Name: class GwArray
// Desc: templated c-style array class for PRIMITIVE data types only
//-----------------------------------------------------------------------------
template <class Item> 
class GwArray
{
public:
    // constructors
    GwArray() { this->Alloc( GW_ARRAY_DEFAULT_SIZE ); m_size = 0; }
    GwArray( UINT initCap ) { this->Alloc( initCap ); m_size = 0; }
    ~GwArray() { this->DeAlloc(); m_size = 0; }

public:
    void SetSize( UINT newCap )
    {
        if( newCap == m_capacity )
            return;

        m_size =( newCap < m_size ? newCap : m_size );

        UINT old_cap = m_capacity;
        UINT numoverlap =( newCap > m_capacity ? m_capacity : newCap );

        Item* oldList = m_list;
        this->Alloc( newCap );

        if( oldList )
        {
            memcpy( m_list, oldList, sizeof( Item )* numoverlap );

            // free( oldList );
            delete [] oldList;
        }
    }

    Item & operator[]( UINT index )
    {
        if( index >= m_capacity )
            assert( index >= 0 && index < m_capacity );

        return m_list[index];
    }

    const Item & operator[]( UINT index ) const
    {
        if( index >= m_capacity )
            assert( index >= 0 && index < m_capacity );

        return m_list[index];
    }

    void PushBack( const Item & item )
    {
        if( m_capacity == 0 )
            SetSize( 2 );
        else if( m_size >= m_capacity )
            SetSize( m_capacity* 2 );

        m_list[m_size] = item;
        m_size++;
    }

    void Resize( UINT newCap ) { SetSize( newCap ); }
    void PopBack()        { if( m_size ) m_size--; }
    void Clear()          { m_size = 0; }
    void Trim()           { Resize( m_size ); }
    UINT Capacity() const { return m_capacity; }
    UINT Size() const     { return m_size; }

protected:
    inline void Alloc( UINT cap )
    {
        if( cap == 0 )
            m_list = NULL;
        else
        {
            m_list = new Item[cap];
            assert( m_list );
            memset( m_list, 0, sizeof( Item )* cap );
        }

        m_capacity = cap;
    }

    inline void DeAlloc()
    {
        if( m_list != NULL )
        {
            //free( m_list );
            delete [] m_list;
            m_list = NULL;
        }

        m_capacity = 0;
    }

protected:
    // array
    Item* m_list;
    // number of entries the array can hold
    UINT m_capacity;
    // number of entries added using PushBack() minus # removed using PopBack()
    UINT m_size;
};




//-----------------------------------------------------------------------------
// Name: class DidcvCalloutApplicant
// Desc: abstract base class for processing DidcvCallout
//-----------------------------------------------------------------------------
class DidcvCalloutApplicant
{
public:
    virtual ~DidcvCalloutApplicant() { }
    virtual BOOL Apply( DidcvCallout* pCallout ) = 0;
};




//-----------------------------------------------------------------------------
// Name: class DidcvCalloutSet
// Desc: a group of DidcvCallout references
//       A callout is the line drawn from each axis/button to a label
//-----------------------------------------------------------------------------
class DidcvCalloutSet
{
public:
    DidcvCalloutSet();
    ~DidcvCalloutSet();

public:
    BOOL  AddCallout( DidcvCallout* pCallout );
    void  Apply( DidcvCalloutApplicant* pCalloutApp );
    void  SetIdentifier( DWORD dwID );
    DWORD GetIdentifier() const;
    void  SetData( void* pData );
    void* GetData() const;

    const GwArray <DidcvCallout*> & GetInternalArrayRef() const;
    void  TrimArrays();  

protected:
    void CleanUp();

protected:
    GwArray <DidcvCallout*> m_calloutList;
    DWORD m_dwSetID;
    void* m_lpData;
};




//-----------------------------------------------------------------------------
// Name: class DidcvCalloutManager
// Desc: data structure for storing and managing callouts
//       A callout is the line drawn from each axis/button to a label
//-----------------------------------------------------------------------------
class DidcvCalloutManager
{
public:
    DidcvCalloutManager();
    ~DidcvCalloutManager();

    // main interface
    BOOL  AddCallout( DidcvCallout* pCallout, INT nView );
    BOOL  SetCalloutState( const DidcvCalloutState* pCalloutState, DWORD dwObjID );
    BOOL  SetAllCalloutState( const DidcvCalloutState* pCalloutState );
    BOOL  SetActionMap( const LPDIACTION pAction, DWORD dwObjID );
    void  ClearAllActionMaps();

    // information
    DWORD GetObjectIDByLocation( const LPPOINT pPt, INT nView );
    BOOL  GetCalloutState( DidcvCalloutState* pCalloutState, DWORD dwObjID );
    BOOL  GetActionMap( DidcvActionMap* pActionMap, DWORD dwObjID );
    const DidcvCalloutSet* GetCalloutSetByView( INT nView ) const;
    const DidcvCalloutSet* GetCalloutSetByObjID( DWORD dwObjID ) const;
    const DidcvCalloutData* GetCalloutDataRef( DWORD dwObjID ) const;
    UINT  GetNumUniqueCallouts() const;

    BOOL  EnumObjects( LPDIRECTINPUTDEVICE8 pDevice, LPDIENUMDEVICEOBJECTSCALLBACK pCallback, LPVOID pvRef, DWORD dwMapOnly );
    BOOL  CalcCanBeCollapsed();

    // allocation
    BOOL  SetCapacity( DWORD dwNumCallouts, DWORD dwNumUniqueObjID, DWORD dwNumViews, BOOL bDeleteContent = TRUE );
    void  TrimArrays();
    void  CleanUp();

protected:
    // helper functions
    DidcvCalloutSet*  Find( const GwArray <DidcvCalloutSet*> & array, DWORD dwIdentifier ) const;
    DidcvCalloutData* GetCalloutData( DWORD dwObjID ) const;

protected:
    // list of all callouts added
    GwArray <DidcvCallout*> m_calloutList;
    // list of callout sets, one for each unique callout id
    GwArray <DidcvCalloutSet*> m_calloutSetListByObjID;
    // list of callout sets, one for each view
    GwArray <DidcvCalloutSet*> m_calloutSetListByView;
};




//-----------------------------------------------------------------------------
// Name: struct DidcvCalloutData
// Desc: data structure holding references to callout data components
//-----------------------------------------------------------------------------
struct DidcvCalloutData
{
    DidcvCalloutState* lpState;    // callout state info
    DidcvActionMap* lpActionMap;   // action mapped this callout

    DidcvCalloutData( DidcvCalloutState* s, DidcvActionMap* a )
        : lpState( s ), lpActionMap( a )
    { }
};




//-----------------------------------------------------------------------------
// Name: struct DidcvCalloutState
// Desc: state information for a callout
//-----------------------------------------------------------------------------
struct DidcvCalloutState
{
    // whether to draw
    BOOL bDrawCallout;
    BOOL bDrawOverlay;
    BOOL bDrawHighlight;
    BOOL bDrawEmptyCallout;
    BOOL bDrawFullname;

    // specifies which state is valid
    DWORD dwFlags;

    // --- member functions ---
    DidcvCalloutState( DWORD f = 0, BOOL c = FALSE, BOOL o = FALSE, 
        BOOL h = FALSE, BOOL e = FALSE, BOOL d = FALSE )
        : dwFlags( f ), bDrawCallout( c ), bDrawOverlay( o ), bDrawHighlight( h ), 
          bDrawEmptyCallout( e ), bDrawFullname( d )
    { }

    void SmartSet( const DidcvCalloutState* other );
    void Copy( const DidcvCalloutState* other ) { *this = *other; }
    DWORD MakeFlag() const;
    void SetFlag( DWORD dwExtFlags );
};




//-----------------------------------------------------------------------------
// Name: struct DidcvActionMap
// Desc: action mapping information for a callout
//-----------------------------------------------------------------------------
struct DidcvActionMap
{
    DIACTION dia;

    DidcvActionMap() { ZeroMemory( &dia, sizeof( DIACTION ) ); }
    void Copy( const DidcvActionMap* other ) { this->dia = other->dia; }

    LPCSTR GetActionName() const { return dia.lptszActionName; }
};




// utility functions
void  DidcvPolyLineArrow( HDC hDC, const POINT* rgpt, INT nPoints, BOOL bDoShadow = FALSE );
HFONT DidcvCreateFont( HDC hdc, const TCHAR* szFaceName, int iDeciPtHeight, int iDeciPtWidth, int iAttributes, BOOL fLogRes);




//-----------------------------------------------------------------------------
// Name: struct rgref
// Desc: templated lightweight c-style array
//-----------------------------------------------------------------------------
template <class T>
struct rgref {
    rgref( T* p ) : pt( p ) {}

    T & operator []( int i ) { return pt[i]; }
    const T & operator []( int i ) const { return pt[i]; }

private:
    T *pt;
};




//-----------------------------------------------------------------------------
// Name: struct SPOINT
// Desc: used by line drawing routine
//-----------------------------------------------------------------------------
struct SPOINT {
    SPOINT()
#define SPOINT_INITIALIZERS \
        p( u.p ), \
        s( u.s ), \
        a((( int* )( void* ) u.a ) ), \
        x( u.p.x ), \
        y( u.p.y ), \
        cx( u.s.cx ), \
        cy( u.s.cy )
        : SPOINT_INITIALIZERS
        { x = y = 0; }

    SPOINT( int, POINT *r ) 
        : p( *r ),
          s( *(( SIZE* )( void* ) r ) ),
          a((( int* )( void* ) r ) ),
          x( r->x ),
          y( r->y ),
          cx( r->x ),
          cy( r->y )
    { }

    SPOINT( const SPOINT & sp ) 
        : SPOINT_INITIALIZERS
    { p = sp.p; }

    SPOINT( int b, int c ) 
        : SPOINT_INITIALIZERS
    { x = b; y = c; }

    SPOINT( const POINT &point )
        : SPOINT_INITIALIZERS
    { p = point; }

    SPOINT( const SIZE &size )
        : SPOINT_INITIALIZERS
    { s = size; }

#undef SPOINT_INITIALIZERS

    SPOINT operator =( const SPOINT &sp ) { p = sp.p; return *this; }
    SPOINT operator =( const POINT &_p ) { p = _p; return *this; }
    SPOINT operator =( const SIZE &_s ) { s = _s; return *this; }

    operator POINT() const { return p; }
    operator SIZE() const { return s; }

    long &x, &y, &cx, &cy;
    POINT &p;
    SIZE &s;
    rgref<int> a;

private:
    union {
        POINT p;
        SIZE s;
        int a[2];
    } u;
};




//-----------------------------------------------------------------------------
// Name: class DidcvBitmap
// Desc: object containing a bitmap
//-----------------------------------------------------------------------------
class DidcvBitmap
{
public:
    ~DidcvBitmap();

    // drawing interface
    BOOL Draw( HDC hDC, INT xStart, INT yStart);
    BOOL Blend( HDC hDC, INT xStart, INT yStart );
    BOOL Blend( VOID* lpBits, INT xStart, INT yStart, INT width, INT height );

    // information
    BOOL GetSize( SIZE* lpSize ) const;
    HBITMAP GetHandle();
    LPVOID GetBits();

public:
    // static function for instantiating a DidcvBitmap
    static DidcvBitmap* Create( LPCTSTR tszFilename );
    static DidcvBitmap* Create( INT width, INT height );

private:
    // private constructor
    DidcvBitmap();

protected:
    // helper functions
    void CleanUp();
    void FigureSize();

    static DidcvBitmap* CreateViaD3dx( LPCTSTR tszFilename );
    static DidcvBitmap* CreateViaLoadImage( HINSTANCE hinst, LPCTSTR tszName, 
        UINT uType, int cx, int cy, UINT fuLoad );

protected:
    // GDI handle to bitmap
    HBITMAP m_hbitmap;
    VOID* m_lpBits;
    SIZE m_size;
};




// alpha blending information
#define DIDCV_ALPHABLEND_DLL_NAME   TEXT( "MSIMG32.DLL" )
#define DIDCV_ALPHABLEND_PROC_NAME  TEXT( "AlphaBlend" )

#if( WINVER >= 0x400 )
  typedef WINGDIAPI BOOL( WINAPI* DIDCV_ALPHABLEND )( HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION );
#else
  typedef DIDCV_ALPHABLEND DWORD
#endif

//-----------------------------------------------------------------------------
// Name: class DidcvAlphaBlend
// Desc: utility class for alpha blending
//-----------------------------------------------------------------------------
class DidcvAlphaBlend
{
public:
    // reference counting interface
    static BOOL AddClient();
    static BOOL ReleaseClient();

    // functions to perform blending
    static BOOL Blend( HDC hDC, INT xStart, INT yStart, INT width, INT height, HBITMAP hbitmap, const SIZE* lpSize );
    static BOOL Blend( VOID* lpDestBits, INT xStart, INT yStart, INT destWidth, INT destHeight, VOID* lpSrcBits, INT srcWidth, INT srcHeight );

protected:
    static DIDCV_ALPHABLEND s_alphaBlendProc;
    static HMODULE s_hDll;
    static DWORD s_dwNumClients;

};




#endif // #ifndef __DIDCV_H__
