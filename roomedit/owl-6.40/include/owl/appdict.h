//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TAppDictionary. This class manages associations between
/// processes/tasks and TApplication pointers.
//----------------------------------------------------------------------------

#if !defined(OWL_APPDICT_H)
#define OWL_APPDICT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>

namespace owl {

class _OWLCLASS TApplication;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup module
/// @{
/// \class TAppDictionary
// ~~~~~ ~~~~~~~~~~~~~~
class _OWLCLASS TAppDictionary {
  public:
  	
/// An application dictionary entry that associates a process ID (Pid) with an
/// application (App). The dictionary is indexed by Pid and can have only 1 entry
/// per process ID.
    struct TEntry {
      uint           Pid;
      TApplication*  App;
    };

/// A dictionary iterator function pointer type that receives a reference to an
/// entry. You can supply a function of this type to the Iterate function to iterate
/// through the entries in the dictionary.
    typedef void (*TEntryIterator)(TEntry&);

  public:
    TAppDictionary();
   ~TAppDictionary();

    TApplication* GetApplication(uint pid = 0);  // default to current pid

    void          Add(TApplication* app, uint pid = 0);
    void          Remove(TApplication* app);
    void          Remove(uint pid);
    void          Condemn(TApplication* app);

    bool          DeleteCondemned();
    void          Iterate(TEntryIterator iter);

  private:
    class TAppDictImp*  Imp;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


/// Global exported TAppDictionary in Owl. User Component DLLs should have a
/// similar 'AppDictionary'.
//
extern _OWLFUNC(TAppDictionary&) OWLGetAppDictionary();

//
/// Global function that calls GetApplication() on owl's app-dictionary.
/// Used by EXEs, or DLLs statically linking Owl. Never returns 0, will make
/// an alias app if needed. Primarily for compatibility
//
_OWLFUNC(TApplication*) GetApplicationObject(uint pid = 0);


} // OWL namespace


//
/// Convenient macro to define a 'AppDictionary' ref and object as needed
/// for use in component DLLs and EXEs
//
#if defined(BI_APP_DLL) && defined(_OWLDLL)
# define DEFINE_APP_DICTIONARY(AppDictionary)    \
  ::owl::TAppDictionary  AppDictionary
#else
# define DEFINE_APP_DICTIONARY(AppDictionary)    \
  ::owl::TAppDictionary& AppDictionary = ::owl::OWLGetAppDictionary()
#endif


#endif  // OWL_APPDICT_H
