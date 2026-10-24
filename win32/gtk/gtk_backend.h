/*
 * gtk_backend.h:  What the GTK 4 backend's files share.
 */

#ifndef W32_GTK_BACKEND_H
#define W32_GTK_BACKEND_H

#include "core/gdi.h"
#include "core/native.h"

#include <gtk/gtk.h>

#include <string>
#include <vector>

namespace w32::native {

class GLView;

class Top
{
public:
   HWND hwnd = nullptr;
   GtkWidget *window = nullptr;
   GtkWidget *overlay = nullptr;   // the drawing area, with the native controls over it
   GtkWidget *area = nullptr;      // takes input, sizes the window
   GtkWidget *picture = nullptr;   // shows pixels, over the area
   GdkTexture *texture = nullptr;  // what picture shows
   cairo_region_t *damage = nullptr;   // presented since texture was made
   guint tick = 0;                 // the frame callback that makes the next texture
   std::vector<uint8_t> pixels;    // the surface: BGRA, opaque
   int width = 0, height = 0;
   int x = 0, y = 0;               // client area position, our screen coordinates
   std::string cursor_name = "default";
   bool cursor_hidden = false;
   bool resizable = false;
   GLView *gl = nullptr;           // Direct3D frames over the GDI surface (gtk/glsurface.cpp)
   GList *icons = nullptr;         // GdkTextures: the window's icon, every size
   Top *owner = nullptr;           // popups (gtk/popup.cpp): the top-level whose surface they are placed on
   bool popup = false;             // window is a GtkPopover, not a GtkWindow
};

// Popup surfaces (TopStyle::owned_popup), gtk/popup.cpp: the popover is
// window; position and size follow top->x/y/width/height.
void CreatePopup(Top *top, Top *owner);
void PlacePopup(Top *top);
void ShowPopup(Top *top, bool show);
void DestroyPopup(Top *top);

// Frees top's GL view (DestroyTop).
void DestroyGLView(Top *top);
// GDI drew r (surface coordinates): what it covers of the GL view shows GDI's,
// as on Windows, where the last to draw a pixel wins; the view hides until the
// next frame (a game drawing its map with GDI between Direct3D frames).
void GLViewDrawnOver(Top *top, const cairo_rectangle_int_t &r);

// A VK_* code for a GDK key value, or 0.
int KeyvalToVk(guint keyval);

// True while a native control of top has the keyboard focus.
bool ControlHasFocus(Top *top);

// CSS for the native controls (Windows-like size and spacing).
void InitStyle();

}  // namespace w32::native

#endif
