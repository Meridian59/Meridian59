//----------------------------------------------------------------------------
// ObjectComponents
// Copyright 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of TOcStorage & TOcStream classes
//----------------------------------------------------------------------------

#if !defined(OCF_OCSTORAG_H)
#define OCF_OCSTORAG_H

#include <owl/private/defs.h>
#include <owl/private/number.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/defs.h>
#include <ocf/ocbocole.h>


//
// Classes referenced
//
interface _ICLASS IRootStorage;
interface _ICLASS IStorage;
interface _ICLASS IStream;
interface _ICLASS ILockBytes;
interface _ICLASS IEnumSTATSTG;

namespace ocf {

//
// Classes defined
//
class _ICLASS TOcStream;
class _ICLASS TOcStorage;

//
// class TOcStream
// ~~~~~ ~~~~~~~~~
class _OCFCLASS TOcStream {
  public:
    TOcStream(TOcStorage& storage, LPCTSTR name, bool create,
              owl::uint32 mode = STGM_READWRITE);
    TOcStream(TOcStream& stream);
    TOcStream(IStream* stream);
   ~TOcStream();

    IStream* GetIStream();

    HRESULT Read(void * pv, owl::ulong cb, owl::ulong * read = 0);
    HRESULT Write(void const * pv, owl::ulong cb, owl::ulong * written = 0);
    HRESULT Seek(owl::int64 move, owl::uint32 origin= STREAM_SEEK_SET,
                 owl::uint64 * newPosition = 0);
    HRESULT SetSize(owl::uint64 newSize);
    HRESULT CopyTo(TOcStream& stream, owl::uint64 cb, owl::uint64 * read = 0,
                   owl::uint64 * written = 0);
    HRESULT Commit(owl::uint32 commitFlags);
    HRESULT Revert();
    HRESULT LockRegion(owl::uint64 offset, owl::uint64 cb, owl::uint32 lockType);
    HRESULT UnlockRegion(owl::uint64 offset, owl::uint64 cb, owl::uint32 lockType);
    HRESULT Stat(STATSTG * statstg, owl::uint32 statFlag);

  protected:
    HRESULT Clone(IStream * * ppstm);

    IStream* StreamI;
};

//
// class TOcStorage
// ~~~~~ ~~~~~~~~~~
class _OCFCLASS TOcStorage {
  public:
    TOcStorage(LPCTSTR fileName, bool create,
               owl::uint32 mode = STGM_READWRITE|STGM_TRANSACTED);
    TOcStorage(ILockBytes * lkbyt, bool create,
               owl::uint32 mode = STGM_READWRITE|STGM_TRANSACTED);
    TOcStorage(TOcStorage& parent, LPCTSTR name, bool create,
               owl::uint32 mode = STGM_READWRITE);
    TOcStorage(IStorage* storage);
   ~TOcStorage();

    IStorage* GetIStorage();

    HRESULT CopyTo(owl::uint32 ciidExclude, IID const * rgiidExclude,
                   SNB snbExclude, TOcStorage& dest);
    HRESULT MoveElementTo(LPCTSTR name, TOcStorage& dest,
                          LPCTSTR newName, owl::uint32 grfFlags);
    HRESULT Commit(owl::uint32 grfCommitFlags);
    HRESULT Revert();
    HRESULT EnumElements(owl::uint32 reserved1, void * reserved2,
                         owl::uint32 reserved3, IEnumSTATSTG ** ppenm);
    HRESULT DestroyElement(LPCTSTR name);
    HRESULT RenameElement(LPCTSTR oldName, LPCTSTR newName);
    HRESULT SetElementTimes(LPCTSTR name, FILETIME const * pctime,
                            FILETIME const * patime,
                            FILETIME const * pmtime);
    HRESULT SetClass(const IID & clsid);
    HRESULT SetStateBits(owl::uint32 grfStateBits, owl::uint32 grfMask);
    HRESULT Stat(STATSTG  *pstatstg, owl::uint32 grfStatFlag);

    HRESULT SwitchToFile(LPCTSTR newPath);

    static HRESULT IsStorageFile(LPCTSTR pwcsName);
    static HRESULT IsStorageILockBytes(ILockBytes * plkbyt);
    static HRESULT SetTimes(LPCTSTR lpszName,
                            FILETIME const * pctime,
                            FILETIME const * patime,
                            FILETIME const * pmtime);

  protected:
    HRESULT CreateStream(LPCTSTR name, owl::uint32 mode, owl::uint32 rsrvd1,
                         owl::uint32 rsrvd2, IStream * * stream);
    HRESULT OpenStream(LPCTSTR name, void  *rsrvd1, owl::uint32 grfMode,
                       owl::uint32 rsrvd2, IStream  * *stream);
    HRESULT CreateStorage(LPCTSTR name, owl::uint32 mode, owl::uint32 rsrvd1,
                          owl::uint32 rsrvd2, IStorage ** storage);
    HRESULT OpenStorage(LPCTSTR name, IStorage * stgPriority,
                        owl::uint32 mode, SNB snbExclude, owl::uint32 rsrvd,
                        IStorage ** storage);
    owl::ulong   AddRef();
    owl::ulong   Release();

#if 0  // not currently implemented
//    TOcStorage*    GetParent() const {return Parent;}
//    int            GetOpenMode() const {return OpenMode;}
//    void           SetOpenMode(int mode) const {OpenMode = mode;}

  protected:
//    int         ThisOpen;     // actual mode bits used for opening storage

  private:
//    int         OpenMode;     // mode and protection flags
//    int         OpenCount;
//    TOcStorage* Parent;
#endif
    IStorage*   StorageI;

  friend TOcStream;
};

} // OCF namespace

#endif  // OCF_OCSTORAG_H
