/*
 * events.cpp:  Native input and window events, as the messages Windows would
 *   send or post: mouse messages to the window under the pointer (or with the
 *   capture), keys to the focus window, activation, sizing and closing.
 */

#include "core/native_events.h"

#include "core/menu.h"
#include "core/message.h"
#include "core/native.h"
#include "core/window.h"

#include <cstdlib>
#include <map>
#include <set>

namespace w32 {

namespace {

std::set<HWND> &LeaveTracked()
{
   static std::set<HWND> tracked;
   return tracked;
}

HWND last_mouse_window = nullptr;

// Double clicks: a second press of the same button, soon and near the first.
struct LastPress
{
   UINT msg = 0;
   DWORD time = 0;
   int x = 0, y = 0;
   HWND hwnd = nullptr;
};
LastPress last_press;

// The focus each top-level window had when it was deactivated.
std::map<HWND, HWND> &SavedFocus()
{
   static std::map<HWND, HWND> saved;
   return saved;
}

bool app_active = false;

// The deepest visible, enabled child of top at (x, y) (top's client coordinates),
// and (x, y) in its client coordinates.
HWND HitTest(HWND top, int *x, int *y)
{
   HWND hwnd = top;
   for (;;)
   {
      Window *w = GetWindowObject(hwnd);
      if (w == nullptr)
         return nullptr;
      HWND child = nullptr;
      for (HWND c : w->children)
      {
         Window *cw = GetWindowObject(c);
         if (cw == nullptr || !cw->visible)
            continue;
         POINT p = {*x, *y};
         if (PtInRect(&cw->rect, p))
         {
            // Windows that say HTTRANSPARENT (statics without SS_NOTIFY) let the
            // mouse through to what is under them.
            POINT screen = p;
            ClientToScreen(hwnd, &screen);
            if (cw->enabled && SendMessageA(c, WM_NCHITTEST, 0, MAKELPARAM(screen.x, screen.y)) == HTTRANSPARENT)
               continue;
            child = c;
            break;
         }
      }
      if (child == nullptr)
         return hwnd;
      Window *cw = GetWindowObject(child);
      *x -= cw->rect.left + cw->client.left;
      *y -= cw->rect.top + cw->client.top;
      if (!cw->enabled)
         return hwnd;   // input to a disabled window is the parent's to ignore
      hwnd = child;
   }
}

}  // namespace

void TrackLeave(HWND hwnd, bool track)
{
   if (track)
      LeaveTracked().insert(hwnd);
   else
      LeaveTracked().erase(hwnd);
}

}  // namespace w32

namespace w32::events {

void Mouse(HWND top, UINT msg, int x, int y, UINT keys, int wheel)
{
   Window *t = GetWindowObject(top);
   if (t == nullptr)
      return;

   if (msg == WM_MOUSEWHEEL)
   {
      // The wheel goes to the focus window, with screen coordinates.
      HWND target = GetFocus();
      if (target == nullptr || TopLevelOf(target) != top)
         target = top;
      POINT p = {x - t->client.left, y - t->client.top};
      ClientToScreen(top, &p);
      PostInput(target, WM_MOUSEWHEEL, MAKEWPARAM(keys, (SHORT) wheel), MAKELPARAM(p.x, p.y));
      return;
   }

   // (x, y) are the surface's; the client area is below the menu bar.
   if (GetCapture() == nullptr && MenuBarMouse(t, msg, x, y))
      return;
   x -= t->client.left;
   y -= t->client.top;

   HWND target;
   int cx = x, cy = y;
   bool captured = false;
   if (HWND capture = GetCapture())
   {
      // The capture takes the mouse wherever it is, in its client coordinates,
      // across top-level windows (a drop-down list's popup has it).
      target = capture;
      captured = true;
      if (TopLevelOf(capture) == top)
      {
         // In surface coordinates, as (x, y) were before the menu bar came off.
         RECT c = ClientRectInTop(capture);
         cx = x + t->client.left - c.left;
         cy = y + t->client.top - c.top;
      }
      else
      {
         POINT p = {x, y};
         ClientToScreen(top, &p);
         ScreenToClient(capture, &p);
         cx = p.x;
         cy = p.y;
      }
   }
   else
      target = HitTest(top, &cx, &cy);
   if (target == nullptr)
      return;

   // Over the target's non-client area (borders, scroll bars): the NC messages,
   // with screen coordinates and the hit-test code.
   if (!captured && msg != WM_MOUSEWHEEL)
   {
      Window *tw = GetWindowObject(target);
      RECT client = {0, 0, tw->client.right - tw->client.left, tw->client.bottom - tw->client.top};
      POINT pt = {cx, cy};
      if (!PtInRect(&client, pt))
      {
         ClientToScreen(target, &pt);
         LPARAM screen = MAKELPARAM(pt.x, pt.y);
         LRESULT hit = SendMessageA(target, WM_NCHITTEST, 0, screen);
         if (hit != HTCLIENT)
         {
            if (last_mouse_window && last_mouse_window != target && LeaveTracked().count(last_mouse_window))
            {
               LeaveTracked().erase(last_mouse_window);
               PostInput(last_mouse_window, WM_MOUSELEAVE, 0, 0);
            }
            last_mouse_window = target;
            if (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
               SendMessageA(target, WM_SETCURSOR, (WPARAM) target, MAKELPARAM((WORD) hit, msg));
            if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
            {
               if (!native::IsActive(t->top))
                  native::Activate(t->top);
               last_press = LastPress();
            }
            PostInput(target, msg - WM_MOUSEFIRST + WM_NCMOUSEMOVE, (WPARAM) hit, screen);
            return;
         }
      }
   }

   // Leaving a tracked window.
   if (last_mouse_window && last_mouse_window != target && LeaveTracked().count(last_mouse_window))
   {
      LeaveTracked().erase(last_mouse_window);
      PostInput(last_mouse_window, WM_MOUSELEAVE, 0, 0);
   }
   last_mouse_window = target;

   if (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
      SendMessageA(target, WM_SETCURSOR, (WPARAM) target, MAKELPARAM(HTCLIENT, msg));

   if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
   {
      if (!native::IsActive(t->top))
         native::Activate(t->top);
      DWORD now = GetTickCount();
      Window *tw = GetWindowObject(target);
      bool dbl = tw && (tw->cls->style & CS_DBLCLKS) && last_press.msg == msg && last_press.hwnd == target &&
                 now - last_press.time <= GetDoubleClickTime() && abs(last_press.x - x) <= 4 &&
                 abs(last_press.y - y) <= 4;
      if (dbl)
      {
         msg = msg == WM_LBUTTONDOWN ? WM_LBUTTONDBLCLK : msg == WM_RBUTTONDOWN ? WM_RBUTTONDBLCLK : WM_MBUTTONDBLCLK;
         last_press = LastPress();
      }
      else
         last_press = {msg, now, x, y, target};
   }

   PostInput(target, msg, keys, MAKELPARAM(cx, cy));
}

void MouseLeave(HWND top)
{
   if (last_mouse_window && TopLevelOf(last_mouse_window) == top && LeaveTracked().count(last_mouse_window))
   {
      LeaveTracked().erase(last_mouse_window);
      PostInput(last_mouse_window, WM_MOUSELEAVE, 0, 0);
   }
   last_mouse_window = nullptr;
}

void Key(HWND top, bool down, int vk, int scan, bool repeat, bool alt, const char *text)
{
   HWND target = GetFocus();
   if (target == nullptr || TopLevelOf(target) != top)
      target = top;
   // lParam: repeat count, scan code, context (Alt), previous state, transition.
   LPARAM lparam = 1 | ((scan & 0xFF) << 16);
   if (vk == VK_RCONTROL || vk == VK_RMENU || vk == VK_INSERT || vk == VK_DELETE || vk == VK_HOME ||
       vk == VK_END || vk == VK_PRIOR || vk == VK_NEXT || (vk >= VK_LEFT && vk <= VK_DOWN) || vk == VK_DIVIDE)
      lparam |= 1 << 24;   // extended key
   if (alt)
      lparam |= 1 << 29;
   if (!down || repeat)
      lparam |= 1 << 30;
   if (!down)
      lparam |= (LPARAM) 1 << 31;
   // Alt combinations and F10 are system keys.
   bool sys = alt || vk == VK_F10 || vk == VK_MENU;
   UINT msg = down ? (sys ? WM_SYSKEYDOWN : WM_KEYDOWN) : (sys ? WM_SYSKEYUP : WM_KEYUP);
   PostInput(target, msg, (WPARAM) vk, lparam, (down && text) ? std::string(text) : std::string());
}

void Activated(HWND top, bool active)
{
   Window *t = GetWindowObject(top);
   if (t == nullptr)
      return;
   // Hidden or disabled windows can't be activated (the toolkit may hand
   // activation to a modal dialog's disabled owner).
   if (active && (!t->visible || !t->enabled))
      return;
   HWND previous = ActiveWindow();
   if (active)
   {
      if (!app_active)
      {
         app_active = true;
         ForEachTopLevel([](HWND h, void *) { SendMessageA(h, WM_ACTIVATEAPP, TRUE, 0); }, nullptr);
      }
      SetActiveWindowState(top);
      SendMessageA(top, WM_NCACTIVATE, TRUE, 0);
      SendMessageA(top, WM_ACTIVATE, WA_ACTIVE, (LPARAM) (previous != top ? previous : nullptr));
   }
   else
   {
      if (previous == top)
         SetActiveWindowState(nullptr);
      SendMessageA(top, WM_NCACTIVATE, FALSE, 0);
      SendMessageA(top, WM_ACTIVATE, WA_INACTIVE, 0);
      // The app goes inactive when no window of ours is active.
      bool any = false;
      ForEachTopLevel([](HWND h, void *a) {
         if (Window *w = GetWindowObject(h); w && w->top && native::IsActive(w->top) && h != nullptr)
            *(bool *) a = true;
      }, &any);
      if (!any && app_active)
      {
         app_active = false;
         ForEachTopLevel([](HWND h, void *) { SendMessageA(h, WM_ACTIVATEAPP, FALSE, 0); }, nullptr);
      }
   }
}

void Focus(HWND top, bool focused)
{
   ResetKeyState();
   if (focused)
   {
      if (!IsWindowVisible(top) || !IsWindowEnabled(top))
         return;
      // The window regains the focus it had, or takes it itself.
      HWND f = SavedFocus()[top];
      if (!IsWindow(f) || TopLevelOf(f) != top)
         f = top;
      if (TopLevelOf(GetFocus()) != top)
         SetFocus(f);
   }
   else if (HWND f = GetFocus(); f && TopLevelOf(f) == top)
   {
      SavedFocus()[top] = f;
      SetFocus(nullptr);
   }
}

void Resized(HWND top, int width, int height)
{
   Window *w = GetWindowObject(top);
   if (w == nullptr)
      return;
   if (w->rect.right - w->rect.left == width && w->rect.bottom - w->rect.top == height)
      return;
   w->rect.right = w->rect.left + width;
   w->rect.bottom = w->rect.top + height;
   ComputeClient(w);
   WINDOWPOS pos = {top, nullptr, w->rect.left, w->rect.top, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE};
   SendMessageA(top, WM_WINDOWPOSCHANGING, 0, (LPARAM) &pos);
   pos.flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;
   SendMessageA(top, WM_WINDOWPOSCHANGED, 0, (LPARAM) &pos);
   InvalidateWhole(top, true);
}

void Moved(HWND top, int x, int y)
{
   Window *w = GetWindowObject(top);
   if (w == nullptr || (w->rect.left == x && w->rect.top == y))
      return;
   OffsetRect(&w->rect, x - w->rect.left, y - w->rect.top);
   SendMessageA(top, WM_MOVE, 0, MAKELPARAM(x, y));
}

void Exposed(HWND, const RECT &)
{
   // The surface keeps what was drawn; the backend shows it again by itself.
}

void CloseRequested(HWND top)
{
   if (IsWindow(top) && IsWindowEnabled(top))
      SendMessageA(top, WM_SYSCOMMAND, SC_CLOSE, 0);
}

void Minimized(HWND top, bool minimized)
{
   Window *w = GetWindowObject(top);
   if (w == nullptr || w->minimized == minimized)
      return;
   w->minimized = minimized;
   SendMessageA(top, WM_SIZE, minimized ? SIZE_MINIMIZED : SIZE_RESTORED,
                MAKELPARAM(w->client.right - w->client.left, w->client.bottom - w->client.top));
   if (!minimized)
      InvalidateWhole(top, true);
}

}  // namespace w32::events
