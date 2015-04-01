//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TSplashWindow
//----------------------------------------------------------------------------

#if !defined(OWL_SPLASHWI_H)
#define OWL_SPLASHWI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/layoutwi.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TDib;
class _OWLCLASS TPictureWindow;
class _OWLCLASS TStatic;
class _OWLCLASS TGauge;

//
/// \class TSplashWindow
// ~~~~~ ~~~~~~~~~~~~~
/// This class creates a layout window that contains a TPictureWindow and
/// optionally, a TStatic and a TGauge.
///
/// The purpose is to display a temporary window that contains a bitmap
/// and other information.
///
/// The temporary window can be closed by
///  - a mouse click        if the style has CaptureMouse turned on
///  - a passage of time    if the TimeOut data member is non-zero
///  - explicitly deleting the TSplashWindow or calling its CloseWindow()
//
class _OWLCLASS TSplashWindow : public TLayoutWindow {
  public:
    /// Styles for the splash window.
    //
    enum TStyle {
      None          = 0x0000,     ///< No styles are applied
      ShrinkToFit   = 0x0001,     ///< Resizes window to fit bitmap
      MakeGauge     = 0x0002,     ///< Display a gauge to indicate progess
      MakeStatic    = 0x0004,     ///< Display a text control
      CaptureMouse  = 0x0008,     ///< Capture mouse clicks
    };

    // Constructor/destructor
    //
    TSplashWindow(const TDib& dib,
                  int width,
                  int height,
                  int style = None,
                  uint timeOut = 0,       // in milliseconds
                  LPCTSTR title = 0,
                  TModule* module = 0);
   ~TSplashWindow();

    // Methods
    //
    void          SetText(LPCTSTR text);
    void SetText(const tstring& text) {SetText(text.c_str());}
    void          SetPercentDone(int percent);

    // Overridden TWindow virtuals
    //
    virtual bool  Create();

  protected:
    void          SetupWindow();
    void          CleanupWindow();

    // Respond to messages
    //
    void          EvLButtonDown(uint modKeys, const TPoint& point);
    void          EvTimer(uint timerId);

    // Accessors
    //
    bool          HasStyle(TStyle) const;
    uint          GetStyle() const;
    TStatic*      GetStatic();
    TGauge*       GetGauge();
    uint          GetTimeOut() const;

  private:
    uint            Style;
    TStatic*        Static;
    TGauge*         Gauge;
    TPictureWindow* PictWindow;
    uint            TimeOut;
    bool            CapturedMouse;

  DECLARE_RESPONSE_TABLE(TSplashWindow);
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Determines if the splash window has a particular style.
//
inline bool TSplashWindow::HasStyle(TStyle style) const
{
  return ToBool((GetStyle() & style) == (uint)style);
}

//
/// Returns the current style of the splash window.
//
inline uint TSplashWindow::GetStyle() const
{
  return Style;
}

//
/// Returns the static control used by the splash window. The static control is only
/// created if the style includes MakeStatic.
//
inline TStatic* TSplashWindow::GetStatic()
{
  return Static;
}

//
/// Returns the gauge used by the splash window. The gauge control is only created
/// if the style includes MakeGauge.
//
inline TGauge* TSplashWindow::GetGauge()
{
  return Gauge;
}

//
/// Returns the number of milliseconds for the splash window to automatically close.
//
inline uint TSplashWindow::GetTimeOut() const
{
  return TimeOut;
}


} // OWL namespace


#endif // OWL_SPLASHWI_H
