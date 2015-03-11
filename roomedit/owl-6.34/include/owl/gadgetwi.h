//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TGadgetList, TGadgetWindow & TGadgetWindowFont
/// A list holding gadgets, & a window class owning & managing tiled gadgets.
//----------------------------------------------------------------------------

#if !defined(OWL_GADGETWI_H)
#define OWL_GADGETWI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/gadget.h>
#include <owl/gdiobjec.h>

namespace owl {

class _OWLCLASS TCelArray;
class _OWLCLASS TTooltip;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
/// \class TGadgetList
// ~~~~~ ~~~~~~~~~~~
/// TGadgetList is a list of gadgets with management functions.
//
class _OWLCLASS TGadgetList {
  public:
    TGadgetList();
    virtual ~TGadgetList();

		/// Enumerates the placement of a gadget. The new gadget is inserted either before
		/// or after another gadget.
		/// You can control the placement of the new gadget by specifying a sibling gadget
		/// that the new gadget is inserted before or after. If the sibling argument in
		/// TGadgetWindow::Insert is 0 then the new gadget is inserted at the beginning or
		/// the end of the existing gadgets. By default, the new gadget is inserted at the
		/// end of the existing gadgets.
    enum TPlacement {Before, After};

		/// \name
		/// @{    
    // Insert a Gadget or a list of gadgets. Gadgets are removed from
    // the source list
    //
    virtual void  Insert(TGadget& gadget, TPlacement = After,
                         TGadget* sibling = 0);
    virtual void  InsertFrom(TGadgetList& list, TPlacement = After,
                         TGadget* sibling = 0);
    /// @}                

    /// Removes (unlinks) a gadget.
    //
    virtual TGadget*  Remove(TGadget& gadget);

    // Callbacks invoked when a gadget is removed or inserted
    //
    virtual void  Inserted(TGadget& gadget);
    virtual void  Removed(TGadget& gadget);

    // Insert/remove methods that take a source/dest TGadgetList&

    // Locate and iterate over gadgets
    //
    uint          GadgetCount() const;
    TGadget*      FirstGadget() const;
    TGadget*      NextGadget(TGadget& gadget) const;
    TGadget*      GadgetFromPoint(const TPoint& point) const;
    TGadget*      GadgetWithId(int id) const;
    TGadget*      operator [](uint index);

  protected_data:
    TGadget*        Gadgets;        ///< Linked list of gadgets
    uint            NumGadgets;     ///< Total number of gadgets
};

//
/// \class TGadgetWindowFont
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// Derived from TFont, TGadgetWindowFont is a specific font used in gadget windows
/// for sizing and default text. You can specify the point size of the font (not the
/// size in pixels) and whether it is bold or italic. You can use one of the
/// following FF_xxxx constants to indicate the font family type:
/// - \c \b  FF_DECORATIVE	Specialty fonts such as Old English.
/// - \c \b  FF_DONTCARE	The font type does not matter.
/// - \c \b  FF_MODERN	Fonts such as Pica, Elite, or Courier with a constant stroke, width,
/// and with or without serifs.
/// - \c \b  FF_ROMAN	Fonts such as Times New Roman and New Century Schoolbook with varied
/// stroke and with serifs.
/// - \c \b  FF_SCRIPT	Fonts such as Script that are designed to resemble handwriting.
/// - \c \b  FF_SWISS	Fonts such as MS Sans Serif with variable stroke width and without
/// serifs.
/// 
/// Depending on the typeface, the font weight can be one of the following FW_xxxx
/// constants:
/// 
/// - \c \b  FW_DONTCARE	Does not matter
/// - \c \b  FW_THIN	Thin
/// - \c \b  FW_EXTRALIGHT	Extra light
/// - \c \b  FW_ULTRALIGHT	Extra light
/// - \c \b  FW_LIGHT	Light
/// - \c \b  FW_NORMAL	Normal
/// - \c \b  FW_REGULAR	Normal font weight
/// - \c \b  FW_MEDIUM	Medium
/// - \c \b  FW_SEMIBOLD	Somewhat bold
/// - \c \b  FW_DEMIBOLD	Somewhat bold
/// - \c \b  FW_BOLD	Bold
/// - \c \b  FW_EXTRABOLD	Extra bold
/// - \c \b  FW_ULTRABOLD	Extra bold
/// - \c \b  FW_BLACK	Heavy weight
/// - \c \b  FW_HEAVY	Heavy weight
/// 
/// Because the font's appearance depends on the typeface, some fonts have only
/// FW_NORMAL, FW_REGULAR, and FW_BOLD available. If FW_DONTCARE is indicated, the
/// default font weight is used.
//
class _OWLCLASS TGadgetWindowFont : public TFont {
  public:
    TGadgetWindowFont(int  pointSize = 10,
                      bool bold = false,
                      bool italic = false);
    TGadgetWindowFont(const TFont& font);
};

//
/// \class TGadgetWindow
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TWindow, TGadgetWindow maintains a list of tiled gadgets for a
/// window and lets you dynamically arrange tool bars. You can specify the following
/// attributes of these gadgets:
/// - \c \b  Horizontal or vertical tiling. Positions the gadgets horizontally
/// or vertically within the inner rectangle (the area excluding borders and
/// margins).
/// - \c \b  Gadget font. Default font to use for gadgets and for calculating
/// layout units. For font information, see the description of TGadgetWindowFont.
/// - \c \b  Left, right, top, and bottom margins. Specified in pixels, layout
/// units (based on the window font), or border units (the width or height of a thin
/// window border).
/// - \c \b  Measurement units. Specified in pixels, layout units, or border
/// units.
/// - \c \b  Gadget window size. A gadget window can shrink-wrap its width,
/// height, or both to fit around its gadgets. By default, horizontally tiled
/// gadgets shrink-wrap to fit the height of the window and vertically tiled gadgets
/// shrink-wrap to fit the width of the window.
/// 
/// TGadgetWindow is the base class for the following derived classes: TControlBar,
/// TMessageBar, TToolBox, and TStatusBar.
//
class _OWLCLASS TGadgetWindow : virtual public TWindow, public TGadgetList {
  public:

    /// Enumeration describing how gadgets should be laid out within the
    /// gadget window.
		/// TGadgetWindow::TileGadgets actually tiles the gadgets in the direction
		/// requested.
    //
    enum TTileDirection {
      Horizontal,       ///< Arrange gadgets in a row
      Vertical,         ///< Arrange gadgets in a column
      Rectangular       ///< Arrange gadgets in rows and columns (2-D grid)
    };

    TGadgetWindow(TWindow*        parent = 0,
                  TTileDirection  direction = Horizontal,
                  TFont*          font = new TGadgetWindowFont,
                  TModule*        module = 0);
   ~TGadgetWindow();

    // Override TWindow member function and choose initial size if shrink
    // wrapping was requested
    //
    bool          Create();

    // Changes the margins and initiates a layout session
    //
    void          SetMargins(TMargins& margins);
    
    TMargins&     GetMargins();
    const TMargins& GetMargins() const;

    // Get or set the direction. Setting the direction swaps dimentional
    // members & initiates a layout session
    //
    TTileDirection GetDirection() const;
    virtual void  SetDirection(TTileDirection direction);

    void          SetRectangularDimensions(int width, int height, int rowMargin= -1);

    TFont&        GetFont();
    uint          GetFontHeight() const;

    // Retrieves/assigns tooltip of/to window
    //
    TTooltip*     GetTooltip() const;
    void          EnableTooltip(bool enable=true);
    void          SetTooltip(TTooltip* tooltip);

    bool          GetWantTooltip() const;
    void          SetWantTooltip(bool wantTooltip);

    // Getting & releasing of the mouse by gadgets.
    //
    bool          GadgetSetCapture(TGadget& gadget);
    void          GadgetReleaseCapture(TGadget& gadget);
    TGadget*      GadgetGetCaptured();

    /// Hint mode settings & action used by contained gadgets
    //
    enum THintMode {
      NoHints,			///< No hints.
      PressHints,		///< Hints when a button is pressed.
      EnterHints		///< Hints when the mouse passes over a gadget.
    };
    void          SetHintMode(THintMode hintMode);
    THintMode     GetHintMode();
    void          SetHintCommand(int id);  ///< (id <= 0) to clear

/// Enumerates the flat styles settings of the gadget.
    enum TFlatStyle{
      NonFlatNormal    =  0x0000L, ///< No Flat style - default.
      FlatStandard    = 0x0001L, ///< Flat style IE 3.0 - base style.
      FlatGrayButtons = 0x0002L, ///< Adds Gray buttons effect to FlatStandard.
      FlatHotText     = 0x0004L, ///< Adds hot text effect like
      FlatSingleDiv   = 0x0008L, ///< Adds single divider ala IE 4.01
      FlatXPTheme     = 0x0010L, ///< Windows XP theme styles
      FlatDefault      = 0xFFFFL, ///< System automatically select apropriated style.
    };

    static void   EnableFlatStyle(uint style = FlatDefault);
    /*virtual*/ static uint  GetFlatStyle();

    // Windows XP theme styles
    //
    bool IsThemed() const;
    void EnableThemeBackground(bool enable = true);
    bool IsThemeBackgroundEnabled() const;
    bool IsBackgroundThemed() const;

    // TGadgetList list management overrides
    // Insert & remove a Gadget, update their Window ptr to this window
    //
    void     Insert(TGadget& gadget, TPlacement = After, TGadget* sibling = 0);
    void     InsertFrom(TGadgetList& list, TPlacement = After, TGadget* sibling = 0);
    TGadget* Remove(TGadget& gadget);

    // Override callbacks invoked when a gadget is removed or inserted
    // Locate and iterate over gadgets
    //
    void     Inserted(TGadget& gadget);
    void     Removed(TGadget& gadget);

    // Shared CelArray management
    //
    virtual void          SetCelArray(TCelArray* sharedCels);
    virtual TCelArray&    GetCelArray(int minX = 0, int minY = 0);

    // During idle time, iterates over the Gadgets invoking their
    // CommandEnable() member function
    //
    bool          IdleAction(long idleCount);

    // Set timer - useful for gadgets that need regular update [Time/Date]
    //
    bool          EnableTimer();

    // Sent by a Gadget when its size has changed. Initiates a layout session
    //
    void          GadgetChangedSize(TGadget& gadget);

    // Begins a layout session which tiles the Gadgets & repaints
    //
    virtual void  LayoutSession();

    // Simply sets the corresponding member data
    //
    void          SetShrinkWrap(bool shrinkWrapWidth, bool shrinkWrapHeight);

    // Get the desired size for this gadget window.
    //
    virtual void  GetDesiredSize(TSize& size);

  protected:
    // Called by Paint(). Iterates over the Gadgets and asks each one to draw
    //
    virtual void  PaintGadgets(TDC& dc, bool erase, TRect& rect);

    // Computes the area inside of the borders and margins
    //
    virtual void  GetInnerRect(TRect& rect);

    int           LayoutUnitsToPixels(int units);

    void          GetMargins(const TMargins& margins,
                             int& left, int& right, int& top, int& bottom);
    void          UseDesiredSize();

    /// Gadget layout information used during the layout process
    //
    class TLayoutInfo {
      public:
        TLayoutInfo(int gadgetCount)
            : DesiredSize(0,0), GadgetBounds(new TRect[gadgetCount]) {}
        TSize            DesiredSize;
        TAPointer<TRect> GadgetBounds;
    };

    // Calculates the layout of the Gadgets in the specified direction
    // Returns the TLayoutInfo to describe the result but does not move the
    // gadgets.
    //
    virtual void  LayoutGadgets(TTileDirection dir, TLayoutInfo& layout);

    /// Tiles the Gadgets in the current direction
    //
    virtual TRect TileGadgets();

    // Selects the font into the DC and calls PaintGadgets()
    //
    void          Paint(TDC& dc, bool erase, TRect& rect);

    virtual void  PositionGadget(TGadget* previous, TGadget* next,
                                 TPoint& point);

    // Overrides of TWindow virtuals
    //
    void          SetupWindow();
    void          CleanupWindow();
    bool          PreProcessMsg(MSG& msg);

    // Message response functions
    //
    void          EvLButtonDown(uint modKeys, const TPoint& point);
    void          EvLButtonUp(uint modKeys, const TPoint& point);
    void          EvLButtonDblClk(uint modKeys, const TPoint& point);
    void          EvRButtonDown(uint modKeys, const TPoint& point);
    void          EvRButtonUp(uint modKeys, const TPoint& point);
    void          EvMouseMove(uint modKeys, const TPoint& point);
    void          EvWindowPosChanging(WINDOWPOS & windowPos);
    void          EvSysColorChange();

    void          EvCreateTooltips();

    // The overridden event handler functions are virtual, and not dispatched
    // thru response tables
    //
    TResult       EvCommand(uint id, HWND hWndCtl, uint notifyCode);
    void          EvCommandEnable(TCommandEnabler& ce);

  // Protected data members
  //
  protected_data:
    TFont*          Font;           ///< Font used for size calculations
// TBrush*         BkgndBrush;     // OBSOLETE: background brush
    TGadget*        Capture;        ///< Gadget that has captured the mouse
    TGadget*        AtMouse;        ///< Last Gadget at mouse position
    TMargins        Margins;
    uint            FontHeight       : 8;
    bool            ShrinkWrapWidth  : 8;
    bool            ShrinkWrapHeight : 8;
    uint            WideAsPossible   : 8;  ///< # of "WideAsPossible" gadgets
    bool            DirtyLayout      : 8;
    TTileDirection  Direction        : 8;
    THintMode       HintMode         : 8;
    TCelArray*      SharedCels;     ///< CelArray that can be shared by gadgets
    TTooltip*        Tooltip;        ///< Tooltip
    bool            WantTooltip;    ///< Flags whether to create a tooltip
    int             RowWidth;       ///< Requested max width of each row
    int             RowMargin;      ///< Settable inter-row margin
    bool            WantTimer;      ///< Flags whether to start a timer
    UINT_PTR        TimerID;        ///< Timer identifier
    static uint     FlatStyle;      ///< FlatStyle
    bool            ThemeBackgroundMode; ///< Indicate enablement of themed backgrounds

  private:
    // Calculate layouts metrics for window and all gadgets for a given
    // direction
    //
    void          LayoutHorizontally(TLayoutInfo&);
    void          LayoutVertically(TLayoutInfo&);
    void          LayoutRectangularly(TLayoutInfo&);
    void          FinishRow(int, TGadget*, TGadget*, int, TLayoutInfo&, int&);

    // Hidden to prevent accidental copying or assignment
    //
    TGadgetWindow(const TGadgetWindow&);
    TGadgetWindow& operator =(const TGadgetWindow&);

  friend class TGadget;

  DECLARE_RESPONSE_TABLE(TGadgetWindow);
  DECLARE_CASTABLE;
};


#define EV_WM_CREATETOOLTIP\
  {{WM_OWLCREATETTIP}, 0, (::owl::TAnyDispatcher) ::owl::v_Dispatch,\
   (TMyPMF)::owl::v_Sig(&TMyClass::EvCreateTooltips)}


//
/// \class TGadgetControl
// ~~~~~ ~~~~~~~~~~~~~~
/// TGadgetControl is a specialized, easily constructed gadget window that holds one
/// gadget for use as a control in a window.
//
class _OWLCLASS TGadgetControl : public TGadgetWindow {
  public:
    TGadgetControl(TWindow*        parent = 0,
                   TGadget*        soleGadget = 0,
                   TFont*          font = new TGadgetWindowFont,
                   TModule*        module = 0);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

//
/// Returns the number of gadgets in the list.
//
inline uint TGadgetList::GadgetCount() const {
  return NumGadgets;
}

//
/// Returns the first gadget of the list.
//
inline TGadget* TGadgetList::FirstGadget() const {
  return Gadgets;
}

//
/// Returns the next gadget in the list relative to a given gadget
//
inline TGadget* TGadgetList::NextGadget(TGadget& gadget) const {
  return gadget.NextGadget();
}

//----------------------------------------------------------------------------

//
/// Returns the direction of tiling for this gadget window.
//
inline TGadgetWindow::TTileDirection TGadgetWindow::GetDirection() const {
  return Direction;
}

//
/// Returns the font being used by this gadget window.
/// \note The font is Sans Serif by default.
//
inline TFont& TGadgetWindow::GetFont() {
  return *Font;
}

//
/// Returns Tooltip
//
inline TTooltip* TGadgetWindow::GetTooltip() const {
  return Tooltip;
}

//
/// Returns the height of the font being used by this gadget window.
//
inline uint TGadgetWindow::GetFontHeight() const {
  return FontHeight;
}

//
/// Sets the mode of the hint text. Defaults to PressHints (displays hint text when
/// a button is pressed).
//
inline void TGadgetWindow::SetHintMode(THintMode hintMode){
  HintMode = hintMode;
}

//
/// Returns the current hint mode of this gadget window.
//
inline TGadgetWindow::THintMode TGadgetWindow::GetHintMode(){
  return HintMode;
}

//
/// Returns the flat style
//
inline uint TGadgetWindow::GetFlatStyle() {
  return FlatStyle;
}

//
/// Retrieve the sizes of the 4 margins in pixels for this gadget window given a
/// margin object
//
inline void TGadgetWindow::GetMargins(const TMargins& margins,
                int& left, int& right, int& top, int& bottom) 
{
  margins.GetPixels(left, right, top, bottom, FontHeight);
}

//
/// Retrieves gadget with capture
//
inline TGadget* TGadgetWindow::GadgetGetCaptured()
{
  return Capture;
}

inline TMargins& TGadgetWindow::GetMargins()
{
	return Margins;
}
		
inline const TMargins& TGadgetWindow::GetMargins() const
{
	return Margins;
}

inline bool TGadgetWindow::GetWantTooltip() const
{
	return WantTooltip;
}

inline void TGadgetWindow::SetWantTooltip(bool wantTooltip)
{
	WantTooltip = wantTooltip;
}


} // OWL namespace


#endif  // OWL_GADGETWI_H
