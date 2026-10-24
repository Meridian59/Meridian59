/*
 * menu.cpp:  HMENU objects, MENU / MENUEX resources, the menu API, and the menu
 *   bar: laid out and drawn by us at the top of the window's surface, its items
 *   opening the toolkit's drop-down menus.
 */

#include "core/menu.h"

#include "core/gdi.h"
#include "core/handles.h"
#include "core/module.h"
#include "core/window.h"

#include <algorithm>
#include <cstring>
#include <map>

namespace w32 {

namespace {

struct MenuItem
{
   UINT type = MFT_STRING;             // MFT_*
   UINT state = 0;                     // MFS_*
   UINT id = 0;
   HMENU sub = nullptr;
   std::string text;
   ULONG_PTR data = 0;
   HBITMAP bitmap = nullptr;
};

struct Menu
{
   std::vector<MenuItem> items;
   bool popup = false;
   HBRUSH back = nullptr;              // MENUINFO hbrBack
   DWORD style = 0;
   ULONG_PTR data = 0;
   HWND hwnd = nullptr;                // the window whose menu bar this is
};

HandleTable<Menu, HMENU> &Menus()
{
   static HandleTable<Menu, HMENU> menus(kMenuHandleBase);
   return menus;
}

Menu *GetMenuObject(HMENU h)
{
   return Menus().Get(h);
}

// The item at pos (MF_BYPOSITION) or with id pos (searching submenus), and the
// menu that holds it.
MenuItem *FindItem(HMENU h, UINT pos, bool by_position, Menu **holder = nullptr, int *index = nullptr)
{
   Menu *m = GetMenuObject(h);
   if (m == nullptr)
      return nullptr;
   if (by_position)
   {
      if (pos >= m->items.size())
         return nullptr;
      if (holder)
         *holder = m;
      if (index)
         *index = (int) pos;
      return &m->items[pos];
   }
   for (size_t i = 0; i < m->items.size(); i++)
   {
      MenuItem &it = m->items[i];
      if (it.sub == nullptr && it.id == pos)
      {
         if (holder)
            *holder = m;
         if (index)
            *index = (int) i;
         return &it;
      }
   }
   for (size_t i = 0; i < m->items.size(); i++)
      if (m->items[i].sub)
         if (MenuItem *found = FindItem(m->items[i].sub, pos, false, holder, index))
            return found;
   // A submenu's own id is its handle (old-style popups).
   for (size_t i = 0; i < m->items.size(); i++)
      if (m->items[i].sub && (UINT) (UINT_PTR) m->items[i].sub == pos)
      {
         if (holder)
            *holder = m;
         if (index)
            *index = (int) i;
         return &m->items[i];
      }
   return nullptr;
}

// An item from the old-style flags (MF_*) and new item value.
void SetFromFlags(MenuItem &it, UINT flags, UINT_PTR id_or_sub, LPCSTR content)
{
   it.type = 0;
   it.state = 0;
   if (flags & MF_SEPARATOR)
      it.type |= MFT_SEPARATOR;
   if (flags & MF_OWNERDRAW)
   {
      it.type |= MFT_OWNERDRAW;
      it.data = (ULONG_PTR) content;
   }
   else if (flags & MF_BITMAP)
   {
      it.type |= MFT_BITMAP;
      it.bitmap = (HBITMAP) content;
   }
   else if (!(flags & MF_SEPARATOR))
      it.text = content ? content : "";
   if (flags & MF_MENUBARBREAK)
      it.type |= MFT_MENUBARBREAK;
   if (flags & MF_MENUBREAK)
      it.type |= MFT_MENUBREAK;
   if (flags & MF_RIGHTJUSTIFY)
      it.type |= MFT_RIGHTJUSTIFY;
   if (flags & MF_GRAYED)
      it.state |= MFS_GRAYED;
   if (flags & MF_DISABLED)
      it.state |= MFS_DISABLED;
   if (flags & MF_CHECKED)
      it.state |= MFS_CHECKED;
   if (flags & MF_HILITE)
      it.state |= MFS_HILITE;
   if (flags & MF_POPUP)
   {
      it.sub = (HMENU) id_or_sub;
      it.id = (UINT) id_or_sub;
      if (Menu *s = GetMenuObject(it.sub))
         s->popup = true;
   }
   else
   {
      it.sub = nullptr;
      it.id = (UINT) id_or_sub;
   }
}

void MenuChanged(Menu *m)
{
   if (m && m->hwnd)
      if (Window *w = GetWindowObject(m->hwnd))
         PaintMenuBar(w);
}

std::string FromUtf16(const WCHAR *s, const BYTE *end, const BYTE **next)
{
   const WCHAR *start = s;
   while ((const BYTE *) (s + 1) <= end && *s)
      s++;
   *next = (const BYTE *) (s + 1);
   std::string out((size_t) (s - start) * 2, '\0');
   int n = WideCharToMultiByte(CP_ACP, 0, start, (int) (s - start), out.data(), (int) out.size(), nullptr, nullptr);
   out.resize(n > 0 ? n : 0);
   return out;
}

// MENU resource items, to the matching MF_END.
const BYTE *ParseMenu(HMENU h, const BYTE *p, const BYTE *end)
{
   while (p + 2 <= end)
   {
      WORD flags = *(const WORD *) p;
      p += 2;
      UINT id = 0;
      if (!(flags & MF_POPUP))
      {
         if (p + 2 > end)
            return end;
         id = *(const WORD *) p;
         p += 2;
      }
      std::string text = FromUtf16((const WCHAR *) p, end, &p);
      if (flags & MF_POPUP)
      {
         HMENU sub = CreatePopupMenu();
         p = ParseMenu(sub, p, end);
         AppendMenuA(h, (flags & ~MF_END) | MF_POPUP, (UINT_PTR) sub, text.c_str());
      }
      else if (id == 0 && text.empty() && !(flags & ~MF_END))
         AppendMenuA(h, MF_SEPARATOR, 0, nullptr);
      else
         AppendMenuA(h, flags & ~(MF_END | MF_POPUP), id, text.c_str());
      if (flags & MF_END)
         break;
   }
   return p;
}

// MENUEX resource items (DWORD-aligned), to the matching end flag.
const BYTE *ParseMenuEx(HMENU h, const BYTE *p, const BYTE *start, const BYTE *end)
{
   while (p + 14 <= end)
   {
      DWORD type = *(const DWORD *) p, state = *(const DWORD *) (p + 4);
      UINT id = *(const DWORD *) (p + 8);
      WORD flags = *(const WORD *) (p + 12);
      p += 14;
      std::string text = FromUtf16((const WCHAR *) p, end, &p);
      p = start + (((p - start) + 3) & ~3);
      MENUITEMINFOA mii = {};
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_STRING;
      mii.fType = type;
      mii.fState = state;
      mii.wID = id;
      mii.dwTypeData = (LPSTR) text.c_str();
      if (flags & 0x01)
      {
         p += 4;   // help id
         HMENU sub = CreatePopupMenu();
         p = ParseMenuEx(sub, p, start, end);
         mii.fMask |= MIIM_SUBMENU;
         mii.hSubMenu = sub;
      }
      InsertMenuItemA(h, GetMenuItemCount(h), TRUE, &mii);
      if (flags & 0x80)
         break;
   }
   return p;
}

// --- The menu bar ------------------------------------------------------------------

struct BarState
{
   std::vector<RECT> rects;            // per item, surface coordinates
   int height = 0;
   int hot = -1;                       // the item whose menu is open
};

std::map<HWND, BarState> &Bars()
{
   static std::map<HWND, BarState> bars;
   return bars;
}

HFONT MenuFont()
{
   static HFONT font = nullptr;
   if (font == nullptr)
   {
      NONCLIENTMETRICSA m = {};
      m.cbSize = sizeof(m);
      SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(m), &m, 0);
      font = CreateFontIndirectA(&m.lfMenuFont);
   }
   return font;
}

HMENU BarMenu(Window *w)
{
   if (w == nullptr || (w->style & WS_CHILD) || w->id == 0)   // a WS_CHILD window's id is no menu
      return nullptr;
   return GetMenuObject((HMENU) w->id) ? (HMENU) w->id : nullptr;
}

// Lays out w's menu bar items in rows across the window's width.
void LayoutBar(Window *w, BarState &bar)
{
   HMENU h = BarMenu(w);
   Menu *m = GetMenuObject(h);
   bar.rects.clear();
   bar.height = 0;
   if (m == nullptr)
      return;
   int width = w->rect.right - w->rect.left;
   int row_height = GetSystemMetrics(SM_CYMENU);
   HDC hdc = GetDC(nullptr);
   HGDIOBJ old = SelectObject(hdc, MenuFont());
   int x = 0, y = 0;
   for (size_t i = 0; i < m->items.size(); i++)
   {
      MenuItem &it = m->items[i];
      int item_width;
      if (it.type & MFT_OWNERDRAW)
      {
         MEASUREITEMSTRUCT mis = {ODT_MENU, 0, it.id, 0, (UINT) row_height, it.data};
         SendMessageA(w->hwnd, WM_MEASUREITEM, 0, (LPARAM) &mis);
         item_width = (int) mis.itemWidth + 2 * GetSystemMetrics(SM_CXEDGE) + 8;
      }
      else if (it.type & MFT_SEPARATOR)
         item_width = 8;
      else
      {
         RECT r = {0, 0, 0, 0};
         DrawTextA(hdc, it.text.c_str(), -1, &r, DT_SINGLELINE | DT_CALCRECT);
         item_width = (r.right - r.left) + 16;
      }
      if (x > 0 && (x + item_width > width || (it.type & (MFT_MENUBREAK | MFT_MENUBARBREAK))))
      {
         x = 0;
         y += row_height;
      }
      bar.rects.push_back({x, y, x + item_width, y + row_height});
      x += item_width;
   }
   SelectObject(hdc, old);
   DeleteDC(hdc);
   bar.height = y + row_height + 1;   // and the line under it
}

void DrawBarItem(Window *w, HDC hdc, Menu *m, size_t i, const RECT &rc, bool selected)
{
   MenuItem &it = m->items[i];
   bool grayed = (it.state & (MFS_GRAYED | MFS_DISABLED)) != 0;
   if (it.type & MFT_OWNERDRAW)
   {
      UINT state = ODS_NOACCEL | (selected ? ODS_SELECTED : 0) | (grayed ? ODS_GRAYED : 0);
      DRAWITEMSTRUCT dis = {ODT_MENU, 0, it.id, ODA_DRAWENTIRE, state, (HWND) w->id, hdc, rc, it.data};
      SendMessageA(w->hwnd, WM_DRAWITEM, 0, (LPARAM) &dis);
      return;
   }
   if (selected)
      FillRect(hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
   if (it.type & MFT_SEPARATOR)
      return;
   SetBkMode(hdc, TRANSPARENT);
   SetTextColor(hdc, GetSysColor(grayed ? COLOR_GRAYTEXT : selected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
   HGDIOBJ old = SelectObject(hdc, MenuFont());
   RECT r = rc;
   DrawTextA(hdc, it.text.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_HIDEPREFIX);
   SelectObject(hdc, old);
}

std::vector<native::MenuEntry> Entries(HMENU h, HWND owner)
{
   std::vector<native::MenuEntry> out;
   Menu *m = GetMenuObject(h);
   if (m == nullptr)
      return out;
   for (size_t i = 0; i < m->items.size(); i++)
   {
      MenuItem &it = m->items[i];
      native::MenuEntry e;
      e.separator = (it.type & MFT_SEPARATOR) != 0;
      std::string text = (it.type & MFT_OWNERDRAW) ? (it.data ? (const char *) it.data : "") : it.text;
      size_t tab = text.find('\t');
      e.text = text.substr(0, tab);
      if (tab != std::string::npos)
         e.accel = text.substr(tab + 1);
      e.id = it.id;
      e.checked = (it.state & MFS_CHECKED) != 0;
      e.radio = (it.type & MFT_RADIOCHECK) != 0;
      e.grayed = (it.state & (MFS_GRAYED | MFS_DISABLED)) != 0;
      if (it.sub)
      {
         // Windows sends this as each submenu opens; the toolkit's menu is built
         // ahead, so all of them get it now.
         SendMessageA(owner, WM_INITMENUPOPUP, (WPARAM) it.sub, MAKELPARAM(i, FALSE));
         e.has_sub = true;
         e.sub = Entries(it.sub, owner);
      }
      out.push_back(std::move(e));
   }
   return out;
}

struct Tracking
{
   HWND hwnd;
   bool bar;
   bool return_cmd;
   bool done = false;
   UINT id = 0;
};

void EndTracking(UINT id, void *data)
{
   auto *t = (Tracking *) data;
   t->id = id;
   t->done = true;
   if (t->bar)
   {
      if (Window *w = GetWindowObject(t->hwnd))
      {
         Bars()[t->hwnd].hot = -1;
         PaintMenuBar(w);
      }
   }
   if (IsWindow(t->hwnd))
   {
      SendMessageA(t->hwnd, WM_MENUSELECT, MAKEWPARAM(0, 0xFFFF), 0);
      SendMessageA(t->hwnd, WM_EXITMENULOOP, !t->bar, 0);
      if (id && !t->return_cmd)
         PostMessageA(t->hwnd, WM_COMMAND, MAKEWPARAM(id, 0), 0);
   }
   if (t->bar)
      delete t;
}

// Opens popup over anchor (surface coordinates of hwnd's top-level window).
void Open(HWND hwnd, HMENU popup, const RECT &anchor, Tracking *t)
{
   HWND top = TopLevelOf(hwnd);
   Window *tw = GetWindowObject(top);
   SendMessageA(hwnd, WM_ENTERMENULOOP, !t->bar, 0);
   if (!t->bar)
      SendMessageA(hwnd, WM_INITMENUPOPUP, (WPARAM) popup, 0);
   auto items = Entries(popup, hwnd);
   if (tw == nullptr || tw->top == nullptr || items.empty())
   {
      EndTracking(0, t);
      return;
   }
   native::TrackMenu(tw->top, items, anchor, EndTracking, t);
}

}  // namespace

int MenuBarHeight(Window *w)
{
   if (BarMenu(w) == nullptr)
   {
      Bars().erase(w->hwnd);
      return 0;
   }
   BarState &bar = Bars()[w->hwnd];
   LayoutBar(w, bar);
   return bar.height;
}

void AttachMenu(HWND hwnd, HMENU menu)
{
   if (Menu *m = GetMenuObject(menu))
      m->hwnd = hwnd;
}

void PaintMenuBar(Window *w)
{
   HMENU h = BarMenu(w);
   Menu *m = GetMenuObject(h);
   if (m == nullptr || !w->visible)
      return;
   BarState &bar = Bars()[w->hwnd];
   if (bar.rects.size() != m->items.size())
      LayoutBar(w, bar);
   HDC hdc = GetWindowDC(w->hwnd);
   RECT all = {0, 0, w->rect.right - w->rect.left, bar.height - 1};
   FillRect(hdc, &all, m->back ? m->back : GetSysColorBrush(COLOR_MENU));
   RECT line = {0, bar.height - 1, all.right, bar.height};
   FillRect(hdc, &line, GetSysColorBrush(COLOR_3DSHADOW));
   for (size_t i = 0; i < m->items.size() && i < bar.rects.size(); i++)
      DrawBarItem(w, hdc, m, i, bar.rects[i], (int) i == bar.hot);
   ReleaseDC(w->hwnd, hdc);
}

bool MenuBarMouse(Window *w, UINT msg, int x, int y)
{
   HMENU h = BarMenu(w);
   Menu *m = GetMenuObject(h);
   if (m == nullptr || y >= w->client.top)
      return false;
   if (msg != WM_LBUTTONDOWN || !w->enabled)
      return true;
   BarState &bar = Bars()[w->hwnd];
   POINT p = {x, y};
   for (size_t i = 0; i < bar.rects.size() && i < m->items.size(); i++)
   {
      if (!PtInRect(&bar.rects[i], p))
         continue;
      MenuItem &it = m->items[i];
      if (it.state & (MFS_GRAYED | MFS_DISABLED) || (it.type & MFT_SEPARATOR))
         return true;
      SendMessageA(w->hwnd, WM_INITMENU, (WPARAM) h, 0);
      if (it.sub == nullptr)
      {
         PostMessageA(w->hwnd, WM_COMMAND, MAKEWPARAM(it.id, 0), 0);
         return true;
      }
      bar.hot = (int) i;
      PaintMenuBar(w);
      SendMessageA(w->hwnd, WM_MENUSELECT, MAKEWPARAM(i, MF_POPUP | MF_HILITE), (LPARAM) h);
      SendMessageA(w->hwnd, WM_INITMENUPOPUP, (WPARAM) it.sub, MAKELPARAM(i, FALSE));
      Open(w->hwnd, it.sub, bar.rects[i], new Tracking{w->hwnd, true, false});
      return true;
   }
   return true;
}

}  // namespace w32

using namespace w32;

HMENU WINAPI CreateMenu(void)
{
   return Menus().Add(new Menu);
}

HMENU WINAPI CreatePopupMenu(void)
{
   auto *m = new Menu;
   m->popup = true;
   return Menus().Add(m);
}

BOOL WINAPI DestroyMenu(HMENU h)
{
   Menu *m = Menus().Remove(h);
   if (m == nullptr)
      return FALSE;
   for (MenuItem &it : m->items)
      if (it.sub)
         DestroyMenu(it.sub);
   delete m;
   return TRUE;
}

BOOL WINAPI IsMenu(HMENU h)
{
   return GetMenuObject(h) != nullptr;
}

HMENU WINAPI LoadMenuIndirectA(const void *templ)
{
   // The size isn't known; resources come through LoadMenuA, which knows it.
   const BYTE *p = (const BYTE *) templ;
   HMENU h = CreateMenu();
   WORD version = *(const WORD *) p, offset = *(const WORD *) (p + 2);
   const BYTE *end = p + 0x10000;
   if (version == 1)
      ParseMenuEx(h, p + 4 + offset, p, end);
   else
      ParseMenu(h, p + 4 + offset, end);
   return h;
}

HMENU WINAPI LoadMenuA(HINSTANCE instance, LPCSTR name)
{
   const BYTE *data;
   DWORD size;
   if (!FindResourceData(instance, name, (LPCSTR) RT_MENU, &data, &size) || size < 4)
      return nullptr;
   HMENU h = CreateMenu();
   WORD version = *(const WORD *) data, offset = *(const WORD *) (data + 2);
   if (version == 1)
      ParseMenuEx(h, data + 4 + offset, data, data + size);
   else
      ParseMenu(h, data + 4 + offset, data + size);
   return h;
}

HMENU WINAPI GetMenu(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return (w && !(w->style & WS_CHILD)) ? (HMENU) w->id : nullptr;
}

BOOL WINAPI SetMenu(HWND hwnd, HMENU h)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || (w->style & WS_CHILD))
      return FALSE;
   if (Menu *old = GetMenuObject((HMENU) w->id))
      old->hwnd = nullptr;
   w->id = (UINT_PTR) h;
   if (Menu *m = GetMenuObject(h))
      m->hwnd = hwnd;
   SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
   return TRUE;
}

BOOL WINAPI DrawMenuBar(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   // Items may have come or gone: the bar may change height.
   int height = Bars().count(hwnd) ? Bars()[hwnd].height : 0;
   if (MenuBarHeight(w) != height)
      SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
   else
      PaintMenuBar(w);
   return TRUE;
}

HMENU WINAPI GetSubMenu(HMENU h, INT pos)
{
   MenuItem *it = FindItem(h, (UINT) pos, true);
   return it ? it->sub : nullptr;
}

INT WINAPI GetMenuItemCount(HMENU h)
{
   Menu *m = GetMenuObject(h);
   return m ? (INT) m->items.size() : -1;
}

UINT WINAPI GetMenuItemID(HMENU h, INT pos)
{
   MenuItem *it = FindItem(h, (UINT) pos, true);
   if (it == nullptr || it->sub)
      return (UINT) -1;
   return it->id;
}

UINT WINAPI GetMenuState(HMENU h, UINT item, UINT flags)
{
   MenuItem *it = FindItem(h, item, (flags & MF_BYPOSITION) != 0);
   if (it == nullptr)
      return (UINT) -1;
   UINT state = it->state | (it->type & (MFT_SEPARATOR | MFT_OWNERDRAW | MFT_BITMAP | MFT_MENUBREAK | MFT_MENUBARBREAK));
   if (it->sub)
      state = (state & 0xFF) | MF_POPUP | (GetMenuItemCount(it->sub) << 8);
   return state;
}

INT WINAPI GetMenuStringA(HMENU h, UINT item, LPSTR buffer, INT size, UINT flags)
{
   MenuItem *it = FindItem(h, item, (flags & MF_BYPOSITION) != 0);
   if (it == nullptr)
      return 0;
   if (buffer == nullptr || size <= 0)
      return (INT) it->text.size();
   lstrcpynA(buffer, it->text.c_str(), size);
   return (INT) strlen(buffer);
}

DWORD WINAPI CheckMenuItem(HMENU h, UINT item, UINT flags)
{
   Menu *holder = nullptr;
   MenuItem *it = FindItem(h, item, (flags & MF_BYPOSITION) != 0, &holder);
   if (it == nullptr)
      return (DWORD) -1;
   DWORD was = (it->state & MFS_CHECKED) ? MF_CHECKED : MF_UNCHECKED;
   if (flags & MF_CHECKED)
      it->state |= MFS_CHECKED;
   else
      it->state &= ~MFS_CHECKED;
   return was;
}

BOOL WINAPI CheckMenuRadioItem(HMENU h, UINT first, UINT last, UINT check, UINT flags)
{
   bool by_position = (flags & MF_BYPOSITION) != 0;
   Menu *m = nullptr;
   int first_index, last_index, check_index;
   if (!FindItem(h, first, by_position, &m, &first_index) || !FindItem(h, last, by_position, nullptr, &last_index) ||
       !FindItem(h, check, by_position, nullptr, &check_index))
      return FALSE;
   for (int i = first_index; i <= last_index && i < (int) m->items.size(); i++)
   {
      MenuItem &it = m->items[i];
      if (i == check_index)
      {
         it.state |= MFS_CHECKED;
         it.type |= MFT_RADIOCHECK;
      }
      else
         it.state &= ~MFS_CHECKED;
   }
   return TRUE;
}

BOOL WINAPI EnableMenuItem(HMENU h, UINT item, UINT flags)
{
   Menu *holder = nullptr;
   MenuItem *it = FindItem(h, item, (flags & MF_BYPOSITION) != 0, &holder);
   if (it == nullptr)
      return -1;
   UINT was = it->state & (MFS_GRAYED | MFS_DISABLED);
   it->state &= ~(MFS_GRAYED | MFS_DISABLED);
   if (flags & MF_GRAYED)
      it->state |= MFS_GRAYED;
   if (flags & MF_DISABLED)
      it->state |= MFS_DISABLED;
   if (holder && !holder->popup)
      MenuChanged(holder);
   return was;
}

BOOL WINAPI InsertMenuA(HMENU h, UINT pos, UINT flags, UINT_PTR id, LPCSTR content)
{
   Menu *m = GetMenuObject(h);
   if (m == nullptr)
      return FALSE;
   Menu *holder = m;
   int index = (int) m->items.size();
   if (pos != (UINT) -1)
   {
      if (flags & MF_BYPOSITION)
         index = std::min<int>((int) pos, (int) m->items.size());
      else if (!FindItem(h, pos, false, &holder, &index))
         holder = m, index = (int) m->items.size();
   }
   MenuItem it;
   SetFromFlags(it, flags, id, content);
   holder->items.insert(holder->items.begin() + index, it);
   return TRUE;
}

BOOL WINAPI AppendMenuA(HMENU h, UINT flags, UINT_PTR id, LPCSTR content)
{
   return InsertMenuA(h, (UINT) -1, flags | MF_BYPOSITION, id, content);
}

BOOL WINAPI ModifyMenuA(HMENU h, UINT pos, UINT flags, UINT_PTR id, LPCSTR content)
{
   MenuItem *it = FindItem(h, pos, (flags & MF_BYPOSITION) != 0);
   if (it == nullptr)
      return FALSE;
   SetFromFlags(*it, flags, id, content);
   return TRUE;
}

BOOL WINAPI RemoveMenu(HMENU h, UINT pos, UINT flags)
{
   Menu *holder = nullptr;
   int index;
   if (!FindItem(h, pos, (flags & MF_BYPOSITION) != 0, &holder, &index))
      return FALSE;
   holder->items.erase(holder->items.begin() + index);
   return TRUE;
}

BOOL WINAPI DeleteMenu(HMENU h, UINT pos, UINT flags)
{
   Menu *holder = nullptr;
   int index;
   if (!FindItem(h, pos, (flags & MF_BYPOSITION) != 0, &holder, &index))
      return FALSE;
   HMENU sub = holder->items[index].sub;
   holder->items.erase(holder->items.begin() + index);
   if (sub)
      DestroyMenu(sub);
   return TRUE;
}

BOOL WINAPI GetMenuItemInfoA(HMENU h, UINT item, BOOL by_position, MENUITEMINFOA *mii)
{
   MenuItem *it = FindItem(h, item, by_position != 0);
   if (it == nullptr || mii == nullptr)
      return FALSE;
   UINT mask = mii->fMask;
   if (mask & (MIIM_FTYPE | MIIM_TYPE))
      mii->fType = it->type;
   if (mask & MIIM_STATE)
      mii->fState = it->state;
   if (mask & MIIM_ID)
      mii->wID = it->id;
   if (mask & MIIM_SUBMENU)
      mii->hSubMenu = it->sub;
   if (mask & MIIM_DATA)
      mii->dwItemData = it->data;
   if (mask & MIIM_BITMAP)
      mii->hbmpItem = it->bitmap;
   if (mask & (MIIM_STRING | MIIM_TYPE))
   {
      if (it->type & (MFT_OWNERDRAW | MFT_BITMAP | MFT_SEPARATOR))
      {
         if (mask & MIIM_TYPE)
            mii->dwTypeData = (it->type & MFT_OWNERDRAW) ? (LPSTR) it->data : (LPSTR) it->bitmap;
         mii->cch = 0;
      }
      else
      {
         if (mii->dwTypeData && mii->cch)
            lstrcpynA(mii->dwTypeData, it->text.c_str(), mii->cch);
         mii->cch = (UINT) it->text.size();
      }
   }
   return TRUE;
}

BOOL WINAPI SetMenuItemInfoA(HMENU h, UINT item, BOOL by_position, const MENUITEMINFOA *mii)
{
   Menu *holder = nullptr;
   MenuItem *it = FindItem(h, item, by_position != 0, &holder);
   if (it == nullptr || mii == nullptr)
      return FALSE;
   UINT mask = mii->fMask;
   if (mask & MIIM_TYPE)
   {
      it->type = mii->fType;
      if (mii->fType & MFT_OWNERDRAW)
         it->data = (ULONG_PTR) mii->dwTypeData;
      else if (mii->fType & MFT_BITMAP)
         it->bitmap = (HBITMAP) mii->dwTypeData;
      else if (!(mii->fType & MFT_SEPARATOR))
         it->text = mii->dwTypeData ? mii->dwTypeData : "";
   }
   if (mask & MIIM_FTYPE)
      it->type = mii->fType;
   if ((mask & MIIM_STRING) && !(it->type & MFT_OWNERDRAW))
      it->text = mii->dwTypeData ? mii->dwTypeData : "";
   if (mask & MIIM_STATE)
      it->state = mii->fState;
   if (mask & MIIM_ID)
      it->id = mii->wID;
   if (mask & MIIM_SUBMENU)
   {
      it->sub = mii->hSubMenu;
      if (Menu *s = GetMenuObject(it->sub))
         s->popup = true;
   }
   if (mask & MIIM_DATA)
      it->data = mii->dwItemData;
   if (mask & MIIM_BITMAP)
      it->bitmap = mii->hbmpItem;
   if (holder && !holder->popup && holder->hwnd)
   {
      // Owner-drawn items measure again.
      if (Window *w = GetWindowObject(holder->hwnd))
         Bars()[holder->hwnd].rects.clear(), PaintMenuBar(w);
   }
   return TRUE;
}

BOOL WINAPI InsertMenuItemA(HMENU h, UINT item, BOOL by_position, const MENUITEMINFOA *mii)
{
   Menu *m = GetMenuObject(h);
   if (m == nullptr || mii == nullptr)
      return FALSE;
   Menu *holder = m;
   int index = (int) m->items.size();
   if (by_position)
      index = std::min<int>((int) item, (int) m->items.size());
   else if (!FindItem(h, item, false, &holder, &index))
      holder = m, index = (int) m->items.size();
   holder->items.insert(holder->items.begin() + index, MenuItem());
   return SetMenuItemInfoA(h, holder == m ? (UINT) index : item, holder == m ? TRUE : by_position, mii);
}

BOOL WINAPI GetMenuInfo(HMENU h, MENUINFO *mi)
{
   Menu *m = GetMenuObject(h);
   if (m == nullptr || mi == nullptr)
      return FALSE;
   if (mi->fMask & MIM_BACKGROUND)
      mi->hbrBack = m->back;
   if (mi->fMask & MIM_STYLE)
      mi->dwStyle = m->style;
   if (mi->fMask & MIM_MENUDATA)
      mi->dwMenuData = m->data;
   return TRUE;
}

BOOL WINAPI SetMenuInfo(HMENU h, const MENUINFO *mi)
{
   Menu *m = GetMenuObject(h);
   if (m == nullptr || mi == nullptr)
      return FALSE;
   if (mi->fMask & MIM_BACKGROUND)
      m->back = mi->hbrBack;
   if (mi->fMask & MIM_STYLE)
      m->style = mi->dwStyle;
   if (mi->fMask & MIM_MENUDATA)
      m->data = mi->dwMenuData;
   if (mi->fMask & MIM_APPLYTOSUBMENUS)
      for (MenuItem &it : m->items)
         if (it.sub)
            SetMenuInfo(it.sub, mi);
   return TRUE;
}

BOOL WINAPI TrackPopupMenuEx(HMENU h, UINT flags, INT x, INT y, HWND hwnd, TPMPARAMS *)
{
   if (GetMenuObject(h) == nullptr || !IsWindow(hwnd))
      return FALSE;
   // The anchor: (x, y) in the top-level window's surface coordinates.
   HWND top = TopLevelOf(hwnd);
   Window *tw = GetWindowObject(top);
   POINT p = {x, y};
   ScreenToClient(top, &p);
   p.x += tw->client.left;
   p.y += tw->client.top;
   Tracking t = {hwnd, false, (flags & TPM_RETURNCMD) != 0};
   Open(hwnd, h, {p.x, p.y, p.x + 1, p.y + 1}, &t);
   // Modal, as Windows' menu loop is.
   MSG msg;
   while (!t.done && GetMessageA(&msg, nullptr, 0, 0) > 0)
   {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
   }
   return (flags & TPM_RETURNCMD) ? (BOOL) t.id : TRUE;
}

BOOL WINAPI TrackPopupMenu(HMENU h, UINT flags, INT x, INT y, INT, HWND hwnd, const RECT *)
{
   return TrackPopupMenuEx(h, flags, x, y, hwnd, nullptr);
}
