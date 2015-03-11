//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of text gadget class TGadget.
/// Definition for TDynamicTextGadget.
//----------------------------------------------------------------------------

#if !defined(OWL_TEXTGADG_H)
#define OWL_TEXTGADG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gadgetwi.h>
#include <owl/gadget.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TTextGadget
// ~~~~~ ~~~~~~~~~~~
/// Derived from TGadget, TTextGadget is a text gadget object. 
///
/// When constructing the text gadget specify how many characters you want
/// room for and how the text should be aligned horizontally.
///
/// The inner bounds are computed by multiplying the number of characters by
/// the maximum character width.
//
class _OWLCLASS TTextGadget : public TGadget {
  public:
  	
/// Enumerates the text-alignment attributes.
    enum TAlign 
    {
    	Left, 	///< Aligns the text at the left edge of the bounding rectangle.
    	Center, ///< Aligns the text horizontally at the center of the bounding rectangle.
    	Right		///< Aligns the text at the right edge of the bounding rectangle.
    };

    TTextGadget(int id = 0, TBorderStyle = Recessed, TAlign = Left,
                uint numChars = 10, LPCTSTR text = 0,
                TFont* font = 0 /*new TGadgetWindowFont*/);

    TTextGadget(
      int id, 
      TBorderStyle, 
      TAlign,
      uint numChars, 
      const tstring& text,
      TFont* font = 0);

    ~TTextGadget();

    // Return a copy of gadget's text
    //
    LPCTSTR   GetText() const;

    // Makes a copy of the text
    //
    void      SetText(LPCTSTR text);
    void SetText(const tstring& text) {SetText(text.c_str());}

    // Added 6/22/98 to allow setting/unsetting of text/background color
    void SetBkgndColor(TColor& color);
    void SetTextColor(TColor& color);
    void ResetBkgndColor();
    void ResetTextColor();

  protected:
    // Override virtual methods defined in TGadget
    //
    void            Paint(TDC& dc);
    virtual TColor   GetEnabledColor() const;
    void            GetDesiredSize(TSize &size);
    void            Invalidate();

  // Data members -- will become private
  //
  protected_data:
    LPTSTR    Text;       ///< new'd copy of the text for this gadget
    uint      TextLen;    ///< strlen of the above text
    TAlign    Align;      ///< Alignment: left, center or right
    uint      NumChars;   ///< Number of chars to reserve space for
    TFont*    Font;       ///< display font
    TColor    BkgndColor; ///< background color of the gadget
    TColor    TextColor;  ///< text color of the gadget

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TTextGadget(const TTextGadget&);
    TTextGadget& operator =(const TTextGadget&);
};

//
/// \class TDynamicTextGadget
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Small class will send EvEnable to window, 
/// You can set text by handling EvCommandEnable message
//
class _OWLCLASS TDynamicTextGadget: public TTextGadget{
  public:
    TDynamicTextGadget(int id = 0, 
                       TBorderStyle = Recessed, 
                       TAlign = Left,
                       uint numChars = 10, 
                       LPCTSTR text = 0,
                       TFont* font = 0);

    TDynamicTextGadget(
      int id, 
      TBorderStyle, 
      TAlign,
      uint numChars, 
      const tstring& text,
      TFont* = 0);

    public:
      virtual void CommandEnable();
}; 

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


// -----------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the text for the gadget.
inline LPCTSTR TTextGadget::GetText() const {
  return Text;
}

//
inline TColor TTextGadget::GetEnabledColor() const {
  return TColor::SysBtnText;
}

//------------------------------------------------------------------------------

} // OWL namespace


#endif  // OWL_TEXTGADG_H
