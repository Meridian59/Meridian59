//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TOcDocument Class
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/oleutil.h>
#include <ocf/ocstorag.h>
#include <ocf/ocdoc.h>
#include <ocf/ocpart.h>
#include <ocf/ocapp.h>
#include <ocf/ocremvie.h>
#include <owl/defs.h>

namespace ocf {

using namespace owl;

DIAG_DECLARE_GROUP(OcfRefCount);

namespace
{
  constexpr _TCHAR  DocStreamName_[] = _T("OcDocument");

  // There are (or used to be) some API that do not allow longer names.
  //
  constexpr auto MaxPartNameSize_ = 32;
} // anonymous namespace

//
//
//
TOcDocument::TOcDocument(TOcApp& app, LPCTSTR fileName)
:
  OcApp(app),
  Name(fileName?fileName:_T("")),
  PartCollection(),
  LinkCollection()
{
  // Host support...
  //
  ActiveView = 0;
  PartID     = 0;
  Storage    = 0;

  bool createNew = ToBool(!fileName || !fileName[0]);
  Storage = new TOcStorage(fileName, createNew);
  TRACEX(OcfRefCount, 1, "TOcDocument() @" << (void*)this);
}

//
//
//
TOcDocument::TOcDocument(TOcApp& app, LPCTSTR fileName, IStorage * storageI)
:
  OcApp(app),
  Name(fileName?fileName:_T("")),
  PartCollection(),
  LinkCollection()
{
  // Host support...
  //
  ActiveView = 0;
  PartID     = 0;
  Storage    = 0;
  OrgStorage = 0;

  if (storageI)   // Storage already created by host application
    Storage = new TOcStorage(storageI);
  else
    Storage = 0;  // No storage yet, wait until a SetStorage() is called
  TRACEX(OcfRefCount, 1, "TOcDocument() @" << (void*)this);
}

//
//
//
TOcDocument::~TOcDocument()
{
  delete Storage;
}

//----------------------------------------------------------------------------

//
/// Set the storage for this document
//
void
TOcDocument::SetStorage(IStorage* storage, bool remember)
{
  if (Storage && storage == Storage->GetIStorage())
    return;

  if (remember) {
    delete Storage;
    OrgStorage = 0;
  }
  else
    OrgStorage = Storage;

  if (storage)
    Storage = new TOcStorage(storage);
  else
    Storage = 0;
}

//
/// Set the storage for this document
//
void
TOcDocument::SetStorage(LPCTSTR path)
{
  delete Storage;
  Storage = new TOcStorage(path, false);
  SetName(path);    // new Storage now has path as a name
}

//
/// Restore the original root IStorage before the save operation
//
bool
TOcDocument::RestoreStorage()
{
  if (OrgStorage) {
    delete Storage;

    Storage    = OrgStorage;
    OrgStorage = 0;
  }

  return true;
}

//
/// Perform saveas operation
//
bool
TOcDocument::SaveToFile(LPCTSTR newName)
{
  PRECONDITION(newName);

  TOcStorage newStorage(newName, true);
  Name = newName;
  return SaveParts(newStorage.GetIStorage(), false);
}

//
/// Save the embedded parts to the provided storage. 'remember' flag is not
/// really used here, as our save/restore is done outside here.
//
bool
TOcDocument::SaveParts(IStorage* storage, bool sameAsLoaded, bool remember)
{
  if (!Storage)
    return true;

  TOcStorage* oldStorage = 0;

  // If the storage passed is not the same as the one we loaded, wrap the new
  // one & use it in here.
  //
  if (!sameAsLoaded) {
    CHECK(storage);

    // If we are switching storages, make sure parts all all pulled in now
    //
    for (TOcPartCollectionIter i(PartCollection); i; i++) {
      TOcPart& part = *i.Current();
      part.FinishLoading();
    }
    oldStorage = Storage;
    Storage = new TOcStorage(storage);
  }

  // Create a stream for part information
  //
  STATSTG statstg;
  if (!HRSucceeded(Storage->Stat(&statstg, STATFLAG_NONAME)))
    return false;

  TOcStream  stream(*Storage, DocStreamName_, true, statstg.grfMode);

  // Write TOcDocument data into stream
  //
  ulong count;
  bool  ok;
  ulong value = PartCollection.Count();
  ok = ToBool(HRSucceeded(stream.Write(&PartID, sizeof PartID, &count)) &&
              HRSucceeded(stream.Write(&value, sizeof value, &count)));

  if (ok) {
    for (TOcPartCollectionIter i(PartCollection); i; i++) {
      TOcPart& part = *i.Current();
      int16 len = int16(part.GetName().Length());

      // Write the part name string, pascal style [len]+chars, no 0
      //
      ok = ToBool(HRSucceeded(stream.Write(&len, sizeof len, &count)) &&
                  HRSucceeded(stream.Write(static_cast<const char*>(part.GetName()), len, &count)) &&
                  part.Save(sameAsLoaded, remember));

      if (!ok)
        break;
    }
  }

  // Deal with the alloc'd storage if there was one. Either put things back on
  // failure, or keep the new storage around for a while
  //
  if (!sameAsLoaded) {
    if (!ok) {
      delete Storage;
      Storage = oldStorage;
    }
    else
      delete oldStorage;
  }

  return ok;
}

//
/// Loads the parts from the current storage into the PartCollection. Return
/// false if a serious error occurs. Having no part stream at all is OK.
//
bool
TOcDocument::LoadParts()
{
  if (!Storage)
    return true;

  /// Clear the part collection
  ///
  /// PartCollection.Clear(); ///DR clear before make a new one.
                              /// There is a case when clearing now causes a crash

  // Open a stream with part information
  //
  STATSTG statstg;
  if (!HRSucceeded(Storage->Stat(&statstg, STATFLAG_NONAME)))
    return false;

  try {
    TOcStream  stream(*Storage, DocStreamName_, false, statstg.grfMode);

    // Read TOcDocument data from stream. Return false if any of the data
    // is missing--something must have been corrupted.
    //
    if (!HRSucceeded(stream.Read(&PartID, sizeof PartID)))
      return false;

    ulong value;
    if (!HRSucceeded(stream.Read(&value, sizeof value)))
      return false;

    // Clear the part collection
    //
    PartCollection.Clear();

    // Rebuild the part collection
    //
    for (int i = (int)value; i; i--) {
      char name[MaxPartNameSize_ + 1];
      int16 len;

      // Read the part name string, pascal style [len]+chars, no 0
      //
      if (!HRSucceeded(stream.Read(&len, sizeof(len))))
        return false;

      if (len < 0 || len > MaxPartNameSize_) {
        // NOTE: Should *never* happen!
        //
        return false;
      }

      if (!HRSucceeded(stream.Read(name, len)))
        return false;
      name[len] = 0;  // 0 was not written

      // !BB Something's wrong here for controls -- can't just create
      // !BB a TOcPart for OCXes..
      // !BB
      new TOcPart(*this, to_tstring(const_cast<LPCSTR>(name)).c_str());
    }
  }
  catch (TXObjComp&) {
    // There is no part related info stream. Thats OK, we then have a document
    // with no parts
  }
  return true;
}

//
//
//
void
TOcDocument::RenameParts(IBRootLinkable * bLDocumentI)
{
  if (Name.empty())
    return; // Temporary file does not have moniker

  for (TOcPartCollectionIter i(PartCollection); i; i++) {
    TOcPart& part = *i.Current();

    IBLinkable * linkI;
    if (HRSucceeded(part.QueryInterface(IID_IBLinkable, (LPVOID *)&linkI))) {
      linkI->OnRename(bLDocumentI, part.GetName());
      linkI->Release();
    }
  }
}

//
//
//
void
TOcDocument::Close()
{
  for (TOcPartCollectionIter i(PartCollection); i; i++) {
    TOcPart& part = *i.Current();
    part.Close();
  }

  for (TOcLinkCollectionIter j((TPtrArray<TOcLinkView*>&)LinkCollection); j; j++) {
    TOcLinkView& link = *j.Current();
    link.Disconnect();
  }

  LinkCollection.Clear();  // Remove the link views now
}

//
//
//
void
TOcDocument::SetActiveView(TOcView* view)
{
  ActiveView = view;
}

//
/// Notify container that doc pathname has changed
//
void
TOcDocument::SetName(const owl::tstring& newName)
{
  Name = newName;

  // Create a moniker for this document
  //
  if (!Name.empty() && ActiveView)
    ActiveView->Rename();
}

} // OCF namespace

//==============================================================================


