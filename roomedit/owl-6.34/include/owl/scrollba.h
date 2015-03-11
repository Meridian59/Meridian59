//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TScrollBar.  This defines the basic behavior of all
/// scrollbar controls.
//----------------------------------------------------------------------------

#if !defined(OWL_SCROLLBAR_H)
#define OWL_SCROLLBAR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>

namespace owl {

struct _OWLCLASS TScrollBarData;

#include <owl/preclass.h>

//
/// \class TScrollBar
/// TScrollBar objects represent standalone vertical and horizontal scroll bar
/// controls. Most of TScrollBar's member functions manage the scroll bar's sliding
/// box (thumb) position and range.
/// 
/// One special feature of TScrollBar is the notify-based set of member functions
/// that automatically adjust the scroll bar's thumb position in response to scroll
/// bar messages.
/// 
/// Never place TScrollBar objects in windows that have either the WS_HSCROLL or
/// WS_VSCROLL styles in their attributes.
//
class _OWLCLASS TScrollBar 
  : public TControl 
{
  public:

    TScrollBar(TWindow* parent, int id, int x, int y, int w, int h, bool isHScrollBar, TModule* module = 0);
    TScrollBar(TWindow* parent, int resourceId, TModule* module = 0);
    TScrollBar(THandle hWnd, TModule* module = 0);

    int    GetLineMagnitude() const;
    void   SetLineMagnitude(int linemagnitude);

    int    GetPageMagnitude() const;
    void   SetPageMagnitude(int pagemagnitude);

    /// \name Virtual functions to interface to scrollbars & derived
    /// @{
    virtual void  GetRange(int& minValue, int& maxValue) const;
    virtual int   GetPosition() const;
    virtual void  SetRange(int minValue, int maxValue, bool redraw = true);
    virtual void  SetPosition(int thumbPos, bool redraw = true);
    virtual int   DeltaPos(int delta);
    virtual void  GetScrollInfo(SCROLLINFO* info) const;
    virtual void  SetScrollInfo(SCROLLINFO* info, bool redraw = true);
    /// @}

    /// \name Called by EvHScroll and EvVScroll response table handlers
    /// These routines all end up calling SetPosition()
    /// @{
    virtual void  SBLineUp();
    virtual void  SBLineDown();
    virtual void  SBPageUp();
    virtual void  SBPageDown();
    virtual void  SBThumbPosition(int thumbPos);
    virtual void  SBThumbTrack(int thumbPos);
    virtual void  SBTop();
    virtual void  SBBottom();
    virtual void  SBEndScroll();
    /// @}

    /// \name Response table handlers that call above virtual functions in
    /// response to messages sent by to us by TWindow::DispatchScroll()
    /// @{
    void          EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    void          EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    /// @}

    /// Safe overload
    //
    void Transfer(TScrollBarData& data, TTransferDirection direction) {Transfer(&data, direction);}

  protected:

    // Override TWindow virtual member functions
    //
    virtual uint Transfer(void* buffer, TTransferDirection direction);
    virtual TGetClassNameReturnType GetClassName();
    virtual void SetupWindow();

  public_data:

    /// The number of range units to scroll the scroll bar when the user requests a
    /// small movement by clicking on the scroll bar's arrows. TScrollBar's constructor
    /// sets LineMagnitude to 1 by default. (The scroll range is 0-100 by default.)
    //
    int  LineMagnitude;
    
    /// The number of range units to scroll the scroll bar when the user requests a
    /// large movement by clicking in the scroll bar's scrolling area. TScrollBar's
    /// constructor sets PageMagnitude to 10 by default. (The scroll range is 0-100 by
    /// default.)
    //
    int  PageMagnitude;

  private:

    // Hidden to prevent accidental copying or assignment
    //
    TScrollBar(const TScrollBar&);
    TScrollBar& operator=(const TScrollBar&);

  DECLARE_RESPONSE_TABLE(TScrollBar);
  DECLARE_STREAMABLE_OWL(TScrollBar, 1);
};

DECLARE_STREAMABLE_INLINES(owl::TScrollBar);

//
// Scroll bar notification macros. methods are: void method()
//
// EV_SB_LINEDOWN(id, method)
// EV_SB_LINEUP(id, method)
// EV_SB_PAGEDOWN(id, method)
// EV_SB_PAGEUP(id, method)
// EV_SB_TOP(id, method)
// EV_SB_BOTTOM(id, method)
// EV_SB_THUMBPOSITION(id, method)
// EV_SB_ENDSCROLL(id, method)
// EV_SB_BEGINTRACK(id, method)

//
/// \struct TScrollBarData
/// The TScrollBarData structure contains integer values that represent a range of
/// thumb positions on the scroll bar. TScrollBar's function GetRange calls
/// TScrollBarData to obtain the highest and lowest thumb positions on the scroll
/// bar. GetPosition calls TScrollBarData to obtain the current thumb position on
/// the scroll bar.
//
struct _OWLCLASS TScrollBarData 
{
  int  LowValue; ///< Contains the lowest value of thumb position in the scroll bar's range.
  int  HighValue; ///< Contains the highest value of the thumb position in the scroll bar's range.
  int  Position; ///< Contains the scroll bar's thumb position.
};

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the current delta to move the thumb when line up/line down is received.
//
inline int TScrollBar::GetLineMagnitude() const
{
  return LineMagnitude;
}

//
/// Sets the delta to move the thumb when line up/line down is received.
//
inline void TScrollBar::SetLineMagnitude(int linemagnitude)
{
  LineMagnitude = linemagnitude;
}


} // OWL namespace


#endif  // OWL_SCROLLBAR_H
