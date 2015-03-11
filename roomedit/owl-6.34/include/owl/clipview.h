//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Mixin class TClipboardViewer that allows a TWindow derived class to hook in
/// on the clipboard notification chain.
//----------------------------------------------------------------------------

#if !defined(OWL_CLIPVIEW_H)
#define OWL_CLIPVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>

namespace owl {


// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup mixin
/// @{
/// \class TClipboardViewer
// ~~~~~ ~~~~~~~~~~~~~~~~
/// Mix-in class that registers as a clipboard viewer when the user interface
/// element is created and removes itself from the clipboard-viewer chain when
/// it is destroyed
//
class _OWLCLASS TClipboardViewer : virtual public TWindow {
  protected:
    TClipboardViewer();
    TClipboardViewer(HWND hWnd, TModule* module = 0);

    TEventStatus DoChangeCBChain(HWND hWndRemoved, HWND hWndNext);
    TEventStatus DoDestroy();
    TEventStatus DoDrawClipboard();  ///< pass to next window in clipboard-viewer chain

    // Override method defined by TWindow
    //
    void   SetupWindow();

    // Message response functions
    //
    void   EvChangeCBChain(HWND hWndRemoved, HWND hWndNext);
    void   EvDestroy();
    void   EvDrawClipboard();  ///< pass to next window in clipboard-viewer chain

    HWND  GetNextWindow() const;
    void  SetNextWindow(HWND hwndnext);

  protected_data:
    THandle  HWndNext;  ///< Next window in clipboard-viewer chain /!CQ 'Next'?

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TClipboardViewer(const TClipboardViewer&);
    TClipboardViewer& operator =(const TClipboardViewer&);

  DECLARE_RESPONSE_TABLE(TClipboardViewer);
  DECLARE_CASTABLE;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//
// Inline implementations
//

//
/// Return the next window in the viewer chain.
//
inline HWND  TClipboardViewer::GetNextWindow() const {
  return HWndNext;
}

//
/// Set the next window in the viewer chain.
//
inline void  TClipboardViewer::SetNextWindow(HWND hwndnext) {
  HWndNext = hwndnext;
}


} // OWL namespace


#endif  // OWL_CLIPVIEW_H
