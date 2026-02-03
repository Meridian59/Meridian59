// ****************************************************************************
// Copyright (C) 1997, 98 by Dieter Windau
// All rights reserved
//
// menubtn.h:    header file
// Version:      1.1
// Date:         24.10.1998
// Author:       Dieter Windau
//
// Collection of freeware OWL button classes
// TMenuButton shows a popup menu when the user click on the button
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02 and Windows NT 4.0 SP3
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************

#ifndef __OWLEXT_MENUBTN_H
#define __OWLEXT_MENUBTN_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#ifndef __OWL_GLYPHBTN_H
#include <owl/glyphbtn.h>
#endif

namespace OwlExt {

class OWLEXTCLASS TMenuButton;

// ********************* TMenuButton ******************************************

class OWLEXTCLASS TMenuButton : public owl::TGlyphButton
{
  public:
    TMenuButton(owl::TWindow* parent, int Id, LPCTSTR text, int X, int Y,
      int W, int H, bool isDefault = false, owl::TModule* module = 0);
    TMenuButton(owl::TWindow* parent, int resourceID, owl::TModule* module = 0);

    void SetMenu(HMENU hMenu);
    // Set the popupMenu. Command Target is the parent window.

    void SetMenu(HMENU hMenu, HWND CmdTarget);
    // Set the popupMenu and the commando target window

  protected:

    // Event handlers
    //
    owl::TResult      BmSetState(owl::TParam1 param1, owl::TParam2 param2);

    virtual void TrackPopupMenu();

  protected:
    HMENU HMenu;
    HWND  CmdTarget;

  DECLARE_RESPONSE_TABLE(TMenuButton);
};

} // OwlExt namespace

#endif
