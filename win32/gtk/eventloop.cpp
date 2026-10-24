/*
 * eventloop.cpp:  The event loop on GLib's default main context, which GTK runs
 *   its windows on.
 */

#include "core/eventloop.h"

#include <glib.h>
#include <glib-unix.h>

#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace w32::loop {

namespace {

std::thread::id gui_thread = std::this_thread::get_id();

struct FdWatch
{
   GSource *source;
   std::function<void(bool, bool)> ready;
};

struct Timeout
{
   guint id;
   std::function<void()> fired;
};

std::map<int, std::unique_ptr<FdWatch>> &FdWatches()
{
   static std::map<int, std::unique_ptr<FdWatch>> watches;
   return watches;
}

std::map<int, std::unique_ptr<Timeout>> &Timeouts()
{
   static std::map<int, std::unique_ptr<Timeout>> timeouts;
   return timeouts;
}

int next_id = 1;

gboolean OnFd(gint, GIOCondition condition, gpointer data)
{
   auto *watch = static_cast<FdWatch *>(data);
   bool readable = (condition & (G_IO_IN | G_IO_HUP | G_IO_ERR)) != 0;
   bool writable = (condition & (G_IO_OUT | G_IO_ERR)) != 0;
   // Copy: the callback may unwatch, freeing the watch.
   auto ready = watch->ready;
   ready(readable, writable);
   return G_SOURCE_CONTINUE;
}

gboolean OnTimeout(gpointer data)
{
   auto fired = static_cast<Timeout *>(data)->fired;
   fired();
   return G_SOURCE_CONTINUE;
}

}  // namespace

void Run(bool wait)
{
   GMainContext *context = g_main_context_default();
   // Everything ready now, after (with wait) at least one event.
   bool blocked = !wait;
   while (g_main_context_iteration(context, !blocked))
      blocked = true;
}

void Wake()
{
   g_main_context_wakeup(g_main_context_default());
}

int WatchFd(int fd, bool read, bool write, std::function<void(bool, bool)> ready)
{
   int id = next_id++;
   auto watch = std::make_unique<FdWatch>();
   GIOCondition condition = (GIOCondition) ((read ? G_IO_IN | G_IO_HUP | G_IO_ERR : 0) | (write ? G_IO_OUT : 0));
   watch->source = g_unix_fd_source_new(fd, condition);
   watch->ready = std::move(ready);
   g_source_set_callback(watch->source, (GSourceFunc) (void (*)()) OnFd, watch.get(), nullptr);
   g_source_attach(watch->source, g_main_context_default());
   FdWatches()[id] = std::move(watch);
   return id;
}

void UnwatchFd(int id)
{
   auto it = FdWatches().find(id);
   if (it == FdWatches().end())
      return;
   g_source_destroy(it->second->source);
   g_source_unref(it->second->source);
   FdWatches().erase(it);
}

int AddTimeout(unsigned ms, std::function<void()> fired)
{
   int id = next_id++;
   auto timeout = std::make_unique<Timeout>();
   timeout->fired = std::move(fired);
   timeout->id = g_timeout_add(ms, OnTimeout, timeout.get());
   Timeouts()[id] = std::move(timeout);
   return id;
}

void RemoveTimeout(int id)
{
   auto it = Timeouts().find(id);
   if (it == Timeouts().end())
      return;
   g_source_remove(it->second->id);
   Timeouts().erase(it);
}

bool IsGuiThread()
{
   return std::this_thread::get_id() == gui_thread;
}

}  // namespace w32::loop
