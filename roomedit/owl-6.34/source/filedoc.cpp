//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TFileDocument, TFileInStream, TFileOutStream,
/// TFileBuf
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/filedoc.h>
#include <stdio.h>

#include <owl/private/strmdefs.h>

using namespace std;

namespace owl {

OWL_DIAGINFO;


//
// class TFileBuf
// ~~~~~ ~~~~~~~~
class _OWLCLASS TFileBuf : public tstreambuf {
    bool buffered_;
    tchar *base_;
    tchar *ebuf_;
  public:
    enum { shDefault = shReadWrite };  // default file sharing

   ~TFileBuf() {
      if (base_) delete[] base_;
   }
   int_type eof(){
     return traits_type::eof();
   }
    TFileBuf(int fhdl, int omode, streampos seekpos);


    virtual int_type overflow(int_type c = tstreambuf::traits_type::eof());
    virtual int_type underflow();

    virtual tstreambuf* setbuf(char_type *s, streamsize n);

    virtual pos_type seekoff(off_type,
                             ios_base::seekdir, ios_base::openmode which =
                             ios_base::in | ios_base::out);

    virtual pos_type seekpos(pos_type sp,
                             ios_base::openmode which =
                                       ios_base::in | ios_base::out);

    virtual int sync();

    int out_waiting(){
      return pptr() ? (int)(pptr() - pbase()) : 0;
    }

    void setb(tchar* beg, tchar* end, int = 0){
      streamsize size_buf = end - beg;
      buffered_ = beg && size_buf;
      setbuf(beg, size_buf);
    }

    int unbuffered(){
      return !buffered_;
    }

    void unbuffered(int buf){
      if (buf){
        buffered_ = false;
        setb(0, 0, 0);
      }
    }

    tchar *base(){
      return base_;
    }

    int blen(){
      return (int) (ebuf_ - base_);
    }

    int       xfd;       // the file descriptor, EOF if closed
    int       mode;      // the opened mode
    streampos last_seek;
    tchar    lahead[2]; // current input char if unbuffered
};

//
// class TFileStreamBase
// ~~~~~ ~~~~~~~~~~~~~~~
//
class _OWLCLASS TFileStreamBase : virtual public tios {
  public:
    TFileStreamBase(int fhdl, int omode, streampos seekpos);
   ~TFileStreamBase() {}
    TFileBuf buf;
};

//
// class TFileInStream
// ~~~~~ ~~~~~~~~~~~~~
//JJH gcc-3.1 doesn't like original inheritance order - it refuses to
//    correctly fill rdbuf variable - effect is that you cannot save
//    anything... (all this just becaus of MI!).
#if (__GNUC__ >= 3)
class _OWLCLASS TFileInStream : public TInStream, public TFileStreamBase {
#else
class _OWLCLASS TFileInStream : public TFileStreamBase, public TInStream {
#endif
  public:
    TFileInStream(TFileDocument& doc, HFILE fhdl, int omode, streampos seekpos)
               : TInStream (doc,0,omode), TFileStreamBase(fhdl,omode,seekpos){}
   ~TFileInStream();
};

//
// class TFileOutStream
// ~~~~~ ~~~~~~~~~~~~~~
//JJH the same as above, you have to change inheritance order in order
//    TFileOutStream to work correctly...
#if (__GNUC__ >= 3)
class _OWLCLASS TFileOutStream : public TOutStream, public TFileStreamBase {
#else
class _OWLCLASS TFileOutStream : public TFileStreamBase, public TOutStream {
#endif
  public:
    TFileOutStream(TFileDocument& doc, HFILE fhdl, int omode, streampos seekpos)
              : TOutStream (doc,0,omode), TFileStreamBase(fhdl,omode,seekpos){}
   ~TFileOutStream();
};


//------------------------------------------------------------------------------

//
/// Opens the file document after checking the file sharing mode (omode).  If a file
/// mode is not specified as read, write, or read and write, OpenThisFile returns 0.
//
/// \todo This method uses obsolete WinApi functions for 16-bit compatibility
/// which is no longer supported
HFILE
TFileDocument::OpenThisFile(int omode, const tstring& fileName, streampos* pseekpos)
{
  int how;
  HFILE fhdl;
  bool exists = false;
  int share = omode & shMask;
  _USES_CONVERSION;

  if (share < shCompat)
    share = (omode & ofWrite) ? shRead : shReadWrite;

  how = ((unsigned)(share-shCompat)) >> 5;

  if (omode & ofWrite) {
    if (!(omode & (ofAtEnd | ofAppend | ofRead)))
      omode |= ofTruncate; // output implies truncate unless in, app, or ate

    if (omode & ofRead)
      how |= OF_READWRITE;
    else
      how |= OF_WRITE;
    if (!((omode & ofNoCreate) && !(omode & ofTruncate))) {
      if (!((omode & ofTruncate) && !(omode & (ofNoCreate | ofNoReplace)))) {
        if ((fhdl=::_lopen(_W2A(fileName.c_str()),OF_READ|OF_SHARE_COMPAT))   !=HFILE_ERROR
        ||  (fhdl=::_lopen(_W2A(fileName.c_str()),OF_READ|OF_SHARE_DENY_NONE))!=HFILE_ERROR) {
          ::_lclose(fhdl);
          exists = true;
          }
        }
      if (!(exists && !(omode & (ofNoCreate | ofNoReplace)))) {
        if ((exists && (omode & ofNoReplace))
        || (!exists && (omode & ofNoCreate)) )
//            return 0; // ? Jogy - this function should return either valid handle or HFILE_ERROR
            return HFILE_ERROR;
        if ((fhdl = _lcreat(_W2A(fileName.c_str()), 0)) == HFILE_ERROR)
            return HFILE_ERROR;
        ::_lclose(fhdl);   // close in order to open with share mode
        }
      }
    }
  else if (omode & ofRead)
    how |= OF_READ;
  else
    return HFILE_ERROR;   // must specfify in, out, or in/out

  if ((fhdl = ::_lopen(_W2A(fileName.c_str()), how)) != HFILE_ERROR) {
    if ((*pseekpos = ::_llseek(fhdl, 0, (omode & (ofAtEnd|ofAppend))
                             ? SEEK_END : SEEK_SET)) == streampos(HFILE_ERROR)) {
      ::_lclose(fhdl);
      return HFILE_ERROR;
    }
  }
  FileLength = GetFileSize((HANDLE)fhdl, 0);
  InfoPresent = GetFileTime((HANDLE)fhdl, &FileCreateTime, &FileAccessTime,
                            &FileUpdateTime);

  NotifyViews(vnDocOpened,omode);
  return fhdl;
}


//
/// Closes the file handle if the associated file was opened by TFileDocument. Calls
/// TDocument::NotifyViews() to notify all views that the file document has closed.
//
void
TFileDocument::CloseThisFile(HFILE fhdl, int omode)
{
  if (!IS_PREV_OPEN(omode)) {
    ::_lclose(fhdl);
    NotifyViews(vnDocClosed, omode);
  }
}

//
/// Overrides TDocument::Open and opens the file using the specified path. If the
/// file is already open, returns 0. Calls TDocument::SetDocPath to set the
/// directory path. If mode is not 0, sets TDocument::OpenMode to mode. If the file
/// cannot be opened, returns 0.
//
bool
TFileDocument::Open(int omode, LPCTSTR path)
{
  if (FHdl != HFILE_ERROR)
    return false;    // if already open at document level

  if (path)
    SetDocPath(path);
  if (omode != 0)
    SetOpenMode(omode);

  streampos seekpos;
  if ((FHdl = OpenThisFile(GetOpenMode(),GetDocPath(),&seekpos)) == HFILE_ERROR)
    return false;
  return true;
}

//
/// Opens a file document using an existing file handle. Sets TDocument::OpenMode to
/// PREV_OPEN and read/write. Sets the document path to 0. Sets FHdl to fhdl. Always
/// returns a nonzero value.
//
bool
TFileDocument::Open(HFILE fhdl)
{
  SetOpenMode(PREV_OPEN | ofReadWrite); // !CQ can we determine open mode?
  SetDocPath(0);  // !CQ can we get path name?
  FHdl = fhdl;
  return true;
}

//
/// Closes the document but does not delete or detach any associated views. Before
/// closing the document, Close calls TDocument::Close to make sure all child
/// documents are closed. If any children are open, Close returns 0 and does not
/// close the document. If all children are closed, Close checks to see if any
/// associated streams are open, and if so, returns 0 and does not close the
/// document. If there are no open streams, Close closes the file.
//
bool
TFileDocument::Close()
{
  if (!TDocument::Close())     // close all children first
    return false;
  if (FHdl != HFILE_ERROR) {   // if open at document level
    if (TDocument::IsOpen())   // cannot close document if streams open
      return false;       // ?should we close streams here?
    CloseThisFile(FHdl, GetOpenMode());
    FHdl = HFILE_ERROR;
  }
  return true;
}

//
/// Calls TDocument::Commit and clears TDocument's DirtyFlag data member, thus
/// indicating that there are no unsaved changes made to the document.
//
bool
TFileDocument::Commit(bool force)
{
  if (!TDocument::Commit(force))      // flush views and child docs
    return false;
  SetDirty(false);
  return true;
}

//
/// Calls TDocument::Revert to notify the views to refresh their data. If clear is
/// false, the data is restored instead of cleared.
//
bool
TFileDocument::Revert(bool clear)
{
  if (!TDocument::Revert(clear))
    return false;
  SetDirty(false);
  return true;
}

static LPCTSTR PropNames[] = {
  _T("Create Time"),   // CreateTime
  _T("Modify Time"),   // ModifyTime
  _T("Access Time"),   // AccessTime
  _T("Storage Size"),  // StorageSize
  _T("File Handle"),   // FileHandle
};

static int PropFlags[] = {
  pfGetBinary|pfGetText,   // CreateTime
  pfGetBinary|pfGetText,   // ModifyTime
  pfGetBinary|pfGetText,   // AccessTime
  pfGetBinary|pfGetText,   // StorageSize
  pfGetBinary,             // FileHandle
};

//
/// Returns the text name of the property given the index value.
//
LPCTSTR
TFileDocument::PropertyName(int index)
{
  if (index <= PrevProperty)
    return TDocument::PropertyName(index);
  else if (index < NextProperty)
    return PropNames[index-PrevProperty-1];
  else
    return 0;
}

//
/// Returns the property attribute constants (pfGetText, pfHidden, and so on).
//
int
TFileDocument::PropertyFlags(int index)
{
  if (index <= PrevProperty)
    return TDocument::PropertyFlags(index);
  else if (index < NextProperty)
    return PropFlags[index-PrevProperty-1];
  else
    return 0;
}

//
/// Gets the property index, given the property name (name). Returns 0 if the name
/// is not found.
//
int
TFileDocument::FindProperty(LPCTSTR name)
{
  int i;
  for (i=0; i < NextProperty-PrevProperty-1; i++)
    if (_tcscmp(PropNames[i], name) == 0)
      return i+PrevProperty+1;
  return TDocument::FindProperty(name);
}

//
/// Overrides TDocument::GetProperty and gets the property ID for the current file
/// document.
//
int
TFileDocument::GetProperty(int prop, void * dest, int textlen)
{
  switch(prop) {
    case FileHandle:
      if (textlen)
        return 0;
      *(HFILE *)dest = FHdl;
      return sizeof(FHdl);
    default:
      if (InfoPresent) {
        switch(prop) {
          case StorageSize:
            if (!textlen) {
              *(unsigned long *)dest = FileLength;
              return sizeof(FileLength);
            }
            else {
              tchar buf[10];
              int len = _stprintf(buf, _T("%ld"), FileLength);
              if (textlen > len)
                textlen = len;
              memcpy(dest, buf, textlen*sizeof(tchar));
              *((LPTSTR)dest + textlen) = 0;
              return len;
            }
          case CreateTime:
            return FormatFileTime(&FileCreateTime, dest, textlen);
          case ModifyTime:
            return FormatFileTime(&FileUpdateTime, dest, textlen);
          case AccessTime:
            return FormatFileTime(&FileAccessTime, dest, textlen);
        }
      }
      return TDocument::GetProperty(prop, dest, textlen);
  }
}

//
/// Sets the property data, which must be in the native data type (either string or
/// binary).
//
bool
TFileDocument::SetProperty(int prop, const void * src)
{
  // File properties currently not settable
  //
  return TDocument::SetProperty(prop, src);
}

//
/// Overrides TDocument::InStream and provides generic input for the particular
/// storage medium. InStream returns a pointer to a TInStream. mode is a combination
/// of the ios bits defined in iostream.h. strmId is not used for file documents.
/// The view reads data from the document as a stream or through stream functions.
//
TInStream*
TFileDocument::InStream(int omode, LPCTSTR /*strmId*/)
{
  HFILE fhdl;
  streampos seekpos;
  if (omode == ofParent)
    omode = GetOpenMode();
  if (!(omode & ofRead))
    return 0;
  if ((fhdl = FHdl) == HFILE_ERROR) {   // if file not open at document level
    if ((fhdl=OpenThisFile (omode, GetDocPath(), &seekpos)) == HFILE_ERROR)
      return 0;
  }
  else {
#if 0 // suggested by Greg Chicares
    omode = GetOpenMode() ? (GetOpenMode() & ~ofBinary) | (omode & ofBinary)
                          : omode;
    omode |= PREV_OPEN;
#else
     ::_lclose(FHdl);
     FHdl = HFILE_ERROR;
    if ((fhdl=OpenThisFile (omode, GetDocPath(), &seekpos)) == HFILE_ERROR)
       return 0;
#endif
  }
  return new TFileInStream(*this, fhdl, omode, seekpos);
}

//
/// Overrides TDocument::OutStream and provides generic input for the particular
/// storage medium. OutStream returns a pointer to a TOutStream. mode is a
/// combination of the ios bits defined in iostream.h. strmId is not used for file
/// documents. Instead, the view reads data from the document through stream
/// functions.
//
TOutStream*
TFileDocument::OutStream(int omode, LPCTSTR /*strmId*/)
{
  HFILE fhdl;
  streampos seekpos;
  if (omode == ofParent)
    omode = GetOpenMode();
  if (!(omode & ofWrite))
    return 0;
  if ((fhdl = FHdl) == HFILE_ERROR) {   // if file not open at document level
    if ((fhdl=OpenThisFile (omode, GetDocPath(), &seekpos)) == HFILE_ERROR)
      return 0;
  }
  else {
#if 0 // suggested by Greg Chicares
    omode = GetOpenMode() ? (GetOpenMode() & ~ofBinary) | (omode & ofBinary)
                          : omode;
    omode |= PREV_OPEN;
#else
    ::_lclose(FHdl);
    FHdl = HFILE_ERROR;
    if ((fhdl=OpenThisFile (omode, GetDocPath(), &seekpos)) == HFILE_ERROR)
       return 0;
#endif
  }
  return new TFileOutStream(*this, fhdl, omode, seekpos);
}

//----------------------------------------------------------------------------

//
//
//
TFileInStream::~TFileInStream()
{
  ((TFileDocument&)Doc).CloseThisFile(buf.xfd, GetOpenMode());
}

//
//
//
TFileOutStream::~TFileOutStream()
{
  if (buf.out_waiting())
    buf.overflow(EOF);
  ((TFileDocument&)Doc).CloseThisFile(buf.xfd, GetOpenMode());
}

//----------------------------------------------------------------------------
// class TFileBuf
//

const int B_size = 516; // natural size for a file buffer, plus 4 for putback;

//
// Make a TFileBuf attached to an open fd
//
TFileBuf::TFileBuf(int fhdl, int omode, streampos seekpos)
{
  base_ = ebuf_ = 0;
  xfd = fhdl;     // assumed to be valid
  mode = omode;   // this may not represent the actual mode opened previously
  last_seek = seekpos;
  tchar* p = new tchar[B_size];
  if (p) {
    setb(p, p+B_size, 1);   // ~streambuf() will delete buffer
    setp(p+4, p+4);
    setg(p, p+4, p+4);
  }
}

//
// Seek file to position.
// We take a simple approach, and don't check for small position changes
// within the current buffer.
//
TFileBuf::pos_type TFileBuf::seekoff(off_type off, ios_base::seekdir dir,
                                     ios_base::openmode /*which*/)
{
  streamsize loff = off;
  if (out_waiting()) {       // flush the output
     if (sync() == eof())
        return eof();
  }
  else if (dir == ios::cur) {
    streamsize count = in_avail();
    if (count != 0) {
      loff -= count;

      //  if we're in text mode, need to allow for newlines
      //  in the buffer
      if ((mode & ofBinary) == 0) {
        tchar *tptr = gptr();
        while (tptr != egptr())
          if (*tptr++ == _T('\n'))
            loff--;
      }
    }
  }

  if (!IsRepresentable<LONG>(loff)) // Need check, because _llseek takes LONG lOffset argument.
    return streampos(eof());
  LONG lOffset = static_cast<LONG>(loff);
  int w = (dir==ios::beg) ? SEEK_SET : ((dir==ios::cur) ? SEEK_CUR:SEEK_END);
  last_seek = ::_llseek(xfd, lOffset, w);

  if (!unbuffered() && base()) {      // set up get and put areas
    size_t pb = (blen() > 8) ? 4 : 1;  // putback area size
    tchar *b = base();
    setp(b + pb, b + pb);
    setg(b, b + pb, b + pb);
  }
  return (last_seek == streampos(HFILE_ERROR)) ?   streampos(eof()) : last_seek;
}

//
//
//
int TFileBuf::sync()
{
  uint count = out_waiting();
  if (count) {
    _USES_CONVERSION;
    tchar* curp;
    tchar* srcp = pbase();
    tchar* endp = srcp + count;

    // Convert LF's to CR/LF if text mode
    //
    if ((mode & ofBinary) == 0) {
      for (curp = srcp; curp < endp; curp++) {
        if (*curp == _T('\n')){
          *curp = _T('\r');
          count = (int)(curp - srcp) + 1;
          if (::_lwrite(xfd, _W2A(srcp), count) != count)
            return eof();
          *(srcp = curp) = _T('\n');
        }
      }
      count = uint(curp - srcp);  // write what remains in the buffer below
    }
    if (::_lwrite(xfd, _W2A(srcp), count) != count)
      return eof();

    // reset get and put areas
    int     pb = (blen() > 8) ? 4 : 1;  // putback area size
    tchar* b = base();
    setp(b+pb, b+blen());
    setg(b, b+pb, b+pb);
  }
  else if (in_avail()) {
    last_seek = ::_llseek(xfd, long(-in_avail()), SEEK_CUR);
    setg(eback(), gptr(), gptr());
    setp(gptr(), gptr());
    if (last_seek == streampos(HFILE_ERROR))
      return eof();
  }
  return 0;
}

//
//
//
TFileBuf::int_type 
TFileBuf::underflow()
{
  if ((mode & (ofRead | ofWrite)) == ofWrite)
    return eof();

  if (in_avail())    // no action needed
    return (tchar)*gptr();

  int c = 0;  // the return value
  int count;  // input character count

  if (!unbuffered() && base()) {     // this is buffered
    if (sync() != 0)
      return eof();

    // find buffer data
    int pb = (blen() > 8) ? 4 : 1;  // putback area size
    tchar* begp = base() + pb;

    // read in a new buffer
    count = ::_lread(xfd, begp, blen()-pb);
    if (count == HFILE_ERROR)
      return eof();

    // remove CR's if text mode
    if ((mode & ofBinary) == 0) {
      tchar* endp = begp + count;
      tchar* dstp = 0;
      tchar* srcp = 0; // initialized only to prevent compiler warning
      tchar* curp;

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
    setg(base(), begp, begp + count);
    setp(begp, begp);

    if (count)
      c = (utchar) *gptr();
  }
  else {     // this is not buffered
    for (;;) {
      count = ::_lread(xfd, lahead, 1);
      if (count == HFILE_ERROR) {
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
// Always flush
//
TFileBuf::int_type TFileBuf::overflow(int_type c)
{
  if ((mode & (ofRead | ofWrite)) == ofRead)
    return eof();

  if (unbuffered() || !base()) {
    if (c != eof()) {
      _USES_CONVERSION;
      uint count;
      tchar b[2];
      if (c == _T('\n') && (mode & ofBinary) == 0) {
        b[0] = _T('\r');
        b[1] = (tchar)c;
        count = 2;
      }
      else {
        b[0] = (tchar)c;
        count = 1;
      }
      if (::_lwrite(xfd, _W2A(b), count) != count)
        return eof();
    }
  }
  else {   // now we know this is buffered and state is not bad

    // resets get and put areas
    if (sync() != 0)
      return eof();

    // reset get and put areas
    int pb = (blen() > 8) ? 4 : 1;  // putback area size
    tchar *b = base();
    setp(b+pb, b+blen());
    setg(b, b+pb, b+pb);

    if (c != eof()) {
      sputc(static_cast<tchar>(c));
      gbump(1);       // pptr and gptr must be the same
    }
  }
  return 1;
}

tstreambuf * TFileBuf::setbuf(char_type *s, streamsize n)
{
  delete[] base_;
  base_ = s;
  ebuf_ = s + n;
  return this;
}

TFileBuf::pos_type TFileBuf::seekpos(pos_type sp,
                 ios_base::openmode which){
  return seekoff(streamoff(sp), ios::beg, which);
}

//----------------------------------------------------------------------------
// class TFileStreamBase
//

//
//
//
TFileStreamBase::TFileStreamBase(int fhdl, int omode, streampos seekpos)
:
  buf(fhdl, omode, seekpos)
{
  tios::init(&buf);
}


IMPLEMENT_STREAMABLE1(TFileDocument, TDocument);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TFileDocument::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TFileDocument* o = GetObject();
  o->FHdl = HFILE_ERROR;    // initialize to closed file
  o->InfoPresent = false;
  ReadBaseObject((TDocument*)o, is);
  return o;
}

//
//
//
void
TFileDocument::Streamer::Write(opstream& os) const
{
  //  assumed that document is committed at this point
  WriteBaseObject((TDocument*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
//=========================================================================================
