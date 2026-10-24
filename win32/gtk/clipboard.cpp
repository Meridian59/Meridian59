/*
 * clipboard.cpp:  The core's clipboard on the display's: what the program copies
 *   becomes the display clipboard's text.  Pasting text other programs copied
 *   needs an asynchronous read GDK only offers inside its event loop, which a
 *   Win32 GetClipboardData can't wait in; that direction isn't linked yet.
 */

#include "gtk/gtk_backend.h"

#include "core/clipboard.h"

namespace w32::native {

namespace {

void Publish(const std::string &utf8)
{
   if (GdkDisplay *display = gdk_display_get_default())
      gdk_clipboard_set_text(gdk_display_get_clipboard(display), utf8.c_str());
}

[[maybe_unused]] const bool linked = [] {
   ClipboardBackend backend;
   backend.publish = Publish;
   SetClipboardBackend(backend);
   return true;
}();

}  // namespace

}  // namespace w32::native
