//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TPictDialog
//
//-------------------------------------------------------------------
#ifndef __OWLEXT_PICTDLG_H
#define __OWLEXT_PICTDLG_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif

#include <owl/dialog.h>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                   TPictDialog
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TPictDialog : public owl::TDialog
{
  // Object lifetime methods
  //
public:
  TPictDialog(owl::TDib* dib, owl::TWindow* parent, owl::TResId resId, owl::TModule* module = 0);
  virtual ~TPictDialog();

  // OWL overrides
  //
protected:
  auto GetWindowClassName() -> owl::TWindowClassName override { return owl::TWindowClassName{_T("PictDlg")}; }
  DECLARE_RESPONSE_TABLE(TPictDialog);
  virtual bool EvEraseBkgnd(HDC hdc);

  // Accessors
  //
public:
  owl::TDib* GetDib()
    { return m_pdib; }

  // Mutators
  //
public:
  owl::TDib* SetDib(owl::TDib* newDib);

  // Internal data
  //
private:
  owl::TDib* m_pdib;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif
