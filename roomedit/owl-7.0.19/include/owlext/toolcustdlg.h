// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// toolcustdlg.h:  header file
// Version:        1.6
// Date:           08.11.1998
// Author:         Dieter Windau
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
// ****************************************************************************

#ifndef __OWLEXT_TOOLCUSTDLG_H
#define __OWLEXT_TOOLCUSTDLG_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#include <owl/template.h>
#ifndef   OWL_INPUTDIA_H
#include <owl/inputdia.h>
#endif
#ifndef   OWL_PROPSHT_H
#include <owl/propsht.h>
#endif
#ifndef   OWL_EDIT_H
#include <owl/edit.h>
#endif
#ifndef   OWL_LISTBOX_H
#include <owl/listbox.h>
#endif

#ifndef __OWLEXT_DOCKINGEX_H
#include <owlext/dockingex.h>
#endif
#ifndef OWL_CHECKLST_H
#include <owl/checklst.h>
#endif
//#ifndef __OWLEXT_MENUBTN_H
//#include <owlext/menubtn.h>
//#endif

namespace OwlExt {

// ****************** Forward declaration *************************************
class OWLEXTCLASS TToolbarInputDialog;
class OWLEXTCLASS TToolbarPage;
class OWLEXTCLASS TCommandsListBox;
class OWLEXTCLASS TCommandsPage;
class OWLEXTCLASS TToolbarCustomizeSheet;
class OWLEXTCLASS TMenuButton;

// ****************** TToolbarInputDialog *************************************

class OWLEXTCLASS TToolbarInputDialog : public owl::TInputDialog
{
  public:
    TToolbarInputDialog(THarborEx* harbor, owl::TWindow* parent, LPCTSTR title,
      LPCTSTR prompt, LPTSTR buffer, int bufferSize, owl::TModule* module = 0);
    ~TToolbarInputDialog();

  protected:
    void EnableOkButton();
    void CmOk();

  protected:
    THarborEx* Harbor;

  DECLARE_RESPONSE_TABLE(TToolbarInputDialog);
};

// ****************** TToolbarPage ********************************************

class OWLEXTCLASS TToolbarPage : public owl::TPropertyPage
{
  public:
    TToolbarPage(TToolbarCustomizeSheet* parent, owl::TResId iconRes = 0,
      owl::TModule* module=0);
    ~TToolbarPage();

    void Update();

  protected:
    void SetupWindow() override;

    void CmToolbar();

    void CmShowTooltips();
    void CmWithAccel();
    void CmDrawGripper();
    void CmFlatStyle();

    void CmNoHints();
    void CmPressHints();
    void CmEnterHints();

    void CmAddToolbar();
    void CmDeleteToolbar();
    void CmRenameToolbar();
    void CmResetToolbar();

    void DisableButtons();

  protected:
    THarborEx*       Harbor;
    owl::TCheckList*      CheckList;
    owl::TCheckListArray  CheckListItems;

  DECLARE_RESPONSE_TABLE(TToolbarPage);
};

// ****************** TCommandsListBox ****************************************

class OWLEXTCLASS TCommandsListBox : public owl::TListBox
{
  public:
    TCommandsListBox(TCommandsPage* parent, int resourceId, THarborEx* harbor,
      owl::TModule* module = 0);

  protected:
    bool         EvEraseBkgnd(HDC hdc);

    // Mouse handlers - needed for drag and drop support
    //
    void         EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);

    void SetupWindow() override;
    void DrawItem(DRAWITEMSTRUCT&) override;

  protected:
    THarborEx*     Harbor;
    TCommandsPage* CommandsPage;

  DECLARE_RESPONSE_TABLE(TCommandsListBox);
};

// ****************** TCommandsPage *******************************************

class OWLEXTCLASS TCommandsPage : public owl::TPropertyPage
{
  public:
    TCommandsPage(TToolbarCustomizeSheet* parent, owl::TResId iconRes = 0,
      owl::TModule* module=0);
    ~TCommandsPage();

    void Update();

  protected:
    void SetupWindow() override;

    void CmCategory();
    void SetDescription(int id);
    void ShowCommands(owl::tstring name);
    auto IdleAction(long idleCount) -> bool override;

  protected:
    THarborEx*        Harbor;
    int               DescId;

    owl::TListBox*         CategoryListBox;
    TCommandsListBox* CommandsListBox;
    owl::TStatic*          Description;
    HMENU             GadgetMenu;
    TMenuButton*      ModifySelectionButton;

  DECLARE_RESPONSE_TABLE(TCommandsPage);
};

// ****************** TToolbarCustomizeSheet **********************************

class OWLEXTCLASS TToolbarCustomizeSheet: public owl::TPropertySheet
{
  friend class TToolbarPage;
  friend class TCommandsPage;

  public:
    TToolbarCustomizeSheet(owl::TWindow* parent, THarborEx* harbor, owl::TModule* module=0);
    ~TToolbarCustomizeSheet();

    void Update();

  protected:
    void SetupWindow() override;
    void Destroy(int retVal = 0) override;

  protected:
    THarborEx*     Harbor;
    TToolbarPage*  ToolbarPage;
    TCommandsPage* CommandsPage;

  DECLARE_RESPONSE_TABLE(TToolbarCustomizeSheet);
};


} // OwlExt namespace

#endif
