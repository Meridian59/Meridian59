//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// OWLEXT's utility functions and macros
//
// Original code by Don Griffin; used with permission.
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_UTIL_H)
#define __OWLEXT_UTIL_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

namespace OwlExt {


// These are in OwlExt.cpp:
//
void    ActivateWindow (owl::TWindow* tWindow);
void    CenterScreenRectOnRect (owl::TRect* rc, owl::TRect* rcOver);
void    CenterWindowOver (HWND, HWND);
void    FastFillRect (HDC dc, int x, int y, int cx, int cy, owl::TColor& color);
void    FastFrameRect3D (HDC hdc, const owl::TRect& r, owl::TColor& clrLT, owl::TColor& clrRB);
LPTSTR  FormatTime (LPTSTR szTime, owl::uint bufSize, bool showSeconds = true);
void    GetTheTime (owl::TSystemTime& sysTime);
owl::uint16  GetTrueOsVer ();
void    UpdateCursor ();

// MFC style function, that draw the frame of a 3d rect with given colors
void Draw3dRect(owl::TDC& dc, const owl::TRect& rect,
  owl::TColor clrTopLeft, owl::TColor clrBottomRight);
void Draw3dRect(owl::TDC& dc, int x, int y, int cx, int cy,
  owl::TColor clrTopLeft, owl::TColor clrBottomRight);

// MFC style function, that draws a filled rect. Is this faster???
void FillSolidRect(owl::TDC& dc, int x, int y, int cx, int cy, owl::TColor clr);
void FillMaskRect(owl::TDC& dc, owl::TRect rect);
void DrawDisabledButton(owl::TDC& dc, const owl::TRect& rc);

owl::tstring GetHintText(owl::TWindow* win, owl::uint id, owl::THintText hintType = owl::htTooltip);

//  Inliners:
//
inline void   CenterWindow (HWND hWnd)
                { CenterWindowOver (hWnd, GetParent(hWnd)); }
inline void   CenterWindow (owl::TWindow *pWnd)
                { CenterWindow (pWnd->GetHandle()); }
inline void   CenterWindowOver (owl::TWindow *pWndOver, owl::TWindow *pWndUnder)
                { CenterWindowOver (pWndOver->GetHandle(), pWndUnder->GetHandle()); }

inline void   FastFillRect (HDC dc, const owl::TPoint &topLeft, const owl::TSize &s,
                            owl::TColor c)
                { FastFillRect (dc, topLeft.x, topLeft.y, s.cx, s.cy, c); }
inline void   FastFillRect (HDC dc, const owl::TRect &r, owl::TColor c)
                { FastFillRect (dc, r.left, r.top, r.Width(), r.Height(), c); }

inline owl::uint16 GetOsVer () { return GetTrueOsVer(); }
inline owl::uint   GetOsVerMajor() { return HIBYTE(GetOsVer()); }
inline owl::uint   GetOsVerMinor() { return LOBYTE(GetOsVer()); }

inline bool   IsKeyPressed (int vkey) { return GetKeyState (vkey) < 0; }

inline owl::uint16 SwapBytes (owl::uint16 w)  { return owl::uint16 ((w << 8) | (w >> 8)); }

////////////////////////////////////////////////////////////////////////////
//  C++ helper functions GetStockBrush, GetStockFont and GetStockPen:
//

// In case WINDOWSX.H was included:
#undef GetStockBrush
#undef GetStockPen
#undef GetStockFont
#undef SelectBrush
#undef SelectPen
#undef SelectFont

enum TStockBrush
{
    sbBlackBrush    = BLACK_BRUSH,
    sbDkGrayBrush   = DKGRAY_BRUSH,
    sbGrayBrush     = GRAY_BRUSH,
    sbHollowBrush   = HOLLOW_BRUSH,
    sbLtGrayBrush   = LTGRAY_BRUSH,
    sbNullBrush     = NULL_BRUSH,
    sbWhiteBrush    = WHITE_BRUSH
};

inline HBRUSH GetStockBrush (TStockBrush sb)
    { return (HBRUSH) GetStockObject (sb); }


enum TStockPen
{
    spWhitePen  = WHITE_PEN,
    spBlackPen  = BLACK_PEN,
    spNullPen   = NULL_PEN
};

inline HPEN GetStockPen (TStockPen sp)
    { return (HPEN) GetStockObject (sp); }


enum TStockFont
{
    sfAnsiFixedFont     = ANSI_FIXED_FONT,
    sfAnsiVarFont       = ANSI_VAR_FONT,
    sfDefaultGuiFont    = DEFAULT_GUI_FONT,
    sfDeviceDefaultFont = DEVICE_DEFAULT_FONT,
    sfOemFixedFont      = OEM_FIXED_FONT,
    sfSystemFont        = SYSTEM_FONT,
    sfSystemFixedFont   = SYSTEM_FIXED_FONT
};

inline HFONT  GetStockFont (TStockFont sf)
    { return (HFONT) GetStockObject (sf); }

inline HPALETTE GetStockPalette ()
    { return (HPALETTE) GetStockObject (DEFAULT_PALETTE); }

inline HBRUSH SelectBrush (HDC dc, HBRUSH hbr)
    { return (HBRUSH) ::SelectObject (dc, hbr); }

inline HPEN SelectPen (HDC dc, HPEN pen)
    { return (HPEN) ::SelectObject (dc, pen); }

inline HFONT SelectFont (HDC dc, HFONT font)
    { return (HFONT) ::SelectObject (dc, font); }



} // OwlExt namespace


#endif