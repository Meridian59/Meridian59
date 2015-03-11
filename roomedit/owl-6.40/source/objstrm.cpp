//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/streambl.h>
#include <owl/objstrm.h>
#include <owl/private/memory.h>
#include <owl/template.h>
#include <owl/pointer.h>

#include <streambuf>
#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#include <owl/except.h>

#if defined(BI_COMP_GNUC)
# define open_mode openmode 
#endif

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

DIAG_DEFINE_GROUP(Objstrm,1,0);

const uint32 streamVersion = 0x0101;
const tchar versionIndicator = _T(':');
const tchar EOS = _T('\0');

const uint8  oldNullStringLen = UCHAR_MAX;
const uint32 nullStringLen = ULONG_MAX;


///////////////////////////////////////////////////////////////////////////////////////////
// multithread support
//
//
/// \cond
struct __TStreamableCont {
  static void RegisterType( ModuleId id, TStreamableClass& );
  static void UnRegisterType( ModuleId id, TStreamableClass& );
  static const ObjectBuilder * Lookup( ModuleId id, LPCSTR name );

  static void CreateTypes();
  static void DestroyTypes();

#if defined(BI_MULTI_THREAD_RTL)
  static TMRSWSection*  Lock;
#endif

  static TStreamableTypes*  Types;
};
/// \endcond

#if defined(BI_MULTI_THREAD_RTL)
TMRSWSection* __TStreamableCont::Lock = 0;
#endif
TStreamableTypes*  __TStreamableCont::Types = 0;

#if defined(BI_MULTI_THREAD_RTL)
#define LOCKTYPES(s) TMRSWSection::TLock __lock(*__TStreamableCont::Lock,s)
#else
#define LOCKTYPES(s)
#endif

void 
__TStreamableCont::RegisterType( ModuleId id, TStreamableClass& cls)
{
  PRECONDITION(Types);
  LOCKTYPES(false);
  Types->RegisterType(id,cls);
}
//
void __TStreamableCont::UnRegisterType( ModuleId id, TStreamableClass& cls)
{
  PRECONDITION(Types);
  {  
    LOCKTYPES(false);
    Types->UnRegisterType(id,cls);
  }
#if defined(BI_MULTI_THREAD_RTL)
  if(!Types)
  {
    delete Lock;
    Lock = 0;
  }
#endif
}
//
const ObjectBuilder * __TStreamableCont::Lookup( ModuleId id, LPCSTR name ) 
{
  PRECONDITION(Types);
  LOCKTYPES(true);
  return Types->Lookup(id, name);
}
//
void __TStreamableCont::CreateTypes()
{
#if defined(BI_MULTI_THREAD_RTL)
  if(!Lock)
  {
    Lock = new TMRSWSection;
  }
#endif    
  {
    LOCKTYPES(false);
    if(!Types)
	{
      Types = new TStreamableTypes;
    }
  }
}
//
void __TStreamableCont::DestroyTypes()
{
  // not locking because this functions is called from UnregisterTypes()
  // and Lock already applyed
  //LOCKTYPES 
  delete Types;
  Types = 0;
}
///////////////////////////////////////////////////////////////////////////////////////
class TStreamableBaseArray: public TPtrArray<TStreamableBase*>{
  public:
    TStreamableBaseArray(int a, int b=0, int c=0): TPtrArray<TStreamableBase*>(a,b,c){}
    TStreamableBaseArray(){}
};
//
class TStreamableClassArray: public TISortedPtrArray<TStreamableClass*>{
  public:
    TStreamableClassArray(int a, int b=0, int c=0): TISortedPtrArray<TStreamableClass*>(a,b,c){}
    TStreamableClassArray(){}

    int Add(TStreamableClass* t);
    bool DetachItem(TStreamableClass* t)
      {
        int index = Find(t);
        if(index != (int)NPOS){
          Remove(index);
          return true;
        }
        return false;
      }
    int Find(TStreamableClass* t) const;
  private:
    int    Add(const TStreamableClass* t);
    bool  DetachItem(const TStreamableClass* t);
    int    Find(const TStreamableClass* t) const;
};
int TStreamableClassArray::Add(TStreamableClass* t)
{
  if(ItemCnt>=Reserved)
     Resize(ItemCnt+1); // on error -> throw xalloc
  unsigned loc = ItemCnt++;
  while( loc > 0 && *t < *(Data[loc-1])) {
    Data[loc] = Data[loc-1];
    loc--;
   }
  Data[loc] = (TStreamableClass*)t;
  return loc;
}
int TStreamableClassArray::Find(TStreamableClass* t) const
{
  if( Size() == 0 )
    return (int)NPOS;
  unsigned lower = 0;
  unsigned upper = Size()-1;

  while( lower < upper && upper != NPOS ){
    unsigned middle = (lower+upper)/2;
    if(*(Data[middle]) == *t)
      return (int)middle;
    if(*(Data[middle]) < *t)
      lower = middle+1;
    else
      upper = middle-1;
  }
  if( lower == upper && *(Data[lower]) == *t)
    return (int)lower;
  return (int)NPOS;
}
//
class TSortedTPWObjObjectArray: public TSortedObjectArray<TPWrittenObjects::TPWObj>{
  public:
    TSortedTPWObjObjectArray(int a, int b=0, int c=0): TSortedObjectArray<TPWrittenObjects::TPWObj>(a,b,c){}
    TSortedTPWObjObjectArray(){}
};

// -----------------------------------------------------------------------------
const char* TStreamer::StreamableName() const
{
    return 0;
}
//----------------------------------------------------------------------------------------------
/// Creates a TStreamableClass object with the given name (n) and the given builder
/// function (b), then registers the type.
/// For example, each streamable has a Build member function of type BUILDER. For
/// type-safe object-stream I/O, the stream manager needs to access the names and
/// the type information for each class. To ensure that the appropriate functions
/// are linked into any application using the stream manager, you must provide a
/// reference such as:
/// \code
/// 	TStreamableClass RegClassName;
/// \endcode
/// where TClassName is the name of the class for which objects need to be streamed.
/// (Note that RegClassName is a single identifier.) This not only registers
/// TClassName (telling the stream manager which Build function to use), it also
/// automatically registers any dependent classes. You can register a class more
/// than once without any harm or overhead.
/// Invoke this function to provide raw memory of the correct size into which an
/// object of the specified class can be read. Because the Build procedure invokes a
/// special constructor for the class, all virtual table pointers are initialized
/// correctly.
/// The distance, in bytes, between the base of the streamable object and the
/// beginning of the TStreamableBase component of the object is d. Calculate d by
/// using the _ _DELTA macro.
/// Example
/// \code
/// TStreamableClass RegTClassName = TStreamableClass("TClassName",
/// TClassName::build, _ _DELTA(TClassName));
/// \endcode
TStreamableClass::TStreamableClass( const char * n,
                                    BUILDER b,
                                    int d,
                                    ModuleId id )
:
  ObjectBuilder( b, d ),
  ModId(id)
{
  ObjectId = strnewdup(n);
  pstream::initTypes();  
  if( id != 0 )   // id == 0 is used only during lookup.
                  // It flags an instance that shouldn't be registered
    __TStreamableCont::RegisterType( id, *this );
}

TStreamableClass::~TStreamableClass()
{
  if( ModId != 0 )
    __TStreamableCont::UnRegisterType( ModId, *this );
  delete [] CONST_CAST(char*, ObjectId);
}


TStreamableTypes::TStreamableTypes()
{
  Types = new TStreamableClassArray(30);
}


TStreamableTypes::~TStreamableTypes()
{
  delete Types;
}

void
TStreamableTypes::RegisterType( ModuleId, TStreamableClass& ts )
{
  PRECONDITION(Types);
  Types->Add(&ts);
}

void TStreamableTypes::UnRegisterType( ModuleId, TStreamableClass& ts )
{
  PRECONDITION(Types);
  Types->DetachItem(&ts);
  if(Types->IsEmpty())
    __TStreamableCont::DestroyTypes();
    //pstream::releaseTypes();
}

const ObjectBuilder *TStreamableTypes::Lookup( ModuleId, LPCSTR name ) const
{
  TStreamableClass sc(name,0,0,0);
  unsigned loc = Types->Find(&sc);

  if( loc == NPOS){
    _USES_CONVERSION;
    tstring msg;
    msg.reserve(128);
    msg = _T("Attempt to stream unregistered type '");
    msg += _A2W(name);
    msg += _T("'");
    TXBase(msg).Throw();
  }
  return (*Types)[loc];
}

void TPReadObjects::RemoveAll()
{
  // delete all objects and flush container
  Data->Flush();
}

void TPReadObjects::RegisterObject( TStreamableBase *adr )
{
  //if(Data->Find( adr ) == (int)NPOS)
  Data->Add( adr );
}

TStreamableBase* TPReadObjects::Find( P_id_type id )
{
  if(id < Data->Size())
    return (*Data)[id];
  return 0;
}

TPReadObjects::TPReadObjects()
{
  Data = new TStreamableBaseArray;
  Data->Add(0);  // prime it: 0 is not a legal index.
}
TPReadObjects::~TPReadObjects()
{
  RemoveAll();
  delete Data;
}

TPWrittenObjects::TPWrittenObjects()
:
  CurId(0)
{
  Data = new TSortedTPWObjObjectArray(10);
}
TPWrittenObjects::~TPWrittenObjects()
{
  delete Data;
}

void TPWrittenObjects::RemoveAll()
{
  CurId = 0;
  Data->Flush();
}

void TPWrittenObjects::RegisterObject( TStreamableBase *adr )
{
  Data->Add(TPWObj(((tchar*)(void*)adr)+1, ++CurId));
}

void TPWrittenObjects::RegisterVB( const TStreamableBase *adr )
{
  Data->Add(TPWObj(adr,++CurId));
}

P_id_type TPWrittenObjects::FindObject( TStreamableBase *d )
{
  unsigned res = Data->Find(TPWObj(((tchar*)(void*)d)+1,0));
  if (res == NPOS)
    return 0;
  else
   return (*Data)[res].Ident;
}

P_id_type TPWrittenObjects::FindVB( TStreamableBase *d )
{
  unsigned res = Data->Find(TPWObj(d,0));
  if (res == NPOS)
    return 0;
  else
  return (*Data)[res].Ident;
}

pstream::~pstream()
{
}

void pstream::initTypes()
{
  __TStreamableCont::CreateTypes();
}

void pstream::releaseTypes()
{
  LOCKTYPES(false);
  __TStreamableCont::DestroyTypes();
}

void pstream::registerType( TStreamableClass *ts )
{
  __TStreamableCont::RegisterType( GetModuleId(), *ts );
}

/// Returns the (absolute) current stream position.
streampos ipstream::tellg()
{
  streampos res;
  if( !good() )
    res = streampos(EOF);
  else{
    res = bp->pubseekoff(0, ios::cur, ios::in );
    if( res == streampos(EOF) )
      clear( ios::failbit );
  }
  return res;
}

/// Moves the stream position to the absolute position given by pos.
ipstream& ipstream::seekg( streampos pos )
{
  if( good() ){
    objs.RemoveAll();
    streampos p = bp->pubseekoff(pos, ios::beg, ios::in );
    if( p == streampos(EOF) )
      clear( ios::failbit );
  }
  return *this;
}

/// Moves to a position relative to the current position by an offset off (+
/// or -) starting at ios::seek_dir. You can set ios::seek_dir to one of the
/// following:
/// - \c \b  	beg (start of stream)
/// - \c \b  	cur (current stream position)
/// - \c \b  	end (end of stream).
ipstream& ipstream::seekg( streamoff off, ios::seek_dir dir )
{
  if( good() ){
    objs.RemoveAll();
#if __GNUC__ >=3
//In GCC ios_base::seek_dir is deprecated by ios_base::seekdir (both exists but cast is needed)
    streampos p = bp->pubseekoff(off, (ios_base::seekdir)dir, ios::in );
#else
    streampos p = bp->pubseekoff((ios::off_type)off, dir, (ios::open_mode)ios::in );
#endif
    if( p == streampos(EOF) )
      clear( ios::failbit );
  }
  return *this;
}

/// Returns the byte at the current stream position.
uint8 ipstream::readByte()
{
  int res;
  if( !good() )
      res = uint8(0);
  else{
    res = bp->sbumpc();
    if( res == EOF )
     clear( ios::failbit );
  }
  return uint8(res);
}

/// Reads sz bytes from current stream position, and writes them to data.
void ipstream::readBytes( void *data, size_t sz )
{
    PRECONDITION( data != 0 );
    if( good() && sz > 0 )
        {
        if( bp->sgetn( (char*)data, sz ) != static_cast<int>(sz) )
            clear( ios::failbit );
        }
}

/// Reads the number of bytes specified by sz into the supplied buffer (data).
void ipstream::freadBytes( void *data, size_t sz )
{
  PRECONDITION( data != 0 );

  if( good() && sz > 0){
    TTmpBuffer<char> buf(sz);

    if( bp->sgetn( (char*)buf, sz ) != static_cast<int>(sz))
      clear( ios::failbit );
    else
      memcpy( data, (char*)buf, sz);
  }
}

/// Returns the word at the current stream position.
uint32 ipstream::readWord()
{
  if( getVersion() > 0 )
    return readWord32();
  else
    return readWord16();
}

/// Returns the 16-bit word at the current stream position.
uint16 ipstream::readWord16()
{
  if( !good() )
    return 0;
  else{
    uint16 temp;
    if( bp->sgetn( (char*)&temp, sizeof( temp ) ) != 
        static_cast<int>(sizeof( temp )) )
      clear( ios::failbit );
    return temp;
  }
}

/// Returns the 32-bit word at the current stream position.
uint32 ipstream::readWord32()
{
  if( !good() )
    return 0;
  else{
    uint32 temp;
    if( bp->sgetn( (char*)&temp, sizeof( temp ) ) != 
        static_cast<int>(sizeof( temp )) )
      clear( ios::failbit );
    return temp;
  }
}

uint32 ipstream::readStringLength()
{
  uint32 len;
  if( getVersion() > 0x0100 ){
    len = readWord32();
  }
  else{
    len = readByte();
    if( len == oldNullStringLen )
      len = nullStringLen;
  }
  return len;
}

/// Allocates a buffer large enough to contain the string at the current
/// stream position and reads the string into the buffer. The caller must free the
/// buffer.
char* ipstream::readString()
{
  if( !good() )
    return 0;
  else{
    uint32 len = readStringLength();
    if( len == nullStringLen )
      return 0;

    char* buf = new char[size_t(len+1)];
    if( buf == 0 )
      return 0;
    readBytes( buf, size_t(len) );
    buf[size_t(len)] = EOS;
    return buf;
  }
}

/// Reads the string at the current stream position into the buffer
/// specified by buf. If the length of the string is greater than maxLen - 1, it
/// reads nothing. Otherwise, it reads the string into the buffer and appends a
/// null-terminating byte.
LPSTR ipstream::readString( LPSTR buf, unsigned maxLen )
{
  PRECONDITION( buf != 0 );

  if( !good() )
    return 0;
  else{
    uint32 len = readStringLength();
    if( len == nullStringLen || len > maxLen-1 )
      return 0;
    readBytes( buf, size_t(len) );
    buf[size_t(len)] = EOS;
    return buf;
  }
}

/// Reads a string from the stream. It determines the length of the string
/// and allocates a character array of the appropriate length. It reads the
/// string into this array and returns a pointer to the string. The caller is
/// expected to free the allocated memory block.
char *ipstream::freadString()
{
  if( !good() )
    return 0;
  else{
    uint32 len = readStringLength();
    if( len == nullStringLen )
      return 0;

    char* buf = new char[size_t(len)+1];
    freadBytes(buf, size_t(len));
    buf[size_t(len)] = EOS;
    return buf;
  }
}

/// Reads a string from the stream into the supplied far buffer (buf). If
/// the length of the string is greater than maxLen-1, it reads nothing. Otherwise,
/// it reads the string into the buffer and appends a null-terminating byte.
char* ipstream::freadString( char *buf, unsigned maxLen )
{
  PRECONDITION(buf != 0 );

  if( !good() )
    return 0;
  else{
    uint32 len = readStringLength();
    if( len == nullStringLen || len > maxLen-1 )
      return 0;

    freadBytes( buf, size_t(len));
    buf[size_t(len)] = EOS;
    return buf;
  }
}

/// Reads the version number of the input stream.
void ipstream::readVersion()
{
  if( !good() )
    version = 0;
  else{
    int res = bp->sgetc();
    if( res == EOF ){
      clear( ios::eofbit );
      version = 0;
      return;
    }
    if( res != versionIndicator )
      version = 0;
    else{
      bp->sbumpc();
      version = readWord32();
    }
  }
}

TStreamableBase *ipstream::readObject( TStreamableBase *&mem,
                                            ModuleId mid )
{
  if( good() ){
    const ObjectBuilder *pc = readPrefix( mid );
    if( pc == 0 )
      mem = 0;
    else{
      readData( pc, mem );
      readSuffix();
    }
  }
  return mem;
}

/// Returns the TStreamableClass object corresponding to the class name stored at
/// the current position in the stream.
const ObjectBuilder *ipstream::readPrefix( ModuleId mid )
{
  char ch = readByte();
  if( ch != '[' ){
    clear( ios::failbit );
    return 0;
  }

  char name[128];
  name[0] = EOS;
  readString( name, sizeof name );
  if( name[0] == EOS ){
    clear( ios::failbit );
    return 0;
  }

  _USES_CONVERSION;

  TRACEX(Objstrm,0,_T("Reading ") << _A2W(name));
  const ObjectBuilder *res = __TStreamableCont::Lookup( mid, name );

  WARNX(Objstrm,res==0,0,_T("Unrecognized class identifier: ") << _A2W(name));
  if( res == 0 ){
    clear( ios::failbit );
    return 0;
  }

  return res;
}

/// If mem is 0, it calls the appropriate build function to allocate memory and
/// initialize the virtual table pointer for the object.
/// Finally, it invokes the appropriate read function to read the object from the
/// stream into the memory pointed to by mem.
void ipstream::readData( const ObjectBuilder *c, TStreamableBase *&mem )
{
  TPointer<TStreamer> strmr(c->Builder(mem));
  mem = strmr->GetObject();

  // register the address
  registerObject( mem );

  uint32 classVer = 0;
  if( getVersion() > 0 )
    classVer = readWord32();

  strmr->Read( *this, classVer );
}

/// Reads and checks the suffix of the object.
void ipstream::readSuffix()
{
  if( !good() )
    return;
  char ch = readByte();
  if( ch != ']' )
    clear( ios::failbit );
}

TStreamableBase *ipstream::readObjectPointer( TStreamableBase *&mem,
                                                   ModuleId mid )
{
  if( !good() )
    return 0;

  char ch = readByte();
  switch( ch ){
    case pstream::ptNull:
      mem = 0;
      break;
    case pstream::ptIndexed:{
        P_id_type index = P_id_type(readWord());
        mem = find( index );
        CHECK( mem != 0 );
        break;
      }
    case pstream::ptObject: {
        const ObjectBuilder *pc = readPrefix( mid );
        readData( pc, mem );
        readSuffix();
        break;
      }
    default:
      clear( ios::failbit );
      break;
  }
  return mem;
}

/// Creates a buffered opstream with the given buffer and sets the bp data
/// member to buf. The state is set to 0.
opstream::opstream()
{
  objs = new TPWrittenObjects;
  if( bp != 0 )
    writeVersion();
}

/// Creates an opstream object without initializing the buffer pointer, bp.
/// Use pstream::init to set the buffer and state.
opstream::opstream( streambuf * sb )
{
  objs = new TPWrittenObjects;
  pstream::init( sb );
  writeVersion();
}

/// Returns the (absolute) current stream position.
streampos opstream::tellp()
{
  streampos res;
  if( !good() )
    res = streampos(EOF);
  else{
    res = bp->pubseekoff(0, ios::cur, ios::out );
    if( res == streampos(EOF) )
      clear( ios::failbit );
  }
  return res;
}

/// Moves the current position of the stream to the absolute position given by pos.
opstream& opstream::seekp( streampos pos )
{
  if( good() ){
    objs->RemoveAll();
#if __GNUC__ >=3
//In GCC ios_base::seek_dir is deprecated by ios_base::seekdir 
    streampos p = bp->pubseekoff(pos, ios::beg, ios::out);
#else
    streampos p = bp->pubseekoff((ios::off_type)pos, (ios::seek_dir)ios::beg, 
                                 (ios::open_mode)ios::out);
#endif
    if( p == streampos(EOF) )
      clear( ios::failbit );
  }
  return *this;
}

/// Form 2: Moves to a position relative to the current position by an offset off (+
/// or -) starting at ios::seek_dir. You can set ios::seek_dir to one if the
/// following:
/// - \c \b  beg (start of stream)
/// - \c \b  cur (current stream position)
/// - \c \b  end (end of stream).
opstream& opstream::seekp( streamoff off, ios::seek_dir dir )
{
  if( good() ){
    objs->RemoveAll();
#if __GNUC__ >=3
//In GCC ios_base::seek_dir is deprecated by ios_base::seekdir 
    streampos p = bp->pubseekoff(off, (ios::seekdir)dir, ios::out );
#else
    streampos p = bp->pubseekoff((ios::off_type)off, dir, (ios::open_mode)ios::out );
#endif
    if( p == streampos(EOF) )
      clear( ios::failbit );
  }
  return *this;
}

void opstream::writeVersion()
{
  if( good() ){
    writeByte( versionIndicator );
    writeWord32( streamVersion );
  }
}

/// Flushes the stream.
opstream& opstream::flush()
{
  if( bp->pubsync() == EOF )
    clear( ios::badbit );
  return *this;
}

/// Writes the byte ch to the stream.
void opstream::writeByte( uint8 ch )
{
  if( good() ){
    if( bp->sputc( ch ) == EOF )
      clear( ios::failbit );
  }
}

/// Writes sz bytes from the data buffer to the stream.
void opstream::writeBytes( const void *data, size_t sz )
{
  PRECONDITION( data != 0 );

  if( good() && sz > 0 ){
    if( bp->sputn( (char*)data, sz ) != static_cast<int>(sz) )
      clear( ios::failbit );
  }
}

/// Writes the 16-bit word us to the stream.
void opstream::writeWord16( uint16 word16 )
{
  if( good() ){
    if( bp->sputn( (char*)&word16, sizeof(word16) ) != 
        static_cast<int>(sizeof(word16)) )
      clear( ios::failbit );
  }
}

/// Writes the 32-bit word us to the stream.
void opstream::writeWord32( uint32 word32 )
{
  if( good() ){
    if( bp->sputn( (char*)&word32, sizeof(word32) ) != 
        static_cast<int>(sizeof(word32)) )
      clear( ios::failbit );
  }
}

/// Writes the specified number of bytes (sz) from the supplied buffer (data) to the
/// stream.
void opstream::fwriteBytes( const void *data, size_t sz )
{
  PRECONDITION( data != 0 );

  if( good() && sz > 0 ){
    char* buf = new char[sz];

    memcpy( buf, data, sz );
    if( bp->sputn( (char*)buf, sz ) != static_cast<int>(sz) )
      clear( ios::failbit );

    delete[] buf;
  }
}

/// Writes str to the stream.
void opstream::writeString( const char* str )
{
  if( !good() )
    return;

  if( str == 0 ){
    writeWord32( nullStringLen );
    return;
  }
  size_t len = strlen( str );
  writeWord32( len );
  writeBytes( str, len );
}

/// Writes the specified far character string (str) to the stream.
void opstream::fwriteString( const char * str )
{
  if( !good() )
    return;

  if( str == 0 ){
    writeWord32( nullStringLen );
    return;
  }
  size_t len = strlen( str );
  writeWord32( len );
  fwriteBytes(str, len);
}

#if !defined(BI_COMP_GNUC)
#pragma warn -par
#endif
/// Writes the object, pointed to by t, to the output stream. The isPtrargument
/// indicates whether the object was allocated from the heap.
void opstream::writeObject( const TStreamableBase* mem, int IFDIAG(isPtr), ModuleId mid )
{
  _USES_CONVERSION;
  WARNX(Objstrm,
        !isPtr && findObject( CONST_CAST(TStreamableBase *,mem) ),
        0,
        _T("Pointer written before object: ")     \
           << _A2W(_OBJ_FULLTYPENAME(mem)) << _T('(') << (void*)mem << _T(')') );
  if( good() ){
    writePrefix( mem );
    writeData( mem, mid );
    writeSuffix( mem );
  }
}
#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

/// Writes the object pointer t to the output stream.
void opstream::writeObjectPointer( const TStreamableBase *t, ModuleId mid )
{
  if( good() ){
    P_id_type index;
    if( t == 0 )
      writeByte( pstream::ptNull );
    else if( (index = findObject( CONST_CAST(TStreamableBase *,t) )) != 0 ){
      writeByte( pstream::ptIndexed );
      writeWord( index );
    }
    else{
      writeByte( pstream::ptObject );
      writeObject( t, 1, mid );
    }
  }
}

/// Writes the class name prefix to the stream. The << operator uses this function
/// to write a prefix and suffix around the data written with writeData. The
/// prefix/suffix is used to ensure type-safe stream I/O.
void opstream::writePrefix( const TStreamableBase *t )
{
  if( good() ){
    writeByte( '[' );
    writeString( _OBJ_TYPENAME(t) );
  }
}

/// Writes data to the stream by calling the write member function of the
/// appropriate class for the object being written.
void opstream::writeData( const TStreamableBase *t, ModuleId mid )
{
  _USES_CONVERSION;
  if( good() ){
    registerObject( CONST_CAST(TStreamableBase *,t) );
    const ObjectBuilder *res = __TStreamableCont::Lookup( mid, _OBJ_TYPENAME(t));
    CHECKX(res, _A2W(_OBJ_FULLTYPENAME(t)));
    TPointer<TStreamer> strmr(res->Builder(CONST_CAST(TStreamableBase *,t)));
    writeWord32( strmr->ClassVersion() );
    strmr->Write( *this );
  }
}

/// Opens the named file in the given mode (app, ate, in, out, binary, trunc,
/// nocreate, or noreplace) and protection. The opened file is attached to this
/// stream.
void fpbase::open( LPCSTR b, int m, int)
{
  if (buf.is_open())
    clear(ios::failbit);
  else if (buf.open(b, static_cast<ios::openmode>(m)))
    clear(ios::goodbit);
  else
    clear(ios::badbit);
}

#if defined(BI_HAS_STREAMWCHAROPEN)
/// Opens the named file in the given mode (app, ate, in, out, binary, trunc,
/// nocreate, or noreplace) and protection. The opened file is attached to this
/// stream.
void fpbase::open(LPCWSTR b, int m, int)
{
  if (buf.is_open())
    clear(ios::failbit);
  else if (buf.open(b, static_cast<ios::openmode>(m)))
    clear(ios::goodbit);
  else
    clear(ios::badbit);
}
#endif

/// Closes the stream and associated file.
void fpbase::close()
{
  if( buf.close() )
    clear(ios::goodbit);
  else
    clear(ios::failbit);
}

/// Sets the location of the buffer to buf and the buffer size to len.
void fpbase::setbuf(LPSTR b, int len)
{
  if( buf.pubsetbuf(b, len) )
    clear(ios::goodbit);
  else
    clear(ios::failbit);
}

//
//  These operators are not friends of string, so
//  they must use only the public interface.
//
_OWLCFUNC(opstream& ) operator << ( opstream& os, const tstring& str)
{
  _USES_CONVERSION;
  os.writeString( _W2A(str.c_str()) );
  return os;
}

/// This operator of ipstream extracts (reads) from the ipstream is, to the string
/// str. It returns a reference to the stream that lets you chain >> operations in
/// the usual way.
_OWLCFUNC(ipstream& ) operator >> ( ipstream& is, tstring& str )
{
  if( is.good() ){
    uint32 len = is.readStringLength();
    if( len == nullStringLen )
      str = _T("");
    else{
      char *temp = new char[size_t(len)+1];
      is.readBytes( temp, size_t(len) );
      temp[size_t(len)] = EOS;
      _USES_CONVERSION;
      str = _A2W(temp);
      delete [] temp;
    }
  }
  return is;
}

} // OWL namespace
/* ========================================================================== */
