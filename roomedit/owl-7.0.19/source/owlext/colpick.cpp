// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// colpick.cpp:  implementation file
// Version:      1.5
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
//
// ****************************************************************************

#include <owlext\pch.h>
#pragma hdrstop

#include <owl/tooltip.h>
#include <owl/gadgetwi.h>
#include <owl/uihelper.h>
#include <owl/chooseco.h>

#include <owlext/util.h>
#include <owlext/harborex.h>
#include <owlext/dockingex.h>
#include <owlext/colpick.h>

using namespace owl;
using namespace std;

namespace OwlExt {

// ******************** TColorPickerData **************************************

COLORREF TColorPickerData::ColorTable8[8] =
{
  RGB(0x00, 0x00, 0x00),
  RGB(0x7F, 0x7F, 0x7F),
  RGB(0x00, 0x00, 0xFF),
  RGB(0xFF, 0x00, 0x00),
  RGB(0xFF, 0xFF, 0x00),
  RGB(0x00, 0xFF, 0x00),
  RGB(0xC0, 0xC0, 0xC0),
  RGB(0xFF, 0xFF, 0xFF)
};

COLORREF TColorPickerData::ColorTable16[16] =
{
  RGB(0x00, 0x00, 0x00),
  RGB(0x7F, 0x7F, 0x7F),
  RGB(0x8B, 0x00, 0x00),
  RGB(0x8B, 0x8B, 0x00),
  RGB(0x00, 0x8B, 0x00),
  RGB(0x00, 0x8B, 0x8B),
  RGB(0x00, 0x00, 0x8B),
  RGB(0x80, 0x00, 0x80),
  RGB(0xFF, 0xFF, 0xFF),
  RGB(0xC0, 0xC0, 0xC0),
  RGB(0xFF, 0x00, 0x00),
  RGB(0xFF, 0xFF, 0x00),
  RGB(0x00, 0xFF, 0x00),
  RGB(0x00, 0xFF, 0xFF),
  RGB(0x00, 0x00, 0xFF),
  RGB(0xFF, 0x00, 0xFF)
};

COLORREF TColorPickerData::ColorTable40[40] =
{
  RGB(0x00, 0x00, 0x00),
  RGB(0xA5, 0x2A, 0x00),
  RGB(0x00, 0x40, 0x40),
  RGB(0x00, 0x55, 0x00),
  RGB(0x00, 0x00, 0x5E),
  RGB(0x00, 0x00, 0x8B),
  RGB(0x4B, 0x00, 0x82),
  RGB(0x28, 0x28, 0x28),
  RGB(0x8B, 0x00, 0x00),
  RGB(0xFF, 0x68, 0x20),
  RGB(0x8B, 0x8B, 0x00),
  RGB(0x00, 0x93, 0x00),
  RGB(0x38, 0x8E, 0x8E),
  RGB(0x00, 0x00, 0xFF),
  RGB(0x7B, 0x7B, 0xC0),
  RGB(0x66, 0x66, 0x66),
  RGB(0xFF, 0x00, 0x00),
  RGB(0xFF, 0xAD, 0x5B),
  RGB(0x32, 0xCD, 0x32),
  RGB(0x3C, 0xB3, 0x71),
  RGB(0x7F, 0xFF, 0xD4),
  RGB(0x7D, 0x9E, 0xC0),
  RGB(0x80, 0x00, 0x80),
  RGB(0x7F, 0x7F, 0x7F),
  RGB(0xFF, 0xC0, 0xCB),
  RGB(0xFF, 0xD7, 0x00),
  RGB(0xFF, 0xFF, 0x00),
  RGB(0x00, 0xFF, 0x00),
  RGB(0x40, 0xE0, 0xD0),
  RGB(0xC0, 0xFF, 0xFF),
  RGB(0x48, 0x00, 0x48),
  RGB(0xC0, 0xC0, 0xC0),
  RGB(0xFF, 0xE4, 0xE1),
  RGB(0xD2, 0xB4, 0x8C),
  RGB(0xFF, 0xFF, 0xE0),
  RGB(0x98, 0xFB, 0x98),
  RGB(0xAF, 0xEE, 0xEE),
  RGB(0x68, 0x83, 0x8B),
  RGB(0xE6, 0xE6, 0xFA),
  RGB(0xFF, 0xFF, 0xFF)
};

TColorPickerData::TColorPickerData(
                   COLORREF colors[],
                   uint     numColors,
                   uint     numColumn,
                   bool     autoParentNotify,
                   bool     usePalette,
                   bool     showDefaultField,
                   uint     defaultFieldResId,
                   COLORREF defaultFieldColor,
                   bool     showCustomField,
                   uint     customFieldResId)
{
  NumColors = numColors;
  Colors = new TColor[NumColors];
  for (uint i=0; i<NumColors; i++)
    Colors[i] = colors[i];
  NumColumn = numColumn;
  NumColumn = std::min(NumColumn, NumColors);
  AutoParentNotify = autoParentNotify;
  UsePalette = usePalette;
  ShowDefaultField = showDefaultField;
  DefaultFieldResId = defaultFieldResId;
  DefaultFieldColor = defaultFieldColor;
  ShowCustomField = showCustomField;
  CustomFieldResId = customFieldResId;
}

TColorPickerData::~TColorPickerData()
{
  delete[] Colors;
}

TColorPickerData& TColorPickerData::operator =(const TColorPickerData& d)
{
  NumColors = d.NumColors;
  Colors = new TColor[NumColors];
  for (uint i=0; i<NumColors; i++)
    Colors[i] = d.Colors[i];
  NumColumn = d.NumColumn;
  NumColumn = std::min(NumColumn, NumColors);
  AutoParentNotify = d.AutoParentNotify;
  UsePalette = d.UsePalette;
  ShowDefaultField = d.ShowDefaultField;
  DefaultFieldResId = d.DefaultFieldResId;
  DefaultFieldColor = d.DefaultFieldColor;
  ShowCustomField = d.ShowCustomField;
  CustomFieldResId = d.CustomFieldResId;
  return *this;
}

bool TColorPickerData::operator ==(const TColorPickerData& d) const
{
  if (NumColors == d.NumColors &&
    NumColumn == d.NumColumn &&
    AutoParentNotify == d.AutoParentNotify &&
    UsePalette == d.UsePalette &&
    ShowDefaultField == d.ShowDefaultField &&
    DefaultFieldResId == d.DefaultFieldResId &&
    DefaultFieldColor == d.DefaultFieldColor &&
    ShowCustomField == d.ShowCustomField &&
    CustomFieldResId == d.CustomFieldResId) {
      for (uint i=0; i<NumColors; i++) {
        if (Colors[i] != d.Colors[i])
          return false;
      }
      return true;
  }
  return false;
}

bool TColorPickerData::operator !=(const TColorPickerData& d) const
{
  return (*this == d) ? false : true;
}

// ******************** TColorPicker ******************************************

#define DEFAULT_BOX_VALUE  -3
#define CUSTOM_BOX_VALUE   -2
#define INVALID_COLOR      -1

#define FLATUPSTATE         1 // paint recessed
#define FLATNORMALSTATE     2 // paint without border
#define FLATDOWNSTATE       3 // paint pressed
#define FLATDOWNSEL         4 // paint pressed (not mask rect)

ColorTableEntry TColorPicker::MapColorTable[NumMapColors] =
{
  { RGB(0x00, 0x00, 0x00), IDS_COLORPICKER + 0 },
  { RGB(0xA5, 0x2A, 0x00), IDS_COLORPICKER + 1 },
  { RGB(0x00, 0x40, 0x40), IDS_COLORPICKER + 2 },
  { RGB(0x00, 0x55, 0x00), IDS_COLORPICKER + 3 },
  { RGB(0x00, 0x00, 0x5E), IDS_COLORPICKER + 4 },
  { RGB(0x00, 0x00, 0x8B), IDS_COLORPICKER + 5 },
  { RGB(0x4B, 0x00, 0x82), IDS_COLORPICKER + 6 },
  { RGB(0x28, 0x28, 0x28), IDS_COLORPICKER + 7 },
  { RGB(0x8B, 0x00, 0x00), IDS_COLORPICKER + 8 },
  { RGB(0xFF, 0x68, 0x20), IDS_COLORPICKER + 9 },
  { RGB(0x8B, 0x8B, 0x00), IDS_COLORPICKER + 10 },
  { RGB(0x00, 0x93, 0x00), IDS_COLORPICKER + 11 },
  { RGB(0x38, 0x8E, 0x8E), IDS_COLORPICKER + 12 },
  { RGB(0x00, 0x00, 0xFF), IDS_COLORPICKER + 13 },
  { RGB(0x7B, 0x7B, 0xC0), IDS_COLORPICKER + 14 },
  { RGB(0x66, 0x66, 0x66), IDS_COLORPICKER + 15 },
  { RGB(0xFF, 0x00, 0x00), IDS_COLORPICKER + 16 },
  { RGB(0xFF, 0xAD, 0x5B), IDS_COLORPICKER + 17 },
  { RGB(0x32, 0xCD, 0x32), IDS_COLORPICKER + 18 },
  { RGB(0x3C, 0xB3, 0x71), IDS_COLORPICKER + 19 },
  { RGB(0x7F, 0xFF, 0xD4), IDS_COLORPICKER + 20 },
  { RGB(0x7D, 0x9E, 0xC0), IDS_COLORPICKER + 21 },
  { RGB(0x80, 0x00, 0x80), IDS_COLORPICKER + 22 },
  { RGB(0x7F, 0x7F, 0x7F), IDS_COLORPICKER + 23 },
  { RGB(0xFF, 0xC0, 0xCB), IDS_COLORPICKER + 24 },
  { RGB(0xFF, 0xD7, 0x00), IDS_COLORPICKER + 25 },
  { RGB(0xFF, 0xFF, 0x00), IDS_COLORPICKER + 26 },
  { RGB(0x00, 0xFF, 0x00), IDS_COLORPICKER + 27 },
  { RGB(0x40, 0xE0, 0xD0), IDS_COLORPICKER + 28 },
  { RGB(0xC0, 0xFF, 0xFF), IDS_COLORPICKER + 29 },
  { RGB(0x48, 0x00, 0x48), IDS_COLORPICKER + 30 },
  { RGB(0xC0, 0xC0, 0xC0), IDS_COLORPICKER + 31 },
  { RGB(0xFF, 0xE4, 0xE1), IDS_COLORPICKER + 32 },
  { RGB(0xD2, 0xB4, 0x8C), IDS_COLORPICKER + 33 },
  { RGB(0xFF, 0xFF, 0xE0), IDS_COLORPICKER + 34 },
  { RGB(0x98, 0xFB, 0x98), IDS_COLORPICKER + 35 },
  { RGB(0xAF, 0xEE, 0xEE), IDS_COLORPICKER + 36 },
  { RGB(0x68, 0x83, 0x8B), IDS_COLORPICKER + 37 },
  { RGB(0xE6, 0xE6, 0xFA), IDS_COLORPICKER + 38 },
  { RGB(0xFF, 0xFF, 0xFF), IDS_COLORPICKER + 39 },
  { RGB(0x00, 0x8B, 0x00), IDS_COLORPICKER + 40 },
  { RGB(0x00, 0x8B, 0x8B), IDS_COLORPICKER + 41 },
  { RGB(0xFF, 0x00, 0xFF), IDS_COLORPICKER + 42 }
};

COLORREF TColorPicker::CustColors[NumCustColors] =
{
  0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
  0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
  0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
  0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000
};

uint TColorPicker::ColorPickerMessage = 0;

//
// Build a response table for all messages/commands handled by the application.
//
DEFINE_RESPONSE_TABLE1(TColorPicker, TWindow)
EV_WM_ACTIVATEAPP,
EV_WM_KILLFOCUS,
EV_WM_MOUSEMOVE,
EV_WM_LBUTTONDOWN,
EV_WM_PALETTECHANGED,
EV_WM_QUERYNEWPALETTE,
EV_WM_KEYDOWN,
END_RESPONSE_TABLE;

TColorPicker::TColorPicker(TWindow* parent,
               const TColorPickerData& data,
               TColor   startColor,
               int      refId,
               LPCTSTR  title,
               TModule* module):
TWindow(parent, title, module),
Data(data)
{
  RefId = refId;
  CurrentChosenColor = startColor;
  CurrentCustomColor = Data.DefaultFieldColor;
  ColorPickerMessage = ::RegisterWindowMessage(TColorPickerMessage);
  PickerTooltip = 0;
  ColorDialogOpened = false;
  PaletteRealized = false;
  BoxMargin = ::GetSystemMetrics(SM_CXBORDER) + 5;
  Attr.Style = WS_POPUP;
  Attr.ExStyle |= WS_EX_WINDOWEDGE;
  Attr.ExStyle |= WS_EX_TOPMOST;
  CurrentSel = INVALID_COLOR;
  ChosenColorSel = GetIndexByColor(CurrentChosenColor);
  NumRows = Data.NumColors / Data.NumColumn;
  if (Data.NumColors % Data.NumColumn)
    NumRows++;

  if (Data.UsePalette == false)
    MyLogPalette = 0;
  else {
    MyLogPalette = (LOGPALETTE*)new char[sizeof(LOGPALETTE) +
      sizeof(PALETTEENTRY) * Data.NumColors];
    MyLogPalette->palVersion = 0x300;
    MyLogPalette->palNumEntries = (WORD) Data.NumColors;
    for (uint i=0; i<Data.NumColors; ++i) {
      TColor color (Data.Colors[i]);
      MyLogPalette->palPalEntry[i].peRed   = color.Red();
      MyLogPalette->palPalEntry[i].peGreen = color.Green();
      MyLogPalette->palPalEntry[i].peBlue  = color.Blue();
      MyLogPalette->palPalEntry[i].peFlags = PC_RESERVED;
    }
  }

  WindowSize = TSize(Data.NumColumn*BoxSize + 2*BoxMargin,
    NumRows*BoxSize + 2*BoxMargin);
  if (Data.ShowCustomField)
    WindowSize.cy += CustomHeight+DistToCustom;;
  if (Data.ShowDefaultField)
    WindowSize.cy += (DefaultHeight+DistToColor);
  SetCurrentChosenColor(startColor);
}

TColorPicker::~TColorPicker()
{
  Destroy(IDCANCEL);
  delete[] MyLogPalette;
  if (PickerTooltip)
    delete PickerTooltip;
}

// Another application has the focus, so we hide the picker window.
void TColorPicker::EvActivateApp(bool active, DWORD hTask)
{
  TWindow::EvActivateApp(active, hTask);
  if (!active && !ColorDialogOpened)
    ExitPicker();
}

// The picker window has lost the focus, so we hide it.
//
void TColorPicker::EvKillFocus(THandle hWndGetFocus )
{
  TWindow::EvKillFocus(hWndGetFocus);
  ReleaseCapture();
}

// Hide the picker window and release the mouse capture.
//
void TColorPicker::ExitPicker()
{
  ReleaseCapture();
  ShowWindow(SW_HIDE);
}

// Retrieve the name of this window's class.
//
auto TColorPicker::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{_T("TColorPickerWindow")};
}

// Retrieve the window class informations.
//
void TColorPicker::GetWindowClass(WNDCLASS& wndClass)
{
  TWindow::GetWindowClass(wndClass);
  wndClass.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
}

// We paint all the cells.
//
void TColorPicker::Paint(TDC& dc, bool erase, TRect& rect)
{
  TWindow::Paint(dc, erase, rect);

  TUIBorder border(GetClientRect(), TUIBorder::WndRaised);
  border.Paint(dc);

  DrawCell(dc, DEFAULT_BOX_VALUE);
  DrawCell(dc, CUSTOM_BOX_VALUE);
  if (MyLogPalette && dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) {
    TPalette palette(*MyLogPalette);
    dc.SelectObject(palette);
    PaletteRealized = (dc.RealizePalette() > 0);
  }
  for (uint i=0; i<Data.NumColors; i++)
    DrawCell(dc, i);
  dc.RestoreObjects();
}

// We create the tooltip control and we send all data to it.
//
void TColorPicker::SetupWindow()
{
  TRect rect;
  owl::tstring toolstring;

  TWindow::SetupWindow();
  SetBkgndColor(TColor::Sys3dFace);
  SetWindowFont((HFONT)GetStockObject(ANSI_VAR_FONT), true);
  PickerTooltip = new TTooltip(this);
  PickerTooltip->Create();

  if (Data.ShowDefaultField) {
    DefaultText = GetModule()->LoadString(Data.DefaultFieldResId);
    toolstring =  GetModule()->LoadString(Data.DefaultFieldResId+2);
    if (toolstring.empty())
      toolstring = DefaultText;
    GetCellRect(DEFAULT_BOX_VALUE, rect);
    TToolInfo ti (*this, rect, 1, toolstring.c_str());
    PickerTooltip->AddTool(ti);
  }
  else
    DefaultText = _T("");

  if (Data.ShowCustomField) {
    CustomText = GetModule()->LoadString(Data.CustomFieldResId);
    toolstring =  GetModule()->LoadString(Data.CustomFieldResId+2);
    if (toolstring.empty())
      toolstring = CustomText;
    GetCellRect(CUSTOM_BOX_VALUE, rect);
    TToolInfo ti (*this, rect, 1, toolstring.c_str());
    PickerTooltip->AddTool(ti);
  }
  else
    CustomText = _T("");

  for (uint i=0; i<Data.NumColors; i++) {
    for (uint j=0; j<NumMapColors; j++) {
      if (Data.Colors[i] == MapColorTable[j].Color) {
        toolstring = GetModule()->LoadString(MapColorTable[j].ResId);
        break;
      }
    }

    if (!toolstring.empty()){
      GetCellRect(i, rect);
      TToolInfo ti (*this, rect, 1, toolstring.c_str());
      PickerTooltip->AddTool(ti);
    }
  }
}

// We must show the picker window at the given coordinates.
//
void TColorPicker::ShowPickerWindow(TPoint& pt, TRect& rect)
{
  CurrentSel = INVALID_COLOR;
  ColorDialogOpened = false;
  TSize size = WindowSize;
  TSize checksize (size.cx + 1, size.cy + 1);
  TPoint point (pt);
  TSize ScreenSize(::GetSystemMetrics(SM_CXSCREEN),
    ::GetSystemMetrics(SM_CYSCREEN));

  // Check the vertical position.
  //
  if (point.y + size.cy > ScreenSize.cy) {
    if ((rect.top >= 0) && (rect.bottom >= 0) && (rect.top - checksize.cy >= 0))
      point.y = rect.top - checksize.cy;
    else point.y = ScreenSize.cy - checksize.cy;
  }

  // Check the horizontal position.
  //
  if (point.x + checksize.cx > ScreenSize.cx)
    point.x = ScreenSize.cx - checksize.cx;
  SetWindowPos(HWND_TOPMOST, point.x, point.y, size.cx, size.cy, SWP_SHOWWINDOW);
  SetCapture();
}

// We retrieve the rectangle for a single cell.
//
void TColorPicker::GetCellRect(int index, TRect& rect)
{
  TRect clientrect = GetClientRect();
  if (index == DEFAULT_BOX_VALUE) {
    if (Data.ShowDefaultField) {
      rect = TRect(BoxMargin, BoxMargin,
        WindowSize.cx-BoxMargin, BoxMargin+DefaultHeight);
    }
    else {
      rect = TRect(0,0,0,0);
    }
  }
  else if (index == CUSTOM_BOX_VALUE) {
    if (Data.ShowCustomField) {
      rect = TRect(BoxMargin, WindowSize.cy-BoxMargin-CustomHeight,
        WindowSize.cx - BoxMargin, WindowSize.cy - BoxMargin);
    }
    else {
      rect = TRect(0,0,0,0);
    }
  }
  else if ((index >= 0) && (index < (int)Data.NumColors)) {
    TPoint pt;
    pt.x = (index - ((index/Data.NumColumn)*Data.NumColumn))*BoxSize+BoxMargin;
    pt.y = (index/Data.NumColumn) * BoxSize + BoxMargin;
    if (Data.ShowDefaultField)
      pt.y += (DefaultHeight+DistToColor);
    rect = TRect(pt, TSize(BoxSize, BoxSize));
  }
}

// We draw a single cell.
//
void TColorPicker::DrawCell(TDC& dc, int index)
{
  TRect rect;
  GetCellRect(index, rect);
  PaintBorder(dc, index);
  rect.Inflate(-2, -2);
  if (index != INVALID_COLOR) {
    if (GetFlatState(index) == FLATDOWNSTATE) {
      FillMaskRect(dc, rect);
    }
    else {
      dc.FillRect(rect, TBrush(TColor::Sys3dFace));
    }
    rect.Inflate(-1, -1);
    TBrush framebrush(TColor::Sys3dShadow);
    if (index != CUSTOM_BOX_VALUE)
      dc.FrameRect(rect, framebrush);
    dc.RestoreBrush();
    rect.Inflate(-1, -1);
  }
  if (index >= 0 && index < (int)Data.NumColors) {
    dc.SelectStockObject(NULL_PEN);
    if (PaletteRealized) {
      TColor palColor(PALETTEINDEX(index));
      dc.FillRect(rect, TBrush(palColor));
    }
    else {
      TColor palColor(Data.Colors[index]);
      dc.FillRect(rect, TBrush(palColor));
    }
    dc.RestorePen();
  }
  else {
    if (index != INVALID_COLOR) {
      dc.SelectStockObject(ANSI_VAR_FONT);
      TColor oldtextcol = dc.SetTextColor(TColor::SysBtnText);
      int oldbkmode = dc.SetBkMode(TRANSPARENT);
      if (index == DEFAULT_BOX_VALUE && !DefaultText.empty()) {
        dc.DrawText(DefaultText.c_str(), -1, rect,
          DT_CENTER | DT_SINGLELINE | DT_VCENTER);
      }

      if (index == CUSTOM_BOX_VALUE && !CustomText.empty()) {
        dc.DrawText(CustomText.c_str(), -1, rect,
          DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        // paint separator line
        //
        TPen pen(TColor::Sys3dShadow);
        dc.SelectObject(pen);
        dc.MoveTo(BoxMargin, rect.top-4-DistToCustom/2);
        dc.LineTo(WindowSize.cy-2*BoxMargin+2, rect.top-4-DistToCustom/2);
        dc.RestorePen();
        TPen pen2(TColor::Sys3dHilight);
        dc.SelectObject(pen2);
        dc.MoveTo(BoxMargin, rect.top-3-DistToCustom/2);
        dc.LineTo(WindowSize.cy-2*BoxMargin+2, rect.top-3-DistToCustom/2);
        dc.RestorePen();
      }
      dc.SetTextColor(oldtextcol);
      dc.SetBkMode(oldbkmode);
      dc.RestoreObjects();
    }
  }
}

// Retrieve the current flat state for a cell.
//
int TColorPicker::GetFlatState(int index)
{
  if (index == ChosenColorSel) {
    if (index == CurrentSel)
      return FLATDOWNSEL;
    else
      return FLATDOWNSTATE;
  }
  if (index == CurrentSel)
    return FLATUPSTATE;
  return FLATNORMALSTATE;
}

// We retrieve the index when we know the color.
//
int TColorPicker::GetIndexByColor(TColor color)
{
  if (color == Data.DefaultFieldColor) {
    if (Data.ShowDefaultField)
      return DEFAULT_BOX_VALUE;
  }
  for (uint i=0; i<Data.NumColors; i++) {
    if (Data.Colors[i] == color)
      return i;
  }
  return INVALID_COLOR;
}

// We changed the current selection cell.
//
void TColorPicker::ColorSelectionChanged(int index)
{
  if (index != CurrentSel) {
    TClientDC dc (*this);

    if (MyLogPalette && dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) {
      TPalette palette(*MyLogPalette);
      dc.SelectObject(palette);
      PaletteRealized = (dc.RealizePalette() > 0);
    }

    int oldsel = CurrentSel;
    CurrentSel = index;
    DrawCell(dc, oldsel);
    if (CurrentSel != INVALID_COLOR) {
      DrawCell(dc, CurrentSel);
      if (CurrentSel == DEFAULT_BOX_VALUE && Data.ShowDefaultField)
        SetStatusBarMessage(Data.DefaultFieldResId+1);
      else if (CurrentSel == CUSTOM_BOX_VALUE && Data.ShowCustomField)
        SetStatusBarMessage(Data.CustomFieldResId+1);
      else
        SetStatusBarMessage(-1);
    }
    else
      SetStatusBarMessage(-1);

    dc.RestoreObjects();
  }
}

// We move the cursor to another cell.
//
void TColorPicker::EvMouseMove(uint modKeys, const TPoint& point)
{
  TWindow::EvMouseMove(modKeys, point);
  ColorSelectionChanged(GetIndexFromPoint(point));
}

// We changed the current chosen color.
//
void TColorPicker::ChosenColorChanged(int index)
{
  bool canExit = false;
  int oldchosen = ChosenColorSel;
  TColor chosencolor;
  if (index != INVALID_COLOR && index == ChosenColorSel) {
    chosencolor = CurrentChosenColor;
    canExit = true;
  }
  else if (index != ChosenColorSel && index != INVALID_COLOR) {
    ChosenColorSel = index;
    Invalidate(false);
    UpdateWindow();
    if (ChosenColorSel >= 0) {
      chosencolor = Data.Colors[ChosenColorSel];
      canExit = true;
    }
    else {
      if (ChosenColorSel == DEFAULT_BOX_VALUE) {
        chosencolor = Data.DefaultFieldColor;
        canExit = true;
      }
    }
  }
  if (index == CUSTOM_BOX_VALUE) {
    ColorDialogOpened = true;
    TChooseColorDialog::TData choose;
    choose.Flags = CC_RGBINIT | CC_FULLOPEN;
    if (CurrentCustomColor == Data.DefaultFieldColor)
      choose.Color = CurrentChosenColor;
    else
      choose.Color = CurrentCustomColor;
    choose.CustColors = (TColor*)CustColors;
    if (TChooseColorDialog(this, choose).Execute() == IDOK) {
      ChosenColorSel = GetIndexByColor(choose.Color);
      chosencolor = choose.Color;
      CurrentCustomColor = choose.Color;
      canExit = true;
    }
    else {
      ChosenColorSel = oldchosen;
      Invalidate(false);
      UpdateWindow();
      SetCapture();
    }
    ColorDialogOpened = false;
    Invalidate(true);
  }
  if (canExit) {
    CurrentChosenColor = chosencolor;
    if (Data.AutoParentNotify)
      NotifyAtParent();
    ExitPicker();
  }
}

// We pressed the left mouse button.
//
void TColorPicker::EvLButtonDown(uint modKeys, const TPoint& point)
{
  TWindow::EvLButtonDown(modKeys, point);

  // If we are in a cell, change the current chosen color and exit.
  // If we aren't in the client rectangle, we exit.
  if (GetClientRect().Contains(point))
    ChosenColorChanged(GetIndexFromPoint(point));
  else
    ExitPicker();
}

// Retrieve the cell's index when we know the current position of the cursor.
//
int TColorPicker::GetIndexFromPoint(TPoint point)
{
  TRect rect;
  GetCellRect(DEFAULT_BOX_VALUE, rect);
  if (rect.Contains(point))
    return DEFAULT_BOX_VALUE;
  GetCellRect(CUSTOM_BOX_VALUE, rect);
  if (rect.Contains(point))
    return CUSTOM_BOX_VALUE;
  for (uint i=0; i<Data.NumColors; i++) {
    GetCellRect(i, rect);
    if (rect.Contains(point))
      return i;
  }
  return (int) INVALID_COLOR;
}

// We paint the border of a cell.
//
void TColorPicker::PaintBorder(TDC& dc, int index)
{
  TRect rect;
  GetCellRect(index, rect);
  if (GetFlatState(index) == FLATUPSTATE) {
    TUIBorder border(rect, TUIBorder::TEdge(TUIBorder::RaisedInner),
      TUIBorder::Rect | TUIBorder::Adjust);
    border.Paint(dc);
  }
  else if (GetFlatState(index) == FLATDOWNSTATE ||
    GetFlatState(index) == FLATDOWNSEL) {
      TUIBorder(rect, TUIBorder::Recessed).Paint(dc);
  }
  else {
    TBrush brush (TColor::Sys3dFace);
    dc.FrameRect(rect, brush);
    dc.RestoreBrush();
  }
}

// We dispatch all mouse messages to the tooltip control.
//
bool TColorPicker::PreProcessMsg(MSG& msg)
{
  if (PickerTooltip && PickerTooltip->IsWindow()) {
    if (msg.hwnd == *this || IsChild(msg.hwnd)) {
      if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
        PickerTooltip->RelayEvent(msg);
      }
    }
  }
  return (TWindow::PreProcessMsg(msg));
}

// We display a message on the status bar.
//
void TColorPicker::SetStatusBarMessage(int id)
{
  TWindow* parent= GetParentO();
  TDecoratedFrame* frame = 0;
  if (parent)
    frame = TYPESAFE_DOWNCAST(parent, TDecoratedFrame);
  while (parent && !frame) {
    parent = parent->GetParentO();
    if (parent)
      frame = TYPESAFE_DOWNCAST(parent, TDecoratedFrame);
  }
  if (frame) {
    if (id > 0) {
      frame->HandleMessage(WM_MENUSELECT, id, 0);
    }
    else {
      frame->HandleMessage(WM_MENUSELECT, 0xFFFF0000, 0);
    }
    frame->HandleMessage(WM_ENTERIDLE, MSGF_MENU);
  }
}

// Retrieve the current chosen color.
//
TColor TColorPicker::GetCurrentChosenColor()
{
  return CurrentChosenColor;
}

// Set the current chosen color.
//
void TColorPicker::SetCurrentChosenColor(TColor color)
{
  CurrentChosenColor = color;
  ChosenColorSel = GetIndexByColor(CurrentChosenColor);
  if (GetHandle() && IsWindowVisible())
    Invalidate();
}

// We notify the parent window that the user has selected a color on the picker.
//
void TColorPicker::NotifyAtParent()
{
  ::PostMessage(GetParentH(), ColorPickerMessage, (WPARAM)RefId,
    (LPARAM)CurrentChosenColor.GetValue());
}

// Someone has changed the palette. We must realize our palette again.
//
void TColorPicker::EvPaletteChanged(THandle hWndPalChg)
{
  TWindow::EvPaletteChanged(hWndPalChg);
  if (hWndPalChg != GetHandle())
    Invalidate();
}

// Give us the chance to realize our palette.
//
bool TColorPicker::EvQueryNewPalette()
{
  bool result;
  Invalidate();
  result = TWindow::EvQueryNewPalette() | PaletteRealized;
  return result;
}

// We manage the keys to move the current selection box.
//
void TColorPicker::EvKeyDown(uint key, uint repeatCount, uint flags)
{
  int newselection;
  TWindow::EvKeyDown(key, repeatCount, flags);

  switch (key) {
  case VK_ESCAPE:
    ExitPicker();
    break;

  case VK_RETURN:
  case VK_SPACE:
    ChosenColorChanged(CurrentSel);
    break;

  case VK_LEFT:
    if ((CurrentSel == INVALID_COLOR) || (CurrentSel == DEFAULT_BOX_VALUE)) {
      if (Data.ShowCustomField)
        newselection = CUSTOM_BOX_VALUE;
      else
        newselection = Data.NumColors - 1;
    }
    else if (CurrentSel == CUSTOM_BOX_VALUE) {
      newselection = Data.NumColors - 1;
    }
    else if (CurrentSel == 0) {
      if (Data.ShowDefaultField)
        newselection = DEFAULT_BOX_VALUE;
      else if (Data.ShowCustomField)
        newselection = CUSTOM_BOX_VALUE;
      else
        newselection = Data.NumColors - 1;
    }
    else
      newselection = CurrentSel - 1;
    ColorSelectionChanged(newselection);
    break;

  case VK_RIGHT:
    if ((CurrentSel == INVALID_COLOR) || (CurrentSel == CUSTOM_BOX_VALUE)) {
      if (Data.ShowDefaultField)
        newselection = DEFAULT_BOX_VALUE;
      else
        newselection = 0;
    }
    else if (CurrentSel == DEFAULT_BOX_VALUE) {
      newselection = 0;
    }
    else if (CurrentSel == (int)Data.NumColors - 1) {
      if (Data.ShowCustomField)
        newselection = CUSTOM_BOX_VALUE;
      else if (Data.ShowDefaultField)
        newselection = DEFAULT_BOX_VALUE;
      else
        newselection = 0;
    }
    else
      newselection = CurrentSel + 1;
    ColorSelectionChanged(newselection);
    break;

  case VK_DOWN:
    if ((CurrentSel == INVALID_COLOR) || (CurrentSel == CUSTOM_BOX_VALUE)) {
      if (Data.ShowDefaultField)
        newselection = DEFAULT_BOX_VALUE;
      else
        newselection = 0;
    }
    else if (CurrentSel == DEFAULT_BOX_VALUE) {
      newselection = 0;
    }
    else if (CurrentSel + Data.NumColumn > Data.NumColors - 1) {
      if (Data.ShowCustomField)
        newselection = CUSTOM_BOX_VALUE;
      else if (Data.ShowDefaultField)
        newselection = DEFAULT_BOX_VALUE;
      else
        newselection = 0;
    }
    else
      newselection = CurrentSel + Data.NumColumn;
    ColorSelectionChanged(newselection);
    break;

  case VK_UP:
    if ((CurrentSel == INVALID_COLOR) || (CurrentSel == DEFAULT_BOX_VALUE)) {
      if (Data.ShowCustomField)
        newselection = CUSTOM_BOX_VALUE;
      else
        newselection = Data.NumColors - 1;
    }
    else if (CurrentSel == CUSTOM_BOX_VALUE) {
      newselection = Data.NumColors - 1;
    }
    else if ((CurrentSel - (int)Data.NumColumn) < 0) {
      if (Data.ShowDefaultField)
        newselection = DEFAULT_BOX_VALUE;
      else if (Data.ShowCustomField)
        newselection = CUSTOM_BOX_VALUE;
      else
        newselection = Data.NumColors - 1;
    }
    else
      newselection = CurrentSel - Data.NumColumn;
    ColorSelectionChanged(newselection);
    break;

  case VK_HOME:
    if (Data.ShowDefaultField)
      newselection = DEFAULT_BOX_VALUE;
    else
      newselection = 0;
    ColorSelectionChanged(newselection);
    break;

  case VK_END:
    if (Data.ShowCustomField)
      newselection = CUSTOM_BOX_VALUE;
    else
      newselection = Data.NumColors - 1;
    ColorSelectionChanged(newselection);
    break;
  }
}

// We retrieve a value from the custom colors array.
//
TColor TColorPicker::GetCustomColorsValue(int index)
{
  if (index < 0 && index >= NumCustColors)
    index = 0;
  return TColor(CustColors[index]);
}

// We set a value on the custom colors array.
//
void TColorPicker::SetCustomColorsValue(int index, TColor color)
{
  if (index >= 0 && index < NumCustColors)
    CustColors[index] = color;
}

} // OwlExt namespace
//======================================================================================
