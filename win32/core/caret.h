/*
 * caret.h:  The caret's hooks for painting and window destruction.
 */

#ifndef W32_CORE_CARET_H
#define W32_CORE_CARET_H

#include "core/win32.h"

namespace w32 {

// BeginPaint/EndPaint of hwnd: the caret is off while the window paints.
void CaretBeginPaint(HWND hwnd);
void CaretEndPaint(HWND hwnd);
// hwnd goes: so does its caret.
void CaretWindowDestroyed(HWND hwnd);
// Takes the caret off if it is in client_rect of hwnd (before its pixels move);
// true if it was, for CaretRestore afterwards.
bool CaretHideIn(HWND hwnd, const RECT &client_rect);
void CaretRestore();

}  // namespace w32

#endif
