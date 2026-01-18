//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TFloatingFrame.
//----------------------------------------------------------------------------

#if !defined(OWL_FLOATFRA_H)
#define OWL_FLOATFRA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/framewin.h>
#include <owl/tinycapt.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup frame
/// @{
//
/// \class TFloatingFrame
// ~~~~~ ~~~~~~~~~~~~~~
/// Derived from TFrameWindow and TTinyCaption, TFloatingFrame implements a floating
/// frame that can be positioned anywhere in the parent window. Except for the
/// addition of a tiny caption bar, the default behavior of TFrameWindow and
/// TFloatingFrame is the same. Therefore, an application that uses TFrameWindow can
/// easily gain the functionality of TFloatingFrame by just changing the name of the
/// class to TFloatingFrame.
///
/// If there is a client window, the floating frame shrinks to fit the client
/// window, leaving room for margins on the top, bottom, left, and right of the
/// frame. Because the floating frame expects the client window to paint its own
/// background, it does nothing in response to a WM_ERASEBKGND message. However, if
/// there is no client window, the floating frame erases the client area background
/// using TColor::SysUIFace.
///
/// If popupPalette is true then, dragFrame is enabled, styles are set to popup
/// with a tiny close box, and a border.
///
/// When dragFrame mode is enabled the floating frame adds an extra non-client
/// margin around the outside to allow moving of the frame. This works well with
/// a thin frame and is nice when the client area is too full to allow dragging
/// from there.
//
class _OWLCLASS TFloatingFrame : public TFrameWindow, public TTinyCaption {
  public:
    TFloatingFrame(TWindow* parent,
                   LPCTSTR  title = 0,
                   TWindow* clientWnd = 0,
                   bool     shrinkToClient = false,
                   int      captionHeight = DefaultCaptionHeight,
                   bool     popupPalette = false,
                   TModule* module = 0);

    TFloatingFrame(
      TWindow* parent,
      const tstring& title,
      TWindow* client = 0,
      bool shrinkToClient = false,
      int captionHeight = DefaultCaptionHeight,
      bool popupPalette = false,
      TModule* = 0);

    void    SetMargins(const TSize& margin);
    void    SetDragFrame(bool dragFrame);

    // Nameless union to declare a constant that is class scoped.
    //
    enum {
      DefaultCaptionHeight = 0
    };

  protected:

    void Init(int captionHeight, bool popupPalette);

    // Override TWindow & TFrameWindow virtuals
    //
    auto GetCommandTarget() -> HWND override;
    void EvCommandEnable(TCommandEnabler&) override;
    auto EvCommand(uint id, HWND hWndCtl, uint notifyCode) -> TResult override;

    /// Event handler helper
    //
    TEventStatus  DoNCHitTest(const TPoint& screenPt, uint& evRes);

    //      Event Handlers
    //
    void    EvSysCommand(uint cmdType, const TPoint& p);
    uint    EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize);
    void     EvNCPaint(HRGN);
    uint    EvNCHitTest(const TPoint& screenPt);
    bool    EvNCActivate(bool active);
    uint    EvMouseActivate(HWND hTopLevel, uint hitCode, TMsgId);
    void    EvActivateApp(bool active, DWORD taskId);

  private:
    TSize   Margin;       ///< Left and right, top and bottom
    bool    DragFrame;    ///< True if frame edges are used to drag, not size window
    bool    AppearActive; ///< Current fake-activation appearance flag

  DECLARE_RESPONSE_TABLE(TFloatingFrame);
  DECLARE_STREAMABLE_OWL(TFloatingFrame, 1);
};
/// @}

DECLARE_STREAMABLE_INLINES(owl::TFloatingFrame);

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Sets the margins of the floating palette window to the size specified in margin
/// and sets the height of the tiny caption bar.
//
inline void TFloatingFrame::SetMargins(const TSize& margin)
{
  Margin = margin;
}

//
/// Sets the flag for additional dragging area.
//
inline void TFloatingFrame::SetDragFrame(bool dragFrame)
{
  DragFrame = dragFrame;
}

} // OWL namespace


#endif  // OWL_FLOATFRA_H
