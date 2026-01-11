//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TOleMDIFrame.
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <owl/decmdifr.h>
#include <owl/statusba.h>
#include <ocf/ocfevent.h>
#include <ocf/olemdifr.h>
#include <type_traits>

namespace ocf {

using namespace owl;

using namespace std;

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OcfOleMenu);

DEFINE_RESPONSE_TABLE2(TOleMDIFrame, TOleFrame, TMDIFrame)
  EV_WM_ACTIVATEAPP,
  EV_OC_APPINSMENUS,
END_RESPONSE_TABLE;

//
/// Constructs a TOleMDIFrame object with the indicated title, menu resource ID,
/// client window, and module instance. By default, because trackMenuSelection is
/// false, menu hint text is not displayed. (These parameters coincide with those of
/// TMDIFrame's constructor.)
///
/// \note If nullptr is passed for `clientWnd`, then a default TMDIClient is created.
//
TOleMDIFrame::TOleMDIFrame(LPCTSTR title, TResId menuResId, unique_ptr<TMDIClient> clientWnd, bool trackMenuSelection, TModule* module)
  : TMDIFrame{title, menuResId, move(clientWnd), module},
  TOleFrame{title, ClientWnd, trackMenuSelection, module}
{}

//
/// String-aware overload.
//
TOleMDIFrame::TOleMDIFrame(const tstring& title, TResId menuResId, unique_ptr<owl::TMDIClient> clientWnd, bool trackMenuSelection, TModule* module)
  : TOleMDIFrame{title.c_str(), menuResId, move(clientWnd), trackMenuSelection, module}
{}

#if defined(OWL5_COMPAT)

//
/// Old OWL 5 constructor.
/// Use the new safe constructor instead.
//
TOleMDIFrame::TOleMDIFrame(LPCTSTR       title,
                           TResId       menuResId,
                           TMDIClient&  clientWnd,
                           bool         trackMenuSelection,
                           TModule*     module)
  : TMDIFrame{title, menuResId, clientWnd, module},
  TOleFrame{title, &clientWnd, trackMenuSelection, module}
// !CQ when did this change?
//  ,TFrameWindow(0, title, &clientWnd, false, module),
//  TWindow(0, title, module)
{
}

#endif

//
/// Destroys the OLE MDI frame window object.
//
TOleMDIFrame::~TOleMDIFrame()
{
}

//
/// Responds to a message indicating that the frame window of this application
/// (hTask) is going to be either activated (active is true) or deactivated (active
/// is false), and forwards this information to the TOcApp object.
//
void
TOleMDIFrame::EvActivateApp(bool active, DWORD threadId)
{
  OcApp->EvActivate(active);
  TMDIFrame::EvActivateApp(active, threadId);
}

//
/// Inserts menus into a provided menu bar, or merges them with a child window and
/// servers. To do this, EvOcAppInsMenus creates a temporary composite menu for the
/// frame and MDI child windows, then copies the shared menu widths to the
/// ObjectComponents structure. It saves the container popups so they can be
/// destroyed later.
//
bool
TOleMDIFrame::EvOcAppInsMenus(TOcMenuDescr & sharedMenu)
{
  if (HOldMenu)
    return true;

  // Recreate a temporary composite menu for frame and MDI child
  //
  TMenuDescr compMenuDesc; // empty menudescr
  if (GetMenuDescr())
    compMenuDesc.Merge(*GetMenuDescr());

  const TMenuDescr* childMenu = GetClientWindow()->GetActiveMDIChild()->GetMenuDescr();

  if (childMenu)
    compMenuDesc.Merge(*childMenu);

  // Mask off the server menus
  //
  compMenuDesc.Merge(TMenuDescr(0,  0, -1, 0, -1, 0, -1));

  // Merge into the OLE shared menubar
  //
  TMenuDescr shMenuDescr(sharedMenu.HMenu,
                         sharedMenu.Width[0],
                         sharedMenu.Width[1],
                         sharedMenu.Width[2],
                         sharedMenu.Width[3],
                         sharedMenu.Width[4],
                         sharedMenu.Width[5]);
  shMenuDescr.Merge(compMenuDesc);

  // Copy the shared menu widths back to the OC struct
  //
  for (int i = 0; i < 6; i++)
    sharedMenu.Width[i] = shMenuDescr.GetGroupCount(i);

  // Save the container popups so they can be destroyed later
  //
  StashContainerPopups(shMenuDescr);

  TRACEX(OcfOleMenu, 0, _T("MDI merged menu ") << hex << static_cast<void*>(sharedMenu.HMenu));
  return true;
}

//
/// Allows default processing for all messages except for a resizing message
/// concerning the frame window, in which case, DefWindowProc returns nothing.
//
TResult
TOleMDIFrame::DefWindowProc(TMsgId message, TParam1 param1, TParam2 param2)
{
  //
  // ::DefFrameProc() will response to WM_SIZE by making the MDI client the
  // same size as the client rectangle; this conflicts with what TLayoutWindow
  // has done
  //
  return message == WM_SIZE ?
                      0 :
                      TMDIFrame::DefWindowProc(message, param1, param2);
}

} // OCF namespace

//==============================================================================

