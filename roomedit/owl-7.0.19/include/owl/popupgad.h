//----------------------------------------------------------------------------//
// ObjectWindows 1998 Copyright by Yura Bidus                                 //
//                                                                            //
// Used code and ideas from Dieter Windau and Joseph Parrello                 //
//                                                                            //
// EMail: dieter.windau@usa.net                                               //
// Web:   http://members.aol.com/softengage/index.htm                         //
// EMail: joparrello@geocities.com, joparrel@tin.it                           //
// Web:   http://space.tin.it/computer/giparrel                               //
//                                                                            //
// Revision 1.0                                                               //
// Edited by Bidus Yura                                                       //
// Date 11/10/98                                                              //
//                                                                            //
// THIS CLASS AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF         //
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO        //
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A             //
// PARTICULAR PURPOSE.                                                        //
/// \file                                                                           //
/// Definition of classes TPopupButtonGadget and TPopupMenuGadget.             //
//----------------------------------------------------------------------------//

#if !defined(OWL_POPUPGAD_H) // Sentry, use file only if it's not already included.
#define OWL_POPUPGAD_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/btntextg.h>


namespace owl {


//
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TPopupButtonGadget
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
class _OWLCLASS TPopupButtonGadget : public TButtonTextGadget{
  public:
    enum TPopupType {
        Normal,           ///< Normal button style
                          ///< - press button -> PopupActionStart()
        DownArrow,        ///< Normal button style and additional a down arrow
                          ///< - press button -> PopupActionStart()
        DownArrowAction,  ///< Separator between left area and down arrow
                          ///< - press left button area -> Activate()
                          ///< - press down arrow area -> PopupActionStart()
     };

    TPopupButtonGadget(  int          id,
                        TResId       glyphResIdOrIndex,
                        TStyle       style = sBitmapText,
                        TPopupType  poptype = DownArrowAction,
                        TType       type = Command,
                        bool        enabled = false,
                        TState       state = Up,
                        bool        sharedGlyph = false,
                        uint         numChars = 4);

    // Default arrow width = 8
    //
    void          SetArrowWidth(uint width);
    uint          GetArrowWidth();

    // TGadget overrides
    //
    void GetDesiredSize(TSize& size) override;
    void SetBounds(const TRect& rect) override;

  protected:

    // TGadget overrides
    //
    void Paint(TDC& dc) override;
    void LButtonDown(uint modKeys, const TPoint& point) override;
    void LButtonUp(uint modKeys, const TPoint& point) override;
    void RButtonDown(uint modKeys, const TPoint&) override;

    // TButtonGadget overrides
    //
    void BeginPressed(const TPoint& p) override;
    void CancelPressed(const TPoint& p, bool mstate=false) override;

    // New virtuals
    //
    virtual void  PaintArrowButton(TDC& dc);
    virtual void  PaintArrow(TDC& dc, const TRect& rect);

    /// Start the PopupAction if the user click
    /// - 1) into the button if PopupType != DownArrowAction
    /// - 2) into the arrow  if PopupType == DownArrowAction
    /// This function only restore button state, overwrite
    /// them with functionality, but don't forget to
    /// call inherited one
    //
    virtual void  PopupAction();

  protected:
    enum TPopFlags{
      ArrowPressed  = 0x001,
      HasCapture    = 0x002,
    };
    TPopupType PopupType;
    uint       PopFlags;
    uint       ArrowWidth;
};


//
// class TPopupMenuGadget
// ~~~~~ ~~~~~~~~~~~~~~~~
//
class _OWLCLASS TPopupMenuGadget : public TPopupButtonGadget{
  public:
    TPopupMenuGadget(  TMenu&       menu,
                       TWindow*     window,
                       int          id,
                      TResId       glyphResIdOrIndex,
                       TStyle       style = sBitmapText,
                      TPopupType  poptype = DownArrowAction,
                      TType       type = Command,
                      bool        enabled = false,
                      TState       state = Up,
                      bool        sharedGlyph = false,
                      uint         numChars = 4);
    ~TPopupMenuGadget();

    // Set the flags. Default: TPM_LEFTALIGN | TPM_LEFTBUTTON
    //
    static void SetFlags(uint flags) { Flags = flags; }

    virtual void PopupAction();

  protected:
    TPopupMenu* PopupMenu;
    TWindow*    CmdTarget;
    static uint Flags;
};
/// @}

#include <owl/posclass.h>


//
// inlines
//

//
inline void
TPopupButtonGadget::SetArrowWidth(uint width)
{
  ArrowWidth = width;
}

//
inline uint
TPopupButtonGadget::GetArrowWidth()
{
  return ArrowWidth;
}


} // OWL namespace

#endif  // OWL_POPUPGAD_H
