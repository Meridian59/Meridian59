//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of GDI DC encapsulation classes:
///   TDC, TWindowDC, TScreenDC, TDesktopDC, TClientDC, TPaintDC, TMetaFileDC,
///   TCreatedDC, TIC, TMemoryDC, TDibDC, TPrintDC
//----------------------------------------------------------------------------

#if !defined(OWL_DC_H)
#define OWL_DC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gdibase.h>
#include <owl/geometry.h>


namespace owl {

class _OWLCLASS TPen;
class _OWLCLASS TBrush;
class _OWLCLASS TFont;
class _OWLCLASS TBitmap;
class _OWLCLASS TPalette;
class _OWLCLASS TIcon;
class _OWLCLASS TCursor;
class _OWLCLASS TDib;
class _OWLCLASS TRegion;
class _OWLCLASS TMetaFilePict;
class _OWLCLASS TEnhMetaFilePict;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup dc
/// @{
/// \class TDC
// ~~~~~ ~~~
/// TDC is the root class for GDI DC wrappers. Each TDC object inherits a Handle
/// from TGdiBase and casts that Handle to an HDC using the HDC operator. Win API
/// functions that take an HDC argument can therefore be called by a corresponding
/// TDC member function without this explicit handle argument.
/// 
/// DC objects can be created directly with TDC constructors, or via the
/// constructors of specialized subclasses (such as TWindowDC, TMemoryDC,
/// TMetaFileDC, TDibDC, and TPrintDC) to get specific behavior. DC objects can be
/// constructed with an already existing and borrowed HDC handle or from scratch by
/// supplying device driver information, as with ::CreateDC. The class TCreateDC
/// takes over much of the creation and deletion work from TDC.
/// 
/// TDC has four handles as protected data members: OrgBrush, OrgPen, OrgFont, and
/// OrgPalette. These handles keep track of the stock GDI objects selected into each
/// DC. As new GDI objects are selected with SelectObject or SelectPalette, these
/// data members store the previous objects. The latter can be restored individually
/// with RestoreBrush, RestorePen, and so on, or they can all be restored with
/// RestoreObjects. When a TDC object is destroyed (via ~TDC::TDC), all the
/// originally selected objects are restored. The data member TDC::ShouldDelete
/// controls the deletion of the TDC object.
//
class _OWLCLASS TDC : protected TGdiBase {
  public:
      /// Which edge(s) to draw. ctor defaults to all 4
    enum TFlag {
      Left        = 0x0001,
      Top         = 0x0002,
      Right       = 0x0004,
      Bottom      = 0x0008,
      TopLeft     = Top | Left,
      TopRight    = Top | Right,
      BottomLeft  = Bottom | Left,
      BottomRight = Bottom | Right,
      Rect        = Top | Left | Bottom | Right,

      Diagonal    = 0x0010,  ///< Draw diagonal edge (NotAvail intrnl)
      Fill        = 0x0800,  ///< Fill in middle
      Soft        = 0x1000,  ///< Soft edge look for buttons
      Adjust      = 0x2000,  ///< Adjust passed rect to client (Not Applicable)
      Flat        = 0x4000,  ///< Flat instead of 3d for use in non-3d windows
      Mono        = 0x8000  ///< Monochrome
    };

    /// Enumeration describing hilevel border styles
    //
    enum TStyle {
      None,             ///<  No border painted at all
      Plain,            ///<  Plain plain window frame
      Raised,           ///<  Status field style raised
      Recessed,         ///<  Status field style recessed
      Embossed,         ///<  Grouping raised emboss bead
      Grooved,          ///<  Grouping groove
      ButtonUp,         ///<  Button in up position
      ButtonDn,         ///<  Button in down position
      WndRaised,        ///<  Raised window outer+inner edge
      WndRecessed,      ///<  Input field & other window recessed
      WellSet,          ///<  Well option set (auto grows + 1)  // !CQ W4 cant do
      ButtonUp3x,       ///<  Button in up position, Win 3.x style
      ButtonDn3x       ///<  Button in down position, Win 3.x style
    };
//    TUIBorder(const TRect& frame, TStyle style, uint flags = 0);

    /// Enumeration describing the type of edge to be drawn
    //
    enum TEdge {
      RaisedOuter = 0x01,   ///< Raised outer edge only
      SunkenOuter = 0x02,   ///< Sunken outer edge only
      RaisedInner = 0x04,   ///< Raised inner edge only
      SunkenInner = 0x08,   ///< Sunken inner edge only
      EdgeOuter =   0x03,   ///< Mask for outer edge bits
      EdgeInner =   0x0C,   ///< Mask for inner edge bits
      EdgeRaised =  RaisedOuter | RaisedInner,   ///< Both inner & outer raised
      EdgeSunken =  SunkenOuter | SunkenInner,   ///< Both inner & outer sunken
      EdgeEtched =  SunkenOuter | RaisedInner,   ///< Outer sunken, inner raised
      EdgeBump   =  RaisedOuter | SunkenInner   ///< Outer raised, inner sunken
    };

    // Constructors / destructor
    //
    TDC(HDC handle);  ///< use an existing DC, doesn't auto delete it
    
    TDC();            ///< for derived classes only
    
    virtual    ~TDC();

    // Type Conversion Operators
    //
    operator    HDC() const;  ///< Must assume const is OK

    /// Validation routine - used mainly for diagnostic purposes
    //
    bool        IsDC() const;

    // DC functions
    //

    void SetDC(HDC hdc) { Handle = hdc; }                   // DLN
    bool         GetDCOrg(TPoint& point) const;
    virtual int  SaveDC() const;
    virtual bool RestoreDC(int savedDC = -1);
    virtual int  GetDeviceCaps(int index) const;
    virtual bool ResetDC(DEVMODE & devMode);

    // Select GDI objects into this DC and restore them to original
    //
    void         SelectObject(const TBrush& brush);
    void         SelectObject(const TPen& pen);
    virtual void SelectObject(const TFont& font);
    void         SelectObject(const TPalette& palette, bool forceBackground=false);
    void         SelectObject(const TBitmap& bitmap);
    virtual void SelectStockObject(int index);
    void         RestoreBrush();
    void         RestorePen();
    virtual void RestoreFont();
    void         RestorePalette();
    void         RestoreBitmap();
    void         RestoreTextBrush();
    void         RestoreObjects();
    HANDLE       GetCurrentObject(uint objectType) const;

    // Drawing tool functions
    //
    bool         GetBrushOrg(TPoint& point) const;
    bool         SetBrushOrg(const TPoint& origin, TPoint * oldOrg=0);
    int          EnumObjects(uint objectType, GOBJENUMPROC proc, void * data) const;

    // Color and palette functions
    //
    TColor       GetNearestColor(const TColor& color) const;
    int          RealizePalette();
    void         UpdateColors();
    uint         GetSystemPaletteEntries(int start, int num,
                                         PALETTEENTRY * entries) const;
    uint         GetSystemPaletteUse() const;
    int          SetSystemPaletteUse(int usage);

    // Drawing attribute functions
    //
    TColor         GetBkColor() const;
    virtual TColor SetBkColor(const TColor& color);
    int          GetBkMode() const;
    int          SetBkMode(int mode);
    int          GetPolyFillMode() const;
    int          SetPolyFillMode(int mode);
    int          GetROP2() const;
    int          SetROP2(int mode);
    int          GetStretchBltMode() const;
    int          SetStretchBltMode(int mode);
    TColor         GetTextColor() const;
    virtual TColor SetTextColor(const TColor& color);

    bool         SetMiterLimit(float newLimit, float* oldLimit=0);

    // Viewport & window mapping functions
    //
    int          GetMapMode() const;
    virtual int  SetMapMode(int mode);

    bool         SetWorldTransform(XFORM & xform);
    bool         ModifyWorldTransform(XFORM & xform, uint32 mode);

    bool         GetViewportOrg(TPoint& point) const;
    TPoint       GetViewportOrg() const;
    virtual bool SetViewportOrg(const TPoint& origin, TPoint * oldOrg=0);
    virtual bool OffsetViewportOrg(const TPoint& delta, TPoint * oldOrg=0);

    bool         GetViewportExt(TSize& extent) const;
    TSize        GetViewportExt() const;
    virtual bool SetViewportExt(const TSize& extent, TSize * oldExtent=0);
    virtual bool ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom,
                                  TSize * oldExtent=0);

    bool         GetWindowOrg(TPoint& point) const;
    TPoint       GetWindowOrg() const;
    bool         SetWindowOrg(const TPoint& origin, TPoint * oldOrg=0);
    bool         OffsetWindowOrg(const TPoint& delta, TPoint * oldOrg=0);

    bool         GetWindowExt(TSize& extent) const;
    TSize        GetWindowExt() const;
    virtual bool SetWindowExt(const TSize& extent, TSize * oldExtent=0);
    virtual bool ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom,
                                TSize * oldExtent=0);

    // Coordinate functions
    //
    bool        DPtoLP(TPoint* points, int count = 1) const;
    bool        DPtoLP(TRect& rect) const;
    bool        LPtoDP(TPoint* points, int count = 1) const;
    bool        LPtoDP(TRect& rect) const;

    // Clip & Update Rect & Rgn functions
    //
    int         GetClipBox(TRect& rect) const;
    TRect       GetClipBox() const;
    int         ExcludeClipRect(const TRect& rect);
    int         ExcludeUpdateRgn(HWND wnd);
    int         IntersectClipRect(const TRect& rect);
    int         OffsetClipRgn(const TPoint& delta);
    int         SelectClipRgn(const TRegion& region);

    //
    /// Selects the given region as the current clipping region for the device context.
    /// The default argument 0 has the effect of removing the device context's clipping region.
    //
    int SelectClipRgn(HRGN r = 0) {return ::SelectClipRgn(GetHDC(), r);}

    //
    /// Removes the device context's clipping region.
    /// Has the same effect as calling SelectClipRgn with no argument (or NULL).
    //
    int RemoveClipRgn() {return SelectClipRgn();}

    bool        PtVisible(const TPoint& point) const;
    bool        RectVisible(const TRect& rect) const;
    bool        GetBoundsRect(TRect& bounds, uint16 flags) const;
    uint        SetBoundsRect(TRect& bounds, uint flags);
    bool        GetClipRgn(TRegion& region) const;

    // MetaFile functions
    //
    int         EnumMetaFile(const TMetaFilePict& metaFile,
                             MFENUMPROC callback, void * data) const;
    bool        PlayMetaFile(const TMetaFilePict& metaFile);
    void        PlayMetaFileRecord(HANDLETABLE & handletable,
                                   METARECORD & metaRecord, int count);
    int         EnumEnhMetaFile(const TEnhMetaFilePict& metaFile,
                                ENHMFENUMPROC callback, void* data,
                                const TRect* rect) const;
    bool        PlayEnhMetaFile(const TEnhMetaFilePict& metaFile, const TRect* rect);
    void        PlayEnhMetaFileRecord(HANDLETABLE& handletable,
                                      ENHMETARECORD& metaRecord, uint count);

    // Output functions- Current position
    //
    bool        GetCurrentPosition(TPoint& point) const;
    bool        MoveTo(int x, int y);
    bool        MoveTo(const TPoint& point);
    bool        MoveTo(const TPoint& point, TPoint& oldPoint);

    // Output functions- Icons & cursors
    //
    bool        DrawIcon(int x, int y, const TIcon& icon);
    bool        DrawIcon(const TPoint& point, const TIcon& icon);
    bool        DrawCursor(int x, int y, const TCursor& cursor);
    bool        DrawCursor(const TPoint& point, const TCursor& cursor);
    bool        DrawIcon(int x, int y, const TIcon& icon, int w, int h,
                         int aniStep=0, HBRUSH bkBr=0, uint flags=DI_NORMAL);
    bool        DrawIcon(const TPoint& point, const TIcon& icon,
                         const TSize& size, int aniStep=0, HBRUSH bkBr=0,
                         uint flags=DI_NORMAL);
    bool        DrawCursor(int x, int y, const TCursor& cursor, int w, int h,
                           int aniStep=0, HBRUSH bkBr=0, uint flags=DI_NORMAL);
    bool        DrawCursor(const TPoint& point, const TCursor& cursor,
                           const TSize& size, int aniStep=0, HBRUSH bkBr=0,
                           uint flags=DI_NORMAL);

    // Output functions- Rects
    //
    bool        FrameRect(int x1, int y1, int x2, int y2, const TBrush& brush);
    bool        FrameRect(const TRect& rect, const TBrush& brush);
    bool        FillRect(int x1, int y1, int x2, int y2, const TBrush& brush);
    bool        FillRect(const TRect& rect, const TBrush& brush);
    bool        InvertRect(int x1, int y1, int x2, int y2);
    bool        InvertRect(const TRect& rect);
    bool        DrawFocusRect(int x1, int y1, int x2, int y2);
    bool        DrawFocusRect(const TRect& rect);

    bool        DrawEdge(const TRect& frame, uint edge, uint flags);
    void        PaintFrame(const TRect& fr, uint flags, const TColor& tlColor, const TColor& brColor);
    // Filled rect using ExtTextOut, no dithered colors & uses or sets BkColor
    //
    bool        TextRect(int x1, int y1, int x2, int y2);
    bool        TextRect(const TRect& rect);
    bool        TextRect(int x1, int y1, int x2, int y2, const TColor& color);
    bool        TextRect(const TRect& rect, const TColor& color);
	bool        FillSolidRect(TRect& r, const TColor& color) { return TextRect(r,color); } //DLN added MFC look-alike

    //
    //
    void        OWLFastWindowFrame(TBrush &brush, TRect &r, int xWidth, int yWidth, uint32 rop = PATCOPY);

    // Output functions- Regions
    //
    bool        FrameRgn(const TRegion& region, const TBrush& brush, const TPoint& p);
    bool        FillRgn(const TRegion& region, const TBrush& brush);
    bool        InvertRgn(const TRegion& region);
    bool        PaintRgn(const TRegion& region);

    // Output functions- Shapes
    //
    bool        AngleArc(int x, int y, uint32 radius, float startAngle,
                         float sweepAngle);
    bool        AngleArc(const TPoint& center, uint32 radius, float startAngle,
                         float sweepAngle);
    bool        Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4,
                    int y4);
    bool        Arc(const TRect& r, const TPoint& start, const TPoint& end);
    bool        Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4,
                      int y4);
    bool        Chord(const TRect& R, const TPoint& Start, const TPoint& End);
    bool        Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4,
                    int y4);
    bool        Pie(const TRect& rect, const TPoint& start, const TPoint& end);
    bool        Ellipse(int x1, int y1, int x2, int y2);
    bool        Ellipse(const TPoint& p1, const TPoint& p2);
    bool        Ellipse(const TPoint& point, const TSize& size);
    bool        Ellipse(const TRect& rect);
    bool        LineTo(int x, int y);
    bool        LineTo(const TPoint& point);
    bool        Polyline(const TPoint* points, int count);
    bool        Polygon(const TPoint* points, int count);
    bool        PolyPolygon(const TPoint* points, const int* PolyCounts,
                            int count);
    bool        PolyPolyline(const TPoint* points, const int* PolyCounts,
                             int count);
    bool        PolyBezier(const TPoint* points, int count);
    bool        PolyBezierTo(const TPoint* points, int count);
    bool        PolyDraw(const TPoint* points, uint8* types, int count);
    bool        PolylineTo(const TPoint* points, int count);
    bool        Rectangle(int x1, int y1, int x2, int y2);
    bool        Rectangle(const TPoint& p1, const TPoint& p2);
    bool        Rectangle(const TPoint& point, const TSize& s);
    bool        Rectangle(const TRect& rect);
    bool        RoundRect(int x1, int y1, int x2, int y2, int x3, int y3);
    bool        RoundRect(const TPoint& p1, const TPoint& p2, const TPoint& rad);
    bool        RoundRect(const TPoint& p, const TSize& s, const TPoint& rad);
    bool        RoundRect(const TRect& rect, const TPoint& rad);

    // Output functions- bitmaps & blitting
    //
    TColor      GetPixel(int x, int y) const;
    TColor      GetPixel(const TPoint& point) const;
    TColor      SetPixel(int x, int y, const TColor& color);
    TColor      SetPixel(const TPoint& p, const TColor& color);
    bool        BitBlt(int dstX, int dstY, int w, int h, const TDC& srcDC,
                       int srcX, int srcY, uint32 rop=SRCCOPY);
    bool        BitBlt(const TRect& dst, const TDC& srcDC,
                       const TPoint& src, uint32 rop=SRCCOPY);
    bool        PatBlt(int x, int y, int w, int h, uint32 rop=PATCOPY);
    bool        PatBlt(const TRect& dst, uint32 rop=PATCOPY);
    bool        ScrollDC(int x, int y, const TRect& scroll, const TRect& clip,
                         TRegion& updateRgn, TRect& updateRect);
    bool        ScrollDC(const TPoint& delta, const TRect& scroll,
                         const TRect& clip, TRegion& updateRgn,
                         TRect& updateRect);
    bool        StretchBlt(int dstX, int dstY, int dstW, int dstH,
                           const TDC& srcDC, int srcX, int srcY, int srcW,
                           int srcH, uint32 rop=SRCCOPY);
    bool        StretchBlt(const TRect& dst, const TDC& srcDC,
                           const TRect& src, uint32 rop=SRCCOPY);
    bool        MaskBlt(const TRect& dst, const TDC& srcDC,
                        const TPoint& src, const TBitmap& maskBm,
                        const TPoint& maskPos, uint32 rop=SRCCOPY);
    bool        PlgBlt(const TPoint& dst, const TDC& srcDC,
                       const TRect& src, const TBitmap& maskBm,
                       const TPoint& maskPos);
    bool        TransparentBlt(const TRect& dst, const TDC& srcDC, const TRect& src, const TColor& transparent);
    bool        GetDIBits(const TBitmap& bitmap, uint startScan, uint numScans,
                          void * bits, const BITMAPINFO & info,
                          uint16 usage);
    bool        GetDIBits(const TBitmap& bitmap, TDib& dib);
    bool        SetDIBits(TBitmap& bitmap, uint startScan, uint numScans,
                          const void * bits, const BITMAPINFO & Info,
                          uint16 usage);
    bool        SetDIBits(TBitmap& bitmap, const TDib& dib);
    bool        SetDIBitsToDevice(const TRect& dst, const TPoint& src,
                                  uint startScan, uint numScans,
                                  const void * bits,
                                  const BITMAPINFO & bitsInfo, uint16 usage);
    bool        SetDIBitsToDevice(const TRect& dst, const TPoint& src,
                                  const TDib& dib);
    bool        StretchDIBits(const TRect& dst, const TRect& src,
                              const void * bits,
                              const BITMAPINFO & bitsInfo,
                              uint16 usage, uint32 rop=SRCCOPY);
    bool        StretchDIBits(const TRect& dst, const TRect& src,
                              const TDib& dib, uint32 rop=SRCCOPY);

    HBITMAP     CreateDIBSection(const BITMAPINFO& info, uint usage, void** bits, HANDLE section=0, uint32 offset=0);

    bool        FloodFill(const TPoint& point, const TColor& color);
    bool        ExtFloodFill(const TPoint& point, const TColor& color, uint16 fillType);

    // Output functions- text
    //
    virtual bool TextOut(int x, int y, 
      const tstring& str, int count = -1);

    bool TextOut(const TPoint& p, 
      const tstring& str, int count = -1);

    virtual bool ExtTextOut(int x, int y, uint16 options, const TRect* r,
      const tstring& str, int count = -1, const int * dx = 0);

    bool ExtTextOut(const TPoint& p, uint16 options, const TRect* r, 
      const tstring& str, int count = -1, const int * dx = 0);

    virtual bool TabbedTextOut(const TPoint& p, 
      const tstring& str, int count, 
      int numPositions, const int* positions, int tabOrigin, TSize& size);

    bool TabbedTextOut(const TPoint& p, 
      const tstring& str, int count, 
      int numPositions, const int* positions, int tabOrigin);

    int DrawText(const tstring& str, int count, const TRect&, uint16 format = 0);

    virtual int DrawText(const tstring& str, int count, TRect&, uint16 format = 0);

    virtual int DrawTextEx(LPTSTR str, int count, TRect* = 0, 
      uint format=0, LPDRAWTEXTPARAMS = 0);

    bool DrawFrameControl(TRect lpRect, UINT nType, UINT nState);

    virtual bool GrayString(const TBrush& brush, GRAYSTRINGPROC outputFunc,
      const tstring& str, int count, const TRect& r);

    bool GetTextExtent(const tstring& str, int stringLen, TSize& size) const;

    TSize GetTextExtent(const tstring& str, int stringLen) const;

    bool GetTabbedTextExtent(const tstring& str, int stringLen, 
      int numPositions, const int * positions, TSize& size) const;

    TSize GetTabbedTextExtent(const tstring& str, int stringLen,
      int numPositions, const int * positions) const;

    uint GetTextAlign() const;

    uint SetTextAlign(uint flags);

    int GetTextCharacterExtra() const;

    int SetTextCharacterExtra(int extra);

    bool SetTextJustification(int breakExtra, int breakCount);

    int GetTextFaceLength() const;

    int GetTextFace(int count, LPTSTR facename) const;

    tstring GetTextFace() const;

    bool GetTextMetrics(TEXTMETRIC & metrics) const;

    uint32 GetGlyphOutline(uint chr, uint format, GLYPHMETRICS & gm,
      uint32 buffSize, void* buffer, const MAT2 & mat2);

    int GetKerningPairs(int pairs, KERNINGPAIR * krnPair);

    uint GetOutlineTextMetrics(uint data, OUTLINETEXTMETRIC & otm);

    // Font functions
    //
    bool        GetCharWidth(uint firstChar, uint lastChar, int* buffer);
    uint32      SetMapperFlags(uint32 flag);
    bool        GetAspectRatioFilter(TSize& size) const;
    int         EnumFonts(LPCTSTR faceName, OLDFONTENUMPROC callback,
                          void * data) const;

    int EnumFonts(const tstring& facename, OLDFONTENUMPROC callback, void* data) const
    {return EnumFonts(facename.c_str(), callback, data);}

    int         EnumFontFamilies(LPCTSTR family,
                                 FONTENUMPROC proc, void * data) const;

    int EnumFontFamilies(const tstring& family, FONTENUMPROC proc, void* data) const
    {return EnumFontFamilies(family.c_str(), proc, data);}

    uint32      GetFontData(uint32 table, uint32 offset, void* buffer, long data);
    bool        GetCharABCWidths(uint firstChar, uint lastChar, ABC* abc);

    // Path functions
    //
    bool        BeginPath();
    bool        CloseFigure();
    bool        EndPath();
    bool        FlattenPath();
    bool        WidenPath();
    bool        FillPath();
    bool        StrokePath();
    bool        StrokeAndFillPath();
    bool        SelectClipPath(int mode);
    HRGN        PathToRegion();

    // Override to implement a twin DC object, like TPrintPreviewDC
    //
    virtual HDC GetAttributeHDC() const;
    HDC         GetHDC() const;

  protected:
//    TDC();          // For use by derived classes only
    TDC(HDC handle, TAutoDelete autoDelete); //use an existing DC
    void        Init();

  protected:
    using TGdiBase::CheckValid;       ///< make this function available to derivatives
    using TGdiBase::Handle;           ///< The handle of this DC. Uses the base class's handle (TGdiBase::Handle.)
    using TGdiBase::ShouldDelete;     ///< Should object delete the Handle in dtor?

 protected_data:

/// Handle to the original GDI brush object for this DC. Holds the previous brush
/// object whenever a new brush is selected with SelectObject(brush).
    HBRUSH      OrgBrush;       

/// Handle to the original GDI pen object for this DC. Holds the previous pen object
/// whenever a new pen is selected with SelectObject(pen).
    HPEN        OrgPen;

/// Handle to the original GDI font object for this DC. Holds the previous font
/// object whenever a new font is selected with SelectObject(font).
    HFONT       OrgFont;

/// Handle to the original GDI palette object for this DC. Holds the previous
/// palette object whenever a new palette is selected with SelectObject(palette).
    HPALETTE    OrgPalette;

    HBITMAP     OrgBitmap;

/// The handle to the original GDI text brush object for this DC.
/// Stores the previous text brush handle whenever a new brush is selected with
/// SelectObject(text_brush).
    HBRUSH      OrgTextBrush;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TDC(const TDC&);
    TDC& operator =(const TDC&);
};

//
/// \class TWindowDC
// ~~~~~ ~~~~~~~~~
/// Derived from TDC, TWindowDC is a device context (DC) class that provides access
/// to the entire area owned by a window. TWindowDC is the base class for any DC
/// class that releases its handle when it is finished.
//
class _OWLCLASS TWindowDC : public TDC {
  public:
    TWindowDC(HWND wnd);
   ~TWindowDC();

  protected:
/// Holds a handle to the window owned by this device context.
    HWND        Wnd;
    TWindowDC();  // for derived classes

  private:
    TWindowDC(const TWindowDC&);
    TWindowDC& operator =(const TWindowDC&);
};

//
/// \class TScreenDC
// ~~~~~ ~~~~~~~~~
/// Derived from TWindowDC, TScreenDC is a DC class that provides direct access to
/// the screen bitmap. TScreenDC gets a DC for handle 0, which is for the whole
/// screen with no clipping. Handle 0 paints on top of other windows.
//
class _OWLCLASS TScreenDC : public TWindowDC {
  public:
    TScreenDC();

  private:
    TScreenDC(const TScreenDC&);
    TScreenDC& operator =(const TScreenDC&);
};

//
/// \class TDesktopDC
// ~~~~~ ~~~~~~~~~~
/// A DC class that provides access to the desktop window's client area which
/// is the window behind all other windows.
//
class _OWLCLASS TDesktopDC : public TWindowDC {
  public:
    TDesktopDC();

  private:
    TDesktopDC(const TDesktopDC&);
    TDesktopDC& operator =(const TDesktopDC&);
};

//
/// \class TClientDC
// ~~~~~ ~~~~~~~~~
/// A device context class derived from TWindowDC, TClientDC provides access to the
/// client area owned by a window.
//
class _OWLCLASS TClientDC : public TWindowDC {
  public:
    TClientDC(HWND wnd);

  private:
    TClientDC(const TClientDC&);
    TClientDC& operator =(const TClientDC&);
};

//
/// \class TPaintDC
// ~~~~~ ~~~~~~~~
/// A DC class that wraps begin and end paint calls for use in an WM_PAINT
/// response function.
//
class _OWLCLASS TPaintDC : public TDC {
  public:
    TPaintDC(HWND wnd);
   ~TPaintDC();

/// The paint structure associated with this TPaintDC object.
    PAINTSTRUCT Ps;

  protected:
/// The associated window handle.
    HWND        Wnd;

  private:
    TPaintDC(const TPaintDC&);
    TPaintDC& operator =(const TPaintDC&);
};

//
/// \class TMetaFileDC
// ~~~~~ ~~~~~~~~~~~
/// A DC class that provides access to a DC with a metafile selected into it
/// for drawing on (into).
//
class _OWLCLASS TMetaFileDC : public TDC {
  public:
    TMetaFileDC(LPCTSTR filename = 0);
    TMetaFileDC(const tstring& filename);
    TMetaFileDC(const TDC& dc, LPCTSTR filename = 0, TRect* rect = 0,
                LPCTSTR description = 0);
    TMetaFileDC(const TDC& dc, const tstring& filename, const TRect& rect, const tstring& appName, const tstring& picName);
   ~TMetaFileDC();

    HMETAFILE Close();

    HENHMETAFILE CloseEnh();
    bool Comment(uint bytes, const void* data);

    bool IsEnhanced() const;

  private:
    bool Enhanced;

    TMetaFileDC(const TMetaFileDC&);
    TMetaFileDC& operator =(const TMetaFileDC&);
};

//
/// \class TCreatedDC
// ~~~~~ ~~~~~~~~~~
/// An abstract TDC class, TCreatedDC serves as the base for DCs that are created
/// and deleted.
/// See TDC for more information about DC objects.
//
class _OWLCLASS TCreatedDC : public TDC {
  public:
    TCreatedDC(LPCTSTR driver, LPCTSTR device,
               LPCTSTR output, const DEVMODE * initData=0);
    TCreatedDC(const tstring& driver, const tstring& device = tstring(), const tstring& output = tstring(), const DEVMODE* = 0);

    TCreatedDC(HDC handle, TAutoDelete autoDelete);  // use an existing DC
   ~TCreatedDC();

  protected:
    TCreatedDC();

  private:
    TCreatedDC(const TCreatedDC&);
    TCreatedDC& operator =(const TCreatedDC&);
};

//
/// \class TIC
// ~~~~~ ~~~
/// Derived from TDC, TIC is a device context (DC) class that provides a constructor
/// for creating a DC object from explicit driver, device, and port names.
//
class _OWLCLASS TIC : public TCreatedDC {
  public:
    TIC(LPCTSTR driver, LPCTSTR device,
        LPCTSTR output, const DEVMODE * initData=0);
    TIC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* = 0);
   ~TIC();

  private:
    TIC(const TIC&);
    TIC& operator =(const TIC&);
};

//
/// \class TMemoryDC
// ~~~~~ ~~~~~~~~~
/// A device context (DC) class derived from TDC, TMemoryDC provides access to a
/// memory DC.
/// \note Important: In order to improve performance, ObjectWindows uses a cache for
/// memory device contexts. A draw-back of the cache is you cannot assume that
/// device contexts are initialized to the default system settings. (The cache may
/// return a device context that has not been used.)
//
class _OWLCLASS TMemoryDC : public TCreatedDC {
  public:
    TMemoryDC();
    TMemoryDC(const TDC& DC);
    TMemoryDC(HDC handle, TAutoDelete autoDelete);
    TMemoryDC(TBitmap& bitmap);
   ~TMemoryDC();

    // Select GDI objects into this DC and restore them to original
    //
    void        SelectObject(const TBrush& brush);
    void        SelectObject(const TPen& pen);
    void        SelectObject(const TFont& font);
    void        SelectObject(const TPalette& palette, bool forceBackground=false);
    void        SelectObject(const TBitmap& bitmap);

    void        RestoreBitmap();
    void        RestoreObjects();

  protected:
    TMemoryDC(LPCTSTR driver, LPCTSTR device,
              LPCTSTR output, const DEVMODE * initData=0);
    TMemoryDC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* initData);

    HBITMAP     OrgBitmap;

  private:
    void Init();

    TMemoryDC(const TMemoryDC&);
    TMemoryDC& operator =(const TMemoryDC&);
};

//
/// \class TDibDC
// ~~~~~ ~~~~~~
/// A DC class that provides access to DIBs using DIBSection, WinG, or the
/// DIB.DRV driver
//
class _OWLCLASS TDibDC : public TMemoryDC {
  public:
    TDibDC();
    TDibDC(const TBitmap& bitmap);

    uint        GetDIBColorTable(uint start, uint entries, RGBQUAD * colors);
    uint        SetDIBColorTable(uint start, uint entries, const RGBQUAD * colors);

    // Screen Update BitBlt's.  DC for display must be the destination.
    //
    bool BitBltToScreen(TDC& dstDC, const TRect& dst, const TPoint& src) const;
    bool BitBltToScreen(TDC& dstDC, int dstX, int dstY, int dstW, int dstH,
                        int srcX=0, int srcY=0) const;
    bool StretchBltToScreen(TDC& dstDC, const TRect& dst, const TRect& src) const;
    bool StretchBltToScreen(TDC& dstDC, int dstX, int dstY, int dstW, int dstH,
                            int srcX, int srcY, int srcW, int srcH) const;

  private:
    // prevent accidental copying of object
    //
    TDibDC(const TDibDC&);
    TDibDC& operator =(const TDibDC&);
};

//
/// \struct TBandInfo
// ~~~~~~ ~~~~~~~~~
/// Equivalent to BANDINFOSTRUCT
//
/// An ObjectWindows struct, TBandInfo is used to pass information to a printer
/// driver that supports banding. TBandInfo is declared as follows:
/// \code
/// struct TBandInfo { 
/// 	bool HasGraphics;
/// 	bool HasText;
/// 	TRect GraphicsRect;
/// }
/// \endcode
/// HasGraphics is true if graphics are (or are expected to be) on the page or in
/// the band; otherwise, it is false.
/// 
/// HasText is true if text is (or is expected to be) on the page or in the band;
/// otherwise, it is false.
/// 
/// GraphicsRect defines the bounding region for all graphics on the page.
//
struct TBandInfo {
  bool  HasGraphics;
  bool  HasText;
  TRect GraphicsRect;
};

//
/// \class TPrintDC
// ~~~~~ ~~~~~~~~
/// A DC class that provides access to a printer
//
class _OWLCLASS TPrintDC : public TCreatedDC {
  public:
    TPrintDC(HDC handle, TAutoDelete autoDelete = NoAutoDelete);
    TPrintDC(LPCTSTR driver, LPCTSTR device,
             LPCTSTR output, const DEVMODE * initData);
    TPrintDC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* initData);

    int         Escape(int escape, int count=0, const void* inData=0,
                       void* outData=0);
    int         SetAbortProc(ABORTPROC proc);
    int         StartDoc(LPCTSTR docName, LPCTSTR output);
    int StartDoc(const tstring& docName, const tstring& output) {return StartDoc(docName.c_str(), output.c_str());}
    int         StartPage();
    int         BandInfo(TBandInfo& bandInfo);
    int         NextBand(TRect& rect);
    int         EndPage();
    int         EndDoc();
    int         AbortDoc();

    uint        QueryEscSupport(int escapeNum);
    int         SetCopyCount(int reqestCount, int& actualCount);

    static uint32 DeviceCapabilities(LPCTSTR driver,
                                    LPCTSTR device,
                                    LPCTSTR port,
                                    int capability, LPTSTR output=0,
                                    LPDEVMODE devmode=0);

    static uint32 DeviceCapabilities(
      const tstring& driver,
      const tstring& device,
      const tstring& port,
      int capability, 
      LPTSTR output = 0,
      LPDEVMODE devmode = 0);

  protected:
    DOCINFO&    GetDocInfo();
    void        SetDocInfo(const DOCINFO& docinfo);

  protected_data:
/// Holds the input and output file names used by TPrintDC::StartDoc. The DOCINFO
/// structure is defined as follows:
/// \code
/// typedef struct { 
///    int cbSize;          // size of the structure, bytes
///    DocInfo lpszDocName;   // document name <= 32 chars inc. final 0
///    DocInfo lpszOutput;    // output file name
/// } DOCINFO;
/// \endcode
/// The lpszOutput field allows a print job to be redirected to a file. If this
/// field is NULL, the output will go to the device for the specified DC.
    DOCINFO     DocInfo;

  private:
    TPrintDC(const TPrintDC&);
    TPrintDC& operator =(const TPrintDC&);
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

//----------------------------------------------------------------------------
// Inlines for DC classes
//

//
// Get the GDI object headers so that we have access to conversion operators,
// etc.
//
#include <owl/gdiobjec.h>
#if !defined(OWL_METAFILE_H)
# include <owl/metafile.h>
#endif


namespace owl {

//
/// Return the handle of the device context.
//
/// Typecasting operator. Converts a pointer to type HDC (the data type representing
/// the handle to a DC).
//
inline TDC::operator HDC() const
{
  return HDC(Handle);
}

//
/// Return the handle of the device context.
//
inline HDC TDC::GetHDC()const
{
  return HDC(Handle);
}

//
/// Attempt to validate the device context handle and return true if the
/// validation succeeded, or false otherwise.
//
inline bool TDC::IsDC() const
{
  return ::GetObjectType(GetHDC()) != 0;
}

//
/// Obtains the final translation origin for this device context and places the
/// value in point. This value specifies the offset used to translate device
/// coordinates to client coordinates for points in an application window. Returns
/// true if the call is successful; otherwise, returns false.
//
inline bool TDC::GetDCOrg(TPoint& point) const
{
  return ::GetDCOrgEx(GetAttributeHDC(), &point);
}

//
/// Places in point the current brush origin of this DC. Returns true if successful;
/// otherwise, returns false.
//
inline bool TDC::GetBrushOrg(TPoint& point) const
{
  return ::GetBrushOrgEx(GetHDC(), &point);
}

//
/// Sets the origin of the currently selected brush of this DC with the given origin
/// value. The previous origin is passed to oldOrg. Returns true if successful;
/// otherwise, returns false.
//
inline bool TDC::SetBrushOrg(const TPoint& org, TPoint * oldOrg)
{
  return ::SetBrushOrgEx(GetHDC(), org.x, org.y, oldOrg);
}

//
/// Enumerates the pen or brush objects available for this DC. The parameter
/// objectType can be either OBJ_BRUSH or OBJ_PEN. For each pen or brush found,
/// proc, a user-defined callback function, is called until there are no more
/// objects found or the callback function returns 0. proc is defined as:
/// \code
/// 	typedef int (CALLBACK* GOBJENUMPROC)(LPVOID, LPARAM);
/// \endcode
/// Parameter data specifies an application-defined value that is passed to proc.
//
inline int TDC::EnumObjects(uint objectType, GOBJENUMPROC proc, void * data) const
{
  return ::EnumObjects(GetHDC(), objectType, proc, (LPARAM)data);
}

//
/// Returns the color nearest to the given Color argument for the current palette of
/// this DC.
//
inline TColor TDC::GetNearestColor(const TColor& color) const
{
  return ::GetNearestColor(GetAttributeHDC(), color);
}

//
/// Maps to the system palette the logical palette entries selected into this DC.
/// Returns the number of entries in the logical palette that were mapped to the
/// system palette.
//
inline int TDC::RealizePalette()
{
  return ::RealizePalette(GetHDC());
}

//
/// Updates the client area of this DC by matching the current colors in the client
/// area to the system palette on a pixel-by-pixel basis.
//
inline void TDC::UpdateColors()
{
  ::UpdateColors(GetHDC());
}

//
/// Retrieves a range of up to num palette entries, starting at start, from the
/// system palette to the entries array of PALETTEENTRY structures. Returns the
/// actual number of entries transferred.
//
inline uint TDC::GetSystemPaletteEntries(int start, int num, PALETTEENTRY * entries) const
{
  return ::GetSystemPaletteEntries(GetHDC(), start, num, entries);
}

//
/// Determines whether this DC has access to the full system palette. Returns
/// SYSPAL_NOSTATIC or SYSPAL_STATIC.
//
inline uint TDC::GetSystemPaletteUse() const
{
  return ::GetSystemPaletteUse(GetHDC());
}

//
/// Changes the usage of this DC's system palette. The usage argument can be
/// SYSPAL_NOSTATIC or SYSPAL_STATIC. Returns the previous usage value.
//
inline int TDC::SetSystemPaletteUse(int usage)
{
  return ::SetSystemPaletteUse(GetHDC(), usage);
}

//
/// Returns the current background color of this DC.
//
inline TColor TDC::GetBkColor() const
{
  return ::GetBkColor(GetAttributeHDC());
}

//
/// Returns the background mode of this DC, either OPAQUE or TRANSPARENT.
//
inline int TDC::GetBkMode() const
{
  return ::GetBkMode(GetAttributeHDC());
}

//
/// Sets the background mode to the given mode argument, which can be either OPAQUE
/// or TRANSPARENT. Returns the previous background mode.
//
inline int TDC::SetBkMode(int mode)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetBkMode(GetHDC(), mode);
  return ::SetBkMode(GetAttributeHDC(), mode);
}

//
/// Returns the current polygon-filling mode for this DC, either ALTERNATE or
/// WINDING.
//
inline int TDC::GetPolyFillMode() const
{
  return ::GetPolyFillMode(GetAttributeHDC());
}

//
/// Description
/// Sets the polygon-filling mode for this DC to the given mode value, either
/// ALTERNATE or WINDING. Returns the previous fill mode.
//
inline int TDC::SetPolyFillMode(int mode)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetPolyFillMode(GetHDC(), mode);
  return ::SetPolyFillMode(GetAttributeHDC(), mode);
}

//
/// Returns the current drawing (raster operation) mode of this DC.
//
inline int TDC::GetROP2() const
{
  return ::GetROP2(GetAttributeHDC());
}

//
/// Sets the current foreground mix mode mode of this DC to the given mode value and
/// returns the previous mode. The mode argument determines how the brush, pen, and
/// existing screen image combine when filling and drawing. mode can be one of the
/// following values:
/// - \c \b  R2_BLACK	Pixel is always binary 0.
/// - \c \b  R2_COPYPEN	Pixel is the pen color.
/// - \c \b  R2_MASKNOTPEN	Pixel is a combination of the colors common to both the display
/// and the inverse of the pen.
/// - \c \b  R2_MASKPEN	Pixel is a combination of the colors common to both the pen and the
/// display.
/// - \c \b  R2_MASKPENNOT	Pixel is a combination of the colors common to both the pen and
/// the inverse of the display.
/// - \c \b  R2_MERGEPEN	Pixel is a combination of the pen color and the display color.
/// - \c \b  R2_MERGENOTPEN	Pixel is a combination of the display color and the inverse of
/// the pen color.
/// - \c \b  R2_MERGEPENNOT	Pixel is a combination of the pen color and the inverse of the
/// display color.
/// - \c \b  R2_NOP	Pixel remains unchanged.
/// - \c \b  R2_NOT	Pixel is the inverse of the display color.
/// - \c \b  R2_NOTCOPYPEN	Pixel is the inverse of the pen color.
/// - \c \b  R2_NOTMASKPEN	Pixel is the inverse of the R2_MASKPEN color.
/// - \c \b  R2_NOTMERGEPEN	Pixel is the inverse of the R2_MERGEPEN color.
/// - \c \b  R2_NOTXORPEN	Pixel is the inverse of the R2_XORPEN color.
/// - \c \b  R2_WHITE	Pixel is always binary 1.
/// - \c \b  R2_XORPEN	Pixel is a combination of the colors in the pen and in the display,
/// but not in both.
//
inline int TDC::SetROP2(int mode)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetROP2(GetHDC(), mode);
  return ::SetROP2(GetAttributeHDC(), mode);
}

//
/// Returns the current stretching mode for this DC: BLACKONWHITE, COLORONCOLOR, or
/// WHITEONBLACK. The stretching mode determines how bitmaps are stretched or
/// compressed by the StretchBlt function.
//
inline int TDC::GetStretchBltMode() const
{
  return ::GetStretchBltMode(GetAttributeHDC());
}

//
/// Sets the stretching mode of this DC to the given mode value and returns the
/// previous mode. The mode argument (BLACKONWHITE, COLORONCOLOR, or WHITEONBLACK)
/// defines which scan lines or columns or both are eliminated by TDC::StretchBlt.
//
inline int TDC::SetStretchBltMode(int mode)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetStretchBltMode(GetHDC(), mode);
  return ::SetStretchBltMode(GetAttributeHDC(), mode);
}

//
/// Returns the current text color of this DC. The text color determines the color
/// displayed by TDC::TextOut and TDC::ExtTextOut.
//
inline TColor TDC::GetTextColor() const
{
  return ::GetTextColor(GetAttributeHDC());
}

//
/// If successful, returns the current window mapping mode of this DC; otherwise,
/// returns 0. The mapping mode defines how logical coordinates are mapped to device
/// coordinates. It also controls the orientation of the device's x- and y-axes. The
/// mode values are shown in the following table:
/// - \c \b  MM_ANISOTROPIC	Logical units are mapped to arbitrary units with arbitrarily
/// scaled axes. SetWindowExtEx and SetViewportExtEx  must be used to specify the
/// desired units, orientation, and scaling.
/// - \c \b  MM_HIENGLISH	Each logical unit is mapped to 0.001 inch. Positive x is to the
/// right; positive y is at the top.
/// - \c \b  MM_HIMETRIC	Each logical unit is mapped to 0.01 millimeter. Positive x is to the
/// right; positive y is at the top.
/// - \c \b  MM_ISOTROPIC	Logical units are mapped to arbitrary units with equally scaled
/// axes; that is, one unit along the x-axis is equal to one unit along the y-axis.
/// SetWindowExtEx and SetViewportExtEx must be used to specify the desired units
/// and the orientation of the axes. GDI makes adjustments as necessary to ensure
/// that the x and y units remain the same size (e.g., if you set the window extent,
/// the viewport is adjusted to keep the units isotropic).
/// - \c \b  MM_LOENGLISH	Each logical unit is mapped to 0.01 inch. Positive x is to the
/// right; positive y is at the top.
/// - \c \b  MM_LOMETRIC	Each logical unit is mapped to 0.1 millimeter. Positive x is to the
/// right; positive y is at the top.
/// - \c \b  MM_TEXT	Each logical unit is mapped to one device pixel. Positive x is to the
/// right; positive y is at the bottom.
/// - \c \b  MM_TWIPS	Each logical unit is mapped to one twentieth of a printer's point
/// (1/1440 inch). Positive x is to the right; positive y is at the top.
//
inline int TDC::GetMapMode() const
{
  return ::GetMapMode(GetAttributeHDC());
}


//
/// Sets the limit of miter joins to newLimit and puts the previous value in
/// oldLimit. Returns true if successful; otherwise, returns false.
//
inline bool TDC::SetMiterLimit(float newLimit, float* oldLimit)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetMiterLimit(GetHDC(), newLimit, oldLimit);
  return ::SetMiterLimit(GetAttributeHDC(), newLimit, oldLimit);
}

//
/// Sets a two-dimensional linear transformation, given by the xform
/// structure, between world space and page space for this DC. Returns true if the
/// call is successful; otherwise, returns false.
//
inline bool TDC::SetWorldTransform(XFORM & xform)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetWorldTransform(GetHDC(), &xform);
  return ::SetWorldTransform(GetAttributeHDC(), &xform);
}

//
/// Changes the current world transformation for this DC using the given xform and
/// mode arguments. mode determines how the given XFORM structure is applied, as
/// listed below:
/// - \c \b  MWT_IDENTITY	Resets the current world transformation using the identity matrix.
/// If this mode is specified, the XFORM structure pointed to by lpXform is ignored.
/// - \c \b  MWT_LEFTMULTIPLY	Multiplies the current transformation by the data in the XFORM
/// structure. (The data in the XFORM structure becomes the left multiplicand, and
/// the data for the current transformation becomes the right multiplicand.)
/// - \c \b  MWT_RIGHTMULTIPLY	Multiplies the current transformation by the data in the XFORM
/// structure. (The data in the XFORM structure becomes the right multiplicand, and
/// the data for the current transformation becomes the left multiplicand.)
/// ModifyWorldTransform returns true if the call is successful; otherwise, it
/// returns false.
//
inline bool TDC::ModifyWorldTransform(XFORM & xform, uint32 mode)
{
  if (GetHDC() != GetAttributeHDC())
    ::ModifyWorldTransform(GetHDC(), &xform, mode);
  return ::ModifyWorldTransform(GetAttributeHDC(), &xform, mode);
}


//
/// The first version sets in the point argument the x- and y-extents (in
/// device-units) of this DC's viewport. It returns true if the call is successful;
/// otherwise, it returns false. The second version returns the x- and y-extents (in
/// device-units) of this DC's viewport.
//
inline bool TDC::GetViewportOrg(TPoint& point) const
{
  return ::GetViewportOrgEx(GetAttributeHDC(), &point);
}

//
/// The first version sets in the point argument the x- and y-extents (in
/// device-units) of this DC's viewport. It returns true if the call is successful;
/// otherwise, it returns false. The second version returns the x- and y-extents (in
/// device-units) of this DC's viewport.
//
inline TPoint TDC::GetViewportOrg() const
{
  TPoint point;
  ::GetViewportOrgEx(GetAttributeHDC(), &point);
  return point;
}

//
/// The first version retrieves this DC's current viewport's x- and y-extents (in
/// device units) and places the values in extent. This version returns true if the
/// call is successful; otherwise, it returns false. The second version returns only
/// these x- and y-extents.
/// The extent value determines the amount of stretching or compression needed in
/// the logical coordinate system to fit the device coordinate system. extent also
/// determines the relative orientation of the two coordinate systems.
//
inline bool TDC::GetViewportExt(TSize& extent) const
{
  return ::GetViewportExtEx(GetAttributeHDC(), &extent);
}

//
//
/// The first version retrieves this DC's current viewport's x- and y-extents (in
/// device units) and places the values in extent. This version returns true if the
/// call is successful; otherwise, it returns false. The second version returns only
/// these x- and y-extents.
/// The extent value determines the amount of stretching or compression needed in
/// the logical coordinate system to fit the device coordinate system. extent also
/// determines the relative orientation of the two coordinate systems.
//
inline TSize TDC::GetViewportExt() const
{
  TSize extent;
  ::GetViewportExtEx(GetAttributeHDC(), &extent);
  return extent;
}

//
/// Places in point the x- and y-coordinates of the origin of the window associated
/// with this DC. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::GetWindowOrg(TPoint& point) const
{
  return ::GetWindowOrgEx(GetAttributeHDC(), &point);
}

//
/// Places in point the x- and y-coordinates of the origin of the window associated
/// with this DC. Returns true if the call is successful; otherwise, returns false.
//
inline TPoint TDC::GetWindowOrg() const
{
  TPoint point;
  ::GetWindowOrgEx(GetAttributeHDC(), &point);
  return point;
}

//
/// Retrieves this DC's window current x- and y-extents (in device units). The first
/// version places the values in extent and returns true if the call is successful;
/// otherwise, it returns false. The second version returns the current extent
/// values. The extent value determines the amount of stretching or compression
/// needed in the logical coordinate system to fit the device coordinate system.
/// extent also determines the relative orientation of the two coordinate systems.
//
inline bool TDC::GetWindowExt(TSize& extent) const
{
  return ::GetWindowExtEx(GetAttributeHDC(), &extent);
}

//
/// Retrieves this DC's window current x- and y-extents (in device units). The first
/// version places the values in extent and returns true if the call is successful;
/// otherwise, it returns false. The second version returns the current extent
/// values. The extent value determines the amount of stretching or compression
/// needed in the logical coordinate system to fit the device coordinate system.
/// extent also determines the relative orientation of the two coordinate systems.
//
inline TSize TDC::GetWindowExt() const
{
  TSize extent;
  ::GetWindowExtEx(GetAttributeHDC(), &extent);
  return extent;
}

//
/// Converts each of the count points in the points array from device points to
/// logical points. The conversion depends on this DC's current mapping mode and the
/// settings of its window and viewport origins and extents. DPtoLP returns true if
/// the call is successful; otherwise, it returns false.
//
inline bool TDC::DPtoLP(TPoint* points, int count) const
{
  return ::DPtoLP(GetAttributeHDC(), points, count);
}

//
inline bool TDC::DPtoLP(TRect& rect) const
{
  return ::DPtoLP(GetAttributeHDC(), (LPPOINT)&rect, 2);
}

//
/// Converts each of the count points in the points array from logical points to
/// device points. The conversion depends on this DC's current mapping mode and the
/// settings of its window and viewport origins and extents. Returns true if the
/// call is successful; otherwise, it returns false.
//
inline bool TDC::LPtoDP(TPoint* points, int count) const
{
  return ::LPtoDP(GetAttributeHDC(), points, count);
}

//
/// Converts each of the count points in the points array from logical points to
/// device points. The conversion depends on this DC's current mapping mode and the
/// settings of its window and viewport origins and extents. Returns true if the
/// call is successful; otherwise, it returns false.
//
inline bool TDC::LPtoDP(TRect& rect) const
{
  return ::LPtoDP(GetAttributeHDC(), (LPPOINT)&rect, 2);
}

//
/// Places the current clip box size of this DC in rect. The clip box is defined as
/// the smallest rectangle bounding the current clipping boundary. The return value
/// indicates the clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping Region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline int TDC::GetClipBox(TRect& rect) const
{
  return ::GetClipBox(GetHDC(), &rect);
}

//
/// Places the current clip box size of this DC in rect. The clip box is defined as
/// the smallest rectangle bounding the current clipping boundary. The return value
/// indicates the clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping Region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline TRect TDC::GetClipBox() const
{
  TRect rect;
  ::GetClipBox(GetHDC(), &rect);
  return rect;
}

//
/// Creates a new clipping region for this DC. This new region consists of the
/// current clipping region minus the given rectangle, rect. The return value
/// indicates the new clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping Region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline int TDC::ExcludeClipRect(const TRect& rect)
{
  return ::ExcludeClipRect(GetHDC(), rect.left, rect.top, rect.right, rect.bottom);
}

//
/// Prevents drawing within invalid areas of a window by excluding an updated region
/// of this DC's window from its clipping region. The return value indicates the
/// resulting clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping Region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline int TDC::ExcludeUpdateRgn(HWND Wnd)
{
  return ::ExcludeUpdateRgn(GetHDC(), Wnd);
}

//
/// Creates a new clipping region for this DC's window by forming the intersection
/// of the current region with the rectangle specified by rect. The return value
/// indicates the resulting clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping Region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline int TDC::IntersectClipRect(const TRect& rect)
{
  return ::IntersectClipRect(GetHDC(), rect.left, rect.top, rect.right, rect.bottom);
}

//
/// Moves the clipping region of this DC by the x- and y-offsets specified in delta.
/// The return value indicates the resulting clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline int TDC::OffsetClipRgn(const TPoint& delta)
{
  return ::OffsetClipRgn(GetHDC(), delta.x, delta.y);
}

//
/// Selects the given region as the current clipping region for this DC. A copy of
/// the given region is used, letting you select the same region for other DC
/// objects. The return value indicates the new clipping region's type as follows:
/// - \c \b  COMPLEXREGION	Clipping Region has overlapping borders.
/// - \c \b  ERROR	Invalid DC.
/// - \c \b  NULLREGION	Clipping region is empty.
/// - \c \b  SIMPLEREGION	Clipping region has no overlapping borders.
//
inline int TDC::SelectClipRgn(const TRegion& region)
{
  return ::SelectClipRgn(GetHDC(), region);
}

//
/// Returns true if the given point lies within the clipping region of this DC;
/// otherwise, returns false.
//
inline bool TDC::PtVisible(const TPoint& point) const
{
  return ::PtVisible(GetHDC(), point.x, point.y);
}

//
/// Returns true if any part of the given rectangle, rect, lies within the clipping
/// region of this DC; otherwise, returns false.
//
inline bool TDC::RectVisible(const TRect& rect) const
{
  return ::RectVisible(GetHDC(), &rect);
}

//
/// Reports in bounds the current accumulated bounding rectangle of this DC or of
/// the Windows manager, depending on the value of flags. Returns true if the call
/// is successful; otherwise, returns false.
/// The flags argument can be DCB_RESET or DCB_WINDOWMGR or both. The flags value
/// work as follows:
/// - \c \b  DCB_RESET	Forces the bounding rectangle to be cleared after being set in bounds.
/// - \c \b  DCB_WINDOWMGR	Reports the Windows current bounding rectangle rather than that of
/// this DC.
///
/// There are two bounding-rectangle accumulations, one for Windows and one for the
/// application. GetBoundsRect returns screen coordinates for the Windows bounds,
/// and logical units for the application bounds. The Windows accumulated bounds can
/// be queried by an application but not altered. The application can both query and
/// alter the DC's accumulated bounds.
//
inline bool TDC::GetBoundsRect(TRect& bounds, uint16 flags) const
{
  return ::GetBoundsRect(GetHDC(), &bounds, flags);
}

//
/// Controls the accumulation of bounding rectangle information for this DC.
/// Depending on the value of flags, the given bounds rectangle (possibly NULL) can
/// combine with or replace the existing accumulated rectangle. flags can be any
/// appropriate combination of the following values:
/// - \c \b  DCB_ACCUMULATE	Add bounds (rectangular union) to the current accumulated
/// rectangle.
/// - \c \b  DCB_DISABLE	Turn off bounds accumulation.
/// - \c \b  DCB_ENABLE	Turn on bounds accumulation (the default setting for bounds
/// accumulation is disabled).
/// - \c \b  DCB_RESET	Set the bounding rectangle to empty.
/// - \c \b  DCB_SET	Set the bounding rectangle to bounds.
///
/// There are two bounding-rectangle accumulations, one for Windows and one for the
/// application. The Windows-accumulated bounds can be queried by an application but
/// not altered. The application can both query and alter the DC's accumulated
/// bounds.
//
inline uint TDC::SetBoundsRect(TRect& bounds, uint flags)
{
  return ::SetBoundsRect(GetHDC(), &bounds, flags);
}

//
/// Retrieves this DC's current clip-region and, if successful, places a copy of it
/// in the region argument. You can alter this copy without affecting the current
/// clip-region. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::GetClipRgn(TRegion& region) const
{
  return ::GetClipRgn(GetHDC(), (HRGN)region);
}

//
/// Enumerates the GDI calls within the given metaFile. Each such call is retrieved
/// and passed to the given callback function, together with any client data from
/// data, until all calls have been processed or a callback function returns 0.
/// MFENUMPROC is defined as:
/// \code
/// typedef int (CALLBACK* MFENUMPROC)(HDC, HANDLETABLE *, METARECORD *, int,
/// LPARAM);
/// \endcode
//
inline int TDC::EnumMetaFile(const TMetaFilePict& metafile, MFENUMPROC callback,
                             void * data) const
{
  return ::EnumMetaFile(GetHDC(), metafile, callback, (long)data);
}

//
/// Plays the contents of the given metaFile on this DC. The metafile can be played
/// any number of times. Returns true if the call is successful; otherwise, returns
/// false.
//
inline bool TDC::PlayMetaFile(const TMetaFilePict& metafile)
{
  return ::PlayMetaFile(GetHDC(), (TMetaFilePict&)metafile);
}

//
/// Plays the metafile record given in metaRecord to this DC by executing the GDI
/// function call contained in that record. Handletable specifies the object handle
/// table to be used. count specifies the number of handles in the table.
//
inline void TDC::PlayMetaFileRecord(HANDLETABLE & handletable,
                                    METARECORD & metaRecord, int count)
{
  ::PlayMetaFileRecord(GetHDC(), &handletable, &metaRecord, count);
}

//
/// Enumerate through the enhanced metafile records.
//
inline int TDC::EnumEnhMetaFile(const TEnhMetaFilePict& metafile, ENHMFENUMPROC callback,
                                void * data, const TRect* rect) const
{
  return ::EnumEnhMetaFile(GetHDC(), metafile, callback, data, rect);
}

//
/// Play the enhanced metafile onto this device context.
//
inline bool TDC::PlayEnhMetaFile(const TEnhMetaFilePict& metafile, const TRect* rect)
{
  return ::PlayEnhMetaFile(GetHDC(), metafile, rect);
}

//
/// Play one record from the enhanced metafile onto this DC.
//
inline void TDC::PlayEnhMetaFileRecord(HANDLETABLE & handletable,
                                       ENHMETARECORD & metaRecord, uint count)
{
  ::PlayEnhMetaFileRecord(GetHDC(), &handletable, &metaRecord, count);
}

//
/// Reports in point the logical coordinates of this DC's current position. Returns
/// true if the call is successful; otherwise, returns false.
//
inline bool TDC::GetCurrentPosition(TPoint& point) const
{
  return ::GetCurrentPositionEx(GetHDC(), &point);
}

//
/// Moves the current position of this DC to the given x- and y-coordinates or to
/// the given point. The third version sets the previous current position in
/// oldPoint. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::MoveTo(int x, int y)
{
  return ::MoveToEx(GetHDC(), x, y, 0);
}

//
/// Moves the current position of this DC to the given x- and y-coordinates or to
/// the given point. The third version sets the previous current position in
/// oldPoint. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::MoveTo(const TPoint& point)
{
  return ::MoveToEx(GetHDC(), point.x, point.y, 0);
}

//
/// Moves the current position of this DC to the given x- and y-coordinates or to
/// the given point. The third version sets the previous current position in
/// oldPoint. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::MoveTo(const TPoint& point, TPoint& OldPoint)
{
  return ::MoveToEx(GetHDC(), point.x, point.y, &OldPoint);
}

//
/// Draws the given icon on this DC. The upper left corner of the drawn icon can be
/// specified by x- and y-coordinates or by the point argument. DrawIcon returns
/// true if the call is successful; otherwise, it returns false.
//
inline bool TDC::DrawIcon(int x, int y, const TIcon& icon)
{
  return ::DrawIcon(GetHDC(), x, y, icon);
}

//
//
/// Draws the given icon on this DC. The upper left corner of the drawn icon can be
/// specified by x- and y-coordinates or by the point argument. DrawIcon returns
/// true if the call is successful; otherwise, it returns false.
//
inline bool TDC::DrawIcon(const TPoint& point, const TIcon& icon)
{
  return ::DrawIcon(GetHDC(), point.x, point.y, icon);
}

//
inline bool TDC::DrawCursor(int x, int y, const TCursor& cursor)
{
  return ::DrawIcon(GetHDC(), x, y, HICON(HCURSOR(cursor)));
}

//
inline bool TDC::DrawCursor(const TPoint& point, const TCursor& cursor)
{
  return ::DrawIcon(GetHDC(), point.x, point.y, HICON(HCURSOR(cursor)));
}

//
inline bool TDC::DrawIcon(int x, int y, const TIcon& icon, int w, int h,
                          int aniStep, HBRUSH bkBr, uint flags)
{
  return ::DrawIconEx(GetHDC(), x, y, icon, w, h, aniStep, bkBr, flags);
}

inline bool TDC::DrawIcon(const TPoint& point, const TIcon& icon,
                          const TSize& size, int aniStep, HBRUSH bkBr, uint flags)
{
  return ::DrawIconEx(GetHDC(), point.x, point.y, icon, size.cx, size.cy,
                      aniStep, bkBr, flags);
}

inline bool TDC::DrawCursor(int x, int y, const TCursor& cursor, int w, int h,
                            int aniStep, HBRUSH bkBr, uint flags)
{
  return ::DrawIconEx(GetHDC(), x, y, HICON(HCURSOR(cursor)), w, h, aniStep,
                      bkBr, flags);
}

inline bool TDC::DrawCursor(const TPoint& point, const TCursor& cursor,
                            const TSize& size, int aniStep, HBRUSH bkBr, uint flags)
{
  return ::DrawIconEx(GetHDC(), point.x, point.y, HICON(HCURSOR(cursor)),
                      size.cx, size.cy, aniStep, bkBr, flags);
}

//
/// Draws a border on this DC around the given rectangle, rect, using the given
/// brush, brush. The height and width of the border is one logical unit. Returns
/// true if the call is successful; otherwise, it returns false.
//
inline bool TDC::FrameRect(int x1, int y1, int x2, int y2, const TBrush& brush)
{
  TRect r(x1, y1, x2, y2);
  return ::FrameRect(GetHDC(), &r, brush);
}

//
/// Draws a border on this DC around the given rectangle, rect, using the given
/// brush, brush. The height and width of the border is one logical unit. Returns
/// true if the call is successful; otherwise, it returns false.
//
inline bool TDC::FrameRect(const TRect& rect, const TBrush& brush)
{
  return ::FrameRect(GetHDC(), &rect, brush);
}

//
/// Fills the given rectangle on this DC using the specified brush. The fill covers
/// the left and top borders but excludes the right and bottom borders. FillRect
/// returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::FillRect(int x1, int y1, int x2, int y2, const TBrush& brush)
{
  TRect r(x1, y1, x2, y2);
  return ::FillRect(GetHDC(), &r, brush);
}

//
/// Fills the given rectangle on this DC using the specified brush. The fill covers
/// the left and top borders but excludes the right and bottom borders. FillRect
/// returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::FillRect(const TRect& rect, const TBrush& brush)
{
  return ::FillRect(GetHDC(), &rect, brush);
}

//
/// Inverts the given rectangle, rect, on this DC. On monochrome displays,
/// black-and-white pixels are interchanged. On color displays, inversion depends on
/// how the colors are generated for particular displays. Calling InvertRect an even
/// number of times restores the original colors. InvertRect returns true if the
/// call is successful; otherwise, it returns false.
///
/// \note Rect must be normalized for inverRect
//
inline bool TDC::InvertRect(int x1, int y1, int x2, int y2)
{
  TRect r(x1, y1, x2, y2);
  ::InvertRect(GetHDC(), &r);
  return true;
}

//
/// Inverts the given rectangle, rect, on this DC. On monochrome displays,
/// black-and-white pixels are interchanged. On color displays, inversion depends on
/// how the colors are generated for particular displays. Calling InvertRect an even
/// number of times restores the original colors. InvertRect returns true if the
/// call is successful; otherwise, it returns false.
//
inline bool TDC::InvertRect(const TRect& rect)
{
  ::InvertRect(GetHDC(), &rect);
  return true;
}

//
/// Draws the given rectangle on this DC in the style used to indicate focus.
/// Calling the function a second time with the same rect argument will remove the
/// rectangle from the display. A rectangle drawn with DrawFocusRect cannot be
/// scrolled. DrawFocusRect returns true if the call is successful; otherwise, it
/// returns false.
//
inline bool TDC::DrawFocusRect(int x1, int y1, int x2, int y2)
{
  TRect r(x1, y1, x2, y2);
  ::DrawFocusRect(GetHDC(), &r);
  return true;
}

//
/// Draws the given rectangle on this DC in the style used to indicate focus.
/// Calling the function a second time with the same rect argument will remove the
/// rectangle from the display. A rectangle drawn with DrawFocusRect cannot be
/// scrolled. DrawFocusRect returns true if the call is successful; otherwise, it
/// returns false.
//
inline bool TDC::DrawFocusRect(const TRect& rect)
{
  ::DrawFocusRect(GetHDC(), &rect);
  return true;
}

//
/// Fills the given rectangle, clipping any text to the rectangle. 
/// Returns true if the call is successful.
//
inline bool TDC::TextRect(int x1, int y1, int x2, int y2)
{
  TRect r(x1, y1, x2, y2);
  return ::ExtTextOut(GetHDC(), 0, 0, ETO_OPAQUE, &r, 0, 0, 0);
}

//
/// Fills the given rectangle, clipping any text to the rectangle. 
/// Returns true if the call is successful.
//
inline bool TDC::TextRect(const TRect& rect)
{
  return ::ExtTextOut(GetHDC(), 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
}

//
/// Fills the given rectangle, clipping any text to the rectangle. 
/// The given color is used to set to the background color before filling.
/// Returns true if the call is successful.
//
inline bool TDC::TextRect(int x1, int y1, int x2, int y2, const TColor& color)
{
  SetBkColor(color);
  TRect r(x1, y1, x2, y2);
  return ::ExtTextOut(GetHDC(), 0, 0, ETO_OPAQUE, &r, 0, 0, 0);
}

//
/// Fills the given rectangle, clipping any text to the rectangle. If no color
/// argument is supplied, the current backgound color is used. If a color argument
/// is supplied, that color is set to the current background color which is then
/// used for filling. TextRect returns true if the call is successful; otherwise, it
/// returns false.
//
inline bool TDC::TextRect(const TRect& rect, const TColor& color)
{
  SetBkColor(color);
  return ::ExtTextOut(GetHDC(), 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
}

//
/// Draws a border on this DC around the given region, region, using the given
/// brush, brush. The width and height of the border is specified by the p argument.
/// Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::FrameRgn(const TRegion& region, const TBrush& brush, const TPoint& p)
{
  return ::FrameRgn(GetHDC(), region, brush, p.x, p.y);
}

//
/// Fills the given region on this DC using the specified brush. FillRgn returns
/// true if the call is successful; otherwise, it returns false.
//
inline bool TDC::FillRgn(const TRegion& region, const TBrush& brush)
{
  return ::FillRgn(GetHDC(), region, brush);
}

//
/// Inverts the given region, on this DC. On monochrome displays, black-and-white
/// pixels are interchanged. On color displays, inversion depends on how the colors
/// are generated for particular displays. Calling InvertRegion an even number (n>=2
/// ) of times restores the original colors. Returns true if the call is successful;
/// otherwise, it returns false.
//
inline bool TDC::InvertRgn(const TRegion& region)
{
  return ::InvertRgn(GetHDC(), region);
}

//
/// Paints the given rectangle using the currently selected brush for this DC. The
/// rectangle can be specified by its upper left coordinates (x, y), width w, and
/// height h, or by a single TRect argument. The raster-operation code, rop,
/// determines how the brush and surface color(s) are combined, as explained in the
/// following table:
/// - \c \b  PATCOPY	Copies pattern to destination bitmap.
/// - \c \b  PATINVERT	Combines destination bitmap with pattern using the Boolean OR
/// operator.
/// - \c \b  DSTINVERT	Inverts the destination bitmap.
/// - \c \b  BLACKNESS	Turns all output to binary 0s.
/// - \c \b  WHITENESS	Turns all output to binary 1s.
///
/// The allowed values of rop for this function are a limited subset of the full 256
/// ternary raster-operation codes; in particular, an operation code that refers to
/// a source cannot be used with PatBlt.
/// Not all devices support the PatBlt function, so applications should call
/// TDC::GetDeviceCaps to check the features supported by this DC.
/// PatBlt returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::PaintRgn(const TRegion& region)
{
  return ::PaintRgn(GetHDC(), region);
}

//
/// Draws a line segment and an arc on this DC using the currently
/// selected pen object. The line is drawn from the current position to the
/// beginning of the arc. The arc is that part of the circle (with the center at
/// logical coordinates (x, y) and positive radius, radius) starting at startAngle
/// and ending at (startAngle + sweepAngle). Both angles are measured in degrees,
/// counterclockwise from the x-axis (the default arc direction). The arc might
/// appear to be elliptical, depending on the current transformation and mapping
/// mode. AngleArc returns true if the figure is drawn successfully; otherwise, it
/// returns false. If successful, the current position is moved to the end point of
/// the arc.
//
inline bool TDC::AngleArc(int x, int y, uint32 radius, float startAngle, float sweepAngle)
{
  return ::AngleArc(GetHDC(), x, y, radius, startAngle, sweepAngle);
}

//
/// Draws a line segment and an arc on this DC using the currently
/// selected pen object. The line is drawn from the current position to the
/// beginning of the arc. The arc is that part of the circle (with the center at
/// logical coordinates (x, y) and positive radius, radius) starting at startAngle
/// and ending at (startAngle + sweepAngle). Both angles are measured in degrees,
/// counterclockwise from the x-axis (the default arc direction). The arc might
/// appear to be elliptical, depending on the current transformation and mapping
/// mode. AngleArc returns true if the figure is drawn successfully; otherwise, it
/// returns false. If successful, the current position is moved to the end point of
/// the arc.
//
inline bool TDC::AngleArc(const TPoint& Center, uint32 radius, float StartAngle, float SweepAngle)
{
  return ::AngleArc(GetHDC(), Center.x, Center.y, radius, StartAngle, SweepAngle);
}

//
/// Draws an elliptical arc on this DC using the currently selected pen object. The
/// center of the arc is the center of the bounding rectangle, specified either by
/// (x1, y1)/(x2, y2) or by the rectangle r. The starting/ending points of the arc
/// are specified either by (x3, y3)/(x4, y4) or by the points start and end. All
/// points are specified in logical coordinates. Arc returns true if the arc is
/// drawn successfully; otherwise, it returns false. The current position is neither
/// used nor altered by this call. The drawing direction default is
/// counterclockwise.
//
inline bool TDC::Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  return ::Arc(GetHDC(), x1, y1, x2, y2, x3, y3, x4, y4);
}

//
/// Draws an elliptical arc on this DC using the currently selected pen object. The
/// center of the arc is the center of the bounding rectangle, specified either by
/// (x1, y1)/(x2, y2) or by the rectangle r. The starting/ending points of the arc
/// are specified either by (x3, y3)/(x4, y4) or by the points start and end. All
/// points are specified in logical coordinates. Arc returns true if the arc is
/// drawn successfully; otherwise, it returns false. The current position is neither
/// used nor altered by this call. The drawing direction default is
/// counterclockwise.
//
inline bool TDC::Arc(const TRect& rect, const TPoint& start, const TPoint& end)
{
  return ::Arc(GetHDC(), rect.left, rect.top, rect.right, rect.bottom, start.x, start.y, end.x, end.y);
}

//
/// Draws and fills a chord (a region bounded by the intersection of an ellipse and
/// a line segment) on this DC using the currently selected pen and brush objects.
/// The ellipse is specified by a bounding rectangle given either by (x1, y1)/(x2,
/// y2) or by the rectangle R. The starting/ending points of the chord are specified
/// either by (x3, y3)/(x4, y4) or by the points Start and End. Chord returns true
/// if the call is successful; otherwise, it returns false. The current position is
/// neither used nor altered by this call.
//
inline bool TDC::Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  return ::Chord(GetHDC(), x1, y1, x2, y2, x3, y3, x4, y4);
}

//
/// Draws and fills a chord (a region bounded by the intersection of an ellipse and
/// a line segment) on this DC using the currently selected pen and brush objects.
/// The ellipse is specified by a bounding rectangle given either by (x1, y1)/(x2,
/// y2) or by the rectangle R. The starting/ending points of the chord are specified
/// either by (x3, y3)/(x4, y4) or by the points Start and End. Chord returns true
/// if the call is successful; otherwise, it returns false. The current position is
/// neither used nor altered by this call.
//
inline bool TDC::Chord(const TRect& rect, const TPoint& start, const TPoint& end)
{
  return ::Chord(GetHDC(), rect.left, rect.top, rect.right, rect.bottom, start.x, start.y, end.x, end.y);
}

//
/// Using the currently selected pen and brush objects, draws and fills a pie-shaped
/// wedge by drawing an elliptical arc whose center and end points are joined by
/// lines. The center of the ellipse is the center of the rectangle specified either
/// by (x1, y1)/(x2, y2) or by the rect argument. The starting/ending points of pie
/// are specified either by (x3, y3)/(x4, y4) or by the points Start and End.
/// Returns true if the call is successful; otherwise, returns false. The current
/// position is neither used nor altered by this call.
//
inline bool TDC::Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  return ::Pie(GetHDC(), x1, y1, x2, y2, x3, y3, x4, y4);
}

//
/// Using the currently selected pen and brush objects, draws and fills a pie-shaped
/// wedge by drawing an elliptical arc whose center and end points are joined by
/// lines. The center of the ellipse is the center of the rectangle specified either
/// by (x1, y1)/(x2, y2) or by the rect argument. The starting/ending points of pie
/// are specified either by (x3, y3)/(x4, y4) or by the points Start and End.
/// Returns true if the call is successful; otherwise, returns false. The current
/// position is neither used nor altered by this call.
//
inline bool TDC::Pie(const TRect& rect, const TPoint& start, const TPoint& end)
{
  return ::Pie(GetHDC(), rect.left, rect.top, rect.right, rect.bottom, start.x, start.y, end.x, end.y);
}

//
/// Draws and fills an ellipse on this DC using the currently selected pen and brush
/// objects. The center of the ellipse is the center of the bounding rectangle
/// specified either by (x1, y1)/(x2, y2) or by the rect argument. Ellipse returns
/// true if the call is successful; otherwise, it returns false. The current
/// position is neither used nor altered by this call.
//
inline bool TDC::Ellipse(int x1, int y1, int x2, int y2)
{
  return ::Ellipse(GetHDC(), x1, y1, x2, y2);
}

//
/// Draws and fills an ellipse on this DC using the currently selected pen and brush
/// objects. The center of the ellipse is the center of the bounding rectangle
/// specified either by (x1, y1)/(x2, y2) or by the rect argument. Ellipse returns
/// true if the call is successful; otherwise, it returns false. The current
/// position is neither used nor altered by this call.
//
inline bool TDC::Ellipse(const TPoint& p1, const TPoint& p2)
{
  return ::Ellipse(GetHDC(), p1.x, p1.y, p2.x, p2.y);
}

//
/// Draws and fills an ellipse on this DC using the currently selected pen and brush
/// objects. The center of the ellipse is the center of the bounding rectangle
/// specified either by (x1, y1)/(x2, y2) or by the rect argument. Ellipse returns
/// true if the call is successful; otherwise, it returns false. The current
/// position is neither used nor altered by this call.
//
inline bool TDC::Ellipse(const TPoint& p, const TSize& s)
{
  return ::Ellipse(GetHDC(), p.x, p.y, p.x+s.cx, p.y+s.cy);
}

//
/// Draws and fills an ellipse on this DC using the currently selected pen and brush
/// objects. The center of the ellipse is the center of the bounding rectangle
/// specified either by (x1, y1)/(x2, y2) or by the rect argument. Ellipse returns
/// true if the call is successful; otherwise, it returns false. The current
/// position is neither used nor altered by this call.
//
inline bool TDC::Ellipse(const TRect& rect)
{
  return ::Ellipse(GetHDC(), rect.left, rect.top, rect.right, rect.bottom);
}

//
/// Draws a line on this DC using the current pen object. The line is drawn from the
/// current position up to, but not including, the given end point, which is
/// specified by (x, y) or by point. If the call is successful, LineTo returns true
/// and the current point is reset to point; otherwise, it returns false.
//
inline bool TDC::LineTo(int x, int y)
{
  return ::LineTo(GetHDC(), x, y);
}

//
/// Draws a line on this DC using the current pen object. The line is drawn from the
/// current position up to, but not including, the given end point, which is
/// specified by (x, y) or by point. If the call is successful, LineTo returns true
/// and the current point is reset to point; otherwise, it returns false.
//
inline bool TDC::LineTo(const TPoint& point)
{
  return ::LineTo(GetHDC(), point.x, point.y);
}

//
/// Using the current pen object, draws on this DC a count of line segments (there
/// must be at least 2). The points array specifies the sequence of points to be
/// connected. The current position is neither used nor altered by Polyline. Returns
/// true if the call is successful; otherwise, returns false.
//
inline bool TDC::Polyline(const TPoint* points, int count)
{
  return ::Polyline(GetHDC(), points, count);
}

//
inline bool TDC::Polygon(const TPoint* points, int count)
{
  return ::Polygon(GetHDC(), (TPoint*)points, count);
}

//
/// Using the current pen and polygon-filling mode, draws and fills on this DC the
/// number of closed polygons indicated in count (which must be >= 2). The polygons
/// can overlap. The points array specifies the vertices of the polygons to be
/// drawn. PolyCounts is an array of count integers specifying the number of
/// vertices in each polygon. Each polygon must be a closed polygon. The current
/// position is neither used nor altered by Polygon. Returns true if the call is
/// successful; otherwise, returns false.
//
inline bool TDC::PolyPolygon(const TPoint* points, const int* polyCounts, int count)
{
  return ::PolyPolygon(GetHDC(), (TPoint*)points, (int*)polyCounts, count); // API typecast
}

//
/// Using the currently selected pen object, draws on this DC the number of
/// polylines (connected line segments) indicated in count. The resulting figures
/// are not filled. The PolyCounts array provides count integers specifying the
/// number of points (>= 2) in each polyline. The points array provides,
/// consecutively, each of the points to be connected. Returns true if the call is
/// successful; otherwise, returns false. The current position is neither used nor
/// altered by this call.
//
inline bool TDC::PolyPolyline(const TPoint* points, const int* polyCounts, int count)
{
  return ::PolyPolyline(GetHDC(), (TPoint*)points, (const DWORD*)polyCounts, count); // API typecast
}

//
/// Draws one or more connected cubic Bezier splines through the points specified in
/// the points array using the currently selected pen object. The first spline is
/// drawn from the first to the fourth point of the array using the second and third
/// points as controls. Subsequent splines, if any, each require three additional
/// points in the array, since the previous end point is taken as the next spline's
/// start point. The count argument ( >= 4 ) specifies the total number of points
/// needed to specify the complete drawing. To draw n splines, count must be set to
/// (3n + 1). Returns true if the call is successful; otherwise, returns false. The
/// current position is neither used nor altered by this call. The resulting figure
/// is not filled.
//
inline bool TDC::PolyBezier(const TPoint* points, int count)
{
  return ::PolyBezier(GetHDC(), (TPoint*)points, count);
}

//
/// Draws one or more connected cubic Beziers splines through the points specified
/// in the points array using the currently selected pen object. The first spline is
/// drawn from the current position to the third point of the array using the first
/// and second points as controls. Subsequent splines, if any, each require three
/// additional points in the array, since the previous end point is taken as the
/// next spline's start point. The count argument ( >= 4 ) specifies the total
/// number of points needed to specify the complete drawing. To draw n splines,
/// count must be set to 3n. Returns true if the call is successful; otherwise,
/// returns false. The current position is moved to the end point of the final
/// Bezier curve. The resulting figure is not filled.
//
inline bool TDC::PolyBezierTo(const TPoint* points, int count)
{
  return ::PolyBezierTo(GetHDC(), (TPoint*)points, count);
}

//
/// Using the currently selected pen object, draws one or more possibly disjoint
/// sets of line segments or Bezier splines or both on this DC. The count points in
/// the points array provide the end points for each line segment or the end points
/// and control points for each Bezier spline or both. The count BYTEs in the types
/// array determine as follows how the corresponding point in points is to be
/// interpreted:
/// - \c \b  PT_BEZIERTO	This point is a control or end point for a Bezier spline.
/// PT_BEZIERTO types must appear in sets of three: the current position is the
/// Bezier start point; the first two PT_BEZIERTO points are the Bezier control
/// points; and the third PT_BEZIERTO point is the Bezier end point, which becomes
/// the new current point. An error occurs if the PT_BEZIERTO types do not appear in
/// sets of three. An end-point PT_BEZIERTO can be bit-wise OR'd with PT_CLOSEFIGURE
/// to indicate that the current figure is to be closed by drawing a spline from
/// this end point to the start point of the most recent disjoint figure.
/// - \c \b  PT_CLOSEFIGURE	Optional flag that can be bit-wise OR'd with PT_LINETO or
/// PT_BEZIERTO, as explained above. Closure updates the current point to the new
/// end point.
/// - \c \b  PT_LINETO	A line is drawn from the current position to this point, which then
/// becomes the new current point. PT_LINETO can be bit-wise OR'd with
/// PT_CLOSEFIGURE to indicate that the current figure is to be closed by drawing a
/// line segment from this point to the start point of the most recent disjoint
/// figure.
/// - \c \b  PT_MOVETO	This point starts a new (disjoint) figure and becomes the new current
/// point.
///
/// PolyDraw is an alternative to consecutive calls to MoveTo, LineTo, Polyline,
/// PolyBezier, and PolyBezierTo. If there is an active path invoked via BeginPath,
/// PolyDraw will add to this path.
/// Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::PolyDraw(const TPoint* points, uint8* types, int count)
{
  return ::PolyDraw(GetHDC(), (TPoint*)points, types, count);
}

//
/// Draws one or more connected line segments on this DC using the currently
/// selected pen object. The first line is drawn from the current position to the
/// first of the count points in the points array. Subsequent lines, if any, connect
/// the remaining points in the array, with each end point providing the start point
/// of the next segment. The final end point becomes the new current point. No
/// filling occurs even if a closed figure is drawn. Returns true if the call is
/// successful; otherwise, returns false.
//
inline bool TDC::PolylineTo(const TPoint* points, int count)
{
  return ::PolylineTo(GetHDC(), (TPoint*)points, count);
}

//
/// Draws and fills a rectangle of the given size on this DC with the current pen
/// and brush objects. The current position is neither used nor altered by this
/// call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::Rectangle(int x1, int y1, int x2, int y2)
{
  return ::Rectangle(GetHDC(), x1, y1, x2, y2);
}

//
/// Draws and fills a rectangle of the given size on this DC with the current pen
/// and brush objects. The current position is neither used nor altered by this
/// call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::Rectangle(const TPoint& p1, const TPoint& p2)
{
  return ::Rectangle(GetHDC(), p1.x, p1.y, p2.x, p2.y);
}

//
/// Draws and fills a rectangle of the given size on this DC with the current pen
/// and brush objects. The current position is neither used nor altered by this
/// call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::Rectangle(const TPoint& p, const TSize& s)
{
  return ::Rectangle(GetHDC(), p.x, p.y, p.x+s.cx, p.y+s.cy);
}

//
/// Draws and fills a rectangle of the given size on this DC with the current pen
/// and brush objects. The current position is neither used nor altered by this
/// call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::Rectangle(const TRect& rect)
{
  return ::Rectangle(GetHDC(), rect.left, rect.top, rect.right, rect.bottom);
}

//
/// Draws and fills a rounded rectangle of the given size on this DC with the
/// current pen and brush objects. The current position is neither used nor altered
/// by this call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::RoundRect(int x1, int y1, int x2, int y2, int x3, int y3)
{
  return ::RoundRect(GetHDC(), x1, y1, x2, y2, x3, y3);
}

//
/// Draws and fills a rounded rectangle of the given size on this DC with the
/// current pen and brush objects. The current position is neither used nor altered
/// by this call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::RoundRect(const TPoint& p1, const TPoint& p2, const TPoint& rad)
{
  return ::RoundRect(GetHDC(), p1.x, p1.y, p2.x, p2.y, rad.x, rad.y);
}

//
/// Draws and fills a rounded rectangle of the given size on this DC with the
/// current pen and brush objects. The current position is neither used nor altered
/// by this call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::RoundRect(const TPoint& p, const TSize& s, const TPoint& rad)
{
  return ::RoundRect(GetHDC(), p.x, p.y, p.x+s.cx, p.y+s.cy, rad.x, rad.y);
}

//
/// Draws and fills a rounded rectangle of the given size on this DC with the
/// current pen and brush objects. The current position is neither used nor altered
/// by this call. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::RoundRect(const TRect& rect, const TPoint& rad)
{
  return ::RoundRect(GetHDC(), rect.left, rect.top, rect.right, rect.bottom,
                     rad.x, rad.y);
}

//
/// Returns the color of the pixel at the given location.
//
inline TColor TDC::GetPixel(int x, int y) const
{
  return ::GetPixel(GetHDC(), x, y);
}

//
/// Returns the color of the pixel at the given location.
//
inline TColor TDC::GetPixel(const TPoint& point) const
{
  return ::GetPixel(GetHDC(), point.x, point.y);
}

//
/// Sets the color of the pixel at the given location to the given color and returns
/// the pixel's previous color.
//
inline TColor TDC::SetPixel(int x, int y, const TColor& color)
{
  return ::SetPixel(GetHDC(), x, y, color);
}

//
/// Sets the color of the pixel at the given location to the given color and returns
/// the pixel's previous color.
//
inline TColor TDC::SetPixel(const TPoint& point, const TColor& color)
{
  return ::SetPixel(GetHDC(), point.x, point.y, color);
}

//
/// Performs a bit-block transfer from srcDc (the given source DC) to this DC (the
/// destination DC). Color bits are copied from a source rectangle to a destination
/// rectangle. The location of the source rectangle is specified either by its upper
/// left-corner logical coordinates (srcX, srcY), or by the TPoint object, src. The
/// destination rectangle can be specified either by its upper left-corner logical
/// coordinates (dstX, dstY), width w, and height h, or by the TRect object, dst.
/// The destination rectangle has the same width and height as the source. The rop
/// argument specifies the raster operation used to combine the color data for each
/// pixel. See TDC::MaskBlt for a detailed list of rop codes.
/// When recording an enhanced metafile, an error occurs if the source DC identifies
/// the enhanced metafile DC.
//
inline bool TDC::BitBlt(int dstX, int dstY, int w, int h, const TDC& srcDC,
                        int srcX, int srcY, uint32 rop)
{
  return ::BitBlt(GetHDC(), dstX, dstY, w, h, srcDC, srcX, srcY, rop);
}

//
/// Performs a bit-block transfer from srcDc (the given source DC) to this DC (the
/// destination DC). Color bits are copied from a source rectangle to a destination
/// rectangle. The location of the source rectangle is specified either by its upper
/// left-corner logical coordinates (srcX, srcY), or by the TPoint object, src. The
/// destination rectangle can be specified either by its upper left-corner logical
/// coordinates (dstX, dstY), width w, and height h, or by the TRect object, dst.
/// The destination rectangle has the same width and height as the source. The rop
/// argument specifies the raster operation used to combine the color data for each
/// pixel. See TDC::MaskBlt for a detailed list of rop codes.
/// When recording an enhanced metafile, an error occurs if the source DC identifies
/// the enhanced metafile DC.
//
inline bool TDC::BitBlt(const TRect& dst, const TDC& srcDC, const TPoint& src,
                        uint32 rop)
{
  return ::BitBlt(GetHDC(), dst.left, dst.top, dst.Width(), dst.Height(),
                  srcDC, src.x, src.y, rop);
}

//
inline bool TDC::PatBlt(int x, int y, int w, int h, uint32 rop)
{
  return ::PatBlt(GetHDC(), x, y, w, h, rop);
}

//
inline bool TDC::PatBlt(const TRect& dst, uint32 rop)
{
  return ::PatBlt(GetHDC(), dst.left, dst.top, dst.Width(), dst.Height(), rop);
}

//
/// Scrolls a rectangle of bits horizontally by x (or delta.x in the second version)
/// device-units, and vertically by y (or delta.y) device-units on this DC. The
/// scrolling and clipping rectangles are specified by scroll and clip. ScrollDC
/// provides data in the updateRgn argument telling you the region (not necessarily
/// rectangular) that was uncovered by the scroll. Similarly, ScrollDC reports in
/// updateRect the rectangle (in client coordinates) that bounds the scrolling
/// update region. This is the largest area that requires repainting.
/// Returns true if the call is successful; otherwise, returns false.
// 
inline bool TDC::ScrollDC(int dx, int dy, const TRect& scroll, const TRect& clip,
                          TRegion& updateRgn, TRect& updateRect)
{
  return ::ScrollDC(GetHDC(), dx, dy, &scroll, &clip, updateRgn, &updateRect);
}

//
/// Scrolls a rectangle of bits horizontally by x (or delta.x in the second version)
/// device-units, and vertically by y (or delta.y) device-units on this DC. The
/// scrolling and clipping rectangles are specified by scroll and clip. ScrollDC
/// provides data in the updateRgn argument telling you the region (not necessarily
/// rectangular) that was uncovered by the scroll. Similarly, ScrollDC reports in
/// updateRect the rectangle (in client coordinates) that bounds the scrolling
/// update region. This is the largest area that requires repainting.
/// Returns true if the call is successful; otherwise, returns false.
///
inline bool TDC::ScrollDC(const TPoint& delta, const TRect& scroll, const TRect& clip,
                          TRegion& updateRgn, TRect& updateRect)
{
  return ::ScrollDC(GetHDC(), delta.x, delta.y, &scroll, &clip, updateRgn, &updateRect);
}

//
/// Copies a bitmap from the source DC to a destination rectangle on this DC
/// specified either by upper left-corner coordinates (dstX, dstY), width dstW, and
/// height dstH, or (in the second version) by a TRect object, dst. The source
/// bitmap is similarly specified with (srcX, srcY), srcW, and srcH, or by a TRect
/// object, src. StretchBlt stretches or compresses the source according to the
/// stretching mode currently set in this DC (the destination DC). The
/// raster-operation code, rop, specifies how the colors are combined in output
/// operations that involve a brush, a source bitmap, and a destination bitmap. The
/// rop codes are described in the entry for TDC::MaskBlt
//
inline bool TDC::StretchBlt(int dstX, int dstY, int dstW, int dstH,
                           const TDC& srcDC, int srcX, int srcY, int srcW,
                           int srcH, uint32 rop)
{
  return ::StretchBlt(GetHDC(), dstX, dstY, dstW, dstH,
                      srcDC, srcX, srcY, srcW, srcH, rop);
}

//
/// Copies a bitmap from the source DC to a destination rectangle on this DC
/// specified either by upper left-corner coordinates (dstX, dstY), width dstW, and
/// height dstH, or (in the second version) by a TRect object, dst. The source
/// bitmap is similarly specified with (srcX, srcY), srcW, and srcH, or by a TRect
/// object, src. StretchBlt stretches or compresses the source according to the
/// stretching mode currently set in this DC (the destination DC). The
/// raster-operation code, rop, specifies how the colors are combined in output
/// operations that involve a brush, a source bitmap, and a destination bitmap. The
/// rop codes are described in the entry for TDC::MaskBlt
//
inline bool TDC::StretchBlt(const TRect& dst, const TDC& srcDC,
                            const TRect& src, uint32 rop)
{
  return ::StretchBlt(GetHDC(),
                      dst.left, dst.top, dst.Width(), dst.Height(),
                      srcDC, src.left, src.top,
                      src.Width(), src.Height(), rop);
}

//
/// Copies a bitmap from the given source DC to this DC. MaskBlt combines the color
/// data from source and destination bitmaps using the given mask and raster
/// operation. The srcDC argument specifies the DC from which the source bitmap will
/// be copied. The destination bitmap is given by the rectangle, dst. The source
/// bitmap has the same width and height as dst. The src argument specifies the
/// logical coordinates of the upper left corner of the source bitmap. The maskBm
/// argument specifies a monochrome mask bitmap. An error will occur if maskBm is
/// not monochrome. The maskPos argument gives the upper left corner coordinates of
/// the mask. The raster-operation code, rop, specifies how the source, mask, and
/// destination bitmaps combine to produce the new destination bitmap. The
/// raster-operation codes are as follows:
/// - \c \b  BLACKNESS	Fill dst with index-0 color of physical palette (default is black).
/// - \c \b  DSTINVERT	Invert dst.
/// - \c \b  MERGECOPY	Merge the colors of source with mask with Boolean AND.
/// - \c \b  MERGEPAINT	Merge the colors of inverted-source with the colors of dst using
/// Boolean OR.
/// - \c \b  NOTSRCCOPY	Copy inverted-source to dst.
/// - \c \b  NOTSRCERASE	Combine the colors of source and dst using Boolean OR, then invert
/// result.
/// - \c \b  PATCOPY	Copy mask to dst.
/// - \c \b  PATINVERT	Combine the colors of mask with the colors of dst using Boolean XOR.
/// - \c \b  PATPAINT	Combine the colors of mask with the colors of inverted-source using
/// Boolean OR, then combine the result with the colors of dst using Boolean OR.
/// - \c \b  SRCAND	Combine the colors of source and dst using the Boolean AND.
/// - \c \b  SRCCOPY	Copy source directly to dst.
/// - \c \b  SRCERASE	Combine the inverted colors of dst with the colors of source using
/// Boolean AND.
/// - \c \b  SRCPAINT	Combine the colors of source and dst using Boolean OR.
/// - \c \b  WHITENESS	Fill dst with index-1 color of physical palette (default is white).
///
/// If rop indicates an operation that excludes the source bitmap, the srcDC
/// argument must be 0. A value of 1 in the mask indicates that the destination and
/// source pixel colors should be combined using the high-order word of rop. A value
/// of 0 in the mask indicates that the destination and source pixel colors should
/// be combined using the low-order word of rop. If the mask rectangle is smaller
/// than dst, the mask pattern will be suitably duplicated.
/// When recording an enhanced metafile, an error occurs if the source DC identifies
/// the enhanced metafile DC.
/// If a rotation or shear transformation is in effect for the source DC when
/// MaskBlt is called, an error occurs. Other transformations are allowed. If
/// necessary, MaskBlt will adjust the destination and mask color formats to match
/// that of the source bitmaps. Before using MaskBlt, an application should call
/// GetDeviceCaps to determine if the source and destination DCs support MaskBlt.
/// MaskBlt returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::MaskBlt(const TRect& dst,
                         const TDC& srcDC, const TPoint& src,
                         const TBitmap& maskBm, const TPoint& maskPos,
                         uint32 rop)
{
  return ::MaskBlt(GetHDC(), dst.left, dst.top, dst.Width(), dst.Height(),
                   srcDC, src.x, src.y, maskBm, maskPos.x, maskPos.y,
                   rop);
}

//
/// Performs a bit-block transfer from the given source DC to this DC.
/// Color bits are copied from the src rectangle on srcDC, the source DC, to the
/// parallelogram dst on this DC. The dst array specifies three points A,B, and C as
/// the corners of the destination parallelogram. The fourth point D is generated
/// internally from the vector equation D = B + C - A. The upper left corner of src
/// is mapped to A, the upper right corner to B, the lower left corner to C, and the
/// lower right corner to D. An optional monochrome bitmap can be specified by the
/// maskBm argument. (If maskBm specifies a valid monochrome bitmap, PlgBlt uses it
/// to mask the colorbits in the source rectangle. An error occurs if maskBm is not
/// a monochrome bitmap.) maskPos specifies the upper left corner coordinates of the
/// mask bitmap. With a valid maskBm, a value of 1 in the mask causes the source
/// color pixel to be copied to dst; a value of 0 in the mask indicates that the
/// corresponding color pixel in dst will not be changed. If the mask rectangle is
/// smaller than dst, the mask pattern will be suitably duplicated.
/// The destination coordinates are transformed according to this DC (the
/// destination DC). The source coordinates are transformed according to the source
/// DC. If a rotation or shear transformation is in effect for the source DC when
/// PlgBlt is called, an error occurs. Other transformations, such as scaling,
/// translation, and reflection are allowed. The stretching mode of this DC (the
/// destination DC) determines how PlgBlt will stretch or compress the pixels if
/// necessary. When recording an enhanced metafile, an error occurs if the source DC
/// identifies the enhanced metafile DC.
/// If necessary, PlgBlt adjusts the source color formats to match that of the
/// destination. An error occurs if the source and destination DCs are incompatible.
/// Before using PlgBlt, an application should call  GetDeviceCaps to determine if
/// the source and destination DCs are compatible.
/// PlgBlt returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::PlgBlt(const TPoint& dst,
                        const TDC& srcDC, const TRect& src,
                        const TBitmap& maskBm, const TPoint& maskPos)
{
  return ::PlgBlt(GetHDC(), (TPoint*)&dst,
                  srcDC, src.left, src.top, src.Width(), src.Height(),
                  maskBm, maskPos.x, maskPos.y);
}

//
/// Performs a bit-block transfer of the color data corresponding to a rectangle of pixels from 
/// the specified source device context into a destination device context.
/// Returns true on success; otherwise false.
/// This function is a wrapper for the Windows API TransparentBlt function.
/// http://msdn.microsoft.com/en-us/library/dd145141
//
inline bool TDC::TransparentBlt(const TRect& dst, const TDC& srcDC, const TRect& src, const TColor& transparent)
{
  return ::TransparentBlt(GetHDC(), 
    dst.X(), dst.Y(), dst.Width(), dst.Height(), 
    srcDC.GetHDC(), src.X(), src.Y(), src.Width(), src.Height(), 
    transparent.GetValue());
}

//
/// The first version retrieves some or all of the bits from the given bitmap on
/// this DC and copies them to the bits buffer using the DIB (device-independent
/// bitmap) format specified by the BITMAPINFO argument, info. numScan scanlines of
/// the bitmap are retrieved, starting at scan line startScan. The usage argument
/// determines the format of the bmiColors member of the BITMAPINFO structure,
/// according to the following table:
/// - \c \b  DIB_PAL_COLORS	The color table is an array of 16-bit indexes into the current
/// logical palette.
/// - \c \b  DIB_RGB_COLORS	The color table contains literal RGB values.
/// - \c \b  DIB_PAL_INDICES	There is no color table for this bitmap. The DIB bits consist of
/// indexes into the system palette. No color translation occurs. Only the
/// BITMAPINFOHEADER portion of BITMAPINFO is filled in.
///
/// In the second version of GetDIBits, the bits are retrieved from bitmap and
/// placed in the dib.Bits data member of the given TDib argument. The BITMAPINFO
/// argument is supplied from dib.info.
/// GetDIBits returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::GetDIBits(const TBitmap& bitmap, uint startScan, uint numScans,
                           void * bits, const BITMAPINFO & info,
                           uint16 usage)
{
  return ::GetDIBits(GetHDC(), bitmap, startScan, numScans, bits,
                    (BITMAPINFO *)&info, usage);
}

//
/// The first version retrieves some or all of the bits from the given bitmap on
/// this DC and copies them to the bits buffer using the DIB (device-independent
/// bitmap) format specified by the BITMAPINFO argument, info. numScan scanlines of
/// the bitmap are retrieved, starting at scan line startScan. The usage argument
/// determines the format of the bmiColors member of the BITMAPINFO structure,
/// according to the following table:
/// - \c \b  DIB_PAL_COLORS	The color table is an array of 16-bit indexes into the current
/// logical palette.
/// - \c \b  DIB_RGB_COLORS	The color table contains literal RGB values.
/// - \c \b  DIB_PAL_INDICES	There is no color table for this bitmap. The DIB bits consist of
/// indexes into the system palette. No color translation occurs. Only the
/// BITMAPINFOHEADER portion of BITMAPINFO is filled in.
///
/// In the second version of GetDIBits, the bits are retrieved from bitmap and
/// placed in the dib.Bits data member of the given TDib argument. The BITMAPINFO
/// argument is supplied from dib.info.
/// GetDIBits returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::GetDIBits(const TBitmap& bitmap, TDib& dib)
{
  return ::GetDIBits(GetHDC(), bitmap, dib.StartScan(), dib.NumScans(),
                     dib.GetBits(), dib.GetInfo(), dib.Usage());
}

//
/// The first version sets the pixels in bitmap (the given destination bitmap on
/// this DC) from the source DIB (device-independent bitmap) color data found in the
/// byte array bits and the BITMAPINFO structure, Info. numScan scanlines are taken
/// from the DIB, starting at scanline startScan. The usage argument specifies how
/// the bmiColors member of BITMAPINFO is interpreted, as explained in
/// TDC::GetDIBits.
/// In the second version of SetDIBits, the pixels are set in bitmap from the given
/// source TDib argument.
/// SetDIBits returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::SetDIBits(TBitmap& bitmap, uint startScan, uint numScans,
                           const void * bits, const BITMAPINFO & info,
                           uint16 usage)
{
  return ::SetDIBits(GetHDC(), bitmap, startScan, numScans,
                     bits, (BITMAPINFO *)&info, usage);
                     // API typecast
}

//
/// The first version sets the pixels in bitmap (the given destination bitmap on
/// this DC) from the source DIB (device-independent bitmap) color data found in the
/// byte array bits and the BITMAPINFO structure, Info. numScan scanlines are taken
/// from the DIB, starting at scanline startScan. The usage argument specifies how
/// the bmiColors member of BITMAPINFO is interpreted, as explained in
/// TDC::GetDIBits.
/// In the second version of SetDIBits, the pixels are set in bitmap from the given
/// source TDib argument.
/// SetDIBits returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::SetDIBits(TBitmap& bitmap, const TDib& dib)
{
  return ::SetDIBits(GetHDC(), bitmap, dib.StartScan(), dib.NumScans(),
                     dib.GetBits(), (BITMAPINFO *)dib.GetInfo(), dib.Usage());
                                   // API typecast
}

//
/// The first version sets the pixels in dst (the given destination rectangle on
/// this DC) from the source DIB (device-independent bitmap) color data found in the
/// byte array bits and the BITMAPINFO structure, bitsInfo. The DIB origin is
/// specified by the point src. numScan scanlines are taken from the DIB, starting
/// at scanline startScan. The usage argument determines how the bmiColors member of
/// BITMAPINFO is interpreted, as explained in TDC::GetDIBits.
/// In the second version of SetDIBitsToDevice, the pixels are set in dst from dib,
/// the given source TDib argument.
/// SetDIBitsToDevice returns true if the call is successful; otherwise, it returns
/// false.
//
inline bool TDC::SetDIBitsToDevice(const TRect& dst, const TPoint& src,
                                   uint startScan, uint numScans,
                                   const void * bits,
                                   const BITMAPINFO & info, uint16 usage)
{
  return ::SetDIBitsToDevice(
             GetHDC(), dst.left, dst.top,
             dst.Width(), dst.Height(), src.x, src.y,
             startScan, numScans,
             (void *)bits, (BITMAPINFO *)&info, usage
         );                  // API typecast
}

//
/// The first version sets the pixels in dst (the given destination rectangle on
/// this DC) from the source DIB (device-independent bitmap) color data found in the
/// byte array bits and the BITMAPINFO structure, bitsInfo. The DIB origin is
/// specified by the point src. numScan scanlines are taken from the DIB, starting
/// at scanline startScan. The usage argument determines how the bmiColors member of
/// BITMAPINFO is interpreted, as explained in TDC::GetDIBits.
/// In the second version of SetDIBitsToDevice, the pixels are set in dst from dib,
/// the given source TDib argument.
/// SetDIBitsToDevice returns true if the call is successful; otherwise, it returns
/// false.
//
inline bool TDC::SetDIBitsToDevice(const TRect& dst, const TPoint& src,
                                   const TDib& dib)
{
  return ::SetDIBitsToDevice(
             GetHDC(),
             dst.left, dst.top, dst.Width(), dst.Height(),
             src.x, src.y,
             dib.StartScan(), dib.NumScans(),
             (void *)dib.GetBits(),
             (BITMAPINFO *)dib.GetInfo(), dib.Usage()
         );                  // API typecast
}

//
/// Copies the color data from src, the source rectangle of pixels in the given DIB
/// (device-independent bitmap) on this DC, to dst, the destination rectangle. The
/// DIB bits and color data are specified in either the byte array bits and the
/// BITMAPINFO structure bitsInfo or in the TDib object, dib. The rows and columns
/// of color data are stretched or compressed to match the size of the destination
/// rectangle. The usage argument specifies how the bmiColors member of BITMAPINFO
/// is interpreted, as explained in TDC::GetDIBits The raster operation code, rop,
/// specifies how the source pixels, the current brush for this DC, and the
/// destination pixels are combined to produce the new image. See TDC::MaskBlt for a
/// detailed list of rop codes.
//
inline bool TDC::StretchDIBits(const TRect& dst, const TRect& src,
                               const void * bits,
                               const BITMAPINFO & info,
                               uint16 usage, uint32 rop)
{
  return ::StretchDIBits(
             GetHDC(), dst.left, dst.top, dst.Width(), dst.Height(),
             src.left, src.top, src.Width(), src.Height(),
             bits, (BITMAPINFO *)&info, usage, rop
         );
         // API typecast
}

//
/// Copies the color data from src, the source rectangle of pixels in the given DIB
/// (device-independent bitmap) on this DC, to dst, the destination rectangle. The
/// DIB bits and color data are specified in either the byte array bits and the
/// BITMAPINFO structure bitsInfo or in the TDib object, dib. The rows and columns
/// of color data are stretched or compressed to match the size of the destination
/// rectangle. The usage argument specifies how the bmiColors member of BITMAPINFO
/// is interpreted, as explained in TDC::GetDIBits The raster operation code, rop,
/// specifies how the source pixels, the current brush for this DC, and the
/// destination pixels are combined to produce the new image. See TDC::MaskBlt for a
/// detailed list of rop codes.
//
inline bool TDC::StretchDIBits(const TRect& dst, const TRect& src,
                               const TDib& dib, uint32 rop)
{
  return ::StretchDIBits(
             GetHDC(),
             dst.left, dst.top, dst.Width(), dst.Height(),
             src.left, src.top, src.Width(), src.Height(),
             dib.GetBits(),
             (BITMAPINFO *)dib.GetInfo(),
             dib.Usage(), rop
         );
             // API typecast
}

//
/// Create a fast DIB under Win32.
//
inline HBITMAP TDC::CreateDIBSection(const BITMAPINFO& info, uint usage, void** bits, HANDLE section, uint32 offset)
{
#ifdef WINELIB
  return ::CreateDIBSection(GetHDC(), (BITMAPINFO*)&info, usage, bits, section, offset);
#else
  return ::CreateDIBSection(GetHDC(), &info, usage, bits, section, offset);
#endif
}

//
/// Fills an area on this DC starting at point and using the currently selected
/// brush object. The color argument specifies the color of the boundary or of the
/// area to be filled. Returns true if the call is successful; otherwise, returns
/// false. FloodFill is maintained in the WIN32 API for compatibility with earlier
/// APIs. New WIN32 applications should use TDC::ExtFloodFill.
//
inline bool TDC::FloodFill(const TPoint& point, const TColor& color)
{
  return ::FloodFill(GetHDC(), point.x, point.y, color);
}

//
/// Fills an area on this DC starting at point and using the currently selected
/// brush object. The color argument specifies the color of the boundary or of the
/// region to be filled. The fillType argument specifies the type of fill, as
/// follows:
/// - \c \b  FLOODFILLBORDER	The fill region is bounded by the given color. This style
/// coincides with the filling method used by FloodFill().
/// - \c \b  FLOODFILLSURFACE	The fill region is defined by the given color. Filling
/// continues outward in all directions as long as this color is encountered. Use
/// this style when filling regions with multicolored borders.
///
/// Not every device supports ExtFloodFill, so applications should test first with
/// TDC::GetDeviceCaps.
/// ExtFloodFill returns true if the call is successful; otherwise, it returns
/// false.
//
inline bool TDC::ExtFloodFill(const TPoint& point, const TColor& color, uint16 fillType)
{
  return ::ExtFloodFill(GetHDC(), point.x, point.y, color, fillType);
}

//
/// Draws up to count characters of the given null-terminated string in the current
/// font on this DC. If count is -1 (the default), the entire string is written.
/// The (x, y) or p arguments specify the logical coordinates of the reference point
/// that is used to align the first character, depending on the current
/// text-alignment mode. This mode can be inspected with TDC::GetTextAlign and
/// changed with TDC::SetTextAlign. By default, the current position is neither used
/// nor altered by TextOut. However, the align mode can be set to TA_UPDATECP, which
/// makes Windows use and update the current position. In this mode, TextOut ignores
/// the reference point argument(s).
/// TextOut returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::TextOut(const TPoint& p, const tstring& str, int count)
{
  return TextOut(p.x, p.y, str, count);
}

//
/// Draws up to count characters of the given null-terminated string in the current
/// font on this DC. If count is -1, the whole string is written.
/// An optional rectangle r can be specified for clipping, opaquing, or both, as
/// determined by the options value. If options is set to ETO_CLIPPED, the rectangle
/// is used for clipping the drawn text. If options is set to ETO_OPAQUE, the
/// current background color is used to fill the rectangle. Both options can be used
/// if ETO_CLIPPED is OR'd with ETO_OPAQUE.
/// The (x, y) orp arguments specify the logical coordinates of the reference point
/// that is used to align the first character. The current text-alignment mode can
/// be inspected with TDC::GetTextAlign and changed with TDC::SetTextAlign. By
/// default, the current position is neither used nor altered by ExtTextOut.
/// However, if the align mode is set to TA_UPDATECP, ExtTextOut ignores the
/// reference point argument(s) and uses or updates the current position as the
/// reference point.
/// The dx argument is an optional array of values used to set the distances between
/// the origins (upper left corners) of adjacent character cells. For example, dx[i]
/// represents the number of logical units separating the origins of character cells
/// i and i+1. If dx is 0, ExtTextOut uses the default inter-character spacings.
/// ExtTextOut returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::ExtTextOut(const TPoint& p, uint16 options, const TRect* rect,
  const tstring& str, int count, const int * dx)
{
  return ExtTextOut(p.x, p.y, options, rect, str, count, dx);
}

//
/// Draws up to count characters of the given null-terminated string in the current
/// font on this DC. If count is -1, the whole string is written.
/// Tabs are expanded according to the given arguments. The positions array
/// specifies numPositions tab stops given in device units. The tab stops must have
/// strictly increasing values in the array. If numPositions and positions are both
/// 0, tabs are expanded to eight times the average character width. If numPositions
/// is 1, all tab stops are taken to be positions[0] apart. tabOrigin specifies the
/// x-coordinate in logical units from which tab expansion will start.
/// The p argument specifies the logical coordinates of the reference point that is
/// used to align the first character depending on the current text-alignment mode.
/// This mode can be inspected with TDC::GetTextAlign and changed with
/// TDC::SetTextAlign. By default, the current position is neither used nor altered
/// by TabbedTextOut. However, if the align mode is set to TA_UPDATECP,
/// TabbedTextOut ignores the reference point argument(s) and uses/updates the
/// current position as the reference point.
/// The size argument in the second version of TabbedTextOut reports the dimensions
/// (size.y = height and size.x = width) of the string in logical units.
/// TabbedTextOut returns true if the call is successful; otherwise, it returns
/// false.
//
inline bool TDC::TabbedTextOut(const TPoint& p, const tstring& str, int count,
  int numPositions, const int* positions, int tabOrigin)
{
  TSize size;
  TabbedTextOut(p, str, count, numPositions, positions, tabOrigin, size);
  return true;
}

//
/// Computes the extent (width and height) in logical units of the text line
/// consisting of stringLen characters from the null-terminated string. The extent
/// is calculated from the metrics of the current font or this DC, but ignores the
/// current clipping region. In the first version of GetTextExtent the extent is
/// returned in size; in the second version, the extent is the returned TSize
/// object. Width is size.cx and height is size.cy.
/// If kerning is being applied, the sum of the extents of the characters in a
/// string might not equal the extent of the string.
//
inline bool TDC::GetTextExtent(const tstring& str, int stringLen, TSize& size) const
{
  return ::GetTextExtentPoint32(GetAttributeHDC(), str.c_str(), stringLen, &size);
}

//
/// Computes the extent (width and height) in logical units of the text line
/// consisting of stringLen characters from the null-terminated string. The extent
/// is calculated from the metrics of the current font or this DC, but ignores the
/// current clipping region. In the first version of GetTextExtent the extent is
/// returned in size; in the second version, the extent is the returned TSize
/// object. Width is size.cx and height is size.cy.
/// If kerning is being applied, the sum of the extents of the characters in a
/// string might not equal the extent of the string.
//
inline TSize TDC::GetTextExtent(const tstring& str, int stringLen) const
{
  TSize size;
  GetTextExtent(str, stringLen, size);
  return size;
}

//
/// Computes the extent (width and height) in logical units of the text line
/// consisting of stringLen characters from the null-terminated string. The extent
/// is calculated from the metrics of the current font or this DC, but ignores the
/// current clipping region. In the first version of GetTabbedTextExtent, the extent
/// is returned in size; in the second version, the extent is the returned TSize
/// object. Width is size.x and width is size.y.
/// The width calculation includes the spaces implied by any tab codes in the
/// string. Such tab codes are interpreted using the numPositions and positions
/// arguments. The positions array specifies numPositions tab stops given in device
/// units. The tab stops must have strictly increasing values in the array. If
/// numPositions and positions are both 0, tabs are expanded to eight times the
/// average character width. If numPositions is 1, all tab stops are taken to be
/// positions[0] apart.
/// If kerning is being applied, the sum of the extents of the characters in a
/// string might not equal the extent of the string.
//
inline bool TDC::GetTabbedTextExtent(const tstring& str, int strLen,
  int numPositions, const int* positions, TSize& size) const
{
  size = TSize(::GetTabbedTextExtent(GetAttributeHDC(), str.c_str(), strLen, numPositions,
    const_cast<int*>(positions)));
  return true;
}

//
/// Computes the extent (width and height) in logical units of the text line
/// consisting of stringLen characters from the null-terminated string. The extent
/// is calculated from the metrics of the current font or this DC, but ignores the
/// current clipping region. In the first version of GetTabbedTextExtent, the extent
/// is returned in size; in the second version, the extent is the returned TSize
/// object. Width is size.x and width is size.y.
/// The width calculation includes the spaces implied by any tab codes in the
/// string. Such tab codes are interpreted using the numPositions and positions
/// arguments. The positions array specifies numPositions tab stops given in device
/// units. The tab stops must have strictly increasing values in the array. If
/// numPositions and positions are both 0, tabs are expanded to eight times the
/// average character width. If numPositions is 1, all tab stops are taken to be
/// positions[0] apart.
/// If kerning is being applied, the sum of the extents of the characters in a
/// string might not equal the extent of the string.
//
inline TSize TDC::GetTabbedTextExtent(const tstring& str, int strLen,
  int numPositions, const int* positions) const
{
  return TSize(::GetTabbedTextExtent(GetAttributeHDC(), str.c_str(), strLen, numPositions,
    const_cast<int*>(positions)));
}

//
/// If successful, returns the current text-alignment flags for this DC; otherwise,
/// returns the value GDI_ERROR. The text-alignment flags determine how TDC::TextOut
/// and TDC::ExtTextOut align text strings in relation to the first character's
/// screen position. GetTextAlign returns certain combinations of the flags listed
/// in the following table:
/// - \c \b  TA_BASELINE	The reference point will be on the baseline of the text.
/// - \c \b  TA_BOTTOM	The reference point will be on the bottom edge of the bounding
/// rectangle.
/// - \c \b  TA_TOP	The reference point will be on the top edge of the bounding rectangle.
/// - \c \b  TA_CENTER	The reference point will be aligned horizontally with the center of
/// the bounding rectangle.
/// - \c \b  TA_LEFT	The reference point will be on the left edge of the bounding rectangle.
/// - \c \b  TA_RIGHT	The reference point will be on the right edge of the bounding
/// rectangle.
/// - \c \b  TA_NOUPDATECP	The current position is not updated after each text output call.
/// - \c \b  TA_UPDATECP	The current position is updated after each text output call.
///
/// 	When the current font has a vertical default baseline (as with Kanji) the
/// following values replace TA_BASELINE and TA_CENTER:
/// - \c \b  VTA_BASELINE	The reference point will be on the baseline of the text.
/// - \c \b  VTA_CENTER	The reference point will be aligned vertically with the center of
/// the bounding rectangle.
///
/// The text-alignment flags are not necessarily single bit-flags and might be equal
/// to 0. The flags must be examined in groups of the following related flags:
/// - \c \b  TA_LEFT, TA_RIGHT, and TA_CENTER
/// - \c \b  TA_BOTTOM, TA_TOP, and TA_BASELINE
/// - \c \b  TA_NOUPDATECP and TA_UPDATECP
///
/// If the current font has a vertical default baseline (as with Kanji), these are
/// groups of related flags:
/// - \c \b  TA_LEFT, TA_RIGHT, and VTA_BASELINE
/// - \c \b  TA_BOTTOM, TA_TOP, and VTA_CENTER
/// - \c \b  TA_NOUPDATECP and TA_UPDATECP
///
/// To verify that a particular flag is set in the return value of this function,
/// the application must perform the following steps:
/// - 1.	Apply the bitwise OR operator to the flag and its related flags.
/// - 2.	Apply the bitwise AND operator to the result and the return value.
/// - 3.	Test for the equality of this result and the flag.
///
/// The following example shows a method for determining which horizontal alignment
/// flag is set:
/// \code
/// switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & dc.GetTextAlign()) { 
/// 	case TA_LEFT:
/// 	...
/// 	case TA_RIGHT:
/// 	...
/// 	case TA_CENTER:
/// 	...
/// }
/// \endcode
//
inline uint TDC::GetTextAlign() const
{
  return ::GetTextAlign(GetAttributeHDC());
}

//
/// Sets the text-alignment flags for this DC. If successful, SetTextAlign returns
/// the previous text-alignment flags; otherwise, it returns GDI_ERROR. The flag
/// values are as listed for the TDC::GetTextAlign function. The text-alignment
/// flags determine how TDC::TextOut and TDC::ExtTextOut align text strings in
/// relation to the first character's screen position.
//
inline uint TDC::SetTextAlign(uint flags)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetTextAlign(GetHDC(), flags);
  return ::SetTextAlign(GetAttributeHDC(), flags);
}

//
/// If successful, returns the current intercharacter spacing, in logical units, for
/// this DC; otherwise, returns INVALID_WIDTH.
//
inline int TDC::GetTextCharacterExtra() const
{
  return ::GetTextCharacterExtra(GetAttributeHDC());
}

//
/// If successful, sets the current intercharacter spacing to extra, in logical
/// units, for this DC, and returns the previous intercharacter spacing. Otherwise,
/// returns 0. If the current mapping mode is not MM_TEXT, the extra value is
/// transformed and rounded to the nearest pixel.
//
inline int TDC::SetTextCharacterExtra(int extra)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetTextCharacterExtra(GetHDC(), extra);
  return ::SetTextCharacterExtra(GetAttributeHDC(), extra);
}

//
/// When text strings are displayed using TDC::TextOut and TDC::ExtTextOut, sets the
/// number of logical units specified in breakExtra as the total extra space to be
/// added to the number of break characters specified in breakCount. The extra space
/// is distributed evenly between the break characters. The break character is
/// usually ASCII 32 (space), but some fonts define other characters.
/// TDC::GetTextMetrics can be used to retrieve the value of the break character.
/// If the current mapping mode is not MM_TEXT, the extra value is transformed and
/// rounded to the nearest pixel.
/// SetTextJustification returns true if the call is successful; otherwise, it
/// returns false.
//
inline bool TDC::SetTextJustification(int breakExtra, int breakCount)
{
  return ::SetTextJustification(GetAttributeHDC(), breakExtra, breakCount);
}

//
/// Retrieves the length of the typeface name for the current font on this DC.
/// The length is the number of characters excluding null-termination.
//
inline int TDC::GetTextFaceLength() const
{
  return GetTextFace(0, 0) - 1;
}

//
/// Retrieves the typeface name for the current font on this DC. Up to count
/// characters of this name are copied to facename. If successful, GetTextFace
/// returns the number of characters actually copied; otherwise, it returns 0.
//
inline int TDC::GetTextFace(int count, LPTSTR facename) const
{
  return ::GetTextFace(GetAttributeHDC(), count, facename);
}

//
/// Fills the metrics structure with metrics data for the current font on this DC.
/// Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::GetTextMetrics(TEXTMETRIC & metrics) const
{
  return ::GetTextMetrics(GetAttributeHDC(), &metrics);
}

//
/// Retrieves TrueType metric and other data for the given character, chr, on this
/// DC and places it in gm and buffer. The format argument specifies the format of
/// the retrieved data as indicated in the following table. (A value of 0 simply
/// fills in the GLYPHMETRICS structure but does not return glyph-outline data.)
/// - \c \b  1 - Retrieves the glyph bitmap.
/// - \c \b  2 - Retrieves the curve data points in the rasterizer's native format and uses the
/// font's design units. With this value of format, the mat2 transformation argument
/// is ignored.
///
/// The gm argument specifies the GLYPHMETRICS structure that describes the
/// placement of the glyph in the character cell. buffSize specifies the size of
/// buffer that receives data about the outline character. If either buffSize or
/// buffer is 0, GetGlyphOutline returns the required buffer size. Applications can
/// rotate characters retrieved in bitmap format (format = 1) by specifying a 2 x 2
/// transformation matrix via the mat2 argument.
/// GetGlyphOutline returns a positive number if the call is successful; otherwise,
/// it returns GDI_ERROR.
//
inline uint32 TDC::GetGlyphOutline(uint chr, uint format, GLYPHMETRICS & gm,
                                  uint32 buffSize, void * buffer,
                                  const MAT2 & mat2)
{
  return ::GetGlyphOutline(GetAttributeHDC(), chr, format, &gm, buffSize,
                           buffer, (MAT2*)&mat2);
}

//
/// Retrieves kerning pairs for the current font of this DC up to the number
/// specified in pairs and copies them into the krnPair array of KERNINGPAIR
/// structures. If successful, the function returns the actual number of pairs
/// retrieved. If the font has more than pairs kerning pairs, the call fails and
/// returns 0. The krnPair array must allow for at least pairs KERNINGPAIR
/// structures. If krnPair is set to 0, GetKerningPairs returns the total number of
/// kerning pairs for the current font.
//
inline int TDC::GetKerningPairs(int pairs, KERNINGPAIR * krnPair)
{
  return ::GetKerningPairs(GetAttributeHDC(), pairs, krnPair);
}

//
/// Retrieves metric information for TrueType fonts on this DC and copies it to the
/// given array of OUTLINETEXTMETRIC structures, otm. This structure contains a
/// TEXTMETRIC and several other metric members, as well as four string-pointer
/// members for holding family, face, style, and full font names. Since memory must
/// be allocated for these variable-length strings in addition to the font metric
/// data, you must pass (with the data argument) the total number of bytes required
/// for the retrieved data. If GetOutlineTextMetrics is called with otm = 0, the
/// function returns the total buffer size required. You can then assign this value
/// to data in subsequent calls.
/// Returns nonzero if the call is successful; otherwise, returns 0.
//
inline uint
TDC::GetOutlineTextMetrics(uint data, OUTLINETEXTMETRIC & otm)
{
  return ::GetOutlineTextMetrics(GetAttributeHDC(), data, &otm);
}

//
/// Retrieves the widths in logical units for a consecutive sequence of characters
/// in the current font for this DC. The sequence is specified by the inclusive
/// range, firstChar to lastChar, and the widths are copied to the given buffer. If
/// a character in the range is not represented in the current font, the width of
/// the default character is assigned. Returns true if the call is successful;
/// otherwise, returns false.
//
inline bool TDC::GetCharWidth(uint firstChar, uint lastChar, int* buffer)
{
  return ::GetCharWidth(GetAttributeHDC(), firstChar, lastChar, buffer);
}

//
/// Alters the algorithm used by the font mapper when mapping logical fonts to
/// physical fonts on this DC. If successful, the function sets the current
/// font-mapping flag to flag and returns the previous mapping flag; otherwise
/// GDI_ERROR is returned. The mapping flag determines whether the font mapper will
/// attempt to match a font's aspect ratio to this DC's aspect ratio. If bit 0 of
/// flag is set to 1, the mapper selects only matching fonts. If no matching fonts
/// exist, a new aspect ratio is chosen and a font is retrieved to match this ratio.
//
inline uint32 TDC::SetMapperFlags(uint32 flag)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetMapperFlags(GetHDC(), flag);
  return ::SetMapperFlags(GetAttributeHDC(), flag);
}

//
/// Retrieves the setting of the current aspect-ratio filter for this DC.
//
inline bool TDC::GetAspectRatioFilter(TSize& size) const
{
  return ::GetAspectRatioFilterEx(GetAttributeHDC(), &size);
}

//
/// Enumerates the fonts available on this DC for the given faceName. The font type,
/// LOGFONT, and TEXTMETRIC data retrieved for each available font is passed to the
/// user-defined callback function together with any optional, user-supplied data
/// placed in the data buffer. The callback function can process this data in any
/// way desired. Enumeration continues until there are no more fonts or until the
/// callback function returns 0. If faceName is 0, EnumFonts randomly selects and
/// enumerates one font of each available typeface. EnumFonts returns the last value
/// returned by the callback function. Note that OLDFONTENUMPROC is defined as
/// FONTENUMPROC for Win32 only. FONTENUMPROC is a pointer to a user-defined
/// function and has the following prototype:
/// \code
/// int CALLBACK EnumFontsProc(LOGFONT *lplf, TEXTMETRIC *lptm, uint32 dwType,
/// LPARAM lpData);
/// \endcode
/// where dwType specifies one of the following font types: DEVICE_FONTTYPE,
/// RASTER_FONTTYPE, or TRUETYPE_FONTTYPE.
//
inline int TDC::EnumFonts(LPCTSTR faceName, OLDFONTENUMPROC callback, void * data) const
{
  return ::EnumFonts(GetAttributeHDC(), faceName, callback, LPARAM(data));
}

//
/// Retrieves the widths of consecutive characters in the range firstChar to
/// lastChar from the current TrueType font of this DC. The widths are reported in
/// the array abc of ABC structures. Returns true if the call is successful;
/// otherwise, returns false.
//
inline bool TDC::GetCharABCWidths(uint firstChar, uint lastChar, ABC* abc)
{
  return ::GetCharABCWidths(GetAttributeHDC(), firstChar, lastChar, abc);
}

//
/// Enumerates the fonts available to this DC in the font family specified by
/// family. The given application-defined callback proc is called for each font in
/// the family or until proc returns 0, and is defined as:
/// \code
/// typedef int (CALLBACK* FONTENUMPROC)(CONST LOGFONT *, CONST TEXTMETRIC *, DWORD,
/// LPARAM);
/// \endcode
/// data lets you pass both application-specific data and font data to proc. If
/// successful, the call returns the last value returned by proc.
//
inline int TDC::EnumFontFamilies(LPCTSTR familyName, FONTENUMPROC callback, void * data) const
{
  return ::EnumFontFamilies(GetAttributeHDC(), familyName, callback, LPARAM(data));
}

//
/// Retrieves font-metric information from a scalable TrueType font file (specified
/// by table and starting at offset into this table) and places it in the given
/// buffer. data specifies the size in bytes of the data to be retrieved. If the
/// call is successful, it returns the number of bytes set in buffer; otherwise, -1
/// is returned.
//
inline uint32 TDC::GetFontData(uint32 table, uint32 offset, void* buffer, long data)
{
  return ::GetFontData(GetAttributeHDC(), table, offset, buffer, data);
}

//
/// Opens a new path bracket for this DC and discards any previous
/// paths from this DC. Once a path bracket is open, an application can start
/// calling draw functions on this DC to define the points that lie within that
/// path. The draw functions that define points in a path are the following TDC
/// members: AngleArc, Arc, Chord, CloseFigure, Ellipse, ExtTextOut, LineTo,
/// MoveToEx, Pie, PolyBezier, PolyBezierTo, PolyDraw, Polygon, Polyline,
/// PolylineTo, PolyPolygon, PolyPolyline, Rectangle, RoundRect, and TextOut.
/// A path bracket can be closed by calling TDC::EndPath.
/// BeginPath returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::BeginPath()
{
  return ::BeginPath(GetHDC());
}

//
/// Closes an open figure in this DC's open path bracket by drawing a
/// line from the current position to the first point of the figure (usually the
/// point specified by the most recent TDC::MoveTo call), and connecting the lines
/// using the current join style for this DC. If you close a figure with TDC::LineTo
/// instead of with CloseFigure, end caps (instead of a join) are used to create the
/// corner. The call fails if there is no open path bracket on this DC. Any line or
/// curve added to the path after a CloseFigure call starts a new figure. A figure
/// in a path remains open until it is explicitly closed with CloseFigure even if
/// its current position and start point happen to coincide.
/// CloseFigure returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::CloseFigure()
{
  return ::CloseFigure(GetHDC());
}

//
/// Closes the path bracket and selects the path it defines into this
/// DC. Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::EndPath()
{
  return ::EndPath(GetHDC());
}

//
/// Transforms any curves in the currently selected path of this DC.
/// All such curves are changed to sequences of linear segments. Returns true if the
/// call is successful; otherwise, returns false.
//
inline bool TDC::FlattenPath()
{
  return ::FlattenPath(GetHDC());
}

//
/// Redefines the current, closed path on this DC as the area that
/// would be painted if the path were stroked with this DC's current pen. The
/// current pen must have been created under the following conditions:
/// If the TPen(int Style, int Width, TColor Color) constructor, or the TPen(const
/// LOGPEN* LogPen) constructor is used, the width of the pen in device units must
/// be greater than 1.
/// If the TPen(uint32 PenStyle, uint32 Width, const TBrush& Brush, uint32
/// StyleCount, LPDWORD pSTyle) constructor, or the TPen(uint32 PenStyle, uint32
/// Width, const LOGBRUSH& logBrush, uint32 StyleCount, LPDWORD pSTyle) constructor
/// is used, the pen must be a geometric pen.
/// Any Bezier curves in the path are converted to sequences of linear segments
/// approximating the widened curves, so no Bezier curves remain in the path after a
/// WidenPath call.
/// WidenPath returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::WidenPath()
{
  return ::WidenPath(GetHDC());
}

//
/// Closes any open figures in the current path of this DC and fills
/// the path's interior using the current brush and polygon fill mode. After filling
/// the interior, FillPath discards the path from this DC.
/// FillPath returns true if the call is successful; otherwise, it returns false.
//
inline bool TDC::FillPath()
{
  return ::FillPath(GetHDC());
}

//
/// Renders the current, closed path on this DC and uses the DC's
/// current pen.
//
inline bool TDC::StrokePath()
{
  return ::StrokePath(GetHDC());
}

//
/// Closes any open figures in the current path of this DC, strokes the
/// outline of the path using the current pen, and fills its interior using the
/// current brush and polygon fill mode. Returns true if the call is successful;
/// otherwise, returns false.
//
inline bool TDC::StrokeAndFillPath()
{
  return ::StrokeAndFillPath(GetHDC());
}

//
/// Selects the current path on this DC as a clipping region, combining any existing
/// clipping region using the specified mode as shown in the following table:
/// - \c \b  RGN_AND	The new clipping region includes the overlapping areas of the current
/// clipping region and the current path (intersection).
/// - \c \b  RGN_COPY	The new clipping region is the current path.
/// - \c \b  RGN_DIFF	The new clipping region includes the areas of the current clipping
/// region with those of the current path excluded.
/// - \c \b  RGN_OR	The new clipping region includes the combined areas of the current
/// clipping region and the current path (union).
/// - \c \b  RGN_XOR	The new clipping region includes the combined areas of the current
/// clipping region and the current path but without the overlapping areas.
///
/// Returns true if the call is successful; otherwise, returns false.
//
inline bool TDC::SelectClipPath(int mode)
{
  return ::SelectClipPath(GetHDC(), mode);
}

//
/// If successful, returns a region created from the closed path in this DC;
/// otherwise, returns 0.
//
inline HRGN TDC::PathToRegion()
{
  return ::PathToRegion(GetHDC());
}

//
inline void TMemoryDC::SelectObject(const TBrush& brush)
{
  TDC::SelectObject(brush);
}

//
inline void TMemoryDC::SelectObject(const TPen& pen)
{
  TDC::SelectObject(pen);
}

//
inline void TMemoryDC::SelectObject(const TFont& font)
{
  TDC::SelectObject(font);
}

//
inline void TMemoryDC::SelectObject(const TPalette& palette,
                                    bool forceBackground)
{
  TDC::SelectObject(palette, forceBackground);
}

//
inline void TMemoryDC::RestoreObjects()
{
  TDC::RestoreObjects();
  RestoreBitmap();
}

//
/// Closes this metafile DC object. Sets the Handle data member to 0 and returns a
/// pointer to a new TMetaFilePict object.
//
inline HMETAFILE TMetaFileDC::Close()
{
  HMETAFILE mf = ::CloseMetaFile(GetHDC());
//#if defined(__GNUC__)//JJH I didn't find better solution for this this
//  TGdiBase::Handle = 0;
//#else
  Handle = 0;
//#endif
  return mf;
}

//
// Returns true if the device context contains an enhanced metafile.
//
inline bool TMetaFileDC::IsEnhanced() const
{
  return Enhanced;
}

//
// Closes the enhanced metafile DC.
//
inline HENHMETAFILE TMetaFileDC::CloseEnh()
{
  HENHMETAFILE emf = ::CloseEnhMetaFile(GetHDC());
//#if defined(__GNUC__)//JJH I didn't find better solution for this this
//  TGdiBase::Handle = 0;
//#else
  Handle = 0;
//#endif
  return emf;
}

//
/// Insert a comment record into the enhanced metafile.
//
inline bool TMetaFileDC::Comment(uint bytes, const void* data) {
  if (IsEnhanced()) {
    return ::GdiComment(GetHDC(), bytes, (const BYTE*)data);
  }
  return false;
}


} // OWL namespace

namespace owl {
//
/// Return the DOCINFO structure.
//
inline DOCINFO& TPrintDC::GetDocInfo()
{
  return DocInfo;
}

//
/// Sets the DOCINFO structure.
//
inline void TPrintDC::SetDocInfo(const DOCINFO& docinfo)
{
  DocInfo = docinfo;
}

//
/// Allows applications to access the capabilities of a particular device that are
/// not directly available through the GDI of this DC. The Escape call is specified
/// by setting a mnemonic value in the escape argument. In Win32 the use of Escape
/// with certain escape values has been replaced by specific functions. The names of
/// these new functions are based on the corresponding escape mnemonic, as shown in
/// the following table:
/// - \c \b  ABORTDOC	Superseded by TPrintDC_AbortDoc in Win32.
/// - \c \b  BANDINFO	Obsolete in Win32. Because all printer drivers for Windows version 3.1
/// and later set the text flag in every band, this escape is useful only for older
/// printer drivers.
/// - \c \b  BEGIN_PATH	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  CLIP_TO_PATH	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  DEVICEDATA	Superseded in Win32. Applications should use the PASSTHROUGH escape
/// to achieve the same functionality.
/// - \c \b  DRAFTMODE	Superseded in Win32. Applications can achieve the same functionality
/// by setting the dmPrintQuality member of the DEVMODE structure to DMRES_DRAFT and
/// passing this structure to the CreateDC function.
/// - \c \b  DRAWPATTERNRECT	No changes for Win32.
/// - \c \b  ENABLEDUPLEX	Superseded in Win32. Applications can achieve the same
/// functionality by setting the dmDuplex member of the DEVMODE structure and
/// passing this structure to the CreateDC function.
/// - \c \b  ENABLEPAIRKERNING	No changes for Win32.
/// - \c \b  ENABLERELATIVEWIDTHS	No changes for Win32.
/// - \c \b  ENDDOC	Superseded by TPrintDC_EndDoc in Win32.
/// - \c \b  END_PATH	No changes for Win32. This escape is specific to PostScript printers.
/// - \c \b  ENUMPAPERBINS	Superseded in Win32. Applications can use
/// TPrintDC::DeviceCapabilities to achieve the same functionality.
/// - \c \b  ENUMPAPERMETRICS	Superseded in Win32. Applications can use
/// TPrintDC::DeviceCapabilities to achieve the same functionality.
/// - \c \b  EPSPRINTING	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  EXT_DEVICE_CAPS	Superseded in Win32. Applications can use TDC::GetDeviceCaps to
/// achieve the same functionality. This escape is specific to PostScript printers.
/// - \c \b  EXTTEXTOUT	Superseded in Win32. Applications can use TDC::ExtTextOut to achieve
/// the same functionality. This escape is not supported by the version 3.1 PCL
/// driver.
/// - \c \b  FLUSHOUTPUT	Removed for Win32.
/// - \c \b  GETCOLORTABLE	Removed for Win32.
/// - \c \b  GETEXTENDEDTEXTMETRICS	No changes for Win32. Support for this escape might
/// change in future versions of Windows.
/// - \c \b  GETEXTENTTABLE	Superseded in Win32. Applications can use ::GetCharWidth to
/// achieve the same functionality. This escape is not supported by the version 3.1
/// PCL or PostScript drivers.
/// - \c \b  GETFACENAME	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  GETPAIRKERNTABLE	No changes for Win32.
/// - \c \b  GETPHYSPAGESIZE	No changes for Win32. Support for this escape might change in
/// future versions of Windows.
/// - \c \b  GETPRINTINGOFFSET	No changes for Win32. Support for this escape might change in
/// future versions of Windows.
/// - \c \b  GETSCALINGFACTOR	No changes for Win32. Support for this escape might change in
/// future versions of Windows.
/// - \c \b  GETSETPAPERBINS	Superseded in Win32. Applications can achieve the same
/// functionality by calling TPrintDC::DeviceCapabilities to find the number of
/// paper bins, calling ::ExtDeviceMode to find the current bin, and then setting
/// the dmDefaultSource member of the DEVMODE structure and passing this structure
/// to the CreateDC function. GETSETPAPERBINS changes the paper bin only for the
/// current device context. A new device context will use the system-default paper
/// bin until the bin is explicitly changed for that device context.
/// - \c \b  GETSETPAPERMETRICS	Obsolete in Win32. Applications can use
/// TPrintDC::DeviceCapabilities and ::ExtDeviceMode to achieve the same
/// functionality.
/// - \c \b  GETSETPAPERORIENT	Obsolete in Win32. Applications can achieve the same
/// functionality by setting the dmOrientation member of the DEVMODE structure and
/// passing this structure to the CreateDC function. This escape is not supported by
/// the Windows 3.1 PCL driver.
/// - \c \b  GETSETSCREENPARAMS	No changes for Win32.
/// - \c \b  GETTECHNOLOGY	No changes for Win32. Support for this escape might change in
/// future versions of Windows. This escape is not supported by the Windows 3.1 PCL
/// driver.
/// - \c \b  GETTRACKKERNTABLE	No changes for Win32.
/// - \c \b  GETVECTORBRUSHSIZE	No changes for Win32. Support for this escape might change
/// in future versions of Windows.
/// - \c \b  GETVECTORPENSIZE	No changes for Win32. Support for this escape might change in
/// future versions of Windows.
/// - \c \b  MFCOMMENT	No changes for Win32.
/// - \c \b  NEWFRAME	No changes for Win32. Applications should use ::StartPage and
/// ::EndPage instead of this escape. Support for this escape might change in future
/// versions of Windows.
/// - \c \b  NEXTBAND	No changes for Win32. Support for this escape might change in future
/// versions of Windows.
/// - \c \b  PASSTHROUGH	No changes for Win32.
/// - \c \b  QUERYESCAPESUPPORT	No changes for Win32.
/// - \c \b  RESTORE_CTM	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SAVE_CTM	No changes for Win32. This escape is specific to PostScript printers.
/// - \c \b  SELECTPAPERSOURCE	Obsolete in Win32. Applications can achieve the same
/// functionality by using TPrintDC::DeviceCapabilities.
/// - \c \b  SETABORTPROC	Superseded in Win32 by ::SetAbortProc. See TPrintDC::SetAbortProc.
/// - \c \b  SETALLJUSTVALUES	No changes for Win32. Support for this escape might change in
/// future versions of Windows. This escape is not supported by the Windows 3.1 PCL
/// driver.
/// - \c \b  SET_ARC_DIRECTION	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SET_BACKGROUND_COLOR	No changes for Win32. Applications should use ::SetBkColor
/// instead of this escape. Support for this escape might change in future versions
/// of Windows.
/// - \c \b  SET_BOUNDS	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SETCOLORTABLE	No changes for Win32. Support for this escape might change in
/// future versions of Windows.
/// - \c \b  SETCOPYCOUNT	Superseded in Win32. An application should call
/// TPrintDC::DeviceCapabilities, specifying DC_COPIES for the capability parameter,
/// to find the maximum number of copies the device can make. Then the application
/// can set the number of copies by passing to the CreateDC function a pointer to
/// the DEVMODE structure.
/// - \c \b  SETKERNTRACK	No changes for Win32.
/// - \c \b  SETLINECAP	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SETLINEJOIN	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SETMITERLIMIT	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SET_POLY_MODE	No changes for Win32. This escape is specific to PostScript
/// printers.
/// - \c \b  SET_SCREEN_ANGLE	No changes for Win32.
/// - \c \b  SET_SPREAD	No changes for Win32.
/// - \c \b  STARTDOC	Superseded in Win32. Applications should call ::StartDoc instead of
/// this escape.
/// - \c \b  TRANSFORM_CTM	No changes for Win32. This escape is specific to PostScript
/// printers.
///
/// Escape calls are translated and sent to the printer device driver. The inData
/// buffer lets you supply any data needed for the escape. You must set count to the
/// size (in bytes) of the inData buffer. If no input data is required, inData and
/// count should be set to the default value of 0. Similarly, you must supply an
/// outData buffer for those Escape calls that retrieve data. If the escape does not
/// supply output, set outData to the default value of 0.
//
inline int TPrintDC::Escape(int esc, int count, const void* inData, void* outData)
{
  return ::Escape(GetHDC(), esc, count, (LPCSTR)inData, outData);
                                        // API type cast
}

//
/// Establishes the user-defined proc as the printer-abort function for this
/// printer. This function is called by TPrintDC::AbortDoc to cancel a print job
/// during spooling.
/// SetAbortProc returns a positive (nonzero) value if successful; otherwise it
/// returns a negative (nonzero) value.
//
inline int TPrintDC::SetAbortProc(ABORTPROC proc)
{
  return ::SetAbortProc(GetHDC(), proc);
}

//
/// Starts a print job for the named document on this printer DC. If successful,
/// StartDoc returns a positive value, the job ID for the document. If the call
/// fails, the value SP_ERROR is returned. Detailed error information can be
/// obtained by calling GetLastError.
/// This function replaces the earlier ::Escape call with value STARTDOC.
//
inline int TPrintDC::StartDoc(LPCTSTR docName, LPCTSTR output)
{
  DocInfo.lpszDocName = (LPTSTR)docName;
  DocInfo.lpszOutput = (LPTSTR)output;

#if (WINVER >= 0x0400) //sga added
 DocInfo.lpszDatatype = NULL; //sga added
#endif //sga added

  return ::StartDoc(GetHDC(), &DocInfo);
}

//
/// Prepares this device to accept data. The system disables ::ResetDC between calls
/// to StartPage and EndPage, so that applications cannot change the device mode
/// except at page boundaries. If successful, StartPage returns a positive value;
/// otherwise, a negative or zero value is returned.
//
inline int TPrintDC::StartPage()
{
  // Only call if not banding
  return ::StartPage(GetHDC());
}

//
/// Retrieves information about the banding capabilities of this device, and copies
/// it to the given bandInfo structure. 
/// If successful, returns a positive value; otherwise the return value is
/// a system error code. 
//
inline int TPrintDC::BandInfo(TBandInfo& bandInfo)
{
  return Escape(BANDINFO, sizeof(TBandInfo), 0, (LPTSTR)&bandInfo);
}

//
/// Tells this printer's device driver that the application has finished writing to
/// a band. The device driver sends the completed band to the Print Manager and
/// copies the coordinates of the next band in the rectangle specified by rect.
/// If successful, NextBand returns a positive value; otherwise the return value is
/// a system error code. Possible failure values are listed below:
/// - \c \b  SP_ERROR	General error.
/// - \c \b  SP_APPABORT	Job terminated because the application's print-canceling function
/// returned 0.
/// - \c \b  SP_USERABORT	User terminated the job.
/// - \c \b  SP_OUTOFDISK	Insufficient disk space for spooling.
/// - \c \b  SP_OUTOFMEMORY	Insufficient memory for spooling.
//
inline int TPrintDC::NextBand(TRect& rect)
{
  return Escape(NEXTBAND, 0, 0, (void*)&rect);
}

//
/// Tells this printer's device driver that the application has finished writing to
/// a page. If successful, EndPage returns a positive value; otherwise a negative
/// or zero value is returned. Possible failure values are listed below:
/// - \c \b  SP_ERROR	General error.
/// - \c \b  SP_APPABORT	Job terminated because the application's print-canceling function
/// returned 0.
/// - \c \b  SP_USERABORT	User terminated the job.
/// - \c \b  SP_OUTOFDISK	Insufficient disk space for spooling.
/// - \c \b  SP_OUTOFMEMORY	Insufficient memory for spooling.
//
inline int TPrintDC::EndPage()
{
  // Only call if not banding
  RestoreObjects();  // Make sure this TDC stays in sync with HDC
  return ::EndPage(GetHDC());
}

//
/// Aborts the current print job on this printer and erases everything drawn since
/// the last call to StartDoc. AbortDoc calls the user-defined function set with
/// TPrintDC::SetAbortProc to abort a print job because of error or user
/// intervention. TPrintDC::EndDoc should be used to terminate a successfully
/// completed print job.
/// If successful, AbortDoc returns a positive or zero value; otherwise a negative
/// value is returned.
//
inline int TPrintDC::AbortDoc()
{
  return ::AbortDoc(GetHDC());
}

//
/// Ends the current print job on this printer. EndDoc should be called immediately
/// after a successfully completed print job. TPrintDC::AbortDoc should be used to
/// terminate a print job because of error or user intervention.
/// If successful, EndDoc returns a positive or zero value; otherwise a negative
/// value is returned.
//
inline int TPrintDC::EndDoc()
{
  return ::EndDoc(GetHDC());
}


//
/// Returns true if the escape specified by escapeNum is implemented on this device;
/// otherwise false.
//
inline uint TPrintDC::QueryEscSupport(int escapeNum)
{
  return Escape(QUERYESCSUPPORT, sizeof(int), &escapeNum);
}

//
/// Sets requestCount to the number of uncollated copies of each page that this
/// printer should print. The actual number of copies to be printed is copied to
/// actualCount. The actual count will be less than the requested count if the
/// latter exceeds the maximum allowed for this device. SetCopyCount returns 1 if
/// successful; otherwise, it returns 0.
//
inline int TPrintDC::SetCopyCount(int reqestCopies, int& actualCopies)
{
  return Escape(SETCOPYCOUNT, sizeof(int), &reqestCopies, &actualCopies);
}

} // OWL namespace


#endif  // OWL_DC_H
