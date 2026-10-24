/*
 * style.cpp:  CSS making GTK's controls fit Windows dialog layouts, which place
 *   controls at fixed sizes in dialog units: no minimum sizes or extra padding.
 */

#include "gtk/gtk_backend.h"

namespace w32::native {

void InitStyle()
{
   GtkCssProvider *css = gtk_css_provider_new();
   gtk_css_provider_load_from_string(css,
      ".m59-control { min-width: 0; min-height: 0; margin: 0; }"
      "button.m59-control { padding: 0 4px; }"
      "entry.m59-control, spinbutton.m59-control { padding: 0 3px; }"
      "checkbutton.m59-control { padding: 0; }"
      "checkbutton.m59-control check, checkbutton.m59-control radio { margin: 0 4px 0 0; min-width: 13px; min-height: 13px; }"
      "dropdown.m59-control > button { padding: 0 4px; }"
      "scrollbar.m59-control slider { min-width: 8px; min-height: 8px; }");
   gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   g_object_unref(css);
}

}  // namespace w32::native
