/*
 * menu.h:  Menus (HMENU): items with ids, text, state and submenus.  A top-level
 *   window's menu bar is a strip of ours above its client area, drawn with GDI
 *   (owner-drawn items get WM_MEASUREITEM / WM_DRAWITEM); drop-down and popup menus
 *   are the toolkit's, built from the items when they open.
 */

#ifndef W32_CORE_MENU_H
#define W32_CORE_MENU_H

#include "core/win32.h"

#include <string>
#include <vector>

namespace w32 {

struct Window;

// Height of hwnd's menu bar (0 without a menu), for the client area's offset.
int MenuBarHeight(Window *w);

// Makes menu the menu bar of top-level window hwnd (at creation).
void AttachMenu(HWND hwnd, HMENU menu);

// Repaints the menu bar of top-level window w.
void PaintMenuBar(Window *w);

// Mouse input on the menu bar strip (x, y in the window's surface coordinates);
// true if it was there.
bool MenuBarMouse(Window *w, UINT msg, int x, int y);

}  // namespace w32

namespace w32::native {

// A menu as the toolkit shows it: what the core read from an HMENU when it opened.
struct MenuEntry
{
   std::string text;                   // ANSI, '&' marks the mnemonic
   std::string accel;                  // the text after a tab ("Ctrl+X"), shown right
   UINT id = 0;
   bool separator = false;
   bool checked = false, radio = false;
   bool grayed = false;
   std::vector<MenuEntry> sub;         // a submenu's items
   bool has_sub = false;
};

class Top;

// Opens a menu below anchor (surface coordinates of top); calls done(id) once it
// closed: the chosen item's id, or 0.
void TrackMenu(Top *top, const std::vector<MenuEntry> &items, const RECT &anchor, void (*done)(UINT id, void *data),
               void *data);

}  // namespace w32::native

#endif
