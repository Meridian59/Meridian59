// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// dockingex.h:  header file
// Version:      1.6
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************
#ifndef __OWLEXT_DOCKINGEX_H
#define __OWLEXT_DOCKINGEX_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#ifndef   OWL_DOCKING_H
#include <owl/docking.h>
#endif
#ifndef   OWL_CONTAIN_H
#include <owl/contain.h>
#endif

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ****************** Forward declaration *************************************
class OWLEXTCLASS THarborEx;
class OWLEXTCLASS THarborManagement;

class OWLEXTCLASS TDockableControlBarEx;
class OWLEXTCLASS TFloatingSlipEx;
class OWLEXTCLASS TEdgeSlipEx;

struct TGridItem;
struct TGridLine;
struct TGridLayout;

// ****************** TDockableControlBarEx ***********************************

class OWLEXTCLASS TDockableControlBarEx : public owl::TDockableControlBar
{
  friend class THarborEx;

  public:
    TDockableControlBarEx(
      owl::TWindow* parent = 0,
      TTileDirection direction = Horizontal,
      owl::TFont* = nullptr,
      owl::TModule* module = 0);
    // default constructor

    TDockableControlBarEx(
      owl::uint id,              // id of toolbar, this should be a unique identifier
      LPCTSTR title,        // name of toolbar, should not be empty
      owl::TWindow* parent = 0,  // see default constructor
      bool _Default = true, // true, for toolbars, that comes out with your app
      bool hasHelp = false, // true, if there exist a helppage with helpid = id
      TTileDirection direction = Horizontal, // see default constructor
      owl::TFont* = nullptr,   // see default constructor
      owl::TModule* module = 0                    // see default constructor
    );
    // new constructor

    virtual ~TDockableControlBarEx();

    void SetDefaultGadgets(const owl::TIntArray& defaultGadgets);
    void SetDefaultGadgets(const int iArray[], owl::uint numGadgets);
    // Set the dafault gadgets with the id. All id's should be added in
    // GadgetsDescriptors. Best place to set the default gadgets is after
    // the toolbar is inserted in harbor

    void InsertDefaultGadgets();
    // Remove all existing gadgets and insert the default gadgets in the toolbar

#if (OWLInternalVersion < 0x06000000L)
    static void EnableFlatStyle(bool enable) { FlatStyle = enable; }
    static bool GetFlatStyle() { return FlatStyle; }
#else
    auto Create() -> bool override;
#endif

    virtual void Hide();
    virtual void Show();
    virtual void PreRemoved();
    // Called by TFloatingSlipEx or TEdgeSlipEx if the conrolbar will be removed

    auto WindowProc(owl::TMsgId, owl::TParam1, owl::TParam2) -> owl::TResult override;
    // Processes incoming messages when toolbar is hide

    void PaintGadgets(owl::TDC&, bool b, owl::TRect&) override;
    // Overridden to paint the actual customize gadget with a black rectangle

    void Paint(owl::TDC&, bool erase, owl::TRect&) override;
    // Overridden to paint the etched toolbar border and the gripper

    void ToggleSlip();
    // toggles it from docked position to floating position and back again

    bool IsDefault() { return Default; }
    void SetDefault(bool b) { Default = b; }

    bool GetHasHelp() { return HasHelp; }
    void SetHasHelp(bool b) { HasHelp = b; }

    virtual owl::TGadget* RemoveEx(owl::TGadget* gadget, bool del);
    virtual void InsertEx(owl::TGadget& gadget,
                          TPlacement placement = After,
                          owl::TGadget* sibling = 0);

    owl::TGadget* PrevGadget(owl::TGadget* gadget) const;
    // Returns previous Gadget

  protected:
    void SetupWindow() override;

    void RemoveAllGadgets();
    // Remove all gadgets from toolbar

    owl::TGadget* GetAndSetCustomizeGadget(owl::TPoint& point);
    // Get the actual customize gadget and set them in harbor or returns 0

    void CheckOnLastGadget();
    // If the controlbar has no visible gadgets a invisible gadget is inserted

    void CheckOnNeighbourSeparatorGadgets(owl::TGadget* actGadget);
    // Check the neighbours gadgets of actGadget. If they are seperator gadgets
    // that are not used they will be removed

    owl::TGadget* GetDragDropGadget(const owl::TPoint& pt, owl::TPoint& p1, owl::TPoint& p2,
      TPlacement& placement);


#if (OWLInternalVersion >= 0x06000000L)
    void EvOwlWindowDocked(owl::uint pos, const owl::TDockingSlip& slip);
#endif

    void EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void EvLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);
    void EvRButtonDown(owl::uint modKeys, const owl::TPoint& point);

    void StorePosAndLocation();
    void LoadSettings(THarborManagement& harborMan);
    void SaveSettings(THarborManagement& harborMan);

    void CmOk();
    void CmCancel();

    // Some internal draw functions
    void DrawVerticalDividers(owl::TDC& dc, owl::TGadget* firstgadg, owl::TGadget* lastgadg,
      int miny, int maxy, bool skipEndOfRowCheck = true);
    void DrawHorizontalDividers(owl::TDC& dc, owl::TGadget* firstgadg, owl::TGadget* lastgadg,
      int minx, int maxx);
    void DrawCustomizeRect(owl::TDC& dc, owl::TRect& rect);

  protected:
#if (OWLInternalVersion < 0x06000000L)
    static bool  FlatStyle;      // Draw gadgets in flat style (similar as OWL6)
#endif

    owl::TRect        Rect;           // Last position. Save when hide
    owl::TAbsLocation Location;       // Last location. Save when hide
    owl::TRect        ToggleRect;     // Position when toggle the slip view
    owl::TAbsLocation ToggleLocation; // Location when toggle the slip view
    bool         Default;        // Controlbar is a default bar
                                 // You can't delete or rename a default bar
    bool         HasHelp;        // Exist a context help for the controlbar
    owl::TIntArray    DefaultGadgets; // this gadgets will be shown by default in bar

  DECLARE_RESPONSE_TABLE(TDockableControlBarEx);
};

// ****************** TFloatingSlipEx *****************************************

class OWLEXTCLASS TFloatingSlipEx: public owl::TFloatingSlip
{
public:
  TFloatingSlipEx(owl::TWindow* parent, int x, int y, owl::TWindow* clientWnd = 0,
    bool shrinkToClient = true, int captionHeight = DefaultCaptionHeight,
    bool popupPalette = true, owl::TModule* module = 0);
  virtual ~TFloatingSlipEx();

  void DockableInsert(owl::TDockable&, const owl::TPoint* topLeft, owl::TRelPosition, owl::TDockable*) override;
  void DockableRemoved(const owl::TRect&) override;

protected:
  void EvNCLButtonDown(owl::uint hitTest, const owl::TPoint& point);
  void EvNCRButtonDown(owl::uint hitTest, const owl::TPoint& point);
  void EvLButtonDown(owl::uint hitTest, const owl::TPoint& point);
  void EvClose();
  void EvNCLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);

  DECLARE_RESPONSE_TABLE(TFloatingSlipEx);
};

// ****************** TEdgeSlipEx *********************************************

class OWLEXTCLASS TEdgeSlipEx : public owl::TWindow, public owl::TDockingSlip
{
  public:
    TEdgeSlipEx(owl::TDecoratedFrame& parent, owl::TAbsLocation location, owl::TModule* module = 0);
    virtual ~TEdgeSlipEx();

    void     SetGripperMargins(TDockableControlBarEx* cb);
    // Set the margins for the gripper

    // Overridden TWindow virtuals
    auto EvCommand(owl::uint id, THandle hWndCtl, owl::uint notifyCode) -> owl::TResult override;
    void EvCommandEnable(owl::TCommandEnabler& commandEnabler) override;
    void SetupWindow() override;

    // These are overriden for any window that wants to be notified on dockable
    // windows being moved or removed.
    void DockableInsert(owl::TDockable&, const owl::TPoint* topLeft = nullptr,
      owl::TRelPosition = owl::rpNone, owl::TDockable* = nullptr) override;
    void DockableMove(owl::TDockable&, const owl::TPoint* topLeft = nullptr,
      owl::TRelPosition position = owl::rpNone, owl::TDockable* = nullptr) override;
    void DockableRemoved(const owl::TRect&) override;
    auto GetLocation() const -> owl::TAbsLocation override;

  protected:
    // Response functions
    void     EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void     EvLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);
    owl::uint     EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize);
    void     EvNCPaint(HRGN);
    bool     EvEraseBkgnd(HDC);
    void     EvParentNotify(const owl::TParentNotify&);
    bool     EvWindowPosChanging(WINDOWPOS& windowPos);

    // Internal tiling functions
    void ArrangeAllDockables(owl::TWindow* NewDockable);
    void LayoutSlip(int Height);

  protected:
    // Internal tiling data
    owl::TAbsLocation Location;
    owl::TGridType    GridType;
    bool InsertingDockable; // To prevent parent notify layout during other layout

  DECLARE_RESPONSE_TABLE(TEdgeSlipEx);
};

// ****************** TGridItem ***********************************************

// Represents a single dockable in the slip
struct TGridItem
{
  // Constructor
  TGridItem(owl::TWindow* Window, owl::TAbsLocation Location);

  // Tiling functions - used to eliminate overlaps along a gridline
  void ArrangePrecedingItems();
  void ArrangeFollowingItems(int SlipWidth);
  void ArrangeLeft();
  void ArrangeRight(bool Limit, int SlipWidth);

  // Data members
  owl::TWindow*   Dockable;      // Dockable window which this item represents
  int        LeftEdge;      // Left or Top depending on orientation
  int        TopEdge;       // Top or Left depending on orientation
  int        Width;         // Width or Height depending on orientation
  int        Height;        // Height or Width depending on orientation
  TGridLine* GridLine;      // Used to retieve the top edge for this grid item
  TGridItem* NextItem;      // Pointer to next item on the same gridline
  TGridItem* PrevItem;      // Pointer to previous item on the same gridline
};

// ****************** TGridLine ***********************************************

// Represents a row of one or more dockables in the slip (linked list)
struct TGridLine
{
  // Constructor and destructor
  TGridLine(owl::uint32 Top);
  ~TGridLine();           // Delete all the grid items in the linked list

  // Tiling functions
  void ComputeHeight();   // Find height of the tallest grid item
  TGridItem* LayoutGridItems(TGridItem* Anchor, int SlipWidth);  // Horizontal layout - sets left edges mainly

  // List management functions
  void InsertGridItem(TGridItem* NewItem);  // Insert into a sorted linked list
  void RemoveGridItem(TGridItem* NewItem);  // Remove from the linked list

  // Data members
  int        TopEdge;     // All grid items share this top edge
  int        Height;      // Height of the tallest grid item
  TGridItem* FirstItem;   // Start of the linked list of grid items
  TGridLine* NextLine;    // Pointer to the next gridline
  TGridLine* PrevLine;    // Pointer to the previous gridline
};

// ****************** TGridLayout *********************************************

// Represents all rows of dockables in the slip (linked list)
struct TGridLayout
{
  // Constructor and destructor
  TGridLayout(owl::TAbsLocation Loc);
  ~TGridLayout();           // Delete all the grid lines in the linked list

  // Tiling functions
  void CreateLists(owl::TWindow* parent, owl::TWindow* dockable);  // Build linked lists
  int  LayoutGridLines();  // Vertical layout - sets top edge mainly
  void LayoutDockables();  // Move actual windows about

  // List management functions
  void InsertGridLine(TGridLine* NewLine);       // Insert into a sorted linked list
  TGridLine* InsertGridItem(TGridItem* NewItem); // Insert into a sorted linked list
  TGridLine* InsertDockableItem();               // Insert into a sorted linked list

  // Data members
  TGridLine* FirstLine;    // Start of the linked list of grid lines
  owl::TWindow*   Parent;       // The TBorderSlip which owns this object
  owl::TWindow*   Dockable;     // The toolbar just dropped on this edge slip
  TGridItem* DockableItem; // The grid item representing Dockable
  owl::TAbsLocation Location;   // The position of the TBorderSlip
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>
} // OwlExt namespace

#endif
