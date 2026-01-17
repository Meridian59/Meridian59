// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// colpick.h:    header file
// Version:      1.6
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// TColorPicker is based on the JPColorPicker class written by Jo Parrello
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************
#ifndef __OWLEXT_COLPICK_H
#define __OWLEXT_COLPICK_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#include <owlext/colpick.rh>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ****************** Forward declaration *************************************
class OWLEXTCLASS TColorPickerData;
class OWLEXTCLASS TColorPicker;

// ******************** TColorPickerData **************************************

class OWLEXTCLASS TColorPickerData {
  public:
    static COLORREF ColorTable8[8];
    static COLORREF ColorTable16[16];
    static COLORREF ColorTable40[40];

  public:
    owl::TColor*  Colors;
    owl::uint    NumColors;
    owl::uint    NumColumn;
    bool    UsePalette;
    bool    AutoParentNotify;
    bool    ShowDefaultField;
    owl::uint    DefaultFieldResId; // string resid for the field
                                  // +1 for statusbar message
                                  // +2 for tooltip (if not present use +0)
    COLORREF DefaultFieldColor; // need if ShowDefaultField true

    bool    ShowCustomField;
    owl::uint    CustomFieldResId;  // string resid for the field
                                  // +1 for statusbar message
                                  // +2 for tooltip (if not present use +0)
  public:
    TColorPickerData(
      COLORREF colors[],
      owl::uint     numColors,
      owl::uint     numColumn = 8,
      bool     usePalette = false,
      bool     autoParentNotify = false,
      bool     showDefaultField = true,
      owl::uint     defaultFieldResId = IDS_AUTOMATIC,
      COLORREF defaultFieldColor = 0xFF000000L,  // Color is black, first 0xFF
                                                 // identifies user defined
      bool     showCustomField = true,
      owl::uint     customFieldResId = IDS_MORECOLORS);

    TColorPickerData(const TColorPickerData& data)
      { *this = data; }

    ~TColorPickerData();

    TColorPickerData& operator =(const TColorPickerData& d);

    bool operator ==(const TColorPickerData& d) const;
    bool operator !=(const TColorPickerData& d) const;
};

// ******************** TColorPicker ******************************************

#define TColorPickerMessage _T("TCOLORPICKERMESSAGE")

const int NumCustColors = 16;
const int NumMapColors  = 43; // Num of colors where a resorce Id is mapped
const int BoxSize       = 18; // Size in pixel of a color rectangle
const int DefaultHeight = 22; // Height of the first field
const int DistToColor   = 4;  // Distance in pixel between first field and colors
const int DistToCustom  = 8;  // Distance in pixel between colors and custom field
const int CustomHeight  = 20; // Height of the custom field

typedef struct {
  COLORREF Color;
  owl::uint     ResId;
} ColorTableEntry;

class OWLEXTCLASS TColorPicker : public owl::TWindow {
  public:
    TColorPicker(owl::TWindow* parent,
                 const TColorPickerData& data,
                 owl::TColor   startColor,
                 int      refId,
                 LPCTSTR  title = 0,
                 owl::TModule* module = 0);
    virtual ~TColorPicker();

  public:
    auto GetWindowClassName() -> owl::TWindowClassName override;
    void GetWindowClass(WNDCLASS&) override;
    void Paint(owl::TDC&, bool erase, owl::TRect&) override;
    void SetupWindow() override;
    virtual void ShowPickerWindow(owl::TPoint& pt, owl::TRect& rect);
    auto PreProcessMsg(MSG&) -> bool override;
    virtual owl::TColor GetCurrentChosenColor();
    virtual void SetCurrentChosenColor(owl::TColor color);
    virtual void NotifyAtParent();

  public:
    static owl::TColor GetCustomColorsValue(int index);
    static void SetCustomColorsValue(int index, owl::TColor color);

  protected:
    void   ExitPicker();
    void   SetStatusBarMessage(int id);
    void   PaintBorder(owl::TDC& dc, int index);
    int    GetIndexFromPoint(owl::TPoint point);
    int    GetIndexByColor(owl::TColor color);
    int    GetFlatState(int index);
    void   GetCellRect(int index, owl::TRect& rect);
    void   DrawCell(owl::TDC& dc, int index);
    void   ChosenColorChanged(int index);
    void   ColorSelectionChanged(int index);
    void   EvActivateApp(bool active, DWORD hTask);
    void   EvKillFocus(THandle hWndGetFocus );
    void   EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    void   EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void   EvPaletteChanged(THandle hWndPalChg);
    bool   EvQueryNewPalette();
    void   EvKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);

  protected:
    static COLORREF        CustColors[NumCustColors];
    static ColorTableEntry MapColorTable[NumMapColors];
    static owl::uint            ColorPickerMessage;

  protected:
    TColorPickerData Data;
    owl::TTooltip*        PickerTooltip;
    int              RefId;
    bool             ColorDialogOpened;
    bool             PaletteRealized;
    owl::TColor           CurrentChosenColor;
    owl::TColor           CurrentCustomColor;
    int              CurrentSel;
    int              ChosenColorSel;
    int              NumRows;
    int              BoxMargin;
    owl::TSize            WindowSize;
    owl::tstring           DefaultText;
    owl::tstring           CustomText;
    LOGPALETTE*      MyLogPalette;

  DECLARE_RESPONSE_TABLE(TColorPicker);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>
} // OwlExt namespace

#endif
