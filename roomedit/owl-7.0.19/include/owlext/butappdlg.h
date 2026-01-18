// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// butappdlg.h:  header file
// Version:      1.6
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
//
// ****************************************************************************
#if !defined(__OWLEXT_BUTAPPDLG_H)
#define __OWLEXT_BUTAPPDLG_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#ifndef   OWL_EDIT_H
#include <owl/edit.h>
#endif
#ifndef   OWL_LISTBOX_H
#include <owl/listbox.h>
#endif

#ifndef __OWLEXT_GADGDESC_H
#include <owlext/gadgdesc.h>
#endif

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ****************** Forward declaration *************************************
class OWLEXTCLASS TButtonGadgetEx;
class OWLEXTCLASS TButtonTextGadgetEx;

class OWLEXTCLASS TImageListBox;
class OWLEXTCLASS TButtonAppearanceDialog;

// ****************** TImageListbox *******************************************

class OWLEXTCLASS TImageListBox : public owl::TListBox
{
  public:
    TImageListBox(owl::TWindow* parent, int resourceId, owl::TCelArray* array=0,
                  owl::TIntArray* idsaray = 0, owl::TModule* module = 0);
    virtual ~TImageListBox();

    virtual void BuildIdsArray(owl::TResId& resId, int maxitems=0);
    virtual void BuildCelArray();
    // Set the  bitmaps by DWORD data. Call this function if all items are added

  protected:
    bool         EvEraseBkgnd(HDC hdc);
    void DrawItem(DRAWITEMSTRUCT&) override;

  protected:
    owl::TCelArray*  CelArray;
    owl::TIntArray*  IdsArray;

  DECLARE_RESPONSE_TABLE(TImageListBox);
};

// ****************** TButtonAppearanceDialog *********************************

class OWLEXTCLASS TButtonAppearanceDialog : public owl::TDialog {
  public:
    TButtonAppearanceDialog(owl::TWindow* parent,
                            TButtonTextGadgetEx* gadget,
                            owl::TCelArray* celaray=0,
                            owl::TIntArray* idsaray = 0,
                            owl::TModule* module=0);

  protected:
    void SetupWindow() override;

    void CmOk();
    void CmImage();
    void CmText();
    void CmImageText();
    void CmButtonText();

    void EnableControls();

  protected:

    TImageListBox*       ImageListBox;
    owl::TEdit*               ButtonEdit;
    TButtonTextGadgetEx* ButtonTextGadget;
    TDisplayType         DisplayType;
    owl::tstring           ButtonText;
    owl::TResId               BmpResId;

  DECLARE_RESPONSE_TABLE(TButtonAppearanceDialog);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif
