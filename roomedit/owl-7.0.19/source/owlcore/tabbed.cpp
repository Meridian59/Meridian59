//------------------------------------------------------------------------------
//  ObjectWindows
//  Copyright (c) 1998 by Bidus Yura. All Rights Reserved.
//
/// \file
///  Source file for implementation of TTabbedBase.
///  Added dynamic Add/DeletePage. Contributed by David Linch.
//------------------------------------------------------------------------------
//@doc TTabbedBase
#include <owl/pch.h>

#include <owl/defs.h>
#include <owl/docmanag.h>
#include <owl/layoutwi.h>
#include <owl/tabbed.h>

OWL_DIAGINFO;

namespace owl {

static void setWindowLayoutMetrics(TLayoutMetrics& lm, TNoteTab* tab,
                             TTabbedBase::TTabPosition pos)
{
  if(pos == TTabbedBase::tpLeft){
    lm.X.LeftOf(tab, 0);
    lm.Y.SameAs(lmParent, lmTop);
    lm.Width.SameAs(lmParent, lmWidth);
    lm.Height.SameAs(lmParent, lmHeight);
  }
  else if(pos == TTabbedBase::tpRight){
    lm.X.SameAs(lmParent, lmLeft);
    lm.Y.SameAs(lmParent, lmTop);
    lm.Width.RightOf(tab);
    lm.Height.SameAs(lmParent, lmHeight);
  }
  else if(pos == TTabbedBase::tpTop){
    lm.X.Below(tab);
    lm.Y.SameAs(lmParent, lmTop);
    lm.Width.SameAs(lmParent, lmWidth);
    lm.Height.SameAs(lmParent, lmHeight);
  }
  else if(pos == TTabbedBase::tpBottom){
    lm.X.SameAs(lmParent, lmLeft);
    lm.Y.SameAs(lmParent, lmTop);
    lm.Width.SameAs(lmParent, lmRight);
    lm.Height.Above(tab);
  }
}

////////////////////////////////////////////////////////////////////////////////
// TTabbedBase
// ~~~~~~~~~~~
DEFINE_RESPONSE_TABLE1(TTabbedBase,   TEventHandler)
  EV_TCN_SELCHANGE(IDC_TABCONTROL,     EvTabControlSelChange),
  EV_TCN_SELCHANGING(IDC_TABCONTROL,  EvTabControlSelChanging),
END_RESPONSE_TABLE;

// -----------------------------------------------------------------------------
TTabbedBase::TTabbedBase(TWindow* self, int x, int y, int w, int h, TTabPosition pos, TNoteTab* tab)
:
  Position(pos)
{
   NoteTab = tab;
  if(!NoteTab)
    NoteTab = new TNoteTab(self, IDC_TABCONTROL, x,y,w,h);
}
// -----------------------------------------------------------------------------
TTabbedBase::TTabbedBase(TWindow* self, TTabPosition pos, TNoteTab* tab)
:
  Position(pos)
{
   NoteTab = tab;
  if(!NoteTab)
    NoteTab = new TNoteTab(self, IDC_TABCONTROL);
}
// -----------------------------------------------------------------------------
/// Add a new tab page.
/// \note Use before the Tab window is created!
int TTabbedBase::Add(TWindow& wnd, LPCTSTR titles)
{
  LPCTSTR text = titles ? titles : wnd.GetCaption();
  if (!text)
    text = _T("");

  int index = NoteTab->Add(text, INT_PTR((TWindow*)&wnd));
#if 0  // this code now in AddPage()
  // if already constructed
  if(NoteTab->GetHandle() && index >=0 ){
    TLayoutWindow* layoutWnd = TYPESAFE_DOWNCAST(this, TLayoutWindow);
    if(layoutWnd){
      TLayoutMetrics lmWindow;
      setWindowLayoutMetrics(lmWindow, NoteTab, Position);
      if(!wnd.GetHandle())
        wnd.Create();

      layoutWnd->SetChildLayoutMetrics(wnd, lmWindow);
      layoutWnd->Layout();
    }
    else{
      if(!wnd.GetHandle())
        wnd.Create();
      AdjustPage(wnd);
    }
    wnd.ShowWindow(SW_SHOWNORMAL);

  }
#endif
  return index;
}
// -----------------------------------------------------------------------------
void TTabbedBase::AdjustPage(TWindow& page)
{
  // assumes that note tab positioning
  TWindow* parent = NoteTab->GetParentO();

  // Retrieve area of tab (in terms of tabbed window)
  TRect tRect;
  NoteTab->GetWindowRect(tRect);
  ::MapWindowPoints(HWND_DESKTOP, *parent, LPPOINT(&tRect), 2);
  TRect pgRect;
  page.GetWindowRect(pgRect);
  ::MapWindowPoints(HWND_DESKTOP, *parent, LPPOINT(&pgRect), 2);
  pgRect.left = pgRect.top   = 0;

  switch(Position){
    case tpLeft:
      pgRect += TPoint(tRect.Width(), 0);
      break;
    case tpRight:
       if(pgRect.right > tRect.left)
         pgRect.right = tRect.left;
      break;
    case tpTop:
       pgRect += TPoint(0, tRect.Height());
      break;
    case tpBottom:
       if(pgRect.bottom > tRect.top)
         pgRect.bottom = tRect.top;
      break;
  }
  page.SetWindowPos(HWND_TOP, pgRect, SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOZORDER);
}
// -----------------------------------------------------------------------------
void TTabbedBase::SetupPages()
{
  // first set all titles
  TWindow* wnd;
  TNoteTabItem Item;
  int count = NoteTab->GetCount();
  for(int i = 0; i< count; i++){
    NoteTab->GetItem(i, Item);
    wnd = ((TWindow*)Item.ClientData);
    if (!wnd->IsWindow())
    {
      wnd->Create();
//Bug#1456799 Jogy (24.3.2006)
    wnd->ShowWindow(SW_HIDE);
    }

    // it will be here only for dialog ????
    if(Item.Label.empty()){
      tchar title[MAX_PATH];
       wnd->GetWindowText(title, MAX_PATH);
      Item.Label = title;
      NoteTab->SetItem(i, Item);
    }
    // Adjust child position
    AdjustPage(*wnd);
  }
  if(count){
    int Selected = NoteTab->GetSel();
    NoteTab->GetItem(Selected, Item);
    wnd = (TWindow*)Item.ClientData;
    // Show the newly selected tab, if any.
    if(wnd)
      wnd->ShowWindow(SW_SHOWNORMAL);
  }
}
// -----------------------------------------------------------------------------
TWindow* TTabbedBase::GetPage(int index)
{
  if(index == -1)
    index = NoteTab->GetSel();
  TNoteTabItem Item;
  NoteTab->GetItem(index, Item);

  return (TWindow*)Item.ClientData;
}

//
/// Hides the current page and shows the page at the given index.
/// Does nothing if the page at the given index is already selected.
/// Note that this function does not send any notifications.
//
void TTabbedBase::SelectPage(int index, bool shouldFocusPage)
{
  PRECONDITION(index >= 0 && index < NoteTab->GetCount());
  if (index == NoteTab->GetSel()) return;

  if (const auto currentPage = GetPage())
    currentPage->ShowWindow(SW_HIDE);
  NoteTab->SetSel(index);
  const auto newPage = GetPage();
  newPage->ShowWindow(SW_SHOWNORMAL);
  if (shouldFocusPage)
    newPage->SetFocus();
}

void TTabbedBase::EvTabControlSelChange(TNotify&)
{
  if (const auto w = GetPage())
    w->ShowWindow(SW_SHOWNORMAL);
}

bool TTabbedBase::EvTabControlSelChanging(TNotify &)
{
  if (const auto w = GetPage())
    w->ShowWindow(SW_HIDE);
  return false; // Don't veto the change.
}

//////////////////////////////////////////////////////////////
//
//
DEFINE_RESPONSE_TABLE2(TTabbedWindow, TLayoutWindow, TTabbedBase)
END_RESPONSE_TABLE;

TTabbedWindow::TTabbedWindow(TWindow* parent, LPCTSTR title, TModule* module)
:
  TLayoutWindow(parent, title, module),
  TTabbedBase(this,0,0,0,0)
{
  Init();
}

TTabbedWindow::TTabbedWindow(TWindow* parent, const tstring& title, TModule* module)
  : TLayoutWindow(parent, title, module),
  TTabbedBase(this,0,0,0,0)
{
  Init();
}

void TTabbedWindow::Init()
{
  SetBkgndColor(TColor::Transparent); // No erase; see TWindow::EvEraseBkgnd.
  ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS); // Clipping reduces resize flicker.
}

//
bool TTabbedWindow::Create()
{
  uint count = GetTabControl()->GetCount();
  uint i;
  for(i = 0; i < count; i++){
    TWindow* wnd = GetPage(i);
    wnd->ModifyStyle(WS_VISIBLE, 0);
  }

  // Calculate tab control height and layout.
  //
  uint height = GetTabControl()->GetMinimalHeight() + 1;
  TLayoutMetrics lmWindow;
  TLayoutMetrics lmNoteTab;
  setWindowLayoutMetrics(lmWindow, GetTabControl(), Position);
  switch(Position){
    case tpLeft:
      lmNoteTab.X.SameAs(lmParent, lmLeft);
      lmNoteTab.Y.SameAs(lmParent, lmTop);
      lmNoteTab.Width.Absolute(lmRight, height);
      lmNoteTab.Width.Units = lmPixels;
      lmNoteTab.Height.SameAs(lmParent, lmHeight);
      break;
    case tpRight:
      lmNoteTab.X.SameAs(lmParent, lmRight);
      lmNoteTab.Y.SameAs(lmParent, lmTop);
      lmNoteTab.Height.SameAs(lmParent, lmHeight);
      lmNoteTab.Width.Absolute(lmLeft, height);
      lmNoteTab.Width.Units = lmPixels;
      break;
    case tpTop:
      lmNoteTab.X.SameAs(lmParent, lmLeft);
      lmNoteTab.Y.SameAs(lmParent, lmTop);
      lmNoteTab.Width.SameAs(lmParent, lmWidth);
      lmNoteTab.Height.Absolute(lmBottom, height);
      lmNoteTab.Height.Units = lmPixels;
      break;
    case tpBottom:
      lmNoteTab.X.SameAs(lmParent, lmLeft);

      // Needed an adjustment of -1 here, otherwise there is a 1 pixel gap
      // below the tab control.
      //
      // TODO: Determine if this is a bug in TLayoutWindow,  or whether this
      // bottom pixel exclusion is indeed correct.
      //
      lmNoteTab.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, -1);
      lmNoteTab.Height.Absolute(height);
      lmNoteTab.Height.Units = lmPixels;
      lmNoteTab.Width.SameAs(lmParent, lmRight);
      break;
  }
  SetChildLayoutMetrics(*GetTabControl(), lmNoteTab);

  for(i = 0; i < count; i++)
    SetChildLayoutMetrics(*GetPage(i), lmWindow);

  TLayoutWindow::Create();

  SetupPages();

  return true;
}
//
/// Add a new tab page.
///\note Need at least one page in order to add a page. The initial page must be added before creating the window.
int TTabbedWindow::AddPage(TWindow& wnd, LPCTSTR titles)
{
   TWindow* wnd1 = (TWindow*)&wnd;
   //  If the layout window hasn't been created, just Add the tab
   if(!GetHandle())
      return Add(*wnd1, titles);

   // Need at least one page in order to add a page
   if(GetTabControl()->GetSel() < 0)
      return -1;

   // Tell the currently selected Tab we're about to change
   TNotify dummy;
   EvTabControlSelChanging(dummy);

   int index = Add(*wnd1, titles);
   wnd1->ModifyStyle(WS_VISIBLE, 0);

   TLayoutMetrics lmWindow;
   GetChildLayoutMetrics(*GetPage(0), lmWindow);

   if(!wnd1->IsWindow())
      wnd1->Create();

   SetChildLayoutMetrics(*wnd1, lmWindow);
   Layout();

   // Tell the new Tab we've changed to it
   EvTabControlSelChange(dummy);

   return index;
}
//
bool TTabbedWindow::DeletePage(TWindow& wnd)
{
   TWindow* wnd1 = (TWindow*)&wnd;
  uint count = GetTabControl()->GetCount();
   for(uint i = 0; i < count; i++){
      if(wnd1 == GetPage(i)) {
         // Tell the currently selected Tab we're about to change
         TNotify dummy;
         EvTabControlSelChanging(dummy);
         GetTabControl()->Delete(i);
         EvTabControlSelChange(dummy);
         return true;
      }
   }
   return false;
}

void TTabbedWindow::ResizeTabControl(int newHeight)
{
  TNoteTab* t = GetTabControl(); CHECK(t);
  TLayoutMetrics m;
  GetChildLayoutMetrics(*t, m);
  m.Height.Absolute(newHeight);
  m.SetMeasurementUnits(lmPixels);
  SetChildLayoutMetrics(*t, m);
  Layout();
  t->Invalidate();
}

//------------------------------------------------------------------------------
// TTabbedView Implementation
//

DEFINE_RESPONSE_TABLE1(TTabbedView, TTabbedWindow)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;

//
//
//
TTabbedView::TTabbedView(TDocument& doc, TWindow* parent)
:
  TTabbedWindow(parent, nullptr, doc.GetDocManager().GetApplication()),
  TView(doc)
{
}

//
// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
bool
TTabbedView::VnIsWindow(HWND hWnd)
{
  return hWnd == GetHandle() || IsChild(hWnd);
}

//----------------------------------------------------------------------------
/*
IMPLEMENT_STREAMABLE2(TTabbedWindow, TLayoutWindow, TTabbed);
IMPLEMENT_STREAMABLE2(TTabbedView, TTabbedWindow, TView);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
*/
//TTabbedView::Streamer::Read(ipstream& is, uint32 /*version*/) const
/*
{
  ReadBaseObject((TTabbedWindow*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TTabbedView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TTabbedWindow*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
}

#endif

*/

} // OWL namespace
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

