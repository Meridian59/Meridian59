/*
 * message.h:  The message queue's internals that other parts of the core use.
 */

#ifndef W32_CORE_MESSAGE_H
#define W32_CORE_MESSAGE_H

#include "core/win32.h"

#include <functional>
#include <string>

// Wine's winuser.h has these for its own sources only: Windows' internal timer
// message (SetSystemTimer) and SetScrollInfo's undocumented flag.
#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118
#endif
#ifndef SIF_RETURNPREV
#define SIF_RETURNPREV 0x1000
#endif

namespace w32 {

// Queues an input message for hwnd, as Windows posts input: in order with posted
// messages.  text: what a WM_KEYDOWN typed, for TranslateMessage's WM_CHAR.
void PostInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, const std::string &text = std::string());

// Runs call on the GUI thread, the caller waiting, as other threads' window
// creation and modal loops must (AppKit allows windows only there; the core's
// queue is that thread's).  On the GUI thread, calls it directly.
LRESULT OnGuiThread(const std::function<LRESULT()> &call);

// GetKeyState's state follows the input messages retrieved (core/sysparams.cpp);
// focus changes resynchronize it with the keyboard, whose releases may have
// gone to other programs.
void KeyStateFromMessage(const MSG &m);
void ResetKeyState();

// Called as windows go: drop their timers and pending paints.
void KillWindowTimers(HWND hwnd);
void ForgetPaint(HWND hwnd);

// The window that has the keyboard focus, the mouse capture, the active window.
HWND FocusWindow();
HWND CaptureWindow();
HWND ActiveWindow();
void SetActiveWindowState(HWND hwnd);

// Native controls follow these (core/controls).
void FocusChanged(HWND hwnd);
void ChildVisibilityChanged(HWND hwnd);
void ChildEnabledChanged(HWND hwnd);

// Painting: a window has something to repaint (its update region became
// non-empty), so a WM_PAINT will come.
void PaintPending();

}  // namespace w32

#endif
