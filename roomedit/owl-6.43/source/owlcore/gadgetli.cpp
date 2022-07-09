//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TGadgetList, the base for classes that own gadgets
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gadget.h>
#include <owl/gadgetwi.h>

namespace owl {

OWL_DIAGINFO;

TGadgetList::TGadgetList()
:
  Gadgets(0),
  NumGadgets(0)
{
}

//
//
//
TGadgetList::~TGadgetList()
{
  TGadget* gadget = Gadgets;

  while (gadget) {
    TGadget*  tmp = gadget;
    gadget = gadget->Next;
    delete tmp;
  }
}

//
/// Returns the gadget that a given window-relative point is in, or 0 if none is
/// found.
//
TGadget*
TGadgetList::GadgetFromPoint(const TPoint& point) const
{
  TGadget* gadget = Gadgets;
  for (; gadget; gadget = gadget->Next)
    if (gadget->PtIn(point - *(TSize*)&gadget->Bounds.TopLeft()))
      break;

  return gadget;
}

//
/// Returns the gadget with a given ID, or 0 if none is found.
//
TGadget*
TGadgetList::GadgetWithId(int id) const
{
  for (TGadget* g = Gadgets; g; g = g->NextGadget())
    if (g->GetId() == id)
      return g;
  return 0;
}

//
/// Returns gadget at a given index.
//
TGadget*
TGadgetList::operator [](uint index)
{
  PRECONDITION(index < NumGadgets);

  TGadget* g;
  for (g = FirstGadget(); index > 0; index--)
    g = NextGadget(*g);
  return g;
}

//
/// Inserts a Gadget. Caller also needs to call LayoutSession() after inserting
/// gadgets if this window has already been created.
///
/// You can specify a sibling Gadget that the new Gadget
/// is to be inserted before or after
///
/// If "sibling" is 0 then the new Gadget is inserted at the beginning or
/// the end. The default is to insert the new Gadget at the end
//
void
TGadgetList::Insert(TGadget& gadget, TPlacement placement, TGadget* sibling)
{
  PRECONDITION(!gadget.Window);  // Would cause problems if still in a window

  TGadget**  g = &Gadgets;

  // Locate spot to insert gadget
  //
  if (sibling || placement == After) {
    while (*g && *g != sibling)
      g = &(*g)->Next;

    CHECK(*g == sibling);
  }
  if (placement == After && *g)
    g = &(*g)->Next;

  // Link supplied gadget into this list
  //
  gadget.Next = *g;
  *g = &gadget;

  // Update count
  //
  NumGadgets++;

  Inserted(gadget);
}

//
/// Inserts a list of gadgets. Caller also needs to call LayoutSession() after
/// inserting gadgets if this window has already been created.
///
/// Gadgets are removed from the source list
//
void
TGadgetList::InsertFrom(TGadgetList& list, TPlacement placement, TGadget* sibling)
{
  if (!list.Gadgets)
    return;

  TGadget**  g = &Gadgets;

  // Locate spot to insert gadgets
  //
  if (sibling || placement == After) {
    while (*g && *g != sibling)
      g = &(*g)->Next;

    CHECK(*g == sibling);
  }
  if (placement == After && *g)
    g = &(*g)->Next;

  // Find tail of list. Let the source list & this list know about the transfer
  // as we go.
  //
  TGadget* last;
  for (TGadget* gd = list.Gadgets; gd; gd = gd->Next) {
    list.Removed(*gd);
    Inserted(*gd);
    last = gd;
  }

  // Link supplied list into this list
  //
  last->Next = *g;
  *g = list.Gadgets;

  // Update counts & source list pointer
  //
  NumGadgets += list.NumGadgets;
  list.Gadgets = 0;
  list.NumGadgets = 0;
}

//
/// Remove (unlinks) a gadget from this gadget window. The gadget is returned but
/// not destroyed. Returns 0 if gadget is not in this window. Caller also needs to
/// call LayoutSession() after inserting/removing gadgets if this gadget window has
/// already been created.
//
TGadget*
TGadgetList::Remove(TGadget& gadget)
{
  if (!Gadgets)
    return 0;

  // Handle head-of-list case
  //
  if (&gadget == Gadgets) {
    Gadgets = Gadgets->Next;
  }
  // Scan for gadget, looking one link ahead
  //
  else {
    TGadget* g = Gadgets;

    while (g->Next && g->Next != &gadget)
      g = g->Next;

    if (!g->Next)   // not found
      return 0;

    g->Next = g->Next->Next;
  }

  NumGadgets--;

  Removed(gadget);

  return &gadget;
}

//
/// A gadget has been inserted. Derived class can override this to update
/// internals
//
void
TGadgetList::Inserted(TGadget& /*gadget*/)
{
}

//
/// A gadget has been removed. Derived class can override this to update
/// internals
//
void
TGadgetList::Removed(TGadget& /*gadget*/)
{
}


} // OWL namespace

