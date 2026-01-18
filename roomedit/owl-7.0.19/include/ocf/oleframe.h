//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OCF_OLEFRAME_H)
#define OCF_OLEFRAME_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/decframe.h>
#include <ocf/ocfevent.h>
#include <ocf/ocreg.h>
#include <ocf/olefacto.h>



namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

//
// pad decorations IDs, 4 total: 32042, 32043, 32044, 32045
//
const int IDW_PADDECORATION = 32042;
const int IDW_REMVIEWBUCKET = 32046;  // Window id for remote view holder
const int IDT_DLLIDLE       = 32000;  // Idle timer ID for DLL servers

struct TOcMenuDescr;
class _ICLASS TOcApp;

//
/// \class TOleFrame
// ~~~~~ ~~~~~~~~~
/// Decorated frame that supports OLE 2 using OCF
//
/// Derived from TDecoratedFrame, TOleFrame provides user-interface support for the
/// main window of a Single Document Interface (SDI) OLE application. Because it
/// inherits TDecoratedFrame's functionality, TOleFrame is able to position
/// decorations, such as toolbars, around the client window. Because of its OLE
/// frame functionality, you will always want to create a TOleFrame as a main frame.
/// For example, TOleFrame supports basic container operations, such as
/// - Creating space in a container's frame window that the server has
/// requested
/// - Merging the container's menu and the server's menu
/// - Processing accelerators and other messages from the server's
/// message queue
///
/// In addition to supporting the customary frame window operations and
/// event-handling, TOleFrame provides functionality that supports OLE 2 menu
/// merging for pop-up menus.
///
/// Through the use of the  EvOcXxxx event-handling member functions, TOleFrame
/// responds to ObjectComponents messages sent to both the server and the container
/// applications. Although most of the messages and functions provide container
/// support, one message, EvOcAppShutDown, is server related, and one function,
/// GetRemViewBucket, supplies server support. Whether TOleFrame functions as a
/// container or a server, it always has a pointer to a corresponding TOcApp.
class _OCFCLASS TOleFrame : public owl::TDecoratedFrame {
  public:
    TOleFrame(LPCTSTR    title,
              owl::TWindow*  clientWnd,
              bool      trackMenuSelection = false,
              owl::TModule*  module = 0);
   ~TOleFrame();
    TOcApp*   GetOcApp();
    void      SetOcApp(TOcApp* app);
    void      AddUserFormatName(LPCTSTR name, LPCTSTR resultName, LPCTSTR id);

    owl::TWindow*  GetRemViewBucket();
    void       OleViewClosing(bool close);

  protected:
    void SetupWindow() override;
    void CleanupWindow() override;
    void Destroy(int retVal) override;

    void      EvSize(owl::uint sizeType, const owl::TSize& size);
    void      EvActivateApp(bool active, DWORD threadId);
    void      EvTimer(owl::uint timerId);

    owl::TResult   EvOcEvent(owl::TParam1 param1, owl::TParam2 param2);
    bool      EvOcAppInsMenus(TOcMenuDescr & sharedMenu);
    bool      EvOcAppMenus(TOcMenuDescr & md);
    bool      EvOcAppProcessMsg(MSG * msg);
    bool      EvOcAppFrameRect(owl::TRect * rect);
    bool      EvOcAppBorderSpaceReq(owl::TRect * rect);
    bool      EvOcAppBorderSpaceSet(owl::TRect * rect);
    void      EvOcAppStatusText(LPCTSTR rect);
    void      EvOcAppRestoreUI();
    void      EvOcAppDialogHelp(TOcDialogHelp & dh);
    bool      EvOcAppShutdown();

  protected:
    void      DestroyStashedPopups();
    void      StashContainerPopups(const  owl::TMenuDescr& shMenuDescr);

/// Points to the ObjectComponents application associated with this frame window.
    TOcApp*    OcApp;

/// Holds the stored, shared pop-up menus.
    owl::TMenu      StashedContainerPopups;

/// Holds the number of menu bars that have been stored. This number indicates how
/// many active in-place editing sessions you have open.
    int        StashCount;

/// Stores the handle to the container's previously saved copy of the menu.
    HMENU      HOldMenu;

  private:
    enum {
      DontCare, UserInitiated, ViewInitiated, ServerInitiated
    } OcShutDown;

  DECLARE_RESPONSE_TABLE(TOleFrame);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

// --------------------------------------------------------------------------
// Inline implementations
//

//
/// Gets the ObjectComponents application associated with this frame window.
inline TOcApp*
TOleFrame::GetOcApp()
{
  return OcApp;
}

//
/// Returns a pointer to the OLE frame's hidden helper window that holds all
/// inactive server windows. It can also hold in-place tool bars and TOleView
/// windows.
inline owl::TWindow*
TOleFrame::GetRemViewBucket()
{
  return ChildWithId(IDW_REMVIEWBUCKET);
}

//
/// Responds to an OC_APPDIALOGHELP message. The dh parameter refers to one of the
/// TOcDialogHelp enum constants that indicate the kind of dialog box the user has
/// open. For example, dhBrowseLinks indicates that the Links dialog box is open.
/// The TOcDialogHelp enum lists the help constants and their dialog box
/// equivalents.
inline void
TOleFrame::EvOcAppDialogHelp(TOcDialogHelp & dh)
{
}

} // OCF namespace


#endif  // OWL_OLEFRAME_H
