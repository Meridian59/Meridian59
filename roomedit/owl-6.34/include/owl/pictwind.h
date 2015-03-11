//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TPictureWindow
//----------------------------------------------------------------------------

#if !defined(OWL_PICTWIND_H)
#define OWL_PICTWIND_H

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


class _OWLCLASS TDib;

/// \addtogroup ctrl
/// @{
/// \class TPictureWindow
// ~~~~~ ~~~~~~~~~~~~~~
/// This class displays a dib in a window in different ways.
/// The dib is owned by the window and will be deleted when the window is
/// deleted.
//
class _OWLCLASS TPictureWindow : public TWindow {
  public:

    /// How to display the bitmap within the window
    //
    enum TDisplayHow {
      UpperLeft = 0,      ///< Displays the DIB in the upper left corner of the window
      Center,             ///< Always centered
      Stretch,            ///< Stretch to fit or shrink to fit
      // Scroll,             // implies Upperleft  ///TH to be implemented
    };

    // constructor and destructor
    //
    TPictureWindow(TWindow* parent, TDib* dib, TDisplayHow = UpperLeft,
                   LPCTSTR title = 0, TModule* module = 0);

    TPictureWindow(
      TWindow* parent, 
      TDib* dib, 
      TDisplayHow,
      const tstring& title, 
      TModule* module = 0);

    ~TPictureWindow();

    // Use new dib and return old dib
    //
    TDib*       SetDib(TDib* newDib);
    TDib*       GetDib() const;

    void        SetHowToDisplay(TDisplayHow how);
    TDisplayHow GetHowToDisplay() const;

    // overridden from TWindow for CS_HREDRAW and CS_VREDRAW
    //
    virtual TGetClassNameReturnType GetClassName();
    void      GetWindowClass(WNDCLASS& wndClass);

  protected:
    void Paint(TDC& dc, bool erase, TRect& rect);

  private:
    TDib* Dib;
    TDisplayHow HowToDisplay;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
// Return the dib.
//
inline TDib*
TPictureWindow::GetDib() const
{
  return Dib;
}

//
// Return how the picture is displayed.
//
inline TPictureWindow::TDisplayHow
TPictureWindow::GetHowToDisplay() const
{
  return HowToDisplay;
}

} // OWL namespace


#endif  // OWL_PICTWIND_H
