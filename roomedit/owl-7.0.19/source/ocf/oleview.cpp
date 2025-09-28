//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TOleView. Doc/View view derived from TOleWindow that
/// supports OLE 2 using OCF TOcView & TOcRemView
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <owl/window.h>
#include <owl/gdiobjec.h>
#include <owl/scroller.h>
#include <owl/docmanag.h>
#include <owl/docview.rh>
#include <ocf/oleview.rh>
#include <ocf/oleframe.h>
#include <ocf/oleview.h>
#include <ocf/oledoc.h>

namespace ocf {

using namespace owl;

OWL_DIAGINFO;

//----------------------------------------------------------------------------
// TOleView
//

DEFINE_RESPONSE_TABLE1(TOleView, TOleWindow)
  EV_VN_ISWINDOW,

  // Container specific messages
  //
  EV_VN_INVALIDATE,
  EV_VN_DOCOPENED,
  EV_VN_DOCCLOSED,
  EV_OC_VIEWPARTINVALID,

  // Server specific messages
  //
  EV_OC_VIEWSAVEPART,
  EV_OC_VIEWLOADPART,
  EV_OC_VIEWINSMENUS,
  EV_OC_VIEWCLOSE,
  EV_OC_VIEWOPENDOC,
  EV_OC_VIEWATTACHWINDOW,
  EV_OC_VIEWSETLINK,
  EV_OC_VIEWBREAKLINK,
  EV_OC_VIEWGETITEMNAME,
END_RESPONSE_TABLE;

//
/// Constructs a TOleView object associated with the given document object (doc) and
/// parent window (parent).
//
TOleView::TOleView(TDocument& doc, TWindow* parent)
:
  TView(doc),
  TOleWindow(parent, doc.GetDocManager().GetApplication())
{
  Destroying = false;
  TOleDocument* oleDoc = TYPESAFE_DOWNCAST(&GetDocument(), TOleDocument);
  CHECK(oleDoc);
  OcDoc = oleDoc->GetOcDoc(); // Let OleWindow member point to it for accessor
}

//
/// Destroys the TOleView object and detaches the view from the associated document.
//
TOleView::~TOleView()
{
  Destroying = true;
  if (IsRemote())
    Destroy();  // Make sure that derived TWindow shutdown virtuals get called
  OcDoc = 0;    // We don't own it, don't let TOleWindow delete it
}

//
/// Override TOleWindow's version to pass info to TOleDocument & provide a
/// second chance to find the RegLink.
///
/// Creates an ObjectComponents view associated with the embedded object. Associates
/// the view with the document template specified in tpl. The isEmbedded parameter
/// is true if the view is an embedded object. The outer parameter refers to the
/// IUnknown interface with which the view will aggregate itself.
//
TOcView*
TOleView::CreateOcView(TRegLink* link, bool isRemote, IUnknown* outer)
{
  // Assume an embedded document until we find out later if we are a link
  // or a load-from-file
  //
  if (isRemote)
    GetDocument().SetEmbedded(true);

  return TOleWindow::CreateOcView(link ? link : GetDocument().GetTemplate(),
                                  isRemote, outer);
}

//
/// Overrides TView's GetViewMenu to make an on-the-fly decision about which menu to
/// use: normal, or embedded.
//
TMenuDescr*
TOleView::GetViewMenu()
{
  if (TView::GetViewMenu())
    return TView::GetViewMenu();

// !CQ && not a link!
  if (IsRemote() && GetModule()->FindResource(IDM_OLEVIEWEMBED, RT_MENU))
    SetViewMenu(new TMenuDescr(IDM_OLEVIEWEMBED, GetModule()));
  else
    SetViewMenu(new TMenuDescr(IDM_OLEVIEW, GetModule()));

  return TView::GetViewMenu();
}

//
/// Does a given THandle belong to this view? Yes if it is us, or a child of us
//
bool
TOleView::VnIsWindow(THandle hWnd)
{
  return hWnd == GetHandle() || IsChild(hWnd);
}

//
/// A view uses this function to verify whether or not it can shut down. If this is
/// a server's view window, CanClose checks to see if any open-editing is occurring
/// on any of the embedded objects in the frame window.  If so, CanClose closes this
/// open-editing session by disconnecting the embedded object from its server. Then,
/// it hides the server's frame window and returns true when appropriate.  If this
/// is a container, CanClose queries all documents and views and returns true when
/// all documents and views can be closed.
///
/// \note Unlike in-place editing, which takes place in the container's window,
/// open-editing occurs in the server's frame window.
//
bool
TOleView::CanClose()
{
  // We don't want to close the view for DLL servers
  //
  if (IsOpenEditing() && !OcApp->IsOptionSet(amExeMode)) {
    TOleFrame* olefr = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(), TOleFrame);
    CHECK(olefr);
    olefr->ShowWindow(SW_HIDE);
    TOleWindow::OleShutDown();
    return false;
  }

  // Say yes if there are more than one TOleView's attached to the document
  //
  if (OtherViewExists() || GetOcRemView())
    return true;

  if (Doc->CanClose()) {
    if (OcDoc)
      OcDoc->Close();
    return true;
  }
  return false;
}

//
/// Checks whether another TOleView already exists.
//
bool
TOleView::OtherViewExists()
{
  TView* curView = GetDocument().GetViewList();
  while (curView) {
    TOleLinkView* oleLinkView = TYPESAFE_DOWNCAST(curView, TOleLinkView);
    if (!oleLinkView && curView != this)
      return true;

    curView = curView->GetNextView();
  }

  return false;
}

//
/// Performs a normal CleanupWindow.  Also lets the OcView object know we have closed.
//
void
TOleView::CleanupWindow()
{
  if (!OtherViewExists()) {

    TOleWindow::CleanupWindow();

    // Delete the TOleView now rather wait until ~TOleFrame if its parent
    // is TRemViewBucket
    //
    TOleFrame* mainWindow = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(),
                                            TOleFrame);
    if (mainWindow && mainWindow->GetRemViewBucket() == Parent && !Destroying)
      GetApplication()->Condemn(this);
  }
}

//
/// Shuts down the associated OCF partners if possible.
//
bool
TOleView::OleShutDown()
{
  // if dll server open edit return
  if (OcApp->IsOptionSet(amExeMode) || !IsOpenEditing())
    TOleWindow::OleShutDown();
  else {
    TOleWindow::OleShutDown();
    if (OcDoc && !OtherViewExists())
      OcDoc->Close();
  }
  return true;
}


//
/// Invalidates the view region specified by p. Use this function to invalidate the
/// bounding rectangle surrounding an embedded object if the object has been
/// changed, usually as a result of in-place editing. If successful, returns true.
//
bool
TOleView::VnInvalidate(TRect& rect)
{
  InvalidateRect(rect, true);

  return true;
}

//
/// Ensures that TOleView's data members, such as DragPart, Pos, and Scale, are
/// initialized properly after a revert operation, which cancels any changes made to
/// the document since the last time the document was saved to storage.
//
bool
TOleView::VnDocOpened(int /*omode*/)
{
  DragPart = 0;
  Pos.SetNull();
  Scale.Reset();
  return true;
}

//
/// Sets the OcDoc data member to 0.
//
bool
TOleView::VnDocClosed(int /*omode*/)
{
  OcDoc = 0;
  return true;
}

//
/// Asks the server to close the view associated with this document. Tests to see if
/// the document has been changed since it was last saved. Returns true if the
/// document and its associated view are closed.
//
bool
TOleView::EvOcViewClose()
{
  TOcRemView* ocRemView = GetOcRemView();

  // When TOcRemView gets shut down in the Embed From File case,
  // we need to cleanup the document and view right away. Otherwise,
  // the document and view will be shut down as part of the frame shut down
  // process.
  //
  if (ocRemView && ocRemView->GetKind() == TOcRemView::LoadFromFile) {
    OcView = 0;   // OcView is going away, so don't mess with it
    OcDoc = 0;    // OleDoc will delete OcDoc, so don't mess with it
    delete &GetDocument();
  }
  else {
    TOleWindow::EvOcViewClose();
  }

  return true;
}

//
/// Asks the server to save the embedded object's data to storage. To save the
/// object, EvOcViewSavePart calls upon the TOleDocument object, which creates
/// storage as necessary for each embedded object. Saves the dimensions of the
/// server's view, which the server uses to tell the container how to redraw the
/// embedded object in the container's window.
//
bool
TOleView::EvOcViewSavePart(TOcSaveLoad & ocSave)
{
  PRECONDITION(ocSave.StorageI);

  TOleDocument* doc = TYPESAFE_DOWNCAST(&GetDocument(), TOleDocument);
  if (!doc)
    return false;

  doc->SetStorage(ocSave.StorageI, (ocSave.SameAsLoad || ocSave.Remember));

  bool status;
  if (ocSave.SaveSelection) {
    status = doc->CommitSelection(*this, ocSave.UserData);
  }
  else {
    // Save view remote view info such as origin and extent
    //
    TOcRemView* ocRemView = GetOcRemView();
    if (ocRemView)
      ocRemView->Save(ocSave.StorageI);

    status = doc->Commit(true);
  }

  // Restore the original storage
  //
  if (!ocSave.SameAsLoad && !ocSave.Remember)
    doc->RestoreStorage();

  return status;
}

//
/// Asks the server to load itself from storage. Loads the document and its
/// associated view.
//
bool
TOleView::EvOcViewLoadPart(TOcSaveLoad & ocLoad)
{
  PRECONDITION(ocLoad.StorageI);

  TOleDocument* doc = TYPESAFE_DOWNCAST(&GetDocument(), TOleDocument);
  doc->SetStorage(ocLoad.StorageI);

  // Load view remote view info such as origin and extent
  //
  CHECK(GetOcRemView());
  GetOcRemView()->Load(ocLoad.StorageI);

  bool status = GetDocument().Open(doc->GetOpenMode());

  if (!ocLoad.Remember)
    doc->SetStorage(0);

  return status;
}

//
/// Asks the container application to open an existing document so the document can
/// receive embedded and linked objects. (Actually, TOleView calls on the
/// TOleDocument object to read the document from storage, using the standard OLE
/// IStorage and IStream interfaces). Assigns a unique string identifier to the
/// document and returns true if successful.
//
bool
TOleView::EvOcViewOpenDoc(LPCTSTR path)
{
  TOleDocument* oleDoc = TYPESAFE_DOWNCAST(&GetDocument(), TOleDocument);
  CHECK(oleDoc && GetOcDoc());

  oleDoc->SetEmbedded(false); // must really be a link or load-from-file
  oleDoc->SetStorage(0);      // release the current storage
  oleDoc->SetDocPath(path);
  oleDoc->InitDoc();
  oleDoc->Open(ofRead, path);

  owl::tstring newName(oleDoc->GetDocPath());
  GetOcDoc()->SetName(newName);
  Invalidate();
  return true;
}

//
/// Inserts the server's menu into the composite menu. Determines the number of
/// groups and the number of pop-up menu items to insert within each group. The
/// shared menu (sharedMenu) is the container's menu merged with the server's menu
/// groups.
//
bool
TOleView::EvOcViewInsMenus(TOcMenuDescr & sharedMenu)
{
  // Recreate a temporary composite menu for frame and child
  //
  TMenuDescr compMenuDesc; // empty menudescr
  if (GetViewMenu()) {
    compMenuDesc.Merge(*GetViewMenu());
    compMenuDesc.Merge(TMenuDescr(0,  -1, 0, -1, 0, -1, 0));
  }

  TMenuDescr shMenuDescr(sharedMenu.HMenu,
                         sharedMenu.Width[0],
                         sharedMenu.Width[1],
                         sharedMenu.Width[2],
                         sharedMenu.Width[3],
                         sharedMenu.Width[4],
                         sharedMenu.Width[5]);
  shMenuDescr.Merge(compMenuDesc);

  for (int i = 0; i < 6; i++)
    sharedMenu.Width[i] = shMenuDescr.GetGroupCount(i);

  return true;
}

//
/// Notifies the active view of any changes made to the embedded object's data
/// (changeInfo).  Also, notifies any other views associated with this document that
/// the bounding rectangle for the document is invalid and needs to be repainted.
/// EvOcViewPartInvalid always returns true.
//
bool
TOleView::EvOcViewPartInvalid(TOcPartChangeInfo& changeInfo)
{
  if (changeInfo.IsDataChange())
    GetDocument().SetDirty(true);

  // Reflect the change in part in other (non-active) views
  //
  TRect rect(changeInfo.GetPart()->GetRect());
  rect.right++;
  rect.bottom++;
  TOleClientDC dc(*this);
  dc.LPtoDP((TPoint*)&rect, 2);

  GetDocument().NotifyViews(vnInvalidate, reinterpret_cast<TParam2>(&rect));

  // Notify container if this is an intermediate container
  //
  InvalidatePart((TOcInvalidate)changeInfo.GetType());

  return true;  // stop further processing by OCF
}

//
/// Attaches this view to its ObjectWindows parent window so the embedded object can
/// be either opened and edited or deactivated. To attach a view to an embedded
/// object, set the attach parameter to true. To detach the embedded object, set the
/// attach parameter to false.
//
bool
TOleView::EvOcViewAttachWindow(bool attach)
{
  TOleFrame* mainWindow = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(),
                                            TOleFrame);
  if (!mainWindow)
    return false;  // server app is shutting down

  // There won't be any TOcRemView if we're reestablishing the link
  //
  if (attach) {
    if (IsOpenEditing()) {
      // Get the normal app notify handler to set up the parent for us
      // knowing that we are now open editing
      //
      if (mainWindow->GetRemViewBucket() == Parent) {
        GetDocument().GetDocManager().PostEvent(dnCreate, *this);
      }
    }
  }
  else {
    if (IsOpenEditing() && Parent != mainWindow)
      Parent->PostMessage(WM_CLOSE);
    SetParent(mainWindow->GetRemViewBucket());  // simple reparent
  }
  return true;
}

//
/// Finds the item name for whole document or for the selection.
//
bool
TOleView::EvOcViewGetItemName(TOcItemName& item)
{
  if (item.Selection) {
    if (DragPart) {
      item.Name = DragPart->GetName();
      return true;
    }
  }
  else {
    item.Name = _T("Content"); // item name representing the whole document
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
// Linking Spport
//

//
/// Responds to an OC_VIEWSETLINK message TOcLinkView sends when the server document
/// provides a link to a container document. EvOcViewSetLink establishes the link
/// between a TOleLinkView and a TOcLinkView. The view parameter references the view
/// with which the document or selection is associated. Returns false if
/// unsuccessful.
/// Non-Doc/View applications use TOleWIndow's implementation of the function.
//
bool
TOleView::EvOcViewSetLink(TOcLinkView& /*view*/)
{
  return false;
}

//
/// Responds to an OC_VIEWBREAKLINK message that TOcLinkView sends when the server
/// document that provides the link shuts down. EvOcViewBreakLink breaks the link
/// with a server document or a selection by deleting the TOleLinkView associated
/// with the TOcLinkView (view). After the link is broken, the container application
/// is left holding a static representation (that is, a metafile) of the linked
/// document. Returns false if unsuccessful.
/// Non-Doc/View applications use TOleWindow's implementation of this function.
//
bool
TOleView::EvOcViewBreakLink(TOcLinkView& view)
{
  // Find the link view with the moniker
  //
  TView* target = GetDocument().QueryViews(vnLinkView, reinterpret_cast<TParam2>(&view), this);

  // Delete a linked view to this document
  //
  delete target;
  return true;
}

IMPLEMENT_STREAMABLE2(TOleView, TOleWindow, TView);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TOleView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  owl::ReadBaseObject((TOleWindow*)GetObject(), is);
  owl::ReadBaseObject((TView*)GetObject(), is);

  GetObject()->Destroying = false;
  TOleDocument* oleDoc = TYPESAFE_DOWNCAST(&GetObject()->GetDocument(), TOleDocument);
  CHECK(oleDoc);
  GetObject()->OcDoc = oleDoc->GetOcDoc(); // Let OleWindow member point to it for accessor
  CHECK(oleDoc);

  return GetObject();
}

//
//
//
void
TOleView::Streamer::Write(opstream& os) const
{
  owl::WriteBaseObject((TOleWindow*)GetObject(), os);
  owl::WriteBaseObject((TView*)GetObject(), os);
}

#endif

//----------------------------------------------------------------------------
// TOleLinkView
//

DEFINE_RESPONSE_TABLE(TOleLinkView)
  EV_VN_LINKVIEW,
  EV_VN_LINKMONIKER,
END_RESPONSE_TABLE;

//
/// Constructs a TOleLinkView object associated with the given document object (doc)
/// and view (view).
//
TOleLinkView::TOleLinkView(TDocument& doc, TOcLinkView& view)
:
  TView(doc),
  OcLinkView(view)
{
  view.AddRef();
}

//
/// Destroys the TOleLinkView object and detaches the view from the associated
/// document.
//
TOleLinkView::~TOleLinkView()
{
  OcLinkView.Release();
}

//
/// Returns true if a TOleLinkView object is associated with the server's
/// TOcRemView, the server's remote link view object. A TOcRemView is the object
/// created by a linking and embedding server so that the server can draw its OLE
/// object in a metafile used by the container. In contrast to VnLinkMoniker, this
/// function searches for the TOleLinkView object using a reference to the view.
//
bool
TOleLinkView::VnLinkView(TOcLinkView& view)
{
  if (&OcLinkView == &view)
    return true;

  return false;
}

//
/// Returns true if a TOleLinkView object is associated with the given server's
/// TOcRemView. In contrast to VnLinkView, this function searches for the view using
/// the specified server's moniker.
/// When the document receives a request for the TOleLinkView associated with a
/// particular moniker, the document sends a vnLinkView notification message to all
/// its attached views. The handler for vnLinkMoniker in TOleLinkView simply returns
/// true if the handler finds a view associated with the moniker.
//
bool
TOleLinkView::VnLinkMoniker(TString& moniker)
{
  if ((TCHAR*)OcLinkView.GetMoniker() == (TCHAR*)moniker)
    return true;

  return false;
}

//
/// When any changes occur to the server document, UpdateLinks updates all
/// containers linked to the view of the server document. If successful, it returns
/// true.
//
bool
TOleLinkView::UpdateLinks()
{
  OcLinkView.Invalidate(invView);
  return true;
}

//
/// Returns the moniker for the selection in a server document associated with this
/// TOleLinkView container's view. By looking at the moniker, the application can
/// find the corresponding objects in its document.
//
/// \note Monker is the source file's path name and the object hierarchy for a linked object.
/// A moniker functions much like a map by showing where the linked object's data is
/// stored and explaining how to find the data. For example, the moniker returned
/// from a word processor for a selected range of text could be the start and end
/// offset of a text stream. The moniker returned for a spreadsheet range could be
/// something like A1:D6. Anything that a server application can use to map to its
/// data can be used as a moniker.
TString&
TOleLinkView::GetMoniker()
{
  return OcLinkView.GetMoniker();
}

} // OCF namespace

//==============================================================================

