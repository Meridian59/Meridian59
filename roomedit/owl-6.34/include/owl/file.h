//----------------------------------------------------------------------------
// ObjectWindwos
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
// Fully rewritten by Yura Bidus
//----------------------------------------------------------------------------

#if !defined(OWL_FILE_H)
#define OWL_FILE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/time.h>
#include <owl/date.h>

#include <owl/gdibase.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WINELIB
//#include <msvcrt/share.h>
//#include <msvcrt/dos.h>
#include <sys/io.h>

#else

#if !defined(__CYGWIN__)
#include <share.h>
#include <dos.h>
#endif
#include <io.h>

#endif //WINELIB

#if defined(BI_COMP_MSC) 
# include <sys/locking.h>
#endif
#include <owl/private/strmdefs.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup utility
/// @{


//------------------------------------------------------------------------------
//
/// \struct TFileStatus
// ~~~~~~ ~~~~~~~~~~~
// 
struct TFileStatus {
    TTime   createTime;
    TTime   modifyTime;
    TTime   accessTime;
    uint32  size;
    uint    attribute;
    tchar   fullName[_MAX_PATH];
};

_OWLCFUNC(tostream &)  operator << (tostream &, const TFileStatus &);

//
/// The byte order type.
enum TByteOrderType {
  boLittle_Endian,  ///< LSB at lowest address: Intel    //
  boBig_Endian,     ///< MSB at lowest address: Motorola //
};

//
//
//
inline TByteOrderType EndianType()
{
  uint16 w=0x0001;
  uint8 *b=(uint8*)&w;
  return (b[0] ? boLittle_Endian : boBig_Endian);
}

//------------------------------------------------------------------------------
/// These identify host program variable types and let the binary I/O package read
/// and write routines know how to store or retrieve bytes from host buffer arrays
/// and when to sign-extend.
//
enum TVarType{
  varEnd,
  varPOINTER,
  varCHAR,
  varUCHAR,
  varSHORT,
  varLONG,
  varINT8,
  varINT16,
  varINT32,
  varUSHORT,
  varULONG,
  varUINT8,
  varUINT16,
  varUINT32,
  varFLOAT,
  varDOUBLE,
  varINT64,
  varUINT64,
  varLastMember,
};
#define BINNTYPE  (varLastMember+1)


//----------------------------------------------------------------------------
//  TBinField  -  Binary I/O Structure Field Description                       
//                                                                            
/// The TBinField struct describes a group of like-typed fields in a structure to be
/// read or written using the Binary I/O Struct calls.
//
struct TBinField {
  TVarType   Type;    ///< Declared type of struct field
  int       Bytes;  ///< # of bytes occupied in file  
  int       Count;  ///< # of repeatitions of this item
};


#define TFILE_ERROR uint32(-1)

/// Represents a NULL file handle.
#define FileNull 0

#define TFILE64_ERROR uint64(0xFFFFFFFF)

class _OWLCLASS TFileHandle; 

//------------------------------------------------------------------------------
//
// class TFile
// ~~~~~ ~~~~~
/// The TFile class encapsulates standard file characteristics and operations.
//
class _OWLCLASS TFile {
  public:
    enum seek_dir {
      beg = 0,		///< Seek from the beginning of the file.
      cur = 1,		///< Seek from the current position in the file.
      end = 2			///< Seek from the end of the file.
    };

    /// Open mode -> remapped into OS specific value internally
    enum OpenMode{
      ReadOnly      =   0x0001,
      WriteOnly     =   0x0002,
      ReadWrite     =   WriteOnly|ReadOnly,

			/// Subsequent open operations on the object will succeed only if write access is requested.
      PermWrite     =   0x0010, 
      
			/// Subsequent open operations on the object will succeed only if read access is requested.
      PermRead      =   0x0020, 
      
      PermReadWrite =   PermWrite|PermRead, //
      PermExclusive =   0x0040, //
      PermNone      =   0x0080, //

			/// Creates a new file. The function fails if the specified file already exists.
      CreateNew     =   0x0100,
      
			/// Creates a new file. The function overwrites the file if it exists.
      CreateAlways  =   0x0200,
      
			/// Opens the file. The function fails if the file does not exist.      
      OpenExisting  =   0x0400,
      
			/// Opens the file. Once opened, the file is truncated so that its size is zero
			/// bytes. The calling process must open the file with at least WriteOnly
			/// access. The function fails if the file does not exist.
      TruncateExist =   0x0800,

      Text          =   0x1000, ///< type Text are used in derived classes only
                                ///< default type is Binary
    };

    /// file attributes -> internally remapped into OS values
    enum Attribute {
      Normal =    0x00,
      RdOnly =    0x01,
      Hidden =    0x02,
      System =    0x04,
      Volume =    0x08,
      Directory = 0x10,
      Archive =   0x20,
      Temporary = 0x40,
    };

		/// Binary data type enumerations.
    enum BinType {
      TypeChar,
      TypeShort,
      TypeLong,
      TypeFloat,
      TypeDouble,
      TypeLong64,
      TypePoint, // == TypeSize,
      TypeRect,
    };

    TFile();
    TFile(const tstring& fileName, const uint32 mode = ReadOnly|PermRead|OpenExisting);
    TFile(TFileHandle* handle, bool shouldClose);
    // if TFileHandle not support Clone() will throw TXNotSupportedCall()
    TFile(const TFile & file);
     virtual ~TFile();

    virtual TFileHandle* GetHandle() const;
    const tstring GetName() const;

    virtual bool    Open(const tstring& fileName, const uint32 mode = ReadOnly|PermRead|OpenExisting);
    virtual bool    Close();

    virtual bool    Length(uint64 newLen);
    virtual uint64   Position64()   const;
    virtual uint64   Length64()   const;
    virtual bool     Length(uint32 newLen);
    virtual uint32   Position() const;
    virtual uint32   Length()   const;

    virtual uint64   Seek(int64 offset, seek_dir origin = beg);
    virtual uint32   Seek(long offset, seek_dir origin = beg);

    uint64          SeekToBegin64();
    uint64           SeekToEnd64();
    uint32           SeekToBegin();
    uint32           SeekToEnd();

    bool            IsOpen() const;
    uint32          GetOpenMode() const;
    uint32          LastError();

    virtual uint32   Read(void * buffer, uint32 numBytes);
    virtual bool      Write(const void * buffer, uint32 numBytes);
    virtual bool    Flush();

    uint            ReadStruct(void * buffer, BinType btype, TByteOrderType type);
    uint            ReadStruct(void * buffer, TBinField * fields,
                               TByteOrderType type);
    static   uint    ReadStruct(uint8 * readBuf, void * buffer,
                              TBinField * fields, TByteOrderType type);
    uint            WriteStruct(void * buffer, BinType btype, TByteOrderType type);
    uint             WriteStruct(void * buffer, TBinField * fields,
                                TByteOrderType type);
    static   uint    WriteStruct(uint8 * writeBuf, void * buffer,
                               TBinField * fields, TByteOrderType type);

    static  uint    StructSize(TBinField * fields);

    virtual bool    LockRange(uint32 position, uint32 count);
    virtual bool    UnlockRange(uint32 position, uint32 count);
    virtual bool    LockRange(uint64 position, uint64 count);
    virtual bool    UnlockRange(uint64 position, uint64 count);

#if defined(OWL5_COMPAT) // must be moved to TFileName
    // don't use, this functions call right to TFileName
    static bool     GetStatus(LPCTSTR name, TFileStatus & status);
    // don't use, this functions call right to TFileName
    static bool     SetStatus(LPCTSTR name, const TFileStatus & status);
#endif
    bool            GetStatus(TFileStatus & status) const;

    // streaming support (internal functions)
    virtual uint8    readUint8();
    virtual uint16    readUint16();
    virtual uint32    readUint32();
    virtual uint64    readUint64();
    virtual float     readFloat();
    virtual double    readDouble();
    virtual LPTSTR    readString( tchar  *);

    virtual void  writeUint8( const uint8 );
    virtual void  writeUint16( const uint16 );
    virtual void  writeUint32( const uint32 );
    virtual void  writeUint64( const uint64 );
    virtual void  writeFloat( const float );
    virtual void  writeDouble( const double );
    virtual void  writeString( const tchar  *);

  protected:
    TFileHandle*  Handle;      ///< Low-level C file handle
    bool           ShouldClose; ///< Should C++ object close file on dtor

    // read/write structure helpers
    
/// Buffer used with structure read/write.
    uint8*    Buffer;
    
/// Size of Buffer used with structure read/write.
    uint      BufSize;
};

//------------------------------------------------------------------------------
// Abstract class
//
// class TFileHandle
// ~~~~~ ~~~~~~~~~~~
//
class _OWLCLASS TFileHandle {
  friend class _OWLCLASS TFile;
  protected:
    TFileHandle(){}
    virtual ~TFileHandle(){}
  public:
    virtual uint    GetOpenMode() = 0;
    virtual const tstring GetName()  = 0;
    virtual uint32  LastError() = 0;

    virtual bool    IsOpen()  = 0;
    virtual TFileHandle* Clone() const = 0;
    virtual bool     Close() = 0;
    virtual uint32   Read(void * buffer, uint32 numBytes) = 0;
    virtual bool     Write(const void * buffer, uint32 numBytes) = 0;
    virtual bool    Length(uint64 newLen) = 0;
    virtual uint64   Position64()   const = 0;
    virtual uint64   Length64()   const = 0;
    virtual bool     Length(uint32 newLen) = 0;
    virtual uint32   Position() const = 0;
    virtual uint32   Length()   const = 0;
    virtual uint64   Seek(int64 offset, TFile::seek_dir origin = TFile::beg) = 0;
    virtual uint32   Seek(long offset, TFile::seek_dir origin = TFile::beg) = 0;
    virtual bool     Flush() = 0;
    virtual bool     LockRange(uint32 position, uint32 count) = 0;
    virtual bool     UnlockRange(uint32 position, uint32 count) = 0;
    virtual bool     LockRange(uint64 position, uint64 count) = 0;
    virtual bool     UnlockRange(uint64 position, uint64 count) = 0;
    virtual bool    GetStatus(TFileStatus  & status) const = 0;

  private: 
    TFileHandle(const TFileHandle& handle);
};
///////////////////////////////////
//
// class TDiskFileHandle
// ~~~~~ ~~~~~~~~~~~~~~~
//
class _OWLCLASS TDiskFileHandle: public  TFileHandle {
  friend class _OWLCLASS TFileName;
  public:
    TDiskFileHandle(const tstring& fileName, uint32 mode);

    TDiskFileHandle* Clone() const;

    virtual uint    GetOpenMode() { return OpenMode; }
    virtual const tstring GetName() { return FileName; }
    virtual uint32 LastError();
    virtual bool    IsOpen()  { return Handle != INVALID_HANDLE_VALUE; }
    
    virtual bool     Close();
    virtual uint32   Read(void * buffer, uint32 numBytes);
    virtual bool     Write(const void * buffer, uint32 numBytes);
    virtual bool    Length(uint64 newLen);
    virtual uint64   Position64()   const;
    virtual uint64   Length64()   const;
    virtual bool     Length(uint32 newLen);
    virtual uint32   Position() const;
    virtual uint32   Length()   const;
    virtual uint64   Seek(int64 offset, TFile::seek_dir origin = TFile::beg);
    virtual uint32   Seek(long offset, TFile::seek_dir origin = TFile::beg);
    virtual bool     Flush();
    virtual bool     LockRange(uint32 position, uint32 count);
    virtual bool     UnlockRange(uint32 position, uint32 count);
    virtual bool     LockRange(uint64 position, uint64 count);
    virtual bool     UnlockRange(uint64 position, uint64 count);
    virtual bool    GetStatus(TFileStatus  & status) const;

  protected:
    TDiskFileHandle(HANDLE handle, const tstring& fileName, uint mode);
    HANDLE      Handle;
    uint        OpenMode;
    tstring  FileName;

  private:
    TDiskFileHandle(const TDiskFileHandle& handle);
};

//------------------------------------------------------------------------------
//
// class TBufferedFile
// ~~~~~ ~~~~~~~~~~~~~
/// The TBufferedFile class is derived from TFile encapsulates standard file
/// characteristics and operations using buffered I/O.
//
class _OWLCLASS TBufferedFile : public TFile{
  public:
/// The initial size of the buffer.
    enum { DefaultBufferSize = 2048};

    TBufferedFile(); 
    TBufferedFile(const tstring& fileName, const uint32 mode = ReadOnly|PermRead|OpenExisting);
    // if TFileHandle isn't support Clone() will be throw
    TBufferedFile(const TBufferedFile & file);
    TBufferedFile(TFileHandle * handle, bool shouldClose);
     
    virtual ~TBufferedFile();

    virtual bool Close();
    virtual bool    Length(uint64 newLen);
    virtual uint64   Position64()   const;
    virtual uint64   Length64()   const;
    virtual uint32   Position() const;
    virtual uint32   Length()   const;
    virtual bool    Length(uint32 newLen);

    virtual uint64   Seek(int64 offset, seek_dir origin = beg);
    virtual uint32   Seek(long offset, seek_dir origin = beg);

    virtual uint32   Read(void * buffer, uint32 numBytes);
    virtual bool      Write(const void* buffer, uint32 numBytes);
    virtual bool     Flush();

    // if buffer == 0, allocate memory
    virtual void     SetBuffer(uint8* buffer, uint size, TAutoDelete autodelete = AutoDelete);

  protected:

    virtual void InitBuffer(uint size = DefaultBufferSize);
    virtual bool FlushBuffer();

  protected:
/// Buffer used to store data in.
    uint8*    FileBuffer;
    
/// True if the buffer should be deleted.
    bool      ShouldDelete;
    
/// Size of FileBuffer in bytes.
    uint      FileBufSize;

/// Pointer to current position in the buffer.
    uint8*    CurByte;
    
/// Offset in file to current position.
    uint32    CurPos;
    
/// Offset in file to byte 0 of the buffer.
    uint32    StartPos;
    
/// Offset in file to the last data byte in the buffer.
    uint32    EndPos;
    
/// True if the buffer is empty; false otherwise.
    bool       BufferEmpty;
};

//------------------------------------------------------------------------------
//
/// The TTextFile class is derived from TBufferedFile and encapsulates standard file
/// characteristics and operations using text based buffered I/O.
//
class _OWLCLASS TTextFile : public TBufferedFile{
  public:
    TTextFile();
    TTextFile(const TTextFile & file);
    TTextFile(TFileHandle* handle, bool shouldClose);
    TTextFile(const tstring& fileName, const uint32 mode = ReadOnly|PermRead|OpenExisting|Text);

    virtual LPTSTR   GetString(LPTSTR buffer, uint32 size);
    tstring GetString(int size);
    virtual bool     WriteString(LPCTSTR str);
    bool WriteString(const tstring& str) {return WriteString(str.c_str());}

    // streaming support (internal functions)
    virtual uint8    readUint8();
    virtual uint16    readUint16();
    virtual uint32    readUint32();
    virtual uint64    readUint64();
    virtual float     readFloat();
    virtual double    readDouble();
    virtual LPTSTR    readString( tchar  *);

    virtual void  writeUint8( const uint8 );
    virtual void  writeUint16( const uint16 );
    virtual void  writeUint32( const uint32 );
    virtual void  writeUint64( const uint64 );
    virtual void  writeFloat( const float );
    virtual void  writeDouble( const double );
    virtual void  writeString( const tchar  *);
};

//------------------------------------------------------------------------------
//
// TFileLineIterator
//
/// The TFileLineIterator class is used to iterate through a TTextFile file.
//
class _OWLCLASS TFileLineIterator {
  public:
    TFileLineIterator(TTextFile& file, uint bufsize = 260);
    virtual ~TFileLineIterator();

    const tchar* operator *() const;
    operator const tchar*() const;
    const tchar* operator ++();
    const tchar* Current() const;
    const uint    Line() const;

  protected:
    TFileLineIterator();
    // all work do here -> must fill LineBuffer
    virtual bool NextLine();

  protected:
/// Pointer to the file being iterated through.
    TTextFile*  File;
    
/// Buffer lines of text are loaded into.
    LPTSTR       LineBuffer;

/// Size of the buffer allocated for loading a line of text.
    uint        BuffSize;
    
/// Current line number in buffer. Line numbering starts at 1.
    uint        LineNumber;
    
    bool        Done;
/// Set true when another line cannot be loaded.
};

//------------------------------------------------------------------------------
//
/// \class TXBadFormat
// ~~~~~ ~~~~~~~~~~~~~
/// The TXBadFormat class is used for throwing exceptions when a bad file format is
/// encountered.
//
class _OWLCLASS TXBadFormat : public TXOwl {
  public:
    TXBadFormat();

    virtual TXBadFormat* Clone() const; // override
    void   Throw();

    static void Raise();
};

// structures for manipulating RIFF headers
#if !defined(owlFCC)
#  if 0
#    define owlFCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |    \
                       (((DWORD)(ch4) & 0xFF00) << 8) |    \
                       (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                       (((DWORD)(ch4) & 0xFF000000) >> 24))
#  else
//#  define extFCC(ch4) ((DWORD)(ch4))
#    define owlFCC(ch0, ch1, ch2, ch3)                                \
      ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |          \
      ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#  endif
#endif

// standard four character codes //
#if !defined(FOURCC_RIFF)
#  define FOURCC_RIFF     owlFCC('R','I','F','F')
#  define FOURCC_LIST     owlFCC('L','I','S','T')
#endif

//------------------------------------------------------------------------------
//
/// RIFF chunk information data structure
//
struct TCkInfo {
  enum ckFlags {
          ckDirty = 0x0001,
          ckList   = 0x0002,
          ckRiff   = 0x0004,
        };

  TCkInfo();

  uint32    CkId;    ///< chunk ID                         
  uint32    Size;   ///< chunk size                       
  uint32    Type;   ///< form type or list type           
  uint32    Offset; ///< offset of data portion of chunk   
  uint32    Flags;  ///< flags used by MMIO functions     
};

//------------------------------------------------------------------------------
//
// TRiffFile        read/write RIFF files
// ~~~~~~~~~
/// The TRiffFile class is used for reading and writing RIFF files.
//
class _OWLCLASS TRiffFile : public TBufferedFile {
  public:
    enum TFindFlags {
      ffReset      = 0x0001,
      ffFindAny    = 0x0002,
      ffFindChunk  = 0x0004,
      ffFindList   = 0x0008,
      ffFindRiff   = 0x0010,
    };
    enum TCreateFlags{
      cfCreateChunk,
      cfCreateList,
      cfCreateRiff,
    };

  public:
    TRiffFile();
    TRiffFile(const TRiffFile & file);
    TRiffFile(TFileHandle* handle, bool shouldClose);
    TRiffFile(const tstring& fileName, const uint32 acess = ReadOnly|PermRead|OpenExisting);

    bool  CreateChunk(TCkInfo& info, const TCreateFlags = cfCreateChunk);
    bool  Ascent(TCkInfo& info);
    bool  Descent(TCkInfo& info, TCkInfo* parent = 0, const TFindFlags = ffFindAny);
};

//------------------------------------------------------------------------------
//
/// QuickTime atom information data structure
//
struct TQtInfo {
  enum qtFlags {qtDirty  = 0x0001,};  ///< only dirty flag currently (for internal using)

  TQtInfo();

  uint32  Size;        //
  uint32   Type;        ///< chunk identifier
  uint32   Offset;      //
  uint32   Flags;       //
};

//------------------------------------------------------------------------------
//
// TQtFile
// ~~~~~~~
//
/// The TQtFile class is used for reading and writing QuickTime files.
//
class _OWLCLASS TQtFile: public TBufferedFile {
  public:
    enum TFindFlags {
      ffFindAny   = 0x0000,
      ffReset     = 0x0001,
      ffFindChunk = 0x0002,
    };
  public:
    TQtFile();
    TQtFile(const TQtFile & file);
    TQtFile(TFileHandle * handle, bool shouldClose);
    TQtFile(const tstring& fileName, const uint32 mode = ReadOnly|PermRead|OpenExisting);

    bool Ascent(TQtInfo & info);
    bool Descent(TQtInfo & info, TQtInfo* parent = 0, 
                 const TFindFlags = TFindFlags(ffFindChunk|ffReset));
    bool CreateChunk(TQtInfo & info);
};


//------------------------------------------------------------------------------
// Not finished
// class TStreamFile
// ~~~~~ ~~~~~
#  if defined(UNICODE)
    //using std::wfstream;
    typedef std::wfstream _tfstream;
#  else
    //using std::fstream;
    typedef std::fstream _tfstream;
#  endif

// class TFileHandle
// ~~~~~ ~~~~~~~~~~~
//
class TStreamFile;
class _OWLCLASS TStreamHandle: public TFileHandle {
  friend class TStreamFile;
  protected:
    TStreamHandle(TStreamFile* parent, const tstring& filename, uint32 mode);
    virtual ~TStreamHandle(){}

  public:
    virtual uint GetOpenMode() { return OpenMode; }
    virtual const tstring GetName() { return FileName; }
    virtual uint32  LastError();

    virtual bool    IsOpen();

    TStreamHandle* Clone() const;

    virtual bool     Close();
    virtual uint32   Read(void * buffer, uint32 numBytes);
    virtual bool     Write(const void * buffer, uint32 numBytes);
    virtual bool    Length(uint64 newLen);
    virtual uint64   Position64()   const;
    virtual uint64   Length64()   const;
    virtual bool     Length(uint32 newLen);
    virtual uint32   Position() const;
    virtual uint32   Length()   const;
    virtual uint64   Seek(int64 offset, TFile::seek_dir origin = TFile::beg);
    virtual uint32   Seek(long offset, TFile::seek_dir origin = TFile::beg);
    virtual bool     Flush();
    virtual bool     LockRange(uint32 position, uint32 count);
    virtual bool     UnlockRange(uint32 position, uint32 count);
    virtual bool     LockRange(uint64 position, uint64 count);
    virtual bool     UnlockRange(uint64 position, uint64 count);
    virtual bool    GetStatus(TFileStatus  & status) const;

  protected:
    TStreamFile*  Parent;
    uint          OpenMode;
    tstring    FileName;
};

///////////////////////////////////////////////////////////////
// class TStreamFile
// ~~~~~ ~~~~~~~~~~~
//
// class _OWLCLASS TStreamFile
// fatal error C1001: INTERNAL COMPILER ERROR  (compiler file 'f:\vs70builds\3077\vc\Compiler\Utc\src\P2\p2symtab.c', line 4533)
class TStreamFile : public TFile, public _tfstream {
  public:
    TStreamFile();
    TStreamFile(TStreamFile & file);
    TStreamFile(TStreamHandle * handle, bool shouldClose);
    TStreamFile(const tstring& filename, const uint32 mode = ReadOnly|PermRead|OpenExisting);
    virtual ~TStreamFile(){}

    virtual bool Open(const tstring& fileName, const uint32 mode = ReadOnly|PermRead|OpenExisting);
};
//------------------------------------------------------------------------------

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//---------------------------------------------------------------------------
// Inlines


//
/// Creates a TFile object with a file handle of FileNull.
//
inline TFile::TFile()
: 
  Handle(FileNull),ShouldClose(false),Buffer(0),BufSize(0)
{
}

//
/// Creates a TFile object and opens file name with the given attributes.
//
inline TFile::TFile(const tstring& fileName, const uint32 mode)
:
  Handle(FileNull),  ShouldClose(true), Buffer(0), BufSize(0)
{
  Open(fileName, mode);
}

//
/// Creates a TFile object with a file handle of handle. Set shouldClose true if the
/// file should be closed on deletion 
/// \note this function can only be used within file.cpp.
//
inline TFile::TFile(TFileHandle* handle, bool shouldClose)
:  
  Handle(handle), ShouldClose(shouldClose), Buffer(0), BufSize(0)
{
}
//
/// Returns Handle.
inline TFileHandle* TFile::GetHandle() const {
  return Handle;
}
//
/// Returns Name.
inline const tstring TFile::GetName() const {
  PRECONDITION(Handle);
  return Handle->GetName();
}
//
/// Returns true if the file is open, false otherwise.
//
inline bool TFile::IsOpen() const {
  return Handle != FileNull;
}
//
/// Returns OpenMode.
inline uint32 TFile::GetOpenMode() const {
  PRECONDITION(Handle);
  return Handle->GetOpenMode();
}
//
/// Repositions the file pointer to the beginning of the file. Returns the position
/// moved to or TFILE_ERROR on error. To get extended error information, call
/// LastError.
//
inline uint32 TFile::SeekToBegin() {
  return Seek(0l, beg);
}
//
/// Repositions the file pointer to the end of the file. Returns the position moved
/// to or TFILE_ERROR on error. To get extended error information, call LastError.
inline uint32 TFile::SeekToEnd(){
  return Seek(0l, end);
}
//
/// Repositions the file pointer to the beginning of the file. Returns
/// the position moved to or TFILE64_ERROR on error. To get extended error
/// information, call LastError.
inline uint64 TFile::SeekToBegin64(){
  return Seek((int64)0, beg);
}
//
/// Repositions the file pointer to the end of the file. Returns the
/// position moved to or TFILE64_ERROR on error. To get extended error information,
/// call LastError.
inline uint64 TFile::SeekToEnd64(){
  return Seek((int64)0, end);
}

//
/// Returns the last error.
/// 
/// Under Windows 9x/NT this is a call to GetLastError().
//
inline uint32 TFile::LastError(){
  PRECONDITION(Handle);
  return Handle->LastError();
}

//
/// Reads numBytes from the file into buffer. The number of bytes read is returned.
inline uint32 TFile::Read(void* buffer, uint32 numBytes){
  PRECONDITION(Handle);
  return Handle->Read(buffer, numBytes);
}

//
/// Writes numbytes of buffer to the file. Returns true if the operation is
/// successful; false otherwise.
inline bool TFile::Write(const void* buffer, uint32 numBytes){
  PRECONDITION(Handle);
  return Handle->Write(buffer, numBytes);
}

//
/// Returns the current position of the file pointer. Returns TFILE_ERROR to
/// indicate an error. To get extended error information, call LastError.
inline uint32 TFile::Position() const{
  PRECONDITION(Handle);
  return Handle->Position();
}

//
/// Returns the file length.
inline uint32 TFile::Length() const{
  PRECONDITION(Handle);
  return Handle->Length(); 
}

//
/// Resizes file to newLen. Returns true if successful, false otherwise. The file
/// must first be opened for success.
inline bool TFile::Length(uint32 newLen){
  PRECONDITION(Handle);
  return Handle->Length(newLen);
}

//
/// Repositions the file pointer to offset bytes from the specified origin. Returns
/// the position moved to or TFILE_ERROR on error. To get extended error
/// information, call LastError.
inline uint32 TFile::Seek(long offset, seek_dir origin){
  PRECONDITION(Handle);
  return Handle->Seek(offset, origin);
}

//
/// Resizes file to newLen. Returns true if successful, false
/// otherwise. The file must first be opened for success.
inline bool TFile::Length(uint64 newLen){
  PRECONDITION(Handle);
  return Handle->Length(newLen);
}
//
/// 32 Bit Only: Returns the current position of the file pointer. Returns
/// TFILE64_ERROR to indicate an error. To get extended error information, call
/// LastError.
inline uint64 TFile::Position64() const{
  PRECONDITION(Handle);
  return Handle->Position64();
}
//
/// Returns the file length.
inline uint64 TFile::Length64() const {
  PRECONDITION(Handle);
  return Handle->Length64();
}
//
/// Repositions the file pointer to offset bytes from the specified
/// origin. Returns the position moved to or TFILE64_ERROR on error. To get extended
/// error information, call LastError.
inline uint64 TFile::Seek(int64 offset, seek_dir origin)
{
  PRECONDITION(Handle);
  return Handle->Seek(offset, origin);
}
//
/// Locks count bytes, beginning at position of the file. Returns true
/// if successful; false otherwise.
inline bool TFile::LockRange(uint64 position, uint64 count)
{
  PRECONDITION(Handle);
  return Handle->LockRange(position, count);
}
//
/// 32 Bit Only: Unlocks the range at the given Position. Returns true if
/// successful; false otherwise.
inline bool TFile::UnlockRange(uint64 position, uint64 count)
{
  PRECONDITION(Handle);
  return Handle->UnlockRange(position, count);
}
//
/// Fills status with the current file status. Returns true if successful, false
/// otherwise.
inline bool TFile::GetStatus(TFileStatus  & status) const
{
  PRECONDITION(Handle);
  return Handle->GetStatus(status);
}
//
// streaming support
inline LPTSTR  TFile::readString( tchar * str){
  // this version just read 256 characters ??? for now
  if(Read(str, 256) == TFILE_ERROR)
    return 0;
  return str;
}
//
inline void  TFile::writeString( const tchar  * str){
  Write(str, static_cast<int>(::_tcslen(str)));
}
// inline stream operators
inline TFile&  operator >> ( TFile& file, int8& c){
  c = (int8)file.readUint8();
  return file;
}
//
inline TFile&  operator >> ( TFile& file, uint8& c){
  c = (uint8)file.readUint8();
  return file;
}
//
inline TFile&  operator >> ( TFile& file, int16& i){
  i = (int16)file.readUint16();
  return file;
}
//
inline TFile&  operator >> ( TFile& file, uint16& i){
  i = file.readUint16();
  return file;
}
//
inline TFile&  operator >> ( TFile& file, signed int& i){
  i = (signed int)file.readUint32();
  return file;
}
//
inline TFile&  operator >> ( TFile& file, unsigned int& i){
  i = (uint)file.readUint32();
  return file;
}
//
inline TFile&  operator >> ( TFile& file, bool& b){
  b = static_cast<bool>(file.readUint32());
  return file;
}
//
inline TFile&  operator >> ( TFile& file, int32& i){
  i = (int32)file.readUint32();
  return file;
}
inline TFile&  operator >> ( TFile& file, uint32& i){
  i = file.readUint32();
  return file;
}
inline TFile&  operator >> ( TFile& file, int64& i){
  i = (int64)file.readUint64();
  return file;
}
inline TFile&  operator >> ( TFile& file, uint64& i){
  i = file.readUint64();
  return file;
}
inline TFile&  operator >> ( TFile& file, float& f){
  f = file.readFloat();
  return file;
}
inline TFile&  operator >> ( TFile& file, double& d){
  d = file.readDouble();
  return file;
}
inline TFile&  operator >> ( TFile& file, tchar * str){
  file.readString(str);
  return file;
}
// writing support
inline TFile&  operator << ( TFile& file, int8 c){
  file.writeUint8((uint8)c);
  return file;
}
inline TFile&  operator << ( TFile& file, uint8 c){
  file.writeUint8(c);
  return file;
}
inline TFile&  operator << ( TFile& file, int16 i){
  file.writeUint16((int16)i);
  return file;
}
inline TFile&  operator << ( TFile& file, uint16 i){
  file.writeUint16((uint16)i);
  return file;
}
inline TFile&  operator << ( TFile& file, signed int i){
  file.writeUint32((signed int)i);
  return file;
}
inline TFile&  operator << ( TFile& file, unsigned int i){
  file.writeUint32((uint)i);
  return file;
}
inline TFile&  operator << ( TFile& file, bool b){
  file.writeUint32((uint32)b);
  return file;
}
inline TFile&  operator << ( TFile& file, int32 i){
  file.writeUint32((int32)i);
  return file;
}
inline TFile&  operator << ( TFile& file, uint32 i){
  file.writeUint32((uint32)i);
  return file;
}
inline TFile&  operator << ( TFile& file, int64 i){
  file.writeUint64(*(uint64*)&i);
  return file;
}
inline TFile&  operator << ( TFile& file, uint64 i){
  file.writeUint64((uint64)i);
  return file;
}
inline TFile&  operator << ( TFile& file, float f){
  file.writeFloat(f);
  return file;
}
inline TFile&  operator << ( TFile& file, double d){
  file.writeDouble(d);
  return file;
}
inline TFile&  operator << ( TFile& file, const tchar  * s){
  file.writeString(s);
  return file;
}
inline TFile&  operator << ( TFile& file, const tstring& s){
  size_t size = s.length();
  file.writeUint32(static_cast<int>(size));
  file.Write(s.c_str(), static_cast<int>(size));
  return file;
}

//
// TBufferedFile

//
/// Creates a TBufferedFile object with a file handle of FileNull and allocated a
/// buffer of DefaultBufferSize bytes.
//
inline TBufferedFile::TBufferedFile(){
  InitBuffer();
}
inline TBufferedFile::TBufferedFile(const TBufferedFile & file)
:  TFile(file)
{
  InitBuffer();
}
//
/// Creates a TBufferedFile object with a file handle of handle. Set shouldClose
/// true if the file should be closed on deletion 
/// \note this function can only be used within file.cpp.
//
inline TBufferedFile::TBufferedFile(TFileHandle* handle, bool shouldClose)
:  TFile(handle, shouldClose)
{
  InitBuffer();
}
//
/// Creates a TBufferedFile object and opens file name with the given attributes.
//
inline TBufferedFile::TBufferedFile(const tstring& fileName, const uint32 mode)
:  TFile(fileName, mode)
{
  InitBuffer();
}
//
/// Flushes the buffer and closes the file. Returns true if successful, false
/// otherwise.
//
inline bool 
TBufferedFile::Close()
{
  if(FlushBuffer())
    return TFile::Close();
  return false;
}
/// Flushes the buffer and then resizes file to newLen. Returns true if
/// successful, false otherwise. The file must first be opened for success.
inline bool  TBufferedFile::Length(uint64 newLen){
  if(FlushBuffer())
    return TFile::Length(newLen);
  return false;
}
/// Returns the file length plus the length of unwritten data in the buffer.
inline uint64 TBufferedFile::Length64() const{
  return TFile::Length64()+uint64(CurPos-StartPos);
}
/// Returns the current position of the file pointer. Returns TFILE64_ERROR 
/// to indicate an error. To get extended error information, call LastError.
inline uint64  TBufferedFile::Position64() const{
  return uint64(CurPos);
}
/// Returns the current position of the file pointer. Returns TFILE_ERROR to 
/// indicate an error. To get extended error information, call LastError.
inline uint32  TBufferedFile::Position() const{
  return CurPos;
}
/// Returns the file length plus the length of unwritten data in the buffer.
inline uint32  TBufferedFile::Length() const{
  return TFile::Length()+(CurPos-StartPos);
}
/// Flushes the buffer and then resizes file to newLen. Returns true if successful,
/// false otherwise. The file must first be opened for success.
inline bool  TBufferedFile::Length(uint32 newLen){
  if(FlushBuffer())
    return TFile::Length(newLen);
  return false;
}
//
/// Writes any data in the buffer to file and then resets the buffer. Returns true
/// if successful; false otherwise.
//
inline bool  TBufferedFile::Flush(){
  if(FlushBuffer())
    return TFile::Flush();
  return false;
}

//
// TTextFile

//
/// Creates a TTextFile object with a file handle of FileNull and allocated a buffer
/// of DefaultBufferSize bytes.
//
inline TTextFile::TTextFile(){
}
//
/// Creates a TTextFile object with a file handle of handle. Set shouldClose true if
/// the file should be closed on deletion 
/// \note this function can only be used within file.cpp.
//
inline TTextFile::TTextFile(TFileHandle* handle, bool shouldClose)
: TBufferedFile(handle, shouldClose)
{
}
//
/// Creates a TTextFile object and opens file name with the given attributes.
//
inline TTextFile::TTextFile(const tstring& fileName, const uint32 mode)
: TBufferedFile(fileName, mode)
{
}
inline TTextFile::TTextFile(const TTextFile & file)
: TBufferedFile(file)
{
}
inline LPTSTR  TTextFile::readString( tchar * str){
  return GetString(str, MAX_PATH);
}
inline void  TTextFile::writeString( const tchar * str){
  Write(str,static_cast<int>(::_tcslen(str)));
}

//
// TFileLineIterator

//
/// Protected default constructor. Intializes everything to 0.
//
inline TFileLineIterator::TFileLineIterator()
: File(0),LineBuffer(0),LineNumber(0),Done(false)
{
}
//
/// Returns a pointer to the start of the buffer if the file has not been completely
/// iterated through; otherwise returns 0.
//
inline const tchar* TFileLineIterator::operator *() const{
  return Done ? 0 : LineBuffer;
}
//
/// Returns a pointer to the start of the buffer if the file has not been completely
/// iterated through; otherwise returns 0.
//
inline TFileLineIterator::operator const tchar*() const{
  return Done ? 0 : LineBuffer;
}
//
/// Returns a pointer to the start of the buffer if the file has not been completely
/// iterated through; otherwise returns 0.
//
inline const tchar* TFileLineIterator::Current() const{
  return Done ? 0 : LineBuffer;
}
//
/// Loads the next line in the file and then returns a pointer to the start of the
/// buffer if the file has not been completely iterated through; otherwise returns 0.
//
inline const tchar* TFileLineIterator::operator ++()
{
  Done = !NextLine();
  if(!Done){
    LineNumber++;
    return LineBuffer;
  }
  return 0;
}
//
/// Returns the current line number.
//
inline const uint TFileLineIterator::Line() const{
  return LineNumber;
}

//
// TCkInfo
//
inline TCkInfo::TCkInfo()
: CkId(0),Size(0),Type(0),Offset(0),Flags(0)
{
}
//
// TRiffFile
//

/// Default constructor. Creates a TRiffFile object with a file handle of FileNull
/// and allocated a buffer of DefaultBufferSize bytes.
inline TRiffFile::TRiffFile()
: TBufferedFile()
{
}
/// Creates a TRiffFile object with a filename name and opens the file with mode
/// mode.
inline TRiffFile::TRiffFile(const tstring& fileName, const uint32 mode)
: TBufferedFile(fileName, mode)
{
}
inline TRiffFile::TRiffFile(const TRiffFile & file)
: TBufferedFile(file)
{
}
inline TRiffFile::TRiffFile(TFileHandle* handle, bool shouldClose)
: TBufferedFile(handle,shouldClose)
{
}
//
// TQtInfo
//
inline TQtInfo::TQtInfo()
: Size(0),Type(0),Offset(0),Flags(0)
{
}

//
// TQtFile
//
/// Default constructor. Creates a TQtFile object with a file handle of FileNull and
/// allocated a buffer of DefaultBufferSize bytes.
inline TQtFile::TQtFile()
: TBufferedFile()
{
}
/// Creates a TQtFile object with a filename name and opens the file with mode mode.
inline TQtFile::TQtFile(const tstring& fileName, const uint32 openMode)
: TBufferedFile(fileName, openMode)
{
}
inline TQtFile::TQtFile(const TQtFile & file)
: TBufferedFile(file)
{
}
inline TQtFile::TQtFile(TFileHandle* handle, bool shouldClose)
: TBufferedFile(handle,shouldClose)
{
}
//------------------------------------------------------------------------------
inline TStreamFile::TStreamFile()
{
}
inline TStreamFile::TStreamFile(TStreamHandle* handle, bool shouldClose)
: TFile(handle, shouldClose)
{
}
inline TStreamFile::TStreamFile(const tstring& filename, const uint32 mode)
{
  Open(filename, mode);
}
//------------------------------------------------------------------------------

} // OWL namespace


#endif  // OWL_FILE_H
