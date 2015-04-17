//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Streamable object implementation for TWindow.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/applicat.h>
#include <owl/appdict.h>
#include <owl/window.h>
#include <owl/scroller.h>

namespace owl {

OWL_DIAGINFO;

IMPLEMENT_STREAMABLE(TWindow);

const int StreamIsTop      = 1;
const int StreamIsTopChild = 2;

#if !defined(BI_NO_OBJ_STREAMING)

//
// Input
//
void *
TWindow::Streamer::Read(ipstream& is, uint32 version) const
{
  TWindow* o = GetObject();
  int flags;
  is >> flags;
  if (flags & StreamIsTop) {
    o->ChildList = 0;   // indicate no children connected yet
    return o;           // we only need to read our child list
  }

  o->Handle = 0;
  o->Parent = 0;
  o->SiblingList = 0;
  o->ChildList = 0;
  o->TransferBuffer = 0;
  o->DefaultProc = 0;
  o->HAccel = 0;
  o->ContextPopupMenu = 0;
  o->SetUniqueId();

  is >> o->Module;
  TResId TempId;
  is >> TempId;
  o->Title = const_cast<LPTSTR>(static_cast<LPCTSTR>(TempId));
  is >> o->Flags;

  if (o->IsFlagSet(wfFromResource)) {
    o->DefaultProc = (WNDPROC)::DefWindowProc;
    memset(&o->Attr, 0, sizeof(o->Attr));
  }
  else 
  {
    UINT_PTR p = reinterpret_cast<UINT_PTR>(o->Attr.Param);
    is >> o->Attr.Style >> o->Attr.ExStyle 
      >> o->Attr.X >> o->Attr.Y >> o->Attr.W >> o->Attr.H >> p;
    o->Attr.Param = reinterpret_cast<LPVOID>(p);
    o->DefaultProc = ::DefWindowProc;
  }

  is >> o->Attr.Id
     >> o->Attr.Menu
     >> o->Attr.AccelTable;

  is >> o->ZOrder;

  is >> o->Parent;

  if (o->Parent) {
    o->Application = o->Parent->GetApplication();

    // Version 1 and version 3 sibling streaming techniques
    //
    if (version == 1) {
      if (flags & StreamIsTopChild)
        o->Parent->ChildList = o;  // set parent's child pointer to this

      is >> o->ChildList;
      is >> o->SiblingList;
    }
    else {
      o->Parent->AddChild(o);

      static bool readSiblings = true;
      bool saveReadSiblings = readSiblings;
      readSiblings = true;
      is >> o->ChildList;
      readSiblings = saveReadSiblings;

      if (readSiblings) {
        readSiblings = false;

        unsigned numSiblings;
        is >> numSiblings;
        for (unsigned i = 0; i < numSiblings; i++) {
          TWindow* sibling;
          is >> sibling;
        }
        readSiblings = true;
      }
    }
  }
  else {
    o->Application = TYPESAFE_DOWNCAST(o->Module,TApplication);
    if (!o->Application)
      o->Application = OWLGetAppDictionary().GetApplication(0);
  }

  is >> o->Scroller;
  if (o->Scroller)
    o->Scroller->SetWindow(o);

  o->HCursor = 0;
  is >> o->CursorModule >> o->CursorResId;
  o->SetCursor(o->CursorModule, o->CursorResId);
  is >> o->BkgndColor;  // !CQ need streamer for TColor

  o->InstanceProc = o->CreateInstanceProc();

  return o;
}

//
// Output
//
void
TWindow::Streamer::Write(opstream& os) const
{
  TWindow* o = GetObject();

  o->AssignZOrder();
  int flags = 0;
  if (o->IsFlagSet(wfStreamTop) || o->IsFlagSet(wfMainWindow))
    flags |= StreamIsTop;
  else if (o->Parent) {
    if ((o->Parent->IsFlagSet(wfStreamTop) || o->Parent->IsFlagSet(wfMainWindow))
      && o->Parent->ChildList == o)
    flags |= StreamIsTopChild;
  }
  os << flags;

  if (flags & StreamIsTop)
    return;

  os << o->Module;
  os << TResId(o->Title);

  uint32 saveFlags = o->Flags;
  if (o->GetHandle())
    saveFlags |= wfAutoCreate;
  os << saveFlags;

  if (!o->IsFlagSet(wfFromResource)) {
    uint32 saveStyle = o->Attr.Style &
                      ~(WS_MINIMIZE | WS_MAXIMIZE | WS_DISABLED | WS_VISIBLE);

    if (o->GetHandle())
      saveStyle |= o->GetWindowLong(GWL_STYLE) &
                   (WS_MINIMIZE | WS_MAXIMIZE | WS_DISABLED | WS_VISIBLE);

    const UINT_PTR p = reinterpret_cast<UINT_PTR>(o->Attr.Param);
    os << saveStyle << o->Attr.ExStyle 
      << o->Attr.X << o->Attr.Y << o->Attr.W << o->Attr.H << p;
  }

  os << o->Attr.Id
     << o->Attr.Menu
     << o->Attr.AccelTable;

  os << o->ZOrder;

  os << o->Parent;

#if 0  // (TWindow::Streamer::ClassVersion() == 1)
  os << o->ChildList;
  os << o->SiblingList;

#else  // version >= 3
  if (o->Parent) {
    static bool writeSiblings = true;
    bool saveWriteSiblings = writeSiblings;
    writeSiblings = true;
    os << o->ChildList;
    writeSiblings = saveWriteSiblings;

    if (writeSiblings) {
      writeSiblings = false;

      os << (o->Parent->NumChildren()-1);
      for (TWindow* sibling = o->SiblingList; sibling != o; sibling = sibling->Next())
        os << sibling;
      writeSiblings = true;
    }
  }

#endif

  os << o->Scroller;

  os << o->CursorModule << o->CursorResId;
  os << o->BkgndColor;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

