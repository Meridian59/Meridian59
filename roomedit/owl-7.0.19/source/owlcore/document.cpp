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
  Tag(nullptr),
  DirtyFlag(false),
  Embedded(false),
  ParentDoc(parent),
  NextDoc(nullptr),
  OpenMode(0),
  Title(nullptr),
  Template(nullptr),
  ViewList(nullptr),
  StreamList(nullptr),
  DocPath(nullptr)
{
  if (parent) {
    DocManager = parent->DocManager;

    // Handle case of 'dummy' parent [parent parented to itself] which was
    // created by the docmanager simply to pass itself to us
    //
    if (parent->ParentDoc == parent) {
      ParentDoc = nullptr;
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
  SetTemplate(nullptr);

  DestroyChildren();

  DestroyViews();

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
    const auto ok = DocManager != nullptr;
    WARN(!ok, _T("TDocument::~TDocument: Terminating due to failed precondition."));
    if (!ok) std::terminate();

    DocManager->PostEvent(dnClose, *this); // WM_OWLDOCUMENT
    DocManager->DocList.Remove(this);
  }

  delete[] Title;
  delete[] DocPath;
}

//
//
//
static const LPCTSTR PropNames[] = {
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
    return nullptr;
  }
  else if (index < NextProperty)
    return PropNames[index-PrevProperty-1];
  else {
    TRACEX(OwlDocView, 0, _T("PropertyName(): index >= PrevProperty!"));
    return nullptr;
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
  PRECONDITION(name != nullptr);
  int i;
  for (i=0; i < NextProperty-PrevProperty-1; i++)
    if (_tcscmp(PropNames[i], name) == 0)
      return i+PrevProperty+1;

  TRACEX(OwlDocView, 0, _T("FindProperty: Index of [") << name << _T("] not found") );
  return 0;
}

//
/// Retrieves the property identified by the given index.
/// If the requested property is text, then `dest` should point to a text buffer, and `textlen`
/// should specify the maximum number of characters the buffer can hold, excluding the terminating
/// null-character, i.e. the buffer must have room for (`textlen` + 1) characters.
///
/// If the requested property is numerical, then it may be requested either as text or in its
/// binary form. To request the property as text, pass a text buffer as described above. To request
/// the property in binary form, `dest` should point to storage of sufficent size, and `textlen`
/// should be zero.
///
/// Non-text properties without textual representation, e.g. file handles, may only be requested
/// in binary form, i.e. `dest` must point to sufficient storage, and `textlen` must be zero.
///
/// \return If the parameter `textlen` is non-zero, which means that the property is requested in
/// string form, the function returns the length of the string, i.e. the character count excluding
/// the terminating null-character. If the parameter `textlen` is zero, which means that property
/// is requested in binary form, the return value is the size of the data in bytes.
///
/// If the property is text, and `textlen` is zero, the function fails and returns 0. The function
/// also fails and returns 0 if `textlen` is non-zero and the property requested can not be
/// expressed as text. It also returns 0 if the property is not defined.
///
/// \sa TDocument::TDocProp
//
int
TDocument::GetProperty(int index, void * dest, int textlen)
{
  LPCTSTR src;
  tchar   numBuf[15];
  switch (index) {
    case DocumentClass: {
        _USES_CONVERSION;
        src = _A2W(_OBJ_FULLTYPENAME(this));
      }
      break;

  case TemplateName:
    src = Template ? Template->GetDescription() : nullptr;
    break;

    case ViewCount: {
      int cnt;
      TView* view;
      for (view=ViewList, cnt=0; view != nullptr; view=view->NextView, cnt++)
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
             << index << _T("] specified!") );
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
  *(reinterpret_cast<LPTSTR>(dest) + textlen) = 0;
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
      SetTitle(reinterpret_cast<LPCTSTR>(src));
      break;

    case StoragePath:
      return SetDocPath(reinterpret_cast<LPCTSTR>(src));

    default:
      TRACEX(OwlDocView, 0, _T("SetProperty(): invalid prop [") << prop <<\
            _T("] specified!"));
      return false;
  }
  return true;
}

//
/// Destroy children first if we have any. Then force close here as a last
/// resort if derived classes have not done so. Since we have destructed down
/// to a TDocument by now, derived closes will not be called.
//
void TDocument::DestroyChildren()
{
  ChildDoc.Destroy();
  Close();
}

//
/// Destroys the views attached to this document.
//
void TDocument::DestroyViews()
{
  // NOTE: View's destructor invokes 'DetachView' which removes it from
  //       the list
  //
  while (ViewList)
    delete ViewList;
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
  delete[] DocPath;
  DocPath = (path && *path) ? strnewdup(path) : nullptr;

  tchar title[_MAX_PATH] = _T("Unknown");  // Never used - but just in case!
  if (!DocPath || GetFileTitle(DocPath, title, COUNTOF(title)) != 0) {
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
    Title = title ? strnewdup(title) : nullptr;
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

  for (seq = -1, view = ViewList; view != nullptr; view = view->NextView) {
    seq -= view->SetDocTitle(Title, seq);  // decrement if title displayed
    if (seq == -3)   // need only check if more than one title displayed
      break;
  }
  if (seq == -1)
    return;
  seq = (seq == -2 ? 0 : 1);
  for (view = ViewList; view != nullptr; view = view->NextView)
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
  view.NextView = nullptr;
  view.Doc = this;
  NotifyViews(vnViewOpened, reinterpret_cast<TParam2>(&view), &view);
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
    return nullptr;
  }
  if (!view->IsOK()) {
    TRACEX(OwlDocView, 0, _T("InitView(): Invalid view object specified!"));
    delete view;
    return nullptr;
  }

  CHECK(DocManager);
  DocManager->PostEvent(dnCreate, *view); // WM_OWLVIEW

  if (!view->IsOK()) {
    TRACEX(OwlDocView, 0, _T("InitView(): Invalid view object post dnCreate!"));
    delete view;
    return nullptr;
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
      DocManager->PostEvent(dnClose, view); // WM_OWLVIEW
      *plist = view.NextView;
      NotifyViews(vnViewClosed, reinterpret_cast<TParam2>(&view), &view);

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
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr) {
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
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr) {
    if (!pdoc->Revert(clear))
      return false;
  }
  return NotifyViews(vnRevert, clear);
}

//
/// Notifies the views of this document, and the views of any child documents, of a change.
///
/// The notification message, WM_OWLNOTIFY, is sent with the given eventId, which is
/// private to the particular document class, and a single generic argument, which will be
/// cast to the actual type of the parameter of the response function. If the optional
/// parameter `exclude` is set, the appointed view will not be notified.
///
/// In contrast to QueryViews, NotifyViews sends the notification message to all views,
/// regardless of the return values. NotifyViews returns `true`, if and only if all views
/// return `true`. If a view does not handle the notification, then a `true` return value is
/// assumed. In other words, NotifyViews will return `false` only if a view handles the
/// notification and explicitly returns `false`. In any case, all views are notified.
//
bool
TDocument::NotifyViews(int eventId, TParam2 param, TView* exclude)
{
  bool answer = true;

  for (auto d = ChildDoc.Next(nullptr); d; d = ChildDoc.Next(d))
  {
    bool r = d->NotifyViews(eventId, param, exclude);
    answer = answer && r;
  }

  TEventHandler::TEventInfo eventInfo(WM_OWLNOTIFY, eventId);
  for (TView* view = ViewList; view != nullptr; view = view->NextView)
  {
    if (view == exclude) continue;

    if (view->Find(eventInfo))
    {
      bool r = view->Dispatch(eventInfo, 0, param) != 0;
      answer = answer && r;
    }
  }
  return answer;
}

//
/// Notifies the views of this document of a change. Does not notify views of child documents.
///
/// The notification message, WM_OWLNOTIFY, is sent with the given eventId, which is
/// private to the particular document class, and a single generic argument, which will be
/// cast to the actual type of the parameter of the response function. If the optional
/// parameter `exclude` is set, the appointed view will not be notified.
///
/// In contrast to QueryViews, NotifyOwnViews sends the notification message to all views,
/// regardless of the return values. NotifyOwnViews returns `true`, if and only if all views
/// return `true`. If a view does not handle the notification, then a `true` return value is
/// assumed. In other words, NotifyOwnViews will return `false` only if a view handles the
/// notification and explicitly returns `false`. In any case, all views are notified.
//
bool
TDocument::NotifyOwnViews(int eventId, TParam2 param, TView* exclude)
{
  bool answer = true;

  TEventHandler::TEventInfo eventInfo(WM_OWLNOTIFY, eventId);
  for (TView* view = ViewList; view != nullptr; view = view->NextView)
  {
    if (view == exclude) continue;

    if (view->Find(eventInfo))
    {
      bool r = view->Dispatch(eventInfo, 0, param) != 0;
      answer = answer && r;
    }
  }
  return answer;
}

//
/// Queries the views of the current document, and the views of any child documents,
/// about a specified event.
///
/// The notification message, WM_OWLNOTIFY, is sent with the given eventId, which is
/// private to the particular document class, and a single generic argument, which will be
/// cast to the actual type of the parameter of the response function. If the optional
/// parameter `exclude` is set, the appointed view will not be queried.
///
/// In contrast to NotifyViews, the query stops at the first view that returns `true`, and
/// the pointer to the view is returned. If no view returns `true`, `nullptr` is returned.
//
TView*
TDocument::QueryViews(int eventId, TParam2 param, TView* exclude)
{
  TView* view;
  TDocument* pdoc = 0;
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr)
    if ((view = pdoc->QueryViews(eventId, param, exclude)) != nullptr)
      return view;

  TEventHandler::TEventInfo eventInfo(WM_OWLNOTIFY, eventId);
  for (view = ViewList; view != nullptr; view = view->NextView)
  {
    if (view == exclude) continue;

    if (view->Find(eventInfo))
      if (view->Dispatch(eventInfo, 0, param))
        return view; // Return first acknowledger
  }
  return nullptr;
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
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr)
    if (pdoc->IsDirty())
      return true;

  return QueryViews(vnIsDirty) != nullptr;
}

//
/// Used by the document manager, HasFocus returns true if this document's view has
/// focus. hwnd is a handle to the document. to determine if the document contains a
/// view with a focus.
//
bool
TDocument::HasFocus(HWND hWnd)
{
  return DocWithFocus(hWnd) != nullptr;
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
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr)
    if (pdoc->DocWithFocus(hWnd))
      return pdoc;

  return QueryViews(vnIsWindow, reinterpret_cast<TParam2>(hWnd)) ? this : nullptr;
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
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr)
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
  while ((pdoc = ChildDoc.Next(pdoc)) != nullptr)
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
bool TDocument::TList::Contains(TDocument* doc)
{
  for (TDocument* pDoc = DocList; pDoc; pDoc = pDoc->NextDoc)
    if (pDoc == doc)
      return true;
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

#if OWL_PERSISTENT_STREAMS

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

#endif

} // OWL namespace

