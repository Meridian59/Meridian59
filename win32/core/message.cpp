/*
 * message.cpp:  Sending, posting and retrieving messages; timers; hooks.
 *
 *   Posted messages, then input ones, each in order.  WM_PAINT and WM_TIMER are made
 *   when nothing else is queued, and WM_QUIT once the queue is empty, as Windows
 *   does.  Messages sent from other threads run on the GUI thread, which the
 *   sender waits for.
 */

#include "core/message.h"

#include "core/eventloop.h"
#include "core/handles.h"
#include "core/native.h"
#include "core/gdi.h"
#include "core/window.h"
#include "core/winproc.h"

#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>

namespace w32 {

namespace {

struct Queued
{
   MSG msg;
   std::string text;         // WM_KEYDOWN: what the key typed
};

// A message another thread sent, or a call it made (call): the GUI thread runs
// it and wakes the sender.
struct CrossThreadSend
{
   HWND hwnd;
   UINT msg;
   WPARAM wparam;
   LPARAM lparam;
   bool unicode = false;
   const std::function<LRESULT()> *call = nullptr;
   LRESULT result = 0;
   bool done = false;
};

std::mutex queue_mutex;
std::condition_variable send_done;
std::deque<Queued> queue;         // posted
std::deque<Queued> input;         // keyboard and mouse, retrieved after posted ones
std::deque<CrossThreadSend *> cross_sends;
bool quit_posted = false;
int quit_code = 0;

// Text from WM_KEYDOWNs retrieved but not yet translated, by the message's time
// and key; TranslateMessage takes it.
std::deque<Queued> pending_chars;

struct Timer
{
   HWND hwnd;
   UINT_PTR id;
   TIMERPROC proc;
   int loop_id;
   uint64_t due = 0;   // when it came due (due_order), 0 while not
};

std::map<std::pair<HWND, UINT_PTR>, Timer> &Timers()
{
   static std::map<std::pair<HWND, UINT_PTR>, Timer> timers;
   return timers;
}

// SetSystemTimer's (carets, scroll bars, controls): WM_SYSTIMER, ids of their own.
std::map<std::pair<HWND, UINT_PTR>, Timer> &SystemTimers()
{
   static std::map<std::pair<HWND, UINT_PTR>, Timer> timers;
   return timers;
}

UINT_PTR next_timer_id = 0x7000;   // ids for SetTimer(NULL, 0 ...)
uint64_t due_order = 0;

// A timer came due; one already due keeps its place.
void TimerDue(Timer &t)
{
   if (t.due == 0)
      t.due = ++due_order;
}

// The timer for hwnd's filter due longest, or null: as Wine's, by when they came
// due, so a window whose timer is always due (one slower to handle than its
// period) doesn't starve the others.
Timer *OldestDue(std::map<std::pair<HWND, UINT_PTR>, Timer> &timers, HWND hwnd)
{
   Timer *oldest = nullptr;
   for (auto &entry : timers)
   {
      Timer &t = entry.second;
      if (t.due == 0 || (hwnd != nullptr && t.hwnd != hwnd && !IsChild(hwnd, t.hwnd)))
         continue;
      if (oldest == nullptr || t.due < oldest->due)
         oldest = &t;
   }
   return oldest;
}
bool paint_pending = false;

DWORD Now()
{
   return GetTickCount();
}

bool Matches(const MSG &m, HWND hwnd, UINT min, UINT max)
{
   if (hwnd != nullptr && hwnd != (HWND) -1 && m.hwnd != hwnd && !IsChild(hwnd, m.hwnd))
      return false;
   if (hwnd == (HWND) -1 && m.hwnd != nullptr)
      return false;
   if (min == 0 && max == 0)
      return true;
   return m.message >= min && m.message <= max;
}

// Low-level and thread keyboard hooks.
struct Hook
{
   int type;
   HOOKPROC proc;
};

HandleTable<Hook, HHOOK> &Hooks()
{
   static HandleTable<Hook, HHOOK> hooks(kMiscHandleBase + 0x100000);
   return hooks;
}

// True if a hook ate the key message.
bool RunKeyboardHooks(const MSG &m)
{
   bool eaten = false;
   std::vector<Hook> hooks;
   Hooks().ForEach([&](HHOOK, Hook *h) { hooks.push_back(*h); });
   for (const Hook &h : hooks)
   {
      if (h.type == WH_KEYBOARD_LL)
      {
         KBDLLHOOKSTRUCT k = {};
         k.vkCode = (DWORD) m.wParam;
         k.scanCode = (m.lParam >> 16) & 0xFF;
         k.flags = (m.message == WM_KEYUP || m.message == WM_SYSKEYUP) ? LLKHF_UP : 0;
         if (m.message == WM_SYSKEYDOWN || m.message == WM_SYSKEYUP)
            k.flags |= LLKHF_ALTDOWN;
         k.time = m.time;
         if (h.proc(HC_ACTION, m.message, (LPARAM) &k))
            eaten = true;
      }
      else if (h.type == WH_KEYBOARD)
      {
         if (h.proc(HC_ACTION, m.wParam, m.lParam))
            eaten = true;
      }
   }
   return eaten;
}

// A window in pre-order (parents first) that needs painting, or nullptr.
HWND FindPaintWindow(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || !w->visible)
      return nullptr;
   if (w->update || w->internal_paint)
      return hwnd;
   // As Windows: from the top of the Z order down, so where siblings overlap
   // without WS_CLIPSIBLINGS, lower ones paint last (a dialog's button over the
   // group box around it).
   for (HWND c : w->children)
      if (HWND found = FindPaintWindow(c))
         return found;
   return nullptr;
}

HWND NextPaintWindow()
{
   if (!paint_pending)
      return nullptr;
   HWND found = nullptr;
   std::vector<HWND> tops;
   ForEachTopLevel([](HWND h, void *data) { static_cast<std::vector<HWND> *>(data)->push_back(h); }, &tops);
   for (HWND t : tops)
      if ((found = FindPaintWindow(t)) != nullptr)
         return found;
   paint_pending = false;
   return nullptr;
}

void RunCrossThreadSends()
{
   for (;;)
   {
      CrossThreadSend *s;
      {
         std::lock_guard<std::mutex> lock(queue_mutex);
         if (cross_sends.empty())
            return;
         s = cross_sends.front();
         cross_sends.pop_front();
      }
      LRESULT r = s->call      ? (*s->call)()
                  : s->unicode ? SendMessageW(s->hwnd, s->msg, s->wparam, s->lparam)
                               : SendMessageA(s->hwnd, s->msg, s->wparam, s->lparam);
      {
         std::lock_guard<std::mutex> lock(queue_mutex);
         s->result = r;
         s->done = true;
      }
      send_done.notify_all();
   }
}

// The next message for PeekMessage/GetMessage, or false.
bool Retrieve(MSG *out, HWND hwnd, UINT min, UINT max, bool remove)
{
   RunCrossThreadSends();

   {
      std::lock_guard<std::mutex> lock(queue_mutex);
      // Posted messages first (the WM_CHARs TranslateMessage posts among them),
      // then input, as Windows.
      for (std::deque<Queued> *q : {&queue, &input})
         for (auto it = q->begin(); it != q->end(); ++it)
         {
            if (!Matches(it->msg, hwnd, min, max))
               continue;
            Queued m = *it;
            if (remove)
               q->erase(it);
            *out = m.msg;
            if (remove && q == &input)
               KeyStateFromMessage(m.msg);
            if (remove && (m.msg.message == WM_KEYDOWN || m.msg.message == WM_SYSKEYDOWN))
               pending_chars.push_back(m);
            return true;
         }
   }

   if (min == 0 || (min <= WM_PAINT && max >= WM_PAINT))
      if (HWND p = NextPaintWindow(); p && (hwnd == nullptr || p == hwnd || IsChild(hwnd, p)))
      {
         *out = {p, WM_PAINT, 0, 0, Now(), native::GetCursorPos()};
         return true;
      }

   if (min == 0 || (min <= WM_TIMER && max >= WM_TIMER))
      if (Timer *t = OldestDue(Timers(), hwnd))
      {
         if (remove)
            t->due = 0;
         *out = {t->hwnd, WM_TIMER, t->id, (LPARAM) t->proc, Now(), native::GetCursorPos()};
         return true;
      }

   if (min == 0 || (min <= WM_SYSTIMER && max >= WM_SYSTIMER))
      if (Timer *t = OldestDue(SystemTimers(), hwnd))
      {
         if (remove)
            t->due = 0;
         *out = {t->hwnd, WM_SYSTIMER, t->id, (LPARAM) t->proc, Now(), native::GetCursorPos()};
         return true;
      }

   std::lock_guard<std::mutex> lock(queue_mutex);
   if (quit_posted && (min == 0 || (min <= WM_QUIT && max >= WM_QUIT)))
   {
      if (remove)
         quit_posted = false;
      *out = {nullptr, WM_QUIT, (WPARAM) quit_code, 0, Now(), native::GetCursorPos()};
      return true;
   }
   return false;
}

}  // namespace

LRESULT OnGuiThread(const std::function<LRESULT()> &call)
{
   if (loop::IsGuiThread())
      return call();
   CrossThreadSend s = {nullptr, 0, 0, 0};
   s.call = &call;
   std::unique_lock<std::mutex> lock(queue_mutex);
   cross_sends.push_back(&s);
   loop::Wake();
   send_done.wait(lock, [&] { return s.done; });
   return s.result;
}

void PostInput(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, const std::string &text)
{
   Queued q;
   q.msg = {hwnd, msg, wparam, lparam, Now(), native::GetCursorPos()};
   q.text = text;
   if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST) && RunKeyboardHooks(q.msg))
      return;
   {
      std::lock_guard<std::mutex> lock(queue_mutex);
      // Mouse moves coalesce, as Windows' do.
      if (msg == WM_MOUSEMOVE && !input.empty() && input.back().msg.message == WM_MOUSEMOVE &&
          input.back().msg.hwnd == hwnd)
         input.back() = q;
      else
         input.push_back(q);
   }
   loop::Wake();
}

void KillWindowTimers(HWND hwnd)
{
   for (auto *timers : {&Timers(), &SystemTimers()})
      for (auto it = timers->begin(); it != timers->end();)
         if (it->second.hwnd == hwnd)
         {
            loop::RemoveTimeout(it->second.loop_id);
            it = timers->erase(it);
         }
         else
            ++it;
   std::lock_guard<std::mutex> lock(queue_mutex);
   for (std::deque<Queued> *q : {&queue, &input})
      for (auto it = q->begin(); it != q->end();)
         it = it->msg.hwnd == hwnd ? q->erase(it) : it + 1;
}

void ForgetPaint(HWND)
{
   // Update regions live on the window; nothing else to drop.
}

void PaintPending()
{
   paint_pending = true;
   loop::Wake();
}

}  // namespace w32

using namespace w32;

/************************************************************************/

LRESULT WINAPI SendMessageA(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (hwnd == HWND_BROADCAST)
   {
      std::vector<HWND> tops;
      ForEachTopLevel([](HWND h, void *data) { static_cast<std::vector<HWND> *>(data)->push_back(h); }, &tops);
      for (HWND h : tops)
         SendMessageA(h, msg, wparam, lparam);
      return 0;
   }
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return 0;
   }
   if (!loop::IsGuiThread())
   {
      CrossThreadSend s = {hwnd, msg, wparam, lparam};
      std::unique_lock<std::mutex> lock(queue_mutex);
      cross_sends.push_back(&s);
      loop::Wake();
      send_done.wait(lock, [&] { return s.done; });
      return s.result;
   }
   return CallProc(w, msg, wparam, lparam);
}

LRESULT WINAPI SendDlgItemMessageA(HWND dialog, INT id, UINT msg, WPARAM wparam, LPARAM lparam)
{
   HWND item = GetDlgItem(dialog, id);
   return item ? SendMessageA(item, msg, wparam, lparam) : 0;
}

BOOL WINAPI PostMessageA(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (hwnd != nullptr && hwnd != HWND_BROADCAST && GetWindowObject(hwnd) == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return FALSE;
   }
   Queued q;
   q.msg = {hwnd, msg, wparam, lparam, Now(), {0, 0}};
   {
      std::lock_guard<std::mutex> lock(queue_mutex);
      queue.push_back(q);
   }
   loop::Wake();
   return TRUE;
}

BOOL WINAPI PostThreadMessageA(DWORD, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return PostMessageA(nullptr, msg, wparam, lparam);
}

void WINAPI PostQuitMessage(INT code)
{
   std::lock_guard<std::mutex> lock(queue_mutex);
   quit_posted = true;
   quit_code = code;
   loop::Wake();
}

BOOL WINAPI PeekMessageA(MSG *msg, HWND hwnd, UINT min, UINT max, UINT flags)
{
   loop::Run(false);
   MSG m;
   if (!Retrieve(&m, hwnd, min, max, (flags & PM_REMOVE) != 0))
      return FALSE;
   if (msg)
      *msg = m;
   return TRUE;
}

BOOL WINAPI GetMessageA(MSG *msg, HWND hwnd, UINT min, UINT max)
{
   MSG m;
   loop::Run(false);
   while (!Retrieve(&m, hwnd, min, max, true))
      loop::Run(true);
   if (msg)
      *msg = m;
   return m.message != WM_QUIT;
}

BOOL WINAPI WaitMessage(void)
{
   MSG m;
   loop::Run(false);
   while (!Retrieve(&m, nullptr, 0, 0, false))
      loop::Run(true);
   return TRUE;
}

LRESULT WINAPI DispatchMessageA(const MSG *msg)
{
   if (msg == nullptr)
      return 0;
   if (msg->message == WM_TIMER && msg->lParam)
   {
      ((TIMERPROC) msg->lParam)(msg->hwnd, WM_TIMER, msg->wParam, GetTickCount());
      return 0;
   }
   Window *w = GetWindowObject(msg->hwnd);
   if (w == nullptr)
      return 0;
   if (msg->message == WM_PAINT)
   {
      LRESULT r = CallProc(w, WM_PAINT, 0, 0);
      // A window that didn't BeginPaint is painted anyway (as DefWindowProc would
      // validate it), or WM_PAINT would come forever.
      if ((w = GetWindowObject(msg->hwnd)) != nullptr && (w->update || w->internal_paint))
         ValidateRect(msg->hwnd, nullptr);
      return r;
   }
   return CallProc(w, msg->message, msg->wParam, msg->lParam);
}

BOOL WINAPI TranslateMessage(const MSG *msg)
{
   if (msg == nullptr || (msg->message != WM_KEYDOWN && msg->message != WM_SYSKEYDOWN))
      return FALSE;
   // The text the key typed, from when the WM_KEYDOWN was retrieved.
   std::string text;
   for (auto it = pending_chars.begin(); it != pending_chars.end(); ++it)
      if (it->msg.time == msg->time && it->msg.wParam == msg->wParam && it->msg.hwnd == msg->hwnd)
      {
         text = it->text;
         pending_chars.erase(pending_chars.begin(), it + 1);
         break;
      }
   if (pending_chars.size() > 64)
      pending_chars.clear();
   if (text.empty())
      return FALSE;
   std::string ansi = Utf8ToAnsi(text);
   UINT char_msg = msg->message == WM_SYSKEYDOWN ? WM_SYSCHAR : WM_CHAR;
   for (unsigned char c : ansi)
      PostMessageA(msg->hwnd, char_msg, c, msg->lParam);
   return TRUE;
}

/************************************************************************/
// Timers

UINT_PTR WINAPI SetTimer(HWND hwnd, UINT_PTR id, UINT ms, TIMERPROC proc)
{
   if (hwnd != nullptr && GetWindowObject(hwnd) == nullptr)
      return 0;
   if (hwnd == nullptr && id == 0)
      id = next_timer_id++;
   auto key = std::make_pair(hwnd, id);
   auto &timers = Timers();
   if (auto it = timers.find(key); it != timers.end())
      loop::RemoveTimeout(it->second.loop_id);
   Timer t = {hwnd, id, proc, 0, 0};
   t.loop_id = loop::AddTimeout(std::max(ms, 10u), [key] {
      auto &timers = Timers();
      if (auto it = timers.find(key); it != timers.end())
         TimerDue(it->second);
   });
   timers[key] = t;
   return hwnd == nullptr ? id : (id ? id : 1);
}

UINT_PTR WINAPI SetSystemTimer(HWND hwnd, UINT_PTR id, UINT ms, void *proc)
{
   if (GetWindowObject(hwnd) == nullptr)
      return 0;
   auto key = std::make_pair(hwnd, id);
   auto &timers = SystemTimers();
   if (auto it = timers.find(key); it != timers.end())
      loop::RemoveTimeout(it->second.loop_id);
   Timer t = {hwnd, id, (TIMERPROC) proc, 0, 0};
   t.loop_id = loop::AddTimeout(std::max(ms, 10u), [key] {
      auto &timers = SystemTimers();
      if (auto it = timers.find(key); it != timers.end())
      {
         TimerDue(it->second);
         loop::Wake();
      }
   });
   timers[key] = t;
   return id ? id : 1;
}

BOOL WINAPI KillSystemTimer(HWND hwnd, UINT_PTR id)
{
   auto &timers = SystemTimers();
   auto it = timers.find(std::make_pair(hwnd, id));
   if (it == timers.end())
      return FALSE;
   loop::RemoveTimeout(it->second.loop_id);
   timers.erase(it);
   return TRUE;
}

BOOL WINAPI KillTimer(HWND hwnd, UINT_PTR id)
{
   auto &timers = Timers();
   auto it = timers.find(std::make_pair(hwnd, id));
   if (it == timers.end())
      return FALSE;
   loop::RemoveTimeout(it->second.loop_id);
   timers.erase(it);
   return TRUE;
}

/************************************************************************/
// Hooks

HHOOK WINAPI SetWindowsHookExA(INT type, HOOKPROC proc, HINSTANCE, DWORD)
{
   if (type != WH_KEYBOARD && type != WH_KEYBOARD_LL)
   {
      SetLastError(ERROR_INVALID_HOOK_FILTER);
      return nullptr;
   }
   return Hooks().Add(new Hook{type, proc});
}

BOOL WINAPI UnhookWindowsHookEx(HHOOK hook)
{
   Hook *h = Hooks().Remove(hook);
   delete h;
   return h != nullptr;
}

LRESULT WINAPI CallNextHookEx(HHOOK, INT, WPARAM, LPARAM)
{
   // Hooks run in turn whatever each returns; there is no next to call.
   return 0;
}

/************************************************************************/
// Unicode versions.  The queue holds ANSI messages; characters convert on the way.

static bool IsCharMessage(UINT msg)
{
   return msg == WM_CHAR || msg == WM_SYSCHAR || msg == WM_DEADCHAR || msg == WM_SYSDEADCHAR;
}

LRESULT WINAPI SendMessageW(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (hwnd == HWND_BROADCAST)
   {
      std::vector<HWND> tops;
      ForEachTopLevel([](HWND h, void *data) { static_cast<std::vector<HWND> *>(data)->push_back(h); }, &tops);
      for (HWND h : tops)
         SendMessageW(h, msg, wparam, lparam);
      return 0;
   }
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return 0;
   }
   if (!loop::IsGuiThread())
   {
      CrossThreadSend s = {hwnd, msg, wparam, lparam, true};
      std::unique_lock<std::mutex> lock(queue_mutex);
      cross_sends.push_back(&s);
      loop::Wake();
      send_done.wait(lock, [&] { return s.done; });
      return s.result;
   }
   return CallProcW(w, msg, wparam, lparam);
}

LRESULT WINAPI SendDlgItemMessageW(HWND dialog, INT id, UINT msg, WPARAM wparam, LPARAM lparam)
{
   HWND item = GetDlgItem(dialog, id);
   return item ? SendMessageW(item, msg, wparam, lparam) : 0;
}

BOOL WINAPI PostMessageW(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return PostMessageA(hwnd, msg, IsCharMessage(msg) ? CharWtoA(wparam) : wparam, lparam);
}

BOOL WINAPI PeekMessageW(MSG *msg, HWND hwnd, UINT min, UINT max, UINT flags)
{
   BOOL r = PeekMessageA(msg, hwnd, min, max, flags);
   if (r && IsCharMessage(msg->message))
      msg->wParam = CharAtoW(msg->wParam);
   return r;
}

BOOL WINAPI GetMessageW(MSG *msg, HWND hwnd, UINT min, UINT max)
{
   BOOL r = GetMessageA(msg, hwnd, min, max);
   if (r > 0 && IsCharMessage(msg->message))
      msg->wParam = CharAtoW(msg->wParam);
   return r;
}

LRESULT WINAPI DispatchMessageW(const MSG *msg)
{
   if (msg == nullptr || !IsCharMessage(msg->message))
      return DispatchMessageA(msg);
   MSG a = *msg;
   a.wParam = CharWtoA(a.wParam);
   return DispatchMessageA(&a);
}
