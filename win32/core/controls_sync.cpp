/*
 * controls_sync.cpp:  Keeps native controls in step with their windows' focus,
 *   visibility and enabled state.
 */

#include "core/message.h"

namespace w32 {

void FocusChanged(HWND) {}
void ChildVisibilityChanged(HWND) {}
void ChildEnabledChanged(HWND) {}

}  // namespace w32
