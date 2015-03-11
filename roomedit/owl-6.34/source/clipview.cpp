//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TClipboardViewer, a TWindow mixin that allows a
/// window to receive clipboard notifications appropriate for being a viewer
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/clipview.h>

namespace owl {

OWL_DIAGINFO;

//
// We only want to search this mixin for events, so don't include any base
// classes in Find()
//
DEFINE_RESPONSE_TABLE(TClipboardViewer)
  EV_WM_CHANGECBCHAIN,
  EV_WM_DESTROY,
  EV_WM_DRAWCLIPBOARD,
END_RESPONSE_TABLE;

//
/// Constructs a TClipboardViewer object.
//
/// Rely on TWindow's default ctor since we will always be mixed-in and another
/// window will perform Init()
//
TClipboardViewer::TClipboardViewer()
{
  HWndNext = 0;
}

//
/// Constructs a TClipboardViewer object with a handle (hWnd) to the windows that
/// will receive notification when the Clipboard's contents are changed.
//
TClipboardViewer::TClipboardViewer(THandle handle, TModule* module)
:
  TWindow(handle, module)
{
}

//
/// Tests to see if the Clipboard has changed and, if so, DoChangeCBChain forwards
/// this message.
//
TEventStatus
TClipboardViewer::DoChangeCBChain(THandle hWndRemoved, THandle hWndNext)
{
  if (hWndRemoved == HWndNext)
    HWndNext = hWndNext;

  else
    ForwardMessage(HWndNext);
  return esComplete;
}

//
/// Handles EvDrawClipboard messages.
//
TEventStatus
TClipboardViewer::DoDrawClipboard()
{
  if (HWndNext)
    ForwardMessage(HWndNext);
  return esPartial;
}

//
/// Removes the window from the Clipboard-viewer chain.
//
TEventStatus
TClipboardViewer::DoDestroy()
{
  ::ChangeClipboardChain(GetHandle(), HWndNext);
  return esPartial;
}

//
/// Adds a window to the Clipboard-viewer chain.
//
void
TClipboardViewer::SetupWindow()
{
  HWndNext = ::SetClipboardViewer(GetHandle());
}

//
/// Responds to a WM_CHANGECBCHAIN message. hWndRemoved is a handle to the window
/// that's being removed. hWndNext is the window following the removed window.
//
void
TClipboardViewer::EvChangeCBChain(THandle hWndRemoved, THandle hWndNext)
{
  if (hWndRemoved == hWndNext)
    HWndNext = hWndNext;

  else
    ForwardMessage(HWndNext);
}

//
/// Responds to a WM_DRAWCLIPBOARD message sent to the window in the
/// Clipboard-viewer chain when the contents of the Clipboard change.
//
void
TClipboardViewer::EvDrawClipboard()
{
  if (DoDrawClipboard() == esComplete)
    return;
  TWindow::EvDrawClipboard();
}

//
/// Responds to a WM_DESTROY message when a window is removed from the
/// Clipboard-viewer chain.
//
void
TClipboardViewer::EvDestroy()
{
  if (DoDestroy() == esComplete)
    return;
  TWindow::EvDestroy();
}

} // OWL namespace
//////////////////
