//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Thanks to Ron Reynolds of Access Health, Inc. for the assist.
//
// TDialogApp
//-------------------------------------------------------------------
#if !defined(__OWLEXT_DLGAPP_H)
#define __OWLEXT_DLGAPP_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif

#include <assert.h>
#include <owl/applicat.h>
#include <owl/framewin.h>
#include <owl/dialog.h>
#include <owl/functor.h>    // Make sure you link with VDBT

namespace OwlExt {

// DialogAppFunctor is a ptr to a fn taking nothing and returning a TDialog*
typedef owl::TFunctor0<owl::TDialog*> DialogAppFunctor;

#define DialogApp_FUNCTOR(func) TFunctionTranslator0<TDialog*,TDialog* (*)()>(func)

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// TDialogApp
//
class OWLEXTCLASS TDialogApp : public owl::TApplication
{
  // Object lifetime methods
  //
public:
  TDialogApp(DialogAppFunctor dlgCreator, LPCTSTR title = 0 )
    : owl::TApplication(title), m_creatorFunctor(dlgCreator)
  { }

  // OWL mutators
  //
protected:
  virtual void InitMainWindow()
  {
    // Create the dialog
    owl::TDialog* dlg = m_creatorFunctor();

    // Create our main window
    SetMainWindow( new owl::TFrameWindow(0, GetName(), dlg, true));
  }

  // Internal data
  //
protected:
  DialogAppFunctor m_creatorFunctor;

  // Explicitly disallowed methods
  //
private:
  TDialogApp(const TDialogApp&);        // DISALLOWED METHOD
  void operator=(const TDialogApp&);    // DISALLOWED METHOD
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif