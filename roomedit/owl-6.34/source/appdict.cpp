//---------------------------------------------------------------------------- 
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TAppDictionary, a dictionary of associations
/// between pids (Process IDs) and TApplication pointers.
/// Used to support GetApplicationObject().
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/appdict.h>
#include <owl/applicat.h>
#if defined(BI_APP_DLL)
#  if !defined(__CYGWIN__) &&  !defined(WINELIB)
#    include <dos.h>
#  endif
# include <string.h>
#endif

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;

// Defining LOCALALLOC_TABLE instructs OWL to use LocalAlloc for the table
//
//#define LOCALALLOC_TABLE

/// Global dictionary used by OWL for EXE Application lookup
//
_OWLFUNC(TAppDictionary&)
OWLGetAppDictionary()
{
  static TAppDictionary OwlAppDictionary;
  return OwlAppDictionary;
};

//----------------------------------------------------------------------------

//
/// Dictionary implementation used to associate Pids (hTasks) with running Owl
/// apps when Owl is in a DLL or used by a DLL. 32bit only needs this when
/// running win32s (no per-instance data) since per-instance data makes it
/// unnecesasry
//

#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif

/// \addtogroup internal
/// @{
//
/// Abstract Base for dictionary implementation
//
class TAppDictImp {
  public:
    virtual ~TAppDictImp() {}

    virtual void                       Add(unsigned pid, TApplication* app) = 0;
    virtual void                       Remove(unsigned pid) = 0;
    virtual TAppDictionary::TEntry*    Lookup(unsigned pid) = 0;
    virtual TAppDictionary::TEntry*    Lookup(TApplication* app) = 0;
    virtual void                       Iterate(TAppDictionary::TEntryIterator) = 0;
    virtual int                        GetCount() const = 0;
};

//
/// Fast, small, per-instance data based Dictionary implementation (32bit only)
//
class TAppDictInstImp : public TAppDictImp {
  public:
    TAppDictInstImp() {Entry.App = 0;}

    void                       Add(unsigned pid, TApplication* app) {Entry.App = app;}
    void                       Remove(unsigned) {Entry.App = 0;}
    TAppDictionary::TEntry*    Lookup(unsigned) {return &Entry;}
    TAppDictionary::TEntry*    Lookup(TApplication* app) {return &Entry;}
    void                       Iterate(TAppDictionary::TEntryIterator iter)
                                 {(*iter)(Entry);}
    int                        GetCount() const {return Entry.App ? 1 : 0;}

  private:
    TAppDictionary::TEntry     Entry;
};
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif

/// @}

//} // OWL namespace

//----------------------------------------------------------------------------
/// \class TAppDictionary
/// TAppDictionary implementation for DLLs only. EXE version is all inline.
/// Flat model must implement here, not inline, because same lib is used by DLLs
///
/// A TAppDictionary is a dictionary of associations between a process ID and an
/// application object. A process ID identifies a process: a program (including all
/// its affiliated code, data, and system resources) that is loaded into memory and
/// ready to execute. A TAppDictionary object supports global application lookups
/// using the global  GetApplicationObject function or TAppDictionary's
/// GetApplication function. If you do not define an application dictionary,
/// ObjectWindows provides a default, global application dictionary that is
/// exported. In fact, for .EXEs, this global application dictionary is
/// automatically used.
/// 
/// TAppDictionary includes a TEntry struct, which stores a process ID and the
/// corresponding application object associated with the ID. The public member
/// functions add, find, and remove the entries in the application dictionary.
/// 
/// If you are statically linking ObjectWindows, you do not have to explicitly
/// create an application dictionary because the default global ObjectWindows
/// application dictionary is used. However, when writing a DLL component that is
/// using ObjectWindows in a DLL, you do need to create your own dictionary. To make
/// it easier to define an application dictionary, ObjectWindows includes a macro
/// DEFINE_APP_DICTIONARY, which automatically creates or references the correct
/// dictionary for your application.
/// 
/// Although this class is transparent to most users building EXEs, component DLLs
/// need to create an instance of a TApplication class for each task that they
/// service. This kind of application differs from an .EXE application in that it
/// never runs a message loop. (All the other application services are available,
/// however.)
/// 
/// Although a component may consist of several DLLs, each with its own TModule, the
/// component as a whole has only one TApplication for each task. A TAppDictionary,
/// which is used for all servers (including DLL servers) and components, lets users
/// produce a complete, self-contained application or component. By using a
/// TAppDictionary, these components can share application objects.
/// 
/// When 16-bit ObjectWindows is statically linked with an .EXE or under Win32, with
/// per- instance data, the TAppDictionary class is implemented as a wrapper to a
/// single application pointer. In this case, there is only one TApplication that
/// the component ever sees.
/// 
/// To build a component DLL using the ObjectWindows DLL, a new TAppDictionary
/// object must be constructed for that DLL. These are the steps an application must
/// follow in order to associate the component DLL with the TAppDictionary, the
/// application, and the window class hierarchy:
/// 1.	 Use the DEFINE_APP_DICTIONARY macro to construct an instance of
/// TAppDictionary. Typically, this will be a static global in one of the
/// application's modules (referred to as "AppDictionary"). The DEFINE_DICTIONARY
/// macro allows the same code to be used for EXEs and DLLs.
/// DEFINE_APP_DICTIONARY(AppDictionary);
/// 2.	Construct a generic TModule and assign it to the global ::Module. This is
/// the default provided in the ObjectWindows' LibMain function.
/// LibMain(...)
///   ::Module  = new TModule(0, hInstance);
/// 3.	When each TApplication instance is constructed, pass a pointer to the
/// TAppDictionary as the last argument to ensure that the application will insert
/// itself into this dictionary. In addition, for 16 bit DLLs, the global module argument
/// needs to be supplied with a placeholder value because the Module construction
/// has already been completed at this point, as a result of the process performed
/// in step 2 above.
/// TApplication* app = new TMyApp(..., app, AppDictionary);
/// 4.	If the Doc/View model is used, supply the application pointer when
/// constructing the TDocManager object.
/// SetDocManager(new TDocManager(mode, this));
/// 5.	When a non-parented window (for example, the main window) is being
/// constructed, pass the application as the module.
/// SetMainWindow(new TFrameWindow(0, "", false, this));

//
/// Application dictionary constructor
//
TAppDictionary::TAppDictionary()
{
    Imp = new TAppDictInstImp();   // Could also use this case if linked to exe
}

//
/// Destroys the TAppDictionary object and calls DeleteCondemned to clean up the
/// condemned applications.
//
TAppDictionary::~TAppDictionary()
{
  DeleteCondemned();
  delete Imp;
}

//
/// Looks up and returns the application associated with a given process ID. The
/// default ID is the ID of the current process. If no application is associated
/// with the process ID, GetApplication returns 0.
//
TApplication*
TAppDictionary::GetApplication(uint pid)
{
  if (!pid)
    pid = ::GetCurrentProcessId();
  TAppDictionary::TEntry* entry = Imp->Lookup(pid);
  return entry ? entry->App : 0;
}

//
/// Adds an application object (app) and corresponding process ID to this
/// dictionary. The default ID is the current process's ID.
//
void
TAppDictionary::Add(TApplication* app, uint pid)
{
  if (!pid)
    pid = ::GetCurrentProcessId();
  Imp->Add(pid, app);
}

//
/// Searches for the dictionary entry using the specified application (app). Then
/// removes a given application and process ID entry from this dictionary, but does
/// not delete the application object.
//
void
TAppDictionary::Remove(TApplication* app)
{
  TAppDictionary::TEntry* entry = Imp->Lookup(app);
  if (entry) {
    entry->App = 0;
    entry->Pid = 0;
  }
}

//
/// Searches for the dictionary entry using the specified process ID (pid). Then
/// removes a given application and its associated process ID entry from this
/// dictionary, but does not delete the application.
//
void
TAppDictionary::Remove(uint pid)
{
  TAppDictionary::TEntry* entry = Imp->Lookup(pid);
  if (entry) {
    entry->App = 0;
    entry->Pid = 0;
  }
}

//
/// Marks an application in this dictionary as condemned by zeroing its process ID
/// so that the application can be deleted later when DeleteCondemned is called.
//
void
TAppDictionary::Condemn(TApplication* app)
{
  TAppDictionary::TEntry* entry = Imp->Lookup(app);
  if (entry)
    entry->Pid = 0;
}

//
//
//
static void _DeleteCondemnedIter(TAppDictionary::TEntry& entry)
{
  if (!entry.Pid) {
    delete entry.App;
    entry.App = 0;
  }
}

//
/// Iterates through the entries in the application dictionary, calling the iter 
/// callback function for each entry.
//
void
TAppDictionary::Iterate(TAppDictionary::TEntryIterator iter)
{
  Imp->Iterate(iter);
}

//
/// Deletes all condemned applications from the dictionary. If no applications
/// remain in the dictionary, DeleteCondemned returns true.
//
bool
TAppDictionary::DeleteCondemned()
{
  Imp->Iterate(_DeleteCondemnedIter);
  return Imp->GetCount() == 0;
}

} // OWL namespace
//
/// Exported entry for Debugger use
//
extern "C" _OWLFUNC(owl::TApplication*)
GetTaskApplicationObject(unsigned pid)
{
  return owl::OWLGetAppDictionary().GetApplication(pid);
}

namespace owl {



//----------------------------------------------------------------------------

//
/// Global function that calls GetApplication() on OWL's app-dictionary.
/// Used by EXEs, or DLLs statically linking OWL. Never returns 0, will make
/// an alias app if needed. Primarily for compatibility
//
_OWLFUNC(TApplication*) GetApplicationObject(unsigned pid)
{
  TApplication* app = OWLGetAppDictionary().GetApplication(pid);
//  CHECK(app);
	WARN(app == 0, _T("OWLGetAppDictionary().GetApplication(pid) returned NULL"));
  if (app)
    return app;

  // Make alias app (will add itself to dictionary) because OWL always needs an
  // app around. If the app is non-OWL, no TApplication will have been
  // constructed.
  // Override default constructor argument to prevent overwrite of ::Module,
  // and pass the default dictionary.
  //
  TModule* tempModule;
  return new TApplication(_T("ALIAS"), tempModule, &(OWLGetAppDictionary()));
}

} // OWL namespace

