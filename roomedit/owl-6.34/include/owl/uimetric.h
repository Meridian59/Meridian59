//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TUIMetric, a UI metrics provider class
//----------------------------------------------------------------------------

#if !defined(OWL_UIMETRIC_H)
#define OWL_UIMETRIC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/wsysinc.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TUIMetric
// ~~~~~ ~~~~~~~~~~
/// TUIMetric encapsulates the GetSystemMetric() API. Typical using:
/// \code
/// Margins.Left = Margins.Right = TUIMetric::CxFixedFrame;
/// Margins.Top = Margins.Bottom = TUIMetric::CyFixedFrame;
/// \endcode
class _OWLCLASS TUIMetric {
  public:
    static const TUIMetric CxScreen;
    static const TUIMetric CyScreen;
    static const TUIMetric CxVScroll;
    static const TUIMetric CyHScroll;
    static const TUIMetric CyCaption;
    static const TUIMetric CxBorder;
    static const TUIMetric CyBorder;
    static const TUIMetric CxFixedFrame;
    static const TUIMetric CyFixedFrame;
    static const TUIMetric CyVThumb;
    static const TUIMetric CxHThumb;
    static const TUIMetric CxIcon;
    static const TUIMetric CyIcon;
    static const TUIMetric CxCursor;
    static const TUIMetric CyCursor;
    static const TUIMetric CyMenu;
    static const TUIMetric CxFullScreen;
    static const TUIMetric CyFullScreen;
    static const TUIMetric CyKanjiWindow;
    static const TUIMetric MousePresent;
    static const TUIMetric CyVScroll;
    static const TUIMetric CxHScroll;
    static const TUIMetric Debug;
    static const TUIMetric SwapButton;
    static const TUIMetric Reserved1;
    static const TUIMetric Reserved2;
    static const TUIMetric Reserved3;
    static const TUIMetric Reserved4;
    static const TUIMetric CxMin;
    static const TUIMetric CyMin;
    static const TUIMetric CxSize;
    static const TUIMetric CySize;
    static const TUIMetric CxSizeFrame;
    static const TUIMetric CySizeFrame;
    static const TUIMetric CxMinTrack;
    static const TUIMetric CyMinTrack;
    static const TUIMetric CxDoubleClk;
    static const TUIMetric CyDoubleClk;
    static const TUIMetric CxIconSpacing;
    static const TUIMetric CyIconSpacing;
    static const TUIMetric MenuDropAlignment;
    static const TUIMetric PenWindows;
    static const TUIMetric DbcsEnabled;
    static const TUIMetric CMouseButtons;
    static const TUIMetric Secure;
    static const TUIMetric CxEdge;
    static const TUIMetric CyEdge;
    static const TUIMetric CxMinSpacing;
    static const TUIMetric CyMinSpacing;
    static const TUIMetric CxSmIcon;
    static const TUIMetric CySmIcon;
    static const TUIMetric CySmCaption;
    static const TUIMetric CxSmSize;
    static const TUIMetric CySmSize;
    static const TUIMetric CxMenuSize;
    static const TUIMetric CyMenuSize;
    static const TUIMetric Arrange;
    static const TUIMetric CxMinimized;
    static const TUIMetric CyMinimized;
    static const TUIMetric CxMaxTrack;
    static const TUIMetric CyMaxTrack;
    static const TUIMetric CxMaximized;
    static const TUIMetric CyMaximized;
    static const TUIMetric ShowSounds;
    static const TUIMetric KeyboardPref;
    static const TUIMetric HighContrast;
    static const TUIMetric ScreenReader;
    static const TUIMetric CleanBoot;
    static const TUIMetric CxDrag;
    static const TUIMetric CyDrag;
    static const TUIMetric Network;
    static const TUIMetric CxMenuCheck;
    static const TUIMetric CyMenuCheck;
    static const TUIMetric SlowMachine;
    static const TUIMetric MideastEnabled;

    static const TUIMetric CxPixelsPerInch;
    static const TUIMetric CyPixelsPerInch;

    static const TUIMetric MouseWheelPresent; // only NT or WinVer > 5.0
    //#if(WINVER >= 0x0500)
    static const TUIMetric XVirtualScreen;
    static const TUIMetric YVirtualScreen;
    static const TUIMetric CxVirtualScreen;
    static const TUIMetric CyVirtualScreen;
    static const TUIMetric CMonitors;
    static const TUIMetric SameDisplayFormat;
    //#endif /* WINVER >= 0x0500 */
    //for NT >= 4.0 or Win 98  == 83 else 76
    static const TUIMetric CMetrics;


    TUIMetric(int i);

    operator int() const;

    int Get() const;
    static int Get(int i);

  private:
    int I;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
inline TUIMetric::TUIMetric(int i)
:
  I(i)
{
}

//
inline TUIMetric::operator int() const {
  return Get();
}

//
inline int TUIMetric::Get() const {
  return Get(I);
}

//
inline int TUIMetric::Get(int i) {
  return ::GetSystemMetrics(i);
}

} // OWL namespace


#endif  // OWL_UIMETRIC_H
