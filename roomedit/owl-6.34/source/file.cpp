//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
// Copyright (c) 1996-1998 Bidus Yura, All rights Reserved
//
/// \file
/// TFile class implementation.
/// Fully rewritten by Yura Bidus
//------------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/file.h>
#include <owl/filename.h>
#include <owl/wsyscls.h>
#include <owl/private/dir.h>              // for findfirst()
#include <owl/private/strmdefs.h>

#if (__BORLANDC__ > 0x580)
#include <istream> 
#endif

#if  defined(__BORLANDC__)
# pragma option -w-ccc // Disable warning; "Condition is always true/false".
# pragma option -w-inl // Disable warning in standard library; "Functions containing 'statement' are not expanded inline".
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;

/// \cond

// ////////////////////////////
// Internal support structure
#include <pshpack1.h>
struct __i_uint32_32{
  uint32 Lo;
  uint32 Hi;
};
struct __i_int32_32{
  uint32 Lo;
  int32 Hi;
};
#include <poppack.h>

/// \endcond


DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlFileSystem, 1, 0);

//
/// Constructs an exception handling class with identity IDS_BADFILEFORMAT.
//
TXBadFormat::TXBadFormat()
:
  TXOwl(IDS_BADFILEFORMAT)
{
}

//
/// Construct a TXBadFormat exception from scratch, and throws it.
//
void
TXBadFormat::Raise()
{
  TXBadFormat().Throw();
}


TXBadFormat*
TXBadFormat::Clone() const
{
  return new TXBadFormat(*this);
}


//
//
void
TXBadFormat::Throw()
{
  throw *this;
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//
// class TDiskFileHandle
// ~~~~~ ~~~~~~~~~~~~~~~
//
//
// helper functions
static void sys2Mode(uint32& access, uint32& sharedMode, uint32& howOpen, uint32 openMode)
{
  if(openMode & TFile::ReadOnly)
    access |= GENERIC_READ;
  if(openMode & TFile::WriteOnly)
    access |= GENERIC_WRITE;

  if(openMode & TFile::PermExclusive)
    sharedMode |= 0; // exclisive - default
  if(openMode & TFile::PermRead)
    sharedMode |= FILE_SHARE_READ; //
  if(openMode & TFile::PermWrite)
    sharedMode |= FILE_SHARE_WRITE; //

//  if(openMode & TFile::PermNone) // not implemented in WIN32
//    ShareMode |= 0; //

  if(openMode & TFile::CreateNew)
    howOpen |= CREATE_NEW  ; // open
  if(openMode & TFile::CreateAlways)
    howOpen |= CREATE_ALWAYS; // open
  if(openMode & TFile::OpenExisting)
    howOpen |= OPEN_EXISTING; // open
  if(openMode & TFile::TruncateExist)
    howOpen |= TRUNCATE_EXISTING; // open
}

//
// class TDiskFileHandle
// ~~~~  ~~~~~~~~~~~~~~~
//
TDiskFileHandle::TDiskFileHandle(const tstring& fileName, uint32 mode)
{
  OpenMode = mode;
  FileName = fileName;

  uint32 Access      = 0;
  uint32 ShareMode  = 0;
  uint32 HowOpen    = 0;
  sys2Mode(Access, ShareMode, HowOpen, OpenMode);

  uint32 flags = FILE_FLAG_RANDOM_ACCESS;
  Handle = ::CreateFile(FileName.c_str(),Access,ShareMode,NULL,HowOpen,flags,0);
}


//
TDiskFileHandle::TDiskFileHandle(HANDLE handle, const tstring& fileName, uint mode)
:
  Handle(handle),  OpenMode(mode),  FileName(fileName)
{
}

  
//
TDiskFileHandle* 
TDiskFileHandle::Clone() const
{
  HANDLE hHandle = 0;
  if(::DuplicateHandle(::GetCurrentProcess(), Handle, 
                      ::GetCurrentProcess(), &hHandle,
                    0, false, DUPLICATE_SAME_ACCESS) && hHandle)
    return new TDiskFileHandle(hHandle, FileName.c_str(), OpenMode);
  return 0;
}


//
uint32 TDiskFileHandle::LastError()
{
  return ::GetLastError();
}


//
bool TDiskFileHandle::Close()
{
  if(!::CloseHandle(Handle))
    return false;
  return true;
}


//
uint32 TDiskFileHandle::Read(void * buffer, uint32 numBytes)
{
  DWORD u32Ret;
  if(::ReadFile(Handle, buffer, numBytes, &u32Ret, NULL))
    return u32Ret;
  return TFILE_ERROR;
}


//
bool TDiskFileHandle::Write(const void * buffer, uint32 numBytes)
{
  DWORD u32ret;
  if(::WriteFile(Handle, buffer, numBytes, &u32ret, NULL))
    return true;
  return false;
}


//
bool TDiskFileHandle::Length(uint64 newLen)
{
  if(Seek(int64(newLen),TFile::beg) == TFILE64_ERROR)
    return false;
  if(::SetEndOfFile(Handle))
    return true;
  return false;
}


//
uint64 TDiskFileHandle::Position64()   const
{
  uint32 ret = ::SetFilePointer(Handle,0,0,(uint32)TFile::cur);
  return uint64(ret);
}


//
uint64 TDiskFileHandle::Length64()   const
{
  __i_uint32_32 ul;
  ul.Lo = ::GetFileSize(Handle, (DWORD *)&ul.Hi);
  return *(uint64*)&ul;
}


//
bool TDiskFileHandle::Length(uint32 newLen)
{
  if(Seek((long)newLen, TFile::beg) == TFILE_ERROR)
    return false;
  if(::SetEndOfFile(Handle))
    return true;
  return false;
}


//
uint32 TDiskFileHandle::Position() const
{
  uint32 ret = ::SetFilePointer(Handle,0,0,(uint32)TFile::cur);
  if( ret != 0xFFFFFFFF)
    return ret;
  return TFILE_ERROR;
}


//
uint32 TDiskFileHandle::Length()   const
{
  uint32 ret = ::GetFileSize(Handle, NULL);
  if( ret != 0xFFFFFFFF)
    return ret;
  return TFILE_ERROR;
}


uint64 TDiskFileHandle::Seek(int64 offset, TFile::seek_dir origin)
{
  __i_int32_32 ul = *(__i_int32_32*)&offset;
  ul.Lo = ::SetFilePointer(Handle,ul.Lo,(LONG*)&ul.Hi,(uint32)origin);
  return uint64(*(int64*)&ul);
}

//
uint32 TDiskFileHandle::Seek(long offset, TFile::seek_dir origin)
{
  uint32 ret = ::SetFilePointer(Handle,offset,0,(uint32)origin);
  if( ret != 0xFFFFFFFF)
    return ret;
  return TFILE_ERROR;
}


//
bool TDiskFileHandle::Flush()
{
  if(::FlushFileBuffers(Handle))
    return true;
  return false;
}

//
bool TDiskFileHandle::LockRange(uint32 position, uint32 count)
{
  return ::LockFile(Handle,position,0, count,0);
}

//
bool TDiskFileHandle::UnlockRange(uint32 position, uint32 count)
{
  if(::UnlockFile(Handle,position, 0, count, 0))
    return true;
  return false;
}


bool TDiskFileHandle::LockRange(uint64 position, uint64 count)
{
  __i_uint32_32 up = *(__i_uint32_32*)&position;
  __i_uint32_32 uc = *(__i_uint32_32*)&count;
  return ::LockFile(Handle, up.Lo,up.Hi,uc.Lo,uc.Hi);
}
//
bool TDiskFileHandle::UnlockRange(uint64 position, uint64 count)
{
  __i_uint32_32 up = *(__i_uint32_32*)&position;
  __i_uint32_32 uc = *(__i_uint32_32*)&count;
  return ::UnlockFile(Handle, up.Lo,up.Hi, uc.Lo,uc.Hi);
}


///////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------
//------------------------------------------------------------------------------

uint __sys2Atr(uint32 attrib)
{
  uint attr = 0;
  if(attrib & FILE_ATTRIBUTE_ARCHIVE)
    attr  |= TFile::Archive;
  if(attrib & FILE_ATTRIBUTE_DIRECTORY)
    attr  |= TFile::Directory;
  if(attrib & FILE_ATTRIBUTE_HIDDEN)
    attr  |= TFile::Hidden;
  if(attrib & FILE_ATTRIBUTE_NORMAL)
    attr  |= TFile::Normal;
  if(attrib & FILE_ATTRIBUTE_READONLY)
    attr  |= TFile::RdOnly;
  if(attrib & FILE_ATTRIBUTE_SYSTEM)
    attr  |= TFile::System;
  if(attrib & FILE_ATTRIBUTE_TEMPORARY)
    attr  |= TFile::Temporary;
  return attr;
}
uint __attr2Sys( uint attrib)
{
  uint attr = 0;
  if(attrib & TFile::Archive)
    attr  |= FILE_ATTRIBUTE_ARCHIVE;
  if(attrib & TFile::Directory)
    attr  |= FILE_ATTRIBUTE_DIRECTORY;
  if(attrib & TFile::Hidden)
    attr  |= FILE_ATTRIBUTE_HIDDEN;
  if(attrib & TFile::Normal)
    attr  |= FILE_ATTRIBUTE_NORMAL;
  if(attrib & TFile::RdOnly)
    attr  |= FILE_ATTRIBUTE_READONLY;
  if(attrib & TFile::System)
    attr  |= FILE_ATTRIBUTE_SYSTEM;
  if(attrib & TFile::Temporary)
    attr  |= FILE_ATTRIBUTE_TEMPORARY;
  return attr;

}


TTime __MakeTTime(uint16 fdate, uint16 ftime);
void __MakeDosTime(const TTime& time, uint16& fdate, uint16& ftime);
uint __sys2Atr(uint32 attrib);
uint __attr2Sys(uint newattr);

//
bool TDiskFileHandle::GetStatus(TFileStatus  & status) const
{
  if (::_tfullpath(status.fullName, FileName.c_str(), COUNTOF(status.fullName)) == 0){
    status.fullName[0] = 0;
    return false;
  }
  uint32 attrib = ::GetFileAttributes(FileName.c_str());
  if(attrib == TFILE_ERROR)
    return false;
  
  status.attribute  = __sys2Atr(attrib);

  TFileTime ftCreate, ftAccess, ftWrite;
  if(::GetFileTime(Handle, &ftCreate, &ftAccess, &ftWrite) != TRUE)
    return false;

  //uint16 ftime,fdate;
  //FileTimeToDosDateTime(&ftCreate, &fdate, &ftime);
  //status.createTime = __MakeTTime(fdate, ftime);
  status.createTime = TTime(ftCreate);
  //FileTimeToDosDateTime(&ftWrite, &fdate, &ftime);
  //status.modifyTime = __MakeTTime(fdate, ftime);
  status.modifyTime = TTime(ftWrite);
  //FileTimeToDosDateTime(&ftAccess, &fdate, &ftime);
  //status.accessTime = __MakeTTime(fdate, ftime);
  status.accessTime = TTime(ftAccess);
  status.size        = Length();

  return true;
}


//////////////////////////////////////////////////////////////////////
// class TFile
// ~~~~~ ~~~~~
//
//
//
/// If ShouldClose is true the file is closed. If the Handle is not FileNull Handle
/// is deleted. Buffer is deleted.
//
TFile::~TFile()
{
  if(ShouldClose)
    Close();
  if(Handle != FileNull)
    delete Handle;
  delete[] Buffer;
}


// if TFileHandle not support Clone() will throw TXNotSupportedCall()
TFile::TFile(const TFile & file)
{
  if(ShouldClose)
    Close();
  if(Handle != FileNull)
    delete Handle;
  Handle = file.GetHandle()->Clone();
  ShouldClose = true;
  
  CHECK(Handle);
  
  if(!Handle)
    TXNotSupportedCall::Raise();
}


/// Opens file name with the given mode. Returns true if successful, false
/// otherwise. Calling Open when the file is already open will fail.
bool TFile::Open(const tstring& fileName, const uint32 openMode)
{
  if(IsOpen())
    return false;
  Handle = new TDiskFileHandle(fileName, openMode);
  if(!Handle->IsOpen()){
    delete Handle;
    Handle = FileNull;
  }
  ShouldClose = true;
  return IsOpen();
}

//------------------------------------------------------------------------------

/// Closes the file. Returns true if successful, false otherwise.
bool TFile::Close()
{
  if(IsOpen() && ShouldClose){
    if(!Handle->Close())
      return false;
  }
  if(Handle != FileNull)
    delete Handle;
  Handle = FileNull;
  ShouldClose = false;
  return true;
}


/// Performs any pending I/O functions. Returns true if successful; false otherwise.
bool TFile::Flush()
{
  PRECONDITION(Handle);
  return Handle->Flush();
}


/// Locks count bytes, beginning at position of the file. Returns true if
/// successful; false otherwise.
bool TFile::LockRange(uint32 position, uint32 count)
{
  PRECONDITION(Handle);
  return Handle->LockRange(position, count);
}

/// Unlocks the range at the given Position. Returns true if successful; false
/// otherwise.
bool TFile::UnlockRange(uint32 position, uint32 count)
{
  PRECONDITION(Handle);
  return Handle->UnlockRange(position, count);
}


inline int binCVTCharMBF (uint8* src, uint8* dst, int /*type*/, int size, int count)
{
  memcpy(dst, src, count*size);
  return count*size;
}
//------------------------------------------------------------------------------
static int binCVTShortMBF(uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  uint16* src = (uint16*)_src;
  uint16* dst = (uint16*)_dst;
  while(count--)
    *dst++ = SwapUint16(*src++);

  return retval;
}
//------------------------------------------------------------------------------
static int binCVTLongMBF(uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  uint32* src = (uint32*)_src;
  uint32* dst = (uint32*)_dst;
  while(count--)
    *dst++ = SwapUint32(*src++);

  return retval;
}
//------------------------------------------------------------------------------
static int binCVTFloatMBF (uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  uint32* src = (uint32*)_src;
  uint32* dst = (uint32*)_dst;
  while(count--)
    *dst++ = SwapUint32(*src++);

  return retval;
}
//------------------------------------------------------------------------------
static int binCVTDoubleMBF (uint8*, uint8*, int, int, int)
{
  return 0;
}
//------------------------------------------------------------------------------
static int binCVTInt64MBF (uint8* _src, uint8* _dst, int /*type*/, int size, int count)  //???????????????????
{
  int retval = count*size;
  __i_uint32_32* src = (__i_uint32_32*)_src;
  __i_uint32_32* dst = (__i_uint32_32*)_dst;
  while(count--){
    dst->Lo = SwapUint32(src->Hi);
    dst->Hi = SwapUint32(src->Lo);
    dst++;
    src++;
  }
  return retval;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline int binCVTCharLBF (uint8* src, uint8* dst, int /*type*/, int size, int count){
  memcpy(dst, src, count*size);
  return count*size;
}
//------------------------------------------------------------------------------
static int binCVTShortLBF (uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  uint16* src = (uint16*)_src;
  uint16* dst = (uint16*)_dst;
  while(count--)
    *dst++ = SwapUint16(*src++);

  return retval;
}
//------------------------------------------------------------------------------
static int binCVTLongLBF (uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  uint32* src = (uint32*)_src;
  uint32* dst = (uint32*)_dst;
  while(count--)
    *dst++ = SwapUint32(*src++);

  return retval;
}
//------------------------------------------------------------------------------
static int binCVTFloatLBF (uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  uint32* src = (uint32*)_src;
  uint32* dst = (uint32*)_dst;
  while(count--)
    *dst++ = SwapUint32(*src++);

  return retval;
}
//------------------------------------------------------------------------------
static int binCVTDoubleLBF (uint8*, uint8*, int, int, int)
{
  return 0;
}
//------------------------------------------------------------------------------
static int binCVTInt64LBF (uint8* _src, uint8* _dst, int /*type*/, int size, int count)
{
  int retval = count*size;
  __i_uint32_32* src = (__i_uint32_32*)_src;
  __i_uint32_32* dst = (__i_uint32_32*)_dst;
  while(count--){
    dst->Lo = SwapUint32(src->Hi);
    dst->Hi = SwapUint32(src->Lo);
    dst++;
    src++;
  }
  return retval;
}
//------------------------------------------------------------------------------
typedef  int (*cvtDispatchFunc)(uint8*, uint8*, int, int, int);  /* Dispatch table entry    */
static cvtDispatchFunc dispatchMBF[] = {      /* MBF -> LBF */
  0,                /* varEnd         */
  binCVTLongMBF,    /* varPOINTER     */
  binCVTCharMBF,    /* varCHAR        */
  binCVTCharMBF,    /* varUCHAR       */
  binCVTShortMBF,   /* varSHORT       */
  binCVTLongMBF,    /* varLONG        */
  binCVTCharMBF,    /* varINT8        */
  binCVTShortMBF,   /* varINT16       */
  binCVTLongMBF,    /* varINT32       */
  binCVTShortMBF,   /* varUSHORT      */
  binCVTLongMBF,    /* varULONG       */
  binCVTCharMBF,    /* varUINT8       */
  binCVTShortMBF,   /* varUINT16      */
  binCVTLongMBF,    /* varUINT32      */
  binCVTFloatMBF,   /* varFLOAT       */
  binCVTDoubleMBF,  /* varDOUBLE      */
  binCVTInt64MBF,   /* varINT64       */
  binCVTInt64MBF,   /* varUINT64r     */
};

static cvtDispatchFunc dispatchLBF[] = {      /* LBF -> MBF */
  0,                /* varEnd         */
  binCVTLongLBF,    /* varPOINTER     */
  binCVTCharLBF,    /* varCHAR        */
  binCVTCharLBF,    /* varUCHAR       */
  binCVTShortMBF,   /* varSHORT       */
  binCVTLongLBF,    /* varLONG        */
  binCVTCharLBF,    /* varINT8        */
  binCVTShortMBF,   /* varINT16       */
  binCVTLongLBF,    /* varINT32       */
  binCVTShortLBF,   /* varUSHORT      */
  binCVTLongLBF,    /* varULONG       */
  binCVTCharLBF,    /* varUINT8       */
  binCVTShortLBF,   /* varUINT16      */
  binCVTLongLBF,    /* varUINT32      */
  binCVTFloatLBF,   /* varFLOAT       */
  binCVTDoubleLBF,  /* varDOUBLE      */
  binCVTInt64LBF,   /* varINT64       */
  binCVTInt64LBF,   /* varUINT64r     */
};

//------------------------------------------------------------------------------
#define BINCHECKRTYPE(type)                        \
  if ( (type) <= 0 || (type) > varLastMember)      \
    return 0;

#define BINCHECKWTYPE(type)                        \
  if ((type) <= 0 || (type) > varLastMember)      \
    return 0;

#define BINCHECKNBYTES(nBytes)   \
  if((nBytes) <= 0 )            \
    return 0;
//------------------------------------------------------------------------------
/// Returns the number of bytes in a binary data structure.
uint TFile::StructSize(TBinField * fields)
{
  uint count = 0;
  for(TBinField *bfp = fields; bfp->Count; bfp++)
    count += bfp->Count*bfp->Bytes;
  return count;
}
//------------------------------------------------------------------------------
/// Read binary structure from file.
/// Returns # of bytes read
uint TFile::ReadStruct(void* buffer, TBinField* fields, TByteOrderType type)
{
  TBinField *bfp;      /* Field pointer              */
  uint count;          /* Number of bytes read        */
  int n;              /* Read status & # bytes read  */
  uint8* curBuffer;
  cvtDispatchFunc* binReadDisp;
  cvtDispatchFunc  dispatch;
  //  int (*dispatch)(uint8*, uint8*, int, int, int );  /* Dispatch table entry    */

  /* Check all values in the fields array before doing anything. */
  for(bfp = fields; bfp->Count; bfp++ )  {
    BINCHECKRTYPE(bfp->Type);
    BINCHECKNBYTES(bfp->Bytes)
  }
  // count size of structure
  count = StructSize(fields);

  // if no conversion
  if(EndianType() == type)
    return (uint)Read(buffer, count);

  // if Litle endian machine -> read as Big endian -> last choise
  // else as Little endian
  binReadDisp = dispatchMBF;
  if(EndianType() == boBig_Endian)
    binReadDisp = dispatchLBF;

  if(Buffer && BufSize < count){
    delete[] Buffer;
    Buffer = 0;
  }

  if(!Buffer){
    BufSize = count*5;
    Buffer = new uint8[BufSize];
  }

  if(!Read(Buffer, count))
    return 0;

  count = 0;
  curBuffer = Buffer;
  for(bfp = fields; bfp->Count; bfp++ ){
    dispatch = binReadDisp[bfp->Type];
    if ((n = (*dispatch)(curBuffer,(uint8*)buffer, bfp->Type, bfp->Bytes, bfp->Count)) == -1)
      return 0;    /* Error occured  */
    count   += n;
    buffer = (uint8*)buffer +  bfp->Bytes * bfp->Count;
    curBuffer += n;
  }
  return count;
}
//------------------------------------------------------------------------------
/// Writes structures to binary file based on fields description.
/// Returns # of bytes written
uint TFile::WriteStruct(void* buffer, TBinField* fields, TByteOrderType type)
{
  TBinField *bfp;      /* Field pointer              */
  uint count;          /* Number of bytes read        */
  int n;              /* Read status & # bytes read  */
  uint8* curBuffer;
  cvtDispatchFunc* binReadDisp;
  cvtDispatchFunc  dispatch;

  /* Check all values in the fields array before doing anything. */
  for(bfp = fields; bfp->Count; bfp++ )  {
    BINCHECKRTYPE(bfp->Type);
    BINCHECKNBYTES(bfp->Bytes)
  }
  // count size of structure
  count = StructSize(fields);

  // if no conversion
  if(EndianType() == type)
    return Write(buffer, count);

  // if Litle endian machine -> read as Big endian -> last choise
  // else as Little endian
  binReadDisp = dispatchMBF;
  if(EndianType() == boBig_Endian)
    binReadDisp = dispatchLBF;

  if(Buffer && BufSize < count){
    delete[] Buffer;
    Buffer = 0;
  }

  if(!Buffer){
    BufSize = count*5;
    Buffer = new uint8[BufSize];
  }

  count = 0;
  curBuffer = Buffer;
  for(bfp = fields; bfp->Count; bfp++ ){
    dispatch = binReadDisp[bfp->Type];
    if ((n = (*dispatch)((uint8*)buffer, curBuffer, bfp->Type, bfp->Bytes, bfp->Count)) == -1)
      return 0;    /* Error occured  */
    count   += n;
    buffer = (uint8*)buffer + bfp->Bytes * bfp->Count;
    curBuffer += n;
  }
  return Write(Buffer, count);
}
//------------------------------------------------------------------------------
/// Read binary structure from buffer.
/// Returns # of bytes read
uint TFile::ReadStruct(uint8* ReadBuf, void* buffer, TBinField* fields, TByteOrderType type)
{
  TBinField *bfp;      /* Field pointer              */
  int count;          /* Number of bytes read        */
  int n;              /* Read status & # bytes read  */
  uint8* curBuffer;
  cvtDispatchFunc* binReadDisp;
  cvtDispatchFunc  dispatch;
//  int (*dispatch)(uint8*, uint8*, int, int, int );  /* Dispatch table entry    */

  /* Check all values in the fields array before doing anything. */
  for(bfp = fields; bfp->Count; bfp++ )  {
    BINCHECKRTYPE(bfp->Type);
    BINCHECKNBYTES(bfp->Bytes)
  }
  // count size of structure
  count = StructSize(fields);

  // if no conversion
  if(EndianType() == type){
    memcpy(buffer, ReadBuf, count);
    return count;
  }

  // if Litle endian machine -> read as Big endian -> last choise
  // else as Little endian
  binReadDisp = dispatchMBF;
  if(EndianType() == boBig_Endian)
    binReadDisp = dispatchLBF;

  count = 0;
  curBuffer = ReadBuf;
  for(bfp = fields; bfp->Count; bfp++ ){
    dispatch = binReadDisp[bfp->Type];
    if ((n = (*dispatch)(curBuffer,(uint8*)buffer, bfp->Type, bfp->Bytes, bfp->Count)) == -1)
      return 0;    /* Error occured  */
    count   += n;
    buffer = (uint8*)buffer + bfp->Bytes * bfp->Count;
    curBuffer += n;
  }
  return count;
}
//------------------------------------------------------------------------------
/// Writes structures to buffer based on fields description.
/// Returns # of bytes written
uint TFile::WriteStruct(uint8* OutBuf, void* buffer, TBinField* fields, TByteOrderType type)
{
  TBinField *bfp;      /* Field pointer              */
  int count;          /* Number of bytes read        */
  int n;              /* Read status & # bytes read  */
  uint8* curBuffer;
  cvtDispatchFunc* binReadDisp;
  cvtDispatchFunc  dispatch;

  /* Check all values in the fields array before doing anything. */
  for(bfp = fields; bfp->Count; bfp++ )  {
    BINCHECKRTYPE(bfp->Type);
    BINCHECKNBYTES(bfp->Bytes)
  }
  // count size of structure
  count = StructSize(fields);

  // if no conversion
  if(EndianType() == type){
    memcpy(OutBuf, buffer, count);
    return count;
  }

  // if Litle endian machine -> read as Big endian -> last choise
  // else as Little endian
  binReadDisp = dispatchMBF;
  if(EndianType() == boBig_Endian)
    binReadDisp = dispatchLBF;

  count = 0;
  curBuffer = OutBuf;
  for(bfp = fields; bfp->Count; bfp++ ){
    dispatch = binReadDisp[bfp->Type];
    if ((n = (*dispatch)((uint8*)buffer, curBuffer, bfp->Type, bfp->Bytes, bfp->Count)) == -1)
      return 0;    /* Error occured  */
    count   += n;
    buffer = (uint8*)buffer + bfp->Bytes * bfp->Count;
    curBuffer += n;
  }
  return count;
}


static TBinField CharField[] = {
  {varCHAR, 1, 1},
  {varEnd,   0, 0},
};
static TBinField ShortField[] = {
  {varSHORT, 2, 1},
  {varEnd,    0, 0},
};
static TBinField LongField[] = {
  {varLONG, 4, 1},
  {varEnd,   0, 0},
};
static TBinField FloatField[] = {
  {varFLOAT, 4, 1},
  {varEnd,    0, 0},
};
static TBinField DoubleField[] = {
  {varDOUBLE, 8, 1},
  {varEnd,     0, 0},
};
static TBinField Long64Field[] = {
  {varDOUBLE, 8, 1},
  {varEnd,     0, 0},
};
static TBinField PointField[] = {
  {varLONG,  4, 2},
  {varEnd,   0, 0},
};
static TBinField RectField[] = {
  {varLONG,  4, 2},
  {varEnd,   0, 0},
};
static TBinField* FieldTypes[] = {
  CharField,
  ShortField,
  LongField,
  FloatField,
  DoubleField,
  Long64Field,
  PointField,
  RectField,
};
//------------------------------------------------------------------------------
/// Read binary predefined structure from file.
/// Returns # of bytes read
uint TFile::ReadStruct(void* buffer, BinType btype, TByteOrderType type)
{
  return ReadStruct(buffer, FieldTypes[btype], type);
}
//------------------------------------------------------------------------------
/// Writes predefined structures to binary file.
uint TFile::WriteStruct(void* buffer, BinType btype, TByteOrderType type)
{
  return WriteStruct(buffer, FieldTypes[btype], type);
}

#if defined(OWL5_COMPAT) // must be moved to TFileName
//
/// Fills status with the status for name. Returns nonzero if successful, 0 otherwise. 
/// \note  This is only available when OWL has been compiled with
/// OWL5_COMPAT defined. The TFileName class should be used instead.
bool TFile::GetStatus(LPCTSTR name, TFileStatus & status)
{
  return TFileName(name).GetStatus(status);
}

//
/// Sets file name's status to status.
/// \note	This is only available when OWL has been compiled with OWL5_COMPAT
/// defined. The TFileName class should be used instead.
bool TFile::SetStatus(LPCTSTR name, const TFileStatus  & status)
{
  return TFileName(name).SetStatus(status);
}
#endif //
//
TFile&  operator >> ( TFile& file, tstring& s)
{
  int size = (int)file.readUint32();
  s = _T("");
  s.resize(size+1);
  if(file.Read((void*)s.c_str(), size) == TFILE_ERROR)
    size = -1;
  s[size+1] = _T('\0');
  return file;
}



uint8 
TFile::readUint8()
{
  char c;
  Read(&c, 1);
  return c;
}


uint16 
TFile::readUint16()
{

  uint16 i;
  ReadStruct(&i, TypeShort, boLittle_Endian);
  return i;
}


uint32 
TFile::readUint32(){
  uint32 i;
  ReadStruct(&i, TypeLong, boLittle_Endian);
  return i;
}


uint64 
TFile::readUint64(){
  uint64 i;
  ReadStruct(&i, TypeLong64, boLittle_Endian);
  return i;
}


float 
TFile::readFloat()
{
  float i;
  ReadStruct(&i, TypeFloat, boLittle_Endian);
  return i;
}


double 
TFile::readDouble()
{
  double i;
  ReadStruct(&i, TypeDouble, boLittle_Endian);
  return i;
}


void  TFile::writeUint8( const uint8 c)
{
  Write(&c, 1);
}


void  TFile::writeUint16( const uint16 i)
{
  WriteStruct((void*)&i, TypeShort, boLittle_Endian);
}


void 
TFile::writeUint32( const uint32 i)
{
  WriteStruct((void*)&i, TypeLong, boLittle_Endian);
}


void 
TFile::writeUint64( const uint64 i)
{
  WriteStruct((void*)&i, TypeLong64, boLittle_Endian);
}


void 
TFile::writeFloat( const float f)
{
  WriteStruct((void*)&f, TypeFloat, boLittle_Endian);
}


void 
TFile::writeDouble( const double d)
{
  WriteStruct((void*)&d, TypeDouble, boLittle_Endian);
}


//------------------------------------------------------------------------------
//
//  TBufferedFile
//  ~~~~~~~~~~~~~

//
/// The buffer is flushed. If ShouldDelete is true the buffer is deleted.
//
TBufferedFile::~TBufferedFile()
{
  FlushBuffer();
  if(ShouldDelete)
    delete[] FileBuffer;
  FileBuffer   = 0;
}
//------------------------------------------------------------------------------
/// Allocates a buffer of size bytes.
void TBufferedFile::InitBuffer(uint size)
{
  BufferEmpty = true;
  EndPos = FileBufSize = size;
  CurByte = FileBuffer = new uint8[size];
  ShouldDelete = true;
  CurPos = StartPos = 0;
}
//------------------------------------------------------------------------------
/// Flushes the buffer by writing any unwritten data to the file. This function is
/// called by Flush() before the operating system is called to flush its buffers.
bool TBufferedFile::FlushBuffer()
{
  // if open mode -> write flush buffer
  if(GetHandle() && (GetHandle()->GetOpenMode() & WriteOnly)){
     if(CurPos != StartPos){
       if(!TFile::Write(FileBuffer, CurPos-StartPos)){
        WARNX(OwlFileSystem,1, 0,"TBufferedFile::FlushBuffer() error Seek(), line: " << __LINE__);
         return false;
      }
       StartPos = CurPos;
      CurByte = FileBuffer;
      EndPos = StartPos + FileBufSize;
      BufferEmpty = true;
    }
  }
  return true;
}


/// Flushes the current buffer and then deletes it if ShouldDelete is true. If
/// buffer is not NULL it is assigned to FileBuffer; otherwise FileBuffer is
/// allocated size bytes. ShouldDelete is then set.
//
void TBufferedFile::SetBuffer(uint8* buffer, uint size, TAutoDelete autodelete)
{
  FlushBuffer();
  if(ShouldDelete)
    delete[] FileBuffer;
  ShouldDelete = autodelete == AutoDelete;
  if(buffer)
    FileBuffer = CurByte = buffer;
  else{
    FileBuffer = CurByte = new uint8[size];
    ShouldDelete = true;
  }
  FileBufSize = size;
  // StartPos = CurPos; set in FlushBuffer
  EndPos    = StartPos + FileBufSize;
}


//
/// Repositions the file pointer to offset bytes from the specified
/// origin. If the position moved to is outside of the buffer, the buffer is
/// flushed. Returns the position moved to or TFILE64_ERROR on error. To get
/// extended error information, call LastError.
//
uint64 TBufferedFile::Seek(int64 offset, seek_dir origin)
{
  PRECONDITION(GetHandle());
  // calculate absolute position ->
  int64 position = CurPos;
  if(origin == cur)
    position += offset;
  else if(origin == end)
    position = int64(Length64()) + offset;
  else
    position = offset;
  uint64 oldpos = CurPos;
   if(position >= int64(StartPos) && position <  int64(EndPos)){
     CurPos = (uint32)position;
     CurByte  = &FileBuffer[CurPos-StartPos];
   }
  else{
    if(!FlushBuffer()){
      WARNX(OwlFileSystem,1, 0,"TBufferedFile::Seek() error FlushBuffer(), line: " << __LINE__);
      return TFILE64_ERROR;
    }
    uint64 newpos = TFile::Seek(offset, origin);
    if(newpos != TFILE64_ERROR){
      StartPos = CurPos = (uint32)position;
       CurByte  = FileBuffer;
      EndPos   = StartPos + FileBufSize;
      BufferEmpty = true;
    }
    else{
      WARNX(OwlFileSystem,1, 0,"TBufferedFile::Seek() error Seek(), line: " << __LINE__);
      return TFILE64_ERROR;
    }
  }
  return oldpos;
}


//
/// Repositions the file pointer to offset bytes from the specified origin. If the
/// position moved to is outside of the buffer, the buffer is flushed. Returns the
/// position moved to or TFILE_ERROR on error. To get extended error information,
/// call LastError.
//
uint32 TBufferedFile::Seek(long offset, seek_dir origin)
{
  PRECONDITION(GetHandle());
  // calculate absolute position ->
  uint32 position = CurPos;
  if(origin == cur)
    position += offset;
  else if(origin == end)
    position = Length() + offset;
  else
    position = offset;
  uint32 oldpos = CurPos;
   if(position >= StartPos && position <  EndPos){
     CurPos = position;
     CurByte  = &FileBuffer[CurPos-StartPos];
   }
  else{
    if(!FlushBuffer()){
      WARNX(OwlFileSystem,1, 0,"TBufferedFile::Seek() error FlushBuffer(), line: " << __LINE__);
      return TFILE_ERROR;
    }
    uint32 newpos = TFile::Seek(offset, origin);
    if(newpos != TFILE_ERROR){
      StartPos = CurPos = position;
       CurByte  = FileBuffer;
      EndPos = StartPos + FileBufSize;
      BufferEmpty = true;
    }
    else{
      WARNX(OwlFileSystem,1, 0,"TBufferedFile::Seek() error Seek(), line: " << __LINE__);
      return TFILE_ERROR;
    }
  }
  return oldpos;
}


//
/// Reads numBytes from the file into buffer. The number of bytes read is returned.
//
uint32 TBufferedFile::Read(void * buffer, uint32 numBytes)
{
  PRECONDITION(GetHandle());
  uint32 retval = numBytes;
  uint8* data = (uint8*)buffer;
  // fill buffer if empty and buffer exist
  if(BufferEmpty && FileBufSize){
     StartPos = CurPos = Position();
    uint32 ret = TFile::Read(FileBuffer, FileBufSize);
    if(ret == TFILE_ERROR){
      WARNX(OwlFileSystem,ret==TFILE_ERROR, 0,"TBufferedFile::Write() error Read(), line: " << __LINE__);
      return ret;
    }
     CurByte      = FileBuffer;
    EndPos       = StartPos + ret;
     BufferEmpty = false;
  }

  while(numBytes > 0){
     uint dataSize = EndPos-CurPos;
    if(numBytes > dataSize){
      if(dataSize > 0){
         memmove(data, CurByte, dataSize);
         numBytes   -= dataSize;
         data       += dataSize;
      }
      uint32 ret;
      if(numBytes > FileBufSize){
        ret = TFile::Read(data, numBytes);
        if(ret == TFILE_ERROR){
          WARNX(OwlFileSystem,ret==TFILE_ERROR, 0,"TBufferedFile::Write() error Read(), line: " << __LINE__);
          return ret;
        }
        BufferEmpty = true;
        numBytes    = 0;
      }
      else{
        ret = TFile::Read(FileBuffer, FileBufSize);
        if(ret == TFILE_ERROR || ret == 0){
          if((buffer == data) && (ret == TFILE_ERROR)){
            WARNX(OwlFileSystem,ret==TFILE_ERROR, 0,"TBufferedFile::Write() error Read(), line: " << __LINE__);
            StartPos   = CurPos;
             return ret;
          }
          else{
            StartPos   = EndPos = CurPos;

            // Note: The number of read bytes should not exceed numBytes (uint32) so we can cast unchecked.
            // TODO: Widen the parameter and return type to avoid this narrowing cast.

            size_t n = data - static_cast<uint8*>(buffer);
            return static_cast<uint32>(n);
          }
        }
      }
      StartPos   = CurPos;
      EndPos     = StartPos + ret;
       CurByte    = FileBuffer;
     }
     else{
      memmove(data, CurByte, numBytes);
       data       += numBytes;
       CurPos    += numBytes;
       CurByte   += numBytes;
       numBytes   =  0;
    }
  }
  return retval;
}


/// Writes numBytes of buffer to the file. If numBytes is greater than the size of
/// the buffer the buffer is flushed and the data is written directly to the file;
/// if the amount of data in the buffer plus numBytes is greater than the capacity
/// of the buffer the buffer is filled and written to the file and the remaining
/// data is placed in the buffer; otherwise the data is appended to the buffer.
/// Returns true if the operation is successful; false otherwise.
//
bool TBufferedFile::Write(const void* buffer, uint32 numBytes)
{
  PRECONDITION(GetHandle());
  // if data size greater then buffer -> write directly
  if(numBytes > FileBufSize){
    if(!FlushBuffer()){
      WARNX(OwlFileSystem,1, 0,"TBufferedFile::Write() error FlushBuffer(), line: " << __LINE__);
       return false;
    }
     if(!TFile::Write(buffer, numBytes))
       return false;
     StartPos = CurPos = Position();
    CurByte = FileBuffer;
    EndPos = StartPos + FileBufSize;
    return true;
  }
  // else use buffer for output
  uint8* data = (uint8*)buffer;
  while(numBytes > 0L){
     uint freeSize = EndPos - CurPos;
    if( numBytes > freeSize ){
      memmove(CurByte, data, freeSize);
       numBytes   -= freeSize;
       data       += freeSize;
       CurPos     += freeSize;
       if(!TFile::Write(FileBuffer, FileBufSize)){
        WARNX(OwlFileSystem,1, 0,"TBufferedFile::Write() error Write(), line: " << __LINE__);
         return false;
      }
       StartPos   = CurPos;
      EndPos     = StartPos + FileBufSize;
      CurByte    = FileBuffer;
     }
     else{
      memmove(CurByte, data, numBytes);
       data       += numBytes;
      CurByte    += numBytes;
      CurPos     += numBytes;
      numBytes  = 0;
     }
  }
  return true;
}


//------------------------------------------------------------------------------
//
//  TTextFile
//  ~~~~~~~~~

//
/// Reads up to size characters and places them in buffer. Normal termination is
/// when a \\r\\n character sequence is encountered. The string in buffer is NULL
/// terminated. NULL is returned on failure; a pointer to buffer is returned on
/// success.
//
LPTSTR
TTextFile::GetString(LPTSTR buffer, uint32 size)
{
  PRECONDITION(GetHandle());

  LPTSTR str = buffer;
  LPTSTR Buff = (LPTSTR)CurByte;
  uint length = 0;
  if(BufferEmpty){
     StartPos = CurPos = Position();
    uint32 retval = TFile::Read(FileBuffer, FileBufSize);
    if(retval == TFILE_ERROR || retval == 0){
      WARNX(OwlFileSystem,retval==TFILE_ERROR,0,_T("TTextFile::GetString() error Read(), line: ") << __LINE__);
      return 0;
    }
     CurByte  = FileBuffer;
    EndPos = StartPos + retval;
     BufferEmpty = false;
  }

  while(*Buff && *Buff != _T('\r') && *Buff != _T('\n') && length < size){
    // if end -> fill buffer
    if(CurPos >= EndPos){
      CurByte = (uint8*)Buff;
      uint32 retval = TFile::Read(FileBuffer, FileBufSize);
      if((retval == TFILE_ERROR || retval == 0) && length==0){
        WARNX(OwlFileSystem,retval==TFILE_ERROR,0,_T("TTextFile::GetString() error Read(), line: ") <<__LINE__);
        return 0;
      }
      StartPos   = CurPos;
      EndPos     = StartPos + retval;
       CurByte    = FileBuffer;
      Buff       = (LPTSTR)CurByte;
      if(!retval)
        break;
    }
#if 0 //  check UNICODE support
    *str = *Buff;
    LPTSTR p = CharNext(str);
    length += p - str;
    str = p;
    p = CharNext(Buff);
    uint delta = p - Buff;
    CurPos   += delta;
    Buff     += delta;
#else
    *str++ = *Buff++;
    length++;
    CurPos++;
#endif
  }
  if(*Buff == _T('\r')){
    Buff++;
    CurPos++;
  }
  if(*Buff == _T('\n')){
    Buff++;
    CurPos++;
  }

  CurByte = (uint8*)Buff;

  // End string with 0
   *str = _T('\0');
  return (*CurByte || length) ? buffer : 0;
}

namespace
{

  //
  // For use with CopyText
  //
  struct TTextFileGetString
  {
    TTextFile& f;
    TTextFileGetString(TTextFile& f_) : f(f_) {}
    int operator()(LPTSTR buf, int bufSize)
    {
      LPTSTR r = f.GetString(buf, static_cast<uint32>(bufSize));
      return r ? bufSize - 1 : 0;
    }
  };

} // namespace

//
/// String-aware overload
//
tstring TTextFile::GetString(int size)
{
  return CopyText(size, TTextFileGetString(*this));
}


//
/// Writes the NULL terminated character string in buffer to the file and adds a
/// terminating \\r\\n character sequence. The NULL character is not written. On
/// success true is returned; false otherwise.
//
bool
TTextFile::WriteString(LPCTSTR buffer)
{
  PRECONDITION(GetHandle());

  size_t len = ::_tcslen(buffer) * sizeof(tchar);

  // TODO: Widen the parameter type for Write to avoid this check and the narrowing cast.
  //
  if (!IsRepresentable<uint32>(len))
    return false;
  bool status = TBufferedFile::Write(buffer, static_cast<uint32>(len)); 
  if(status)
    status  = TBufferedFile::Write(_T("\r\n"), 2);
  return status;
}


uint8  
TTextFile::readUint8()
{
  uint8 c;
  Read(&c, 1);
  return c;
}


uint16 
TTextFile::readUint16()
{
  return (uint16)readUint32();
}


uint32 
TTextFile::readUint32()
{
  TXNotSupportedCall::Raise();
  return 0;  //?????????????????????????
}
uint64 
TTextFile::readUint64()
{
  TXNotSupportedCall::Raise();
  return 0;  //?????????????????????????
}

float  
TTextFile::readFloat()
{
  return (float)readDouble();
}
double 
TTextFile::readDouble()
{
  TXNotSupportedCall::Raise();
  return 0; //?????????????????????????
}


void 
TTextFile::writeUint8( const uint8 u)
{
  Write(&u, 1);
}


void 
TTextFile::writeUint16( const uint16 u)
{
  writeUint32(u);
}


void 
TTextFile::writeUint32( const uint32 u)
{
  tchar buf[20];
#if defined __GNUC__
  _tprintf(buf, _T("%d"), u);
#else
  _ltot(u, buf, 10);
#endif
  Write(buf, static_cast<uint32>(::_tcslen(buf))); // TODO: Widen the paramteter type to avoid this narrowing cast.
  return;
}


void 
TTextFile::writeUint64( const uint64)
{
  TXNotSupportedCall::Raise();
  return; //?????????????????????????????
}
void 
TTextFile::writeFloat( const float f)
{
  writeDouble(f);
}
void 
TTextFile::writeDouble( const double)
{
  TXNotSupportedCall::Raise();
  return;//????????????????????????????????????????????
}


//
/// Constructor for iterating through file. The bufsize paramater is used for
/// setting the buffer size.
//
TFileLineIterator::TFileLineIterator(TTextFile& file, uint bufsize)
:
  File(&file),
  LineBuffer(0),
  BuffSize(bufsize),
  LineNumber(0),
  Done(false)
{
  LineBuffer = new tchar[BuffSize];
  Done = !NextLine();
}
//------------------------------------------------------------------------------
/// Deletes the buffer.
TFileLineIterator::~TFileLineIterator()
{
  delete[] LineBuffer;
}
//------------------------------------------------------------------------------
// all work do here -> must fill LineBuffer
//
/// Loads the next line in the file. Returns true if successful; false otherwise.
//
bool TFileLineIterator::NextLine()
{
  if(!File || !File->IsOpen())
    return false;
  return ToBool(File->GetString(LineBuffer, BuffSize) != 0);
}


//------------------------------------------------------------------------------

/// \cond

#include <pshpack1.h>
struct __TChunk {
  uint32  CkId;  // 'RIFF' or 'LIST'
  uint32  Size;  // data size
  uint32  Type;  // type for main chunk 'WAVE'
};
#include <poppack.h>

/// \endcond

//------------------------------------------------------------------------------
static TBinField ChunkField[] = {
  {varLONG, 4, 1}, /* FOURCC  CkId;  // 'RIFF' or 'LIST' */
  {varLONG, 4, 1}, /* data     size                       */
  {varEnd,   0, 0},
};
//------------------------------------------------------------------------------
static TBinField ListField[] = {
  {varLONG, 4, 1}, /* FOURCC  CkId;  // 'RIFF' or 'LIST' */
  {varLONG, 4, 1}, /* data     size                       */
  {varLONG, 4, 1}, /* type     for main chunk 'WAVE'      */
  {varEnd,   0, 0},
};
//------------------------------------------------------------------------------
// without first member
static TBinField ChunkField1[] = {
  {varLONG, 4, 1}, /* data     size                       */
  {varEnd,   0, 0},
};
//------------------------------------------------------------------------------
static TBinField ListField1[] = {
  {varLONG, 4, 1}, /* data     size                       */
  {varLONG, 4, 1}, /* type     for main chunk 'WAVE'      */
  {varEnd,   0, 0},
};
//------------------------------------------------------------------------------
/// Writes a data structure of the specified type to the associated file.
bool TRiffFile::CreateChunk(TCkInfo& info, const TCreateFlags type)
{
  CHECK(IsOpen());

  __TChunk chunk;
  chunk.CkId  = 0;
  TBinField* fields = ListField;

  info.Flags = TCkInfo::ckDirty;
  chunk.CkId   = info.CkId;  // type of Atom
  if(type == cfCreateList){
    chunk.CkId  = FOURCC_LIST;
    info.Flags |= TCkInfo::ckList;
  }
  else if(type == cfCreateRiff){
    chunk.CkId   = FOURCC_RIFF;
    info.Flags |= TCkInfo::ckRiff;
  }

  chunk.Type   = info.Type;  // type of Atom
  chunk.Size  = 0;          // data size not include size of atom

  if(type == cfCreateChunk)
    fields = ChunkField;

  if(!WriteStruct(&chunk, fields, boLittle_Endian)){
    WARNX(OwlFileSystem,1, 0,"TRiffFile::CreateChunk() error WriteStruct(), line: " << __LINE__);
    return false;
  }

  info.Offset = Position();
  if(type == cfCreateList || type == cfCreateRiff)
    info.Offset -= 4;

  return true;
}
//------------------------------------------------------------------------------
bool TRiffFile::Ascent(TCkInfo& info)
{
  CHECK(IsOpen());

  if(info.Flags & TCkInfo::ckDirty){
    info.Size = Position() - info.Offset;
    long offset = info.Offset - sizeof(__TChunk) + 4;
    if(Seek(offset, beg) == TFILE_ERROR){
      WARNX(OwlFileSystem,1, 0,"TRiffFile::Ascent() error Seek(), line: " << __LINE__);
      return false;
    }

    __TChunk chunk;
    chunk.CkId   = info.CkId;
    chunk.Type   = info.Type;
    chunk.Size  = info.Size;  // data size not include size of atom

    if(info.Flags & TCkInfo::ckList)
      chunk.CkId  = FOURCC_LIST;
    else if(info.Flags & TCkInfo::ckRiff)
      chunk.CkId   = FOURCC_RIFF;

    TBinField* fields = ChunkField;
    if(info.Flags & TCkInfo::ckList || info.Flags & TCkInfo::ckRiff)
      fields = ListField;

    if(!WriteStruct(&chunk, fields, boLittle_Endian)){
      WARNX(OwlFileSystem,1, 0,"TRiffFile::Ascent() error WriteStruct(), line: " << __LINE__);
      return false;
    }
    info.Flags = 0;
  }
  // seek to beginning the next atom if any
  long size = info.Offset + info.Size;
  if(Seek(size, beg) == TFILE_ERROR){
    WARNX(OwlFileSystem,1, 0,"TRiffFile::Ascent() error Seek(), line: " << __LINE__);
    return false;
  }

  return true;
}
/* -------------------------------------------------------------------------- */
bool TRiffFile::Descent(TCkInfo& info, TCkInfo* parent, const TFindFlags flags)
{
  CHECK(IsOpen());

  __TChunk chunk;

  if(flags & ffReset){
    if(parent){
      long offset = parent->Offset;
      if(parent->Flags & TCkInfo::ckList || parent->Flags & TCkInfo::ckRiff)
        offset += 4;
      if(Seek(offset, beg) == TFILE_ERROR){
        WARNX(OwlFileSystem,1, 0,"TRiffFile::Descent() error Seek(), line: " << __LINE__);
        return false;
      }
    }
    else // else if parent = 0
      if(SeekToBegin() == TFILE_ERROR){
        WARNX(OwlFileSystem,1, 0,"TRiffFile::Descent() error SeekToBegin(), line: " << __LINE__);
        return false;
      }
  }

  bool bRiff;
  bool done = false;
  bool retval = false;
  uint32 riff, size;
  TBinField* fields;
  TBinField longField[] = {  {varLONG, 4, 1},  {varEnd,   0, 0},};

  do{
    if(!ReadStruct(&riff, longField, boLittle_Endian)){
      WARNX(OwlFileSystem,1, 0,"TRiffFile::Descent() error ReadStruct(), line: " << __LINE__);
      return false;
    }

    fields = ChunkField1;
    chunk.CkId = riff;
    info.Flags  = 0;

    bRiff = false;
    if(riff == FOURCC_RIFF){
      info.Flags |= TCkInfo::ckRiff;
      bRiff = true;
      fields = ListField1;
    }
    else if(riff == FOURCC_LIST){
      info.Flags |= TCkInfo::ckList;
      bRiff = true;
      fields = ListField1;
    }

    if(!ReadStruct(&chunk.Size, fields, boLittle_Endian)){
      WARNX(OwlFileSystem,1, 0,"TRiffFile::Descent() error ReadStruct(), line: " << __LINE__);
      return false;
    }

    info.Offset = Position();
    if(bRiff)
      info.Offset -= 4;

    info.Size   = chunk.Size;

    // if any atom
    if(flags & ffFindAny){
      info.CkId   = chunk.CkId;
      info.Type   = bRiff ? chunk.Type : 0;
      retval      = true;
      break;
    }
    //if set flag FindChunk search it
    else if(flags & ffFindChunk && chunk.CkId == info.CkId){
      info.Type   = 0;
      retval      = true;
      break;
    }
    else if(flags & ffFindList && chunk.Type == info.Type){
      info.Type   = chunk.Type;
      info.CkId   = FOURCC_LIST;
      retval      = true;
      break;
    }
    else if(flags & ffFindRiff && chunk.CkId == FOURCC_RIFF){
      info.Type   = chunk.Type;
      info.CkId   = FOURCC_RIFF;
      retval      = true;
      break;
    }

    size = chunk.Size;
    // if list chunk exclude size one field
    if(bRiff)
      size -= 4;
    if(Seek((long)size, cur) == TFILE_ERROR){
      done     = true;
      retval  = true;
    }
  }
  while(!done);

  return retval;
}


/// \cond

#if !defined(qtstruct_h)
#include <pshpack1.h>
struct __MacAtom {
  uint32  Size;
  uint32   Type;
};
static TBinField MacAtomFields[] = {
  {varLONG, 4, 1},   /* uint32  Size;                            */
  {varCHAR, 1, 4},   /* uint32   Type; not swappable              */
  {varEnd,  0, 0},
};
#include <poppack.h>
#endif

/// \endcond

/* -------------------------------------------------------------------------- */
bool TQtFile::Descent(TQtInfo& info, TQtInfo* parent, const TFindFlags flags)
{
  CHECK(IsOpen());

  __MacAtom atom;

  uint32 position = 0;
  uint32 max_size;

  if(flags & ffReset){
    if(parent){
      if(Seek((long)parent->Offset, beg) == TFILE_ERROR){
        WARNX(OwlFileSystem,1, 0,"TQtFile::Descent() error seek line: " << __LINE__);
        return false;
      }
      max_size = parent->Size + parent->Offset;
      position = parent->Offset;
    }
    else{ // else if parent = 0
      max_size = Length();
      if(SeekToBegin() == TFILE_ERROR){
        WARNX(OwlFileSystem,1, 0,"TQtFile::Descent() error seek to begin line: " << __LINE__);
        return false;
      }
    }
  }
  else{
     position = Position();
    if(parent)
      max_size = parent->Size + parent->Offset - position;
    else // else if parent = 0
      max_size = Length();
  }
  bool done = false;
  bool status = false;

  do{
    if(ReadStruct(&atom, MacAtomFields, boBig_Endian)){
      if(atom.Type == info.Type){
        info.Size   = atom.Size;
        info.Offset = Position();
        info.Flags  = 0;
        status = true;
        done = true;
      }
      //if set flag FindChunk search it
      if(atom.Type != info.Type && flags & ffFindChunk){
        if (atom.Size==0 ||
            Seek(long(atom.Size-sizeof(atom)),cur)==TFILE_ERROR)
          done = true;
        position += atom.Size;
      }
      else
        done = true;
    }
    else
      done = true;
  }
  while(!done && position < max_size);

  return status;
}
//------------------------------------------------------------------------------
bool TQtFile::Ascent(TQtInfo& info)
{
  CHECK(IsOpen());
  if(info.Flags & TQtInfo::qtDirty){
    info.Size = Position() - info.Offset + sizeof(__MacAtom);
    long offset = info.Offset - sizeof(__MacAtom);
    if(Seek(offset, beg)==TFILE_ERROR){
      WARNX(OwlFileSystem,1, 0,"TQtFile::Ascent() error seek, line: " << __LINE__);
      return false;
    }

    __MacAtom atom;
    atom.Type  = info.Type;  // type of Atom
    atom.Size  = info.Size;  // data size include size of atom
    if(!WriteStruct(&atom, MacAtomFields, boBig_Endian)){
      WARNX(OwlFileSystem,1, 0,"TQtFile::Ascent() error write to file, line: " << __LINE__);
      return false;
    }
    info.Flags = 0;
  }
  // seek to beginning the next atom if any
  return ToBool(Seek(long(info.Size+info.Offset-sizeof(__MacAtom)), beg) != TFILE_ERROR);
}
// -----------------------------------------------------------------------------
/// Writes a data structure of the specified type to the associated file.
bool TQtFile::CreateChunk(TQtInfo& info)
{
  CHECK(IsOpen());

  __MacAtom atom;
  info.Flags = TQtInfo::qtDirty;
  atom.Type  = info.Type;  // type of Atom
  atom.Size  = 0;          // data size not include size of atom

  if(!WriteStruct(&atom, MacAtomFields, boBig_Endian)){
    WARNX(OwlFileSystem,1, 0,"TQtFile::CreateChunk() error write to file, line: " << __LINE__);
    return false;
  }

  info.Offset = Position();
  return ToBool(info.Offset != TFILE_ERROR);
}


// -----------------------------------------------------------------------------
static void sys2StrmMode(int& uMode, uint32 openMode)
{
  if(!(openMode & TFile::Text))
    uMode |= ios::binary;
  if(openMode & TFile::ReadWrite){
    uMode |= ios::in|ios::out;
    if((openMode&TFile::TruncateExist)==0 && 
       (openMode&TFile::CreateNew)==0)
      uMode |= ios::app;
  }
  else if(openMode & TFile::ReadOnly)
    uMode |= ios::in;
  else if(openMode & TFile::WriteOnly)
    uMode |= ios::out;

  if(openMode & TFile::CreateNew ||
     openMode & TFile::CreateAlways ||
     openMode & TFile::TruncateExist)
    uMode |= ios::trunc; // create
}
// -----------------------------------------------------------------------------
// class TStreamHandle
// ~~~~~ ~~~~~~~~~~~~~
//
//
TStreamHandle::TStreamHandle(TStreamFile* parent, const tstring& fileName, uint32 mode)
:
  Parent(parent),
  FileName(fileName),
  OpenMode(mode)
{
  // fstream
  int uMode = 0;
  sys2StrmMode(uMode, mode);
  _USES_CONVERSION;
  Parent->open(_W2A(fileName.c_str()), (ios::openmode)uMode);
}
//
uint32 TStreamHandle::LastError()
{
  return Parent->bad();
}
//
bool TStreamHandle::IsOpen()
{
  return Parent->rdbuf()->is_open();
}
//
TStreamHandle*
TStreamHandle::Clone() const
{
  TXNotSupportedCall::Raise();
  return 0;//????????????????????????????????
}
//
bool TStreamHandle::Close()
{
  if(Parent->rdbuf()->close() == 0)
    return false;
  return true;
}
//
uint32 TStreamHandle::Read(void * buffer, uint32 numBytes)
{
  Parent->read(static_cast<tchar*>(buffer), numBytes/sizeof(tchar));
  streamsize n = Parent->gcount();

  // Note: n cannot be greater than numBytes, so we can cast unchecked.
  // TODO: Widen the parameter and return type to avoid this narrowing cast.
  //
  return Parent->fail() == 0 ? numBytes : static_cast<uint32>(n);
}
//
bool TStreamHandle::Write(const void * buffer, uint32 numBytes)
{
  Parent->write((const tchar *)buffer, numBytes/sizeof(tchar));
  return Parent->fail()==0 ? true : false;
}
//
bool TStreamHandle::Length(uint64 newLen)
{
  return Length((uint32)newLen);
}
//
uint64 TStreamHandle::Position64()   const
{
  uint32 retval = Position();
   if(retval==TFILE_ERROR)
    return TFILE64_ERROR;
  return uint64(retval);
}
//
uint64 TStreamHandle::Length64()   const
{
  return uint64(Length());
}
//
bool TStreamHandle::Length(uint32 /*newLen*/)
{
//  if(chsize((uint)Handle->GetHandle(), (long)newLen) != TFILE_ERROR)
//    return true;
  TXNotSupportedCall::Raise();//?????????????????????????????
  return false;
}
//
uint32 TStreamHandle::Position() const
{
  streampos retval;
  if(OpenMode & TFile::WriteOnly)
    retval = Parent->tellp();
  else
    retval = Parent->tellg();
  if(retval == streampos(-1))
    return TFILE_ERROR;
  return (uint32)retval;
}
//
uint32 TStreamHandle::Length()   const
{
  if(OpenMode & TFile::WriteOnly){ 
    streampos retval = Parent->tellp();
    if(retval == streampos(-1))
      return TFILE_ERROR;
    return (uint32)retval;
  }
  uint32 last_pos = ((TStreamHandle*)this)->Seek(0l, TFile::end);
  uint32 pos = Position();
  ((TStreamHandle*)this)->Seek((long)last_pos, TFile::beg);
  return pos;
}
//
uint64 TStreamHandle::Seek(int64 offset, TFile::seek_dir origin)
{
  return uint64(Seek((long)offset, origin));
}
//
uint32 TStreamHandle::Seek(long offset, TFile::seek_dir origin)
{
  uint32 position  = Position();
# if defined(__BORLANDC__) && (__BORLANDC__ < 0x580)
//older BC compilers use ios::seek_dir instead of ios::seekdir
#  define seekdir seek_dir
# endif
  if(OpenMode & TFile::WriteOnly)
    Parent->seekp((streamoff)offset, (ios::seekdir)origin);
  else
    Parent->seekg((streamoff)offset, (ios::seekdir)origin);
# if defined(__BORLANDC__) && (__BORLANDC__ < 0x580) 
#  undef seekdir 
# endif
  return position;
}
//
bool TStreamHandle::Flush()
{
  if(OpenMode & TFile::WriteOnly)
    Parent->flush();
  return true;
}
//
bool TStreamHandle::LockRange(uint32 /*position*/, uint32 /*count*/)
{
  TXNotSupportedCall::Raise();
  return false;
}
//
bool TStreamHandle::UnlockRange(uint32 /*position*/, uint32 /*count*/)
{
  TXNotSupportedCall::Raise();
  return false;
}
//
//
bool TStreamHandle::LockRange(uint64 position, uint64 count)
{
  return LockRange((uint32)position, (uint32)count);
}
//
bool TStreamHandle::UnlockRange(uint64 position, uint64 count)
{
  return UnlockRange((uint32)position, (uint32)count);
}

bool TStreamHandle::GetStatus(TFileStatus  & /*status*/) const
{
  TXNotSupportedCall::Raise();
  return false;
}


// -----------------------------------------------------------------------------
//
// TStreamFile in work
// ~~~~~~~~~~~
//
//------------------------------------------------------------------------------
bool TStreamFile::Open(const tstring& fileName, const uint32 mode)
{
  if(IsOpen())
    return false;
  Handle = new TStreamHandle(this, fileName, mode);
  if(!Handle->IsOpen()){
    delete (TStreamFile*)Handle;
    Handle = FileNull;
  }
  return IsOpen();
}
//------------------------------------------------------------------------------

} // OWL namespace
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

