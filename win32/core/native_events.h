/*
 * native_events.h:  What the backend reports to the core: input, focus, resizing,
 *   exposure and closing of the native windows it made for HWNDs.
 */

#ifndef W32_CORE_NATIVE_EVENTS_H
#define W32_CORE_NATIVE_EVENTS_H

#include "core/win32.h"

namespace w32::events {

// Mouse: msg is WM_MOUSEMOVE, WM_LBUTTONDOWN ... WM_MOUSEWHEEL; (x, y) in the
// top-level window's surface coordinates (its menu bar, then its client area); keys the MK_* flags; wheel in WHEEL_DELTA
// units for WM_MOUSEWHEEL.  The core finds the window under the point.
void Mouse(HWND top, UINT msg, int x, int y, UINT keys, int wheel = 0);
void MouseLeave(HWND top);

// Keyboard: vk is a VK_* code, scan the scan code; repeat when held down.  text is
// the UTF-8 the key typed (for WM_CHAR), if any.
void Key(HWND top, bool down, int vk, int scan, bool repeat, bool alt, const char *text);

void Focus(HWND top, bool focused);        // the top-level window gained/lost keyboard focus
void Activated(HWND top, bool active);
void Resized(HWND top, int width, int height);   // new surface size; surface reallocated
void Moved(HWND top, int x, int y);              // new client position, screen coordinates
void Exposed(HWND top, const RECT &rect);        // surface must be redrawn (client coordinates)
void CloseRequested(HWND top);                   // the user closed it: WM_CLOSE
void Minimized(HWND top, bool minimized);

}  // namespace w32::events

#endif
