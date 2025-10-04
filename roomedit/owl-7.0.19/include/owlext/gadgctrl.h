// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// gadgetctrl.h: header file
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
// ****************************************************************************

#ifndef __OWLEXT_GADGCTRL_H
#define __OWLEXT_GADGCTRL_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#include <owl/combobox.h>
#include <owl/edit.h>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ****************** Forward declaration *************************************
class OWLEXTCLASS TGadgetComboBox;
class OWLEXTCLASS TGadgetEdit;

// ******************** TGadgetCombobox ***************************************
#define TGadgetComboBoxMessage _T("TGADGETCOMBOBOXMESSAGE")

class OWLEXTCLASS TGadgetComboBox : public owl::TComboBox {
  public:
    static bool AutoMainWindowNotify;
  public:
    TGadgetComboBox(owl::TWindow* parent, int id, int x, int y, int w, int h,
      owl::uint32 style, owl::uint textLimit, owl::TModule* module=0);
    virtual ~TGadgetComboBox();

    void Ok();
    void Cancel();

    auto IdleAction(long idleCount) -> bool override;

  protected:
    void SetupWindow() override;

    void EvSetFocus(HWND hWndLostFocus);

    void CbnCloseUp();
    void CbnSelChange();

    HBRUSH EvCtlColor(HDC hDC, THandle hWndChild, owl::uint ctlType);
    auto PreProcessMsg(MSG& msg) -> bool override;
    void RedrawBorder();
    void EvPaint();

  protected:
    static owl::uint GadgetComboBoxMessage;

  protected:
    int    OldIdx;    // Index, when the combox get the focus. Need for ESC
    LPTSTR OldString; // String, when the combox get the focus. Need for ESC
    bool   MouseInControl;
    HWND   hwndEdit;

   DECLARE_RESPONSE_TABLE(TGadgetComboBox);
};

// ******************** TGadgetEdit *******************************************
#define TGadgetEditMessage _T("TGADGETEDITMESSAGE")

class OWLEXTCLASS TGadgetEdit : public owl::TEdit {
  public:
    static bool AutoMainWindowNotify;
  public:
    TGadgetEdit(owl::TWindow* parent, int id, LPCTSTR text, int x, int y,
      int w, int h,  owl::uint textLimit=0, bool multiline=false, owl::TModule* module=0);
    virtual ~TGadgetEdit();

    void Ok();
    void Cancel();

    auto IdleAction(long idleCount) -> bool override;

  protected:
    void SetupWindow() override;
    auto PreProcessMsg(MSG&) -> bool override;
    void RedrawBorder();
    void EvPaint();
    void EvSetFocus(HWND hWndLostFocus);

  protected:
    static owl::uint GadgetEditMessage;

  protected:
    LPTSTR OldString; // String, when the combox get the focus. Need for ESC
    bool   MouseInControl;

   DECLARE_RESPONSE_TABLE(TGadgetEdit);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>
} // OwlExt namespace

#endif
