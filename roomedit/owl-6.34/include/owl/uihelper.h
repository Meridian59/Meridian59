//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of the UI Helper Classes: TUIHandle, TUIBorder, TUIFace, TUIPart
//----------------------------------------------------------------------------

#if !defined(OWL_UIHELPER_H)
#define OWL_UIHELPER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/eventhan.h>
#include <owl/wsyscls.h>
#include <owl/color.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS  TDC;
class _OWLCLASS  TBrush;
class _OWLCLASS  THatch8x8Brush;
class _OWLCLASS  TBitmap;
class _OWLCLASS  TIcon;
class _OWLCLASS  TColor;

//
// Flags of DrawFrameControl API
//
#if !defined(DFC_CAPTION)
# define DFC_CAPTION             1
#endif
#if !defined(DFC_MENU)
# define DFC_MENU                2
#endif
#if !defined(DFC_SCROLL)
# define DFC_SCROLL              3
#endif
#if !defined(DFC_BUTTON)
# define DFC_BUTTON              4
#endif
#if !defined(DFCS_CAPTIONCLOSE)
# define DFCS_CAPTIONCLOSE       0x0000
#endif
#if !defined(DFCS_CAPTIONMIN)
# define DFCS_CAPTIONMIN         0x0001
#endif
#if !defined(DFCS_CAPTIONMAX)
# define DFCS_CAPTIONMAX         0x0002
#endif
#if !defined(DFCS_CAPTIONRESTORE)
# define DFCS_CAPTIONRESTORE     0x0003
#endif
#if !defined(DFCS_CAPTIONHELP)
# define DFCS_CAPTIONHELP        0x0004
#endif
#if !defined(DFCS_MENUARROW)
# define DFCS_MENUARROW          0x0000
#endif
#if !defined(DFCS_MENUCHECK)
# define DFCS_MENUCHECK          0x0001
#endif
#if !defined(DFCS_MENUBULLET)
# define DFCS_MENUBULLET         0x0002
#endif
#if !defined(DFCS_MENUARROWRIGHT)
# define DFCS_MENUARROWRIGHT     0x0004
#endif
#if !defined(DFCS_SCROLLUP)
# define DFCS_SCROLLUP           0x0000
#endif
#if !defined(DFCS_SCROLLDOWN)
# define DFCS_SCROLLDOWN         0x0001
#endif
#if !defined(DFCS_SCROLLLEFT)
# define DFCS_SCROLLLEFT         0x0002
#endif
#if !defined(DFCS_SCROLLRIGHT)
# define DFCS_SCROLLRIGHT        0x0003
#endif
#if !defined(DFCS_SCROLLCOMBOBOX)
# define DFCS_SCROLLCOMBOBOX     0x0005
#endif
#if !defined(DFCS_SCROLLSIZEGRIP)
# define DFCS_SCROLLSIZEGRIP     0x0008
#endif
#if !defined(DFCS_SCROLLSIZEGRIPRIGHT)
# define DFCS_SCROLLSIZEGRIPRIGHT 0x0010
#endif
#if !defined(DFCS_BUTTONCHECK)
# define DFCS_BUTTONCHECK        0x0000
#endif
#if !defined(DFCS_BUTTONRADIOIMAGE)
# define DFCS_BUTTONRADIOIMAGE   0x0001
#endif
#if !defined(DFCS_BUTTONRADIOMASK)
# define DFCS_BUTTONRADIOMASK    0x0002
#endif
#if !defined(DFCS_BUTTONRADIO)
# define DFCS_BUTTONRADIO        0x0004
#endif
#if !defined(DFCS_BUTTON3STATE)
# define DFCS_BUTTON3STATE       0x0008
#endif
#if !defined(DFCS_BUTTONPUSH)
# define DFCS_BUTTONPUSH         0x0010
#endif
#if !defined(DFCS_INACTIVE)
# define DFCS_INACTIVE           0x0100
#endif
#if !defined(DFCS_PUSHED)
# define DFCS_PUSHED             0x0200
#endif
#if !defined(DFCS_CHECKED)
# define DFCS_CHECKED            0x0400
#endif
#if !defined(DFCS_ADJUSTRECT)
# define DFCS_ADJUSTRECT         0x2000
#endif
#if !defined(DFCS_FLAT)
# define DFCS_FLAT               0x4000
#endif
#if !defined(DFCS_MONO)
# define DFCS_MONO               0x8000
#endif


//
/// \class TUIHandle
// ~~~~~ ~~~~~~~~~
/// Assists in drawing user manupulators for on-screen objects
//
/// TUIHandle manages and draws various kinds of UI handles, including hatched
/// border handles, and resizing grapples (small squares that appear along the
/// edges) on a rectangle. You can use this class to create a hatched border that
/// encloses various kinds of drawing objects you want to manipulate.
/// 
/// With the help of this class, you can create an application that lets you
/// - Resize the shape of the rectangle by pointing to and grabbing one
/// of the grapples on the border
/// - Move the entire rectangle by clicking in the middle of the
/// rectangle
/// 
/// Although by default a hatched border with eight grapples is created, you can
/// control whether grapples appear. In addition, you can vary the pattern of the
/// border by drawing a dashed frame enclosing a rectangle or a rectangle filled
/// with hatch marks. TuiHandle uses THatch8x8Brush to draw the hatched border.
/// 
/// TUIHandle uses the enum TWhere to return the area where the user points and
/// clicks the mouse (referred to as a hit area).
/// 
/// The following diagram displays a UI handle and identifies several small square
/// grapples where hit testing occurs.
/// \image html bm267.BMP
/// 
/// The following code fragment sets up a hatched border and UI grapples for an OLE
/// 2 container application:
/// \code
/// // Do the default rectangle painting.
///   .
///   .
///   .
/// TRect r //...; Insert your rectangle drawing code here.
/// 
/// // Draw 8 grapples with a border on top of the object.
/// TUIHandle handle(r, TUIHandle::HandlesIn|
///                  TUIHandle::Grapples|
///                  TUIHandle::HatchBorder, 5);
/// handle.Paint(dc);
/// // Insert your code here....
///   .
///   .
///   .
/// // Draw a hatched border.
/// TUIHandle handle(r, TUIHandle::HatchBorder, 5);
/// handle.Paint(dc);
/// \endcode
//
class _OWLCLASS TUIHandle {
  public:

    /// Enumeration describing the location of a grapple selection.
    /// These constants typically refer to the [mouse] hittest location
    /// which caused the grapples to response to a move or resize operation.
    //
		/// The InsideSpecial designation refers to the area inside the rectangle when the
		/// hit area needs to be treated specially (for example, because it might contain
		/// text or graphics). Normally, if the area inside the rectangle is hit, it means
		/// that user wants to move the rectangle. However, if there is text inside the
		/// rectangle, the user might click on this area in order to enter text. This latter
		/// situation is referred to as an inside special case.
		/// 
		/// The hit area (Where) can be converted to a row and a column by using the
		/// following equations:
		/// - Row = Where / 3
		/// - Column = Where mod 3
		/// 
		/// The value of Where ranges from 0 (TopLeft) to 8 (BottomRight) and corresponds to
		/// the following areas of a rectangle:
		/// \image html bm268.BMP
		/// 
		/// You can then use these values to calculate the movement of the object and to
		/// resize the object.
		//
    enum TWhere {
      TopLeft     = 0,   ///< Upper left corner
      TopCenter   = 1,   ///< Middle of upper edge
      TopRight    = 2,   ///< Upper right corner
      MidLeft     = 3,   ///< Middle of left edge
      MidCenter   = 4,   ///< Somewhere inside the rectangle
      MidRight    = 5,   ///< Middle of right edge
      BottomLeft  = 6,   ///< Lower left corner
      BottomCenter= 7,   ///< Middle of lower edge
      BottomRight = 8,   ///< Lower right corner
      Outside     =-1,   ///< Hit completely outside the object
      Inside      =-2    ///< Hit inside object, not on handles, & InsideSpecial is set
    };

    /// Enumeration describing the type of grapples to be drawn
    //
		/// If a hatched border with grapples is drawn inside a rectangle, it sits within
		/// the borders of the outer frame of the rectangle. If a hatched border with
		/// grapples is drawn outside the rectangle, it is drawn outside the boundary of the
		/// rectangle's frame. In the latter case, the function GetBoundingRect() returns a
		/// larger rectangle.
    enum TStyle {
      HandlesIn     = 0,    ///< Handles on the inside of the rectange
      HandlesOut    = 1,    ///< Handles on the outside of the rectangle
      Framed        = 2,    ///< Frame rect is drawn solid
      DashFramed    = 4,    ///< Frame rect is drawn dashed
      Grapples      = 8,    ///< Eight grapple boxes are drawn
      HatchBorder   = 16,   ///< Border handle area drawn hatched
      HatchRect     = 32,   ///< Whole rect drawn hatched
      InsideSpecial = 64,   ///< Inside area hit-tested independently
    };
    TUIHandle(const TRect& frame,
              uint style = HandlesIn|Grapples|HatchBorder,
              int thickness = 5);
              // !CQ Use TSysMetric::CxEdge, CxBorder...

    void   MoveTo(int x, int y);
    void   Move(int dx, int dy);
    void   Size(int w, int h);
    TRect  GetBoundingRect() const;

    TWhere HitTest(const TPoint& point) const;
    void   Paint(TDC& dc) const;

    static uint16 GetCursorId(TWhere where);

  private:
    TRect  Frame;
    uint   Style;
    TPoint HandleBox;
};

//
/// \class TUIBorder
// ~~~~~ ~~~~~~~~~
/// Assists in drawing borders of many styles.
/// Uses win4.0 calls when available
//
class _OWLCLASS TUIBorder {
  public:
    /// Constructor flags to limit or modify border
    //
    enum TFlag {
      // Which edge(s) to draw. ctor defaults to all 4
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
      Mono        = 0x8000,  ///< Monochrome
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
      ButtonDn3x,       ///<  Button in down position, Win 3.x style
    };
    TUIBorder(const TRect& frame, TStyle style, uint flags = 0);

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
      EdgeBump   =  RaisedOuter | SunkenInner,   ///< Outer raised, inner sunken
    };
    TUIBorder(const TRect& frame, TEdge edge, uint flags = 0);

    void   MoveTo(int x, int y);
    void   Move(int dx, int dy);
    void   Size(int w, int h);
    TRect  GetBoundingRect() const;
    TRect  GetClientRect() const;

    void   Paint(TDC& dc) const;

    // Wrapper / Emulation of Win4.0 API call
    //
    static bool DrawEdge(TDC& dc, const TRect& frame, uint edge, uint flags);

    // Paint a 2-color frame. With and w/o bevel corner color specified
    //
    static void PaintFrame(TDC& dc, const TRect& frame, uint flags,
                           const TColor& tlColor, const TColor& brColor);
    static void PaintFrameC(TDC& dc, const TRect& frame, uint flags,
                            const TColor& tlColor, const TColor& brColor,
                            const TColor& bcColor);

    // Paint a custom frame
    //
//    static void PaintWT(TDC& dc, const TRect& frame);  // WindowText color
//    static void Paint3H(TDC& dc, const TRect& frame);  // 3dHighlight color

  protected:
  private:
    TRect  Frame;
//    uint   Style;
    uint   Edge;
    uint   Flags;
};

//
/// \class TUIFace
// ~~~~~ ~~~~~~~
/// TUIFace assists in painting UI elements in various states.
//
class _OWLCLASS TUIFace {
  public:

    /// Enumeration describing the state of the bitmap to be drawn
    //
    enum TState {
      Normal,     ///< Normal state
      Down,       ///< Down or Option set state (hilit background, +1,+1)
      Indeterm,   ///< Indeterminant, or mixed-value state
      Disabled,   ///< Disabled or Unavailable state (embossed 3d no color)
      Default,    ///< Default btn state (bold)
    };

    TUIFace(const TRect& faceRect, const TBitmap& bm,
            const TColor& bgndColor = TColor::Sys3dFace,
            const TColor& fillColor = TColor::Sys3dFace);
    TUIFace(const TRect& faceRect, const tstring& text,
            const TColor& fillColor = TColor::Sys3dFace, uint format=0);
    TUIFace(const TRect& faceRect, const TIcon& icon,
            const TColor& bgndColor = TColor::Sys3dFace,
            const TColor& fillColor = TColor::Sys3dFace);

    void  Paint(TDC& dc, const TPoint& dstPt, TState state, bool pressed,
                bool fillFace = true);
    void  Paint(TDC& dc, const TRect& srcRect, const TPoint& dstPt,
                TState state, bool pressed, bool fillFace = true);
    void  PaintMask(TDC& dc, const TPoint& pt);

    // Wrapper / Emulation of Win4.0 API call [::DrawState]
    //
    static bool Draw(TDC& dc, HBRUSH hbr, LPARAM lp, WPARAM wp,
                     int x, int y, int cx, int cy, uint flags);

    // Internal static helper functions that can be used alone
    //
    static void   PaintBm(const TBitmap& glyph, const TRect& glyphSrcRect,
                          TDC& dc, const TRect& faceRect, const TPoint& dstPt,
                          TState state, bool pressed, bool fillFace,
                          const TColor& maskColor, const TColor& fillColor);
    static void   PaintIcon(const TIcon& icon, const TRect& glyphSrcRect,
                          TDC& dc, const TRect& faceRect, const TPoint& dstPt,
                          TState state, bool pressed, bool fillFace,
                          const TColor& maskColor, const TColor& fillColor);
    static void   PaintText(const tstring& text, TDC& dc, const TRect& faceRect,
                            const TPoint& dstPt, TState state, bool pressed,
                            bool fillFace, const TColor& fillColor, uint16 format);

    static void   PaintMaskBm(const TBitmap& glyph, TDC& dc, const TPoint& pt,
                              const TColor& maskColor);

    static void   BuildMask(TDC& maskDC, const TPoint& maskDst, const TSize& maskSize,
                            TDC& glyphDC, const TRect& glyphRect,
                            const TColor& maskColor);
    static void   FillBackground(TDC& dc, TDC& maskDC,
                                 const TRect& dstRect, const TBrush& brush);
    static void   DitherBackground(TDC& dc, TDC& maskDC, const TRect& dstRect);
    static int    DrawTextEtched(TDC& dc, const tstring& str, int count,
                                 const TRect& rect, uint16 format);

    static THatch8x8Brush& GetDitherBrush();
    static TBitmap&        GetGlyphMask(const TSize& minSize);

  private:
    enum TSource {sText, sBitmap, sIcon};
    TSource SourceType;
    tstring Text;
    const TBitmap*  Bm;
    const TIcon* Icon;

    const TRect Rect;
    const TColor BkgndColor;
    const TColor FillColor;
    uint Format;
};


//
/// \class TUIPart
// ~~~~~ ~~~~~~~
/// Encapsulates the DrawFrameControl 32-bit API.
//
class _OWLCLASS TUIPart {
  public:

    /// Enumeration describing the part to be drawn.
    /// \note TType describes the part, TState then refines the exact
    ///       glyph of the selected part.
    //
    enum  TType {
      uiButton  = DFC_BUTTON,     ///< Draw a button glyph
      uiCaption = DFC_CAPTION,    ///< Draw a caption glyph
      uiMenu    = DFC_MENU,       ///< Draw a menu glyph
      uiScroll  = DFC_SCROLL      ///< Draw a scroll bar glyph
    };

    /// Enumeration describing the various glyphs available for buttons,
    /// captions, menus and scrollbar parts.
    //
    enum  TState {
      Button3State      = DFCS_BUTTON3STATE,     ///< Three-state button
      ButtonCheck       = DFCS_BUTTONCHECK,      ///< Check box
      ButtonPush        = DFCS_BUTTONPUSH,       ///< Push button
      ButtonRadio       = DFCS_BUTTONRADIO,      ///< Radio button
      ButtonRadioImage  = DFCS_BUTTONRADIOIMAGE, ///< Image for radio button (nonsquare needs image)
      ButtonRadioMask   = DFCS_BUTTONRADIOMASK,  ///< Mask for radio button (nonsquare needs mask)
      Checked           = DFCS_CHECKED,          ///< Draw button as checked
      Flat              = DFCS_FLAT,             ///< Draw button with flat border
      Inactive          = DFCS_INACTIVE,         ///< Draw button grayed
      Mono              = DFCS_MONO,             ///< Draw button with monochrome border
      Pushed            = DFCS_PUSHED,           ///< Draw button pushed
      CaptionClose      = DFCS_CAPTIONCLOSE,     ///< Close button
      CaptionHelp       = DFCS_CAPTIONHELP,      ///< Help button
      CaptionMax        = DFCS_CAPTIONMAX,       ///< Maximize button
      CaptionMin        = DFCS_CAPTIONMIN,       ///< Minimize button
      CaptionRestore    = DFCS_CAPTIONRESTORE,   ///< Restore button
      MenuArrow         = DFCS_MENUARROW,        ///< Submenu arrow
      MenuBullet        = DFCS_MENUBULLET,       ///< Bullet
      MenuCheck         = DFCS_MENUCHECK,        ///< Check mark
      ScrollCombo       = DFCS_SCROLLCOMBOBOX,   ///< Combo box scroll bar
      ScrollDown        = DFCS_SCROLLDOWN,       ///< Down arrow of scroll bar
      ScrollLeft        = DFCS_SCROLLLEFT,       ///< Left arrow of scroll bar
      ScrollRight       = DFCS_SCROLLRIGHT,      ///< Right arrow of scroll bar
      ScrollSizeGrip    = DFCS_SCROLLSIZEGRIP,   ///< Size grip in bottom-right corner of window
      ScrollUp          = DFCS_SCROLLUP          ///< Up arrow of scroll bar
    };

    TUIPart();
    bool Paint(TDC& dc, TRect& rect, TType type, TState state);
    bool DrawFrameControl(TDC& dc, TRect& rect, TType type, TState state);

  protected:
};


//
/// \class TUIHelper
// ~~~~~ ~~~~~~~~~
/// Wrapper for misc. Win4 USER functions.
//
class _OWLCLASS TUIHelper {
  public:
    static HANDLE LoadImage(HINSTANCE hI, LPCTSTR, uint, int, int, uint);
    static bool DrawState(HDC, HBRUSH, DRAWSTATEPROC, TParam2, TParam1, int, int, int, int, UINT);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Initializes the button face with a bitmap.
//
inline
TUIFace::TUIFace(const TRect& faceRect, const TBitmap& bm,
                 const TColor& bkgndColor, const TColor& fillColor)
:
  Bm(&bm),SourceType(sBitmap),Rect(faceRect),BkgndColor(bkgndColor),
  FillColor(fillColor),Format(0)
{
}

//
/// Initializes the button face with a text string.
//
inline
TUIFace::TUIFace(const TRect& faceRect, const tstring& text,
                 const TColor& fillColor, uint format)
:
  Text(text), SourceType(sText),Rect(faceRect), BkgndColor(TColor::None),
  FillColor(fillColor),Format(format)
{
}

//
/// Initializes the button face with an icon.
//
inline
TUIFace::TUIFace(const TRect& faceRect, const TIcon& icon,
                 const TColor& /*bkgndColor*/, const TColor& fillColor)
:
  Icon(&icon), SourceType(sIcon), Rect(faceRect), 
  BkgndColor(TColor::None), FillColor(fillColor),Format(0)
{
}

} // OWL namespace


#endif  // OWL_UIHELPER_H
