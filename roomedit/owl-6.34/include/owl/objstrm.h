//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
// Revision by Yura Bidus
//
//----------------------------------------------------------------------------

#if !defined(OWL_OBJSTRM_H)
#define OWL_OBJSTRM_H

#include <owl/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/streambl.h>


#include <iostream>
#include <fstream>

#include <typeinfo>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup base
/// @{

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  The __link() macro forces the compiler to link in a static instance   */
/*  of class TStreamableClass, which in turn registers its associated     */
/*  TStreamable object with the stream manager.  Applications that do     */
/*  not use streaming won't use __link(), and that will reduce the amount */
/*  of code that is linked into the application.                          */
/*                                                                        */
/*------------------------------------------------------------------------*/

/// \cond
struct fLink
{
    struct fLink *f;
    class TStreamableClass *t;
};

#define __link( s )             \
  extern TStreamableClass s;    \
  static fLink force ## s =     \
    { (fLink *)&force ## s, (TStreamableClass *)&s };
/// \endcond

typedef unsigned P_id_type;

class _OWLCLASS _RTTI TStreamable;
class _OWLCLASS TStreamableTypes;
class _OWLCLASS opstream;
class _OWLCLASS ipstream;



_OWLCFUNC(ipstream &) operator >> ( ipstream & is, tstring & str );

_OWLCFUNC(opstream &) operator << ( opstream & os,
                                          const tstring & str );

/* -----------------------------------------------------------------------*/
/*                                                                        */
/*  _OBJ_TYPENAME(obj) provides a macro for getting the type name from a  */
/*  pointer to an object. If runtime type information is available, this  */
/*  macro uses the typeid of the object to get its name. If runtime type  */
/*  information is not available, it uses the CastableID() for the object.*/
/*                                                                        */
/* -----------------------------------------------------------------------*/


#define _OBJ_FULLTYPENAME(obj) typeid(*obj).name()
#if defined(BI_COMP_MSC) && !defined(UNIX) && !defined(__EDG__)
#  if defined(BI_COMPATIBLE_STREAMING)
#    define _OBJ_TYPENAME(obj) typeid(*obj).name() + 6
#  else
#    define _OBJ_TYPENAME(obj) typeid(*obj).raw_name()
#  endif
#else
#  define _OBJ_TYPENAME(obj) typeid(*obj).name()
#endif
/* -----------------------------------------------------------------------*/
/*                                                                        */
/*! \ class TStreamable                                                     */
/*                                                                        */
/*!  This is the base class from which all streamable objects must be      */
/*!  derived.                                                              */
/*                                                                        */
/* -----------------------------------------------------------------------*/


enum StreamableInit { streamableInit };

/// Classes that inherit from TStreamableBase are known as streamable classes (their
/// objects can be written to and read from streams). If you develop your own
/// streamable classes, make sure that TStreamableBase is somewhere in their
/// ancestry.
/// 
/// Using an existing streamable class as a base is the easiest way to create a
/// streamable class. If your class must also fit into an existing class hierarchy,
/// you can use multiple inheritance to derive a class from TStreamableBase .
class _OWLCLASS _RTTI TStreamableBase {
  public:
    virtual  ~TStreamableBase();

};


class _OWLCLASS _RTTI TStreamable : public TStreamableBase {

    friend class _OWLCLASS _RTTI TOldStreamer;

  protected:

    virtual LPCSTR  streamableName() const = 0;

    virtual void *  read( ipstream& ) = 0;
    virtual void  write( opstream& )  = 0;

};

//
/// Provides a base class for all streamable objects.
class _OWLCLASS _RTTI TStreamer {

    friend class ipstream;
    friend class opstream;

  public:

/// Returns the address of the TStreamableBase component of the streamable object.
    TStreamableBase *  GetObject() const { return object; }

  protected:

/// Constructs the TStreamer object, and initializes the streamable object pointer.
    TStreamer( TStreamableBase *obj ) : object(obj) {}

/// This pure virtual member function must be redefined for every streamable class.
/// It returns the name of the streamable class, which is used by the stream manager
/// to register the streamable class. The name returned must be a zero-terminated
/// string.
    virtual LPCSTR  StreamableName() const = 0;

/// This pure virtual member function must be redefined for every streamable class.
/// It must read the necessary data members for the streamable class from the
/// supplied ipstream.
    virtual void *  Read( ipstream&, uint32 ) const = 0;

/// This pure virtual function must be redefined for every streamable class. It must
/// write the necessary streamable class data members to the supplied opstream
/// object. Write is usually implemented by calling the Write member function (if
/// available) of a base class, and then inserting any additional data members for
/// the derived class.
    virtual void  Write( opstream& ) const = 0;

  private:

    virtual uint32  ClassVersion() const = 0;

    TStreamableBase *object;

};

/// Provides a base class for all streamable objects.
class _OWLCLASS _RTTI TOldStreamer : public TStreamer {

  public:

/// Constructs the TOldStreamer object, and initializes the streamable object
/// pointer.
     TOldStreamer( TStreamable *obj ) : TStreamer(obj) {};

  protected:

/// This pure virtual member function must be redefined for every streamable class.
/// It returns the name of the streamable class, which is used by the stream manager
/// to register the streamable class. The name returned must be a zero-terminated
/// string.
    virtual LPCSTR  StreamableName() const
        {
        return STATIC_CAST(TStreamable *,GetObject())->streamableName();
        }

/// This pure virtual member function must be redefined for every streamable class.
/// It must read the necessary data members for the streamable class from the
/// supplied ipstream.
    virtual void *  Read( ipstream& is, uint32 ) const
        {
        return STATIC_CAST(TStreamable *,GetObject())->read( is );
        }

/// This pure virtual function must be redefined for every streamable class. It must
/// write the necessary streamable class data members to the supplied opstream
/// object. Write is usually implemented by calling the Write member function (if
/// available) of a base class, and then inserting any additional data members for
/// the derived class.
    virtual void  Write( opstream& os ) const
        {
        STATIC_CAST(TStreamable *,GetObject())->write( os );
        }

  private:

    virtual uint32  ClassVersion() const
        {
        return 0;
        }

};

class _OWLCLASS _RTTI TNewStreamer : public TStreamer {

  public:

/// Constructs the TNewStreamer object, and initializes the streamable object
/// pointer.
     TNewStreamer( TStreamableBase *obj ) : TStreamer(obj) {};

  protected:

    virtual LPCSTR  StreamableName() const
        {
        return _OBJ_TYPENAME(GetObject());
        }

};


/* ------------------------------------------------------------------------*/
/*                                                                         */
/*!   \class TPWrittenObjects                                                */
/*                                                                         */
/*!   Maintains a database of all objects that have been written to the     */
/*!   current persistent stream.                                            */
/*                                                                         */
/*!   Used by opstream when it writes a pointer from a stream to save the   */
/*!   address and ID of the object being written.                           */
/*                                                                         */
/* ------------------------------------------------------------------------*/

class  TSortedTPWObjObjectArray;

class _OWLCLASS TPWrittenObjects {

    friend class opstream;

  public:

    void  RemoveAll();

    class _OWLCLASS TPWObj {
      public:

         TPWObj() : Address(0), Ident(0) {}
         TPWObj( const void *adr, P_id_type id ) :
                        Address(adr), Ident(id) {}

        bool  operator == ( const TPWObj& o) const
            { return TAddrInt(Address) == TAddrInt(o.Address); }

        bool  operator < ( const TPWObj& o ) const
            { return TAddrInt(Address) < TAddrInt(o.Address); }

        const void *Address;
        P_id_type Ident;
        typedef uint32 TAddrInt;
    };

  private:

     TPWrittenObjects();
     ~TPWrittenObjects();

    void  RegisterObject( TStreamableBase *adr );
    void  RegisterVB( const TStreamableBase *adr );
    P_id_type  FindObject( TStreamableBase *adr );
    P_id_type  FindVB( TStreamableBase *adr );

    P_id_type                   CurId;
    TSortedTPWObjObjectArray*   Data;
};


/* ------------------------------------------------------------------------*/
/*                                                                         */
/*!   \class TPReadObjects                                                   */
/*                                                                         */
/*!   Maintains a database of all objects that have been read from the      */
/*!   current persistent stream.                                            */
/*                                                                         */
/*!   Used by ipstream when it reads a pointer from a stream to determine   */
/*!   the address of the object being referred to.                          */
/*                                                                         */
/* ------------------------------------------------------------------------*/

class TStreamableBaseArray;

class _OWLCLASS TPReadObjects {
    friend class ipstream;
  public:

    void  RemoveAll();

  private:
     TPReadObjects();
     ~TPReadObjects();

    void  RegisterObject( TStreamableBase *adr );
    TStreamableBase *  Find( P_id_type id );

    TStreamableBaseArray* Data;
};

///   \class pstream                                                        
///   Base class for handling streamable objects.                           

class _OWLCLASS pstream {
    friend class TStreamableTypes;
    friend class TStreamableClass;
  public:

/// Enumerates object pointer types.
    enum PointerTypes { ptNull, ptIndexed, ptObject };

     pstream( std::streambuf * );
    virtual  ~pstream();

    int  rdstate() const;
    int  eof() const;
    int  fail() const;
    int  bad() const;
    int  good() const;
    void  clear( int = 0 );
     operator void *() const;
    int  operator ! () const;

    std::streambuf *  rdbuf() const;

    static void  initTypes();
    static void  releaseTypes();

    static void  registerType( TStreamableClass *ts );

  protected:

     pstream();

    std::streambuf *bp;
    int state;

    void  init( std::streambuf * );
    void  setstate( int );

//    static TStreamableTypes *types;
};

/* ------------------------------------------------------------------------*/
/*                                                                         */
/*!   \class ipstream                                                        */
/*                                                                         */
/*!   Base class for reading streamable objects                             */
/*                                                                         */
/* ------------------------------------------------------------------------*/
inline ipstream&  operator >> ( ipstream&, int8& );
inline ipstream&  operator >> ( ipstream&, uint8& );
inline ipstream&  operator >> ( ipstream&, char& );
inline ipstream&  operator >> ( ipstream&, signed short& );
inline ipstream&  operator >> ( ipstream&, unsigned short& );
inline ipstream&  operator >> ( ipstream&, signed int& );
inline ipstream&  operator >> ( ipstream&, unsigned int& );
inline ipstream&  operator >> ( ipstream&, bool& );
inline ipstream&  operator >> ( ipstream&, signed long& );
inline ipstream&  operator >> ( ipstream&, unsigned long& );
inline ipstream&  operator >> ( ipstream&, int64& );
inline ipstream&  operator >> ( ipstream&, uint64& );
inline ipstream&  operator >> ( ipstream&, float& );
inline ipstream&  operator >> ( ipstream&, double&);
inline ipstream&  operator >> ( ipstream&, long double&);


/// ipstream, a specialized input stream derivative of pstream, is the base class
/// for reading (extracting) streamable objects.
class _OWLCLASS ipstream : virtual public pstream {
    friend class TStreamableClass;
  public:

     ipstream( std::streambuf * );

    std::streampos  tellg();
    ipstream&  seekg( std::streampos );
    ipstream&  seekg( std::streamoff, std::ios::seek_dir );

    uint8  readByte();
    void  readBytes( void *, size_t );
    void  freadBytes( void * data, size_t sz );

    uint32  readWord();
    uint16  readWord16();
    uint32  readWord32();

    LPSTR  readString();
    LPSTR  readString( LPSTR , unsigned );
    char  *  freadString();
    char  *  freadString( char *buf,
                                            unsigned maxLen );

    friend ipstream&  operator >> ( ipstream&, int8& );
    friend ipstream&  operator >> ( ipstream&, uint8& );
    friend ipstream&  operator >> ( ipstream&, char& );
    friend ipstream&  operator >> ( ipstream&, signed short& );
    friend ipstream&  operator >> ( ipstream&, unsigned short& );
    friend ipstream&  operator >> ( ipstream&, signed int& );
    friend ipstream&  operator >> ( ipstream&, unsigned int& );
    friend ipstream&  operator >> ( ipstream&, bool& );
    friend ipstream&  operator >> ( ipstream&, signed long& );
    friend ipstream&  operator >> ( ipstream&, unsigned long& );
    friend ipstream&  operator >> ( ipstream&, int64& );
    friend ipstream&  operator >> ( ipstream&, uint64& );
    friend ipstream&  operator >> ( ipstream&, float& );
    friend ipstream&  operator >> ( ipstream&, double& );
    friend ipstream&  operator >> ( ipstream&, long double& );
    friend ipstream &   operator >> ( ipstream&, tstring&);

    uint32  getVersion() const;

    TStreamableBase *  readObject( TStreamableBase *&mem, ModuleId mid = GetModuleId() );
    TStreamableBase *  readObjectPointer( TStreamableBase *&mem, ModuleId mid = GetModuleId() );

    TStreamableBase *  find( P_id_type );
    void  registerObject( TStreamableBase *adr );

  protected:

     ipstream();

    const ObjectBuilder *  readPrefix( ModuleId mid );
    void  readData( const ObjectBuilder *,
                          TStreamableBase *& );
    void  readSuffix();

    void  readVersion();

private:

    uint32 readStringLength();
    TPReadObjects objs;
    uint32 version;
};

///   \class opstream                                                        
///   Base class for writing streamable objects                             
//
/// opstream, a specialized derivative of pstream, is the base class for writing
/// (inserting) streamable objects.

inline opstream&  operator << ( opstream&, int8 );
inline opstream&  operator << ( opstream&, uint8 );
inline opstream&  operator << ( opstream&, char );
inline opstream&  operator << ( opstream&, signed short );
inline opstream&  operator << ( opstream&, unsigned short );
inline opstream&  operator << ( opstream&, signed int );
inline opstream&  operator << ( opstream&, unsigned int );
inline opstream&  operator << ( opstream&, bool );
inline opstream&  operator << ( opstream&, signed long );
inline opstream&  operator << ( opstream&, unsigned long );
inline opstream&  operator << ( opstream&, int64);
inline opstream&  operator << ( opstream&, uint64);
inline opstream&  operator << ( opstream&, float );
inline opstream&  operator << ( opstream&, double );
inline opstream&  operator << ( opstream&, long double );

class _OWLCLASS opstream : virtual public pstream {
  public:

     opstream( std::streambuf * );
    virtual  ~opstream();

    std::streampos  tellp();
    opstream&  seekp( std::streampos );
    opstream&  seekp( std::streamoff, std::ios::seek_dir );
    opstream&  flush();

    void  writeByte( uint8 );
    void  writeBytes( const void *, size_t );
    void  fwriteBytes( const void *data, size_t sz );

    void  writeWord( uint32 );
    void  writeWord16( uint16 );
    void  writeWord32( uint32 );

    void  writeString( const char * );
    void  fwriteString( const char * str );

    friend opstream&  operator << ( opstream&, int8 );
    friend opstream&  operator << ( opstream&, uint8 );
    friend opstream&  operator << ( opstream&, char );
    friend opstream&  operator << ( opstream&, signed short );
    friend opstream&  operator << ( opstream&, unsigned short );
    friend opstream&  operator << ( opstream&, signed int );
    friend opstream&  operator << ( opstream&, unsigned int );
    friend opstream&  operator << ( opstream&, bool );
    friend opstream&  operator << ( opstream&, signed long );
    friend opstream&  operator << ( opstream&, unsigned long );
    friend opstream&  operator << ( opstream&, int64);
    friend opstream&  operator << ( opstream&, uint64);
    friend opstream&  operator << ( opstream&, float );
    friend opstream&  operator << ( opstream&, double );
    friend opstream&  operator << ( opstream&, long double );

    void  writeObject( const TStreamableBase *t, int isPtr = 0, ModuleId mid = GetModuleId() );
    void  writeObjectPointer( const TStreamableBase *t, ModuleId mid = GetModuleId() );

    P_id_type  findObject( TStreamableBase *adr );
    void  registerObject( TStreamableBase *adr );

    P_id_type  findVB( TStreamableBase *adr );
    void  registerVB( TStreamableBase *adr );

  protected:

     opstream();

    void  writePrefix( const TStreamableBase * );
    void  writeData( const TStreamableBase *, ModuleId mid );
    void  writeSuffix( const TStreamableBase * );

    void  writeVersion();

  private:

    TPWrittenObjects* objs;
};

///   \class fpbase                                                          
///   Base class for handling streamable objects on file streams           
//
/// Provides the basic operations common to all object file stream I/O. It is a base
/// class for handling streamable objects on file streams.

class _OWLCLASS fpbase : virtual public pstream {
  public:

    enum { openprot = 0666 }; // default open mode
     fpbase();
     fpbase( LPCSTR, int, int = openprot );
#if defined(BI_HAS_STREAMWCHAROPEN)    
     fpbase( LPCWSTR, int, int = openprot );
#endif
    
    void  open( LPCSTR, int, int = openprot );
#if defined(BI_HAS_STREAMWCHAROPEN)    
    void  open( LPCWSTR, int, int = openprot );
#endif    

    void  close();
    void  setbuf( LPSTR, int );
    std::filebuf *  rdbuf();

  private:

    std::filebuf buf;
};

///   \class ifpstream                                                       
///   Base class for reading streamable objects from file streams           
//
/// ifpstream is a simple "mix" of its bases, fpbase and ipstream. It provides the
/// base class reading (extracting) streamable objects from file streams.

class _OWLCLASS ifpstream : public fpbase, public ipstream {
  public:

     ifpstream();
     ifpstream( LPCSTR,
                      int = std::ios::in,
                      int = fpbase::openprot
                    );
#if defined(BI_HAS_STREAMWCHAROPEN)    
     ifpstream( LPCWSTR,
                      int = std::ios::in,
                      int = fpbase::openprot
                    );
#endif                    

    std::filebuf *  rdbuf();
    	
    void  open( LPCSTR ,
                      int = std::ios::in,
                      int = fpbase::openprot
                    );
#if defined(BI_HAS_STREAMWCHAROPEN)    
    void  open( LPCWSTR ,
                      int = std::ios::in,
                      int = fpbase::openprot
                    );
#endif                   
};

///   \class ofpstream                                                       
///   Base class for writing streamable objects to file streams             
//
/// Provides the base class for writing (inserting) streamable objects to file
/// streams.

class _OWLCLASS ofpstream : public fpbase, public opstream {
  public:

     ofpstream();
     ofpstream( LPCSTR ,
                      int = std::ios::out,
                      int = fpbase::openprot
                    );
#if defined(BI_HAS_STREAMWCHAROPEN)    
     ofpstream( LPCWSTR ,
                      int = std::ios::out,
                      int = fpbase::openprot
                    );
#endif                    

    std::filebuf *  rdbuf();

    void  open( LPCSTR,
                      int = std::ios::out,
                      int = fpbase::openprot
                    );
#if defined(BI_HAS_STREAMWCHAROPEN)    
    void  open( LPCWSTR,
                      int = std::ios::out,
                      int = fpbase::openprot
                    );
#endif
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   Inline functions                                                      */
/*                                                                         */
/* ------------------------------------------------------------------------*/

/// Creates a buffered pstream with the given buffer. The state is set to 0.
inline  pstream::pstream( std::streambuf *sb ){
  init( sb );
}

/// Returns the current state value.
inline int  pstream::rdstate() const {
  return state;
}

/// Returns nonzero on end of stream.
inline int  pstream::eof() const {
  return state & std::ios::eofbit;
}

/// Returns nonzero if a previous stream operation failed.
inline int  pstream::fail() const {
  return state & (std::ios::failbit | std::ios::badbit | std::ios::goodbit);
}

/// Returns nonzero if an error occurs.
inline int  pstream::bad() const {
  return state & (std::ios::badbit | std::ios::goodbit);
}

/// Returns nonzero if no error states have been recorded for the stream (that is,
/// no errors have occurred).
inline int  pstream::good() const {
  return state == 0;
}

/// Sets the stream state to the given value (defaults to 0).
inline void  pstream::clear( int i ){
  state = (i & 0xFF) | (state & std::ios::goodbit);
}

/// Converts to a void pointer.
inline  pstream::operator void *() const {
  return fail() ? 0 : (void *)this;
}

/// Overloads the NOT operator. Returns 0 if the operation has failed (that is, if
/// pstream::fail returned nonzero); otherwise, returns nonzero.
inline int  pstream::operator! () const {
  return fail();
}

/// Returns the pb pointer to the buffer assigned to the stream.
inline std::streambuf *  pstream::rdbuf() const {
  return bp;
}

/// Creates a pstream without initializing the buffer pointer bp or state.
/// Use init to set the buffer and setstate to set the state.
inline  pstream::pstream(){
}

/// The init member function initializes the stream and sets state to 0 and bp to
/// sbp.
inline void  pstream::init( std::streambuf *sbp ){
  state = 0;
  bp = sbp;
}

/// Updates the state data member with state |= (b & 0xFF).
inline void  pstream::setstate( int b ){
  state |= (b&0xFF);
}

/// Creates a buffered ipstream with the given buffer. The state is set to 0.
inline  ipstream::ipstream( std::streambuf *sb ){
  pstream::init( sb );
  readVersion();
}

/// Creates a buffered ipstream without initializing the buffer pointer, bp.
/// Use psteam::init to set the buffer and state.
inline  ipstream::ipstream(){
  if( bp != 0 )
    readVersion();
}

/// Returns a pointer to the object corresponding to Id.
inline TStreamableBase *  ipstream::find( P_id_type id ){
  return objs.Find( id );
}

/// Registers the object pointed to by adr.
inline void  ipstream::registerObject( TStreamableBase *adr ){
  objs.RegisterObject( adr );
}

/// Returns the object version number.
inline uint32  ipstream::getVersion() const {
  return version;
}
inline  opstream::~opstream(){
  delete objs;
}

/// Writes the 32-bit word us to the stream.
inline void  opstream::writeWord( uint32 word32 ){
  writeWord32( word32 );
}

/// Writes the class name suffix to the stream. The << operator uses this function
/// to write a prefix and suffix around the data written with writeData. The
/// prefix/suffix is used to ensure type-safe stream I/O.
inline void  opstream::writeSuffix( const TStreamableBase * ){
  writeByte( ']' );
}

/// Returns the type ID for the object pointed to by adr.
inline P_id_type  opstream::findObject( TStreamableBase *adr ){
  return objs->FindObject( adr );
}

/// Registers the class of the object pointed to by adr.
inline void  opstream::registerObject( TStreamableBase *adr ){
  objs->RegisterObject( adr );
}

/// Returns a pointer to the virtual base.
inline P_id_type  opstream::findVB( TStreamableBase *adr ){
  return objs->FindVB( adr );
}

/// Registers a virtual base class.
inline void  opstream::registerVB( TStreamableBase *adr ){
  objs->RegisterVB( adr );
}

/// Creates a buffered fpbase object.
inline  fpbase::fpbase(){
  pstream::init( &buf );
}

/// Creates a buffered fpbase object. It opens the file specified by name,
/// using the mode omode and protection prot; and attaches this file to the stream.
inline  fpbase::fpbase( LPCSTR name, int omode, int prot ){
  pstream::init( &buf );
  open( name, omode, prot );
}

#if defined(BI_HAS_STREAMWCHAROPEN)
/// Creates a buffered fpbase object. It opens the file specified by name,
/// using the mode omode and protection prot; and attaches this file to the stream.
inline  fpbase::fpbase( LPCWSTR name, int omode, int prot ){
  pstream::init( &buf );
  open( name, omode, prot );
}
#endif

/// Returns a pointer to the current file buffer.
inline std::filebuf *  fpbase::rdbuf(){
  return &buf;
}

/// Creates a buffered ifpstream object using a default buffer.
inline  ifpstream::ifpstream(){
}

/// Creates a buffered ifpstream object. It opens the file specified by name
/// using the mode mode and protection prot; and attaches this file to the stream.
inline  ifpstream::ifpstream( LPCSTR name, int omode, int prot )
:
  fpbase( name, omode | std::ios::in | std::ios::binary, prot )
{
}

#if defined(BI_HAS_STREAMWCHAROPEN)
/// Creates a buffered ifpstream object. It opens the file specified by name
/// using the mode mode and protection prot; and attaches this file to the stream.
inline  ifpstream::ifpstream( LPCWSTR name, int omode, int prot )
:
  fpbase( name, omode | std::ios::in | std::ios::binary, prot )
{
}
#endif

/// Returns a pointer to the current file buffer.
inline std::filebuf *  ifpstream::rdbuf(){
  return fpbase::rdbuf();
}

/// It opens the named file in the given mode (app, ate, in, out, binary, trunc,
/// nocreate, or noreplace) and protection. The default mode for ifpstream is
/// ios::in (input) with openprot protection. The opened file is attached to this
/// stream.
inline void  ifpstream::open( LPCSTR name,
                                    int omode,
                                    int prot )
{
  fpbase::open( name, omode | std::ios::in | std::ios::binary, prot );
  readVersion();
}

#if defined(BI_HAS_STREAMWCHAROPEN)
/// It opens the named file in the given mode (app, ate, in, out, binary, trunc,
/// nocreate, or noreplace) and protection. The default mode for ifpstream is
/// ios::in (input) with openprot protection. The opened file is attached to this
/// stream.
inline void  ifpstream::open( LPCWSTR name,
                                    int omode,
                                    int prot )
{
  fpbase::open( name, omode | std::ios::in | std::ios::binary, prot );
  readVersion();
}
#endif

/// Creates a buffered ofpstream object using a default buffer.
inline  ofpstream::ofpstream(){
}

/// Creates a buffered ofpstream object. It opens the file specified by
/// name, using the mode mode, and protection prot; and attaches this file to the
/// stream
inline  ofpstream::ofpstream( LPCSTR name, int omode, int prot )
:
  fpbase( name, omode | std::ios::out | std::ios::binary, prot )
{
}

#if defined(BI_HAS_STREAMWCHAROPEN)
/// Creates a buffered ofpstream object. It opens the file specified by
/// name, using the mode mode, and protection prot; and attaches this file to the
/// stream
inline  ofpstream::ofpstream( LPCWSTR name, int omode, int prot )
:
  fpbase( name, omode | std::ios::out | std::ios::binary, prot )
{
}
#endif

inline std::filebuf *  ofpstream::rdbuf(){
  return fpbase::rdbuf();
}

/// Opens the named file in the given mode (app, ate, in, out, binary, trunc,
/// nocreate, or noreplace) and protection. The default mode for ofpstream is
/// ios::out (output) with openprot protection. The opened file is attached to this
/// stream.
inline void  ofpstream::open( LPCSTR name,
                                    int omode,
                                    int prot )
{
  fpbase::open( name, omode | std::ios::out | std::ios::binary, prot );
  writeVersion();
}

#if defined(BI_HAS_STREAMWCHAROPEN)
/// Returns a pointer to the current file buffer.
inline void  ofpstream::open( LPCWSTR name,
                                    int omode,
                                    int prot )
{
  fpbase::open( name, omode | std::ios::out | std::ios::binary, prot );
  writeVersion();
}
#endif

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, int8& ch )
{
  ch = ps.readByte();
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, uint8& ch ){
  ch = ps.readByte();
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, char &ch ){
  ch = ps.readByte();
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, signed short &sh ){
  sh = ps.readWord16();
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, unsigned short &sh ){
  sh = ps.readWord16();
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, signed int &i ){
  i = (int)(ps.readWord());
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, unsigned int &i ){
  i = (unsigned int)(ps.readWord());
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, signed long &l ){
  ps.readBytes( &l, sizeof(l) );
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, unsigned long &l ){
  ps.readBytes( &l, sizeof(l) );
  return ps;
}

inline ipstream&  operator >> (ipstream& ps, int64& v)
{
  ps.readBytes(&v, sizeof v);
  return ps;
}

inline ipstream&  operator >> (ipstream& ps, uint64& v)
{
  ps.readBytes(&v, sizeof v);
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, float &f ){
  ps.readBytes( &f, sizeof(f) );
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, double &d ){
  ps.readBytes( &d, sizeof(d) );
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, long double &l ){
  ps.readBytes( &l, sizeof(l) );
  return ps;
}

/// This friend operator of ipstream extracts (reads) from the ipstream ps, to the
/// given argument. It returns a reference to the stream that lets you chain >>
/// operations in the usual way.
/// The data type of the argument determines how the read is performed. For example,
/// reading a signed char is implemented using readByte.
inline ipstream&  operator >> ( ipstream& ps, bool &b ){
  b = static_cast<bool>(ps.readWord32());
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, int8 ch ){
  ps.writeByte( ch );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, uint8 ch ){
  ps.writeByte( ch );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, char ch ){
  ps.writeByte( ch );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, signed short sh ){
  ps.writeWord16( sh );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, unsigned short sh ){
  ps.writeWord16( sh );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, signed int i ){
  ps.writeWord32( i );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, unsigned int i ){
  ps.writeWord32( i );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, signed long l ){
  ps.writeBytes( &l, sizeof(l) );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, unsigned long l ){
  ps.writeBytes( &l, sizeof(l) );
  return ps;
}

inline opstream&  operator << (opstream& ps, int64 v)
{
  ps.writeBytes(&v, sizeof v);
  return ps;
}

inline opstream&  operator << (opstream& ps, uint64 v)
{
  ps.writeBytes(&v, sizeof v);
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, float f ){
  ps.writeBytes( &f, sizeof(f) );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, double d ){
  ps.writeBytes( &d, sizeof(d) );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, long double l ){
  ps.writeBytes( &l, sizeof(l) );
  return ps;
}

/// This friend operator of opstream inserts (writes) the given argument to the
/// given ipstream object.
/// The data type of the argument determines the form of write operation employed.
inline opstream&  operator << ( opstream& ps, bool b ){
  ps.writeWord32( b );
  return ps;
}

template <class Base> void WriteBaseObject( Base *base, opstream& out )
{
  typedef typename Base::Streamer Base_Streamer;

  Base_Streamer strmr(base);
  out << strmr.ClassVersion();
  strmr.Write( out );
}

template <class Base> void ReadBaseObject( Base *base, ipstream& in )
{
  typedef typename Base::Streamer Base_Streamer;

  uint32 version = 0;
  if( in.getVersion() > 0 )
    in >> version;
  Base_Streamer(base).Read( in, version );
}

template <class Base> void WriteVirtualBase( Base *base, opstream& out )
{
  typedef typename Base::Streamer Base_Streamer;

 if( !out.good() )
    return;
  if( out.findVB( base ) != 0 ){
    out.writeByte( pstream::ptIndexed );    // use ptIndexed to indicate
                                            // that we've already seen
                                            // this virtual base. Don't
                                            // need to actually write it.
  }
  else{
    Base_Streamer strmr(base);
    out.registerObject( (TStreamableBase *)((char*)base + 1) );
    out.writeByte( pstream::ptObject );
    out.writeWord32( strmr.ClassVersion() );
    strmr.Write( out );
  }
}

template <class Base> void ReadVirtualBase( Base *base, ipstream& in )
{
  typedef typename Base::Streamer Base_Streamer;

  char ch;
  in >> ch;
  switch( ch ){
    case pstream::ptIndexed:
      break;      // We've already read this virtual base
    case pstream::ptObject: {
        uint32 ver = 0;
        if( in.getVersion() > 0 )
          ver = in.readWord32();
        Base_Streamer strmr(base);
        // register the address
        in.registerObject(strmr.GetObject());
        strmr.Read( in, ver );
      }
      break;
  }
}

//
//  Individual Components for Streamable Declarations
//

#if defined (__BORLANDC__) && defined(_WIN64)
#define READBASEOBJECT(cls)   ReadBaseObject<cls>( cls *, ::owl::ipstream& )
#define WRITEBASEOBJECT(cls)  WriteBaseObject<cls>( cls *, ::owl::opstream& )
#define READVIRTUALBASE(cls)  ReadVirtualBase<cls>( cls *, ::owl::ipstream& )
#define WRITEVIRTUALBASE(cls) WriteVirtualBase<cls>( cls *, ::owl::opstream& )
#else
#define READBASEOBJECT(cls)   ReadBaseObject( cls *, ::owl::ipstream& )
#define WRITEBASEOBJECT(cls)  WriteBaseObject( cls *, ::owl::opstream& )
#define READVIRTUALBASE(cls)  ReadVirtualBase( cls *, ::owl::ipstream& )
#define WRITEVIRTUALBASE(cls) WriteVirtualBase( cls *, ::owl::opstream& )
#endif



#define DECLARE_STREAMERS(cls, ver )                                \
public:                                                             \
    class Streamer : public ::owl::TNewStreamer                    \
        {                                                           \
        public:                                                     \
                                                                    \
        Streamer( ::owl::TStreamableBase *obj );                   \
                                                                    \
        virtual ::owl::uint32 ClassVersion() const                         \
            { return ver; }                                         \
                                                                    \
        virtual void Write( ::owl::opstream& ) const;              \
        virtual void *Read( ::owl::ipstream&, ::owl::uint32 ) const;      \
                                                                    \
        cls *GetObject() const                                      \
            {                                                       \
            return object;                                          \
            }                                                       \
                                                                    \
        static ::owl::TStreamer *Build( ::owl::TStreamableBase *obj ) \
            {                                                       \
            return new Streamer( obj ? obj : new cls(::owl::streamableInit) ); \
            }                                                       \
                                                                    \
        private:                                                    \
            cls *object;                                            \
                                                                    \
        };                                                          \
    friend class Streamer;                                          \
	friend void ::owl:: READBASEOBJECT( cls ); \
	friend void ::owl:: WRITEBASEOBJECT( cls );\
	friend void ::owl:: READVIRTUALBASE( cls );\
    friend void ::owl:: WRITEVIRTUALBASE( cls )


#define DECLARE_STREAMER( exp, cls, ver )                           \
public:                                                             \
    class exp Streamer : public ::owl::TNewStreamer                \
        {                                                           \
        public:                                                     \
                                                                    \
        Streamer( ::owl::TStreamableBase *obj );                   \
                                                                    \
        virtual ::owl::uint32 ClassVersion() const                         \
            { return ver; }                                         \
                                                                    \
        virtual void Write( ::owl::opstream& ) const;              \
        virtual void *Read( ::owl::ipstream&, ::owl::uint32 ) const;      \
                                                                    \
        cls *GetObject() const                                      \
            {                                                       \
            return object;                                          \
            }                                                       \
                                                                    \
        static ::owl::TStreamer *Build( ::owl::TStreamableBase *obj ) \
            {                                                       \
            return new Streamer( obj ? obj : new cls(::owl::streamableInit) ); \
            }                                                       \
                                                                    \
        private:                                                    \
            cls *object;                                            \
                                                                    \
        };                                                          \
    friend class exp Streamer;                                      \
    friend void ::owl:: READBASEOBJECT( cls ); \
	friend void ::owl:: WRITEBASEOBJECT( cls ); \
	friend void ::owl:: READVIRTUALBASE( cls ); \
    friend void ::owl:: WRITEVIRTUALBASE( cls )


#define DECLARE_STREAMER_FROM_BASES(cls, base )                     \
public:                                                             \
    class Streamer : public base::Streamer                          \
        {                                                           \
        public:                                                     \
                                                                    \
        Streamer( ::owl::TStreamableBase *obj ) : base::Streamer(obj){} \
                                                                    \
        cls *GetObject() const                                      \
            {                                                       \
            return object;                                          \
            }                                                       \
                                                                    \
        static ::owl::TStreamer *Build( ::owl::TStreamableBase *obj ) \
            {                                                       \
            return new Streamer( obj ? obj : new cls(::owl::streamableInit) ); \
            }                                                       \
                                                                    \
        private:                                                    \
            cls *object;                                            \
                                                                    \
        };                                                          \
    friend class Streamer;                                                \
	friend void ::owl:: READBASEOBJECT( cls );   \
	friend void ::owl:: WRITEBASEOBJECT( cls );  \
	friend void ::owl:: READVIRTUALBASE( cls );  \
	friend void ::owl:: WRITEVIRTUALBASE( cls )


#define DECLARE_STREAMER_FROM_BASE( exp, cls, base )                \
public:                                                             \
    class exp Streamer : public base::Streamer                      \
        {                                                           \
        public:                                                     \
                                                                    \
        Streamer( ::owl::TStreamableBase *obj ) : base::Streamer(obj){}    \
                                                                    \
        cls *GetObject() const                                      \
            {                                                       \
            return object;                                          \
            }                                                       \
                                                                    \
        static ::owl::TStreamer *Build( ::owl::TStreamableBase *obj )             \
            {                                                       \
            return new Streamer( obj ? obj : new cls(::owl::streamableInit) ); \
            }                                                       \
                                                                    \
        private:                                                    \
            cls *object;                                            \
                                                                    \
        };                                                          \
    friend class exp Streamer;                                      \
    friend void ::owl:: READBASEOBJECT( cls ); \
	friend void ::owl:: WRITEBASEOBJECT( cls );\
	friend void ::owl:: READVIRTUALBASE( cls );\
	friend void ::owl:: WRITEVIRTUALBASE( cls )

#define DECLARE_ABSTRACT_STREAMERS(cls, ver )                       \
public:                                                             \
    class Streamer : public ::owl::TNewStreamer                            \
        {                                                           \
        public:                                                     \
                                                                    \
        Streamer( ::owl::TStreamableBase *obj );                           \
                                                                    \
        virtual ::owl::uint32 ClassVersion() const                         \
            { return ver; }                                         \
                                                                    \
        virtual void Write( ::owl::opstream& ) const;                      \
        virtual void *Read( ::owl::ipstream&, ::owl::uint32 ) const;              \
                                                                    \
        cls *GetObject() const                                      \
            {                                                       \
            return object;                                          \
            }                                                       \
                                                                    \
        private:                                                    \
            cls *object;                                            \
                                                                    \
        };                                                          \
    friend class Streamer;                                          \
    friend void ::owl:: READBASEOBJECT( cls ); \
	friend void ::owl:: WRITEBASEOBJECT( cls );\
	friend void ::owl:: READVIRTUALBASE( cls );\
	friend void ::owl:: WRITEVIRTUALBASE( cls )

#define DECLARE_ABSTRACT_STREAMER( exp, cls, ver )                  \
public:                                                             \
    class exp Streamer : public ::owl::TNewStreamer                        \
        {                                                           \
        public:                                                     \
                                                                    \
        Streamer( ::owl::TStreamableBase *obj );                           \
                                                                    \
        virtual ::owl::uint32 ClassVersion() const                         \
            { return ver; }                                         \
                                                                    \
        virtual void Write( ::owl::opstream& ) const;                      \
        virtual void *Read( ::owl::ipstream&, ::owl::uint32 ) const;              \
                                                                    \
        cls *GetObject() const                                      \
            {                                                       \
            return object;                                          \
            }                                                       \
                                                                    \
        private:                                                    \
            cls *object;                                            \
                                                                    \
        };                                                          \
    friend class exp Streamer;                                      \
	friend void ::owl:: READBASEOBJECT( cls ); \
	friend void ::owl:: WRITEBASEOBJECT( cls );\
	friend void ::owl:: READVIRTUALBASE( cls );\
	friend void ::owl:: WRITEVIRTUALBASE( cls )


#define DECLARE_STREAMABLE_OPS( cls )                               \
static ::owl::ipstream& readRef( ::owl::ipstream& is, cls& cl );                  \
friend inline ::owl::ipstream& operator >> ( ::owl::ipstream& is, cls& cl )       \
    { return cls::readRef( is, cl ); }                              \
static ::owl::ipstream& readPtr( ::owl::ipstream& is, cls*& cl );                 \
friend inline ::owl::ipstream& operator >> ( ::owl::ipstream& is, cls*& cl )      \
    { return cls::readPtr( is, cl ); }                              \
static ::owl::opstream& writeRef( ::owl::opstream& is, const cls& cl );           \
friend inline ::owl::opstream& operator << ( ::owl::opstream& os, const cls& cl ) \
    { return cls::writeRef( os, cl ); }                             \
static ::owl::opstream& writePtr( ::owl::opstream& is, const cls* cl );           \
friend inline ::owl::opstream& operator << ( ::owl::opstream& os, const cls* cl ) \
    { return cls::writePtr( os, cl ); }

#define DECLARE_STREAMABLE_INLINES_I( cls )


#define DECLARE_STREAMABLE_CTOR( cls )                              \
public:                                                             \
    cls ( ::owl::StreamableInit )

//
// Castable declaration macros
//
# define DECLARE_CASTABLE friend class std::type_info

//
// Streamable declaration macros
//
#if !defined(BI_NO_OBJ_STREAMING)

#define DECLARE_STREAMABLE( exp, cls, ver )                         \
    DECLARE_CASTABLE ;                                              \
    DECLARE_STREAMER( exp, cls, ver );                              \
    DECLARE_STREAMABLE_OPS( cls );                                  \
    DECLARE_STREAMABLE_CTOR( cls )

#define DECLARE_STREAMABLES(cls, ver )                              \
    DECLARE_CASTABLE ;                                              \
    DECLARE_STREAMERS(cls, ver );                                   \
    DECLARE_STREAMABLE_OPS( cls );                                  \
    DECLARE_STREAMABLE_CTOR( cls )

#define DECLARE_STREAMABLE_FROM_BASE( exp, cls, base )              \
    DECLARE_CASTABLE;                                               \
    DECLARE_STREAMER_FROM_BASE( exp, cls, base );                   \
    DECLARE_STREAMABLE_OPS( cls );                                  \
    DECLARE_STREAMABLE_CTOR( cls )

#define DECLARE_STREAMABLE_FROM_BASES(cls, base )                   \
    DECLARE_CASTABLE;                                               \
    DECLARE_STREAMER_FROM_BASES(cls, base );                        \
    DECLARE_STREAMABLE_OPS( cls );                                  \
    DECLARE_STREAMABLE_CTOR( cls )

#define DECLARE_ABSTRACT_STREAMABLE( exp, cls, ver )                \
    DECLARE_CASTABLE;                                               \
    DECLARE_ABSTRACT_STREAMER( exp, cls, ver );                     \
    DECLARE_STREAMABLE_OPS( cls );                                  \
    DECLARE_STREAMABLE_CTOR( cls )

#define DECLARE_ABSTRACT_STREAMABLES(cls, ver )                     \
    DECLARE_CASTABLE;                                               \
    DECLARE_ABSTRACT_STREAMERS(cls, ver );                          \
    DECLARE_STREAMABLE_OPS( cls );                                  \
    DECLARE_STREAMABLE_CTOR( cls )

// only for OWL
#if defined(_BUILDOWLDLL) || defined(_OWLDLL)
#  define DECLARE_STREAMABLE_OWL( cls, ver )                     \
    DECLARE_STREAMABLE( _OWLCLASS, cls, ver )
# define DECLARE_STREAMABLE_FROM_BASE_OWL( cls, base )          \
    DECLARE_STREAMABLE_FROM_BASE( _OWLCLASS, cls, base )
#define DECLARE_ABSTRACT_STREAMABLE_OWL(cls, ver )              \
    DECLARE_ABSTRACT_STREAMABLE( _OWLCLASS, cls, ver )                

#define DECLARE_STREAMABLE_INLINES( cls )                       \
    DECLARE_STREAMABLE_INLINES_I( cls )

#else
#  define DECLARE_STREAMABLE_OWL(cls, ver )                      \
    DECLARE_STREAMABLES( cls, ver ) 
# define DECLARE_STREAMABLE_FROM_BASE_OWL( cls, base )          \
    DECLARE_STREAMABLE_FROM_BASES(cls, base )
#define DECLARE_ABSTRACT_STREAMABLE_OWL(cls, ver )              \
    DECLARE_ABSTRACT_STREAMABLES(cls, ver )                

#define DECLARE_STREAMABLE_INLINES( cls )                       \
    DECLARE_STREAMABLE_INLINES_I( cls )

#endif


#else  // if BI_NO_OBJ_STREAMING

#define DECLARE_STREAMABLE( exp, cls, ver )                         \
    DECLARE_CASTABLE

#define DECLARE_STREAMABLE_S(cls, ver )                             \
    DECLARE_CASTABLE

#define DECLARE_STREAMABLE_FROM_BASE( exp, cls, base )              \
    DECLARE_CASTABLE

#define DECLARE_STREAMABLE_FROM_BASE_S( cls, base )                 \
    DECLARE_CASTABLE

#define DECLARE_ABSTRACT_STREAMABLE( exp, cls, ver )                \
    DECLARE_CASTABLE

#define DECLARE_ABSTRACT_STREAMABLE_S( cls, ver )                   \
    DECLARE_CASTABLE

#define DECLARE_STREAMABLE_OWL(cls, ver )                           \
    DECLARE_CASTABLE

# define DECLARE_STREAMABLE_FROM_BASE_OWL( cls, base )              \
    DECLARE_CASTABLE

#define DECLARE_ABSTRACT_STREAMABLE_OWL( cls, ver )                 \
    DECLARE_CASTABLE

#define DECLARE_STREAMABLE_INLINES( cls )

#endif  // if/else BI_NO_OBJ_STREAMING

//
// Castable implementation macros
//

// These macros are obsolete, since all modern compilers support RTTI. They will be moved to backward compatibility section
#define IMPLEMENT_CASTABLE( cls )
#define IMPLEMENT_CASTABLE1( cls, base1 )
#define IMPLEMENT_CASTABLE2( cls, base1, base2 )
#define IMPLEMENT_CASTABLE3( cls, base1, base2, base3 )
#define IMPLEMENT_CASTABLE4( cls, base1, base2, base3, base4 )
#define IMPLEMENT_CASTABLE5( cls, base1, base2, base3, base4, base5 )



//
// Streamable implementation mactos
//
#if !defined(BI_NO_OBJ_STREAMING)
#  if defined(BI_COMP_MSC) && !defined(UNIX) && !defined(__EDG__)
#    if defined(BI_COMPATIBLE_STREAMING)
#      define IMPLEMENT_STREAMABLE_CLASS( cls )    \
         ::owl::TStreamableClass r ## cls( typeid(cls).name() + 6, &cls::Streamer::Build )
#    else
#      define IMPLEMENT_STREAMABLE_CLASS( cls )    \
         ::owl::TStreamableClass r ## cls( typeid(cls).raw_name(), &cls::Streamer::Build )
#    endif
#  else
#    define IMPLEMENT_STREAMABLE_CLASS( cls )    \
       ::owl::TStreamableClass r ## cls( typeid(cls).name(), &cls::Streamer::Build )
#  endif
#else
#  define IMPLEMENT_STREAMABLE_CLASS( cls )
#endif

#if !defined(BI_NO_OBJ_STREAMING)

#define IMPLEMENT_STREAMABLE_POINTER_IMPL(template_prefix, cls)\
  template_prefix owl::ipstream& cls::readPtr(::owl::ipstream& is, cls*& cl)\
  {::owl::TStreamableBase *t = 0; cl = dynamic_cast<cls*>(is.readObjectPointer(t)); return is;}\
  \
  template_prefix owl::ipstream& cls::readRef(::owl::ipstream& is, cls& cl)\
  {::owl::TStreamableBase* p = &cl; is.readObject(p); return is;}\
  \
  template_prefix owl::opstream& cls::writeRef(::owl::opstream& os, const cls& cl)\
  {os.writeObject(&cl); return os;}\
  \
  template_prefix owl::opstream& cls::writePtr(::owl::opstream& os, const cls* cl)\
  {os.writeObjectPointer(cl); return os;}

#define IMPLEMENT_STREAMABLE_POINTER(cls)\
  IMPLEMENT_STREAMABLE_POINTER_IMPL(, cls)

#else
#define IMPLEMENT_STREAMABLE_POINTER( cls )
#endif

#if !defined(BI_NO_OBJ_STREAMING)

#define IMPLEMENT_STREAMER( cls )                                   \
cls::Streamer::Streamer( ::owl::TStreamableBase *obj ) :                   \
    ::owl::TNewStreamer(obj), object(TYPESAFE_DOWNCAST(obj,cls)){}

#define IMPLEMENT_STREAMABLE_CTOR( cls )                            \
cls::cls ( ::owl::StreamableInit ) {}

#define IMPLEMENT_STREAMABLE_CTOR1_IMPL(template_prefix, cls, constructor, base1)\
  template_prefix cls::constructor(::owl::StreamableInit) : base1(::owl::streamableInit) {}

#define IMPLEMENT_STREAMABLE_CTOR1(cls, base1)\
  IMPLEMENT_STREAMABLE_CTOR1_IMPL(, cls, cls, base1)

#define IMPLEMENT_STREAMABLE_CTOR2( cls, base1, base2 )             \
cls::cls ( ::owl::StreamableInit ) :                                       \
    base1 ( ::owl::streamableInit ),                                       \
    base2 ( ::owl::streamableInit ) {}

#define IMPLEMENT_STREAMABLE_CTOR3( cls, base1, base2, base3 )      \
cls::cls ( ::owl::StreamableInit ) :                                       \
    base1 ( ::owl::streamableInit ),                                       \
    base2 ( ::owl::streamableInit ),                                       \
    base3 ( ::owl::streamableInit ) {}

#define IMPLEMENT_STREAMABLE_CTOR4( cls, base1, base2, base3, base4 )\
cls::cls ( ::owl::StreamableInit ) :                                       \
    base1 ( ::owl::streamableInit ),                                       \
    base2 ( ::owl::streamableInit ),                                       \
    base3 ( ::owl::streamableInit ),                                       \
    base4 ( ::owl::streamableInit ) {}

#define IMPLEMENT_STREAMABLE_CTOR5( cls, base1,base2,base3,base4,base5)\
cls::cls ( ::owl::StreamableInit ) :                                       \
    base1 ( ::owl::streamableInit ),                                       \
    base2 ( ::owl::streamableInit ),                                       \
    base3 ( ::owl::streamableInit ),                                       \
    base4 ( ::owl::streamableInit ),                                       \
    base5 ( ::owl::streamableInit ) {}

#else  // if !defined(BI_NO_OBJ_STREAMING)
# define IMPLEMENT_STREAMER( cls )
# define IMPLEMENT_STREAMABLE_CTOR( cls )
# define IMPLEMENT_STREAMABLE_CTOR1( cls, base1 )
# define IMPLEMENT_STREAMABLE_CTOR2( cls, base1, base2 )
# define IMPLEMENT_STREAMABLE_CTOR3( cls, base1, base2, base3 )
# define IMPLEMENT_STREAMABLE_CTOR4( cls, base1, base2, base3, base4 )
# define IMPLEMENT_STREAMABLE_CTOR5( cls, base1,base2,base3,base4,base5)
#endif
                                                                    \

//
//  Standard Combinations of Streamable Implementations
//

#if !defined(BI_NO_OBJ_STREAMING)
#define IMPLEMENT_ABSTRACT_STREAMABLE( cls )                        \
IMPLEMENT_STREAMER( cls );                                          \
IMPLEMENT_STREAMABLE_CTOR( cls );                                   \
IMPLEMENT_STREAMABLE_POINTER( cls )

#define IMPLEMENT_ABSTRACT_STREAMABLE1( cls, base1 )                \
IMPLEMENT_STREAMER( cls );                                          \
IMPLEMENT_STREAMABLE_CTOR1( cls, base1 );                           \
IMPLEMENT_STREAMABLE_POINTER( cls )

#define IMPLEMENT_ABSTRACT_STREAMABLE2( cls, base1, base2 )         \
IMPLEMENT_STREAMER( cls );                                          \
IMPLEMENT_STREAMABLE_CTOR2( cls, base1, base2 );                    \
IMPLEMENT_STREAMABLE_POINTER( cls )

#define IMPLEMENT_ABSTRACT_STREAMABLE3( cls, base1, base2, base3 )  \
IMPLEMENT_STREAMER( cls );                                          \
IMPLEMENT_STREAMABLE_CTOR3( cls, base1, base2, base3 );             \
IMPLEMENT_STREAMABLE_POINTER( cls )

#define IMPLEMENT_ABSTRACT_STREAMABLE4( cls, base1, base2, base3, base4 )\
IMPLEMENT_STREAMER( cls );                                          \
IMPLEMENT_STREAMABLE_CTOR4( cls, base1, base2, base3, base4 );      \
IMPLEMENT_STREAMABLE_POINTER( cls )

#define IMPLEMENT_ABSTRACT_STREAMABLE5( cls, base1, base2, base3, base4, base5 )\
IMPLEMENT_STREAMER( cls );                                          \
IMPLEMENT_STREAMABLE_CTOR5( cls, base1, base2, base3, base4, base5 );\
IMPLEMENT_STREAMABLE_POINTER( cls )

#define IMPLEMENT_STREAMABLE( cls )                                 \
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_ABSTRACT_STREAMABLE( cls )

#define IMPLEMENT_STREAMABLE1( cls, base1 )                         \
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_ABSTRACT_STREAMABLE1( cls, base1 )

#define IMPLEMENT_STREAMABLE2( cls, base1, base2 )                  \
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_ABSTRACT_STREAMABLE2( cls, base1, base2 )

#define IMPLEMENT_STREAMABLE3( cls, base1, base2, base3 )           \
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_ABSTRACT_STREAMABLE3( cls, base1, base2, base3 )

#define IMPLEMENT_STREAMABLE4( cls, base1, base2, base3, base4 )    \
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_ABSTRACT_STREAMABLE4( cls, base1, base2, base3, base4 )

#define IMPLEMENT_STREAMABLE5( cls, base1, base2, base3, base4, base5 )\
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_ABSTRACT_STREAMABLE5( cls, base1, base2, base3, base4, base5 )

#define IMPLEMENT_STREAMABLE_FROM_BASE( cls, base1 )                \
IMPLEMENT_STREAMABLE_CLASS( cls );                                  \
IMPLEMENT_STREAMABLE_CTOR1( cls, base1 );                           \
IMPLEMENT_STREAMABLE_POINTER( cls )

#else  // if BI_NO_OBJ_STREAMING

#define IMPLEMENT_ABSTRACT_STREAMABLE( cls )
#define IMPLEMENT_ABSTRACT_STREAMABLE1( cls, base1 )
#define IMPLEMENT_ABSTRACT_STREAMABLE2( cls, base1, base2 )
#define IMPLEMENT_ABSTRACT_STREAMABLE3( cls, base1, base2, base3 )
#define IMPLEMENT_ABSTRACT_STREAMABLE4( cls, base1, base2, base3, base4 )
#define IMPLEMENT_ABSTRACT_STREAMABLE5( cls, base1, base2, base3, base4, base5 )
#define IMPLEMENT_STREAMABLE( cls )
#define IMPLEMENT_STREAMABLE1( cls, base1 )
#define IMPLEMENT_STREAMABLE2( cls, base1, base2 )
#define IMPLEMENT_STREAMABLE3( cls, base1, base2, base3 )
#define IMPLEMENT_STREAMABLE4( cls, base1, base2, base3, base4 )
#define IMPLEMENT_STREAMABLE5( cls, base1, base2, base3, base4, base5 )
#define IMPLEMENT_STREAMABLE_FROM_BASE( cls, base1 )

#endif  // if/else BI_NO_OBJ_STREAMING

/// @}

} // OWL namespace


#endif  // OWL_OBJSTRM_H
