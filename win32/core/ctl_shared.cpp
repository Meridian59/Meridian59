/*
 * ctl_shared.cpp:  What Wine's user32-style controls need from other parts of
 *   Windows: Unicode resource loaders and system parameters, accessibility and
 *   IME no-ops (no input method editor), drag detection, and a few kernel32
 *   functions.  Other areas own some of these; they are here until merged.
 */

#define OEMRESOURCE   // the OBM_* ids
#include "core/win32.h"

#include "core/window.h"
#include "core/winproc.h"

#include <commctrl.h>
#include <imm.h>

#include <cstring>

using namespace w32;

/************************************************************************/
// Resources and parameters

HCURSOR WINAPI LoadCursorW(HINSTANCE instance, LPCWSTR name)
{
   return LoadCursorA(instance, AnsiArg(name));
}

HICON WINAPI LoadIconW(HINSTANCE instance, LPCWSTR name)
{
   return LoadIconA(instance, AnsiArg(name));
}

// The OEM bitmaps (LoadBitmap(NULL, OBM_*)): Windows' classic glyphs, drawn.
static HBITMAP OemBitmap(UINT id)
{
   UINT type = DFC_SCROLL, state;
   int size = GetSystemMetrics(SM_CXVSCROLL);
   switch (id)
   {
   case OBM_COMBO: state = DFCS_SCROLLCOMBOBOX; break;
   case OBM_UPARROW: state = DFCS_SCROLLUP; break;
   case OBM_DNARROW: state = DFCS_SCROLLDOWN; break;
   case OBM_LFARROW: state = DFCS_SCROLLLEFT; break;
   case OBM_RGARROW: state = DFCS_SCROLLRIGHT; break;
   case OBM_UPARROWD: state = DFCS_SCROLLUP | DFCS_PUSHED; break;
   case OBM_DNARROWD: state = DFCS_SCROLLDOWN | DFCS_PUSHED; break;
   case OBM_LFARROWD: state = DFCS_SCROLLLEFT | DFCS_PUSHED; break;
   case OBM_RGARROWD: state = DFCS_SCROLLRIGHT | DFCS_PUSHED; break;
   case OBM_UPARROWI: state = DFCS_SCROLLUP | DFCS_INACTIVE; break;
   case OBM_DNARROWI: state = DFCS_SCROLLDOWN | DFCS_INACTIVE; break;
   case OBM_LFARROWI: state = DFCS_SCROLLLEFT | DFCS_INACTIVE; break;
   case OBM_RGARROWI: state = DFCS_SCROLLRIGHT | DFCS_INACTIVE; break;
   case OBM_CHECK: type = DFC_MENU, state = DFCS_MENUCHECK, size = 13; break;
   case OBM_MNARROW: type = DFC_MENU, state = DFCS_MENUARROW, size = 13; break;
   case OBM_CLOSE: type = DFC_CAPTION, state = DFCS_CAPTIONCLOSE, size = GetSystemMetrics(SM_CYCAPTION) - 2; break;
   default: return nullptr;
   }
   HDC screen = GetDC(nullptr);
   HBITMAP bitmap = CreateCompatibleBitmap(screen, size, size);
   ReleaseDC(nullptr, screen);
   HDC dc = CreateCompatibleDC(nullptr);
   HGDIOBJ old = SelectObject(dc, bitmap);
   RECT r = {0, 0, size, size};
   DrawFrameControl(dc, &r, type, state);
   SelectObject(dc, old);
   DeleteDC(dc);
   return bitmap;
}

HBITMAP WINAPI LoadBitmapW(HINSTANCE instance, LPCWSTR name)
{
   if (instance == nullptr && IS_INTRESOURCE(name))
      return OemBitmap(LOWORD(name));
   return LoadBitmapA(instance, AnsiArg(name));
}

static void FontAtoW(const LOGFONTA &a, LOGFONTW *w)
{
   memcpy(w, &a, offsetof(LOGFONTA, lfFaceName));
   std::basic_string<WCHAR> face = AnsiToWide(a.lfFaceName);
   memset(w->lfFaceName, 0, sizeof(w->lfFaceName));
   memcpy(w->lfFaceName, face.data(), std::min(face.size(), (size_t) LF_FACESIZE - 1) * sizeof(WCHAR));
}

BOOL WINAPI SystemParametersInfoW(UINT action, UINT param, LPVOID out, UINT ini)
{
   switch (action)
   {
   case SPI_GETNONCLIENTMETRICS:
   {
      auto *w = (NONCLIENTMETRICSW *) out;
      if (w == nullptr)
         return FALSE;
      NONCLIENTMETRICSA a = {};
      a.cbSize = sizeof(a);
      if (!SystemParametersInfoA(action, sizeof(a), &a, ini))
         return FALSE;
      UINT size = w->cbSize;
      memset(w, 0, sizeof(*w));
      w->cbSize = size;
      w->iBorderWidth = a.iBorderWidth;
      w->iScrollWidth = a.iScrollWidth;
      w->iScrollHeight = a.iScrollHeight;
      w->iCaptionWidth = a.iCaptionWidth;
      w->iCaptionHeight = a.iCaptionHeight;
      w->iSmCaptionWidth = a.iSmCaptionWidth;
      w->iSmCaptionHeight = a.iSmCaptionHeight;
      w->iMenuWidth = a.iMenuWidth;
      w->iMenuHeight = a.iMenuHeight;
      FontAtoW(a.lfCaptionFont, &w->lfCaptionFont);
      FontAtoW(a.lfSmCaptionFont, &w->lfSmCaptionFont);
      FontAtoW(a.lfMenuFont, &w->lfMenuFont);
      FontAtoW(a.lfStatusFont, &w->lfStatusFont);
      FontAtoW(a.lfMessageFont, &w->lfMessageFont);
      return TRUE;
   }
   case SPI_GETICONTITLELOGFONT:
   {
      if (out == nullptr)
         return FALSE;
      NONCLIENTMETRICSA a = {};
      a.cbSize = sizeof(a);
      SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(a), &a, 0);
      FontAtoW(a.lfMessageFont, (LOGFONTW *) out);
      return TRUE;
   }
   case SPI_GETKEYBOARDCUES:
   case SPI_GETFLATMENU:
   case SPI_GETGRADIENTCAPTIONS:
   case SPI_GETCOMBOBOXANIMATION:
   case SPI_GETLISTBOXSMOOTHSCROLLING:
   case SPI_GETHOTTRACKING:
   case SPI_GETMENUANIMATION:
   case SPI_GETTOOLTIPANIMATION:
   case SPI_GETSELECTIONFADE:
   case SPI_GETUIEFFECTS:
   case SPI_GETCLIENTAREAANIMATION:
      // The classic, plain behaviour: no cues hidden, no animation.
      if (out == nullptr)
         return FALSE;
      *(BOOL *) out = action == SPI_GETKEYBOARDCUES;
      return TRUE;
   case SPI_GETFONTSMOOTHING:
      if (out)
         *(BOOL *) out = TRUE;
      return out != nullptr;
   case SPI_GETFOCUSBORDERWIDTH:
   case SPI_GETFOCUSBORDERHEIGHT:
      if (out)
         *(UINT *) out = 1;
      return out != nullptr;
   case SPI_GETCARETWIDTH:
      if (out)
         *(DWORD *) out = 1;
      return out != nullptr;
   case SPI_GETMOUSEHOVERWIDTH:
   case SPI_GETMOUSEHOVERHEIGHT:
      if (out)
         *(UINT *) out = 4;
      return out != nullptr;
   case SPI_GETMOUSEHOVERTIME:
      if (out)
         *(UINT *) out = 400;
      return out != nullptr;
   case SPI_GETDRAGFULLWINDOWS:
      if (out)
         *(BOOL *) out = TRUE;
      return out != nullptr;
   }
   return SystemParametersInfoA(action, param, out, ini);
}

BOOL WINAPI GetMonitorInfoW(HMONITOR monitor, MONITORINFO *info)
{
   if (info == nullptr)
      return FALSE;
   if (info->cbSize >= sizeof(MONITORINFOEXW))
   {
      MONITORINFOEXA a = {};
      a.cbSize = sizeof(a);
      if (!GetMonitorInfoA(monitor, (MONITORINFO *) &a))
         return FALSE;
      auto *w = (MONITORINFOEXW *) info;
      w->rcMonitor = a.rcMonitor;
      w->rcWork = a.rcWork;
      w->dwFlags = a.dwFlags;
      std::basic_string<WCHAR> device = AnsiToWide(a.szDevice);
      lstrcpynW(w->szDevice, device.c_str(), CCHDEVICENAME);
      return TRUE;
   }
   return GetMonitorInfoA(monitor, info);
}

UINT WINAPI GetDpiForWindow(HWND hwnd)
{
   return IsWindow(hwnd) ? 96 : 0;
}

INT WINAPI InternalGetWindowText(HWND hwnd, LPWSTR text, INT size)
{
   // The window's own text, without asking its procedure.
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || text == nullptr || size <= 0)
      return 0;
   std::basic_string<WCHAR> s = AnsiToWide(w->text.c_str(), (int) w->text.size());
   lstrcpynW(text, s.c_str(), size);
   return lstrlenW(text);
}

/************************************************************************/
// Input

BOOL WINAPI DragDetect(HWND hwnd, POINT pt)
{
   // As Windows: with the capture, until the button goes up (no drag) or the
   // pointer leaves the drag rectangle around pt (screen coordinates) (drag).
   RECT rect;
   int cx = GetSystemMetrics(SM_CXDRAG), cy = GetSystemMetrics(SM_CYDRAG);
   SetRect(&rect, pt.x - cx, pt.y - cy, pt.x + cx, pt.y + cy);
   SetCapture(hwnd);
   for (;;)
   {
      MSG msg;
      if (!GetMessageA(&msg, nullptr, 0, 0))
      {
         PostQuitMessage((INT) msg.wParam);
         break;
      }
      if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_MBUTTONUP)
      {
         ReleaseCapture();
         return FALSE;
      }
      if (msg.message == WM_MOUSEMOVE)
      {
         POINT p = {(SHORT) LOWORD(msg.lParam), (SHORT) HIWORD(msg.lParam)};
         ClientToScreen(msg.hwnd, &p);
         if (!PtInRect(&rect, p))
         {
            ReleaseCapture();
            return TRUE;
         }
         continue;
      }
      if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
         break;
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
      if (GetCapture() != hwnd)
         return FALSE;
   }
   ReleaseCapture();
   return FALSE;
}

void WINAPI NotifyWinEvent(DWORD, HWND, LONG, LONG)
{
   // Accessibility events: there are no listeners.
}

/************************************************************************/
// The input method editor: none, as with a keyboard layout without one.

W32_EXPORT HIMC WINAPI ImmGetContext(HWND)
{
   return nullptr;
}

W32_EXPORT BOOL WINAPI ImmReleaseContext(HWND, HIMC)
{
   return TRUE;
}

W32_EXPORT HIMC WINAPI ImmAssociateContext(HWND, HIMC)
{
   return nullptr;
}

W32_EXPORT BOOL WINAPI ImmSetCompositionWindow(HIMC, COMPOSITIONFORM *)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI ImmSetCompositionFontW(HIMC, LOGFONTW *)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI ImmSetCompositionFontA(HIMC, LOGFONTA *)
{
   return FALSE;
}

W32_EXPORT LONG WINAPI ImmGetCompositionStringW(HIMC, DWORD, LPVOID, DWORD)
{
   return IMM_ERROR_GENERAL;
}

W32_EXPORT LONG WINAPI ImmGetCompositionStringA(HIMC, DWORD, LPVOID, DWORD)
{
   return IMM_ERROR_GENERAL;
}

W32_EXPORT BOOL WINAPI ImmIsIME(HKL)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI ImmGetOpenStatus(HIMC)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI ImmSetOpenStatus(HIMC, BOOL)
{
   return FALSE;
}

W32_EXPORT BOOL WINAPI ImmNotifyIME(HIMC, DWORD, DWORD, DWORD)
{
   return FALSE;
}

W32_EXPORT HWND WINAPI ImmGetDefaultIMEWnd(HWND)
{
   return nullptr;
}

/************************************************************************/
// kernel32

DWORD WINAPI GetVersion(void)
{
   // As GetVersionExA reports: the low word is major.minor, the high the build.
   OSVERSIONINFOA v = {};
   v.dwOSVersionInfoSize = sizeof(v);
   if (!GetVersionExA(&v))
      return 0x0A28000A;
   return (v.dwMajorVersion & 0xFF) | ((v.dwMinorVersion & 0xFF) << 8) | ((v.dwBuildNumber & 0x7FFF) << 16);
}

BOOL WINAPI IsValidLocale(LCID lcid, DWORD)
{
   return PRIMARYLANGID(LANGIDFROMLCID(lcid)) != LANG_NEUTRAL || lcid == LOCALE_SYSTEM_DEFAULT ||
          lcid == LOCALE_USER_DEFAULT;
}

UINT WINAPI GetDriveTypeW(LPCWSTR root)
{
   // One drive, C:, the file system the client's paths resolve on.
   if (root == nullptr || ((root[0] == 'C' || root[0] == 'c') && root[1] == ':' && (root[2] == 0 || root[2] == '\\')))
      return DRIVE_FIXED;
   return DRIVE_NO_ROOT_DIR;
}

static void FindDataAtoW(const WIN32_FIND_DATAA &a, WIN32_FIND_DATAW *w)
{
   memset(w, 0, sizeof(*w));
   w->dwFileAttributes = a.dwFileAttributes;
   w->ftCreationTime = a.ftCreationTime;
   w->ftLastAccessTime = a.ftLastAccessTime;
   w->ftLastWriteTime = a.ftLastWriteTime;
   w->nFileSizeHigh = a.nFileSizeHigh;
   w->nFileSizeLow = a.nFileSizeLow;
   std::basic_string<WCHAR> name = AnsiToWide(a.cFileName), alt = AnsiToWide(a.cAlternateFileName);
   lstrcpynW(w->cFileName, name.c_str(), MAX_PATH);
   lstrcpynW(w->cAlternateFileName, alt.c_str(), 14);
}

HANDLE WINAPI FindFirstFileW(LPCWSTR pattern, WIN32_FIND_DATAW *data)
{
   WIN32_FIND_DATAA a;
   HANDLE h = FindFirstFileA(WideToAnsi(pattern).c_str(), &a);
   if (h != INVALID_HANDLE_VALUE && data)
      FindDataAtoW(a, data);
   return h;
}

BOOL WINAPI FindNextFileW(HANDLE h, WIN32_FIND_DATAW *data)
{
   WIN32_FIND_DATAA a;
   if (!FindNextFileA(h, &a))
      return FALSE;
   if (data)
      FindDataAtoW(a, data);
   return TRUE;
}

/************************************************************************/
// Image lists (comctl32's imagelist.c, another area): until it is built in, no
// image list loads, so buttons never have one.

extern "C" {

__attribute__((weak)) HIMAGELIST WINAPI ImageList_LoadImageW(HINSTANCE, LPCWSTR, INT, INT, COLORREF, UINT, UINT)
{
   return nullptr;
}

__attribute__((weak)) BOOL WINAPI ImageList_Destroy(HIMAGELIST)
{
   return FALSE;
}

__attribute__((weak)) BOOL WINAPI ImageList_Draw(HIMAGELIST, INT, HDC, INT, INT, UINT)
{
   return FALSE;
}

__attribute__((weak)) BOOL WINAPI ImageList_GetIconSize(HIMAGELIST, INT *cx, INT *cy)
{
   if (cx)
      *cx = 0;
   if (cy)
      *cy = 0;
   return FALSE;
}

__attribute__((weak)) INT WINAPI ImageList_GetImageCount(HIMAGELIST)
{
   return 0;
}

}  // extern "C"

/************************************************************************/
// Window helpers the controls' users need

BOOL WINAPI GetComboBoxInfo(HWND hwnd, COMBOBOXINFO *info)
{
   return (BOOL) SendMessageW(hwnd, CB_GETCOMBOBOXINFO, 0, (LPARAM) info);
}

HWND WINAPI GetAncestor(HWND hwnd, UINT type)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   switch (type)
   {
   case GA_PARENT:
      // The parent, or the desktop for top-level windows.
      return IsChildWindow(w) ? w->parent : GetDesktopWindow();
   case GA_ROOT:
      return TopLevelOf(hwnd);
   case GA_ROOTOWNER:
   {
      HWND root = TopLevelOf(hwnd);
      for (Window *r = GetWindowObject(root); r && r->owner; r = GetWindowObject(r->owner))
         root = r->owner;
      return root;
   }
   }
   return nullptr;
}
