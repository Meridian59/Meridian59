//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TDocument
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/docmanag.h>
#include <owl/appdict.h>
#include <owl/docview.rh>
#include <string.h>
#include <stdio.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlDocView);        // General Doc/View diagnostic group

// !BB Is provided by DocManager to avoid sharing
//    of index between multiple applications.
// int TDocument::UntitledIndex = 0;


//
/// Although you do not create a TDocument object directly, you must call the
/// constructor when you create a derived class. parent points to the parent of the
/// new document. If no parent exists, parent is 0.
//
TDocument::TDocument(TDocument* parent)
:
  Tag(0),
  DirtyFlag(false),
  Embedded(false),
  ParentDoc(parent),
  NextDoc(0),
  OpenMode(0),
  Title(0),
  Template(0),
  ViewList(0),
  StreamList(0),
  DocPath(0)
{
  if (parent) {
    DocManager = parent->DocManager;

    // Handle case of 'dummy' parent [parent parented to itself] which was
    // created by the docmanager simply to pass itself to us
    //
    if (parent->ParentDoc == parent) {
      ParentDoc = 0;
      CHECK(DocManager);
      DocManager->DocList.Insert(this);
    }
    else
      parent->ChildDoc.Insert(this);
  }
  else {
    TApplication* app = OWLGetAppDictionary().GetApplication(0);
    CHECK(app);
    DocManager = app->GetDocManager();
    if (!DocManager)
      TXOwl::Raise(IDS_NODOCMANAGER);  // No doc manager to catch this one
    DocManager->DocList.Insert(this);
  }
}

//
/// Deletes a TDocument object. Normally, Close is called first. TDocument's
/// destructor destroys all children and closes all open streams. If this is the
/// last document that used the template, it closes the object's template and any
/// associated views, deletes the object's stream, and removes itself from the
/// parent's list of children if a parent exists. If there is no parent, it removes
/// itself from the document manager's document list.
//
TDocument::~TDocument()
{
  // Is this a dummy document?
  //
  if (ParentDoc == this)
    return;

  // Unref template - prevents self autodelete when deleting views, including
  // views in child documents
  //
  SetTemplate(0);

  // Destroy children first if we have any. Then force close here as a last
  // resort if derived classes have not done so. Since we have destructed down
  // to a TDocument by now, derived closes will not be called.
  //
  ChildDoc.Destroy();
  Close();

  // NOTE: View's destructor invokes 'DetachView' which removes it from
  //       the list
  //
  while (ViewList)
    delete ViewList;

  // Delete all streams, should only be present if abort or coding error
  //
  while (StreamList) {
    TRACEX(OwlDocView, 0, _T("~TDocument(): StreamList not 0!"));
    delete StreamList;
  }

  // Detach from parent and doc manager
  //
  if (ParentDoc) {
    ParentDoc->ChildDoc.Remove(this);
  }
  else {
    CHECK(DocManager);
    DocManager->PostEvent(dnClose, *this);
    DocManager->DocList.Remove(this);
  }

  delete[] Title;
  delete[] DocPath;
}

//
//
//
static LPTSTR PropNames[] = {
  _T("Document Class"),  // DocumentClass
  _T("Template Name"),   // TemplateName
  _T("View Count"),      // ViewCount
  _T("Storage Path"),    // StoragePath
  _T("Document Title"),  // DocTitle
};

//
//
//
static int PropFlags[] = {
  pfGetText|pfConstant,  // DocumentClass
  pfGetText,             // TemplateName
  pfGetBinary|pfGetText, // ViewCount
  pfGetText|pfSettable,  // StoragePath
  pfGetText|pfSettable,  // DocTitle
};

//
/// Returns the name of the property given the index value (index).
//
LPCTSTR
TDocument::PropertyName(int index)
{
  if (index <= PrevProperty) {
    TRACEX(OwlDocView, 0, _T("PropertyName(): index <= PrevProperty!"));
    return 0;
  }
  else if (index < NextProperty)
    return PropNames[index-PrevProperty-1];
  else {
    TRACEX(OwlDocView, 0, _T("PropertyName(): index >= PrevProperty!"));
    return 0;
  }
}

//
/// Returns the attributes of a specified property given the index (index) of the
/// property whose attributes you want to retrieve.
//
int
TDocument::PropertyFlags(int index)
{
  if (index <= PrevProperty) {
    TRACEX(OwlDocView, 0, _T("PropertyFlags(): index <= PrevProperty!"));
    return 0;
  }
  else if (index < NextProperty)
    return PropFlags[index-PrevProperty-1];
  else {
    TRACEX(OwlDocView, 0, _T("PropertyFlags(): index >= PrevProperty!"));
    return 0;
  }
}

//
/// Gets the property index, given the property name (name). Returns either the
/// integer index number that corresponds to the name or 0 if the name isn't found
/// in the list of properties.
//
int
TDocument::FindProperty(LPCTSTR name)
{
  int i;
  for (i=0; i < NextProperty-PrevProperty-1; i++)
    if (_tcscmp(PropNames[i], name) == 0)
      return i+PrevProperty+1;

  TRACEX(OwlDocView, 0, _T("FindProperty(): ") \
        _T("index of [") << tstring(name).c_str() << _T("] not found") );
  return 0;
}

//
/// Returns the total number of properties for this document, where index is the
/// property index, dest contains the property data, and textlen is the size of the
/// array. If textlen is 0, property data is returned as binary data; otherwise,
/// property data is returned as text data.
//
int
TDocument::GetProperty(int prop, void * dest, int textlen)
{
  LPCTSTR src;
  tchar   numBuf[15];
  switch (prop) {
    case DocumentClass: {
        _USES_CONVERSION;
        src = _A2W(_OBJ_FULLTYPENAME(this));
      }
      break;

    case TemplateName:
      src = Template ? Template->GetDescription() : 0;
      break;

    case ViewCount: {
      int cnt;
      TView* view;
      for (view=ViewList, cnt=0; view != 0; view=view->NextView, cnt++)
        ; // Do nothing
      if (!textlen) {
        *(int *)dest = cnt;
        return sizeof(int);
      }
      wsprintf(numBuf, _T("%d"), cnt);
      src = numBuf;
      break;
    }

    case StoragePath:
      src = DocPath;
      break;

    case DocTitle:
      src = Title;
      break;

    default:
      TRACEX(OwlDocView, 0, _T("GetProperty(): invalid property [") 
             << prop << _T("] specified!") );
      return 0;
  }

  if (!textlen) {
    TRACEX(OwlDocView, 0, _T("GetProperty(): 0-Length buffer specified!"));
    return 0;
  }
  int srclen = src ? static_cast<int>(::_tcslen(src)) : 0;
  if (textlen > srclen)
    textlen = srclen;
  if (textlen)
    memcpy(dest, src, textlen*sizeof(tchar));
  *((LPTSTR)dest + textlen) = 0;
  return srclen;
}

//
/// Sets the value of the property, given index, the index value of the property,
/// and src, the data type (either binary or text) to which the property must be
/// set.
//
bool
TDocument::SetProperty(int prop, const void * src)
{
  switch (prop) {
    case DocTitle:
      SetTitle((LPCTSTR)src);
      break;

    case StoragePath:
      return SetDocPath((LPCTSTR)src);

    default:
      TRACEX(OwlDocView, 0, _T("SetProperty(): invalid prop [") << prop <<\
            _T("] specified!"));
      return false;
  }
  return true;
}

//
/// Returns the this pointer as the root document.
//
TDocument&
TDocument::RootDocument()
{
  TDocument* pdoc = this;
  while (pdoc->ParentDoc)
    pdoc = pdoc->ParentDoc;
  return *pdoc;
}

//
/// Sets the current document manager to the argument dm.
//
void
TDocument::SetDocManager(TDocManager& dm)
{
  if (!ParentDoc) {
    if (DocManager)  // test needed for TDocManager::Streamer::Read()
      DocManager->DocList.Remove(this);
    dm.DocList.Insert(this);
  }
  DocManager = &dm;
}

//
/// Sets the document path for Open and Save operations.
//
bool
TDocument::SetDocPath(LPCTSTR path)
{
  // Path has been set already
  //
  if (path && (path == DocPath)) {
    TRACEX(OwlDocView, 0, _T("SetDocPath(): path [") << tstring(path).c_str() << _T("] ")\
                          _T("already set!"));
    return true;
  }

  delete[] DocPath;
  DocPath = (path && *path) ? strnewdup(path) : 0;

  tchar title[_MAX_PATH] = _T("Unknown");  // Never used - but just in case!
  if (!DocPath || TCommDlg::GetFileTitle(DocPath, title, COUNTOF(title)) != 0) {
    CHECK(DocManager);
    CHECK(DocManager->GetApplication());

    int len = DocManager->GetApplication()->LoadString(IDS_UNTITLED,
                                                       title, COUNTOF(title));
    if (DocManager->IsFlagSet(dmMDI))
      wsprintf(title+len, _T("%d"), ++(DocManager->GetUntitledIndex()));
  }
  SetTitle(title);
  return true;  // derived classes may validate path
}

//
/// Sets the title of the document.
//
void
TDocument::SetTitle(LPCTSTR title)
{
  if (!Title || title!=Title)
  {    //This check allows calls like SetTitle(GetTitle()) (which is indirect way of calling ReindexFrames)
    delete[] Title;
    Title = title ? strnewdup(title) : 0;
  }
  ReindexFrames();
}

//
/// Sets the document template. However, if the template type is incompatible with
/// the file, the document manager will refuse to save the file as this template
/// type.
//
bool
TDocument::SetTemplate(TDocTemplate* tpl)
{
  if (Template) {
    CHECK(DocManager);
    DocManager->UnRefTemplate(*Template);
  }
  if (tpl) {
    CHECK(DocManager);
    DocManager->RefTemplate(*tpl);
  }
  Template = tpl;
  return true;
}

//
/// Force view title and index update.
//
void
TDocument::ReindexFrames()
{
  TView* view;
  int seq;

  for (seq = -1, view = ViewList; view != 0; view = view->NextView) {
    seq -= view->SetDocTitle(Title, seq);  // decrement if title displayed
    if (seq == -3)   // need only check if more than one title displayed
      break;
  }
  if (seq == -1)
    return;
  seq = (seq == -2 ? 0 : 1);
  for (view = ViewList; view != 0; view = view->NextView) 
    {
    //DLN: added this if condition to avoid PRECONDITIONs in debug build
    // which occur if program closed by closing main window
    if ( view->GetWindow() && view->GetWindow()->GetHandle() )
    seq += view->SetDocTitle(Title, seq);  // increment if title displayed
  }
}

//
/// Called from TStream's constructor, AttachStream attaches a stream to the current
/// document.
//
void
TDocument::AttachStream(TStream& strm)
{
  strm.NextStream = StreamList;
  StreamList = &strm;
}

//
/// Called from TStream's destructor, DetachStream detaches the stream from the
/// current document.
//
void
TDocument::DetachStream(TStream& strm)
{
  TStream** plist = &StreamList;
  for ( ; *plist; plist = &(*plist)->NextStream) {
    if (*plist == &strm) {
      *plist = strm.NextStream;
      return;
    }
  }
}

//
/// Gets the next entry in the stream. Holds 0 if none exists.
//
TStream*
TDocument::NextStream(const TStream* strm)
{
  return strm ? strm->NextStream : StreamList;
}

//
/// Gets the next view in the list of views. Holds 0 if none exists.
//
TView*
TDocument::NextView(const TView* view)
{
  return view ? view->NextView : ViewList;
}

//
/// Called from TView constructor.
//
void
TDocument::AttachView(TView& view)
{
  TView** ppview;

  for (ppview = &ViewList; *ppview; ppview = &(*ppview)->NextView)
    ;
  *ppview = &view;    // insert at end of list
  view.NextView = 0;
  view.Doc = this;
  NotifyViews(vnViewOpened, (long)&view, &view);
}

//
/// Initializes the view. Notifies others a view is created by posting the dnCreate
/// event.
//
TView*
TDocument::InitView(TView* view)
{
  if (!view) {
    TRACEX(OwlDocView, 0, _T("InitView(): 0 view specified!"));
    return 0;
  }
  if (!view->IsOK()) {
    TRACEX(OwlDocView, 0, _T("InitView(): Invalid view object specified!"));
    delete view;
    return 0;
  }

  CHECK(DocManager);
  DocManager->PostEvent(dnCreate, *view);

  if (!view->IsOK()) {
    TRACEX(OwlDocView, 0, _T("InitView(): Invalid view object post dnCreate!"));
    delete view;
    return 0;
  }

  ReindexFrames();
  TView::BumpNextViewId();

  return view;
}

//
/// DetachView is invoked from TView's destructor so that the view can detach
/// itself from this document. True is returned if the detachment is successful
/// indicating that this document should be deleted.
//
bool
TDocument::DetachView(TView& view)
{
  TView** plist = &ViewList;
  for (; *plist; plist = &(*plist)->NextView) {
    if (*plist == &view) {

      // Found the view, now detach it and notify app and other views
      //
      DocManager->PostEvent(dnClose, view);
      *plist = view.NextView;
      NotifyViews(vnViewClosed, (long)&view, &view);

      // Cleanup doc if last view was just closed and dtAutoDelete
      // or dtAutoOpen is set. dtAutoOpen will cause an autoclose, while
      // dtAutoDelete will delete this doc also.
      //
      if (!ViewList) {
        if (Template && ((Template->Flags & dtAutoDelete) ||
                         (Template->Flags & dtAutoOpen))) {
          // Close document streams
          //
          if (IsOpen())
            Close();

          // Returning true will cause ~TView to delete document. Using
          // 'view.IsOK()' caters for cases where TView's construction failed.
          //
          return (Template->Flags & dtAutoDelete) && view.IsOK();
        }
      }
      else {
        ReindexFrames();
      }
      break;
    }
  }
  return false;
}

//
/// Saves the current data to storage. When a file is closed, the document manager
/// calls either Commit or Revert. If force is true, all data is written to storage.
/// Commit checks any child documents and commits their changes to storage also.
/// Before the current data is saved, all child documents must return true. If all
/// child documents return true, Commit flushes the views for operations that
/// occurred since the last time the view was checked. After all data for the
/// document is updated and saved, Commit returns true.
//
bool
TDocument::Commit(bool force)
{
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0) {
    if (!pdoc->Commit(force))
      return false;
  }

  WARNX(OwlDocView, !DocPath, 0, _T("Commit(): 0 DocPath!"));
  return NotifyViews(vnCommit, force);
}

//
/// Performs the reverse of Commit() and cancels any changes made to the document
/// since the last commit. If clear is true, data is not reloaded for views. Revert
/// also checks all child documents and cancels any changes if all children return
/// true. When a file is closed, the document manager calls either Commit() or Revert.
/// Returns true if the operation is successful.
//
bool
TDocument::Revert(bool clear)
{
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0) {
    if (!pdoc->Revert(clear))
      return false;
  }
  return NotifyViews(vnRevert, clear);
}

//
/// Notifies the views of the current document and the views of any child documents
/// of a change. In contrast to QueryViews, NotifyViews sends notification of an
/// event to all views and returns true if all views returned a true result. The
/// event, EV_OWLNOTIFY, is sent with an event code, which is private to the
/// particular document and view class, and a long argument, which can be cast
/// appropriately to the actual type passed in the argument of the response
/// function.
//
bool
TDocument::NotifyViews(int event, long item, TView* exclude)
{
  bool answer = true;

  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0)
    answer = (answer && pdoc->NotifyViews(event, item, exclude));

  TEventHandler::TEventInfo eventInfo(WM_OWLNOTIFY, event);
  for (TView* view = ViewList; view != 0; view = view->NextView)
    if (view != exclude && view->Find(eventInfo))
      answer = (answer && (view->Dispatch(eventInfo, 0, item) != 0));

  return answer;
}

//
/// Queries the views of the current document and the views of any child documents
/// about a specified event, but stops at the first view that returns true. In
/// contrast to NotifyViews(), QueryViews returns a pointer to the first view that
/// responded to an event with a true result. The event, EV_OWLNOTIFY, is sent with
/// an event code (which is private to the particular document and view class) and a
/// long argument (which can be cast appropriately to the actual type passed in the
/// argument of the response function).
//
TView*
TDocument::QueryViews(int event, long item, TView* exclude)
{
  TView* view;
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0)
    if ((view = pdoc->QueryViews(event, item, exclude)) != 0)
      return view;

  TEventHandler::TEventInfo eventInfo(WM_OWLNOTIFY, event);
  for (view = ViewList; view != 0; view = view->NextView) {
    if (view != exclude) {
      if (view->Find(eventInfo)) {
        if (view->Dispatch(eventInfo, 0, item)) {
          return view;            // Return first acknowledger
        }
      }
    }
  }
  return 0;
}

//
/// Returns true if the document or one of its views has changed but has not been
/// saved.
//
bool
TDocument::IsDirty()
{
  if (DirtyFlag)
    return true;

  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0)
    if (pdoc->IsDirty())
      return true;

  return QueryViews(vnIsDirty) != 0;
}

//
/// Used by the document manager, HasFocus returns true if this document's view has
/// focus. hwnd is a handle to the document. to determine if the document contains a
/// view with a focus.
//
bool
TDocument::HasFocus(HWND hWnd)
{
  return DocWithFocus(hWnd) != 0;
}

//
/// Return pointer to this document or one of its child documents if the spcecified
/// window parameter is a view associated with the document.
/// \note Unlike 'HasFocus', this method allows you to distinguish whether the
/// document with focus is a child document.
//
TDocument*
TDocument::DocWithFocus(HWND hWnd)
{
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0)
    if (pdoc->DocWithFocus(hWnd))
      return pdoc;

  return QueryViews(vnIsWindow, (long)hWnd) ? this : 0;
}

//
/// Checks to see if all child documents can be closed before closing the current
/// document. If any child returns false, CanClose returns false and aborts the
/// process. If all children return true, calls TDocManager::FlushDoc. If FlushDoc
/// finds that the document has been changed but not saved, it displays a message
/// asking the user to either save the document, discard any changes, or cancel the
/// operation. If the document has not been changed and all children's CanClose
/// functions return true, this CanClose function returns true.
//
bool
TDocument::CanClose()
{
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0)
    if (!pdoc->CanClose())
      return false;

  return DocManager->FlushDoc(*this);  // do the UI in the doc manager
}

//
/// Closes the document but does not delete or detach the document. Before closing
/// the document, Close checks any child documents and tries to close them before
/// closing the parent document. Even if you write your own Close function, call
/// TDocument's version to ensure that all child documents are checked before the
/// parent document is closed.
//
bool
TDocument::Close()
{
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != 0)
    if (!pdoc->Close())
      return false;

  return true;
}

//
/// Posts the error message passed as a string resource ID in sid. choice is one or
/// more of the MB_Xxxx style constants.
//
uint
TDocument::PostError(uint sid, uint choice)
{
  return DocManager->PostDocError(*this, sid, choice);
}

//----------------------------------------------------------------------------

//
//
//
TDocument*
TDocument::TList::Next(const TDocument* doc)
{
  return doc ? doc->NextDoc : DocList;
}

//
//
//
bool
TDocument::TList::Insert(TDocument* doc)
{
  TDocument* pdoc;
  for (pdoc = DocList; pdoc; pdoc = pdoc->NextDoc)
    if (pdoc == doc)
      return false;
  doc->NextDoc = DocList;
  DocList = doc;
  return true;
}

//
//
//
bool
TDocument::TList::Remove(TDocument* doc)
{
  TDocument** ppdoc;
  for (ppdoc = &DocList; *ppdoc; ppdoc = &(*ppdoc)->NextDoc) {
    if (*ppdoc == doc) {
      *ppdoc = doc->NextDoc;
      return true;
    }
  }
  return false;
}

//
//
//
void
TDocument::TList::Destroy()
{
  while (DocList)
    delete DocList;   // removes it entry from destructor
}


IMPLEMENT_ABSTRACT_STREAMABLE(TDocument);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TDocument::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TDocument* o = GetObject();

  o->NextDoc = 0;
  o->StreamList = 0;
  o->DocManager = 0;
  o->DirtyFlag = false;

  is >> o->OpenMode;
#if defined(UNICODE)
  _USES_CONVERSION;
  char * docPath = is.freadString();
  char * title = is.freadString();

  o->DocPath = _A2W(docPath);
  o->Title   = _A2W(title);

  delete[] docPath;
  delete[] title;
#else
  o->DocPath = is.freadString();
  o->Title   = is.freadString();
#endif
  is >> o->Template;  // static templates must have been already streamed
  is >> o->ParentDoc;
  o->ViewList = 0;    // must init, does not get set until after view streamed
  is >> o->ViewList;

  is >> TView::NextViewId;  // static, but must get set by at least 1 document

  return o;
}

//
//
//
void
TDocument::Streamer::Write(opstream& os) const
{
  TDocument* o = GetObject();

  while (!o->CanClose())   // can't permit cancel here
    ;
  os << o->OpenMode;
  _USES_CONVERSION;
  os.fwriteString(_W2A(o->DocPath));
  os.fwriteString(_W2A(o->Title));
  os << o->Template;       // templates already streamed, must be so if static
  os << o->ParentDoc;
  os << o->ViewList;       // each view streams out the next
  os << TView::NextViewId; // insure that this static var gets set on reload
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

