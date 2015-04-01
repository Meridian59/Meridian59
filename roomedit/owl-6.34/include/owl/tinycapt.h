//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OWL_TINYCAPT_H)
#define OWL_TINYCAPT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>


namespace owl {

class _OWLCLASS TFont;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup mixin
/// @{
//
/// \class TTinyCaption
// ~~~~~ ~~~~~~~~~~~~
/// Derived from TWindow, TTinyCaption is a mix-in class that handles a set of
/// non-client events to produce a smaller caption bar for a window. Whenever it
/// displays the caption bar, TTinyCaption checks the window style and handles the
/// WS_SYSMENU, WS_MINIMIZEBOX, WS_MAXIMIZEBOX display attributes. Thus, you can use
/// TTinyCaption to set the attributes of the tiny caption bar before enabling the
/// caption. For example,
/// \code
/// Attr.Style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX |
/// WS_MAXIMIZEBOX;
/// \endcode
/// TTinyCaption provides functions that let you manipulate frame types, border
/// styles, and menus. You can adjust the height of the caption bar or accept the
/// default height, which is about one-half the height of a standard caption bar. If
/// you set CloseBox to true, then the window will close when you click the close
/// box instead of displaying the system menu.
/// The sample program OWLCMD.CPP on BC5.02 distribution disk displays the following
/// tiny caption bar:
/// \image html bm263.BMP
/// 
/// If you are using TTinyCaption as a mix-in class that does partial event
/// handling, call the DoXxxx function in the mix-in class (instead of the EvXxxx
/// function) to avoid duplicating default processing. The following example from
/// OWLCMD.CPP (a sample program on your distribution disk) illustrates this
/// process:
/// \code
/// void TMyFrame::EvSysCommand(uint cmdType,TPoint& p)
/// { 
/// 	if (TTinyCaption::DoSysCommand(cmdType, p) == esPartial)
/// 	FrameWindow::EvSysCommand(cmdType, p);
/// \endcode
/// The TFLoatingFrame class can be used with TTinyCaption to produce a close box.
/// See the sample programs OWLCMD.CPP and MDIFILE.CPP on BC5.0x distribution disk for
/// examples of how to use TTinyCaption.
//
class _OWLCLASS TTinyCaption : public virtual TWindow {
  protected:
    TTinyCaption();
   ~TTinyCaption();

    /// Pass closeBox=true to replace SystemMenu box with a box that will 
    /// close window when clicked
    /// Used for floating palettes, etc.
    //
    void        EnableTinyCaption(int ch=0, bool closeBox=false);

    // Controller class must handle events that call these mixin handlers
    //
    TEventStatus DoNCHitTest(const TPoint& screenPt, uint& evRes);
    TEventStatus DoNCPaint();
    TEventStatus DoNCCalcSize(bool calcValidRects, 
                              NCCALCSIZE_PARAMS & calcSize, uint& evRes);
    TEventStatus DoNCLButtonDown(uint hitTest, const TPoint& screenPt);
    TEventStatus DoMouseMove(uint hitTest, const TPoint& screenPt);
    TEventStatus DoLButtonUp(uint hitTest, const TPoint& screenPt);
    TEventStatus DoNCActivate(bool active, bool& evRes);
    TEventStatus DoCommand(uint id, HWND hWndCtl, uint notifyCode, TResult& evRes);
    TEventStatus DoSysCommand(uint cmdType, const TPoint& p);

    void        PaintButton(TDC& dc, TRect& boxRect, bool pressed);
    void        PaintCloseBox(TDC& dc, TRect& boxRect, bool pressed);
    void        PaintSysBox(TDC& dc, TRect& boxRect, bool pressed);
    void        PaintMinBox(TDC& dc, TRect& boxRect, bool pressed);
    void        PaintMaxBox(TDC& dc, TRect& boxRect, bool pressed);
    void        PaintCaption(bool active);
    void        DoSysMenu();

    TRect       GetCaptionRect();  // Get caption area for hit test or paint
    TRect       GetSysBoxRect();   // Same for close box
    TRect       GetMinBoxRect();
    TRect       GetMaxBoxRect();

    uint        EvNCHitTest(const TPoint& screenPt);
    void     EvNCPaint(HRGN);
    uint        EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize);
    void        EvNCLButtonDown(uint hitTest, const TPoint& screenPt);
    void        EvMouseMove(uint hitTest, const TPoint& screenPt);
    void        EvLButtonUp(uint hitTest, const TPoint& screenPt);
    bool        EvNCActivate(bool active);
    TResult     EvCommand(uint id, HWND hWndCtl, uint notifyCode);
    void        EvSysCommand(uint cmdType, const TPoint& p);

  protected_data:
    // Cache system metrics 
    //
    TSize    Border;        ///< Thin frame border size for dividers
    TSize    Frame;         ///< Actual Left and Right, Top and Bottom frame size

    bool     CloseBox;      ///< True for special close box
    bool     TCEnabled;     ///< Is true if the tiny caption bar is displayed. 
    int      CaptionHeight; ///< Height of the caption bar.
    TFont*   CaptionFont;   ///< Tiny font for caption bar
    uint     DownHit;       ///< Mouse down hit test result
    bool     IsPressed;     ///< Is a button currently pressed
    bool     WaitingForSysCmd; ///< Is true if TTinyCaption is ready to receive system messages.

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TTinyCaption(const TTinyCaption&);
    TTinyCaption& operator =(const TTinyCaption&);

  DECLARE_RESPONSE_TABLE(TTinyCaption);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TTinyCaption, 1);
  DECLARE_STREAMABLE_OWL(TTinyCaption, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TTinyCaption );

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_TINYCAPT_H
