//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Base class TGadget and simple derived TSeparatorGadget.
//----------------------------------------------------------------------------

#if !defined(OWL_GADGET_H)
#define OWL_GADGET_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/dc.h>

namespace owl {

class _OWLCLASS TGadgetWindow;

//
/// \name Ids of some OWL predefined gadgets
/// @{
#define IDG_FIRST       30000   				///< first predefined gadget ID
#define IDG_LAST        31000   				///< last predefined gadget ID
#define IDG_MESSAGE     IDG_FIRST + 0		///< + 0 is for backwards compatability
#define IDG_STATUS_EXT  IDG_FIRST + 1		///< The ID for an extended selection gadget    
#define IDG_STATUS_CAPS IDG_FIRST + 2		///< The ID for a capslock gadget
#define IDG_STATUS_NUM  IDG_FIRST + 3		///< The ID for a numlock gadget
#define IDG_STATUS_SCRL IDG_FIRST + 4		///< The ID for a scroll lock gadget
#define IDG_STATUS_OVR  IDG_FIRST + 5		///< The ID for an overwrite gadget
#define IDG_STATUS_REC  IDG_FIRST + 6		///< The ID for a record gadget
#define IDG_SIZEGRIP    IDG_FIRST + 7		///< The ID for a size grip gadget
#define IDG_FLATHANDLE  IDG_FIRST + 8		///< The ID for a flat bar handle
/// @}

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{


//
/// \struct TMargins
// ~~~~~~ ~~~~~~~~
/// Used by the TGadgetWindow and TGadget classes, TMargins contains the
/// measurements of the margins for the gadget. The constructor initializes Units to
/// LayoutUnits and sets Left, Right, Top, and Bottom equal to 0.
//
struct _OWLCLASS TMargins 
{
  enum TUnits {Pixels, LayoutUnits, BorderUnits};

  TUnits  Units;

  int  Left   : 8;
  int  Right  : 8;
  int  Top    : 8;
  int  Bottom : 8;

  TMargins() {Units = LayoutUnits; Left = Right = Top = Bottom = 0;}

  TMargins(TUnits u, int l, int r, int t, int b)
      {Units = u; Left = l; Right = r; Top = t; Bottom = b;}

  void GetPixels(int& left, int& right, int& top, int& bottom, int fontHeight) const;
};

//
/// \class TGadget
// ~~~~~ ~~~~~~~
/// TGadget is the base class for the following derived gadget classes:
/// - \c \b  TBitmapGadget	Displays a bitmap
/// - \c \b  TButtonGadget	Uses a bitmap to simulate a button gadget
/// - \c \b  TButtonTextGadget 	Derived from TButtonGadget and adds displays optionally text
/// or bitmap or both.
/// - \c \b  TControlGadget	Encapsulates inserting a control such as an edit control or a
/// combobox, into a gadget window.
/// - \c \b  TFlatHandleGadget 	Uses by TControlBar/TDockableControlBar to simulate FlatGrip
/// when FlatState enabled.
/// - \c \b  TMenuGadget 	Displays ppopup menu
/// - \c \b  TPopupButtonGadget 	Derived from TButtonTextGadget and adds button for popup
/// actions
/// - \c \b  TPopupMenuGadget 	Derived from TPopupButtonGadget and show popup menu
/// - \c \b  TTextGadget	Displays text
/// - \c \b  TSeparatorGadget	Separates logical groups of gadgets.
/// - \c \b  TSizeGripGadget 	Used on the far right of a status bar to provide re-sizing
/// grip for the host window.
/// 
/// TGadget interface objects belong to a gadget window, have borders and margins,
/// and have their own coordinate system. The margins are the same as those for
/// TGadgetWindow and borders are always measured in border units.
/// 
/// To set the attributes for the gadget, you can either choose a border style
/// (which automatically sets the individual border edges) or set the borders and
/// then override the member function PaintBorder to create a custom look for your
/// gadget. If you change the borders, margins, or border style, the gadget window's
/// GadgetChangedSize member function is invoked.
/// 
/// Although, by default, gadgets shrink-wrap to fit around their contents, you can
/// control this attribute by setting your own values for ShrinkWrapWidth and
/// ShrinkWrapHeight.
/// 
/// A gadget window, being an actual window, receives messages from the mouse. After
/// the gadget window receives the message, it decides which gadget should receive
/// the message by calling the member function directly instead of sending or
/// posting a message.
///
/// Typically you would either choose a border style (which automatically
/// sets the individual border edges) or set the borders and then override
/// member function PaintBorder() to create a custom look
///
/// \note changing the borders, margins, or border style all end up invoking
///       the gadget window's GadgetChangedSize() member function
//
class _OWLCLASS TGadget 
{
  public:
    /// Gadget border styles. These match TIUBorder's TStyles exactly.
		/// For an example of border styles,
		/// see the sample ObjectWindows program, MDIFILE.CPP, on BC5.x distribution disk.
		/// Border Style Constants
    enum TBorderStyle 
    {
      None,             ///<  No border painted at all
      Plain,            ///<  Plain window frame
      Raised,           ///<  Raised above the gadget window
      Recessed,         ///<  Recessed into the window
      Embossed,         ///<  Painted with an embossed border
      Grooved,          ///<  Grouping groove
      ButtonUp,         ///<  Button in up position
      ButtonDn,         ///<  Button in down position
      WndRaised,        ///<  Inner and outer edge of the window are raised
      WndRecessed,      ///<  Input field and other window are recessed
      WellSet,          ///<  Well option set (auto grows + 1)
    };

    TGadget(int id = 0, TBorderStyle borderStyle = None);
    virtual ~TGadget();

    /// Border dimensions
    ///
		/// Holds the values for the left, right, top, and bottom measurements of the
		/// gadget.
    struct _OWLCLASS TBorders 
    {
      uint  Left;
      uint  Right;
      uint  Top;
      uint  Bottom;

      TBorders() {Left = Right = Top = Bottom = 0;}
    };

    int            GetId() const;

		/// @{
    void           SetBorders(const TBorders& borders);
    TBorders&      GetBorders();

    void           SetMargins(const TMargins& margins);
    TMargins&      GetMargins();

    void           SetBorderStyle(TBorderStyle bs);
    TBorderStyle   GetBorderStyle() const;
		/// @}

    TRect&         GetBounds();
    const TRect&   GetBounds() const;

    virtual void   SetEnabled(bool enabled);
    bool           GetEnabled() const;

    void           SetVisible(bool visible);
    bool           IsVisible() const;

    bool           IsWindowVisible() const;  ///< check to see if the button's created

    void           SetEndOfRow(bool eor);
    bool           IsEndOfRow() const;

		bool           IsWideAsPossible() const;
		void           SetWideAsPossible(bool wide = true);


    // Simply sets the corresponding member data.
    //
    void           SetShrinkWrap(bool shrinkWrapWidth, bool shrinkWrapHeight);

    // Directly alters the size of the gadget.
    //
    void           SetSize(const TSize& size);

    // Get the size that this gadget would like to be
    //
    virtual void   GetDesiredSize(TSize& size);

    // Returns the amount of space in pixels taken up by the borders and
    // margins
    //
    void           GetOuterSizes(int& left, int& right, int& top, int& bottom);

    // Returns the area excluding the borders and margins
    //
    void           GetInnerRect(TRect& rect);

    // Set the bounding rectangle for this gadget
    //
    virtual void   SetBounds(const TRect& rect);

    virtual bool   IdleAction(long idleCount);
    virtual void   CommandEnable();
    virtual void   SysColorChange();

    TGadget*       NextGadget();

  protected:

		TGadgetWindow* GetGadgetWindow();
		const TGadgetWindow* GetGadgetWindow() const;

    // The following virtual methods are called to allow the gadget
    // to initialize, cleanup or update any internal information
    // when its inserted into a window, moved or removed.
    //
    virtual void   Created();
    virtual void   Inserted();
    virtual void   Moved();
    virtual void   Removed();

    bool           IsHaveMouse() const;
    void           SetInMouse(bool state);
    
    bool           GetTrackMouse() const;
    void           SetTrackMouse(bool track);

    void           Invalidate(bool erase = true);
    void           InvalidateRect(const TRect& rect, // receiver's coord system
                                  bool  erase = true);
    void           Update();  ///< Paint now if possible

    /// Default behavior returns true if the point is within the receiver's
    /// bounding rect. "point" is in the receiver's coordinate system
    //
    virtual bool   PtIn(const TPoint& point);

    virtual void   Paint(TDC& dc);

    /// Self sent by method Paint(). Override this is if you want to
    /// implement a border style that isn't supported
    //
    virtual void   PaintBorder(TDC& dc);

    // Mouse virtuals forwarded from TGadgetWindows message handlers.
    // "point" is in the receiver's coordinate system.

    // MouseMove is only called if the mouse is captured. Enter & leave when
    // mouse enters & leaves a gadget & is not captured.
    //
    virtual void   MouseEnter(uint modKeys, const TPoint& point);
    virtual void   MouseMove(uint modKeys, const TPoint& point);
    virtual void   MouseLeave(uint modKeys, const TPoint& point);

    // Captures the mouse if "TrackMouse" is set.
    //
    virtual void   LButtonDown(uint modKeys, const TPoint& point);
    virtual void   RButtonDown(uint modKeys, const TPoint& point);

    // Releases the mouse capture if "TrackMouse" is set.
    //
    virtual void   LButtonUp(uint modKeys, const TPoint& point);
    virtual void   RButtonUp(uint modKeys, const TPoint& point);

    void           ChangeBorderStyle(TBorderStyle bs);

  // Data members -- will become private
  //
  public_data:
/// If Clip is false, clipping borders have not been established. If Clip is true,
/// the drawing for each gadget is restrained by the gadget's border.
    bool             Clip;         
    
/// Initially set to false, WideAsPossible indicates whether the gadget width will
/// be adjusted by the gadget window to be as wide as possible in the remaining
/// space.
    bool             WideAsPossible;  

  protected_data:
/// References the owning or parent window for the gadget.
    TGadgetWindow*   Window;          
    
/// Contains the bounding rectangle for the gadget in gadget window coordinates.
    TRect            Bounds;          
    
/// Contains the border style for the gadget.
    TBorderStyle     BorderStyle;     
    
/// Contains the border measurements of TGadget::GetInnerRect().
    TBorders         Borders;         

/// Contains the margin measurements of the rectangle or the gadget.
    TMargins         Margins;         
    
    bool             ShrinkWrapWidth; ///< Shrink the width to fit contents?
    bool             ShrinkWrapHeight;///< Shrink the height to fit contents?
    
/// Initialized to false. When TrackMouse is true, the gadget captures  the mouse on
/// LButtonDown by calling TGadgetWindow's GadgetSetCapture and releases the mouse
/// on  LButtonUp by calling GadgetReleaseCapture.
    bool             TrackMouse;      
    
/// True if mouse in gadget.
    bool             MouseInGadget;   
    
/// Contains the gadget's ID.
    int              Id;              

  private:
    TGadget*         Next;
    enum 
    {
      Enabled  = 0x01,
      Visible  = 0x02,
      EndOfRow = 0x04,
    };
    uint16           Flags;           ///< Enabled, EndOfRow flags

    // Hidden to prevent accidental copying or assignment
    //
    TGadget(const TGadget&);
    TGadget& operator =(const TGadget&);

  friend class _OWLCLASS TGadgetList;   // Access to Next for containment
  friend class _OWLCLASS TGadgetWindow; // Access to Next for containment
  DECLARE_CASTABLE;
};

//
/// \class TSeparatorGadget
// ~~~~~ ~~~~~~~~~~~~~~~~
/// TSeparatorGadget is a simple class you can use to create a separator between
/// gadgets. To do so, you must specify the size of the separator in units of
/// SM_CXBORDER (width of the window frame) and SM_CYBORDER (height of the window
/// frame). Determines the width and height of the gadget and sets the right and
/// bottom boundaries of the separator. By default, the separator disables itself
/// and turns off shrink-wrapping. Note that the default border style is none.
///
/// "size" is used for both the width and the height
//
class _OWLCLASS TSeparatorGadget : public TGadget 
{
  public:                                     // only for Flat mode
    TSeparatorGadget(int size = 0, int id = 0, bool showseparator=true);

    void Inserted();
    void Paint(TDC& dc);
  
    void SetShowSep(bool show = true) {ShowSeparator = show; }
    bool GetShowSep()                  {return ShowSeparator; }

  protected:
    bool ShowSeparator;
};

//
/// \class TFlatHandleGadget
// ~~~~~ ~~~~~~~~~~~~~~~
/// A gadget that is used on the far right of a status bar to provide re-sizing
/// grip for the host window.
//
class _OWLCLASS TFlatHandleGadget : public TSeparatorGadget 
{
  public:
    TFlatHandleGadget(int id = IDG_FLATHANDLE);
    virtual void Paint(TDC& dc);
    virtual void GetDesiredSize(TSize& size);
};

//
/// \class TSizeGripGadget
// ~~~~~ ~~~~~~~~~~~~~~~
/// TSizeGripGadget is a gadget that is used on the far right of a status bar to
/// provide re-sizing grip for the host window.
//
class _OWLCLASS TSizeGripGadget : public TSeparatorGadget 
{
  public:
    TSizeGripGadget(int id = IDG_SIZEGRIP);
    void Paint(TDC& dc);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Gets the ID for the gadget.
//
inline int TGadget::GetId () const 
{
  return Id;
}

//
/// Gets the gadget's borders measured in border units that are based on SM_CXBORDER
/// and SM_CYBORDER.
//
inline TGadget::TBorders& TGadget::GetBorders() 
{
  return Borders;
}


//
/// Gets the margin dimensions.
//
inline TMargins& TGadget::GetMargins() 
{
  return Margins;
}

//
/// Gets the style for the gadget's borders.
//
inline TGadget::TBorderStyle TGadget::GetBorderStyle() const 
{
  return BorderStyle;
}

//
/// Returns the boundary rectangle for the gadget.
//
inline TRect& TGadget::GetBounds() 
{
  return Bounds;
}

//
/// Returns the boundary rectangle for the gadget.
//
inline const TRect& TGadget::GetBounds() const
{
  return Bounds;
}

//
/// Determines whether keyboard and mouse input have been enabled for the specified
/// gadget. If the gadget is enabled, GetEnabled returns true; otherwise, it returns
/// false. By default, keyboard and mouse input are enabled.
//
inline bool TGadget::GetEnabled() const 
{
  return Flags & Enabled;
}

//
/// Changes the visibility of the gadget.
//
inline void TGadget::SetVisible(bool visible) 
{
  if (visible)
    Flags |= Visible;
  else
    Flags &= ~Visible;
}

//
/// Returns true if the gadget is visible.
//
inline bool TGadget::IsVisible() const 
{
  return Flags & Visible;
}

//
/// Return true if mouse inside gadget.
//
inline bool TGadget::IsHaveMouse() const
{
  return MouseInGadget;
}

/// Sets if mouse inside gadget or not.
inline void TGadget::SetInMouse(bool state)
{
  MouseInGadget = state;
}

inline bool TGadget::GetTrackMouse() const
{
	return TrackMouse;
}

inline void TGadget::SetTrackMouse(bool track)
{
	TrackMouse = track;
}


//
/// Sets the end of row property for the gadget. The end of row property is used to
/// tile gadgets.
//
inline void TGadget::SetEndOfRow(bool eor) 
{
  if (eor)
    Flags |= EndOfRow;
  else
    Flags &= ~EndOfRow;
}

//
/// Returns true if this gadget is at the end of a row.
//
inline bool TGadget::IsEndOfRow() const 
{
  return Flags & EndOfRow;
}

//
/// Returns the next gadget in the list of gadgets.
//
inline TGadget* TGadget::NextGadget() 
{
  return Next;
}

/// Return a pointer to the owning or parent window for the gadget.
inline TGadgetWindow* TGadget::GetGadgetWindow()
{
	return Window;
}

/// Return a pointer to the owning or parent window for the gadget.
inline const TGadgetWindow* TGadget::GetGadgetWindow() const
{
	return Window;
}

/// Indicates whether the gadget width will be adjusted by the gadget window 
/// to be as wide as possible in the remaining space.
inline bool TGadget::IsWideAsPossible() const 
{
	return WideAsPossible;
}

/// Sets the flag that indicates whether the gadget width will be adjusted by the gadget window 
/// to be as wide as possible in the remaining space.
inline void TGadget::SetWideAsPossible(bool wide)
{
	WideAsPossible = wide;
}

/// Simply changes the border style without recalculating the gadget size
inline void TGadget::ChangeBorderStyle(TBorderStyle bs)
{
	BorderStyle = bs;
}



} // OWL namespace


#endif  // OWL_GADGET_H
