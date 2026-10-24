/*
 * window.cpp:  Window classes, creating and destroying windows, the window tree,
 *   geometry, window longs and properties.
 */

#include "core/window.h"

#include "core/menu.h"

#include "core/caret.h"

#include "core/handles.h"
#include "core/eventloop.h"
#include "core/message.h"
#include "core/native.h"
#include "core/region.h"
#include "core/winproc.h"

#include <algorithm>
#include <cstring>
#include <strings.h>

namespace w32 {

namespace {

HandleTable<Window, HWND> &Windows()
{
   static HandleTable<Window, HWND> windows(kWindowHandleBase);
   return windows;
}

struct ClassKey
{
   std::string name;   // lower case
   HINSTANCE instance;
   bool operator<(const ClassKey &o) const
   {
      return name != o.name ? name < o.name : instance < o.instance;
   }
};

std::map<ClassKey, std::unique_ptr<WindowClass>> &Classes()
{
   static std::map<ClassKey, std::unique_ptr<WindowClass>> classes;
   return classes;
}

std::string Lower(const char *s)
{
   std::string out(s ? s : "");
   for (char &c : out)
      c = (char) tolower((unsigned char) c);
   return out;
}

// Class names may be atoms (MAKEINTATOM): the low word, as "#<n>".
std::string ClassName(LPCSTR name)
{
   if (IS_INTRESOURCE(name))
      return "#" + std::to_string((unsigned) (UINT_PTR) name);
   return name;
}

HWND focus_window = nullptr;
HWND capture_window = nullptr;
HWND active_window = nullptr;

// Non-client border widths of a child window, per side, by style (as Windows 2000's
// classic metrics: what the client's layout code expects).
RECT NonClientInsets(DWORD style, DWORD exstyle)
{
   int edge = 0;
   if (exstyle & WS_EX_DLGMODALFRAME)
      edge += 3;
   else if ((style & WS_CAPTION) == WS_DLGFRAME || (style & WS_THICKFRAME))
      edge += 3;
   else if (style & WS_BORDER)
      edge += 1;
   if (exstyle & WS_EX_CLIENTEDGE)
      edge += 2;
   if (exstyle & WS_EX_STATICEDGE)
      edge += 1;
   RECT r = {edge, edge, edge, edge};
   if ((style & WS_CAPTION) == WS_CAPTION)
      r.top += GetSystemMetrics((exstyle & WS_EX_TOOLWINDOW) ? SM_CYSMCAPTION : SM_CYCAPTION);
   if (style & WS_VSCROLL)
      r.right += GetSystemMetrics(SM_CXVSCROLL);
   if (style & WS_HSCROLL)
      r.bottom += GetSystemMetrics(SM_CYHSCROLL);
   return r;
}

// Top-level windows without the toolkit's decorations (popups without a caption
// or sizing frame, and WS_CHILD windows of the desktop) have a non-client area of
// ours, as child windows do.
bool HasOwnFrame(const Window *w)
{
   if (IsChildWindow(w))
      return true;
   return (w->style & WS_CHILD) ||
          ((w->style & WS_POPUP) && (w->style & WS_CAPTION) != WS_CAPTION && !(w->style & WS_THICKFRAME));
}

void ComputeClientArea(Window *w)
{
   int width = w->rect.right - w->rect.left, height = w->rect.bottom - w->rect.top;
   if (!HasOwnFrame(w))
   {
      // The menu bar is ours, at the top of the surface.
      int bar = std::min(MenuBarHeight(w), height);
      w->client = {0, bar, width, height};
      return;
   }
   RECT in = NonClientInsets(w->style, w->exstyle);
   w->client = {in.left, in.top, std::max(in.left, width - (int) in.right),
                std::max(in.top, height - (int) in.bottom)};
}

void Unlink(Window *w)
{
   if (w->parent)
      if (Window *p = GetWindowObject(w->parent))
         p->children.erase(std::remove(p->children.begin(), p->children.end(), w->hwnd), p->children.end());
}

native::TopStyle TopStyleFor(const Window *w)
{
   DWORD style = w->style, exstyle = w->exstyle;
   native::TopStyle s;
   // A caption is both of WS_CAPTION's bits; WS_BORDER alone is a thin frame.
   bool caption = (style & WS_CAPTION) == WS_CAPTION;
   s.caption = caption;
   s.resizable = (style & WS_THICKFRAME) != 0;
   s.minimize_box = (style & WS_MINIMIZEBOX) != 0;
   s.maximize_box = (style & WS_MAXIMIZEBOX) != 0;
   s.tool = (exstyle & WS_EX_TOOLWINDOW) != 0;
   s.popup_menu_like = (style & WS_POPUP) && !caption && !(style & WS_THICKFRAME);
   // Windows that never take the focus and show over their owner (drop-down
   // lists, tooltips) are popups placed on the owner's surface.
   s.owned_popup = w->owner && ((style & WS_CHILD) ||
                                (s.popup_menu_like && ((exstyle & (WS_EX_TOPMOST | WS_EX_NOACTIVATE)) ||
                                                       strcasecmp(w->cls->name.c_str(), "tooltips_class32") == 0)));
   if (style & WS_CHILD)
      s.popup_menu_like = true;
   return s;
}

}  // namespace

Window *GetWindowObject(HWND hwnd)
{
   if (hwnd == nullptr)
      return nullptr;
   Window *w = Windows().Get(hwnd);
   return w;
}

void ComputeClient(Window *w)
{
   ComputeClientArea(w);
}

bool IsChildWindow(const Window *w)
{
   // A WS_CHILD window of the desktop (SetParent(hwnd, NULL)) is a top-level one.
   return (w->style & WS_CHILD) != 0 && w->parent != nullptr;
}

HWND TopLevelOf(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   while (w != nullptr && IsChildWindow(w) && w->parent)
      w = GetWindowObject(w->parent);
   return w ? w->hwnd : nullptr;
}

RECT ClientRectInTop(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return {};
   int x = w->client.left, y = w->client.top;
   Window *c = w;
   while (IsChildWindow(c) && c->parent)
   {
      x += c->rect.left;
      y += c->rect.top;
      c = GetWindowObject(c->parent);
      if (c == nullptr)
         break;
      x += c->client.left;
      y += c->client.top;
   }
   return {x, y, x + (w->client.right - w->client.left), y + (w->client.bottom - w->client.top)};
}

WindowClass *FindClass(const char *name, HINSTANCE instance)
{
   std::string key = Lower(ClassName(name).c_str());
   auto &classes = Classes();
   auto it = classes.find({key, instance});
   if (it != classes.end())
      return it->second.get();
   // Any instance's class, then the system classes.
   for (auto &entry : classes)
      if (entry.first.name == key && !entry.second->system)
         return entry.second.get();
   it = classes.find({key, nullptr});
   return it != classes.end() ? it->second.get() : nullptr;
}

void RegisterSystemClass(const char *name, WNDPROC proc, UINT style, int window_extra,
                         HCURSOR cursor, HBRUSH background, bool unicode)
{
   auto cls = std::make_unique<WindowClass>();
   cls->name = name;
   cls->proc = proc;
   cls->style = style;
   cls->window_extra = window_extra;
   cls->cursor = cursor;
   cls->background = background;
   cls->unicode = unicode;
   cls->system = true;
   Classes()[{Lower(name), nullptr}] = std::move(cls);
}

LRESULT CallProc(Window *w, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return CallProcRef({w->proc, w->unicode}, w->hwnd, msg, wparam, lparam, false);
}

LRESULT CallProcW(Window *w, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return CallProcRef({w->proc, w->unicode}, w->hwnd, msg, wparam, lparam, true);
}

void InvalidateWhole(HWND hwnd, bool erase)
{
   RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) | RDW_ALLCHILDREN);
}

HWND FocusWindow() { return focus_window; }
HWND CaptureWindow() { return capture_window; }
HWND ActiveWindow() { return active_window; }
void SetActiveWindowState(HWND hwnd) { active_window = hwnd; }

}  // namespace w32

using namespace w32;

/************************************************************************/
// Classes

static ATOM RegisterClassCommon(const WNDCLASSEXA *wc, bool unicode)
{
   if (wc == nullptr || wc->lpszClassName == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }
   std::string key = Lower(ClassName(wc->lpszClassName).c_str());
   auto &classes = Classes();
   if (classes.count({key, wc->hInstance}))
   {
      SetLastError(ERROR_CLASS_ALREADY_EXISTS);
      return 0;
   }
   auto cls = std::make_unique<WindowClass>();
   cls->name = ClassName(wc->lpszClassName);
   cls->proc = wc->lpfnWndProc;
   cls->unicode = unicode;
   cls->style = wc->style;
   cls->class_extra = wc->cbClsExtra;
   cls->window_extra = wc->cbWndExtra;
   cls->instance = wc->hInstance;
   cls->icon = wc->hIcon;
   cls->icon_small = wc->hIconSm;
   cls->cursor = wc->hCursor;
   cls->background = wc->hbrBackground;
   if (wc->lpszMenuName)
      cls->menu_name = IS_INTRESOURCE(wc->lpszMenuName) ? "#" + std::to_string((UINT_PTR) wc->lpszMenuName)
                                                         : std::string(wc->lpszMenuName);
   cls->extra.assign(wc->cbClsExtra, 0);
   classes[{key, wc->hInstance}] = std::move(cls);
   // Atoms: any non-zero value; the client only tests for failure.
   static ATOM next_atom = 0xC000;
   return next_atom++;
}

ATOM WINAPI RegisterClassA(const WNDCLASSA *wc)
{
   if (wc == nullptr)
      return 0;
   WNDCLASSEXA ex = {sizeof(ex), wc->style, wc->lpfnWndProc, wc->cbClsExtra, wc->cbWndExtra,
                     wc->hInstance, wc->hIcon, wc->hCursor, wc->hbrBackground, wc->lpszMenuName,
                     wc->lpszClassName, nullptr};
   return RegisterClassCommon(&ex, false);
}

ATOM WINAPI RegisterClassExA(const WNDCLASSEXA *wc)
{
   return RegisterClassCommon(wc, false);
}

BOOL WINAPI UnregisterClassA(LPCSTR name, HINSTANCE instance)
{
   std::string key = Lower(ClassName(name).c_str());
   auto &classes = Classes();
   auto it = classes.find({key, instance});
   if (it == classes.end() || it->second->system)
   {
      SetLastError(ERROR_CLASS_DOES_NOT_EXIST);
      return FALSE;
   }
   bool used = false;
   Windows().ForEach([&](HWND, Window *w) { used = used || w->cls == it->second.get(); });
   if (used)
   {
      SetLastError(ERROR_CLASS_HAS_WINDOWS);
      return FALSE;
   }
   classes.erase(it);
   return TRUE;
}

/************************************************************************/
// Creating and destroying windows

HWND WINAPI CreateWindowExA(DWORD exstyle, LPCSTR class_name, LPCSTR title, DWORD style,
                            INT x, INT y, INT width, INT height, HWND parent, HMENU menu,
                            HINSTANCE instance, LPVOID param)
{
   if (!loop::IsGuiThread())
      return (HWND) OnGuiThread([&] {
         return (LRESULT) CreateWindowExA(exstyle, class_name, title, style, x, y, width, height, parent, menu,
                                          instance, param);
      });
   WindowClass *cls = FindClass(class_name, instance);
   if (cls == nullptr)
   {
      SetLastError(ERROR_CANNOT_FIND_WND_CLASS);
      return nullptr;
   }
   // Wine's animation control (ANIMATE_CLASSA) draws from a thread of its own
   // unless ACS_TIMER; windows and their DCs belong to the GUI thread here.
   if (strcasecmp(cls->name.c_str(), "SysAnimate32") == 0)
      style |= 0x0008;   // ACS_TIMER
   bool child = (style & WS_CHILD) != 0;
   if (child && GetWindowObject(parent) == nullptr)
   {
      SetLastError(ERROR_TLW_WITH_WSCHILD);
      return nullptr;
   }

   auto *w = new Window;
   w->cls = cls;
   w->proc = cls->proc;
   w->unicode = cls->unicode;
   w->style = style & ~WS_VISIBLE;   // visible once shown, after WM_CREATE, as on Windows
   w->exstyle = exstyle;
   w->instance = instance;
   w->text = (title && !IS_INTRESOURCE(title)) ? title : "";
   w->extra.assign(cls->window_extra, 0);
   if (child)
   {
      w->parent = parent;
      w->id = (UINT_PTR) menu;
   }
   else
   {
      // A top-level window's owner is the parent's top-level window.
      w->owner = parent ? TopLevelOf(parent) : nullptr;
      if (menu == nullptr && !cls->menu_name.empty())
         menu = LoadMenuA(instance, cls->menu_name[0] == '#' ? MAKEINTRESOURCEA(atoi(cls->menu_name.c_str() + 1))
                                                               : cls->menu_name.c_str());
      w->id = (UINT_PTR) menu;
      if (!(style & WS_POPUP))
         w->style |= WS_CLIPSIBLINGS | WS_CAPTION;   // overlapped windows always have a caption
   }

   // CW_USEDEFAULT: top-level windows go near the work area's top-left.
   RECT work = native::WorkArea();
   if (x == (INT) CW_USEDEFAULT)
      x = child ? 0 : work.left + 32, y = child ? 0 : work.top + 32;
   if (width == (INT) CW_USEDEFAULT)
      width = child ? 0 : (work.right - work.left) * 3 / 4, height = child ? 0 : (work.bottom - work.top) * 3 / 4;
   w->rect = {x, y, x + width, y + height};
   w->client = {0, 0, width, height};   // until WM_NCCALCSIZE, as on Windows (WM_NCCREATE sees it)

   HWND hwnd = Windows().Add(w);
   w->hwnd = hwnd;
   if (!child)
      AttachMenu(hwnd, (HMENU) w->id);
   if (child)
   {
      Window *p = GetWindowObject(parent);
      // New child windows go to the bottom of the Z order, as Windows links them:
      // creation order is the order of GW_HWNDNEXT, a dialog's tab order.
      p->children.push_back(hwnd);
   }

   CREATESTRUCTA cs = {param, instance, menu, parent, height, width, y, x, (LONG) (w->style | (style & WS_VISIBLE)),
                       title, class_name, exstyle};

   // WM_GETMINMAXINFO, WM_NCCREATE, WM_NCCALCSIZE, WM_CREATE, as Windows sends them.
   if (!child)
   {
      MINMAXINFO mmi = {};
      mmi.ptMaxTrackSize = {work.right - work.left, work.bottom - work.top};
      SendMessageA(hwnd, WM_GETMINMAXINFO, 0, (LPARAM) &mmi);
   }
   if (!SendMessageA(hwnd, WM_NCCREATE, 0, (LPARAM) &cs))
   {
      DestroyWindow(hwnd);
      return nullptr;
   }
   if (GetWindowObject(hwnd) == nullptr)
      return nullptr;
   ComputeClientArea(w);
   {
      NCCALCSIZE_PARAMS params = {};
      params.rgrc[0] = w->rect;
      SendMessageA(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM) &params.rgrc[0]);
   }

   if (!child)
   {
      Window *owner = GetWindowObject(w->owner);
      w->top = native::CreateTop(hwnd, TopStyleFor(w), w->rect.left, w->rect.top,
                                 width, height, owner ? owner->top : nullptr);
      native::SetTitle(w->top, w->text);
      if (cls->icon)
         native::SetIcon(w->top, cls->icon);
   }

   if (SendMessageA(hwnd, WM_CREATE, 0, (LPARAM) &cs) == -1)
   {
      DestroyWindow(hwnd);
      return nullptr;
   }
   if ((w = GetWindowObject(hwnd)) == nullptr)
      return nullptr;

   // DefWindowProc's WM_SIZE / WM_MOVE after creation.
   SendMessageA(hwnd, WM_SIZE, SIZE_RESTORED,
                MAKELPARAM(w->client.right - w->client.left, w->client.bottom - w->client.top));
   SendMessageA(hwnd, WM_MOVE, 0, MAKELPARAM(w->rect.left + w->client.left, w->rect.top + w->client.top));

   if (child && !(exstyle & WS_EX_NOPARENTNOTIFY))
      SendMessageA(parent, WM_PARENTNOTIFY, MAKEWPARAM(WM_CREATE, (WORD) w->id), (LPARAM) hwnd);

   if (style & WS_VISIBLE)
   {
      // Not shown yet, whatever WM_CREATE set (a control may set its style with
      // WS_VISIBLE): ShowWindow sees the change.
      w->style &= ~WS_VISIBLE;
      w->visible = false;
      ShowWindow(hwnd, (style & WS_MAXIMIZE) ? SW_SHOWMAXIMIZED : (style & WS_MINIMIZE) ? SW_SHOWMINIMIZED : SW_SHOW);
   }
   return hwnd;
}

static void DestroyTree(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return;
   // Children first, after their parent was told (WM_DESTROY goes parent first,
   // WM_NCDESTROY child first).
   std::vector<HWND> children = w->children;
   for (HWND c : children)
      DestroyTree(c);
   if ((w = GetWindowObject(hwnd)) == nullptr)
      return;
   SendMessageA(hwnd, WM_NCDESTROY, 0, 0);
   if ((w = GetWindowObject(hwnd)) == nullptr)
      return;
   Unlink(w);
   if (w->top)
      native::DestroyTop(w->top);
   if (w->update)
      DeleteObject(w->update);
   KillWindowTimers(hwnd);
   ForgetPaint(hwnd);
   CaretWindowDestroyed(hwnd);
   Windows().Remove(hwnd);
   delete w;
}

static void SendDestroy(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || w->destroying)
      return;
   w->destroying = true;
   SendMessageA(hwnd, WM_DESTROY, 0, 0);
   if ((w = GetWindowObject(hwnd)) == nullptr)
      return;
   std::vector<HWND> children = w->children;
   for (HWND c : children)
      SendDestroy(c);
}

BOOL WINAPI DestroyWindow(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return FALSE;
   }
   if (w->destroying)
      return TRUE;

   // Owned top-level windows go first.
   std::vector<HWND> owned;
   Windows().ForEach([&](HWND h, Window *o) { if (o->owner == hwnd) owned.push_back(h); });
   for (HWND h : owned)
      DestroyWindow(h);

   if ((w = GetWindowObject(hwnd)) == nullptr)
      return TRUE;
   if (IsChildWindow(w) && !(w->exstyle & WS_EX_NOPARENTNOTIFY) && w->parent)
      SendMessageA(w->parent, WM_PARENTNOTIFY, MAKEWPARAM(WM_DESTROY, (WORD) w->id), (LPARAM) hwnd);

   if (w->visible)
      ShowWindow(hwnd, SW_HIDE);

   // Focus, capture and activation leave the dying tree.
   for (HWND h = focus_window; h; h = GetParent(h))
      if (h == hwnd)
      {
         focus_window = nullptr;
         break;
      }
   if (capture_window && (capture_window == hwnd || IsChild(hwnd, capture_window)))
      ReleaseCapture();
   if (active_window == hwnd)
      active_window = nullptr;

   // A top-level window's menu goes with it.
   HMENU menu = (w->style & WS_CHILD) ? nullptr : (HMENU) w->id;
   SendDestroy(hwnd);
   DestroyTree(hwnd);
   if (menu)
      DestroyMenu(menu);
   return TRUE;
}

/************************************************************************/
// The tree

BOOL WINAPI IsWindow(HWND hwnd)
{
   return GetWindowObject(hwnd) != nullptr;
}

HWND WINAPI GetParent(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   // A child's parent (the desktop for its WS_CHILD windows); a popup's owner.
   if (w->style & WS_CHILD)
      return w->parent ? w->parent : GetDesktopWindow();
   return (w->style & WS_POPUP) ? w->owner : nullptr;
}

// A child window made a child of the desktop: a top-level window at the same
// screen position, shown on a surface of its own placed relative to its former
// top-level window's (which Wine's combo boxes do with their drop-down lists).
static HWND ToDesktop(Window *w)
{
   HWND hwnd = w->hwnd, old = w->parent;
   bool visible = w->visible;
   if (visible)
      ShowWindow(hwnd, SW_HIDE);
   RECT r;
   GetWindowRect(hwnd, &r);
   HWND top = TopLevelOf(old);
   Unlink(w);
   w->parent = nullptr;
   w->owner = top;
   w->rect = r;
   ComputeClientArea(w);
   Window *owner = GetWindowObject(top);
   w->top = native::CreateTop(hwnd, TopStyleFor(w), r.left, r.top, r.right - r.left, r.bottom - r.top,
                              owner ? owner->top : nullptr);
   if (visible)
      ShowWindow(hwnd, SW_SHOWNA);
   return old;
}

HWND WINAPI SetParent(HWND hwnd, HWND parent)
{
   Window *w = GetWindowObject(hwnd);
   if (w && IsChildWindow(w) && (parent == nullptr || parent == GetDesktopWindow()))
      return ToDesktop(w);
   Window *p = GetWindowObject(parent);
   if (w == nullptr || p == nullptr || !IsChildWindow(w))
      return nullptr;
   HWND old = w->parent;
   Unlink(w);
   w->parent = parent;
   p->children.insert(p->children.begin(), hwnd);
   InvalidateWhole(parent, true);
   return old;
}

BOOL WINAPI IsChild(HWND parent, HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   while (w != nullptr && IsChildWindow(w))
   {
      if (w->parent == parent)
         return TRUE;
      w = GetWindowObject(w->parent);
   }
   return FALSE;
}

HWND WINAPI GetWindow(HWND hwnd, UINT cmd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   if (cmd == GW_CHILD)
      return w->children.empty() ? nullptr : w->children.front();
   if (cmd == GW_OWNER)
      return w->owner;
   std::vector<HWND> siblings;
   if (Window *p = GetWindowObject(w->parent); p && IsChildWindow(w))
      siblings = p->children;
   else
      Windows().ForEach([&](HWND h, Window *o) { if (!IsChildWindow(o)) siblings.push_back(h); });
   if (siblings.empty())
      return nullptr;
   auto it = std::find(siblings.begin(), siblings.end(), hwnd);
   switch (cmd)
   {
   case GW_HWNDFIRST: return siblings.front();
   case GW_HWNDLAST: return siblings.back();
   case GW_HWNDNEXT: return (it == siblings.end() || it + 1 == siblings.end()) ? nullptr : *(it + 1);
   case GW_HWNDPREV: return (it == siblings.end() || it == siblings.begin()) ? nullptr : *(it - 1);
   }
   return nullptr;
}

HWND WINAPI GetDesktopWindow(void)
{
   // A window of its own, so the code that centers dialogs on it gets the screen.
   static HWND desktop = nullptr;
   if (desktop == nullptr)
   {
      auto *w = new Window;
      static WindowClass cls;
      cls.name = "#32769";
      cls.proc = DefWindowProcA;
      w->cls = &cls;
      w->proc = DefWindowProcA;
      w->style = WS_VISIBLE | WS_POPUP;
      w->visible = true;
      w->rect = native::ScreenRect();
      w->client = {0, 0, w->rect.right - w->rect.left, w->rect.bottom - w->rect.top};
      desktop = Windows().Add(w);
      w->hwnd = desktop;
   }
   return desktop;
}

BOOL WINAPI EnumChildWindows(HWND parent, WNDENUMPROC proc, LPARAM lparam)
{
   Window *w = GetWindowObject(parent);
   if (w == nullptr)
      return FALSE;
   // All descendants, depth first; a snapshot, as windows may be made or destroyed.
   std::vector<HWND> all;
   std::vector<HWND> stack(w->children.rbegin(), w->children.rend());
   while (!stack.empty())
   {
      HWND h = stack.back();
      stack.pop_back();
      all.push_back(h);
      if (Window *c = GetWindowObject(h))
         stack.insert(stack.end(), c->children.rbegin(), c->children.rend());
   }
   for (HWND h : all)
      if (IsWindow(h) && !proc(h, lparam))
         break;
   return TRUE;
}

INT WINAPI GetDlgCtrlID(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return (w && IsChildWindow(w)) ? (INT) w->id : 0;
}

HWND WINAPI GetDlgItem(HWND dialog, INT id)
{
   Window *w = GetWindowObject(dialog);
   if (w == nullptr)
      return nullptr;
   for (HWND c : w->children)
      if (Window *cw = GetWindowObject(c); cw && (INT) (WORD) cw->id == (INT) (WORD) id)
         return c;
   return nullptr;
}

/************************************************************************/
// Geometry

BOOL WINAPI GetWindowRect(HWND hwnd, RECT *rect)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || rect == nullptr)
      return FALSE;
   *rect = w->rect;
   if (IsChildWindow(w))
   {
      POINT p = {0, 0};
      ClientToScreen(w->parent, &p);
      OffsetRect(rect, p.x, p.y);
   }
   return TRUE;
}

BOOL WINAPI GetClientRect(HWND hwnd, RECT *rect)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || rect == nullptr)
      return FALSE;
   *rect = {0, 0, w->client.right - w->client.left, w->client.bottom - w->client.top};
   return TRUE;
}

// Client area's top-left in screen coordinates.
static POINT ClientOrigin(Window *w)
{
   POINT p = {w->rect.left + w->client.left, w->rect.top + w->client.top};
   while (IsChildWindow(w) && w->parent)
   {
      w = GetWindowObject(w->parent);
      if (w == nullptr)
         break;
      p.x += w->rect.left + w->client.left;
      p.y += w->rect.top + w->client.top;
   }
   return p;
}

BOOL WINAPI ClientToScreen(HWND hwnd, POINT *p)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || p == nullptr)
      return FALSE;
   POINT o = ClientOrigin(w);
   p->x += o.x;
   p->y += o.y;
   return TRUE;
}

BOOL WINAPI ScreenToClient(HWND hwnd, POINT *p)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || p == nullptr)
      return FALSE;
   POINT o = ClientOrigin(w);
   p->x -= o.x;
   p->y -= o.y;
   return TRUE;
}

INT WINAPI MapWindowPoints(HWND from, HWND to, POINT *points, UINT count)
{
   POINT a = {0, 0}, b = {0, 0};
   if (from)
      ClientToScreen(from, &a);
   if (to)
      ClientToScreen(to, &b);
   for (UINT i = 0; i < count; i++)
   {
      points[i].x += a.x - b.x;
      points[i].y += a.y - b.y;
   }
   return MAKELONG(a.x - b.x, a.y - b.y);
}

BOOL WINAPI SetWindowPos(HWND hwnd, HWND after, INT x, INT y, INT cx, INT cy, UINT flags)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;

   WINDOWPOS pos = {hwnd, after, x, y, cx, cy, flags};
   if (flags & SWP_NOMOVE)
      pos.x = w->rect.left, pos.y = w->rect.top;
   if (flags & SWP_NOSIZE)
      pos.cx = w->rect.right - w->rect.left, pos.cy = w->rect.bottom - w->rect.top;
   SendMessageA(hwnd, WM_WINDOWPOSCHANGING, 0, (LPARAM) &pos);
   if ((w = GetWindowObject(hwnd)) == nullptr)
      return FALSE;

   RECT old = w->rect;
   RECT old_client = w->client;
   bool moved = pos.x != old.left || pos.y != old.top;
   bool sized = pos.cx != old.right - old.left || pos.cy != old.bottom - old.top;

   // Z order among siblings.
   if (!(flags & SWP_NOZORDER) && IsChildWindow(w))
      if (Window *p = GetWindowObject(w->parent))
      {
         auto &c = p->children;
         c.erase(std::remove(c.begin(), c.end(), hwnd), c.end());
         if (after == HWND_TOP || after == HWND_TOPMOST || after == nullptr)
            c.insert(c.begin(), hwnd);
         else if (after == HWND_BOTTOM || after == HWND_NOTOPMOST)
            c.push_back(hwnd);
         else
         {
            auto it = std::find(c.begin(), c.end(), after);
            c.insert(it == c.end() ? c.end() : it + 1, hwnd);
         }
      }

   w->rect = {pos.x, pos.y, pos.x + pos.cx, pos.y + pos.cy};
   if (sized || (flags & SWP_FRAMECHANGED))
      ComputeClientArea(w);

   if (!IsChildWindow(w) && w->top && (moved || sized))
      native::SetGeometry(w->top, pos.x, pos.y, pos.cx, pos.cy);

   if (flags & SWP_SHOWWINDOW)
      ShowWindow(hwnd, SW_SHOWNA);
   else if (flags & SWP_HIDEWINDOW)
      ShowWindow(hwnd, SW_HIDE);
   if ((w = GetWindowObject(hwnd)) == nullptr)
      return TRUE;

   if (IsChildWindow(w) && w->visible && (moved || sized || (flags & SWP_FRAMECHANGED)) && !(flags & SWP_NOREDRAW))
   {
      // The parent repaints where the window was and where it is.
      RECT both;
      UnionRect(&both, &old, &w->rect);
      RedrawWindow(w->parent, &both, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
   }
   if ((sized || (flags & SWP_FRAMECHANGED)) && !(flags & SWP_NOREDRAW))
      InvalidateWhole(hwnd, true);

   // A frame change (scroll bars shown) resizes the client area: WM_SIZE comes.
   bool client_sized = w->client.right - w->client.left != old_client.right - old_client.left ||
                       w->client.bottom - w->client.top != old_client.bottom - old_client.top;
   pos.flags = (flags & ~SWP_NOSIZE) | (moved ? 0 : SWP_NOMOVE) | (sized || client_sized ? 0 : SWP_NOSIZE);
   SendMessageA(hwnd, WM_WINDOWPOSCHANGED, 0, (LPARAM) &pos);
   return TRUE;
}

BOOL WINAPI MoveWindow(HWND hwnd, INT x, INT y, INT cx, INT cy, BOOL repaint)
{
   return SetWindowPos(hwnd, nullptr, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE | (repaint ? 0 : SWP_NOREDRAW));
}

BOOL WINAPI GetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *wp)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || wp == nullptr)
      return FALSE;
   wp->flags = 0;
   wp->showCmd = w->minimized ? SW_SHOWMINIMIZED : w->maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
   wp->ptMinPosition = {-1, -1};
   wp->ptMaxPosition = {-1, -1};
   wp->rcNormalPosition = w->rect;
   return TRUE;
}

BOOL WINAPI SetWindowPlacement(HWND hwnd, const WINDOWPLACEMENT *wp)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || wp == nullptr)
      return FALSE;
   const RECT &r = wp->rcNormalPosition;
   SetWindowPos(hwnd, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
   ShowWindow(hwnd, wp->showCmd);
   return TRUE;
}

BOOL WINAPI AdjustWindowRect(RECT *rect, DWORD style, BOOL menu)
{
   return AdjustWindowRectEx(rect, style, menu, 0);
}

BOOL WINAPI AdjustWindowRectEx(RECT *rect, DWORD style, BOOL menu, DWORD exstyle)
{
   // Top-level windows' frames are the toolkit's, outside their rect; only child
   // windows have non-client areas of ours.
   if (rect == nullptr)
      return FALSE;
   if ((style & WS_CHILD) || ((style & WS_POPUP) && (style & WS_CAPTION) != WS_CAPTION && !(style & WS_THICKFRAME)))
   {
      RECT in = NonClientInsets(style, exstyle);
      rect->left -= in.left;
      rect->top -= in.top;
      rect->right += in.right;
      rect->bottom += in.bottom;
   }
   else if (menu)
      rect->top -= GetSystemMetrics(SM_CYMENU) + 1;
   return TRUE;
}

/************************************************************************/
// Showing and enabling

BOOL WINAPI ShowWindow(HWND hwnd, INT cmd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   bool was_visible = w->visible;
   bool show = cmd != SW_HIDE;

   if (show != was_visible)
   {
      SendMessageA(hwnd, WM_SHOWWINDOW, show, 0);
      if ((w = GetWindowObject(hwnd)) == nullptr)
         return was_visible;
      w->visible = show;
      if (show)
         w->style |= WS_VISIBLE;
      else
         w->style &= ~WS_VISIBLE;
      if (w->top)
         native::Show(w->top, show);
      if (IsChildWindow(w))
         RedrawWindow(w->parent, &w->rect, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
      if (show)
         InvalidateWhole(hwnd, true);
      ChildVisibilityChanged(hwnd);
      // As Windows' SetWindowPos: a shown window erases now, a hidden child's
      // parent too.
      if (show)
         EraseNow(hwnd);
      else if ((w = GetWindowObject(hwnd)) != nullptr && IsChildWindow(w))
         EraseNow(w->parent);
      // As Wine's: a hidden child hands the focus in it to its parent.
      if (!show && (w = GetWindowObject(hwnd)) != nullptr && IsChildWindow(w) && focus_window &&
          (focus_window == hwnd || IsChild(hwnd, focus_window)))
         SetFocus(w->parent);
      if ((w = GetWindowObject(hwnd)) == nullptr)
         return was_visible;
   }

   if (w->top)
   {
      if (cmd == SW_SHOWMINIMIZED || cmd == SW_MINIMIZE || cmd == SW_SHOWMINNOACTIVE)
         native::SetMinimized(w->top, true), w->minimized = true;
      else if (cmd == SW_SHOWMAXIMIZED)
         native::SetMaximized(w->top, true), w->maximized = true;
      else if (cmd == SW_RESTORE)
      {
         native::SetMinimized(w->top, false);
         native::SetMaximized(w->top, false);
         w->minimized = w->maximized = false;
      }
      if (show && cmd != SW_SHOWNA && cmd != SW_SHOWNOACTIVATE && cmd != SW_SHOWMINNOACTIVE)
         native::Activate(w->top);
   }
   return was_visible;
}

BOOL WINAPI IsWindowVisible(HWND hwnd)
{
   for (Window *w = GetWindowObject(hwnd); w; w = IsChildWindow(w) ? GetWindowObject(w->parent) : nullptr)
   {
      if (!w->visible)
         return FALSE;
      if (!IsChildWindow(w))
         return TRUE;
   }
   return FALSE;
}

BOOL WINAPI IsIconic(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return w && w->minimized;
}

BOOL WINAPI IsZoomed(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return w && w->maximized;
}

BOOL WINAPI EnableWindow(HWND hwnd, BOOL enable)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   bool was_disabled = !w->enabled;
   if (w->enabled != (enable != FALSE))
   {
      w->enabled = enable != FALSE;
      if (enable)
         w->style &= ~WS_DISABLED;
      else
         w->style |= WS_DISABLED;
      if (!enable && (focus_window == hwnd || IsChild(hwnd, focus_window)))
         SetFocus(nullptr);
      SendMessageA(hwnd, WM_ENABLE, enable, 0);
      ChildEnabledChanged(hwnd);
   }
   return was_disabled;
}

BOOL WINAPI IsWindowEnabled(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return w && w->enabled;
}

/************************************************************************/
// Text

BOOL WINAPI SetWindowTextA(HWND hwnd, LPCSTR text)
{
   if (GetWindowObject(hwnd) == nullptr)
      return FALSE;
   return (BOOL) SendMessageA(hwnd, WM_SETTEXT, 0, (LPARAM) text);
}

INT WINAPI GetWindowTextA(HWND hwnd, LPSTR text, INT size)
{
   if (GetWindowObject(hwnd) == nullptr || text == nullptr || size <= 0)
      return 0;
   text[0] = 0;
   return (INT) SendMessageA(hwnd, WM_GETTEXT, size, (LPARAM) text);
}

INT WINAPI GetWindowTextLengthA(HWND hwnd)
{
   if (GetWindowObject(hwnd) == nullptr)
      return 0;
   return (INT) SendMessageA(hwnd, WM_GETTEXTLENGTH, 0, 0);
}

/************************************************************************/
// Window longs

static LONG_PTR GetLong(HWND hwnd, INT index, bool ptr, bool unicode = false)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return 0;
   }
   switch (index)
   {
   case GWLP_WNDPROC: return (LONG_PTR) ProcValue({w->proc, w->unicode}, unicode);
   case GWLP_HINSTANCE: return (LONG_PTR) w->instance;
   case GWLP_HWNDPARENT: return (LONG_PTR) (IsChildWindow(w) ? w->parent : w->owner);
   case GWLP_ID: return (LONG_PTR) w->id;
   case GWLP_USERDATA: return w->user_data;
   case GWL_STYLE: return (LONG) w->style;
   case GWL_EXSTYLE: return (LONG) w->exstyle;
   }
   size_t size = ptr ? sizeof(LONG_PTR) : sizeof(LONG);
   if (index < 0 || (size_t) index + size > w->extra.size())
   {
      SetLastError(ERROR_INVALID_INDEX);
      return 0;
   }
   if (ptr)
   {
      LONG_PTR v;
      memcpy(&v, &w->extra[index], sizeof(v));
      return v;
   }
   LONG v;
   memcpy(&v, &w->extra[index], sizeof(v));
   return v;
}

static LONG_PTR SetLong(HWND hwnd, INT index, LONG_PTR value, bool ptr, bool unicode = false)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return 0;
   }
   LONG_PTR old;
   switch (index)
   {
   case GWLP_WNDPROC:
   {
      old = (LONG_PTR) ProcValue({w->proc, w->unicode}, unicode);
      ProcRef p = ProcFromValue((WNDPROC) value, unicode);
      w->proc = p.fn;
      w->unicode = p.unicode;
      return old;
   }
   case GWLP_HINSTANCE:
      old = (LONG_PTR) w->instance;
      w->instance = (HINSTANCE) value;
      return old;
   case GWLP_ID:
      old = (LONG_PTR) w->id;
      w->id = (UINT_PTR) value;
      return old;
   case GWLP_USERDATA:
      old = w->user_data;
      w->user_data = value;
      return old;
   case GWL_STYLE:
   {
      old = (LONG) w->style;
      STYLESTRUCT ss = {w->style, (DWORD) value};
      SendMessageA(hwnd, WM_STYLECHANGING, GWL_STYLE, (LPARAM) &ss);
      if ((w = GetWindowObject(hwnd)) == nullptr)
         return old;
      w->style = ss.styleNew;
      w->visible = (w->style & WS_VISIBLE) != 0;
      w->enabled = !(w->style & WS_DISABLED);
      SendMessageA(hwnd, WM_STYLECHANGED, GWL_STYLE, (LPARAM) &ss);
      return old;
   }
   case GWL_EXSTYLE:
   {
      old = (LONG) w->exstyle;
      STYLESTRUCT ss = {w->exstyle, (DWORD) value};
      SendMessageA(hwnd, WM_STYLECHANGING, GWL_EXSTYLE, (LPARAM) &ss);
      if ((w = GetWindowObject(hwnd)) == nullptr)
         return old;
      w->exstyle = ss.styleNew;
      SendMessageA(hwnd, WM_STYLECHANGED, GWL_EXSTYLE, (LPARAM) &ss);
      return old;
   }
   }
   size_t size = ptr ? sizeof(LONG_PTR) : sizeof(LONG);
   if (index < 0 || (size_t) index + size > w->extra.size())
   {
      SetLastError(ERROR_INVALID_INDEX);
      return 0;
   }
   old = GetLong(hwnd, index, ptr);
   if (ptr)
      memcpy(&w->extra[index], &value, sizeof(value));
   else
   {
      LONG v = (LONG) value;
      memcpy(&w->extra[index], &v, sizeof(v));
   }
   return old;
}

LONG WINAPI GetWindowLongA(HWND hwnd, INT index)
{
   return (LONG) GetLong(hwnd, index, false);
}

LONG WINAPI SetWindowLongA(HWND hwnd, INT index, LONG value)
{
   return (LONG) SetLong(hwnd, index, value, false);
}

LONG_PTR WINAPI GetWindowLongPtrA(HWND hwnd, INT index)
{
   return GetLong(hwnd, index, true);
}

LONG_PTR WINAPI SetWindowLongPtrA(HWND hwnd, INT index, LONG_PTR value)
{
   return SetLong(hwnd, index, value, true);
}

DWORD WINAPI GetClassLongA(HWND hwnd, INT index)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return 0;
   switch (index)
   {
   case GCL_STYLE: return w->cls->style;
   case GCL_CBWNDEXTRA: return w->cls->window_extra;
   case GCL_CBCLSEXTRA: return w->cls->class_extra;
   }
   return 0;
}

ULONG_PTR WINAPI GetClassLongPtrA(HWND hwnd, INT index)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return 0;
   switch (index)
   {
   case GCLP_HCURSOR: return (ULONG_PTR) w->cls->cursor;
   case GCLP_HICON: return (ULONG_PTR) w->cls->icon;
   case GCLP_HICONSM: return (ULONG_PTR) w->cls->icon_small;
   case GCLP_HBRBACKGROUND: return (ULONG_PTR) w->cls->background;
   case GCLP_WNDPROC: return (ULONG_PTR) ProcValue({w->cls->proc, w->cls->unicode}, false);
   case GCLP_HMODULE: return (ULONG_PTR) w->cls->instance;
   }
   return GetClassLongA(hwnd, index);
}

ULONG_PTR WINAPI SetClassLongPtrA(HWND hwnd, INT index, LONG_PTR value)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return 0;
   ULONG_PTR old = GetClassLongPtrA(hwnd, index);
   switch (index)
   {
   case GCLP_HCURSOR: w->cls->cursor = (HCURSOR) value; break;
   case GCLP_HICON: w->cls->icon = (HICON) value; break;
   case GCLP_HICONSM: w->cls->icon_small = (HICON) value; break;
   case GCLP_HBRBACKGROUND: w->cls->background = (HBRUSH) value; break;
   }
   return old;
}

INT WINAPI GetClassNameA(HWND hwnd, LPSTR buffer, INT size)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || buffer == nullptr || size <= 0)
      return 0;
   lstrcpynA(buffer, w->cls->name.c_str(), size);
   return (INT) strlen(buffer);
}

/************************************************************************/
// Properties

BOOL WINAPI SetPropA(HWND hwnd, LPCSTR name, HANDLE value)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   w->props[Lower(ClassName(name).c_str())] = value;
   return TRUE;
}

HANDLE WINAPI GetPropA(HWND hwnd, LPCSTR name)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   auto it = w->props.find(Lower(ClassName(name).c_str()));
   return it == w->props.end() ? nullptr : it->second;
}

HANDLE WINAPI RemovePropA(HWND hwnd, LPCSTR name)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   auto it = w->props.find(Lower(ClassName(name).c_str()));
   if (it == w->props.end())
      return nullptr;
   HANDLE value = it->second;
   w->props.erase(it);
   return value;
}

/************************************************************************/
// Window procedures

LRESULT WINAPI CallWindowProcA(WNDPROC proc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return CallProcRef(ProcFromValue(proc, false), hwnd, msg, wparam, lparam, false);
}

/************************************************************************/
// Focus, capture and activation

HWND WINAPI GetFocus(void)
{
   return GetWindowObject(focus_window) ? focus_window : nullptr;
}

HWND WINAPI SetFocus(HWND hwnd)
{
   HWND old = GetFocus();
   if (hwnd == old)
      return old;
   if (hwnd != nullptr)
   {
      Window *w = GetWindowObject(hwnd);
      if (w == nullptr)
         return nullptr;
      // Focus goes to a top-level window's tree: activate it first.
      HWND top = TopLevelOf(hwnd);
      if (Window *t = GetWindowObject(top); t && t->top && t->visible && !native::IsActive(t->top))
         native::Activate(t->top);
   }
   // As Windows: the focus moves first, so the window losing it sees the new
   // one in GetFocus() (the client redraws its focus borders by it); a
   // WM_KILLFOCUS handler that moves it again cancels the WM_SETFOCUS.
   focus_window = hwnd;
   if (old)
   {
      SendMessageA(old, WM_KILLFOCUS, (WPARAM) hwnd, 0);
      if (GetFocus() != hwnd)
         return old;
   }
   if (hwnd && IsWindow(hwnd))
   {
      FocusChanged(hwnd);
      SendMessageA(hwnd, WM_SETFOCUS, (WPARAM) old, 0);
   }
   return old;
}

HWND WINAPI GetCapture(void)
{
   return GetWindowObject(capture_window) ? capture_window : nullptr;
}

HWND WINAPI SetCapture(HWND hwnd)
{
   HWND old = GetCapture();
   capture_window = hwnd;
   if (Window *t = GetWindowObject(TopLevelOf(hwnd)); t && t->top)
      native::CaptureMouse(t->top, true);
   if (old && old != hwnd)
      SendMessageA(old, WM_CAPTURECHANGED, 0, (LPARAM) hwnd);
   return old;
}

BOOL WINAPI ReleaseCapture(void)
{
   HWND old = GetCapture();
   capture_window = nullptr;
   if (old)
   {
      if (Window *t = GetWindowObject(TopLevelOf(old)); t && t->top)
         native::CaptureMouse(t->top, false);
      SendMessageA(old, WM_CAPTURECHANGED, 0, 0);
   }
   return TRUE;
}

HWND WINAPI GetActiveWindow(void)
{
   return GetWindowObject(active_window) ? active_window : nullptr;
}

HWND WINAPI GetForegroundWindow(void)
{
   return GetActiveWindow();
}

HWND WINAPI SetActiveWindow(HWND hwnd)
{
   HWND old = GetActiveWindow();
   if (Window *w = GetWindowObject(TopLevelOf(hwnd)); w && w->top)
      native::Activate(w->top);
   return old;
}

BOOL WINAPI SetForegroundWindow(HWND hwnd)
{
   SetActiveWindow(hwnd);
   return TRUE;
}

HWND WINAPI WindowFromPoint(POINT p)
{
   // The deepest visible window under the screen point, among our top-level windows.
   HWND found = nullptr;
   Windows().ForEach([&](HWND h, Window *w) {
      if (found || IsChildWindow(w) || !w->visible || w->top == nullptr)
         return;
      if (PtInRect(&w->rect, p))
         found = h;
   });
   if (found == nullptr)
      return nullptr;
   for (;;)
   {
      POINT c = p;
      ScreenToClient(found, &c);
      HWND child = ChildWindowFromPointEx(found, c, CWP_SKIPINVISIBLE);
      if (child == nullptr || child == found)
         return found;
      found = child;
   }
}

HWND WINAPI ChildWindowFromPointEx(HWND parent, POINT p, UINT flags)
{
   Window *w = GetWindowObject(parent);
   if (w == nullptr)
      return nullptr;
   RECT client = {0, 0, w->client.right - w->client.left, w->client.bottom - w->client.top};
   if (!PtInRect(&client, p))
      return nullptr;
   for (HWND c : w->children)
   {
      Window *cw = GetWindowObject(c);
      if (cw == nullptr)
         continue;
      if ((flags & CWP_SKIPINVISIBLE) && !cw->visible)
         continue;
      if ((flags & CWP_SKIPDISABLED) && !cw->enabled)
         continue;
      if (PtInRect(&cw->rect, p))
         return c;
   }
   return parent;
}

HWND WINAPI ChildWindowFromPoint(HWND parent, POINT p)
{
   return ChildWindowFromPointEx(parent, p, CWP_ALL);
}

namespace w32 {

void ForEachTopLevel(void (*f)(HWND, void *), void *data)
{
   std::vector<HWND> tops;
   Windows().ForEach([&](HWND h, Window *w) {
      if (!IsChildWindow(w) && w->top)
         tops.push_back(h);
   });
   for (HWND h : tops)
      if (IsWindow(h))
         f(h, data);
}

}  // namespace w32

/************************************************************************/
// Unicode versions: text goes through the ANSI ones, procedures keep their set.

ATOM WINAPI RegisterClassExW(const WNDCLASSEXW *wc)
{
   if (wc == nullptr)
      return 0;
   AnsiArg name(wc->lpszClassName), menu(wc->lpszMenuName);
   WNDCLASSEXA ex = {sizeof(ex), wc->style, wc->lpfnWndProc, wc->cbClsExtra, wc->cbWndExtra,
                     wc->hInstance, wc->hIcon, wc->hCursor, wc->hbrBackground,
                     wc->lpszMenuName ? menu.get() : nullptr, name, wc->hIconSm};
   return RegisterClassCommon(&ex, true);
}

ATOM WINAPI RegisterClassW(const WNDCLASSW *wc)
{
   if (wc == nullptr)
      return 0;
   WNDCLASSEXW ex = {sizeof(ex), wc->style, wc->lpfnWndProc, wc->cbClsExtra, wc->cbWndExtra,
                     wc->hInstance, wc->hIcon, wc->hCursor, wc->hbrBackground, wc->lpszMenuName,
                     wc->lpszClassName, nullptr};
   return RegisterClassExW(&ex);
}

BOOL WINAPI UnregisterClassW(LPCWSTR name, HINSTANCE instance)
{
   return UnregisterClassA(AnsiArg(name), instance);
}

HWND WINAPI CreateWindowExW(DWORD exstyle, LPCWSTR class_name, LPCWSTR title, DWORD style, INT x, INT y,
                            INT width, INT height, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param)
{
   // A Unicode class's procedure gets WM_NCCREATE/WM_CREATE with Unicode text by
   // the translation.
   return CreateWindowExA(exstyle, AnsiArg(class_name), title ? AnsiArg(title).get() : nullptr, style, x, y,
                          width, height, parent, menu, instance, param);
}

BOOL WINAPI IsWindowUnicode(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   return w && w->unicode;
}

LONG WINAPI GetWindowLongW(HWND hwnd, INT index)
{
   return (LONG) GetLong(hwnd, index, false, true);
}

LONG WINAPI SetWindowLongW(HWND hwnd, INT index, LONG value)
{
   return (LONG) SetLong(hwnd, index, value, false, true);
}

LONG_PTR WINAPI GetWindowLongPtrW(HWND hwnd, INT index)
{
   return GetLong(hwnd, index, true, true);
}

LONG_PTR WINAPI SetWindowLongPtrW(HWND hwnd, INT index, LONG_PTR value)
{
   return SetLong(hwnd, index, value, true, true);
}

DWORD WINAPI GetClassLongW(HWND hwnd, INT index)
{
   return GetClassLongA(hwnd, index);
}

ULONG_PTR WINAPI GetClassLongPtrW(HWND hwnd, INT index)
{
   Window *w = GetWindowObject(hwnd);
   if (w && index == GCLP_WNDPROC)
      return (ULONG_PTR) ProcValue({w->cls->proc, w->cls->unicode}, true);
   return GetClassLongPtrA(hwnd, index);
}

INT WINAPI GetClassNameW(HWND hwnd, LPWSTR buffer, INT size)
{
   if (buffer == nullptr || size <= 0)
      return 0;
   char name[256];
   int n = GetClassNameA(hwnd, name, sizeof(name));
   std::basic_string<WCHAR> w = AnsiToWide(name, n);
   int copy = std::min((int) w.size(), size - 1);
   memcpy(buffer, w.data(), copy * sizeof(WCHAR));
   buffer[copy] = 0;
   return copy;
}

BOOL WINAPI SetPropW(HWND hwnd, LPCWSTR name, HANDLE value)
{
   return SetPropA(hwnd, AnsiArg(name), value);
}

HANDLE WINAPI GetPropW(HWND hwnd, LPCWSTR name)
{
   return GetPropA(hwnd, AnsiArg(name));
}

HANDLE WINAPI RemovePropW(HWND hwnd, LPCWSTR name)
{
   return RemovePropA(hwnd, AnsiArg(name));
}

BOOL WINAPI SetWindowTextW(HWND hwnd, LPCWSTR text)
{
   if (GetWindowObject(hwnd) == nullptr)
      return FALSE;
   return (BOOL) SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM) text);
}

INT WINAPI GetWindowTextW(HWND hwnd, LPWSTR text, INT size)
{
   if (GetWindowObject(hwnd) == nullptr || text == nullptr || size <= 0)
      return 0;
   text[0] = 0;
   return (INT) SendMessageW(hwnd, WM_GETTEXT, size, (LPARAM) text);
}

INT WINAPI GetWindowTextLengthW(HWND hwnd)
{
   return (INT) SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
}

LRESULT WINAPI CallWindowProcW(WNDPROC proc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return CallProcRef(ProcFromValue(proc, true), hwnd, msg, wparam, lparam, true);
}
