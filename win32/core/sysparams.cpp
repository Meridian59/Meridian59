/*
 * sysparams.cpp:  System metrics (Windows' classic values, which the client's
 *   layout assumes) and colors (Windows 10's), monitors, keyboard state and the
 *   cursor.
 */

#include "core/gdi.h"
#include "core/message.h"
#include "core/native.h"
#include "core/native_events.h"
#include "core/window.h"

#include <cstring>

namespace w32 {

namespace {

// Windows 10's colors (what GetSysColor answers there, classic controls and
// all), by COLOR_* index.
const COLORREF kSysColors[] = {
   RGB(200, 200, 200),   // COLOR_SCROLLBAR
   RGB(0, 0, 0),         // COLOR_BACKGROUND
   RGB(153, 180, 209),   // COLOR_ACTIVECAPTION
   RGB(191, 205, 219),   // COLOR_INACTIVECAPTION
   RGB(240, 240, 240),   // COLOR_MENU
   RGB(255, 255, 255),   // COLOR_WINDOW
   RGB(100, 100, 100),   // COLOR_WINDOWFRAME
   RGB(0, 0, 0),         // COLOR_MENUTEXT
   RGB(0, 0, 0),         // COLOR_WINDOWTEXT
   RGB(0, 0, 0),         // COLOR_CAPTIONTEXT
   RGB(180, 180, 180),   // COLOR_ACTIVEBORDER
   RGB(244, 247, 252),   // COLOR_INACTIVEBORDER
   RGB(171, 171, 171),   // COLOR_APPWORKSPACE
   RGB(0, 120, 215),     // COLOR_HIGHLIGHT
   RGB(255, 255, 255),   // COLOR_HIGHLIGHTTEXT
   RGB(240, 240, 240),   // COLOR_BTNFACE
   RGB(160, 160, 160),   // COLOR_BTNSHADOW
   RGB(109, 109, 109),   // COLOR_GRAYTEXT
   RGB(0, 0, 0),         // COLOR_BTNTEXT
   RGB(0, 0, 0),         // COLOR_INACTIVECAPTIONTEXT
   RGB(255, 255, 255),   // COLOR_BTNHIGHLIGHT
   RGB(105, 105, 105),   // COLOR_3DDKSHADOW
   RGB(227, 227, 227),   // COLOR_3DLIGHT
   RGB(0, 0, 0),         // COLOR_INFOTEXT
   RGB(255, 255, 225),   // COLOR_INFOBK
   RGB(181, 181, 181),   // 25 (unused)
   RGB(0, 102, 204),     // COLOR_HOTLIGHT
   RGB(185, 209, 234),   // COLOR_GRADIENTACTIVECAPTION
   RGB(215, 228, 242),   // COLOR_GRADIENTINACTIVECAPTION
   RGB(51, 153, 255),    // COLOR_MENUHILIGHT
   RGB(240, 240, 240),   // COLOR_MENUBAR
};

}  // namespace

}  // namespace w32

using namespace w32;

DWORD WINAPI GetSysColor(INT index)
{
   if (index < 0 || index >= (INT) (sizeof(kSysColors) / sizeof(kSysColors[0])))
      return 0;
   return kSysColors[index];
}

HBRUSH WINAPI GetSysColorBrush(INT index)
{
   static HBRUSH brushes[sizeof(kSysColors) / sizeof(kSysColors[0])];
   if (index < 0 || index >= (INT) (sizeof(kSysColors) / sizeof(kSysColors[0])))
      return nullptr;
   if (brushes[index] == nullptr)
   {
      auto *b = new GdiBrush;
      b->color = kSysColors[index];
      b->sys_color = index;
      b->stock = true;
      brushes[index] = (HBRUSH) AddGdiObject(b);
   }
   return brushes[index];
}

INT WINAPI GetSystemMetrics(INT index)
{
   RECT screen = native::ScreenRect(), work = native::WorkArea();
   switch (index)
   {
   case SM_CXSCREEN: return screen.right - screen.left;
   case SM_CYSCREEN: return screen.bottom - screen.top;
   case SM_CXVIRTUALSCREEN: return screen.right - screen.left;
   case SM_CYVIRTUALSCREEN: return screen.bottom - screen.top;
   case SM_XVIRTUALSCREEN: case SM_YVIRTUALSCREEN: return 0;
   case SM_CXFULLSCREEN: return work.right - work.left;
   case SM_CYFULLSCREEN: return work.bottom - work.top;
   case SM_CXMAXIMIZED: return work.right - work.left;
   case SM_CYMAXIMIZED: return work.bottom - work.top;
   case SM_CXVSCROLL: case SM_CYHSCROLL: return 16;
   case SM_CYVSCROLL: case SM_CXHSCROLL: return 16;
   case SM_CXHTHUMB: case SM_CYVTHUMB: return 16;
   case SM_CYCAPTION: return 19;
   case SM_CYSMCAPTION: return 15;
   case SM_CYMENU: return 19;
   case SM_CXBORDER: case SM_CYBORDER: return 1;
   case SM_CXEDGE: case SM_CYEDGE: return 2;
   case SM_CXDLGFRAME: case SM_CYDLGFRAME: return 3;
   case SM_CXFRAME: case SM_CYFRAME: return 4;
   case SM_CXICON: case SM_CYICON: return 32;
   case SM_CXSMICON: case SM_CYSMICON: return 16;
   case SM_CXCURSOR: case SM_CYCURSOR: return 32;
   case SM_CXDOUBLECLK: case SM_CYDOUBLECLK: return 4;
   case SM_CXDRAG: case SM_CYDRAG: return 4;
   case SM_CMOUSEBUTTONS: return 3;
   case SM_MOUSEPRESENT: return 1;
   case SM_MOUSEWHEELPRESENT: return 1;
   case SM_CMONITORS: return 1;
   case SM_SWAPBUTTON: return 0;
   case SM_CXMIN: return 112;
   case SM_CYMIN: return 27;
   case SM_CXMINTRACK: return 112;
   case SM_CYMINTRACK: return 27;
   case SM_CXMAXTRACK: return screen.right - screen.left + 12;
   case SM_CYMAXTRACK: return screen.bottom - screen.top + 12;
   }
   return 0;
}

BOOL WINAPI SystemParametersInfoA(UINT action, UINT param, PVOID out, UINT)
{
   switch (action)
   {
   case SPI_GETWORKAREA:
      if (out)
         *(RECT *) out = native::WorkArea();
      return TRUE;
   case SPI_GETNONCLIENTMETRICS:
   {
      auto *m = (NONCLIENTMETRICSA *) out;
      if (m == nullptr)
         return FALSE;
      UINT size = m->cbSize;
      memset(m, 0, sizeof(*m));
      m->cbSize = size;
      m->iBorderWidth = 1;
      m->iScrollWidth = m->iScrollHeight = 16;
      m->iCaptionWidth = m->iCaptionHeight = 18;
      m->iMenuWidth = m->iMenuHeight = 18;
      m->iSmCaptionWidth = m->iSmCaptionHeight = 14;
      // Windows 11's fonts: Segoe UI 9 points, all of them.
      LOGFONTA lf = {};
      lf.lfHeight = -12;
      lf.lfWeight = FW_NORMAL;
      lf.lfCharSet = DEFAULT_CHARSET;
      strcpy(lf.lfFaceName, "Segoe UI");
      m->lfMenuFont = m->lfStatusFont = m->lfMessageFont = m->lfSmCaptionFont = m->lfCaptionFont = lf;
      return TRUE;
   }
   case SPI_GETKEYBOARDDELAY:
      if (out)
         *(INT *) out = 1;
      return TRUE;
   case SPI_GETKEYBOARDSPEED:
      if (out)
         *(DWORD *) out = 31;
      return TRUE;
   case SPI_GETMOUSESPEED:
      if (out)
         *(INT *) out = 10;
      return TRUE;
   case SPI_GETWHEELSCROLLLINES:
      if (out)
         *(UINT *) out = 3;
      return TRUE;
   case SPI_GETSCREENSAVEACTIVE:
      if (out)
         *(BOOL *) out = FALSE;
      return TRUE;
   case SPI_SETSCREENSAVEACTIVE:
   case SPI_SETKEYBOARDSPEED:
   case SPI_SETKEYBOARDDELAY:
      return TRUE;
   }
   (void) param;
   return FALSE;
}

/************************************************************************/
// Monitors: one, the primary screen.

static HMONITOR const kMonitor = (HMONITOR) (UINT_PTR) 0x12340000;

HMONITOR WINAPI MonitorFromWindow(HWND, DWORD)
{
   return kMonitor;
}

HMONITOR WINAPI MonitorFromPoint(POINT, DWORD)
{
   return kMonitor;
}

HMONITOR WINAPI MonitorFromRect(const RECT *, DWORD)
{
   return kMonitor;
}

BOOL WINAPI GetMonitorInfoA(HMONITOR monitor, MONITORINFO *info)
{
   if (monitor != kMonitor || info == nullptr)
      return FALSE;
   info->rcMonitor = native::ScreenRect();
   info->rcWork = native::WorkArea();
   info->dwFlags = MONITORINFOF_PRIMARY;
   if (info->cbSize >= sizeof(MONITORINFOEXA))
      strcpy(((MONITORINFOEXA *) info)->szDevice, "\\\\.\\DISPLAY1");
   return TRUE;
}

BOOL WINAPI EnumDisplaySettingsA(LPCSTR, DWORD mode, DEVMODEA *dm)
{
   // The current mode only.
   if (dm == nullptr || (mode != ENUM_CURRENT_SETTINGS && mode != ENUM_REGISTRY_SETTINGS && mode != 0))
      return FALSE;
   RECT screen = native::ScreenRect();
   WORD size = dm->dmSize;
   memset(dm, 0, sizeof(*dm));
   dm->dmSize = size ? size : sizeof(*dm);
   dm->dmPelsWidth = screen.right - screen.left;
   dm->dmPelsHeight = screen.bottom - screen.top;
   dm->dmBitsPerPel = 32;
   dm->dmDisplayFrequency = 60;
   dm->dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
   return TRUE;
}

HRESULT WINAPI DwmSetWindowAttribute(HWND, DWORD, LPCVOID, DWORD)
{
   // Dark title bars and the like: the toolkit's theme decides.
   return S_OK;
}

BOOL WINAPI MessageBeep(UINT)
{
   native::Beep();
   return TRUE;
}

/************************************************************************/
// Keyboard state

// As Windows: the keys' and mouse buttons' state as of the input messages the
// program has retrieved (a key tapped between two frames is still down while
// its WM_KEYDOWN is handled), not the keyboard's now.  Toggles, and the sided
// modifiers (VK_LSHIFT ...), which no message carries, are the keyboard's.
static BYTE queue_keys[256];

static bool Sided(int vk)
{
   return vk >= VK_LSHIFT && vk <= VK_RMENU;
}

static bool QueueKeyDown(int vk)
{
   if (vk < 0 || vk > 255)
      return false;
   return Sided(vk) ? native::KeyDown(vk) : (queue_keys[vk] & 0x80) != 0;
}

namespace w32 {

void KeyStateFromMessage(const MSG &m)
{
   auto set = [](int vk, bool down) { queue_keys[vk] = down ? 0x80 : 0; };
   switch (m.message)
   {
   case WM_KEYDOWN: case WM_SYSKEYDOWN: set(m.wParam & 0xFF, true); break;
   case WM_KEYUP: case WM_SYSKEYUP: set(m.wParam & 0xFF, false); break;
   case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: set(VK_LBUTTON, true); break;
   case WM_LBUTTONUP: set(VK_LBUTTON, false); break;
   case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: set(VK_RBUTTON, true); break;
   case WM_RBUTTONUP: set(VK_RBUTTON, false); break;
   case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: set(VK_MBUTTON, true); break;
   case WM_MBUTTONUP: set(VK_MBUTTON, false); break;
   }
}

void ResetKeyState()
{
   for (int vk = 0; vk < 256; vk++)
      queue_keys[vk] = native::KeyDown(vk) ? 0x80 : 0;
}

}  // namespace w32

SHORT WINAPI GetKeyState(INT vk)
{
   // High bit: down; low bit: toggled.
   return (SHORT) ((QueueKeyDown(vk) ? 0x8000 : 0) | (native::KeyToggled(vk) ? 1 : 0));
}

SHORT WINAPI GetAsyncKeyState(INT vk)
{
   return native::KeyDown(vk) ? (SHORT) 0x8000 : 0;
}

// Synthesized keys go where the toolkit's do, typing what a US layout would
// for letters (with Ctrl, their control characters), digits, space, Return and
// Backspace.
VOID WINAPI keybd_event(BYTE vk, BYTE scan, DWORD flags, ULONG_PTR)
{
   HWND focus = GetFocus();
   if (focus == nullptr)
      return;
   bool down = !(flags & KEYEVENTF_KEYUP), shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
   bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
   char text[2] = "";
   if (down)
   {
      if (vk >= 'A' && vk <= 'Z')
         text[0] = (char) (control ? vk - 'A' + 1 : shift ? vk : vk - 'A' + 'a');
      else if ((vk >= '0' && vk <= '9') || vk == VK_SPACE || vk == VK_RETURN || vk == VK_BACK)
         text[0] = (char) (vk == VK_RETURN ? '\r' : vk);
   }
   events::Key(TopLevelOf(focus), down, vk, scan, false, (GetKeyState(VK_MENU) & 0x8000) != 0, text);
}

BOOL WINAPI GetKeyboardState(BYTE *state)
{
   if (state == nullptr)
      return FALSE;
   for (int vk = 0; vk < 256; vk++)
      state[vk] = (BYTE) ((QueueKeyDown(vk) ? 0x80 : 0) | (native::KeyToggled(vk) ? 1 : 0));
   return TRUE;
}

BOOL WINAPI SetKeyboardState(BYTE *state)
{
   // The program's view of the keys, until messages change it (the client
   // clears held keys this way).
   if (state == nullptr)
      return FALSE;
   for (int vk = 0; vk < 256; vk++)
      queue_keys[vk] = state[vk] & 0x80;
   return TRUE;
}

/************************************************************************/
// The cursor

BOOL WINAPI GetCursorPos(POINT *p)
{
   if (p == nullptr)
      return FALSE;
   *p = native::GetCursorPos();
   return TRUE;
}

BOOL WINAPI SetCursorPos(INT x, INT y)
{
   return native::SetCursorPos(x, y);
}

static HCURSOR current_cursor = nullptr;

HCURSOR WINAPI SetCursor(HCURSOR cursor)
{
   HCURSOR old = current_cursor;
   current_cursor = cursor;
   // The cursor shows over the window the pointer is in (or that has the capture).
   HWND over = GetCapture();
   if (over == nullptr)
      over = WindowFromPoint(native::GetCursorPos());
   if (Window *top = GetWindowObject(TopLevelOf(over)); top && top->top)
      native::SetCursor(top->top, cursor);
   return old;
}

HCURSOR WINAPI GetCursor(void)
{
   return current_cursor;
}

static int cursor_count = 0;

INT WINAPI ShowCursor(BOOL show)
{
   // A display count: the cursor shows while it is >= 0.
   bool was_shown = cursor_count >= 0;
   cursor_count += show ? 1 : -1;
   bool shown = cursor_count >= 0;
   if (shown != was_shown)
   {
      ForEachTopLevel([](HWND h, void *s) {
         if (Window *w = GetWindowObject(h); w && w->top)
            native::ShowCursor(w->top, *(bool *) s);
      }, &shown);
   }
   return cursor_count;
}

BOOL WINAPI TrackMouseEvent(TRACKMOUSEEVENT *tme)
{
   // WM_MOUSELEAVE comes from the backend's leave events for tracked windows.
   if (tme == nullptr)
      return FALSE;
   if (tme->dwFlags & TME_LEAVE)
      TrackLeave(tme->hwndTrack, !(tme->dwFlags & TME_CANCEL));
   return TRUE;
}

UINT WINAPI GetDoubleClickTime(void)
{
   return 500;
}
