//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software
// All rights reserved.
//
//  Description:  An editable listbox. Double clicking an item lets you edit
//                it. An EN_CHANGE notification is sent to the parent when
//                the user finishes editing.
//
// Original code by Chris Kohloff; used with permission:
//  Author:       Christopher Kohlhoff (chris@tenermerx.com)
//
//  Date:         30 May 1997
//
//-------------------------------------------------------------------
#ifndef __OWLEXT_EDITLIST_H
#define __OWLEXT_EDITLIST_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
# include <owlext/core.h>
#endif


// Grab necessary OWL headers
//
#if !defined(OWL_LISTBOX_H)
# include <owl/listbox.h>
#endif


namespace owl {class _OWLCLASS TEdit;};  // forward declaration

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                     TEditList
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class OWLEXTCLASS TEditList : public owl::TListBox {
  // Object lifetime methods
  //
  public:
    TEditList(owl::TWindow* parent, int id, int x, int y, int w, int h, owl::TModule* module = 0);
    TEditList(owl::TWindow* parent, int id, owl::TModule* module = 0);
    virtual ~TEditList();

  // Accessors
  //
  public:
    owl::TEdit* GetEdit();

  // Mutators
  //
  public:
    void BeginEdit();
    void EndEdit();

  // OWL overridden methods
  //
  public:
    void SetupWindow() override;
    void EvLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);
    void EditKillFocus();

  // Internal data
  //
  protected:
    owl::TEdit* Edit;

  DECLARE_RESPONSE_TABLE(TEditList);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

inline owl::TEdit* TEditList::GetEdit(){
  return Edit;
}

} // OwlExt namespace

#endif  //__OWLEXT_COMBOEXP_H

