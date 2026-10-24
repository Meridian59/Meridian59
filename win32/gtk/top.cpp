/*
 * top.cpp:  Top-level windows on GTK 4.  A GtkWindow holds a GtkOverlay: its child
 *   a GtkDrawingArea that sizes the window and takes its input, over it a
 *   GtkPicture showing the window's GDI surface as a texture (transparent to
 *   input), and over that what renders with OpenGL (gtk/glsurface.cpp).
 */

#include "gtk/gtk_backend.h"

#include "core/cursor.h"
#include "core/native_events.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <mutex>

namespace w32::native {

namespace {

std::once_flag init_once;
POINT cursor_pos = {0, 0};
bool keys_down[256];
bool keys_toggled[256];

UINT KeysFromState(GdkModifierType state)
{
   UINT keys = 0;
   if (state & GDK_BUTTON1_MASK) keys |= MK_LBUTTON;
   if (state & GDK_BUTTON2_MASK) keys |= MK_MBUTTON;
   if (state & GDK_BUTTON3_MASK) keys |= MK_RBUTTON;
   if (state & GDK_SHIFT_MASK) keys |= MK_SHIFT;
   if (state & GDK_CONTROL_MASK) keys |= MK_CONTROL;
   return keys;
}

// A new texture of the surface, made once a frame from what was presented: GSK
// uploads only the update region of a texture made as an update of the last.
gboolean OnTick(GtkWidget *, GdkFrameClock *, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   top->tick = 0;
   if (top->pixels.empty())
      return G_SOURCE_REMOVE;
   GBytes *bytes = g_bytes_new(top->pixels.data(), top->pixels.size());
   GdkMemoryTextureBuilder *builder = gdk_memory_texture_builder_new();
   gdk_memory_texture_builder_set_bytes(builder, bytes);
   gdk_memory_texture_builder_set_width(builder, top->width);
   gdk_memory_texture_builder_set_height(builder, top->height);
   gdk_memory_texture_builder_set_stride(builder, (gsize) top->width * 4);
   gdk_memory_texture_builder_set_format(builder, GDK_MEMORY_B8G8R8X8);
   if (top->texture && gdk_texture_get_width(top->texture) == top->width &&
       gdk_texture_get_height(top->texture) == top->height && top->damage)
   {
      gdk_memory_texture_builder_set_update_texture(builder, top->texture);
      gdk_memory_texture_builder_set_update_region(builder, top->damage);
   }
   GdkTexture *texture = gdk_memory_texture_builder_build(builder);
   g_object_unref(builder);
   g_bytes_unref(bytes);
   gtk_picture_set_paintable(GTK_PICTURE(top->picture), GDK_PAINTABLE(texture));
   if (top->texture)
      g_object_unref(top->texture);
   top->texture = texture;
   if (top->damage)
   {
      cairo_region_destroy(top->damage);
      top->damage = nullptr;
   }
   return G_SOURCE_REMOVE;
}

void Damage(Top *top, const cairo_rectangle_int_t &r)
{
   if (top->damage == nullptr)
      top->damage = cairo_region_create();
   cairo_region_union_rectangle(top->damage, &r);
   if (top->tick == 0)
      top->tick = gtk_widget_add_tick_callback(top->picture, OnTick, top, nullptr);
}

void Resize(Top *top, int width, int height)
{
   width = std::max(width, 1);
   height = std::max(height, 1);
   if (width == top->width && height == top->height)
      return;
   // Keep what was drawn, as the client may only repaint what's new.
   std::vector<uint8_t> next((size_t) width * height * 4, 0);
   for (int y = 0; y < std::min(height, top->height); y++)
      memcpy(&next[(size_t) y * width * 4], &top->pixels[(size_t) y * top->width * 4],
             (size_t) std::min(width, top->width) * 4);
   for (size_t i = 3; i < next.size(); i += 4)
      next[i] = 0xFF;
   top->pixels.swap(next);
   top->width = width;
   top->height = height;
   if (top->picture)
      Damage(top, {0, 0, width, height});
}

void OnResize(GtkDrawingArea *, int width, int height, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   Resize(top, width, height);
   events::Resized(top->hwnd, top->width, top->height);
}

void OnMotion(GtkEventControllerMotion *c, double x, double y, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   cursor_pos = {top->x + (int) x, top->y + (int) y};
   GdkModifierType state = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(c));
   events::Mouse(top->hwnd, WM_MOUSEMOVE, (int) x, (int) y, KeysFromState(state));
}

void OnLeave(GtkEventControllerMotion *, gpointer data)
{
   events::MouseLeave(static_cast<Top *>(data)->hwnd);
}

UINT ButtonMessage(guint button, bool down)
{
   switch (button)
   {
   case GDK_BUTTON_PRIMARY: return down ? WM_LBUTTONDOWN : WM_LBUTTONUP;
   case GDK_BUTTON_SECONDARY: return down ? WM_RBUTTONDOWN : WM_RBUTTONUP;
   case GDK_BUTTON_MIDDLE: return down ? WM_MBUTTONDOWN : WM_MBUTTONUP;
   }
   return 0;
}

int ButtonVk(guint button)
{
   return button == GDK_BUTTON_PRIMARY ? VK_LBUTTON : button == GDK_BUTTON_SECONDARY ? VK_RBUTTON : VK_MBUTTON;
}

void OnPress(GtkGestureClick *g, int, double x, double y, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(g));
   GdkModifierType state = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(g));
   keys_down[ButtonVk(button)] = true;
   // The pressed button is down from now on, though the state predates the press.
   UINT keys = KeysFromState(state) | (button == GDK_BUTTON_PRIMARY ? MK_LBUTTON :
                                       button == GDK_BUTTON_SECONDARY ? MK_RBUTTON : MK_MBUTTON);
   gtk_widget_grab_focus(top->area);
   if (UINT msg = ButtonMessage(button, true))
      events::Mouse(top->hwnd, msg, (int) x, (int) y, keys);
}

void OnRelease(GtkGestureClick *g, int, double x, double y, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(g));
   GdkModifierType state = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(g));
   keys_down[ButtonVk(button)] = false;
   UINT keys = KeysFromState(state) & ~(button == GDK_BUTTON_PRIMARY ? MK_LBUTTON :
                                        button == GDK_BUTTON_SECONDARY ? MK_RBUTTON : MK_MBUTTON);
   if (UINT msg = ButtonMessage(button, false))
      events::Mouse(top->hwnd, msg, (int) x, (int) y, keys);
}

gboolean OnScroll(GtkEventControllerScroll *c, double, double dy, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   GdkModifierType state = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(c));
   if (dy != 0)
      events::Mouse(top->hwnd, WM_MOUSEWHEEL, cursor_pos.x - top->x, cursor_pos.y - top->y, KeysFromState(state),
                    (int) (-dy * WHEEL_DELTA));
   return TRUE;
}

gboolean OnKey(Top *top, guint keyval, guint keycode, GdkModifierType state, bool down)
{
   int vk = KeyvalToVk(keyval);
   if (vk == 0)
      return FALSE;
   bool repeat = down && keys_down[vk];
   keys_down[vk] = down;
   // Generic VK_SHIFT/CONTROL/MENU follow either side.
   if (vk == VK_LSHIFT || vk == VK_RSHIFT)
      keys_down[VK_SHIFT] = down;
   if (vk == VK_LCONTROL || vk == VK_RCONTROL)
      keys_down[VK_CONTROL] = down;
   if (vk == VK_LMENU || vk == VK_RMENU)
      keys_down[VK_MENU] = down;
   if (down && (vk == VK_CAPITAL || vk == VK_NUMLOCK || vk == VK_SCROLL))
      keys_toggled[vk] = !keys_toggled[vk];
   // Windows reports the generic key code for these.
   int report = (vk == VK_LSHIFT || vk == VK_RSHIFT) ? VK_SHIFT : (vk == VK_LCONTROL || vk == VK_RCONTROL) ? VK_CONTROL
                : (vk == VK_LMENU || vk == VK_RMENU) ? VK_MENU : vk;
   char text[8] = "";
   gunichar ch = gdk_keyval_to_unicode(keyval);
   if (down && ch && !(state & (GDK_CONTROL_MASK | GDK_ALT_MASK)))
      text[g_unichar_to_utf8(ch, text)] = 0;
   else if (down && ch && (state & GDK_CONTROL_MASK) && ch >= 'a' && ch <= 'z')
   {
      // Ctrl+letter types the control character, as on Windows.
      text[0] = (char) (ch - 'a' + 1);
      text[1] = 0;
   }
   bool alt = (state & GDK_ALT_MASK) != 0 && report != VK_MENU;
   events::Key(top->hwnd, down, report, (int) keycode - 8, repeat, alt || (report == VK_MENU && down), text);
   return TRUE;
}

gboolean OnKeyPressed(GtkEventControllerKey *, guint keyval, guint keycode, GdkModifierType state, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   // Keys for native controls go to them (gtk/controls.cpp routes those).
   if (ControlHasFocus(top))
      return FALSE;
   return OnKey(top, keyval, keycode, state, true);
}

void OnKeyReleased(GtkEventControllerKey *, guint keyval, guint keycode, GdkModifierType state, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   if (!ControlHasFocus(top))
      OnKey(top, keyval, keycode, state, false);
}

gboolean OnCloseRequest(GtkWindow *, gpointer data)
{
   events::CloseRequested(static_cast<Top *>(data)->hwnd);
   return TRUE;   // the core destroys the window (DestroyWindow), or not
}

void OnActive(GObject *, GParamSpec *, gpointer data)
{
   Top *top = static_cast<Top *>(data);
   bool active = gtk_window_is_active(GTK_WINDOW(top->window));
   events::Activated(top->hwnd, active);
   events::Focus(top->hwnd, active);
}

}  // namespace

std::string AppId();

void Init()
{
   std::call_once(init_once, [] {
      // Direct3D renders with desktop OpenGL in contexts shared with GDK's, so GDK
      // must use desktop OpenGL too, not the OpenGL ES it prefers.
      std::string disable = getenv("GDK_DISABLE") ? getenv("GDK_DISABLE") : "";
      if (disable.find("gles-api") == std::string::npos)
         setenv("GDK_DISABLE", (disable.empty() ? "gles-api" : disable + ",gles-api").c_str(), 1);
      // The app id GTK gives the compositor (prgname, without a GApplication).
      g_set_prgname(AppId().c_str());
      gtk_init();
      // Windows' dialogs and controls are small and plain: the default theme's
      // padding makes the client's layouts overflow.
      InitStyle();
   });
}

Top *CreateTop(HWND hwnd, const TopStyle &style, int x, int y, int width, int height, Top *owner)
{
   Init();
   auto *top = new Top;
   top->hwnd = hwnd;
   top->x = x;
   top->y = y;
   top->overlay = gtk_overlay_new();
   if (style.owned_popup && owner)
   {
      top->popup = true;
      CreatePopup(top, owner);
   }
   else
   {
      top->window = gtk_window_new();
      gtk_window_set_resizable(GTK_WINDOW(top->window), style.resizable);
      gtk_window_set_decorated(GTK_WINDOW(top->window), style.caption && !style.popup_menu_like);
      if (owner)
         gtk_window_set_transient_for(GTK_WINDOW(top->window), GTK_WINDOW(owner->window));
      gtk_window_set_default_size(GTK_WINDOW(top->window), std::max(width, 1), std::max(height, 1));
      gtk_window_set_child(GTK_WINDOW(top->window), top->overlay);
   }
   top->area = gtk_drawing_area_new();
   gtk_widget_set_focusable(top->area, TRUE);
   gtk_widget_set_hexpand(top->area, TRUE);
   gtk_widget_set_vexpand(top->area, TRUE);
   // A fixed-size window is its content's size; a resizable one starts at its
   // default size and can shrink.
   gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(top->area), style.resizable ? 1 : std::max(width, 1));
   gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(top->area), style.resizable ? 1 : std::max(height, 1));
   top->resizable = style.resizable && !top->popup;
   gtk_overlay_set_child(GTK_OVERLAY(top->overlay), top->area);
   top->picture = gtk_picture_new();
   gtk_picture_set_content_fit(GTK_PICTURE(top->picture), GTK_CONTENT_FIT_FILL);
   gtk_picture_set_can_shrink(GTK_PICTURE(top->picture), TRUE);
   gtk_widget_set_can_target(top->picture, FALSE);
   gtk_overlay_add_overlay(GTK_OVERLAY(top->overlay), top->picture);
   g_signal_connect(top->area, "resize", G_CALLBACK(OnResize), top);
   Resize(top, width, height);

   GtkEventController *motion = gtk_event_controller_motion_new();
   g_signal_connect(motion, "motion", G_CALLBACK(OnMotion), top);
   g_signal_connect(motion, "leave", G_CALLBACK(OnLeave), top);
   gtk_widget_add_controller(top->area, motion);

   GtkGesture *click = gtk_gesture_click_new();
   gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 0);
   g_signal_connect(click, "pressed", G_CALLBACK(OnPress), top);
   g_signal_connect(click, "released", G_CALLBACK(OnRelease), top);
   gtk_widget_add_controller(top->area, GTK_EVENT_CONTROLLER(click));

   GtkEventController *scroll = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
   g_signal_connect(scroll, "scroll", G_CALLBACK(OnScroll), top);
   gtk_widget_add_controller(top->area, scroll);

   // Keys at the window, before any widget: the core routes them (focus window).
   GtkEventController *keys = gtk_event_controller_key_new();
   gtk_event_controller_set_propagation_phase(keys, GTK_PHASE_CAPTURE);
   g_signal_connect(keys, "key-pressed", G_CALLBACK(OnKeyPressed), top);
   g_signal_connect(keys, "key-released", G_CALLBACK(OnKeyReleased), top);
   gtk_widget_add_controller(top->window, keys);

   if (top->popup)
   {
      PlacePopup(top);
      return top;
   }
   g_signal_connect(top->window, "close-request", G_CALLBACK(OnCloseRequest), top);
   g_signal_connect(top->window, "notify::is-active", G_CALLBACK(OnActive), top);
   return top;
}

void DestroyTop(Top *top)
{
   if (top == nullptr)
      return;
   DestroyGLView(top);
   if (top->tick)
      gtk_widget_remove_tick_callback(top->picture, top->tick);
   if (top->texture)
      g_object_unref(top->texture);
   if (top->damage)
      cairo_region_destroy(top->damage);
   g_list_free_full(top->icons, g_object_unref);
   if (top->popup)
      DestroyPopup(top);
   else
      gtk_window_destroy(GTK_WINDOW(top->window));
   delete top;
}

void SetTitle(Top *top, const std::string &title)
{
   if (top->popup)
      return;
   gtk_window_set_title(GTK_WINDOW(top->window), AnsiToUtf8(title.c_str()).c_str());
}

void SetGeometry(Top *top, int x, int y, int width, int height)
{
   // Wayland places windows itself; the position is kept for our coordinates.
   top->x = x;
   top->y = y;
   if (top->popup)
   {
      gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(top->area), std::max(width, 1));
      gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(top->area), std::max(height, 1));
      Resize(top, width, height);
      PlacePopup(top);
      return;
   }
   if (!top->resizable)
   {
      gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(top->area), std::max(width, 1));
      gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(top->area), std::max(height, 1));
   }
   gtk_window_set_default_size(GTK_WINDOW(top->window), std::max(width, 1), std::max(height, 1));
   Resize(top, width, height);
}

void Show(Top *top, bool show)
{
   if (top->popup)
      return ShowPopup(top, show);
   gtk_widget_set_visible(top->window, show);
}

void SetMinimized(Top *top, bool minimized)
{
   if (top->popup)
      return;
   if (minimized)
      gtk_window_minimize(GTK_WINDOW(top->window));
   else
      gtk_window_unminimize(GTK_WINDOW(top->window));
}

void SetMaximized(Top *top, bool maximized)
{
   if (top->popup)
      return;
   if (maximized)
      gtk_window_maximize(GTK_WINDOW(top->window));
   else
      gtk_window_unmaximize(GTK_WINDOW(top->window));
}

void SetModal(Top *top, bool modal)
{
   if (top->popup)
      return;
   gtk_window_set_modal(GTK_WINDOW(top->window), modal);
}

void Activate(Top *top)
{
   // A popup is never active itself; its owner is.
   if (top->popup)
      return Activate(top->owner);
   gtk_window_present(GTK_WINDOW(top->window));
}

bool IsActive(Top *top)
{
   if (top && top->popup)
      return IsActive(top->owner);
   return top && gtk_window_is_active(GTK_WINDOW(top->window));
}

// The icon as textures, set on the window's surface: GDK hands them to the
// compositor (xdg-toplevel-icon), so no installed theme icon is needed.
void ApplyIcon(Top *top)
{
   if (top->icons == nullptr || top->popup)
      return;
   if (GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(top->window)); surface && GDK_IS_TOPLEVEL(surface))
      gdk_toplevel_set_icon_list(GDK_TOPLEVEL(surface), top->icons);
}

void OnRealizeIcon(GtkWidget *, gpointer data)
{
   ApplyIcon(static_cast<Top *>(data));
}

// The application's id: the Wayland app id GTK sends, and the desktop entry's
// name the compositor looks the icon up by.
std::string AppId()
{
   static std::string id = [] {
      char exe[4096];
      ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
      std::string base = n > 0 ? std::string(exe, n) : "meridian";
      base = base.substr(base.find_last_of('/') + 1);
      return "org.meridian59." + base;
   }();
   return id;
}

// On Wayland a window's icon is its application's, from a desktop entry: as Wine
// does for Windows programs, the icon goes into the user's hicolor theme and a
// hidden entry names it (written once per change).
void PublishAppIcon(GList *textures)
{
   static bool done = false;
   if (done)
      return;
   done = true;
   const char *data = g_get_user_data_dir();
   std::string id = AppId();
   for (GList *l = textures; l; l = l->next)
   {
      GdkTexture *t = GDK_TEXTURE(l->data);
      int w = gdk_texture_get_width(t), h = gdk_texture_get_height(t);
      std::string dir = std::string(data) + "/icons/hicolor/" + std::to_string(w) + "x" + std::to_string(h) + "/apps";
      g_mkdir_with_parents(dir.c_str(), 0755);
      GBytes *png = gdk_texture_save_to_png_bytes(t);
      std::string path = dir + "/" + id + ".png";
      gchar *old = nullptr;
      gsize old_size = 0;
      gsize size;
      const void *bytes = g_bytes_get_data(png, &size);
      if (!g_file_get_contents(path.c_str(), &old, &old_size, nullptr) || old_size != size || memcmp(old, bytes, size))
         g_file_set_contents(path.c_str(), (const gchar *) bytes, (gssize) size, nullptr);
      g_free(old);
      g_bytes_unref(png);
   }
   char exe[4096];
   ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
   std::string entry = "[Desktop Entry]\nType=Application\nName=Meridian 59\nExec=" +
                       (n > 0 ? std::string(exe, n) : std::string("meridian")) + "\nIcon=" + id +
                       "\nStartupWMClass=" + id + "\nNoDisplay=true\n";
   std::string dir = std::string(data) + "/applications";
   g_mkdir_with_parents(dir.c_str(), 0755);
   std::string path = dir + "/" + id + ".desktop";
   gchar *old = nullptr;
   if (!g_file_get_contents(path.c_str(), &old, nullptr, nullptr) || entry != old)
      g_file_set_contents(path.c_str(), entry.c_str(), (gssize) entry.size(), nullptr);
   g_free(old);
}

void SetIcon(Top *top, HICON icon)
{
   const CursorImage *c = GetCursorImage(icon);
   if (c == nullptr || c->pixels.empty())
      return;
   g_list_free_full(top->icons, g_object_unref);
   top->icons = nullptr;
   std::vector<const CursorImage *> images;
   for (const CursorImage &s : c->sizes)
      images.push_back(&s);
   if (images.empty())
      images.push_back(c);
   for (const CursorImage *img : images)
   {
      // Straight-alpha ARGB to GDK's premultiplied RGBA.
      std::vector<uint8_t> rgba((size_t) img->width * img->height * 4);
      for (size_t i = 0; i < img->pixels.size(); i++)
      {
         uint32_t p = img->pixels[i], a = p >> 24;
         rgba[i * 4] = (uint8_t) (((p >> 16) & 0xFF) * a / 255);
         rgba[i * 4 + 1] = (uint8_t) (((p >> 8) & 0xFF) * a / 255);
         rgba[i * 4 + 2] = (uint8_t) ((p & 0xFF) * a / 255);
         rgba[i * 4 + 3] = (uint8_t) a;
      }
      GBytes *bytes = g_bytes_new(rgba.data(), rgba.size());
      GdkTexture *t = gdk_memory_texture_new(img->width, img->height, GDK_MEMORY_R8G8B8A8_PREMULTIPLIED, bytes,
                                             (gsize) img->width * 4);
      g_bytes_unref(bytes);
      top->icons = g_list_append(top->icons, t);
   }
   PublishAppIcon(top->icons);
   if (gtk_widget_get_realized(top->window))
      ApplyIcon(top);
   else
      g_signal_connect(top->window, "realize", G_CALLBACK(OnRealizeIcon), top);
}

Surface GetSurface(Top *top)
{
   Surface s;
   s.pixels = top->pixels.data();
   s.width = top->width;
   s.height = top->height;
   s.stride = top->width * 4;
   return s;
}

void Present(Top *top, const RECT &r)
{
   int x = std::max(0, (int) r.left), y = std::max(0, (int) r.top);
   int right = std::min(top->width, (int) r.right), bottom = std::min(top->height, (int) r.bottom);
   if (right > x && bottom > y)
   {
      Damage(top, {x, y, right - x, bottom - y});
      GLViewDrawnOver(top, {x, y, right - x, bottom - y});
   }
}

POINT GetCursorPos()
{
   return cursor_pos;
}

bool SetCursorPos(int x, int y)
{
   // Wayland can't warp the pointer; keep the position the client expects.
   cursor_pos = {x, y};
   return false;
}

void ShowCursor(Top *top, bool show)
{
   top->cursor_hidden = !show;
   gtk_widget_set_cursor_from_name(top->area, show ? top->cursor_name.c_str() : "none");
}

void CaptureMouse(Top *, bool)
{
   // GTK grabs the pointer for a widget while a button is held, which is when the
   // client captures it.
}

bool KeyDown(int vk)
{
   return vk >= 0 && vk < 256 && keys_down[vk];
}

bool KeyToggled(int vk)
{
   return vk >= 0 && vk < 256 && keys_toggled[vk];
}

RECT ScreenRect()
{
   Init();
   GdkDisplay *display = gdk_display_get_default();
   GListModel *monitors = display ? gdk_display_get_monitors(display) : nullptr;
   if (monitors && g_list_model_get_n_items(monitors) > 0)
   {
      GdkMonitor *m = GDK_MONITOR(g_list_model_get_item(monitors, 0));
      GdkRectangle r;
      gdk_monitor_get_geometry(m, &r);
      g_object_unref(m);
      return {r.x, r.y, r.x + r.width, r.y + r.height};
   }
   return {0, 0, 1920, 1080};
}

RECT WorkArea()
{
   return ScreenRect();
}

int ScreenDpi()
{
   return 96;
}

void Beep()
{
   if (GdkDisplay *display = gdk_display_get_default())
      gdk_display_beep(display);
}

}  // namespace w32::native

#include "core/cursor.h"

namespace w32::native {

void SetCursor(Top *top, HCURSOR cursor)
{
   const CursorImage *c = GetCursorImage(cursor);
   if (c == nullptr)
      return;
   GdkCursor *gc = nullptr;
   if (!c->standard.empty())
   {
      top->cursor_name = c->standard;
      if (!top->cursor_hidden)
         gtk_widget_set_cursor_from_name(top->area, c->standard.c_str());
      return;
   }
   // An image: GDK takes premultiplied RGBA.
   std::vector<uint8_t> rgba((size_t) c->width * c->height * 4);
   for (size_t i = 0; i < c->pixels.size(); i++)
   {
      uint32_t p = c->pixels[i];
      uint32_t a = p >> 24;
      rgba[i * 4] = (uint8_t) (((p >> 16) & 0xFF) * a / 255);
      rgba[i * 4 + 1] = (uint8_t) (((p >> 8) & 0xFF) * a / 255);
      rgba[i * 4 + 2] = (uint8_t) ((p & 0xFF) * a / 255);
      rgba[i * 4 + 3] = (uint8_t) a;
   }
   GBytes *bytes = g_bytes_new(rgba.data(), rgba.size());
   GdkTexture *texture = gdk_memory_texture_new(c->width, c->height, GDK_MEMORY_R8G8B8A8_PREMULTIPLIED, bytes,
                                                (gsize) c->width * 4);
   g_bytes_unref(bytes);
   gc = gdk_cursor_new_from_texture(texture, c->hot_x, c->hot_y, nullptr);
   g_object_unref(texture);
   top->cursor_name = "default";
   if (!top->cursor_hidden)
      gtk_widget_set_cursor(top->area, gc);
   g_object_unref(gc);
}

}  // namespace w32::native
