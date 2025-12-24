// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1997, 98 by Dieter Windau
// All rights reserved
//
// dlgtoolbox.h: header file
// Version:      1.0
// Date:         24.09.1998
// Author:       Dieter Windau (used with permision)
//
// Collection of freeware OWL button classes
// TDialogToolBox is derived from TToolBox and can be used inside a dialog
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
#if !defined (__OWLEXT_DLGTOOLB_H) && !defined (__DLGTOOLBOX_H)
#define __OWLEXT_DLGTOOLB_H
#define __DLGTOOLBOX_H

#include <owlext/defs.h>

#ifndef   OWL_TOOLBOX_H
#include <owl/toolbox.h>
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


class OWLEXTCLASS TDialogToolBox;

// ************************* TDialogToolBox ***********************************

class OWLEXTCLASS TDialogToolBox : public owl::TToolBox {
  public:
    TDialogToolBox(owl::TWindow* parent, owl::uint resourceId, int numColumns = 2,
                   int numRows = owl::AS_MANY_AS_NEEDED,
                   TTileDirection direction = Horizontal);
    ~TDialogToolBox();

  protected:
    void SetupWindow();
    owl::TResult EvCommand(owl::uint id, THandle hWndCtl, owl::uint notifyCode);

  protected:
    owl::uint ResourceId;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif // __OWLEXT_DLGTOOLB_H
