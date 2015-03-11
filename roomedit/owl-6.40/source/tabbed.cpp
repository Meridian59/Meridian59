//------------------------------------------------------------------------------
//  ObjectWindows
//  Copyright © 1998 by Bidus Yura. All Rights Reserved.
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
void TTabbedBase::EvTabControlSelChange(TNotify&)
{
  int Selected = NoteTab->GetSel();
  TNoteTabItem Item;
  NoteTab->GetItem(Selected, Item);

  // Set the pointer to currently selected item
  TWindow* wnd = (TWindow*)Item.ClientData;
  if(wnd)
    wnd->ShowWindow(SW_SHOWNORMAL);
}
// -----------------------------------------------------------------------------
bool TTabbedBase::EvTabControlSelChanging(TNotify &)
{
  int Selected = NoteTab->GetSel();
  TNoteTabItem Item;
  NoteTab->GetItem(Selected, Item);
  TWindow* wnd = (TWindow*)Item.ClientData;

  if (wnd)
    wnd->ShowWindow(SW_HIDE);

  return false;
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
}

TTabbedWindow::TTabbedWindow(TWindow* parent, const tstring& title, TModule* module)
  : TLayoutWindow(parent, title, module),
  TTabbedBase(this,0,0,0,0)
{}

//
bool TTabbedWindow::Create()
{
  uint count = GetTabControl()->GetCount();
  uint i;
  for(i = 0; i < count; i++){
    TWindow* wnd = GetPage(i);
    wnd->ModifyStyle(WS_VISIBLE, 0);
  }
  // Use the default UI font
  //
  TScreenDC dc;
  uint height = TDefaultGUIFont().GetHeight(dc);

  TLayoutMetrics lmWindow;
  TLayoutMetrics lmNoteTab;
  setWindowLayoutMetrics(lmWindow, GetTabControl(), Position);
  switch(Position){
    case tpLeft:
      lmNoteTab.X.SameAs(lmParent, lmLeft);
      lmNoteTab.Y.SameAs(lmParent, lmTop);
      lmNoteTab.Width.Absolute(lmRight, height);
      lmNoteTab.Height.SameAs(lmParent, lmHeight);
      break;
    case tpRight:
      lmNoteTab.X.SameAs(lmParent, lmRight);
      lmNoteTab.Y.SameAs(lmParent, lmTop);
      lmNoteTab.Height.SameAs(lmParent, lmHeight);
      lmNoteTab.Width.Absolute(lmLeft, height);
      break;
    case tpTop:
      lmNoteTab.X.SameAs(lmParent, lmLeft);
      lmNoteTab.Y.SameAs(lmParent, lmTop);
      lmNoteTab.Width.SameAs(lmParent, lmWidth);
      lmNoteTab.Height.Absolute(lmBottom, height);
      break;
    case tpBottom:
      lmNoteTab.X.SameAs(lmParent, lmLeft);
      lmNoteTab.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, 0);
      lmNoteTab.Height.Absolute(height);
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
  TTabbedWindow(parent, 0, doc.GetDocManager().GetApplication()),
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

#if !defined(BI_NO_OBJ_STREAMING)

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

#endif  // if !defined(BI_NO_OBJ_STREAMING)

*/

} // OWL namespace
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

