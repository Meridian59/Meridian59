//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TDragList, a listbox that has dragging capabilities for
/// items.
//
// The owner of a TDragList should have in its response table:
//  EV_REGISTERED(DRAGLISTMSGSTRING, xxx)
//----------------------------------------------------------------------------

#if !defined(OWL_DRAGLIST_H)
#define OWL_DRAGLIST_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>

#include <owl/eventhan.h>
#include <owl/listbox.h>
#include <owl/commctrl.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{
/// \class TDragList
// ~~~~~ ~~~~~~~~~
/// TDragList is a draggable list box. A draggable list box is a list box which can
/// be used to drag items to change their position. The program responds to drag
/// events accordingly. Typical usage includes a list of fields that can be
/// reordered.
//
class _OWLCLASS TDragList : public TListBox {
  public:

    /// Enumeration used to specify the type of cursor to be displayed
    /// during a drag operation. The cursor provides feedback to the user
    /// about whether the object being dragged can be dropped and the
    /// operation resulting from the drop.
    //
    enum TCursorType {
      dlStop = DL_STOPCURSOR,   ///< stop cursor - item cannot be dropped now
      dlCopy = DL_COPYCURSOR,   ///< copy cursor - item dragged will be copied
      dlMove = DL_MOVECURSOR,   ///< move cursor - item dragged will be moved
    };

    // Constructors
    //
    TDragList(TWindow*       parent,
             int             id,
             int x, int y, int w, int h,
             TModule*        module = 0);

    TDragList(TWindow* parent, int resourceId, TModule* module = 0);

    // Override these virtual functions for handling dragging events
    //
    virtual bool        BeginDrag(int item, const TPoint& point);
    virtual TCursorType Dragging(int item, const TPoint& point);
    virtual void        Dropped(int item, const TPoint& point);
    virtual void        CancelDrag(int item, const TPoint& point);

    // Wrappers for the common control functions
    //
    void DrawInsert(int item);
    int  ItemFromPoint(const TPoint& p, bool scroll = true);

  protected:
    void SetupWindow();

  private:
    TResult DragNotify(TParam1, TParam2);

  DECLARE_RESPONSE_TABLE(TDragList);
};

//
/// \class TDragListEventHandler
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~
/// A TEventHandler mix-in.
/// This class is designed to handle the drag list notifications and
/// forward the messages from the parent window to the TDragList class.
//
class _OWLCLASS TDragListEventHandler : virtual public TEventHandler {
  public:
    TResult DragNotify(TParam1, TParam2);

  DECLARE_RESPONSE_TABLE(TDragListEventHandler);
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_DRAGLIST_H

