//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TMessageBar.
//----------------------------------------------------------------------------

#if !defined(OWL_MESSAGEB_H)
#define OWL_MESSAGEB_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gadgetwi.h>

namespace owl {


// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

const int IDW_STATUSBAR     = 32040;  // Window ID used to locate status bars

/// \addtogroup gadgets
/// @{
/// \class TMessageBar
// ~~~~~ ~~~~~~~~~~~
/// Derived from TGadgetWindow, TMessageBar implements a message bar with one text
/// gadget as wide as the window and no border. Normally positioned at the bottom of
/// the window, the message bar uses the default gadget window font and draws a
/// highlighted line at the top.
//
class _OWLCLASS TMessageBar : public TGadgetWindow {
  public:
    TMessageBar(TWindow*   parent = 0,
                TFont*     font = new TGadgetWindowFont,
                TModule*   module = 0);

    // Set the text for the default text message gadget or a text gadget by id
    //
    void    SetText(const tstring& text);
    void    SetMessageText(int id, const tstring& text);

    /// Sets the hint text displayed on the messagebar.
    /// The hint text overlaps the messagebar contents, so it is only meant to be displayed temporarily. 
    /// Call ClearHintText to remove the hint text and restore the messagebar.
    /// Note: If a null-pointer or an empty string is passed, the hint text is cleared.
    //
    virtual void SetHintText(LPCTSTR text);

    /// String-aware overload
    //
    void SetHintText(const tstring& text) {SetHintText(text.c_str());}

    /// Clears the hint text, thus restoring the underlying content on the messagebar.
    //
    void ClearHintText();

  protected:
    // Compensate for highlight line
    //
    void   GetInnerRect(TRect& rect);
    void   GetDesiredSize(TSize& rect);

    // Draw highlight line, then HintText if any, or gadgets
    //
    void   PaintGadgets(TDC& dc, bool erase, TRect& rect);

    bool   GetHighlightLine() const;
    void   SetHighlightLine(bool highlightline);

    LPCTSTR  GetHintText() const;

  protected_data:
    bool   HighlightLine;  ///< does this bar have a upper highlight line?
    tstring HintText;       ///< current command hint being displayed, if any

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TMessageBar(const TMessageBar&);
    TMessageBar& operator =(const TMessageBar&);

  DECLARE_CASTABLE;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns true if the message bar has an upper highlight line.
//
inline bool TMessageBar::GetHighlightLine() const {
  return HighlightLine;
}

//
/// Sets the flag for the message bar to have an upper highlight line.
//
inline void TMessageBar::SetHighlightLine(bool highlightline) {
  HighlightLine = highlightline;
}

//
/// Returns the cached hint text for the current message.
//
inline LPCTSTR TMessageBar::GetHintText() const {
  return HintText.c_str();
}


} // OWL namespace


#endif  // OWL_MESSAGEB_H
