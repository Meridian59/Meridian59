//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OCF_STGDOC_H)
#define OCF_STGDOC_H

#include <owl/private/defs.h>
#include <ocf/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/docview.h>

//
// Forward reference OLE interface without including headers
//
  interface IStorage;
  interface IStream;


namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

//
// class TStorageDocument
// ~~~~~ ~~~~~~~~~~~~~~~~
class _OCFCLASS TStorageDocument : public owl::TDocument {
  public:
    enum TStgDocProp {
      PrevProperty = owl::TDocument::NextProperty-1,
      CreateTime,        // FILETIME
      ModifyTime,        // FILETIME
      AccessTime,        // FILETIME
      StorageSize,       // owl::ulong
      IStorageInstance,  // IStorage*
      NextProperty,
    };
    TStorageDocument(owl::TDocument* parent = 0);
   ~TStorageDocument();
    virtual bool  ReleaseDoc();

    // Implement virtual methods of owl::TDocument
    //
    owl::TInStream*    InStream(int omode, LPCTSTR strmId=0);
    owl::TOutStream*    OutStream(int omode, LPCTSTR strmId=0);
    bool          Open(int omode, LPCTSTR stgId);
    bool          Close();
    bool          Commit(bool force = false);
    bool          CommitTransactedStorage();
    bool          Revert(bool clear = false);
    bool          SetDocPath(LPCTSTR path);
    bool          IsOpen();

    int           FindProperty(LPCTSTR name);  // return index
    int           PropertyFlags(int index);
    LPCTSTR        PropertyName(int index);
    int           PropertyCount();
    int           GetProperty(int index, void * dest, int textlen=0);
    bool          SetProperty(int index, const void * src);

    // Additional methods for obtaining or changing the IStorage
    //
    virtual bool  SetStorage(IStorage* stg, bool remember = true);  // Set a new IStorage
    virtual bool  RestoreStorage();
    virtual IStorage* GetNewStorage();
    IStorage*     GetStorage();

    virtual bool  OpenHandle(int omode, HANDLE hGlobal); // open on global memory
    virtual bool  SetHandle(int omode, HANDLE hGlobal, bool create = false, bool remember = false);
    virtual bool  GetHandle(HGLOBAL* handle);

  protected:
    int           GetThisOpen();
    IStorage*     GetOrgStorageI();
    ILockBytes*   GetLockBytes();

  protected_data:
    int           ThisOpen;    ///< Actual mode bits used for opening storage
    IStorage*     StorageI;    ///< Current IStorage instance, 0 if not open
    IStorage*     OrgStorageI; ///< Pointer to original IStorage interface
    ILockBytes*   LockBytes;   ///< Pointer to ILockBytes used, if any

  private:
    bool          CanRelease;  ///< Can we release the IStorage?
    int           OpenCount;

    void          DetachStream(owl::TStream& strm); // Override owl::TDocument virtual

  DECLARE_STREAMABLE_OCF(TStorageDocument,1);
  friend class TStorageInStream;
  friend class TStorageOutStream;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

// --------------------------------------------------------------------------
// Inline implementations

//
/// Construct a default Storage document object
//
inline TStorageDocument::TStorageDocument(owl::TDocument* parent)
:
  owl::TDocument(parent), StorageI(0), OpenCount(0), CanRelease(false),
  OrgStorageI(0), LockBytes(0)
{
}

//
/// Return 'true' if the storage document object has opened an OLE storage.
/// Return 'false' otherwise.
//
inline bool TStorageDocument::IsOpen() {
  return (StorageI != 0);
}

//
/// Return the number of properties supported by the storage document object.
/// \note The number includes the inherited properties of the storage
//       document object.
//
inline int TStorageDocument::PropertyCount() {
  return NextProperty - 1;
}

//
/// Return the IStorage interface pointer currently associated with the
/// storage document object. Returns 0 if no storage is currently opened.
//
inline IStorage* TStorageDocument::GetStorage() {
  return StorageI;
}

//
/// Return the mode bits used to open the storage currently associated with
/// this storage object.
//
inline int TStorageDocument::GetThisOpen() {
  return ThisOpen;
}

//
/// Return a pointer to the original IStorage interface associated with this
/// storage document object.
//
inline IStorage* TStorageDocument::GetOrgStorageI() {
  return OrgStorageI;
}

//
/// Return a pointer to the ILockBytes interface currently being used by this
/// storage document object. Return 0 if no ILockBytes interface is in use.
//
inline ILockBytes* TStorageDocument::GetLockBytes() {
  return LockBytes;
}

} // OCF namespace


#endif  // OWL_STGDOC_H
