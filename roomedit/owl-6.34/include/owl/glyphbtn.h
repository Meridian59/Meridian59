//-------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TGlyphButton
//-------------------------------------------------------------------------

#if !defined(OWL_GLYPHBTN_H)
#define OWL_GLYPHBTN_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

// Macro defining class name (usable in resource definition)
//
#if !defined(OWL_GLYPHBTN)
#if defined(UNICODE)
# define OWL_GLYPHBTN  L"OWL_GlyphBtn"
#else
# define OWL_GLYPHBTN  "OWL_GlyphBtn"
#endif
#endif

#if !defined(RC_INVOKED)

#include <owl/button.h>
#include <owl/uihelper.h>
#include <owl/bitset.h>
#include <owl/color.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TFont;
class _OWLCLASS TCelArray;

/// \addtogroup ctrl
/// @{
/// \class TBtnBitmap
// ~~~~~ ~~~~~~~~~~
/// TBtnBitmap is an enhanced version of TBitmap with the ability to update the face
/// color of a bitmap to match the current system BTNFACE color.
//
class _OWLCLASS TBtnBitmap : public TBitmap {
  public:
    TBtnBitmap(HINSTANCE hInstance, TResId resId,
               const TColor& faceColor = DefaultFaceColor);
    TBtnBitmap(HBITMAP hBitmap,
               const TColor& faceColor = DefaultFaceColor,
               TAutoDelete autoDelete = NoAutoDelete);

    void        UpdateFaceColor();
    static void MapColor(TDC& dc, TDC& srcDC, const TColor& toColor,
                         const TColor& fromColor, int width, int height);

    /// By default, we'll assume that bitmaps were designed with a gray background
    //
    static const TColor DefaultFaceColor;

  protected:
    TColor      FaceColor;    ///< Current face color of bitmap
};

//
/// \class TGlyphButton
// ~~~~~ ~~~~~~~~~~~~
/// TGlyphButton encapsulates a control which is capable of displaying a bitmap (or
/// glyph) and/or text.
//
class _OWLCLASS TGlyphButton : public TButton  {
  public:
    enum TBtnType { 
      btOk,         ///< 'Ok' button
      btCancel,     ///< 'Cancel' button
      btYes,        ///< 'Yes' button
      btNo,         ///< 'No' button
      btClose,      ///< 'Close' button
      btAbort,      ///< 'Abort' button
      btIgnore,     ///< 'Ignore' button
      btHelp,       ///< 'Help' button
      btApply,      ///< 'Apply' button
      btBrowse,     ///< 'Browse' button
      btRevert,     ///< 'Revert' button
      btAdd,        ///< 'Add' button
      btDelete,     ///< 'Delete' button
      btEdit,       ///< 'Edit' button
      btKey,        ///< 'Key' button
      btSetup,      ///< 'Setup' button
      btCustom,     ///< Custom button
    };
  public:
    TGlyphButton(TWindow*        parent,
                 int             id,
                 LPCTSTR         text,
                 int X, int Y, int W, int H,
                 TBtnType        type = btCustom,
                 bool            isDefault = false,
                 TModule*        module = 0);

    TGlyphButton(
      TWindow* parent,
      int id,
      const tstring& text,
      int X, int Y, int W, int H,
      TBtnType type = btCustom,
      bool isDefault = false,
      TModule* module = 0);

    TGlyphButton(TWindow* parent, int resourceId= 0, TBtnType type = btCustom, TModule* module = 0);
   ~TGlyphButton();

    /// Enumeration describing the state when a particular bitmap should
    /// be displayed. Used with the 'SetGlyph' method.
    /// \note If only a single 'up' bitmap is specified, TGlyphBtn will
    ///       automatically generate the bitmaps for the other states.
    //
    enum TGlyphType {
      gtUp       = TUIFace::Normal,   ///< Bitmap for when button is up
      gtDisabled = TUIFace::Disabled, ///< Bitmap for when button is disabled
      gtDown,                         ///< Bitmap for when button is depressed
      gtCellArray,
    };

    /// \name Methods for specifying the glyphs that should be used
    /// @{

    //
    /// Note that TColor::Sys3dFace is the 'face color' used to denote transparency.
    /// If you want a different color to denote transparency, you must map the colors of the 
    /// bitmap before passing it. TBtnBitmap is made for this purpose.
    //
    void SetGlyph(TBitmap* bitmap, TGlyphType = gtUp, TAutoDelete autoDelete = AutoDelete);

    //
    /// Note that TBtnBitmap::DefaultFaceColor is used to denote transparency.
    /// If you want a different color to denote transparency, then create a TBtnBitmap and use
    /// the SetGlyph (TBitmap*, ...) overload instead.
    //
    void SetGlyph(TResId resId, TModule* module = 0, TGlyphType = gtUp);

    //
    /// Note that TBtnBitmap::DefaultFaceColor is used to denote transparency.
    /// If you want a different color to denote transparency, then create a TBtnBitmap and use
    /// the SetGlyph (TBitmap*, ...) overload instead.
    //
    void SetGlyph(HBITMAP hBitmap, TGlyphType = gtUp, TAutoDelete autoDelete = NoAutoDelete);

    /// @}

    /// Enumeration describing how to position the text and glyph
    /// of the button.
    //
    enum  TLayoutStyle {
      lsNone,                   ///< Use specified coordinates

      // Horizontal layouts [center vertically]
      //
      lsH_SGST,                 ///< Space Glyph Space Text (Horizontal)
      lsH_GST,                  ///< Glyph Space Text (Horizontal)
      lsH_STSG,                 ///< Space Text  Space Glyph (Horizontal)
      lsH_TSGS,                 ///< Text  Space Glyph Space (Horizontal)

      // Vertical layouts [center horizontally]
      lsV_SGST,                 ///< Space Glyph Space Text (Vertical)
      lsV_GST,                  ///< Glyph Space Text (Vertical)
      lsV_STSG,                 ///< Space Text  Space Glyph (Vertical)
      lsV_TSGS,                 ///< Text  Space Glyph Space (Vertical)
    };

    // Methods to specify how the glyph and/or text should be laid out
    //
    void      SetLayoutStyle(TLayoutStyle style);
    void      SetTextOrigin(int x, int y);
    void      SetGlyphOrigin(int x, int y);

  protected:
    // Overriden TWindow virtuals
    //
    void      SetupWindow();
    void      Paint(TDC& dc, bool erase, TRect& rect);
    void      GetWindowClass(WNDCLASS& wndClass);
    virtual TGetClassNameReturnType GetClassName();

    // Routine which determines location of text and glyph
    //
    virtual void LayoutTextGlyph(const TRect& faceRect, TRect& textRect,
                                 TRect& glyphRect);
    // Initialization routine
    //
    void      InitVars();

    /// Init glip
    //
    void      InitGlyp(TBtnType type);

    /// Enumeration describing the current state of the button.
    //
    enum  TButtonInfo {
      biPushed     = 0x0001,      ///< Button is currently depressed
      biFocus      = 0x0002,      ///< Button has focus
      biDefault    = 0x0004,      ///< Button is a 'DefaultPushButton'
      biDisabled   = 0x0008,      ///< Button is disabled
      biShowText   = 0x0010,      ///< Button should display its caption
      biShowGlyph  = 0x0020,      ///< Button should draw its glyph
      biFlatStyle = 0x0040,      ///< Button draws border in flat style
      biGtUp       = 0x0080,      ///< Was Set Up Glyph
      biGtDown    = 0x0100,      ///< Was Set Down Glyph
      biGtDisabled= 0x0200,      ///< Was Set Disabled Glyph
      biHot       = 0x0400,      // Mouse over the button
    };

    // Set, clear, query flags about the state of the control
    //
    int       Clear(int t);
    int       Set(int t);
    bool      IsSet(int t) const;

    // Event handlers
    //
    void        EvPaint();
    bool        EvEraseBkgnd(HDC);
    void        EvSetFocus(THandle hWndLostFocus);
    void        EvKillFocus(THandle hWndGetFocus);
    HFONT       EvGetFont();
    void        EvSetFont(HFONT hFont, bool redraw);
    uint        EvGetDlgCode(const MSG* msg);
    void        EvLButtonDown(uint modKeys, const TPoint& point);
    void        EvLButtonDblClk(uint modKeys, const TPoint& point);
    void        EvLButtonUp(uint modKeys, const TPoint& point);
    void        EvMouseMove(uint modKeys, const TPoint& point);
    void        EvKeyDown(uint key, uint repeatCount, uint flags);
    void        EvKeyUp(uint key, uint repeatCount, uint flags);
    void        EvEnable(bool enabled);
    void        EvCancelMode();
    void         EvSysColorChange();
    void EvMouseLeave();
    TResult     BmGetState(TParam1, TParam2);
    TResult     BmSetState(TParam1, TParam2);
    TResult     BmSetStyle(TParam1, TParam2);

    // Helper functions
    //
    void          PaintNow();
    void          PaintButton(TDC& dc);
    virtual void  PaintFrame(TDC& dc, TRect& rect);
    virtual void   PaintDefaultRect(TDC& dc, TRect& rect);
    virtual void  PaintFace(TDC& dc, const TRect& rect);
    virtual void  PaintFocusRect(TDC& dc, const TRect& faceRect);
    void          ClearCapture();
    virtual void  BuildCelArray(const TBitmap* cell);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TGlyphButton(const TGlyphButton&);
    TGlyphButton& operator =(const TGlyphButton&);

    TCelArray*      CelArray;       ///< CelArray used to cache glyph states
    TFont*          BtnFont;        ///< Pointer to font used by button
    int             xGlyph;         ///< x (left) coordinate of glyph
    int             yGlyph;         ///< y (top ) coordinate of glyph
    int             xText;          ///< x (left) coordinate of text
    int             yText;          ///< y (top ) coordinate of text
    TLayoutStyle    LayStyle;       ///< How text & glyph should be laid out
    TBtnType        BtnType;        ///< Button type
    TBitFlags<uint> Flags;          ///< Status flags

  DECLARE_RESPONSE_TABLE(TGlyphButton);
};
/// @}
// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Clear flags about the state of the control
//
inline int
TGlyphButton::Clear(int t)
{
  return Flags.Clear(t);
}

//
/// Set flags about the state of the control
//
inline int
TGlyphButton::Set(int t)
{
  return Flags.Set(t);
}

//
/// Query flags about the state of the control
//
inline bool
TGlyphButton::IsSet(int t) const
{
  return Flags.IsSet(t);
}

} // OWL namespace

#endif  // !RC_INVOKED


#endif  // OWL_GLYPHBTN_H
