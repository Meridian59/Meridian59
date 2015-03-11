//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TEditView
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/editview.h>
#include <owl/docview.rh>
#include <owl/editview.rh>
#include <memory>

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlDocView);        // General Doc/View diagnostic group


//
// TEditView response table
//
DEFINE_RESPONSE_TABLE1(TEditView, TEditSearch)
  EV_VN_DOCCLOSED,
  EV_VN_ISWINDOW,
  EV_VN_ISDIRTY,
  EV_VN_COMMIT,
  EV_VN_REVERT,
  EV_WM_NCDESTROY,
END_RESPONSE_TABLE;

//
/// Creates a TEditView object associated with the specified document and parent
/// window. Sets AttrAccelTable to IDA_EDITVIEW to identify the edit view. Sets the
/// TView private data member ViewMenu to the new TMenuDescr for this view.
//
TEditView::TEditView(TDocument& doc, TWindow* parent)
:
  TEditSearch(parent, GetNextViewId(), (LPCTSTR)0),
  TView(doc),
  Origin(0)
{
  Attr.AccelTable = IDA_EDITVIEW;
  if (::FindResource(*GetModule(), TResId(IDM_EDITVIEW), RT_MENU))
    SetViewMenu(new TMenuDescr(IDM_EDITVIEW, 0,2,0,0,0,1, GetModule()));
}

//
/// Used internally by TEditView to manage memory.
/// This member is not available under Presentation Manager.
//
void
TEditView::EvNCDestroy()
{
  TEditSearch::EvNCDestroy();// call TWindow::EvNCDestroy, this may be deleted
}

//
/// Destroys a TEditView object.
//
TEditView::~TEditView()
{
}

//
/// Returns a nonzero value if the window's handle passed in hWnd is the same as
/// that of the view's display window or a child of it.
//
bool
TEditView::VnIsWindow(HWND hWnd)
{
  return hWnd == TWindow::GetHandle() || IsChild(hWnd);
}

//
/// Indicates that the document has been closed. mode is one of the ofxxxx document
/// open enum constants.
//
bool
TEditView::VnDocClosed(int omode)
{
  if (VnIsDirty() || !(omode & ofWrite))  // make sure someone else's write
    return false;

  int top = GetFirstVisibleLine();
  int selbeg;
  int selend;
  TEdit::GetSelection(selbeg, selend);
  TEdit::Clear();
  LoadData();
  Scroll(0, top);
  TEdit::SetSelection(selbeg, selend);

  return true;
}

//
/// Reads the view from the stream and closes the file. It returns a nonzero value
/// if the view was successfully loaded. If the file cannot be read, LoadData posts
/// an error and returns 0.
//
bool
TEditView::LoadData()
{
  auto_ptr<tistream> inStream (Doc->InStream(ios::in | ios::binary));
  if (!inStream.get()) 
  {
    Doc->PostError(IDS_UNABLEOPEN, MB_OK);
    return false;
  }
  inStream->seekg(0, ios::end);
  streamsize count = inStream->tellg() - streampos(0);
  inStream->seekg(0, ios::beg);

  // TODO: Widen the parameter type to LockBuffer to avoid this check and narrowing cast.
  //
  if (!IsRepresentable<uint>(count + 1)) {
    Doc->PostError(IDS_NOMEMORYFORVIEW, MB_OK);
    return false;
  }
  LPTSTR buf = LockBuffer(static_cast<uint>(count + 1));
  if (!buf) {
    Doc->PostError(IDS_NOMEMORYFORVIEW, MB_OK);
    return false;
  }

  inStream->read(buf, count);
  bool status = (inStream->gcount() == count);
  buf[count] = 0;    // 0 terminate buffer
  UnlockBuffer(buf, true);
  if (!status)
    Doc->PostError(IDS_READERROR, MB_OK);
  return status;
}

//
/// Creates the view's window. Calls GetDocPath to determine if the file is new or
/// if it already has data. If there is data, calls LoadData to add the data to the
/// view. If the view's window cannot be created, Create indicates that the view is
/// invalid.
//
bool
TEditView::Create()
{
  try {
    TEditSearch::Create();   // throws exception TWindow::TXWindow
  }
  catch(TXOwl&) {
    Doc->PostError(IDS_NOMEMORYFORVIEW, MB_OK);
    NotOK();
    return true;   // cannot return false - throws another exception
  }
  if (Doc->GetDocPath() == 0) {
    return true;           // new file, no data to display
  }
  if (!LoadData())
    NotOK();
  return true;
}

//
/// Commits changes made in the view to the document. If force is nonzero, all data,
/// even if unchanged, is saved to the document.
//
bool
TEditView::VnCommit(bool force)
{
  if (!force && !(VnIsDirty()))
    return true;

  auto_ptr<tostream> outStream(Doc->OutStream(ios::out | ios::binary));
  if (!outStream.get()) 
  {
    Doc->PostError(IDS_UNABLEOPEN, MB_OK);
    return false;
  }
  outStream->seekp(Origin);

  bool status = false;
  LPTSTR buf = LockBuffer();
  if (buf) {
    streamsize count = ::_tcslen(buf);
    outStream->write(buf, count);
    status = outStream->good();
    UnlockBuffer(buf);
    ClearModify();   // reset edit control
  }
  if (!status)
    Doc->PostError(IDS_WRITEERROR, MB_OK);

  return status;
}

//
/// Returns a nonzero value if changes made to the view should be erased and the
/// data from the document should be restored to the view. If clear is nonzero, the
/// data is cleared instead of restored to the view.
//
bool
TEditView::VnRevert(bool clear)
{
  TEdit::Clear();
  ClearModify();   // reset edit control
  return clear ? true : LoadData();
}


IMPLEMENT_STREAMABLE2(TEditView, TEditSearch, TView);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TEditView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TEditSearch*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  is >> GetObject()->Origin;
  return GetObject();
}

//
//
//
void
TEditView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TEditSearch*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
  os << GetObject()->Origin;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

