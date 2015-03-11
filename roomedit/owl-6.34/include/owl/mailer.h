//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Class definition for implementation of Mail enabling.
//----------------------------------------------------------------------------

#if !defined(OWL_MAILER_H)
#define OWL_MAILER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/module.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TMailer
// ~~~~~ ~~~~~~~
/// The TMailer class encapsulates the MAPI DLL (MAPI [32].DLL). It provides an easy
/// method to dynamically test for the availability of the DLL and bind to its
/// exported functions at runtime. By using the TMailer class instead of direct
/// calls to the MAPI DLL, ObjectWindows applications can provide the appropriate
/// behavior when running in an environment where the DLL is not available.
/// 
/// Each data member of the TMailer class corresponds to the API with a similar name
/// exposed by the MAPI DLL. For example, TMailerDll::MAPISendDocuments corresponds
/// to the MAPISendDocuments API exported by the MAPI DLL.
/// The following is a list of the members of the TMailer class corresponding to
/// functions exposed by the DLL. For more information about these members, consult
/// the documentation about the corresponding API exposed by the MAPI DLL.
//
class _OWLCLASS TMailer : public TModule {
  public:
    TMailer();

    bool  IsMAPIAvailable() const;
    void  SendDocuments(TWindow* owner, LPCTSTR paths,
                        LPCTSTR names = 0, bool asynchWork = false);

    void  SendDocuments(TWindow* owner, const tstring& paths, const tstring& names = tstring(), bool asynchWork = false)
    {SendDocuments(owner, paths.c_str(), names.empty() ? 0 : names.c_str(), asynchWork);}

    TModuleProc5<uint32,uint32,LPSTR,LPSTR,LPSTR,uint32> MAPISendDocuments;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_MAILER_H
