//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OCF_OLEMDIFR_H)
#define OCF_OLEMDIFR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/oleframe.h>
#include <owl/mdi.h>
#include <memory>

namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TOleMDIFrame
// ~~~~~ ~~~~~~~~~~~~
/// Decorated MDI frame that supports OLE 2 using OCF
//
/// Derived from TMDIFrame and TOleFrame, TOleMDIFrame provides OLE user-interface
/// support for the main window of a Multiple Document Interface (MDI) application.
/// TOleMDIFrame also talks directly to the ObjectComponents classes through the use
/// of a pointer to the OcApp object.
///
/// TOleMDIFrame inherits functionality from TMDIFrame that supports the use of MDI
/// frame windows designed to serve as the main windows of an MDI-compliant
/// application. It also inherits decorated frame window functionality that supports
/// the addition of decorations (such as toolbars and status lines) to the frame
/// window. In addition, TOleMDIFrame inherits the ability to
/// - Create space that the server has requested in a container's frame
/// window
/// - Merge the container's menu into the server's menu
/// - Process accelerators and other messages from the server's message
/// queue
/// - Support OLE 2 menu merging for pop-up menus
///
/// TOleMDIFrame also inherits from TOleFrame the ability to talk directly to the
/// ObjectComponents classes through the use of a pointer to the OcApp object.
///
/// See Step 14 of the ObjectWindows tutorial for an example of a program that uses
/// TOleMDIFrame  to create an OLE-enabled decorated MDI frame window.
class _OCFCLASS TOleMDIFrame : public owl::TMDIFrame, public TOleFrame {
  public:

    TOleMDIFrame(
      LPCTSTR title,
      owl::TResId menuResId,
      std::unique_ptr<owl::TMDIClient> clientWnd = nullptr,
      bool trackMenuSelection = false,
      owl::TModule* = nullptr);

    TOleMDIFrame(
      const owl::tstring& title,
      owl::TResId menuResId,
      std::unique_ptr<owl::TMDIClient> clientWnd = nullptr,
      bool trackMenuSelection = false,
      owl::TModule* = nullptr);

#if defined(OWL5_COMPAT)

    TOleMDIFrame(LPCTSTR      title,
                 owl::TResId       menuResId,
                 owl::TMDIClient&  clientWnd,
                 bool          trackMenuSelection = false,
                 owl::TModule*      module = 0);

#endif

   ~TOleMDIFrame();

  protected:
    virtual owl::TResult DefWindowProc(owl::TMsgId message, owl::TParam1, owl::TParam2);
    bool       EvOcAppInsMenus(TOcMenuDescr & sharedMenu);
    void       EvActivateApp(bool active, DWORD threadId);

  DECLARE_RESPONSE_TABLE(TOleMDIFrame);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

typedef TOleMDIFrame TOleMdiFrame;


} // OCF namespace


#endif  // OWL_OLEMDIFR_H
