/*
 * ctl_uxtheme.cpp:  uxtheme.dll with themes off, as Windows' classic scheme: no
 *   theme data opens, so the controls (Wine's comctl32) draw the classic way.
 *   Functions that need no theme (the parent's background, system colors and
 *   sizes) work.
 */

#include "core/win32.h"

// uxtheme.h puts the visibility attribute before STDAPI's extern "C", where C++
// ignores it (with a warning): the definitions below export themselves.
#pragma push_macro("DECLSPEC_IMPORT")
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#include <uxtheme.h>
#pragma pop_macro("DECLSPEC_IMPORT")

W32_EXPORT HRESULT WINAPI CloseThemeData(HTHEME theme)
{
   return theme ? E_HANDLE : S_OK;
}

W32_EXPORT HRESULT WINAPI DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *rect)
{
   // The parent paints its background and client area under the child, as
   // uxtheme does with or without a theme.
   HWND parent = GetParent(hwnd);
   if (parent == nullptr)
      return S_OK;
   RECT r;
   if (rect)
      r = *rect;
   else
      GetClientRect(hwnd, &r);
   MapWindowPoints(hwnd, parent, (POINT *) &r, 2);
   INT saved = SaveDC(hdc);
   HRGN clip = CreateRectRgnIndirect(rect ? rect : &r);
   if (rect)
      ExtSelectClipRgn(hdc, clip, RGN_AND);
   DeleteObject(clip);
   POINT org;
   OffsetViewportOrgEx(hdc, -r.left + (rect ? rect->left : 0), -r.top + (rect ? rect->top : 0), &org);
   SendMessageW(parent, WM_ERASEBKGND, (WPARAM) hdc, 0);
   SendMessageW(parent, WM_PRINTCLIENT, (WPARAM) hdc, PRF_CLIENT);
   RestoreDC(hdc, saved);
   return S_OK;
}

W32_EXPORT HRESULT WINAPI EnableThemeDialogTexture(HWND, DWORD)
{
   return S_OK;
}

W32_EXPORT HRESULT WINAPI EnableTheming(BOOL)
{
   return S_OK;
}

W32_EXPORT void WINAPI SetThemeAppProperties(DWORD)
{
}

W32_EXPORT HRESULT WINAPI SetWindowTheme(HWND hwnd, LPCWSTR, LPCWSTR)
{
   // Windows tells the window its theme changed.
   if (IsWindow(hwnd))
      SendMessageW(hwnd, WM_THEMECHANGED, 0, 0);
   return S_OK;
}

W32_EXPORT COLORREF WINAPI GetThemeSysColor(HTHEME, int id)
{
   return GetSysColor(id);
}

W32_EXPORT HBRUSH WINAPI GetThemeSysColorBrush(HTHEME, int id)
{
   return GetSysColorBrush(id);
}

W32_EXPORT int WINAPI GetThemeSysSize(HTHEME, int id)
{
   return GetSystemMetrics(id);
}

W32_EXPORT HRESULT WINAPI BufferedPaintInit(void)
{
   return S_OK;
}

W32_EXPORT HRESULT WINAPI BufferedPaintUnInit(void)
{
   return S_OK;
}

// Everything that needs theme data: there is none.

W32_EXPORT HRESULT WINAPI DrawThemeBackground(HTHEME,HDC,int,int,const RECT*,const RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI DrawThemeBackgroundEx(HTHEME,HDC,int,int,const RECT*, const DTBGOPTS*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI DrawThemeEdge(HTHEME,HDC,int,int,const RECT*,UINT,UINT, RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI DrawThemeIcon(HTHEME,HDC,int,int,const RECT*,HIMAGELIST,int)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI DrawThemeText(HTHEME,HDC,int,int,LPCWSTR,int,DWORD,DWORD, const RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI DrawThemeTextEx(HTHEME,HDC,int,int,LPCWSTR,int,DWORD,RECT*, const DTTOPTS*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetCurrentThemeName(LPWSTR,int,LPWSTR,int,LPWSTR,int)
{
   return E_HANDLE;
}

W32_EXPORT DWORD WINAPI GetThemeAppProperties(void)
{
   return 0;
}

W32_EXPORT HRESULT WINAPI GetThemeBackgroundContentRect(HTHEME,HDC,int,int, const RECT*,RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeBackgroundExtent(HTHEME,HDC,int,int,const RECT*,RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeBackgroundRegion(HTHEME,HDC,int,int,const RECT*,HRGN*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeBool(HTHEME,int,int,int,BOOL*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeColor(HTHEME,int,int,int,COLORREF*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeDocumentationProperty(LPCWSTR,LPCWSTR,LPWSTR,int)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeEnumValue(HTHEME,int,int,int,int*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeFilename(HTHEME,int,int,int,LPWSTR,int)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeFont(HTHEME,HDC,int,int,int,LOGFONTW*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeInt(HTHEME,int,int,int,int*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeIntList(HTHEME,int,int,int,INTLIST*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeMargins(HTHEME,HDC,int,int,int,RECT*,MARGINS*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeMetric(HTHEME,HDC,int,int,int,int*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemePartSize(HTHEME,HDC,int,int,RECT*,THEMESIZE,SIZE*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemePosition(HTHEME,int,int,int,POINT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemePropertyOrigin(HTHEME,int,int,int,PROPERTYORIGIN*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeRect(HTHEME,int,int,int,RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeString(HTHEME,int,int,int,LPWSTR,int)
{
   return E_HANDLE;
}

W32_EXPORT BOOL WINAPI GetThemeSysBool(HTHEME,int)
{
   return 0;
}

W32_EXPORT HRESULT WINAPI GetThemeSysFont(HTHEME,int,LOGFONTW*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeSysInt(HTHEME,int,int*)
{
   return E_HANDLE;
}


W32_EXPORT HRESULT WINAPI GetThemeSysString(HTHEME,int,LPWSTR,int)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeTextExtent(HTHEME,HDC,int,int,LPCWSTR,int,DWORD, const RECT*,RECT*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeTextMetrics(HTHEME,HDC,int,int,TEXTMETRICW*)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetThemeTransitionDuration(HTHEME,int,int,int,int,DWORD*)
{
   return E_HANDLE;
}

W32_EXPORT HTHEME WINAPI GetWindowTheme(HWND)
{
   return nullptr;
}

W32_EXPORT HRESULT WINAPI HitTestThemeBackground(HTHEME,HDC,int,int,DWORD,const RECT*, HRGN,POINT,WORD*)
{
   return E_HANDLE;
}

W32_EXPORT BOOL WINAPI IsAppThemed(void)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI IsCompositionActive(void)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI IsThemeActive(void)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI IsThemeBackgroundPartiallyTransparent(HTHEME,int,int)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI IsThemeDialogTextureEnabled(HWND)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI IsThemePartDefined(HTHEME,int,int)
{
   return FALSE;
}

W32_EXPORT HTHEME WINAPI OpenThemeData(HWND,LPCWSTR)
{
   return nullptr;
}

W32_EXPORT HTHEME WINAPI OpenThemeDataEx(HWND,LPCWSTR,DWORD)
{
   return nullptr;
}

W32_EXPORT HTHEME WINAPI OpenThemeDataForDpi(HWND,LPCWSTR,UINT)
{
   return nullptr;
}

W32_EXPORT HRESULT WINAPI SetWindowThemeAttribute(HWND,enum WINDOWTHEMEATTRIBUTETYPE,PVOID,DWORD)
{
   return E_HANDLE;
}

W32_EXPORT HPAINTBUFFER WINAPI BeginBufferedPaint(HDC, const RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *,HDC *)
{
   return nullptr;
}

W32_EXPORT HRESULT WINAPI EndBufferedPaint(HPAINTBUFFER, BOOL)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI BufferedPaintClear(HPAINTBUFFER, const RECT *)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI BufferedPaintSetAlpha(HPAINTBUFFER, const RECT *, BYTE)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI GetBufferedPaintBits(HPAINTBUFFER, RGBQUAD **, int *)
{
   return E_HANDLE;
}

W32_EXPORT HDC WINAPI GetBufferedPaintDC(HPAINTBUFFER)
{
   return nullptr;
}

W32_EXPORT HDC WINAPI GetBufferedPaintTargetDC(HPAINTBUFFER)
{
   return nullptr;
}

W32_EXPORT HRESULT WINAPI GetBufferedPaintTargetRect(HPAINTBUFFER, RECT *prc)
{
   return E_HANDLE;
}

W32_EXPORT HANIMATIONBUFFER WINAPI BeginBufferedAnimation(HWND, HDC, const RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *, BP_ANIMATIONPARAMS *, HDC *, HDC *)
{
   return nullptr;
}

W32_EXPORT BOOL WINAPI BufferedPaintRenderAnimation(HWND, HDC)
{
   return 0;
}

W32_EXPORT HRESULT WINAPI BufferedPaintStopAllAnimations(HWND)
{
   return E_HANDLE;
}

W32_EXPORT HRESULT WINAPI EndBufferedAnimation(HANIMATIONBUFFER, BOOL)
{
   return E_HANDLE;
}

