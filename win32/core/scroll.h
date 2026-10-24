/*
 * scroll.h:  The standard scroll bars' part in the non-client area (drawing,
 *   hit testing, tracking), for DefWindowProc.
 */

#ifndef W32_CORE_SCROLL_H
#define W32_CORE_SCROLL_H

#include "core/message.h"

namespace w32 {

void RegisterScrollBarClass();

// Draws hwnd's WS_VSCROLL/WS_HSCROLL bars on its window DC hdc.
void DrawStandardScrollBars(HWND hwnd, HDC hdc);

// HTVSCROLL, HTHSCROLL, HTSIZE (the corner) or HTNOWHERE for a point in window
// coordinates of hwnd's non-client area.
LRESULT ScrollBarHitTest(HWND hwnd, POINT window_pt);

// A button press on a bar (SB_CTL: client coordinates; SB_VERT/SB_HORZ: screen
// coordinates), tracked until the button goes up.
void TrackScrollBar(HWND hwnd, int bar, POINT pt);

}  // namespace w32

#endif
