//----------------------------------------------------------------------------
// Borland Class Library
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Class definitions for object streaming
/// \code
///
///                 +----------------+  +-------------+
///                 |TStreamableTypes|  |ObjectBuilder|
///                 +--+----------+--+  +-------------+
///                    |class name|     |  BUILDER    |
///                    +----------+     |  delta      |
///                          |          +-------------+
///                          |                 |
///                          |                 /\ .
///                          |         +--------
///                          |         |
///                          |         |
///                        +----------------+
///                        |TStreamableClass|
///                        +----------------+
///                        |  Module ID     |
///                        +----------------+
/// \endcode
//----------------------------------------------------------------------------

#if !defined(OWL_STREAMBL_H)
#define OWL_STREAMBL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined(_INC_STRING)
# include <string.h>
#endif

#include <owl/private/wsysinc.h>

#if defined(_FASTTHIS)
#  define _OWLFASTTHIS __fastthis
#else
#  define _OWLFASTTHIS
#endif

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

typedef unsigned ModuleId;
inline ModuleId GetModuleId() { return 1; }

class _OWLCLASS _OWLFASTTHIS _RTTI TStreamer;
class _OWLCLASS _OWLFASTTHIS _RTTI TStreamableBase;

typedef TStreamer* (* BUILDER)( TStreamableBase * );

struct _OWLCLASS ObjectBuilder {

    enum { NoDelta = -1 };

     ObjectBuilder( BUILDER b, int d ) : Builder( b ), Delta( d ) {}

    BUILDER Builder;
    int Delta;

};

#define __DELTA( d ) (FP_OFF((TStreamable *)(d *)1)-1)

/// TStreamableClass is used by the private database class and pstream in the
/// registration of streamable classes.
class _OWLCLASS TStreamableClass : public ObjectBuilder {
  public:

     TStreamableClass( LPCSTR n,
                             BUILDER b,
                             int d = NoDelta,
                             ModuleId mid = GetModuleId() );

     ~TStreamableClass();

    int  operator == ( const TStreamableClass& n ) const
        {
        if( strcmp( ObjectId, n.ObjectId ) != 0 )
            return 0;
        else
            return (ModId == 0 || n.ModId == 0 || ModId == n.ModId);
        }

    int  operator < ( const TStreamableClass& n ) const
        {
        int res = strcmp( ObjectId, n.ObjectId );
        if( res < 0 )
            return 1;
        else if( res > 0 )
            return 0;
        else if( ModId == 0 || n.ModId == 0 || ModId == n.ModId )
            return 0;
        else
            return ModId < n.ModId;
        }

  private:

    const char* ObjectId;
    ModuleId ModId;

};
class TStreamableClassArray;
class _OWLCLASS TStreamableTypes {
  public:
     TStreamableTypes();
     ~TStreamableTypes();

    void  RegisterType( ModuleId id, TStreamableClass& );
    void  UnRegisterType( ModuleId id, TStreamableClass& );
    const ObjectBuilder *  Lookup( ModuleId id, LPCSTR name ) const;

  private:
    TStreamableClassArray*  Types;
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace


#endif  // OWL_STREAMBLE_H
