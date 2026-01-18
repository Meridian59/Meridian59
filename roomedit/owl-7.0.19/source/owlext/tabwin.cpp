//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TTabWindow (& friends) implementation
//
// TODO: * Change tabbedWindowArray to be a collection of TTabWindowDatum,
//         in order to support adding tabs w/ controls for client areas,
//         since controls don't have a "title" per se
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/tabwin.h>

using namespace owl;

namespace OwlExt {

// Diagnostic group declarations/definitions
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlexTabWindow, 1, 0);
DIAG_DECLARE_GROUP(OwlMsg);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TTabWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*TResponseTableEntry< TTabWindow >  TTabWindow::__entries[] =
{
{0, 0, 0, 0}
};
*/
DEFINE_RESPONSE_TABLE1(TTabWindow, TWindow)
END_RESPONSE_TABLE;

struct TTabWindowDatum
{
  TWindow* m_ptr;
  LPCTSTR  m_title;
};

TTabWindow::TTabWindow(TWindow* parent, TWindow* array[])
:
TWindow(parent, (LPCTSTR)0)
{
  Attr.X = Attr.Y = Attr.W = Attr.H = 0;
  TRACEX(OwlexTabWindow, 1, _T("TTabWindow constructed @ ") << (void*)this);

  // add array to tabbedWindowArray
  if (array)
    Attach(array);
}
TTabWindow::TTabWindow(TWindow* parent, uint32 X, uint32 Y, uint32 W, uint32 H, TWindow* array[])
:
TWindow(parent, (LPCTSTR)0)
{
  Attr.X = X;
  Attr.Y = Y;
  Attr.W = W;
  Attr.H = H;
  TRACEX(OwlexTabWindow, 1, _T("TTabWindow constructed @ ") << (void*)this);

  // add array to tabbedWindowArray
  if (array)
    Attach(array);
}

//DEFINE_RESPONSE_TABLE1(TTabWindow, TWindow)
//END_RESPONSE_TABLE;
/*bool TTabWindow::Find(TEventInfo& eventInfo, TEqualOperator equal)
{
eventInfo.Object = (TGeneric*)this;
return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal)
|| TWindow::Find(eventInfo, equal);
}*/

void TTabWindow::Attach(TWindow* ptr, LPCTSTR /*title*/)
{
  // Add to collection of TWindow*'s
  tabbedWindowArray.Add(const_cast<TWindow*>(ptr));
  if (GetHandle()){
    // Reparent the given widget
    ptr->SetParent(this);

    if (!ptr->GetHandle())
      ptr->Create();
  }
}
void TTabWindow::Attach(TWindow* windowArray[])
{
  for (int i=0; windowArray[i]; i++)
    Attach(windowArray[i]);
}
TWindow* TTabWindow::Detach(int32 index)
{
  TWindow* retval = tabbedWindowArray[index];
  // Remove window element from list
  tabbedWindowArray.Detach(index);
  return retval;
}
int32 TTabWindow::Retrieve(TWindow* ptr)
{
  int index=0;
  for (TPtrArray<TWindow*>::Iterator iter(tabbedWindowArray); iter; iter++, index++)
    if (iter.Current() == ptr)
      return index;

  // throw an exception instead?
  return -1;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                               TWin95TabWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DEFINE_RESPONSE_TABLE1(TWin95TabWindow, TTabWindow)
  EV_WM_SIZE,
  EV_TCN_SELCHANGE(TTabWindow::ID_TABCTRL, TabSelChange),
  EV_TCN_SELCHANGING(TTabWindow::ID_TABCTRL, TabSelChanging),
  EV_TCN_KEYDOWN(TTabWindow::ID_TABCTRL, TabKeyDown),
END_RESPONSE_TABLE;

TWin95TabWindow::TWin95TabWindow(TWindow* parent, TWindow* array[])
:
TTabWindow(parent, array),
m_tabCtrl(0)
{
}
TWin95TabWindow::TWin95TabWindow(TWindow* parent, uint32 X, uint32 Y, uint32 W, uint32 H, TWindow* array[])
:
TTabWindow(parent, X, Y, W, H, array),
m_tabCtrl(0)
{
}
TWin95TabWindow::~TWin95TabWindow()
{
  delete m_tabCtrl;
}

int32 TWin95TabWindow::GetWindowCount()
{
  if ((unsigned)m_tabCtrl->GetCount() != tabbedWindowArray.GetItemsInContainer()){
    TRACE("This is a problem--tabCtrl->GetCount() != tabbedWindowArray");
  }
  return TTabWindow::GetWindowCount();
}

void TWin95TabWindow::SetupWindow()
{
  // If our W/H values are still 0, pull them from our parent
  if (Attr.W==0 || Attr.W==CW_USEDEFAULT)
    Attr.W = Parent->GetClientRect().Width();
  if (Attr.H==0 || Attr.H==CW_USEDEFAULT)
    Attr.H = Parent->GetClientRect().Height();

  // Create tab control, if not done already
  if (GetTabCtrl()==0)
    m_tabCtrl = new TTabControl(this, TTabWindow::ID_TABCTRL, 0, 0, Attr.W, Attr.H);

  TTabWindow::SetupWindow();

  // Add the tabs & pages already in tabbedWindowArray
  TWindow* ptr;
  uint i;
  for (i=0; i<tabbedWindowArray.GetItemsInContainer(); i++) {
    ptr = tabbedWindowArray[i];

    // Reparent and create
    ptr->SetParent(this);
    if (!ptr->GetHandle())
      ptr->Create();
    // Retrieve window's text, use that for tab text
    TAPointer<_TCHAR> txt(new _TCHAR[ptr->GetWindowTextLength()+1]);
    ptr->GetWindowText(txt, ptr->GetWindowTextLength()+1);
    m_tabCtrl->Add(TTabItem(txt));
    // Select new tab as current
    m_tabCtrl->SetSel(m_tabCtrl->GetCount()-1);
    // Center and display associated TWindow*
    AdjustTabClient();
  }

  // Hide all windows but the first
  for (i=1; i<tabbedWindowArray.GetItemsInContainer(); i++)
    tabbedWindowArray[i]->ShowWindow(SW_HIDE);

  // Set to the first tab
  SetSelectedTab(0);
  m_tabCtrl->ShowWindow(SW_SHOW);
}

void TWin95TabWindow::Attach(TWindow* ptr, LPCTSTR /*title*/)
{
  TTabWindow::Attach(ptr);

  if (m_tabCtrl)
  {
    // Retrieve window's text, use that for tab text
    TAPointer<_TCHAR> txt(new _TCHAR[ptr->GetWindowTextLength()+1]);
    ptr->GetWindowText(txt, ptr->GetWindowTextLength()+1);
    m_tabCtrl->Add(TTabItem(txt));
    // Select new tab as current
    SetSelectedTab(m_tabCtrl->GetCount()-1);
    ptr->ShowWindow(SW_SHOW);
  }
}
TWindow* TWin95TabWindow::Detach(int32 index)
{
  // Remove tab from tab control
  m_tabCtrl->Delete(index);

  return TTabWindow::Detach(index);
}
void TWin95TabWindow::SetSelectedTab(int32 index)
{
  m_tabCtrl->SetSel(index);

  // TTabControl does not send the TCN_ messages, so we need to "fake" it
  TNotify dummy;
  TabSelChanging(dummy);
  TabSelChange(dummy);
}
void TWin95TabWindow::AdjustTabClient()
{
  // Retrieve pointer to tab control, the client window
  if (m_tabCtrl && m_tabCtrl->GetHandle())
  {
    // Retrieve the window rectangle of the tab control
    TRect rect;
    m_tabCtrl->GetWindowRect(rect);

    // NOTE: GetWindowRect returns screen coordinates... we'll need
    //       to have the coordinates relative to the frame window,
    //       the parent of both the tab and client window
    ::MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)&rect, 2);

    // Ask the tab for it's 'client-area' based in it window location
    m_tabCtrl->AdjustRect(false, rect);

    // Move the Client window
    TWindow* Client = tabbedWindowArray[m_tabCtrl->GetSel()];
    if (Client && Client->GetHandle())
      Client->SetWindowPos(HWND_TOP, rect, SWP_SHOWWINDOW);
  }
}

void TWin95TabWindow::EvSize(uint sizeType, const TSize& size)
{
  TWindow::EvSize(sizeType, size);
  if (m_tabCtrl && m_tabCtrl->GetHandle())
  {
    // Adjust tab control to new window size
    m_tabCtrl->SetWindowPos(0, 0, 0, size.cx, size.cy, SWP_NOZORDER | SWP_NOMOVE);
  }
  AdjustTabClient();
}
void TWin95TabWindow::TabSelChange(TNotify &)
{
  AdjustTabClient();
  // Here, GetSel() returns the after-the-click-current tab selected
  tabbedWindowArray[m_tabCtrl->GetSel()]->ShowWindow(SW_SHOW);
  tabbedWindowArray[m_tabCtrl->GetSel()]->SetWindowPos(HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}
bool TWin95TabWindow::TabSelChanging(TNotify &)
{
  // Here, GetSel() returns the before-the-click-current tab selected
  if (tabbedWindowArray[m_tabCtrl->GetSel()])
    tabbedWindowArray[m_tabCtrl->GetSel()]->ShowWindow(SW_HIDE);
  return false;
}
void TWin95TabWindow::TabKeyDown(TTabKeyDown &)
{
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                TNoteTabWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
DEFINE_RESPONSE_TABLE1(TNoteTabWindow, TWindow)
EV_WM_SIZE,
EV_TCN_SELCHANGE(TTabWindow::ID_TABCTRL, TabSelChange),
EV_TCN_SELCHANGING(TTabWindow::ID_TABCTRL, TabSelChanging),
EV_TCN_KEYDOWN(TTabWindow::ID_TABCTRL, TabKeyDown),
END_RESPONSE_TABLE;
*/


} // OwlExt namespace

