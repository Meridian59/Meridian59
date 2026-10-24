/*
 * native.h:  What the core needs from the platform toolkit (gtk/ on Linux, appkit/
 *   on macOS): top-level windows with a surface GDI draws on, input, the cursor,
 *   screens and text rendering.  The core calls these; the backend calls back into
 *   the core through native_events.h.
 */

#ifndef W32_CORE_NATIVE_H
#define W32_CORE_NATIVE_H

#include "core/win32.h"

#include <cstdint>
#include <string>
#include <vector>

namespace w32 {

// 32-bit pixels, premultiplied BGRA in memory order (B, G, R, A), row 0 at the top:
// a 32-bit DIB's layout with an opaque alpha byte.
struct Surface
{
   uint8_t *pixels = nullptr;
   int width = 0, height = 0;
   int stride = 0;                     // bytes per row
};

namespace native {

void Init();

// --- Top-level windows ---------------------------------------------------------
class Top;

struct TopStyle
{
   bool caption = true;                // title bar
   bool resizable = false;
   bool minimize_box = false, maximize_box = false;
   bool tool = false;                  // no taskbar entry (WS_EX_TOOLWINDOW)
   bool popup_menu_like = false;       // undecorated, unmanaged (WS_POPUP without caption)
   bool owned_popup = false;           // a popup surface placed on its owner's (drop-down lists, tooltips): never
                                       // active or focused, positioned relative to the owner's client area
};

// Makes (hidden) a top-level window for hwnd with client area width x height at
// screen position (x, y), owned by owner's top (or none).
Top *CreateTop(HWND hwnd, const TopStyle &style, int x, int y, int width, int height, Top *owner);
void DestroyTop(Top *top);

void SetTitle(Top *top, const std::string &title);
void SetGeometry(Top *top, int x, int y, int width, int height);  // client area, screen coordinates
void Show(Top *top, bool show);
void SetMinimized(Top *top, bool minimized);
void SetMaximized(Top *top, bool maximized);
// Modal: blocks input to the owner while shown (DialogBox).
void SetModal(Top *top, bool modal);
void Activate(Top *top);
bool IsActive(Top *top);
void SetIcon(Top *top, HICON icon);

// The client area's pixels.  Valid until the next call into the backend that may
// resize the window; the core redraws what Invalidate reports.
Surface GetSurface(Top *top);
// Shows rect of the surface on screen (after GDI drew into it).
void Present(Top *top, const RECT &rect);

// --- Input -------------------------------------------------------------------
// Pointer position in screen coordinates, as last seen (toolkits on Wayland can't
// query or warp the pointer globally; position over our windows is tracked).
POINT GetCursorPos();
bool  SetCursorPos(int x, int y);
void  SetCursor(Top *top, HCURSOR cursor);
void  ShowCursor(Top *top, bool show);
void  CaptureMouse(Top *top, bool capture);
// Keyboard state of a virtual key (VK_*), from the last input events.
bool  KeyDown(int vk);
bool  KeyToggled(int vk);

// --- Screens -------------------------------------------------------------------
RECT ScreenRect();        // the primary screen
RECT WorkArea();          // the primary screen less panels
int  ScreenDpi();

// --- Text ----------------------------------------------------------------------
// A font for a LOGFONT: the face (Windows names mapped to installed equivalents),
// the height in pixels (negative: of the characters; positive: of the cell),
// weight, italic, underline, strike-out.
class Font;

struct FontMetrics
{
   int height = 0;              // ascent + descent
   int ascent = 0, descent = 0;
   int internal_leading = 0, external_leading = 0;
   int average_width = 0, max_width = 0;
   int weight = FW_NORMAL;
   bool italic = false, underline = false, strike_out = false;
   std::string face;
};

// Whether a font of the face (a Windows face name) is installed, or stands in
// for it (MS Sans Serif's raster sizes, Liberation's Arial ...).
bool FaceInstalled(const char *face);
Font *CreateFont(const LOGFONTA &lf);
void DestroyFont(Font *font);
FontMetrics GetFontMetrics(Font *font);
// Advance width of utf8 (a run, kerned and shaped as drawn).
int TextWidth(Font *font, const std::string &utf8);
// For each code point of utf8, the x of its trailing edge in the run as TextWidth
// measures and RenderText draws it (so the last is the run's width).
std::vector<int> CharEdges(Font *font, const std::string &utf8);
// One character's ABC widths (leading bearing, ink width, trailing bearing).
void CharWidths(Font *font, uint32_t codepoint, int *a, int *b, int *c);
// Kerning between two characters (adjustment to the advance, pixels; 0 if none).
int Kerning(Font *font, uint32_t first, uint32_t second);
// Draws utf8 as coverage (0-255) into a width x height 8-bit mask, with the
// text's top-left (not its baseline) at the mask's (0, 0); width is TextWidth
// plus overhang, height the font's height.
std::vector<uint8_t> RenderText(Font *font, const std::string &utf8, int *width, int *height);

// --- Common dialogs ------------------------------------------------------------
// The toolkit's color and font choosers, modal over owner (or none); done runs
// once they close, ok false if cancelled.
void ChooseColor(Top *owner, COLORREF initial, void (*done)(bool ok, COLORREF color, void *data), void *data);
void ChooseFont(Top *owner, const LOGFONTA &initial, void (*done)(bool ok, const LOGFONTA &font, void *data),
                void *data);

// --- Clipboard / sound / misc --------------------------------------------------------
void Beep();

// Dark or light window decorations and toolkit widgets (menus, dialogs).
void SetDarkTheme(bool dark);

}  // namespace native
}  // namespace w32

#endif
