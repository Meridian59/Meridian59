//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TOleDocument. Doc/View document that supports OLE 2
/// using OCF TOcDocument
//----------------------------------------------------------------------------
#include <ocf/pch.h>


#include <owl/docmanag.h>
#include <ocf/olemdifr.h>
#include <ocf/ocdoc.h>
#include <ocf/ocapp.h>
#include <ocf/oledoc.h>
#include <ocf/oleframe.h>
#include <ocf/oleview.h>

namespace ocf {

using namespace owl;

OWL_DIAGINFO;

//
/// Constructs a TOleDocument object associated with the given parent TDocument object.
//
TOleDocument::TOleDocument(TDocument* parent)
:
  TStorageDocument(parent),
  OcDoc(0),
  Closing(false)
{
}

//
/// Destroys the TOleDocument object.  In the case of an OLE container, the compound
/// file remains open until all the views shut down.
//
TOleDocument::~TOleDocument()
{
  delete OcDoc;
}

//
/// Prepares the document for closing. Before closing the current document, checks
/// to see if all child documents can be closed. If any child returns false,
/// CanClose returns false and aborts the process. If all children return true,
/// CanClose checks to see if the document has been changed. If so, it asks the user
/// to save the document, discard any changes, or cancel the operation. If the
/// document has not been changed and all child documents return true, this CanClose
/// function returns true, thus indicating that the document can be closed.
/// CanClose also calls ReleaseDoc on its associated ObjectComponents document to
/// make sure that all the embedded objects are closed properly.
//
bool
TOleDocument::CanClose()
{
  //
  // if it's an open edit dll stop the closing process
  TView* curView = GetViewList();
  while (curView) {
    // get the ole view
    TOleView* oleView = TYPESAFE_DOWNCAST(curView, TOleView);
    if (oleView && oleView->IsOpenEditing() && !GetOcApp()->IsOptionSet(amExeMode)) {
      TOleFrame* olefr = TYPESAFE_DOWNCAST(oleView->GetApplication()->GetMainWindow(), TOleFrame);
      CHECK(olefr);
      olefr->ShowWindow(SW_HIDE);
      oleView->OleShutDown();
      return false; // don't close
    }
    curView = curView->GetNextView();
  }

  // Just say yes if we are already in the closing process, or are embedded,
  // or have multiple views open
  //

  if (Closing || IsEmbedded())
    return true;

  return TDocument::CanClose();
}

//
/// Shuts down the TOleView's.
//
void
TOleDocument::OleViewClose()
{
  TView* curView = GetViewList();
  while (curView) {
    TOleView* oleView = TYPESAFE_DOWNCAST(curView, TOleView);
    if (oleView)
      oleView->OleShutDown();

    curView = curView->GetNextView();
  }
}

//
/// Ensures that the IStorage is released properly and disconnects any active server
/// in the document. A compound file must be closed before it is reopened.
//
bool
TOleDocument::Close()
{
  // Make sure that TOleView's are closed first
  //
  OleViewClose();
  OcDoc->Close();
  return TStorageDocument::Close();
}

//
/// Releases the ObjectComponents document when the server is finished using the
/// document.
//
bool
TOleDocument::ReleaseDoc()
{
  PRECONDITION(OcDoc);

  TStorageDocument::ReleaseDoc();
  OcDoc->SetStorage((IStorage*)0);

  return true;
}

//
/// Attaches the IStorage pointer (stg) to this document. If successful, SetStorage
/// returns true.
//
bool
TOleDocument::SetStorage(IStorage* stg, bool remember)
{
  PRECONDITION(OcDoc);

  // If a storage is provided, then we are now using container's IStorage
  //
  if (stg)
    Embedded = true;

  OcDoc->SetStorage(stg, remember);
  TStorageDocument::SetStorage(stg, remember);

  return true;
}

//
/// Restores the original root IStorage before the save operation.
//
bool
TOleDocument::RestoreStorage()
{
  PRECONDITION(OcDoc);

  OcDoc->RestoreStorage();
  TStorageDocument::RestoreStorage();

  return true;
}

//
/// Before the document is actually opened, PreOpen gives the derived class a chance
/// to perform a particular operation; for example, setting a different open mode
/// for the compound document.
//
void
TOleDocument::PreOpen()
{
  SetOpenMode(ofReadWrite | ofTransacted);
}

//
/// Overrides the TDocument::InitDoc function and creates or opens a compound file
/// so that there is an IStorage associated with this document's embedded objects.
/// Uses a TOcDocument object to perform the actual interaction with the OLE
/// IStorage  and IStream interfaces, which are ultimately responsible for
/// establishing the relationship between a compound file and its storage.
//
bool
TOleDocument::InitDoc()
{
  if (IsOpen())
    return true; // compound file already open

  // Give user a chance to set a different open mode
  //
  PreOpen();

  if (GetDocPath())
    SetOpenMode(GetOpenMode() | (ofNoCreate));
  else
    SetOpenMode(GetOpenMode() | ofTemporary);

  if (TStorageDocument::Open(GetOpenMode(), GetDocPath())) {
    if (OcDoc) { // use the existing ocdoc
      OcDoc->SetStorage(StorageI);
    }
    else if (GetOcApp()) {
      OcDoc = new TOcDocument(*GetOcApp(), GetDocPath(), StorageI);
    }

    return true;
  }

  return false;
}

//
/// Commits the current document's data to storage. If force is true and the data is
/// not dirty, all data is written to storage and Commit returns true. If force is
/// false, the data is written only if it is dirty.
//
bool
TOleDocument::Commit(bool force)
{
  if (Write())
    return TStorageDocument::Commit(force);
  else
    return false;
}

//
/// Loads the embedded objects, if any, using the path specified in path. mode is a
/// combination of bits that specify how the embedded objects are opened (for
/// example, read only, read/write, and so on). By default, objects are opened in
/// ofReadWrite and ofTransacted modes.
//
bool
TOleDocument::Open(int, LPCTSTR path)
{
  if (path)
    SetDocPath(path);

  return Read();
}
//
/// Checks to see if the current document's path is the same as the TOcDocument's
/// path. If the paths are not the same, PathChanged returns true.
//
bool TOleDocument::PathChanged()
{
  return _tcsicmp(OcDoc->GetName().c_str(), GetDocPath()) != 0;
}
//
/// Saves the embedded objects to the compound file. A container should call this
/// function to save its embedded objects to storage.
//
bool
TOleDocument::Write()
{
  // Switch to new storage if path has changed & it is permanent ("SaveAs")
  //
  IStorage* newStorageI;
  bool saveAs = PathChanged() && !OrgStorageI;      // also is 'remember'
  bool sameAsLoad = !PathChanged() && !OrgStorageI; // use current storage
  if (saveAs) {
    // Update link monikers
    //
    owl::tstring newName(GetDocPath());
    OcDoc->SetName(newName);

    if (IsEmbedded())
      newStorageI = StorageI; // Use the one assigned by container
    else
      newStorageI = GetNewStorage();
  }
  else
    newStorageI = StorageI;

  return newStorageI ?
    OcDoc->SaveParts(newStorageI, sameAsLoad, saveAs) :
    false;
}

//
/// Loads the embedded objects from the compound file. A container should call this
/// function to load any embedded objects.
//
bool
TOleDocument::Read()
{
  // Load the embedded objects, if any
  //
  return OcDoc->LoadParts();
}

//
/// Performs the reverse of Commit. Revert cancels any changes made to the document
/// since the last time the document was saved to storage.
//
bool
TOleDocument::Revert(bool clear)
{
  if (!StorageI)
    return true;                    // return OK if storage already released

  if (!TDocument::Revert(clear) || !ReleaseDoc())
    return false;

  if (!clear) {
    InitDoc();
    Open(0);
  }

  SetDirty(false);
  return true;
}

//
/// Returns the ObjectComponents application associated with this TOleDocument object.
//
TOcApp*
TOleDocument::GetOcApp()
{
  TOleFrame* olefr = TYPESAFE_DOWNCAST(GetDocManager().GetApplication()->GetMainWindow(), TOleFrame);

  return olefr->GetOcApp();
}



} // OCF namespace

//==============================================================================

