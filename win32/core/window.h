/*
 * window.h:  Windows and window classes: the tree of HWNDs, their procedures,
 *   styles, geometry and state.  Native toolkit objects only back top-level
 *   windows (decorations, input, a surface GDI draws on) and native controls.
 */

#ifndef W32_CORE_WINDOW_H
#define W32_CORE_WINDOW_H

#include "core/win32.h"

#include <commctrl.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace w32 {

namespace native {
class Top;       // backend: a top-level window
class Control;   // backend: a native control (button, edit ...)
}

struct WindowClass
{
   std::string name;
   WNDPROC proc = nullptr;
   bool unicode = false;               // proc takes Unicode messages (RegisterClassW)
   UINT style = 0;
   int class_extra = 0, window_extra = 0;
   HINSTANCE instance = nullptr;
   HICON icon = nullptr, icon_small = nullptr;
   HCURSOR cursor = nullptr;
   HBRUSH background = nullptr;
   std::string menu_name;
   std::vector<BYTE> extra;            // cbClsExtra bytes
   bool system = false;                // built-in (Button, Edit ...), not unregistrable
};

struct Window
{
   HWND hwnd = nullptr;
   WindowClass *cls = nullptr;
   WNDPROC proc = nullptr;
   bool unicode = false;               // proc takes Unicode messages
   DWORD style = 0, exstyle = 0;
   HWND parent = nullptr;              // child windows: the parent; others: none
   HWND owner = nullptr;               // top-level windows: the owner
   std::vector<HWND> children;         // in Z order, topmost first
   HINSTANCE instance = nullptr;
   UINT_PTR id = 0;                    // child: control id; top-level: the menu (HMENU)
   std::string text;

   // Geometry.  rect: the window, in the parent's client coordinates (child) or
   // screen coordinates (top-level).  client: the client area, relative to rect's
   // top-left.  Top-level windows have no non-client area of ours: the toolkit's
   // decorations are outside rect.
   RECT rect = {};
   RECT client = {};

   bool visible = false;
   bool enabled = true;
   bool minimized = false, maximized = false;
   bool destroying = false;

   LONG_PTR user_data = 0;
   std::vector<BYTE> extra;            // cbWndExtra bytes (DWLP_MSGRESULT ... for dialogs)
   std::map<std::string, HANDLE> props;

   // Painting: the part of the client area to repaint, in client coordinates.
   HRGN update = nullptr;
   bool erase_pending = false;
   bool internal_paint = false;

   // Backend objects.
   native::Top *top = nullptr;         // top-level windows
   native::Control *control = nullptr; // native controls
   void *control_state = nullptr;      // the control procedure's data (core/controls)
};

Window *GetWindowObject(HWND hwnd);

// The top-level ancestor (itself for a top-level window).
HWND TopLevelOf(HWND hwnd);

// rect of hwnd's client area, in the client coordinates of its top-level window.
RECT ClientRectInTop(HWND hwnd);

bool IsChildWindow(const Window *w);

// Sets w->client from its rect, style and menu bar.
void ComputeClient(Window *w);

// Calls the window's procedure with a message in ANSI form (CallProcW: Unicode),
// translating for a procedure of the other set.
LRESULT CallProc(Window *w, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CallProcW(Window *w, UINT msg, WPARAM wparam, LPARAM lparam);

// The class registered under name (case-insensitive) for instance, or a system
// class; nullptr if none.
WindowClass *FindClass(const char *name, HINSTANCE instance);

// Registers a built-in class (controls); proc handles its messages.
void RegisterSystemClass(const char *name, WNDPROC proc, UINT style, int window_extra,
                         HCURSOR cursor, HBRUSH background, bool unicode = false);

// Invalidates the window's whole client area (and its children's).
void InvalidateWhole(HWND hwnd, bool erase);

// Erases now what hwnd and its descendants have pending (WM_NCPAINT,
// WM_ERASEBKGND), as Windows does when a window is shown or a child hidden, so
// a later BeginPaint doesn't erase over what was drawn meanwhile.
void EraseNow(HWND hwnd);

// Calls f(hwnd, data) for each top-level window.
void ForEachTopLevel(void (*f)(HWND, void *), void *data);

// TrackMouseEvent(TME_LEAVE): WM_MOUSELEAVE when the pointer leaves hwnd.
void TrackLeave(HWND hwnd, bool track);

}  // namespace w32

#endif
