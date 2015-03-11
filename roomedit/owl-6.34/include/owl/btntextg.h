//----------------------------------------------------------------------------//
// ObjectWindows 1998 Copyright by Yura Bidus                                 //
//                                                                            //
// Used code and ideas from Dieter Windau and Joseph Parrello                 //
//                                                                            //
//                                                                            //
// THIS CLASS AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF         //
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO        //
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A             //
// PARTICULAR PURPOSE.                                                        //
/// \file                                                                            //
/// Definition of classes TButtonTextGadget.                                   //
//----------------------------------------------------------------------------//

#if !defined(OWL_BTNTEXTG_H)
#define OWL_BTNTEXTG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/buttonga.h>

namespace owl {

#include <owl/preclass.h>

//
/// \addtogroup gadgets
/// @{
/// \class TButtonTextGadget
/// Derived from TButtonGadget, TButtonTextGadget represents buttons with bitmap and
/// text, or with text only, or with bitmap only, that you can click on or off. You
/// can also apply attributes such as color, style, and shape (notched or unnotched)
/// to your button-text gadgets.
/// 
/// In addition to TButtonGadget, TButtonTextGadget has several types to deal with text:
/// 
/// - \c \b  TAlign        Defines alignment for text (aLeft, aCenter, aRight),
/// - \c \b  TStyle        Defines style of gadget (sBitmap, sText, sBitmapText).
/// - \c \b  TLayoutStyle  Defines Layout Bitmap with text(lTextLeft,lTextTop,lTextRight,lTextBottom).
//
class _OWLCLASS TButtonTextGadget : public TButtonGadget
{
  public:

    //
    /// Enumerates the text-alignment attributes.
    //
    enum TAlign 
    {
      aLeft,   ///< Aligns the text at the left edge of the bounding rectangle.
      aCenter, ///< Aligns the text horizontally at the center of the bounding rectangle.
      aRight   ///< Aligns the text at the right edge of the bounding rectangle.
    };
    
    //
    /// TStyle contains values that defines how gadget will be displayed:
    //
    enum TStyle 
    {
      sBitmap=0x001, ///< Only the bitmap is displayed.
      sText=0x002, ///< Only text is displayed.
      sBitmapText=0x003	///< Both text and bitmap are displayed.
    };
    
    //
    /// TLayoutStyle contains values that defines how bitmap and text will be layout.
    //
    enum TLayoutStyle 
    {
    	lTextLeft, ///< Text left, bitmap right.
    	lTextTop, ///< Text top, bitmap bottom.
    	lTextRight, ///< Text right, bitmap left.
    	lTextBottom	///< Text bottom, bitmap top.
    };

    TButtonTextGadget(
      int id,
      TResId glyphResIdOrIndex,
      TStyle style = sBitmapText,
      TType type = Command,
      bool enabled = false,
      TState state = Up,
      bool sharedGlyph = false,
      uint numChars = 4);

    virtual ~TButtonTextGadget();

    LPCTSTR GetText() const;
    void SetText(const tstring& text, bool repaint=true);
    void SetText(LPCTSTR s, bool repaint = true) {SetText(owl::tstring(s ? s : _T("")), repaint);}
    TStyle GetStyle() const;
    void SetStyle(const TStyle style, bool   repaint=true);
    TAlign GetAlign() const;
    void SetAlign(const TAlign align, bool repaint=true);
    TLayoutStyle GetLayoutStyle() const;
    void SetLayoutStyle(const TLayoutStyle style, bool repaint=true);

    TFont* GetFont()  const;
    void SetFont(TFont* font, bool repaint = true);

    //
    // Override virtual methods defined in TGadget
    //
    virtual void GetDesiredSize(TSize &size);
    virtual void SetBounds(const TRect& rect);

    //
    // Override and initiate a WM_COMMAND_ENABLE message
    //
    virtual void  CommandEnable();

  protected:
    virtual void Paint(TDC& dc);
    virtual void Created();
    virtual void Layout(TRect& srcRect, TRect& textRect, TRect& btnRect);
    virtual void PaintText(TDC& dc, TRect& rect, const tstring& text);
    virtual void SysColorChange();

    void GetTextSize(TSize& size);

  //
  // Data members -- will become private
  //
  protected_data:
    tstring Text;
    uint NumChars; ///< Number of chars to reserve space for
    TAlign Align; ///< Alignment: left, center or right
    TStyle Style; ///< Style Bitmap, Text, Bitmap and Text
    TLayoutStyle LayoutStyle;///< Layout style
    TFont* Font; ///< The display font; if Font == 0, it will try to get the font from TGadgetWindow.

  private:
    //
    // Hidden to prevent accidental copying or assignment
    //
    TButtonTextGadget(const TButtonTextGadget&);
    TButtonTextGadget& operator =(const TButtonTextGadget&);
};

/// @}

#include <owl/posclass.h>

//
// --------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the Style for the gadget.
//
inline TButtonTextGadget::TStyle TButtonTextGadget::GetStyle() const{
  return Style;
}

//
/// Returns the text for the gadget.
//
inline LPCTSTR TButtonTextGadget::GetText() const {
  return Text.c_str();
}

//
/// Returns the Font used by gadget (or NULL if it doesn't exist).
//
inline TFont* TButtonTextGadget::GetFont() const {
  return Font;
}

//
/// Returns the Align for the gadget.
//
inline TButtonTextGadget::TAlign TButtonTextGadget::GetAlign() const {
  return Align;
}

//
/// Returns the LayoutStyle for the gadget.
//
inline TButtonTextGadget::TLayoutStyle TButtonTextGadget::GetLayoutStyle() const{
  return LayoutStyle;
}

} // OWL namespace

#endif  // OWL_BTNTEXTG_H
