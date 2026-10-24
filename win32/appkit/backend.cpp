/*
 * backend.cpp:  The AppKit backend's C++ half: core/native.h on what the
 *   Objective-C++ half provides (bridge.h), with Windows' types turned into the
 *   bridge's, and AppKit's events into the core's.
 */

#include "core/cursor.h"
#include "core/fontmap.h"
#include "core/gdi.h"
#include "core/glsurface.h"
#include "core/menu.h"
#include "core/native.h"
#include "core/native_events.h"
#include "core/rasterfont.h"
#include "core/window.h"
#include "gfx/gfx.h"

#include "appkit/bridge.h"

#include <cstring>
#include <strings.h>
#include <map>

namespace w32::native {

namespace {

mac::Rect ToMac(const RECT &r)
{
   return {(int) r.left, (int) r.top, (int) r.right, (int) r.bottom};
}

RECT FromMac(const mac::Rect &r)
{
   return {r.left, r.top, r.right, r.bottom};
}

mac::FontSpec SpecOf(const LOGFONTA &lf)
{
   mac::FontSpec s;
   s.face = AnsiToUtf8(lf.lfFaceName);
   s.height = lf.lfHeight;
   s.weight = lf.lfWeight ? lf.lfWeight : FW_NORMAL;
   s.italic = lf.lfItalic != 0;
   s.underline = lf.lfUnderline != 0;
   s.strike_out = lf.lfStrikeOut != 0;
   s.fixed_pitch = (lf.lfPitchAndFamily & 0x3) == FIXED_PITCH;
   return s;
}

// "&File" -> "File" (macOS menus show no mnemonics); "&&" -> "&".
std::string Title(const std::string &ansi)
{
   std::string out;
   for (size_t i = 0; i < ansi.size(); i++)
   {
      if (ansi[i] == '&')
      {
         if (i + 1 < ansi.size() && ansi[i + 1] == '&')
            out += '&', i++;
         continue;
      }
      out += ansi[i];
   }
   return AnsiToUtf8(out.c_str());
}

std::vector<mac::MenuItem> Items(const std::vector<MenuEntry> &entries)
{
   std::vector<mac::MenuItem> out;
   for (const MenuEntry &e : entries)
   {
      mac::MenuItem m;
      m.title = Title(e.text);
      m.id = e.id;
      m.separator = e.separator;
      m.checked = e.checked;
      m.grayed = e.grayed;
      m.has_sub = e.has_sub;
      m.sub = Items(e.sub);
      out.push_back(std::move(m));
   }
   return out;
}

}  // namespace

void Init()
{
   mac::Init();
}

Top *CreateTop(HWND hwnd, const TopStyle &style, int x, int y, int width, int height, Top *owner)
{
   mac::TopStyle s;
   s.caption = style.caption;
   s.resizable = style.resizable;
   s.minimize_box = style.minimize_box;
   s.maximize_box = style.maximize_box;
   s.tool = style.tool;
   s.popup_menu_like = style.popup_menu_like;
   return mac::CreateTop(hwnd, s, x, y, width, height, owner);
}

void DestroyGLView(Top *top);

void DestroyTop(Top *top)
{
   if (top == nullptr)
      return;
   DestroyGLView(top);
   mac::DestroyTop(top);
}

void SetTitle(Top *top, const std::string &title)
{
   mac::SetTitle(top, AnsiToUtf8(title.c_str()));
}

void SetGeometry(Top *top, int x, int y, int width, int height)
{
   mac::SetGeometry(top, x, y, width, height);
}

void Show(Top *top, bool show)
{
   mac::Show(top, show);
}

void SetMinimized(Top *top, bool minimized)
{
   mac::SetMinimized(top, minimized);
}

void SetMaximized(Top *top, bool maximized)
{
   mac::SetMaximized(top, maximized);
}

void SetModal(Top *, bool)
{
   // The core disables the owner, which is what makes a dialog modal on Windows.
}

void Activate(Top *top)
{
   mac::Activate(top);
}

bool IsActive(Top *top)
{
   return top && mac::IsActive(top);
}

void SetIcon(Top *, HICON icon)
{
   // macOS shows the application's icon, not windows': the first window's is it,
   // as the main window's is the program's on Windows.
   static bool set = false;
   const CursorImage *c = GetCursorImage(icon);
   if (set || c == nullptr || c->pixels.empty())
      return;
   set = true;
   std::vector<mac::CursorSpec> images;
   auto add = [&](const CursorImage &img) {
      mac::CursorSpec s;
      s.width = img.width;
      s.height = img.height;
      s.pixels = img.pixels.data();
      images.push_back(s);
   };
   for (const CursorImage &img : c->sizes)
      add(img);
   if (images.empty())
      add(*c);
   mac::SetAppIcon(images);
}

Surface GetSurface(Top *top)
{
   mac::Surface m = mac::GetSurface(top);
   Surface s;
   s.pixels = m.pixels;
   s.width = m.width;
   s.height = m.height;
   s.stride = m.stride;
   return s;
}

void Present(Top *top, const RECT &rect)
{
   mac::Present(top, ToMac(rect));
}

POINT GetCursorPos()
{
   mac::Point p = mac::GetCursorPos();
   return {p.x, p.y};
}

bool SetCursorPos(int x, int y)
{
   return mac::SetCursorPos(x, y);
}

void SetCursor(Top *top, HCURSOR cursor)
{
   const CursorImage *c = GetCursorImage(cursor);
   if (c == nullptr)
      return;
   mac::CursorSpec s;
   s.standard = c->standard;
   s.width = c->width;
   s.height = c->height;
   s.hot_x = c->hot_x;
   s.hot_y = c->hot_y;
   s.pixels = c->pixels.data();
   mac::SetCursor(top, s);
}

void ShowCursor(Top *top, bool show)
{
   mac::ShowCursor(top, show);
}

void CaptureMouse(Top *, bool)
{
   // AppKit sends drags and the release to the view that got the press.
}

bool KeyDown(int vk)
{
   return mac::KeyDown(vk);
}

bool KeyToggled(int vk)
{
   return mac::KeyToggled(vk);
}

RECT ScreenRect()
{
   return FromMac(mac::ScreenRect());
}

RECT WorkArea()
{
   return FromMac(mac::WorkArea());
}

int ScreenDpi()
{
   return 96;
}

// Fonts drawn from Windows' raster fonts (core/rasterfont.cpp) instead.
static std::map<Font *, const RasterFont *> &RasterFonts()
{
   static std::map<Font *, const RasterFont *> fonts;
   return fonts;
}

static const RasterFont *RasterOf(Font *font)
{
   auto it = RasterFonts().find(font);
   return it == RasterFonts().end() ? nullptr : it->second;
}

bool FaceInstalled(const char *face)
{
   return mac::FaceInstalled(face);
}

Font *CreateFont(const LOGFONTA &lf)
{
   // Windows' face and, for its own faces, its metrics (core/fontmap.h): drawn by
   // the face's stand-in at the em they make.
   mac::FontSpec spec = SpecOf(lf);
   RealizedFont w = RealizeFont(lf);
   if (w.known)
   {
      spec.face = w.draw;
      spec.height = -w.em;
      spec.windows_metrics = true;
      mac::FontMetrics &m = spec.metrics;
      m.height = w.height;
      m.ascent = w.ascent;
      m.descent = w.descent;
      m.internal_leading = w.internal_leading;
      m.external_leading = w.external_leading;
      m.average_width = w.average_width;
      m.max_width = w.max_width;
      m.weight = w.weight;
      m.italic = spec.italic;
      m.underline = spec.underline;
      m.strike_out = spec.strike_out;
      m.face = w.face;
   }
   Font *f = mac::MakeFont(spec);
   if (const RasterFont *r = OpenRasterFont(lf))
      RasterFonts()[f] = r;
   return f;
}

void DestroyFont(Font *font)
{
   RasterFonts().erase(font);
   mac::DestroyFont(font);
}

FontMetrics GetFontMetrics(Font *font)
{
   if (const RasterFont *r = RasterOf(font))
   {
      RasterMetrics m = RasterFontMetrics(r);
      FontMetrics f;
      f.height = m.height;
      f.ascent = m.ascent;
      f.descent = m.descent;
      f.internal_leading = m.internal_leading;
      f.external_leading = m.external_leading;
      f.average_width = m.average_width;
      f.max_width = m.max_width;
      f.weight = m.weight;
      f.italic = m.italic;
      f.face = m.face;
      return f;
   }
   mac::FontMetrics m = mac::GetFontMetrics(font);
   FontMetrics f;
   f.height = m.height;
   f.ascent = m.ascent;
   f.descent = m.descent;
   f.internal_leading = m.internal_leading;
   f.external_leading = m.external_leading;
   f.average_width = m.average_width;
   f.max_width = m.max_width;
   f.weight = m.weight;
   f.italic = m.italic;
   f.underline = m.underline;
   f.strike_out = m.strike_out;
   f.face = Utf8ToAnsi(m.face);
   return f;
}

int TextWidth(Font *font, const std::string &utf8)
{
   if (const RasterFont *r = RasterOf(font))
      return RasterTextWidth(r, Utf8ToAnsi(utf8));
   return mac::TextWidth(font, utf8);
}

void CharWidths(Font *font, uint32_t codepoint, int *a, int *b, int *c)
{
   if (const RasterFont *r = RasterOf(font))
   {
      WCHAR w = codepoint < 0x10000 ? (WCHAR) codepoint : '?';
      char ansi = '?';
      WideCharToMultiByte(CP_ACP, 0, &w, 1, &ansi, 1, nullptr, nullptr);
      RasterCharWidths(r, (unsigned char) ansi, a, b, c);
      return;
   }
   mac::CharWidths(font, codepoint, a, b, c);
}

std::vector<int> CharEdges(Font *font, const std::string &utf8)
{
   if (const RasterFont *r = RasterOf(font))
   {
      // Each code point's trailing edge: the widths so far.
      std::vector<int> edges;
      int x = 0;
      for (size_t i = 0; i < utf8.size();)
      {
         size_t n = 1;
         while (i + n < utf8.size() && ((unsigned char) utf8[i + n] & 0xC0) == 0x80)
            n++;
         std::string ansi = Utf8ToAnsi(utf8.substr(i, n));
         x += RasterTextWidth(r, ansi.empty() ? "?" : ansi);
         edges.push_back(x);
         i += n;
      }
      return edges;
   }
   return mac::CharEdges(font, utf8);
}

int Kerning(Font *font, uint32_t first, uint32_t second)
{
   if (RasterOf(font))
      return 0;
   return mac::Kerning(font, first, second);
}

std::vector<uint8_t> RenderText(Font *font, const std::string &utf8, int *width, int *height)
{
   if (const RasterFont *r = RasterOf(font))
      return RasterRender(r, Utf8ToAnsi(utf8), width, height);
   return mac::RenderText(font, utf8, width, height);
}

void TrackMenu(Top *top, const std::vector<MenuEntry> &items, const RECT &anchor, void (*done)(UINT, void *),
               void *data)
{
   done(mac::TrackMenu(top, Items(items), ToMac(anchor)), data);
}

void ChooseColor(Top *owner, COLORREF initial, void (*done)(bool, COLORREF, void *), void *data)
{
   uint32_t chosen = initial;
   bool ok = mac::PickColor(owner, initial, &chosen);
   done(ok, ok ? (COLORREF) chosen : initial, data);
}

void ChooseFont(Top *owner, const LOGFONTA &initial, void (*done)(bool, const LOGFONTA &, void *), void *data)
{
   mac::FontSpec chosen;
   bool ok = mac::PickFont(owner, SpecOf(initial), &chosen);
   LOGFONTA lf = initial;
   if (ok)
   {
      std::string face = Utf8ToAnsi(chosen.face);
      memset(lf.lfFaceName, 0, sizeof(lf.lfFaceName));
      strncpy(lf.lfFaceName, face.c_str(), LF_FACESIZE - 1);
      lf.lfHeight = chosen.height;
      lf.lfWidth = 0;
      lf.lfWeight = chosen.weight;
      lf.lfItalic = chosen.italic;
   }
   done(ok, lf, data);
}

void Beep()
{
   mac::Beep();
}

void SetDarkTheme(bool dark)
{
   mac::SetDarkTheme(dark);
}

}  // namespace w32::native

// --- Direct3D's surfaces ----------------------------------------------------------

namespace w32 {

namespace {

class ViewSurface : public GLSurface
{
public:
   explicit ViewSurface(HWND hwnd) : hwnd_(hwnd) {}

   bool MakeCurrent() override
   {
      native::Top *top = TopOf();
      return top && mac::GLMakeCurrent(top);
   }

   void Present(unsigned texture, int, int, const RECT &dest) override
   {
      native::Top *top = TopOf();
      if (top == nullptr)
         return;
      // dest is in hwnd's client coordinates; the view sits on the surface.
      RECT c = ClientRectInTop(hwnd_);
      mac::GLPresent(top, texture,
                     {(int) (c.left + dest.left), (int) (c.top + dest.top), (int) (c.left + dest.right),
                      (int) (c.top + dest.bottom)});
   }

   void *GetProcAddress(const char *name) override
   {
      return mac::GLProcAddress(name);
   }

   HWND hwnd() const { return hwnd_; }

private:
   native::Top *TopOf()
   {
      Window *w = GetWindowObject(TopLevelOf(hwnd_));
      return w ? w->top : nullptr;
   }

   HWND hwnd_;
};

std::map<HWND, ViewSurface *> &Surfaces()
{
   static std::map<HWND, ViewSurface *> surfaces;
   return surfaces;
}

}  // namespace

GLSurface *GetGLSurface(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   Window *t = w ? GetWindowObject(TopLevelOf(hwnd)) : nullptr;
   if (t == nullptr || t->top == nullptr)
      return nullptr;
   auto &surfaces = Surfaces();
   auto it = surfaces.find(hwnd);
   if (it != surfaces.end())
      return it->second;
   auto *s = new ViewSurface(hwnd);
   surfaces[hwnd] = s;
   return s;
}

}  // namespace w32

namespace w32::native {

// The window's surfaces go: devices on them are lost.
void DestroyGLView(Top *top)
{
   HWND hwnd = nullptr;
   for (auto &[h, s] : Surfaces())
      if (Window *w = GetWindowObject(TopLevelOf(h)); w && w->top == top)
         hwnd = TopLevelOf(h);
   auto &surfaces = Surfaces();
   for (auto it = surfaces.begin(); it != surfaces.end();)
   {
      if ((hwnd && TopLevelOf(it->second->hwnd()) == hwnd) || !IsWindow(it->second->hwnd()))
      {
         delete it->second;
         it = surfaces.erase(it);
      }
      else
         ++it;
   }
}

}  // namespace w32::native

// --- AppKit's events, to the core -------------------------------------------------

namespace w32::mac {

void OnMouseMove(void *hwnd, int x, int y, unsigned keys)
{
   events::Mouse((HWND) hwnd, WM_MOUSEMOVE, x, y, keys);
}

void OnButton(void *hwnd, Button button, bool down, int x, int y, unsigned keys)
{
   static const UINT downs[] = {WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN};
   static const UINT ups[] = {WM_LBUTTONUP, WM_RBUTTONUP, WM_MBUTTONUP};
   events::Mouse((HWND) hwnd, (down ? downs : ups)[(int) button], x, y, keys);
}

void OnWheel(void *hwnd, int x, int y, unsigned keys, int delta)
{
   events::Mouse((HWND) hwnd, WM_MOUSEWHEEL, x, y, keys, delta);
}

void OnMouseLeave(void *hwnd)
{
   events::MouseLeave((HWND) hwnd);
}

void OnKey(void *hwnd, bool down, int vk, int scan, bool repeat, bool alt, const char *text)
{
   events::Key((HWND) hwnd, down, vk, scan, repeat, alt, text);
}

void OnResized(void *hwnd, int width, int height)
{
   events::Resized((HWND) hwnd, width, height);
}

void OnMoved(void *hwnd, int x, int y)
{
   events::Moved((HWND) hwnd, x, y);
}

void OnActivated(void *hwnd, bool active)
{
   events::Activated((HWND) hwnd, active);
   events::Focus((HWND) hwnd, active);
}

void OnCloseRequested(void *hwnd)
{
   events::CloseRequested((HWND) hwnd);
}

bool TextFieldFocused()
{
   char cls[32] = "";
   HWND focus = GetFocus();
   return focus && GetClassNameA(focus, cls, sizeof(cls)) &&
          (strcasecmp(cls, "Edit") == 0 || strncasecmp(cls, "RichEdit", 8) == 0);
}

void OnTextShortcut(void *hwnd, int vk, int scan)
{
   if (!TextFieldFocused())
      return;
   // Control is down for just this key, as the field reads it (GetKeyState).
   const char text[2] = {(char) (vk - 'A' + 1), 0};
   events::Key((HWND) hwnd, true, VK_CONTROL, 0x1D, false, false, "");
   events::Key((HWND) hwnd, true, vk, scan, false, false, text);
   events::Key((HWND) hwnd, false, vk, scan, false, false, "");
   events::Key((HWND) hwnd, false, VK_CONTROL, 0x1D, false, false, "");
}

void OnQuit()
{
   // As closing the active window's root owner: the game's main window, which
   // saves and exits; not while a modal dialog has it disabled.
   HWND w = GetActiveWindow();
   for (HWND owner; w && (owner = GetWindow(w, GW_OWNER)) != nullptr;)
      w = owner;
   if (w && IsWindowEnabled(w))
      events::CloseRequested(w);
   else
      MessageBeep(MB_OK);
}

void OnMinimized(void *hwnd, bool minimized)
{
   events::Minimized((HWND) hwnd, minimized);
}

void OnGLStateChanged()
{
   gfx::InvalidateGLState();
}

}  // namespace w32::mac
