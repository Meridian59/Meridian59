//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TDragList
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/draglist.h>

namespace owl {

OWL_DIAGINFO;

// Let the compiler know that the following template instances will be defined elsewhere.
//#pragma option -Jgx

DEFINE_RESPONSE_TABLE1(TDragList, TListBox)
  EV_REGISTERED(DRAGLISTMSGSTRING, DragNotify),
END_RESPONSE_TABLE;

//
/// Constructor for creating a drag list dynamically.
//
TDragList::TDragList(TWindow* parent, int id, int x, int y, int w, int h,
                     TModule* module)
:
  TListBox(parent, id, x, y, w, h, module)
{
}

//
/// Constructor for creating a drag list from a resource.
//
TDragList::TDragList(TWindow* parent, int resourceId, TModule* module)
:
  TListBox(parent, resourceId, module)
{
}

//
/// SetupWindow for the drag listbox must call MakeDragList().
//
void
TDragList::SetupWindow()
{
  // Call base class
  //
  TListBox::SetupWindow();

  if (TCommCtrl::IsAvailable())
    TCommCtrl::Dll()->MakeDragList(*this);
}

//
/// Handle the DRAGLISTMSGSTRING notification by calling virtual functions
/// based on the notification message.
//
/// Responds to the DL_BEGINDRAG, DL_DRAGGING, DL_DROPPED, and DL_CANCELDRAG
/// messages by calling the BeginDrag, Dragging, Dropped, and CancelDrag functions
/// respectively.
//
TResult
TDragList::DragNotify(TParam1, TParam2 lp)
{
  DRAGLISTINFO * info = (DRAGLISTINFO *)lp;
  if (info) {
    TPoint p = info->ptCursor;
    int item = ItemFromPoint(p);

    switch (info->uNotification) {
      case DL_BEGINDRAG:
        return BeginDrag(item, p);

      case DL_DRAGGING:
        return Dragging(item, p);

      case DL_DROPPED:
        Dropped(item, p);
        break;

      case DL_CANCELDRAG:
        CancelDrag(item, p);
        break;

      default:
        // Should not ever happen.
        ///TH perhaps throw an exception?
        break;
    }
  }
  return 0;
}

//
/// The drag UI has started. Return true to allow drag.
//
bool
TDragList::BeginDrag(int, const TPoint&)
{
  return false;
}

//
/// User has moved the mouse. Return the type of cursor to represent the allowable
/// action.
//
TDragList::TCursorType
TDragList::Dragging(int, const TPoint&)
{
  return dlStop;
}

//
/// User has dropped the item.
//
void
TDragList::Dropped(int, const TPoint&)
{

}

//
/// User has cancelled the drag.
//
void
TDragList::CancelDrag(int, const TPoint&)
{

}

//
/// Draws the drag cursor.
//
void
TDragList::DrawInsert(int item)
{
  if (TCommCtrl::IsAvailable())
    TCommCtrl::Dll()->DrawInsert(*GetParentO(), *this, item);
}

//
/// Retrieve the item from the specified point. Return -1 if the point is not on an
/// item. 'scroll' determines whether the listbox will scroll if the point is above
/// or below the listbox.
//
int
TDragList::ItemFromPoint(const TPoint& p, bool scroll)
{
  if (TCommCtrl::IsAvailable())
    return TCommCtrl::Dll()->LBItemFromPt(*this, p, scroll);

  return -1;
}

//----------------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TDragListEventHandler, TEventHandler)
  EV_REGISTERED(DRAGLISTMSGSTRING, DragNotify),
END_RESPONSE_TABLE;

//
/// Forward the drag notification messages from the parent window
/// to the drag listbox for it to handle.
//
TResult
TDragListEventHandler::DragNotify(TParam1 wp, TParam2 lp)
{
  DRAGLISTINFO * info = (DRAGLISTINFO *)lp;
  if (info) {
    // Forward messages from parent to listbox
    //
    return ::SendMessage(info->hWnd,
                         ::RegisterWindowMessage(DRAGLISTMSGSTRING),
                         wp, lp);
  }
  return 0;
}

} // OWL namespace
/* ========================================================================== */

