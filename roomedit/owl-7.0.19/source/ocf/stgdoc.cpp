//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
/// \file
/// Implementation of classes TStorageDocument, TDocFile, and associated streams
//
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <owl/docview.h>      // force Windows headers in before OLE
#include <owl/string.h>
#include <ocf/stgdoc.h>
#include <stdio.h>

#include <streambuf>

namespace ocf {

using namespace owl;

using namespace std;

OWL_DIAGINFO;

//
// Simple refcount debug assistant
//
#if defined(CHECK_REFCOUNT)
static void RefCountCheck(IStorage * si) {
  uint32 count = si->AddRef();
  count = si->Release();
}
#else
# define RefCountCheck(si)
#endif

const int B_size    = 516;                // default buffer size
const _TCHAR DefaultStreamName[] = _T("Contents");

//
// class TStorageBuf
// ~~~~~ ~~~~~~~~~~~
class  TStorageBuf : public streambuf {
    bool buffered_;
    char *base_;
    char *ebuf_;
  public:
    // Constructors, destructor
    //
    TStorageBuf  * open(IStorage& stg, LPCTSTR name, int omode);
    TStorageBuf();                       // make a closed TStorageBuf
    virtual ~TStorageBuf();

    int is_open() { return opened; }     // is the file open
    IStream* fd() { return strm; }

    TStorageBuf  * close();          // flush and close file
//  TStorageBuf  * attach(IStream*); // attach this TStorageBuf to opened IStream

    int_type eof(){
      return traits_type::eof();
    }

    virtual int_type overflow(int_type c = traits_type::eof());
    virtual int_type underflow();
    virtual int sync();
    virtual streambuf * setbuf(char_type *, streamsize);

    virtual pos_type seekoff(off_type off, ios_base::seekdir way,
                             ios_base::openmode which = ios_base::in |
                                                        ios_base::out);
    virtual pos_type seekpos(pos_type sp,
                             ios_base::openmode which =
                                       ios_base::in | ios_base::out);
    int out_waiting(){
      return pptr() ? (int)(pptr() - pbase()) : 0;
    }

    void setb(char *beg, char *end, int = 0){
      streamsize size_buf = end - beg;
      buffered_ = beg && size_buf;
      setbuf(beg, size_buf);
    }

    int unbuffered(){
      return !buffered_;
    }

    char *base(){
      return base_;
    }

    int blen(){
      return (int) (ebuf_ - base_);
    }

  protected:
//  IStorage* stg;       // parent storage
    IStream*  strm;
    int       mode;      // the opened mode
    short     opened;    // non-zero if stream is open
    uint64    last_seek;
    char      lahead[2]; // current input char if unbuffered
};

//
// class TStorageStreamBase
// ~~~~~ ~~~~~~~~~~~~~~~~~~
class TStorageStreamBase : virtual public ios {
  public:
    TStorageStreamBase(IStorage& stg, LPCTSTR name, int mode);
   ~TStorageStreamBase() {}
    void    setbuf(char  *, int);
    void    close();

    TStorageBuf buf;
};

//
// class TStorageInStream
// ~~~~~ ~~~~~~~~~~~~~~~~
class TStorageInStream : public TStorageStreamBase,
                                       public TInStream {
  public:
    TStorageInStream(TStorageDocument& doc,  LPCTSTR name, int mode)
                           : TInStream(doc,         name,     mode),
                   TStorageStreamBase(*doc.StorageI,name,     mode) {}
   ~TStorageInStream() {}
};

//
// class TStorageOutStream
// ~~~~~ ~~~~~~~~~~~~~~~~~
class TStorageOutStream : public TStorageStreamBase,
                                        public TOutStream {
  public:
    TStorageOutStream(TStorageDocument& doc,  LPCTSTR name, int mode)
                           : TOutStream(doc,         name,     mode),
                    TStorageStreamBase(*doc.StorageI,name,     mode) {}
   ~TStorageOutStream() {}
};

inline long IosToStgOpenMode(int omode)
{
  return ((omode & (ofRead|ofWrite)) - 1);
}

inline long IosToStgShareMode(int omode)
{
  return ((omode & shMask) - shCompat) >> 5;
}

inline long IosToStgTempMode(int omode)
{
  return (omode & ofTemporary) ? STGM_DELETEONRELEASE : 0;
}

inline long IosToStgMode(int omode)
{
  return IosToStgOpenMode(omode) |
         IosToStgShareMode(omode) |
         ((long)(omode & (ofTransacted|ofPreserve|ofPriority)) << 4);
}

const int stgRdWrMask  = (int)(STGM_READWRITE | STGM_READ | STGM_WRITE);
const int stgShareMask = (int)(STGM_SHARE_DENY_NONE | STGM_SHARE_DENY_READ |
                         STGM_SHARE_DENY_WRITE| STGM_SHARE_EXCLUSIVE);
const long stgModeMask  = (STGM_TRANSACTED | STGM_PRIORITY | STGM_CONVERT);

inline int StgToIosOpenMode(long grfMode)
{
  return (int)((grfMode & stgRdWrMask) + 1);
}

inline int StgToIosShareMode(long grfMode)
{
  return (int)(((grfMode & stgShareMask) << 5) + shCompat);
}

inline int StgToIosTempMode(long grfMode)
{
  return (grfMode & STGM_DELETEONRELEASE) ? ofTemporary : 0;
}

inline int StgToIosMode(long grfMode)
{
  return StgToIosOpenMode(grfMode) |
         StgToIosShareMode(grfMode) |
         (int)((grfMode & stgModeMask) >> 4);
}

//----------------------------------------------------------------------------
// class TStorageDocument
//

//
//
//
TStorageDocument::~TStorageDocument()
{
  ReleaseDoc();
  SetDirty(false); // ~TDocument() will Close() after destroying children
}

//
/// Release the IStorage and close the document
//
bool
TStorageDocument::ReleaseDoc()
{
  CanRelease = true; // now we can release the storage
  return Close();
}

//
/// Open the compound file with a given path
//
bool
TStorageDocument::Open(int omode, LPCTSTR name)
{
  if (StorageI)
    return true;

  do {  // establish try block
    HRESULT hr;
    IStorage* parentStg;
    int pmode = 0;

    if (!omode)
      omode = GetOpenMode();

    if (GetParentDoc()) {
      pmode = GetParentDoc()->GetOpenMode();
      if (!(GetParentDoc()->Open(pmode ? pmode : omode, name)))
        return false;
    }
    ++OpenCount;
    if (!omode)
      omode = pmode;
    if (!(omode & (ofRead | ofWrite)))
      break;

    // Provide default share mode flags & enforce root storage restrictions
    // when in direct mode
    //
    if (!(omode & shMask)) {
      if (!(omode & ofTransacted) && !GetParentDoc()) { // direct mode root stg
        if (omode & ofWrite)
          omode |= ofRead | shNone; // let others do nothing if we are writing
        else
          omode |= shRead;          // let others only read if we are readonly
      }
      else {
        if (omode & ofWrite)
          omode |= shRead;       // let others only read if we are writing
        else
          omode |= shReadWrite;  // let others read/write if we are readonly
      }
    }

    ThisOpen = omode;
    SetOpenMode(omode);  // remember the open mode
    if (name && name[0])
      SetDocPath(name);
    else
      name = GetDocPath();
    long grfMode = IosToStgMode(omode) | IosToStgTempMode(omode);
    long childMode = (grfMode & ~stgShareMask) | STGM_SHARE_EXCLUSIVE;

    bool stgopen = true;
    bool stgcreate = false;

    if (omode & ofWrite) {
      if (omode & (ofNoReplace)) {
        stgopen = false;
        stgcreate = true;
      }
      else if (!(omode & (ofNoCreate))) {
        stgcreate = true;
        if (omode & ofTruncate || !(omode & (ofAtEnd | ofAppend))) {
          stgopen = false;
          grfMode |= STGM_CREATE;
          childMode |= STGM_CREATE;
        }
      }
    }
    if (GetParentDoc()) {
      if (!GetParentDoc()->GetProperty(FindProperty(_T("IStorage Instance")),
                                       &parentStg, 0))
        break;
      if (stgopen) {
        hr = parentStg->OpenStorage(OleStr(name), 0, childMode, 0, 0, &StorageI);
        if (SUCCEEDED(hr))
          stgcreate = false;
      }
      if (stgcreate) {
        hr = parentStg->CreateStorage(OleStr(name), childMode, 0, 0, &StorageI);
      }
    }
    else {
      if (stgopen) {
        hr = ::StgOpenStorage(OleStr(name), 0, grfMode, 0, 0, &StorageI);
        if (SUCCEEDED(hr))
          stgcreate = false;
      }
      if (stgcreate) {
        hr = ::StgCreateDocfile(OleStr(name), grfMode, 0, &StorageI);
      }
    }
    RefCountCheck(StorageI);

    if (!SUCCEEDED(hr))
      break;
    NotifyViews(vnDocOpened,ThisOpen);
    return true;  // successful return
  } while (0);    // dummy for break scoping, never executed

  // exception handling
  //
  if (GetParentDoc())
    GetParentDoc()->Close();
  --OpenCount;

  return false;
}

//
/// Give an IStorage to document. This typically happens for OLE servers.
//
bool
TStorageDocument::SetStorage(IStorage* stg, bool remember)
{
  if (stg == StorageI)
    return true; // already set

  if (StorageI) {
    RefCountCheck(StorageI);
    if (remember) {
      StorageI->Release();  // Release the ole one
      OrgStorageI = 0;
    }
    else
      OrgStorageI = StorageI;
  }

  StorageI = stg;
  if (!StorageI)
    return true;  // done

  StorageI->AddRef();
  RefCountCheck(StorageI);

  STATSTG stgInfo;
  if (!SUCCEEDED(stg->Stat(&stgInfo, 0)))
    return false;

  SetOpenMode(StgToIosMode(stgInfo.grfMode) | StgToIosTempMode(stgInfo.grfMode));

  ThisOpen = GetOpenMode();
  if (remember)
  {
    if (stgInfo.pwcsName)
      SetDocPath(OleStr(stgInfo.pwcsName));
    else
      SetDocPath(_T("  "));
  }

  if (stgInfo.pwcsName) {
    IMalloc* memmgr;
    if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &memmgr))) {
      memmgr->Free(stgInfo.pwcsName);    // Free the string from the Stat call
      memmgr->Release();
    }
  }

//  NotifyViews(vnDocOpened,ThisOpen);
  return true;
}

//
/// Restore the original root IStorage before the save operation
//
bool
TStorageDocument::RestoreStorage()
{
  if (OrgStorageI) {
    if (StorageI)
      StorageI->Release();

    StorageI = OrgStorageI;
    OrgStorageI = 0;
  }

  return true;
}

//
/// Get a new IStorage, typically in a SaveAs situation. Release StorageI and
/// set it to the new storage if all OK.
//
IStorage*
TStorageDocument::GetNewStorage()
{
  PRECONDITION(StorageI);

  STATSTG stgInfo;
  if (!SUCCEEDED(StorageI->Stat(&stgInfo, STATFLAG_NONAME)))
    return 0;

  // Create another root storage based on the new doc path
  // Should close the current document and open a new one.
  //
  IStorage* newStorage = 0;

  HRESULT hr;
  hr = ::StgOpenStorage(OleStr(GetDocPath()), 0, stgInfo.grfMode, 0, 0, &newStorage);
  if (!SUCCEEDED(hr))
    hr = ::StgCreateDocfile(OleStr(GetDocPath()), stgInfo.grfMode, 0, &newStorage);

  if (SUCCEEDED(hr)) {
    StorageI->Release(); // Release the old root storage
    StorageI = newStorage;
  }

  return newStorage;
}

//
/// Create an IStorage based on a memory handle
//
bool
TStorageDocument::OpenHandle(int omode, HANDLE hGlobal)
{
  int pmode = 0;

  if (!omode)
    omode = GetOpenMode();

  if (!omode)
    omode = pmode;
  if (!(omode & (ofRead | ofWrite)))
    return false;
  if (!(omode & shMask))
    omode |= shNone;

  ThisOpen = omode;
  SetOpenMode(omode);  // remember the open mode
  long grfMode = IosToStgMode(omode) | STGM_CREATE;
  if (!SUCCEEDED(CreateILockBytesOnHGlobal(hGlobal, (omode & ofTemporary)!=0, &LockBytes)))
    return false;
  if (!SUCCEEDED(StgCreateDocfileOnILockBytes(LockBytes, grfMode, 0, &StorageI))) {
    LockBytes->Release();
    LockBytes = 0;
    return false;
  }
  ++OpenCount;
  NotifyViews(vnDocOpened,omode);
  return true;
}

//
/// Replace the IStorage with an istorage based on a memory handle
//
bool
TStorageDocument::SetHandle(int omode, HANDLE hGlobal, bool create, bool remember)
{
  int pmode = 0;

  if (!omode)
    omode = GetOpenMode();

  if (!omode)
    omode = pmode;
  if (!(omode & (ofRead | ofWrite)))
    return false;
  if (!(omode & shMask))
    omode |= shNone;

  ThisOpen = omode;
  SetOpenMode(omode);  // remember the open mode
  long grfMode = IosToStgMode(omode);
  if (!SUCCEEDED(CreateILockBytesOnHGlobal(hGlobal, (omode & ofTemporary)!=0, &LockBytes)))
    return false;

  IStorage* storageI = 0;
  if (!SUCCEEDED(StgOpenStorageOnILockBytes(LockBytes, 0, grfMode, 0, 0, &storageI))) {
    if (create && !SUCCEEDED(StgCreateDocfileOnILockBytes(LockBytes,
                             grfMode |STGM_CREATE, 0, &storageI))) {
      LockBytes->Release();
      LockBytes = 0;
      return false;
    }

    if (!storageI)
      return false;
  }

  RefCountCheck(storageI);

  // Don't change the embedded flag
  bool embed = IsEmbedded();
  SetStorage(storageI, remember);
  SetEmbedded(embed);

  storageI->Release(); // release extra refcount held by SetStorage
  return true;
}

//
/// Get the global handle from ILockBytes
//
bool
TStorageDocument::GetHandle(HGLOBAL* handle)
{
  if (!LockBytes)
    return false;

  GetHGlobalFromILockBytes(LockBytes, handle);
  return true;
}

//
//
//
bool
TStorageDocument::Close()
{
  if (!StorageI || !CanRelease)
    return true;

  if (StorageI)
    StorageI->Release();

  StorageI = 0;
  NotifyViews(vnDocClosed,ThisOpen);
  SetDirty(false);
  CanRelease = false;
  if (GetParentDoc())
    GetParentDoc()->Close();
  return true;
}

//
void
TStorageDocument::DetachStream(TStream& strm)
{
  TDocument::DetachStream(strm);
  TStorageDocument::Close();
}

//
//
//
TInStream*
TStorageDocument::InStream(int omode, LPCTSTR strmId)
{
  TInStream* inStream;
  if (omode == (ofParent))
    omode = IsOpen() ? ThisOpen : GetOpenMode();
  if (!(omode & ofRead))
    return 0;
  if (!TStorageDocument::Open(GetOpenMode() ? GetOpenMode() | ofRead : omode, strmId))
    return 0;

  inStream = new TStorageInStream(*this, strmId, omode);
  if (inStream && !inStream->good()) {
    delete inStream;
    TStorageDocument::Close();  // close
    return 0;
  }

  return inStream;
}

//
//
//
TOutStream*
TStorageDocument::OutStream(int omode, LPCTSTR strmId)
{
  TOutStream* outStream;
  if (omode == (ofParent))
    omode = IsOpen() ? ThisOpen : GetOpenMode();
  if (!(omode & ofWrite))
    return 0;
  if (!TStorageDocument::Open(GetOpenMode() ? GetOpenMode() | ofWrite : omode, strmId))
    return 0;
  outStream = new TStorageOutStream(*this, strmId, omode); // change
  if (outStream && !outStream->good()) {
    delete outStream;
    TStorageDocument::Close();  // change
    return 0;
  }
  SetDirty();     // we don't really know at this point if it will be dirty!!
  return outStream;
}

//
//
//
bool
TStorageDocument::SetDocPath(LPCTSTR path)
{
  TDocument::SetDocPath(path);
  return true;
}

//
//
//
bool
TStorageDocument::Commit(bool force)
{
  if (!TDocument::Commit(force))      // flush views and child docs
    return false;
  if (!StorageI)
    return true;                      // return OK if storage already released

  CommitTransactedStorage();
  SetDirty(false);
  return true;
}

//
//
//
bool
TStorageDocument::CommitTransactedStorage()
{
  HRESULT hr = StorageI->Commit(0);  // try 2phase commit first
  if (!SUCCEEDED(hr)) // check for STG_S_TRYOVERWRITE, but GetScode not in lib
    hr = StorageI->Commit(STGC_OVERWRITE);   // try less robust method
  if (!SUCCEEDED(hr))
    return false;

  return true;
}

//
//
//
bool
TStorageDocument::Revert(bool clear)
{
  if (!StorageI)
    return true;                    // return OK if storage already released
  if (!TDocument::Revert(clear) || !SUCCEEDED(StorageI->Revert()))
    return false;
  SetDirty(false);
  return true;
}

//
//
//
static LPCTSTR PropNames[] = {
  _T("Create Time"),      // CreateTime
  _T("Modify Time"),      // ModifyTime
  _T("Access Time"),      // AccessTime
  _T("Storage Size"),     // StorageSize
  _T("IStorage Instance"),// IStorageInstance
};

//
//
//
static int PropFlags[] = {
  pfGetBinary|pfGetText,   // CreateTime
  pfGetBinary|pfGetText,   // ModifyTime
  pfGetBinary|pfGetText,   // AccessTime
  pfGetBinary|pfGetText,   // StorageSize
  pfGetBinary,             // IStorage
};

//
//
//
LPCTSTR
TStorageDocument::PropertyName(int index)
{
  if (index <= PrevProperty)
    return TDocument::PropertyName(index);
  else if (index < NextProperty)
    return PropNames[index-PrevProperty-1];
  else
    return 0;
}

//
//
//
int
TStorageDocument::PropertyFlags(int index)
{
  if (index <= PrevProperty)
    return TDocument::PropertyFlags(index);
  else if (index < NextProperty)
    return PropFlags[index-PrevProperty-1];
  else
    return 0;
}

//
//
//
int
TStorageDocument::FindProperty(LPCTSTR name)
{
  for (int i = 0; i < NextProperty-PrevProperty-1; i++)
    if (_tcscmp(PropNames[i], name) == 0)
      return i+PrevProperty+1;
  return 0;
}

//
//
//
int
TStorageDocument::GetProperty(int prop, void * dest, int textlen)
{
  // Formats a file date + time string.
  //
  const auto formatFileTime = [](const FILETIME& pft, void* dest, int textlen) -> int
  {
    tchar buf[40];
    int len;

    if (!textlen)
    {
      *(FILETIME*) dest = pft;
      return sizeof(FILETIME);
    }
    SYSTEMTIME dt;
    FileTimeToSystemTime(&pft, &dt);
    len = _sntprintf(buf, COUNTOF(buf), _T("%d/%d/%d %02d:%02d:%02d.%02d"),
      dt.wMonth, dt.wDay, dt.wYear,
      dt.wHour, dt.wMinute, dt.wSecond, dt.wMilliseconds / 10);
    if (textlen > len)
      textlen = len;
    memcpy(dest, buf, textlen * sizeof(tchar));
    *((LPTSTR) dest + textlen) = _T('\0');
    return len;
  };

  STATSTG stgInfo;

  switch (prop) {
    case IStorageInstance:
      if (textlen)
        return 0;
      *(IStorage**)dest = StorageI;
      return sizeof(IStorage*);

    default:
      if (StorageI) {
        StorageI->Stat(&stgInfo, STATFLAG_NONAME);
        switch (prop) {
          case StorageSize:
            if (!textlen) {
              *(ulong *)dest = (uint32)(*(uint64*)&stgInfo.cbSize);
              return sizeof(ulong);
            }
            else {
              TCHAR buf[10];
              int len = _stprintf(buf, _T("%ld"), (uint32)(*(uint64*)&stgInfo.cbSize));
              if (textlen > len)
                textlen = len;
              memcpy(dest, buf, textlen);
              *((LPTSTR)dest + textlen) = 0;
              return len;
            }
          case AccessTime:
            return formatFileTime(stgInfo.atime, dest, textlen);
          case CreateTime:
            return formatFileTime(stgInfo.ctime, dest, textlen);
          case ModifyTime:
            return formatFileTime(stgInfo.mtime, dest, textlen);
        }
      }
      return TDocument::GetProperty(prop, dest, textlen);
  }
}

//
//
//
bool
TStorageDocument::SetProperty(int prop, const void * src)
{
  // docfile properties currently not settable
  //
  return TDocument::SetProperty(prop, src);
}

IMPLEMENT_STREAMABLE1(TStorageDocument, TDocument);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TStorageDocument::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  owl::ReadBaseObject((TDocument*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TStorageDocument::Streamer::Write(opstream& os) const
{
  owl::WriteBaseObject((TDocument*)GetObject(), os);
}

#endif

//----------------------------------------------------------------------------
//  class TStorageStreamBase
//

//
//
//
TStorageStreamBase::TStorageStreamBase(IStorage& stg, LPCTSTR name, int mode)
:
  buf()
{
  ios::init(&buf);

  if (buf.is_open())
    clear(ios::failbit);     // fail - already open
  else if (buf.open(stg, name, mode))
    clear(ios::goodbit);     // successful open
  else
    clear(ios::badbit);      // open failed
}

void TStorageStreamBase::setbuf(char* newbuf, int len)
{
  if (buf.setbuf(newbuf, len))
    clear(ios::goodbit);
  else
#if defined(BI_COMP_BORLANDC)
    setstate(ios::failbit);
#else
    clear(ios::failbit);
#endif
}

void TStorageStreamBase::close()
{
  if (buf.close())
    clear(ios::goodbit);
  else
#if defined(BI_COMP_BORLANDC)
    setstate(ios::failbit);
#else
    clear(ios::failbit);
#endif
}

//----------------------------------------------------------------------------
//  class TStorageBuf
//

//
// make a closed TStorageBuf
//
TStorageBuf::TStorageBuf()
:
  buffered_(false),
  base_(0),
  ebuf_(0)
{
  mode = 0;
  opened = 0;
  char* p = new char[B_size];
  if (p) {
    setb(p, p+B_size, 1);   // ~streambuf() will delete buffer
    setp(p+4, p+4);
    setg(p, p+4, p+4);
  }
}

//
/// We assume that mode= means that we attached to an already-open file,
/// and should not now close it.  We do flush it in any case.
//
TStorageBuf::~TStorageBuf()
{
  if (mode)
    close();
  else
    overflow(eof());
  delete[] base_;
}

//
/// Open or create IStream with mode and protection, attach to this TStorageBuf.
//
TStorageBuf* TStorageBuf::open(IStorage& stg, LPCTSTR name, int omode)
{
//int share = omode & shMask;
//if (share < shCompat)
//  share = shNone;
//how = (share-shCompat) >> 5;

  if (opened || !omode)
    return 0;

  if (!name)
    name = DefaultStreamName;

  bool  stgopen = true;
  bool  stgcreate = false;
  uint32 how = STGM_SHARE_EXCLUSIVE;  // must open streams and child stg exclusive

  if (omode & ofWrite) {
    if (!(mode & (ofAtEnd | ofAppend | ofRead)))
      omode |= ofTruncate;  // output implies truncate unless in, app, or ate
    if (omode & ofRead)
      how |= STGM_READWRITE;
    else
      how |= STGM_WRITE;
    if (omode & (ofNoReplace)) {
      stgopen = false;
      stgcreate = true;
    }
    else if (!(omode & (ofNoCreate))) {
      stgcreate = true;
      if (omode & ofTruncate) {
        stgopen = false;
        how |= STGM_CREATE;
      }
    }
  }
  else if (omode & ofRead)
    how |= STGM_READ;
  else
    return 0;   // must specfify in, out, or in/out

//if (omode & ofAppend)    // what does this mean for docfile?!!
//    how |= O_APPEND;

  // Now try to open or create
  //
  if (stgopen) {
    HRESULT hr = stg.OpenStream(OleStr(name), 0, how, 0, &strm);
    if (SUCCEEDED(hr))
      stgcreate = false;
    else
      return 0;
  }
  if (stgcreate) {
    HRESULT hr = stg.CreateStream(OleStr(name), how, 0, 0, &strm);
    if (!SUCCEEDED(hr))
      return 0;
  }

  // Finish up
  //
  opened = 1;
  mode = omode;
  LARGE_INTEGER uli = {0,0};
  if ((omode & ofAtEnd) != 0
      && !SUCCEEDED(strm->Seek(uli, STREAM_SEEK_END, (ULARGE_INTEGER*)&last_seek))) {
    strm->Release();
    strm = 0;
    return 0;
  }
  char* b = base();       // buffer address
  int pb = b ? ((blen() > 8) ? 4 : 1) : 0;    // putback area size
  setp(b+pb, b+pb);
  setg(b, b+pb, b+pb);

  return this;
}

#if 0
//
// attach this TStorageBuf to open IStream -- assume fd is actually open
//
TStorageBuf* TStorageBuf::attach(IStream* f)
{
  STATSTG stat;
  if (opened)
    return 0;

  if (f->Stat(&stat, STATFLAG_NONAME) != 0)
    return 0;

  if (f->CreateStream(STREAMNAME,stat.grfMode & ~STGM_TRANSACTED,0,0,&strm)!)
    return 0;

  stg = f;        // assumed to be valid
  opened = 1;
  int rwmode = stat.grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE);
  mode = rwmode != STGM_WRITE ? ofRead : 0;
  if (rwmode != STGM_READ)
    mode |= ofWrite;

  char* b = base();       // buffer address
  if (!b) {
    b = new char[B_size];
    if (b)
      setb(b, b+B_size, 1);   // ~streambuf() will delete buffer
  }
  int pb = b ? ((blen() > 8) ? 4 : 1) : 0;    // putback area size
  setp(b+pb, b+pb);
  setg(b, b+pb, b+pb);
  return this;
}

#endif

//
//
//
streambuf*
TStorageBuf::setbuf(char_type* b, streamsize len)
{
  if (opened && base())
    return 0;        // already open with a buffer -- no change

  int pb;            // putback area size
  if (b && len > 0)  // use b as the new buffer
    pb = (len > 8) ? 4 : 1; // guard against tiny buffers
  else {             // unbuffered
    len = pb = 0;
    b = 0;
  }
  delete[] base_;
  base_ = b;
  ebuf_ = b + len;
  setp(b+pb, b+pb);
  setg(b, b+pb, b+pb);
  return this;
}

//
/// Seek file to position.
/// We take a simple approach, and don't check for small position changes
/// within the current buffer.
//
TStorageBuf::pos_type TStorageBuf::seekoff(off_type off,
                         ios_base::seekdir dir, ios_base::openmode /* which */)
{
  int64 loff = static_cast<int64>(off);
  int count = out_waiting();

  if (count) {  // flush the output
    ulong actual;
    HRESULT hr = strm->Write(pbase(), (ulong)count, &actual);
    if (!SUCCEEDED(hr) || (int)actual != count)
      return eof();
  }
  else if (dir == ios::cur) {
    count = static_cast<int>(in_avail());
    if (count != 0) {
      loff -= count;

      // If we're in text mode, need to allow for newlines in the buffer
      //
      if ((mode & ofBinary) == 0) {
        char *tptr = gptr();
        while (tptr != egptr())
          if (*tptr++ == _T('\n'))
            loff--;
      }
    }
  }

  uint32 w = (dir == ios::beg) ? STREAM_SEEK_SET
       : ((dir == ios::cur) ? STREAM_SEEK_CUR
       :       /* ios::end */ STREAM_SEEK_END);
  int64 i64pos = loff;
  if (!SUCCEEDED(strm->Seek(*(LARGE_INTEGER*)&i64pos, w, (ULARGE_INTEGER*)&last_seek)))
    return eof(); //?!!
  if (!unbuffered() && base()) {      // set up get and put areas
    int pb = (blen() > 8) ? 4 : 1;  // putback area size
    char *b = base();
    setp(b+pb, b+pb);
    setg(b, b+pb, b+pb);
  }
  return (uint32)last_seek;
}

TStorageBuf::pos_type TStorageBuf::seekpos(pos_type sp,
                 ios_base::openmode which){
  return seekoff(streamoff(sp), ios::beg, which);
}

//
//
//
int TStorageBuf::sync()
{
  if (!opened)
    return eof();

  ulong actual;
  int count = out_waiting();
  if (count) {
    char* curp;
    char* srcp = pbase();
    char* endp = srcp + count;

    // Convert LF's to CR/LF if text mode
    //
    if ((mode & ofBinary) == 0) {
      for (curp = srcp; curp < endp; curp++) {
        if (*curp == _T('\n')) {
          *curp = _T('\r');
          count = (int)(curp - srcp + 1);
          HRESULT hr = strm->Write(srcp, (unsigned long)count, &actual);
          if (!SUCCEEDED(hr) || (int)actual != count)
            return eof();
          *(srcp = curp) = _T('\n');
        }
      }
      count = (int)(curp - srcp);  // write what remains in the buffer below
    }
    HRESULT hr = strm->Write(srcp, (ulong)count, &actual);
    if (!SUCCEEDED(hr) || (int)actual != count)
        return eof();

    // Reset get and put areas
    //
    int pb = (blen() > 8) ? 4 : 1;  // putback area size
    char *b = base();
    setp(b+pb, b+blen());
    setg(b, b+pb, b+pb);

  }
  else if (in_avail()) {
     int64 i64 = int64(long(-in_avail()));
    if (!SUCCEEDED(strm->Seek(*(LARGE_INTEGER*)&i64, STREAM_SEEK_CUR, (ULARGE_INTEGER*)&last_seek)))
      return eof();
    setg(eback(), gptr(), gptr());
    setp(gptr(), gptr());
  }
  return 0;
}

//
//
//
TStorageBuf::int_type TStorageBuf::underflow()
{
  ulong  actual;
  uint   count;  // input character count
  int    c;      // the return value

  if (!opened || (mode & (ofRead | ofWrite)) == ofWrite)
    return eof();

  if (in_avail())                   // no action needed
    return (utchar)*gptr();

  if (!unbuffered() && base()) {    // this is buffered
    if (sync() != 0)
      return eof();

    // find buffer data
    //
    int pb = (blen() > 8) ? 4 : 1;  // putback area size
    char* begp = base() + pb;

    // read in a new buffer
    //
    HRESULT hr = strm->Read(begp, blen()-pb, &actual);
    if (!SUCCEEDED(hr))
      return eof();
    count = (unsigned)actual;

    // remove CR's if text mode
    //
    if ((mode & ofBinary) == 0) {
      char* endp = begp + count;
      char* dstp = 0;
      char* srcp = 0;
      char* curp;

      for (curp = begp;  curp < endp; curp++) {
        if (*curp == _T('\r')) {
          if (dstp) {
            memcpy(dstp, srcp, (int)(curp - srcp));
            dstp += (int)(curp - srcp);
          }
          else
            dstp = curp;
          srcp = curp + 1;
        }
      }
      if (dstp) {
        endp = dstp + (int)(curp - srcp);
        if (curp != srcp)
          memcpy(dstp, srcp, (int)(curp - srcp));
      }
      count = (int)(endp - begp);
    }

    // set up get and put areas
    //
    setg(base(), begp, begp + count);
    setp(begp, begp);

    if (count)
      c = (utchar)*gptr();

  }
  else {     // this is not buffered
    for (;;) {
      HRESULT hr = strm->Read(lahead, 1, &actual);
      if (!SUCCEEDED(hr) || actual == 0) {
        c = eof();
        setg(0, 0, 0);
      }
      else {
        c = (utchar)lahead[0];
        if ((mode & ofBinary) == 0  && c == _T('\r'))
          continue;
        setg(lahead, lahead, lahead+1);
      }
      break;
    }
  }
  if (!count)
    c = eof();    // end of file
  return c;
}

//
/// always flush
//
TStorageBuf::int_type TStorageBuf::overflow(int_type c)
{
  if (!opened || (mode & (ofRead | ofWrite)) == ofRead)
    return eof();

  if (unbuffered() || !base()) {
    if (c != eof()) {
      int count;
      char b[2];
      if (c == _T('\n') && (mode & ofBinary) == 0) {
        b[0] = _T('\r');
        b[1] = (char)c;
        count = 2;
      }
      else {
        b[0] = (char)c;
        count = 1;
      }
      ulong actual = 0;
      strm->Write(&c, (ulong)count, &actual);
      if ((int)actual != count)
        return eof();
    }
  }
  else {    // now we know this is buffered and state is not bad

    // resets get and put areas
    //
    if (sync() != 0)
      return eof();

    // reset get and put areas
    //
    int pb = (blen() > 8) ? 4 : 1;  // putback area size
    char *b = base();
    setp(b+pb, b+blen());
    setg(b, b+pb, b+pb);

    if (c != eof()) {
      sputc(c);
      gbump(1);       // pptr and gptr must be the same
    }
  }
  return 1;
}

//
/// flush and close file
//
TStorageBuf* TStorageBuf::close()
{
  if (!opened)
    return 0;         // nothing to do

  int ores = 0;       // result of overflow()
  if (out_waiting() && overflow(eof()) == eof())
    ores = 1;
  if ((mode & ofWrite) /* && !strm->Commit(0) */)
    ores = 1;

  strm->Release();    /// Question: should we release if commit fails?!!
  strm = 0;
  opened = 0;
  return ores ? 0 : this;
}

} // OCF namespace

//==============================================================================


