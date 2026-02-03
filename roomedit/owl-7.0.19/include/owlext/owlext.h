//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// OWL Extensions TModule-derivative header
//-------------------------------------------------------------------
#ifndef __OWLEXT_OWLEXT_H
#define __OWLEXT_OWLEXT_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


#if defined (_BUILDOWLEXTDLL)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TOWLEXTDll
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TOWLEXTDll : public owl::TModule {
  // Class friendships
  //
  friend int OWLEXTEntryPoint(HINSTANCE, owl::uint32, LPVOID);

  // Object lifetime methods
  //
  public:
    TOWLEXTDll(bool shouldLoad = true, bool mustLoad = false);

  private:
    TOWLEXTDll(HINSTANCE hInstance);      // Only this source module needs this

  public:
    virtual ~TOWLEXTDll();

  // Accessors
  //
  public:
    owl::TModuleProc0<long> GetOWLEXTVersion;
    owl::TModuleVersionInfo& GetVersionInfo();
    static TOWLEXTDll& Instance();

  // Mutators
  //
  public:
    int ThreadAttach();
    int ThreadDetach();

  // Internal data
  //
  private:
};
#endif //#if defined (_BUILDOWLEXTDLL)

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

#if defined (_BUILDOWLEXTDLL)
// Singleton accessor method; does the same as TOWLEXTDll::Instance(), but
// some may find the global function syntax to be cleaner
//
TOWLEXTDll* GetOWLEXTDll();
#endif //#if defined (_BUILDOWLEXTDLL)


} // OwlExt namespace


#endif
