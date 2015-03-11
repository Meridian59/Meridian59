//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TListBoxView
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/listboxview.h>
#include <owl/inputdia.h>
#include <owl/listboxview.rh>
#include <owl/docview.rh>
#include <owl/edit.rh>
#include <stdio.h>

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlDocView);        // General Doc/View diagnostic group

// Let the compiler know that the following template instances will be defined elsewhere.
//#pragma option -Jgx


const tchar VirtualLastLineStr[] = _T("---");  // Last virtual line appended to list

DEFINE_RESPONSE_TABLE1(TListBoxView, TListBox)
  EV_COMMAND(CM_EDITUNDO,   CmEditUndo),
  EV_COMMAND(CM_EDITCUT,    CmEditCut),
  EV_COMMAND(CM_EDITCOPY,   CmEditCopy),
  EV_COMMAND(CM_EDITPASTE,  CmEditPaste),
  EV_COMMAND(CM_EDITCLEAR,  CmEditClear),
  EV_COMMAND(CM_EDITDELETE, CmEditDelete),
  EV_COMMAND(CM_EDITADD,    CmEditAdd),
  EV_COMMAND(CM_EDITEDIT,   CmEditItem),
  EV_WM_GETDLGCODE,
  EV_NOTIFY_AT_CHILD(LBN_DBLCLK, CmEditItem),
  EV_NOTIFY_AT_CHILD(LBN_SELCHANGE, CmSelChange),
  EV_VN_DOCCLOSED,
  EV_VN_ISWINDOW,
  EV_VN_ISDIRTY,
  EV_VN_COMMIT,
  EV_VN_REVERT,
END_RESPONSE_TABLE;

//
/// Creates a TListBoxView object associated with the specified document and parent
/// window. Sets Attr.AccelTable to IDA_LISTVIEW to identify the edit view. Sets the
/// view style to WS_HSCROLL | LBS_NOINTEGRALHEIGHT.
/// Sets TView::ViewMenu to the new TMenuDescr for this view.
//
TListBoxView::TListBoxView(TDocument& doc, TWindow* parent)
:
  TListBox(parent, GetNextViewId(), 0,0,0,0),
  TView(doc),
  DirtyFlag(false),
  Origin(0),
  MaxWidth(0)
{
  Attr.Style &= ~(LBS_SORT);
  Attr.Style |= (WS_HSCROLL | LBS_NOINTEGRALHEIGHT);
  Attr.AccelTable = IDA_LISTVIEW;
  if (::FindResource(*GetModule(), TResId(IDM_LISTVIEW), RT_MENU))
    SetViewMenu(new TMenuDescr(IDM_LISTVIEW, 0,1,0,0,0,1, GetModule()));
}

//
/// Adds a string into the view. Returns the index at which the string is added.
//
int
TListBoxView::AddString(LPCTSTR str)
{
  long style = GetWindowLong(GWL_STYLE);
  if (!(style & LBS_SORT)) {
    int itemsInListBox = GetCount();
    if (itemsInListBox > 0) {
      // before the end of list marker
      return InsertString(str, itemsInListBox-1);
    }
  }
  return TListBox::AddString(str);
}

//
/// Sets the maximum horizontal extent for the list view window.
//
void
TListBoxView::SetExtent(const tstring& str)
{
  if (str.length() == 0)
    return;

  TClientDC  clientDC(*this);
  TSize extent = clientDC.GetTextExtent(str, str.length());
  extent.cx += 2; // room for focus rectangle

  if (extent.cx > MaxWidth)
    SetHorizontalExtent(MaxWidth = extent.cx);
}

//
/// Indicates that the document has been closed.
//
bool
TListBoxView::VnDocClosed(int omode)
{
  if (DirtyFlag == 2 || !(omode & ofWrite))  // make sure someone else's write
    return false;
  int top = GetTopIndex();
  int sel = GetSelIndex();
  LoadData(top, sel);
  return true;
}

//
/// Reads the view from the stream and closes the file. Returns true if the view was
/// successfully loaded.
/// Throws an xmsg exception and displays the error message TListBoxView initial read
/// error if the file can't be read. Returns false if the view can't be loaded.
//
bool
TListBoxView::LoadData(int top, int sel)
{
  CmEditClear();    // Clear list & remove virtual last line temporarily
  DeleteString(0);

  long style = GetWindowLong(GWL_STYLE);
  if (!(style & LBS_SORT))
    TListBox::AddString(VirtualLastLineStr);     // Append virtual last line

  DirtyFlag = false;

  tistream* inStream;
  if ((inStream = Doc->InStream(ios::in)) == 0) {
    Doc->PostError(IDS_UNABLEOPEN, MB_OK);
    return false;
  }
  bool status;
  for (;;) {
    tchar buf[100+1];
    inStream->getline(buf, COUNTOF(buf)-1);
    if (!inStream->gcount() && !inStream->good()) {
      status = ToBool(inStream->eof());
      break;
    }
    AddString(buf);
    SetExtent(buf);
  }
  SetTopIndex(top);
  SetSelIndex(sel);
  delete inStream;   // close file in case process switch
  if (!status)
    Doc->PostError(IDS_READERROR, MB_OK);
  return status;
}

//
/// Overrides TWindow::Create and creates the view's window. Determines if the file
/// is new or already has data. If there is data, calls LoadData to add the data to
/// the view. If the view's window can't be created, Create throws a TXWindow
/// exception.
//
bool
TListBoxView::Create()
{
  try {
    TListBox::Create();   // throws exception TWindow::TXWindow
  }
  catch (TXOwl& ) {
    Doc->PostError(IDS_NOMEMORYFORVIEW, MB_OK);
    return true;   // cannot return false - throws another exception
  }
  if (Doc->GetDocPath() == 0) {
    CmEditClear();         // perform any clearing initialization
    return true;           // new file, no data to display
  }
  if (!LoadData(0, 0))
    NotOK();
  return true;
}

//
/// Commits changes made in the view to the document. If force is nonzero, all data,
/// even if it's unchanged, is saved to the document.
//
bool
TListBoxView::VnCommit(bool force)
{
  if (!force && !DirtyFlag)
    return true;

  tostream* outStream = Doc->OutStream(ios::out);
  if (outStream == 0) {
    Doc->PostError(IDS_UNABLEOPEN, MB_OK);
    return false;
  }
  outStream->seekp(Origin);
  int count = GetCount();
  for (int index = 0; index < count-1; index++) {  // don't write last virtual line
    int len = GetStringLen(index);
    TAPointer<tchar> buf(new tchar[len+1]);
    GetString((tchar*)buf, index);
    *outStream << (tchar*)buf << _T('\n');
    //delete buf;
  }
  DirtyFlag = 2;           // to detect our own close notification

  bool status = ToBool(outStream->good());
  delete outStream;
  DirtyFlag = false;
  if (!status)
    Doc->PostError(IDS_WRITEERROR, MB_OK);

  return status;
}

//
/// Indicates if changes made to the view should be erased, and if the data from the
/// document should be restored to the view. If clear is a nonzero value, the data
/// is cleared instead of restored to the view.
//
bool
TListBoxView::VnRevert(bool clear)
{
  if (!clear && Doc->GetDocPath() != 0)
    return LoadData(0,0);
  CmEditClear();
  DirtyFlag = false;
  return true;
}

//
/// Overrides TWindow's response to a WM_GETDLGCODE message (an input procedure
/// associated with a control that isn't a check box) by calling
/// TWindow::DefaultProcessing.
/// EvGetDlgCode returns a code that indicates how the list box control message is
/// to be treated.
//
uint
TListBoxView::EvGetDlgCode(const MSG*)
{
  uint retVal = (uint)DefaultProcessing();
  retVal |= DLGC_WANTCHARS;
  return retVal;
}

//
/// Handler to undo the last operation performed on the underlying ListBox.
/// \note This feature is not implemented in the current version of ObjectWindows.
//
void
TListBoxView::CmEditUndo()
{
#if BI_MSG_LANGUAGE == 0x0411
  MessageBox("‚±‚Ì‹@”\‚ÍŽÀ‘•‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ", "Œ³‚É–ß‚·", MB_OK);
#else
  MessageBox(_T("Feature not implemented"), _T("Undo"), MB_OK);  
#endif
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITCUT by
/// calling CmEditCopy and CmEditDelete to delete a text string from the list view.
/// Sets the data member DirtyFlag to true.
//
void
TListBoxView::CmEditCut()
{
  CmEditCopy();
  CmEditDelete();
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITCOPY and
/// copies the selected text to the Clipboard.
//
void
TListBoxView::CmEditCopy()
{
  int index = GetSelIndex();
  int count = GetCount();
  if (count <= 1 || index >= count)
    return;

  TClipboard cb(*this, false);
  if (cb.EmptyClipboard()) {
    int len = GetStringLen(index);
    HANDLE cbhdl = ::GlobalAlloc(GHND,len+0+1);
    LPTSTR buf = (LPTSTR)::GlobalLock(cbhdl);
    GetString(buf, index);
    ::GlobalUnlock(cbhdl);
#if defined(UNICODE)
    cb.SetClipboardData(CF_UNICODETEXT, cbhdl);
#else
    cb.SetClipboardData(CF_TEXT, cbhdl);
#endif
  }
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITPASTE by
/// inserting text into the list box using functions in TListBox.
//
void
TListBoxView::CmEditPaste()
{
  int index = GetSelIndex();
  if (index < 0)
    index = 0;

  TClipboard cb(*this, false);
  if (!cb)
    return;   // clipboard open by another program

#if defined(UNICODE)
  HANDLE cbhdl = cb.GetClipboardData(CF_UNICODETEXT);
#else
  HANDLE cbhdl = cb.GetClipboardData(CF_TEXT);
#endif
  if (cbhdl) {
    LPTSTR text = (LPTSTR)::GlobalLock(cbhdl);
    InsertString(text, index);
    SetSelIndex(index+1);
    DirtyFlag = true;
    ::GlobalUnlock(cbhdl);
  }
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITDELETE by
/// deleting the currently selected text.
//
void
TListBoxView::CmEditDelete()
{
  int count = GetCount();
  int index = GetSelIndex();
  if (count <= 1 || index >= count-1)
    return;

  DeleteString(index);
  SetSelIndex(index);
  DirtyFlag = true;
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITCLEAR by
/// clearing the items in the list box using functions in TListBox.
//
void
TListBoxView::CmEditClear()
{
  int count = GetCount();
  if (count == 1)
    return;
  if (count) {
    ClearList();
    DirtyFlag = true;
    SetHorizontalExtent(MaxWidth = 0);
  }
  long style = GetWindowLong(GWL_STYLE);
  if (!(style & LBS_SORT))
    TListBox::AddString(VirtualLastLineStr);
}

static int linePrompt(TWindow* parent, int index, UINT id,
                      LPTSTR buf, int buflen)
{
  tchar msg[41];
  _stprintf(msg, parent->LoadString(IDS_LISTNUM).c_str(), index);
  return TInputDialog(parent, msg, parent->LoadString(id).c_str(), buf, buflen).Execute();
}

//
/// Automatically responds to CM_LISTADD message by getting the length of the input
/// string and inserting the text string into the list view. Sets the data member
/// DirtyFlag to true.
//
void
TListBoxView::CmEditAdd()
{
  tchar inputText[101];
  *inputText = 0;

  int index = GetSelIndex();
  if (index < 0)
    index = 0;

  if (linePrompt(this,index+1,CM_EDITADD,inputText,COUNTOF(inputText)) == IDOK) {
    InsertString(inputText, index);
    SetSelIndex(index+1);
    SetExtent(inputText);
    DirtyFlag = true;
  }
}

//
/// Automatically responds to a CM_LISTEDIT message by getting the input text and
/// inserting the text into the list view. Sets DirtyFlag to a nonzero value to
/// indicate that the view has been changed and not saved.
//
void
TListBoxView::CmEditItem()
{
  int index = GetSelIndex();
  int count = GetCount();

  if (index == count-1) {
    CmEditAdd();
    return;
  }

  if (index < 0 || index >= count-1)
    return;

  tchar inputText[101];
  GetSelString(inputText, COUNTOF(inputText)-1);

  if (linePrompt(this,index+1,CM_EDITEDIT,inputText,COUNTOF(inputText))==IDOK) {
    DeleteString(index);
    InsertString(inputText, index);
    SetExtent(inputText);
    SetSelIndex(index);
    DirtyFlag = true;
  }
}

//
// To prevent interpreting as unprocessed accelerator
//
/// Automatically responds to a LBN_SELCHANGE message (which indicates that the
/// contents of the list view have changed) by calling TWindow::DefaultProcessing.
//
void
TListBoxView::CmSelChange()
{
}


IMPLEMENT_STREAMABLE2(TListBoxView, TListBox, TView);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TListBoxView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TListBox*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  is >> GetObject()->Origin;
  return GetObject();
}

//
//
//
void
TListBoxView::Streamer::Write(opstream &os) const
{
  WriteBaseObject((TListBox*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
  os << GetObject()->Origin;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

