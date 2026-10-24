/*
 * bridge.h:  Between the AppKit backend's two halves.  Its Objective-C++ files
 *   see no Windows headers (Apple's declare C functions and types under Windows'
 *   names: GetCurrentProcess, LoadResource, Polygon, sockaddr ...), only this;
 *   backend.cpp implements core/native.h with what they provide here, and turns
 *   their events into the core's.
 */

#ifndef W32_APPKIT_BRIDGE_H
#define W32_APPKIT_BRIDGE_H

#include <cstdint>
#include <string>
#include <vector>

namespace w32::native {
class Top;
class Font;
}

// Names here avoid Windows' A/W macros (CreateFont ...), which rename them in
// backend.cpp only.
namespace w32::mac {

using native::Font;
using native::Top;

struct Rect
{
   int left = 0, top = 0, right = 0, bottom = 0;
};

struct Point
{
   int x = 0, y = 0;
};

// --- Provided by the Objective-C++ half ----------------------------------------

void Init();

struct TopStyle
{
   bool caption = true, resizable = false, minimize_box = false, maximize_box = false;
   bool tool = false, popup_menu_like = false;
};

// hwnd is the core's HWND, handed back in events.
Top *CreateTop(void *hwnd, const TopStyle &style, int x, int y, int width, int height, Top *owner);
void DestroyTop(Top *top);
void SetTitle(Top *top, const std::string &utf8);
void SetGeometry(Top *top, int x, int y, int width, int height);
void Show(Top *top, bool show);
void SetMinimized(Top *top, bool minimized);
void SetMaximized(Top *top, bool maximized);
void Activate(Top *top);
bool IsActive(Top *top);

struct Surface
{
   uint8_t *pixels = nullptr;          // BGRA, row 0 at the top
   int width = 0, height = 0, stride = 0;
};
Surface GetSurface(Top *top);
void Present(Top *top, const Rect &rect);

Point GetCursorPos();
bool SetCursorPos(int x, int y);
struct CursorSpec
{
   std::string standard;               // a CSS cursor name, or empty for the image
   int width = 0, height = 0, hot_x = 0, hot_y = 0;
   const uint32_t *pixels = nullptr;   // straight-alpha ARGB, rows top first
};
void SetCursor(Top *top, const CursorSpec &cursor);
// The application's (Dock) icon, from every size it has.
void SetAppIcon(const std::vector<CursorSpec> &images);
void ShowCursor(Top *top, bool show);
bool KeyDown(int vk);
bool KeyToggled(int vk);

Rect ScreenRect();
Rect WorkArea();
void Beep();
void SetDarkTheme(bool dark);

// Text.  Sizes in pixels: height < 0 is the em, > 0 the cell.
struct FontMetrics
{
   int height = 0, ascent = 0, descent = 0, internal_leading = 0, external_leading = 0;
   int average_width = 0, max_width = 0;
   int weight = 400;
   bool italic = false, underline = false, strike_out = false;
   std::string face;                   // as GetTextFace gives it, UTF-8
};
struct FontSpec
{
   std::string face;                   // UTF-8
   int height = 0, weight = 400;
   bool italic = false, underline = false, strike_out = false;
   bool fixed_pitch = false;
   bool windows_metrics = false;       // metrics are Windows' (max_width 0: the font's)
   FontMetrics metrics;
};
bool FaceInstalled(const std::string &face);
Font *MakeFont(const FontSpec &spec);
void DestroyFont(Font *font);
FontMetrics GetFontMetrics(Font *font);
int TextWidth(Font *font, const std::string &utf8);
// Each code point's trailing edge in the run, as TextWidth measures it.
std::vector<int> CharEdges(Font *font, const std::string &utf8);
void CharWidths(Font *font, uint32_t codepoint, int *a, int *b, int *c);
int Kerning(Font *font, uint32_t first, uint32_t second);
std::vector<uint8_t> RenderText(Font *font, const std::string &utf8, int *width, int *height);

// Menus: tracks modally; the chosen item's id, or 0.
struct MenuItem
{
   std::string title;                  // UTF-8, mnemonics already gone
   unsigned id = 0;
   bool separator = false, checked = false, grayed = false, has_sub = false;
   std::vector<MenuItem> sub;
};
unsigned TrackMenu(Top *top, const std::vector<MenuItem> &items, const Rect &anchor);

// Common dialogs, modal.  Colors are 0x00BBGGRR (COLORREF).
bool PickColor(Top *owner, uint32_t initial, uint32_t *chosen);
bool PickFont(Top *owner, const FontSpec &initial, FontSpec *chosen);

// OpenGL over the surface: rect in surface coordinates.
bool GLMakeCurrent(Top *top);
void GLPresent(Top *top, unsigned texture, const Rect &rect);
void *GLProcAddress(const char *name);

// --- Provided by backend.cpp: the events, to the core ------------------------------

// keys: the MK_* bits (vk.h).
enum class Button { Left, Right, Middle };
void OnMouseMove(void *hwnd, int x, int y, unsigned keys);
void OnButton(void *hwnd, Button button, bool down, int x, int y, unsigned keys);
void OnWheel(void *hwnd, int x, int y, unsigned keys, int delta);
void OnMouseLeave(void *hwnd);
void OnKey(void *hwnd, bool down, int vk, int scan, bool repeat, bool alt, const char *text);
void OnResized(void *hwnd, int width, int height);
void OnMoved(void *hwnd, int x, int y);
void OnActivated(void *hwnd, bool active);
void OnCloseRequested(void *hwnd);
// Whether the focus is in a text field (an edit, a rich edit).
bool TextFieldFocused();
// Cmd+A, C, V, X or Z (vk): as Ctrl+ to a focused text field, or nothing.
void OnTextShortcut(void *hwnd, int vk, int scan);
// The application menu's Quit (Cmd+Q).
void OnQuit();
void OnMinimized(void *hwnd, bool minimized);
// The window's GL state changed under the Direct3D device.
void OnGLStateChanged();

}  // namespace w32::mac

#endif
