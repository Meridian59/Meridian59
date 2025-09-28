//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TDecoratedMDIFrame
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/decmdifr.h>
#include <type_traits>

namespace owl {

OWL_DIAGINFO;

//
// make sure that TDecoratedFrame is first in the list so we find its
// WM_SIZE handler
//
DEFINE_RESPONSE_TABLE2(TDecoratedMDIFrame, TDecoratedFrame, TMDIFrame)
END_RESPONSE_TABLE;

//
/// Constructs a decorated MDI frame of the specified client window with the
/// indicated menu resource ID. By default, menu hint text is not displayed.
///
/// \note If nullptr is passed for `clientWnd`, then a default TMDIClient is created.
//
TDecoratedMDIFrame::TDecoratedMDIFrame(LPCTSTR title, TResId menuResId, std::unique_ptr<TMDIClient> clientWnd, bool trackMenuSelection, TModule* module)
  : TMDIFrame{title, menuResId, move(clientWnd), module},
  TDecoratedFrame{nullptr, title, ClientWnd, trackMenuSelection, module}
{}

//
/// String-aware overload.
//
TDecoratedMDIFrame::TDecoratedMDIFrame(const tstring& title, TResId menu, std::unique_ptr<TMDIClient> clientWnd, bool trackMenuSelection, TModule* module)
  : TDecoratedMDIFrame{title.c_str(), menu, move(clientWnd), trackMenuSelection, module}
{}

#if defined(OWL5_COMPAT)

//
/// Old OWL 5 constructor.
/// Use the new safe constructor instead.
//
TDecoratedMDIFrame::TDecoratedMDIFrame(LPCTSTR         title,
                                       TResId          menuResId,
                                       TMDIClient&     clientWnd,
                                       bool            trackMenuSelection,
                                       TModule*        module)
:
  TWindow(0, title, module),
  TFrameWindow(0, title, &clientWnd, false, module),
  TMDIFrame(title, menuResId, clientWnd, module),
  TDecoratedFrame(0, title, &clientWnd, trackMenuSelection, module)
{
}

//
/// String-aware overload of the old OWL 5 constructor.
/// Use the new safe constructor instead.
//
TDecoratedMDIFrame::TDecoratedMDIFrame(const tstring& title, TResId menu, TMDIClient& clientWnd, bool trackMenuSelection, TModule* module)
:
  TWindow(0, title.c_str(), module),
  TFrameWindow(0, title.c_str(), &clientWnd, false, module),
  TMDIFrame(title.c_str(), menu, clientWnd, module),
  TDecoratedFrame(0, title.c_str(), &clientWnd, trackMenuSelection, module)
{}

#endif

//
/// Overrides TWindow::DefWindowProc . If the message parameter is WM_SIZE,
/// DefWindowProc returns 0; otherwise, DefWindowProc returns the result of calling
/// TMDIFRame::DefWindowProc.
//
TResult
TDecoratedMDIFrame::DefWindowProc(TMsgId message, TParam1 param1, TParam2 param2)
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



IMPLEMENT_STREAMABLE4(TDecoratedMDIFrame, TMDIFrame, TDecoratedFrame,
                      TFrameWindow, TWindow);
#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TDecoratedMDIFrame::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TMDIFrame*)GetObject(), is);
  ReadBaseObject((TDecoratedFrame*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TDecoratedMDIFrame::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TMDIFrame*)GetObject(), os);
  WriteBaseObject((TDecoratedFrame*)GetObject(), os);
}

#endif


} // OWL namespace

