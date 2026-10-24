/*
 * dialogs.cpp:  The common dialogs as GTK 4's color and font choosers, and the
 *   dark theme preference.
 */

#include "gtk/gtk_backend.h"

#include <algorithm>
#include <cstring>

namespace w32::native {

namespace {

struct ColorRequest
{
   void (*done)(bool, COLORREF, void *);
   void *data;
};

void OnColor(GObject *source, GAsyncResult *result, gpointer user)
{
   auto *r = static_cast<ColorRequest *>(user);
   GdkRGBA *rgba = gtk_color_dialog_choose_rgba_finish(GTK_COLOR_DIALOG(source), result, nullptr);
   COLORREF c = 0;
   if (rgba)
   {
      auto channel = [](float v) { return (BYTE) std::clamp((int) (v * 255.0f + 0.5f), 0, 255); };
      c = RGB(channel(rgba->red), channel(rgba->green), channel(rgba->blue));
      gdk_rgba_free(rgba);
   }
   r->done(rgba != nullptr, c, r->data);
   g_object_unref(source);
   delete r;
}

struct FontRequest
{
   void (*done)(bool, const LOGFONTA &, void *);
   void *data;
   LOGFONTA initial;
};

void OnFont(GObject *source, GAsyncResult *result, gpointer user)
{
   auto *r = static_cast<FontRequest *>(user);
   PangoFontDescription *desc = gtk_font_dialog_choose_font_finish(GTK_FONT_DIALOG(source), result, nullptr);
   LOGFONTA lf = r->initial;
   if (desc)
   {
      const char *family = pango_font_description_get_family(desc);
      std::string ansi = Utf8ToAnsi(family ? family : "");
      memset(lf.lfFaceName, 0, sizeof(lf.lfFaceName));
      strncpy(lf.lfFaceName, ansi.c_str(), LF_FACESIZE - 1);
      // Character height in pixels, from points at 96 DPI unless absolute.
      double size = pango_font_description_get_size(desc) / (double) PANGO_SCALE;
      if (!pango_font_description_get_size_is_absolute(desc))
         size = size * 96.0 / 72.0;
      if (size > 0)
         lf.lfHeight = -(LONG) (size + 0.5);
      lf.lfWidth = 0;
      lf.lfWeight = pango_font_description_get_weight(desc);
      PangoStyle style = pango_font_description_get_style(desc);
      lf.lfItalic = style == PANGO_STYLE_ITALIC || style == PANGO_STYLE_OBLIQUE;
      pango_font_description_free(desc);
   }
   r->done(desc != nullptr, lf, r->data);
   g_object_unref(source);
   delete r;
}

GtkWindow *Parent(Top *owner)
{
   return owner ? GTK_WINDOW(owner->window) : nullptr;
}

}  // namespace

void ChooseColor(Top *owner, COLORREF initial, void (*done)(bool, COLORREF, void *), void *data)
{
   Init();
   GtkColorDialog *dialog = gtk_color_dialog_new();
   gtk_color_dialog_set_with_alpha(dialog, FALSE);
   gtk_color_dialog_set_modal(dialog, TRUE);
   GdkRGBA rgba = {GetRValue(initial) / 255.0f, GetGValue(initial) / 255.0f, GetBValue(initial) / 255.0f, 1.0f};
   gtk_color_dialog_choose_rgba(dialog, Parent(owner), &rgba, nullptr, OnColor, new ColorRequest{done, data});
}

void ChooseFont(Top *owner, const LOGFONTA &initial, void (*done)(bool, const LOGFONTA &, void *), void *data)
{
   Init();
   GtkFontDialog *dialog = gtk_font_dialog_new();
   gtk_font_dialog_set_modal(dialog, TRUE);
   PangoFontDescription *desc = pango_font_description_new();
   pango_font_description_set_family(desc, AnsiToUtf8(initial.lfFaceName).c_str());
   if (initial.lfHeight)
      pango_font_description_set_absolute_size(desc, abs(initial.lfHeight) * PANGO_SCALE);
   pango_font_description_set_weight(desc, (PangoWeight) (initial.lfWeight ? initial.lfWeight : FW_NORMAL));
   pango_font_description_set_style(desc, initial.lfItalic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
   gtk_font_dialog_choose_font(dialog, Parent(owner), desc, nullptr, OnFont, new FontRequest{done, data, initial});
   pango_font_description_free(desc);
}

void SetDarkTheme(bool dark)
{
   Init();
   g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", dark ? TRUE : FALSE, nullptr);
}

}  // namespace w32::native
