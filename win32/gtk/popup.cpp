/*
 * popup.cpp:  Popup surfaces: top-level windows that show over their owner and
 *   never take activation or the keyboard (drop-down lists, tooltips).  On
 *   Wayland a top-level window can't be placed, but a popup surface can, relative
 *   to its parent's: a GtkPopover on the owner's drawing area, without arrow,
 *   padding or grab, pointing at the window's rectangle in the owner's client
 *   coordinates.  Its contents are the same overlay and drawing area as a
 *   GtkWindow's, so drawing and input work as for any top-level window (the core
 *   gives the mouse to whatever window has the capture, across surfaces).
 */

#include "gtk/gtk_backend.h"

namespace w32::native {

namespace {

void InitPopupStyle()
{
   static bool done = false;
   if (done)
      return;
   done = true;
   GtkCssProvider *css = gtk_css_provider_new();
   gtk_css_provider_load_from_string(css,
      "popover.m59-popup { padding: 0; margin: 0; background: none; box-shadow: none; }"
      "popover.m59-popup > contents { padding: 0; margin: 0; border: none; border-radius: 0; box-shadow: none;"
      " background: none; min-width: 0; min-height: 0; }"
      "popover.m59-popup > arrow { min-width: 0; min-height: 0; }");
   gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
   g_object_unref(css);
}

}  // namespace

void CreatePopup(Top *top, Top *owner)
{
   InitPopupStyle();
   top->owner = owner;
   top->window = gtk_popover_new();
   GtkPopover *p = GTK_POPOVER(top->window);
   gtk_widget_add_css_class(top->window, "m59-popup");
   gtk_popover_set_has_arrow(p, FALSE);
   // No grab: the owner keeps the keyboard and the pointer outside, as Windows'
   // drop-down lists (the core's capture decides who gets the mouse).
   gtk_popover_set_autohide(p, FALSE);
   gtk_popover_set_position(p, GTK_POS_BOTTOM);
   gtk_widget_set_focusable(top->window, FALSE);
   gtk_widget_set_can_focus(top->window, FALSE);
   gtk_popover_set_child(p, top->overlay);
   gtk_widget_set_parent(top->window, owner->area);
}

void PlacePopup(Top *top)
{
   if (top->owner == nullptr)
      return;
   // Below a one-pixel-high rectangle as wide as the popup, along its top edge:
   // GTK centers the popup horizontally on it, so their left edges meet.
   GdkRectangle r = {top->x - top->owner->x, top->y - top->owner->y - 1, std::max(top->width, 1), 1};
   gtk_popover_set_pointing_to(GTK_POPOVER(top->window), &r);
   gtk_widget_set_size_request(top->area, std::max(top->width, 1), std::max(top->height, 1));
   if (gtk_widget_get_visible(top->window))
      gtk_popover_present(GTK_POPOVER(top->window));
}

void ShowPopup(Top *top, bool show)
{
   if (show)
   {
      PlacePopup(top);
      gtk_popover_popup(GTK_POPOVER(top->window));
   }
   else
      gtk_popover_popdown(GTK_POPOVER(top->window));
}

void DestroyPopup(Top *top)
{
   gtk_widget_unparent(top->window);
}

}  // namespace w32::native
