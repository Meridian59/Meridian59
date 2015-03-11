//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file 
/// Definition of class TStatusBar.
//----------------------------------------------------------------------------

#if !defined(OWL_STATUSBA_H)
#define OWL_STATUSBA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/messageb.h>
#include <owl/textgadg.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
//
/// \class TStatusBar
// ~~~~~ ~~~~~~~~~~
/// Status bars have more options than a plain message bar: you can have
/// multiple text gadgets, different style borders, and you can reserve space
/// for mode indicators
//
class _OWLCLASS TStatusBar : public TMessageBar {
  public:
/// Enumerates the keyboard modes. By default, these are arranged horizontally on
/// the status bar from left to right. Sets the extended selection, CapsLock,
/// NumLock, ScrollLock, Overtype, recording macro, and size grip indicators.
    enum TModeIndicator {
      ExtendSelection = 1,
      CapsLock        = 1 << 1,
      NumLock         = 1 << 2,
      ScrollLock      = 1 << 3,
      Overtype        = 1 << 4,
      RecordingMacro  = 1 << 5,
      SizeGrip        = 1 << 6
    };

    TStatusBar(TWindow*              parent = 0,
               TGadget::TBorderStyle borderStyle = TGadget::Recessed,
               uint                  modeIndicators = 0,
               TFont*                font = 0, //new TGadgetWindowFont(6),
               TModule*              module = 0);

    /// By default, adds "gadget" after the existing text gadgets and before
    /// the mode indicator gadgets. Sets the border style to the style specified
    /// during construction.
    //
    void        Insert(TGadget& gadget, TPlacement = After, TGadget* sibling = 0);

    /// Overriden method of TMessageBar to use our own text gadget
    /// Set (or clear if 0) menu/command item hint text displayed in/on bar
    //
    virtual void SetHintText(LPCTSTR text);

    // Access string-aware overload
    //
    using TMessageBar::SetHintText;

    // In order for the mode indicator status to appear you must have
    // specified the mode when the window was constructed
    //
    bool        GetModeIndicator(TModeIndicator i) const;
    void        SetModeIndicator(TModeIndicator, bool state);
    void        ToggleModeIndicator(TModeIndicator);

    struct TSpacing {
      TMargins::TUnits  Units;
      int               Value;
      TSpacing();
    };

    /// Sets the spacing to be used between mode indicator gadgets
    //
    void        SetSpacing(const TSpacing& spacing);

    /// Control whether hint text is display over the whole window or in
    /// a text gadget.
    //
    void        SetWideHints(bool on);

  protected:
    void        PositionGadget(TGadget* previous, TGadget* next, TPoint& point);
    TSpacing&   GetSpacing();

    uint        GetNumModeIndicators();
    uint        GetModeIndicators() const;
    void        SetModeIndicators(uint modeindicators);

    ///BGM SetModeIndicators is nearly useless; after construction, TStatusBar
    ///BGM pays almost no attention to the ModeIndicators member

    void        EvOwlFrameSize(uint sizeType, const TSize&);
    uint        EvNCHitTest(const TPoint& point);

  protected_data:
/// One of the enumerated border styles--None, Plain, Raised, Recessed, or
/// Embossed--used by the mode indicators on the status bar.
    TGadget::TBorderStyle  BorderStyle;
    	
/// Specifies the spacing between mode indicators on the status bar.
    TSpacing               Spacing;
    
/// Specifies the number of mode indicators, which can range from 1 to 5.
    uint                   NumModeIndicators;
    
/// The ModeIndicators bit field indicates which mode indicators have been created
/// for the status bar.
    uint                   ModeIndicators;

/// Specifies the mode of the status bar. This can be any one of the values of
/// TModeIndicator enum, such as CapsLock, NumLock, ScrollLock, Overtype,
/// RecordingMacro, or ExtendSelection.
    uint                   ModeIndicatorState;

    bool                   WideHintText;

  private:
    bool        GetGadgetAndStrings(TModeIndicator mode, TTextGadget*& gadget,
                                    LPCTSTR& strOn);
    bool        IsModeIndicator(TGadget* gadget);

    void        InsertSizeGrip(void);

    // Hidden to prevent accidental copying or assignment
    //
    TStatusBar(const TStatusBar&);
    TStatusBar& operator =(const TStatusBar&);

  DECLARE_CASTABLE;

  DECLARE_RESPONSE_TABLE(TStatusBar);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//
//

//
/// Returns the current status bar mode indicator.
//
inline bool TStatusBar::GetModeIndicator(TStatusBar::TModeIndicator i) const {
  return (ModeIndicatorState & i) ? 1 : 0;
}

//
/// Initialize spacing.
//
inline TStatusBar::TSpacing::TSpacing() {
  Units = TMargins::LayoutUnits;
  Value = 0;
}

//
/// Uses the TSpacing values to set the spacing to be used between mode indicator
/// gadgets. TSpacing sets the status-bar margins in layout units. Typically, the
/// message indicator (the leftmost text gadget) is left-justified on the status bar
/// and the other indicators are right-justified. See TLayoutMetrics for a detailed
/// explanation of layout units and constraints.
/// \code
/// struct TSpacing { 
///     TMargins::TUnits  Units;
///     int Value;
///     TSpacing() { Units = TMargins::LayoutUnits; Value = 0;}
/// };
/// \endcode
inline void TStatusBar::SetSpacing(const TStatusBar::TSpacing& spacing) {
  Spacing = spacing;
}

//
/// Returns the spacing between the mode indicator gadgets.
//
inline TStatusBar::TSpacing& TStatusBar::GetSpacing() {
  return Spacing;
}

//
/// Returns the number of mode indicators that are on.
//
inline uint TStatusBar::GetNumModeIndicators() {
  return NumModeIndicators;
}

//
/// Returns the bit flags for which indicator is on.
//
inline uint TStatusBar::GetModeIndicators() const {
  return ModeIndicators;
}

//
/// Sets the bit flags for which indicator is on.
//
inline void TStatusBar::SetModeIndicators(uint modeindicators) {
  ModeIndicators = modeindicators;
}

//
/// Informs the StatusBar whether hints should be displayed in a text gadget
/// (on == false) or over the area of the whole statusbar (on == true).
//
inline void TStatusBar::SetWideHints(bool on) {
  WideHintText = on;
}

} // OWL namespace


#endif  // OWL_STATUSBA_H
