//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TDockable classes: TDockableGadgetWindow & TDockableControlBar
/// TDockingSlip classes: TFloatingSlip & TEdgeSlip
/// and the THarbor docking manager
//----------------------------------------------------------------------------
#if !defined(OWL_DOCKING_H)
#define OWL_DOCKING_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/decframe.h>
#include <owl/floatfra.h>
#include <owl/gadgetwi.h>
#include <owl/controlb.h>


namespace owl {

class _OWLCLASS THarbor;
class _OWLCLASS TDockingSlip;


#include <owl/preclass.h>

/// \addtogroup Docking
/// @{

/// General use 2-D rectangular relative position enum
//
/// Used here to described the location of a docking window [more specifically
/// the location of the EdgeSlip containing docking windows].
//
/// \note This enumeration corresponds to the TLocation enumeration of
///       TDecoratedFrame.
//
enum TRelPosition {
  rpNone,                       ///< No location specified
  rpAbove,                      ///< Refers to top edge of frame
  rpBelow,                      ///< Refers to bottom edge of frame
  rpRightOf,                    ///< Refers to left edge of frame
  rpLeftOf                      ///< Refers to right edge of frame
};

//
/// \class TDockable
// ~~~~~ ~~~~~~~~~
/// TDockable is an abstract base class for fixed or floating docking windows that
/// want to be dockable into docking slips.
//
class _OWLCLASS TDockable {
  public:

/// Returns the rectangle of the dockable window in the given location. The dockable
/// chooses its layout (horizontal, vertical or rectangle) in that position.
    virtual TSize ComputeSize(TAbsLocation loc, TSize* dim) = 0;

/// Return the rectangle of the dockable window in the given location. The dockable
/// chooses its layout (horizontal, vertical or rectangle) in that position.
    virtual void  GetRect(TRect& rect) = 0;

/// Changes the window to be equal to the size returned from ComputeNNNNN.
    virtual void  Layout(TAbsLocation loc, TSize* dim=0) = 0;

/// This is overriden for any window that wants to be docked. This routine is called
/// from an LButtonDown to determine if the mouse down should move the window or
/// perform an action within the docked window. For example, if a gadget window is
/// docked, this routine would decide if the mousedown is on an enabled gadget; if
/// so, the control is not to be moved. Instead, the action of the gadget being
/// depressed is performed. Otherwise, the gadget window is moved within the docking
/// window.
    virtual bool  ShouldBeginDrag(TPoint& pt) = 0;

/// Gets dockable's window and hides it, in case we have to toss it around a bit.
    virtual TWindow* GetWindow() = 0;

/// Returns the harbor containing the dockable object.
    virtual THarbor* GetHarbor() = 0;
};

//
/// \class TDockableGadgetWindow
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~
/// A version of TGadgetWindow which is dockable
//
class _OWLCLASS TDockableGadgetWindow : public TGadgetWindow, public TDockable {
  public:
    TDockableGadgetWindow(TWindow*        parent = 0,
                          TTileDirection  direction = Horizontal,
                          TFont*          font = 0,
                          TModule*        module = 0);
   ~TDockableGadgetWindow() override;

    // Override TDockable virtuals
    //
    auto ComputeSize(TAbsLocation, TSize*) -> TSize override;
    void GetRect(TRect&) override;
    void Layout(TAbsLocation, TSize* = nullptr) override;
    auto ShouldBeginDrag(TPoint&) -> bool override;
    auto GetWindow() -> TWindow* override;
    auto GetHarbor() -> THarbor* override;
    virtual TGadget* GetGripGadget();

    // Override TGadgetWindow virtuals
    //
    void LayoutSession() override;

  protected:
    void          EvLButtonDown(uint modKeys, const TPoint& point);
    void          EvOwlWindowDocked(uint pos, const TDockingSlip& slip);
    bool          EvSetCursor(HWND hWndCursor, uint codeHitTest, TMsgId mouseMsg);

    /// Returns the layout direction to use when placed in a slip
    /// This can be overridden to provide different docking behaviour
    //
    virtual TTileDirection GetDirectionForLocation(TAbsLocation loc);

    TCursor*      Cursor;
  DECLARE_RESPONSE_TABLE(TDockableGadgetWindow);
};

//
/// \class TDockableControlBar
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
/// A control bar class that is dockable
//
class _OWLCLASS TDockableControlBar : public TDockableGadgetWindow {
  public:
    TDockableControlBar(TWindow*        parent = 0,
                        TTileDirection  direction = Horizontal,
                        TFont*          font = 0,
                        TModule*        module = 0);

  protected:
    void AdjustMargins();
    void EvOwlWindowDocked(uint pos, const TDockingSlip& slip);

  DECLARE_RESPONSE_TABLE(TDockableControlBar);
  DECLARE_CASTABLE;
};

//
/// \class TDockingSlip
// ~~~~~ ~~~~~~~~~~~~
/// TDockingSlip is an abstract base class for windows which accepts and holds
/// dockable windows.
//
class _OWLCLASS TDockingSlip {
  public:
    // These are overriden for any window that wants to be notified on dockable
    // windows being moved or removed.
    //
/// Inserts the dockable into the slip, based on arguments given.
    virtual void DockableInsert(TDockable& dockable, const TPoint* topLeft=0,
                                TRelPosition position=rpNone, TDockable* relDockable=0) = 0;

    virtual void DockableMove(TDockable& dockable, const TPoint* topLeft=0,
                              TRelPosition position=rpNone, TDockable* relDockable=0) = 0;

    virtual void DockableRemoved(const TRect& orgRect) = 0;

    virtual TAbsLocation GetLocation() const = 0;

    // Called by Harbor to initialize back pointer
    //
    void     SetHarbor(THarbor* harbor);
    THarbor* GetHarbor() const;

  protected:
    THarbor* Harbor;
};

//
/// \class TFloatingSlip
// ~~~~~ ~~~~~~~~~~~~~
/// TFloatingSlip is a floating frame which can hold a dockable window. It is also
/// dockable in order to act as a proxy for its held dockable when dragging.
//
class _OWLCLASS TFloatingSlip : public TFloatingFrame,
                                public TDockingSlip, public TDockable
{
  public:
    TFloatingSlip(TWindow*        parent,
                  int x, int y,
                  TWindow*        clientWnd = 0,
                  bool            shrinkToClient = true,
                  int             captionHeight = DefaultCaptionHeight,
                  bool            popupPalette = true,  // false?
                  TModule*        module = 0);

    // Override TDockingSlip virtuals
    //
    void DockableInsert(TDockable&, const TPoint* topLeft, TRelPosition, TDockable* relDockable) override;
    void DockableMove(TDockable&, const TPoint* topLeft, TRelPosition, TDockable* relDockable) override;
    void DockableRemoved(const TRect&) override;

    auto GetLocation() const -> TAbsLocation override;

    // Override TDockable virtuals
    //
    auto ComputeSize(TAbsLocation, TSize*) -> TSize override;
    void GetRect(TRect&) override;
    void Layout(TAbsLocation, TSize* = nullptr) override;
    auto ShouldBeginDrag(TPoint&) -> bool override;
    auto GetWindow() -> TWindow* override;
    auto GetHarbor() -> THarbor* override;

  protected:
    void    EvNCLButtonDown(uint hitTest, const TPoint& point);
    void    EvLButtonDown(uint hitTest, const TPoint& point);
    bool    EvSizing(uint side, TRect& rect);
    bool    EvWindowPosChanging(WINDOWPOS & windowPos);
    void    EvWindowPosChanged(const WINDOWPOS& windowPos);
    void    EvClose();
    void    EvSettingChange(uint, LPCTSTR);
    void    EvGetMinMaxInfo(MINMAXINFO & info);

  DECLARE_RESPONSE_TABLE(TFloatingSlip);
};

enum TGridType {   ///< Grid type corresponds with Location:
  YCoord,            ///< Top & bottom edge have Y coords parallel horiz
  XCoord,            ///< Left & right edge have X coords parallel vertically
};

//
/// \class TEdgeSlip
// ~~~~~ ~~~~~~~~~
/// TEdgeSlip is the class of windows used by THarbor as the actual docking slips
/// along the decorated frame client's edges.
//
class _OWLCLASS TEdgeSlip : public TWindow, public TDockingSlip {
  public:
    TEdgeSlip(TDecoratedFrame& parent, TAbsLocation location, TModule* module = 0);

    // Overridden TWindow virtuals
    //
    auto EvCommand(uint id, THandle hWndCtl, uint notifyCode) -> TResult override;
    void EvCommandEnable(TCommandEnabler& commandEnabler) override;
    void SetupWindow() override;

    // Overridden TDockingSlip virtuals
    //
    void DockableInsert(TDockable& dockable, const TPoint* topLeft,
      TRelPosition position, TDockable* relDockable) override;
    void DockableMove(TDockable& dockable, const TPoint* topLeft,
      TRelPosition position, TDockable* relDockable) override;
    void DockableRemoved(const TRect& orgRect) override;
    auto GetLocation() const -> TAbsLocation override;

    /// Called by Harbor when creating this docking window on a drop
    // !CQ fairly kludgy. Should be handled by above docking slip handlers
    //
    void     ReLayout(bool forcedLayout);

  protected:
    void     EvLButtonDown(uint modKeys, const TPoint& point);
    void     EvLButtonDblClk(uint modKeys, const TPoint& point);
    uint     EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize);

    void     EvNCPaint(HRGN);

    bool     EvEraseBkgnd(HDC);
    void     EvParentNotify(const TParentNotify&);
    bool     EvWindowPosChanging(WINDOWPOS & windowPos);

    // Internal dockable tiling support
    //
    void     CompressGridLines();
    void     CompressParallel(int width);
    TSize    ComputeDockingArea();

    TWindow* MoveDraggedWindow(TWindow* draggedWindow);
    void     MoveAllOthers(TWindow* draggedWindow, TWindow* movedWindow);

    int      GridSize(int baseCoord);

  protected_data:
    TAbsLocation    Location;     ///< Location on the frame
    TGridType       GridType;     ///< Type of grid

  DECLARE_RESPONSE_TABLE(TEdgeSlip);
};

//
/// \class THarbor
// ~~~~~ ~~~~~~~
/// THarbor is the object that holds all the docking slips. It performs the actual
/// docking insertion and coordination. It is never visible; it is a window in order
/// to capture the mouse.
//
class _OWLCLASS THarbor : public TWindow {
  public:
    THarbor(TDecoratedFrame& df);
   ~THarbor() override;

    /// Dockable window insertion
    //
    TDockingSlip*   Insert(TDockable& dockable,
                           TAbsLocation location,
                           const TPoint* where = 0,
                           TRelPosition position = rpNone,
                           TDockable* relativeTo = 0);

    /// Move a dockable from one slip to another, programatically
    //
    void            Move(TDockable& dockable,
                         TAbsLocation location,
                         const TPoint* where = 0,
                         TRelPosition position = rpNone,
                         TDockable* relativeTo = 0);

    /// Remove a dockable from the harbor
    //
    void            Remove(TDockable& dockable);

    /// Called by docking slips
    //
    bool            DockDraggingBegin(TDockable& dockable, const TPoint& pt,
                                      TAbsLocation location,
                                      TDockingSlip* dockingNotify = 0);

  protected:
    TEdgeSlip*      GetEdgeSlip(TAbsLocation location);
    void            SetEdgeSlip(TAbsLocation location, TEdgeSlip* slip);
    TRelPosition    GetSlipPosition(TAbsLocation location);

    // Factory members - could be overridden by derived THarbor class
    //
    virtual TEdgeSlip* ConstructEdgeSlip(TDecoratedFrame& df,
                                         TAbsLocation location);
    virtual TFloatingSlip* ConstructFloatingSlip(TDecoratedFrame& df,
                                                 int x, int y,
                                                 TWindow* dockableWindow);

    // Mouse handlers - invoked when harbor grabs capture
    //
    void     EvMouseMove(uint modKeys, const TPoint& point);
    void     EvLButtonUp(uint modKeys, const TPoint& point);
    void     EvLButtonDblClk(uint modKeys, const TPoint& point);

//  private:
  protected: //Sirma (Krasi)
    TDecoratedFrame&  DecFrame;

    // Docking slip windows
    //
    TEdgeSlip*   SlipTop;
    TEdgeSlip*   SlipLeft;
    TEdgeSlip*   SlipBottom;
    TEdgeSlip*   SlipRight;

    // Dockable dragging state. Put into a struct packet
    //
    TDockable*    DragDockable;
    TWindowDC*    DragDC;
    TAbsLocation  DragOrgSlipLoc;  ///< Original slip location on mouseDown.
    TAbsLocation  DragSlipLoc;
    TRect         DragFrame;
    TSize         DragVSize;       ///< Size of dockable when vertical
    TSize         DragHSize;       ///< Size of dockable when horizontal
    TSize         DragNSize;       ///< Size of dockable when a natural shape
    TPoint        DragAnchor;
    TDockingSlip* DragNotificatee; ///< Slip notified on drag operations

    TRect         SlipTR;
    TRect         SlipBR;
    TRect         SlipLR;
    TRect         SlipRR;

  DECLARE_RESPONSE_TABLE(THarbor);
};
/// @}
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Set the back pointer to the harbor.
//
inline void TDockingSlip::SetHarbor(THarbor* harbor)
{
  Harbor = harbor;
}

//
/// Retrieve the associated harbor.
//
inline THarbor* TDockingSlip::GetHarbor() const
{
  return Harbor;
}

} // OWL namespace


#endif  // OWL_DOCKING_H
