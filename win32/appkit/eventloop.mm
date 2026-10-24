/*
 * eventloop.mm:  The event loop on AppKit's: NSApp's events, with file
 *   descriptors as dispatch sources on the main queue (level-triggered, as GLib's
 *   watches), timeouts as run loop timers in every mode.  Their callbacks and
 *   Wake post an application event, so a Run(true) waiting for events returns.
 */

#include "appkit/appkit_backend.h"
#include "core/eventloop.h"

#include <dispatch/dispatch.h>
#include <map>
#include <memory>
#include <pthread.h>

namespace w32::loop {

namespace {

struct FdWatch
{
   dispatch_source_t read = nullptr, write = nullptr;
   std::function<void(bool, bool)> ready;
};

struct Timeout
{
   CFRunLoopTimerRef timer = nullptr;
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
pthread_t gui_thread = pthread_self();

// An event that only ends nextEventMatchingMask's wait.
void PostWakeup()
{
   @autoreleasepool
   {
      NSEvent *e = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:NSZeroPoint modifierFlags:0
                                     timestamp:0 windowNumber:0 context:nil subtype:0x4D35 data1:0 data2:0];
      [NSApp postEvent:e atStart:NO];
   }
}

// The watch id's descriptor is readable or writable.
void OnFd(int id, bool readable)
{
   auto it = FdWatches().find(id);
   if (it == FdWatches().end())
      return;
   auto ready = it->second->ready;
   ready(readable, !readable);
   PostWakeup();
}

dispatch_source_t FdSource(int fd, dispatch_source_type_t type, int id, bool readable)
{
   dispatch_source_t s = dispatch_source_create(type, (uintptr_t) fd, 0, dispatch_get_main_queue());
   dispatch_source_set_event_handler(s, ^{
      OnFd(id, readable);
   });
   dispatch_resume(s);
   return s;
}

void OnTimer(CFRunLoopTimerRef, void *info)
{
   int id = (int) (intptr_t) info;
   auto it = Timeouts().find(id);
   if (it == Timeouts().end())
      return;
   auto fired = it->second->fired;
   fired();
   PostWakeup();
}

}  // namespace

void Run(bool wait)
{
   @autoreleasepool
   {
      mac::Init();
      NSDate *until = wait ? [NSDate distantFuture] : [NSDate distantPast];
      for (;;)
      {
         NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:until inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
         if (e == nil)
            break;
         if (!(e.type == NSEventTypeApplicationDefined && e.subtype == 0x4D35))
            [NSApp sendEvent:e];
         // Then whatever else is ready, without waiting.
         until = [NSDate distantPast];
      }
   }
}

void Wake()
{
   PostWakeup();
}

int WatchFd(int fd, bool read, bool write, std::function<void(bool, bool)> ready)
{
   int id = next_id++;
   auto w = std::make_unique<FdWatch>();
   w->ready = std::move(ready);
   if (read)
      w->read = FdSource(fd, DISPATCH_SOURCE_TYPE_READ, id, true);
   if (write)
      w->write = FdSource(fd, DISPATCH_SOURCE_TYPE_WRITE, id, false);
   FdWatches()[id] = std::move(w);
   return id;
}

void UnwatchFd(int id)
{
   auto it = FdWatches().find(id);
   if (it == FdWatches().end())
      return;
   for (dispatch_source_t s : {it->second->read, it->second->write})
      if (s)
      {
         dispatch_source_cancel(s);
         dispatch_release(s);
      }
   FdWatches().erase(it);
}

int AddTimeout(unsigned ms, std::function<void()> fired)
{
   int id = next_id++;
   auto t = std::make_unique<Timeout>();
   t->fired = std::move(fired);
   CFRunLoopTimerContext context = {0, (void *) (intptr_t) id, nullptr, nullptr, nullptr};
   double interval = std::max(ms, 1u) / 1000.0;
   t->timer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + interval, interval, 0, 0,
                                   OnTimer, &context);
   CFRunLoopAddTimer(CFRunLoopGetMain(), t->timer, kCFRunLoopCommonModes);
   Timeouts()[id] = std::move(t);
   return id;
}

void RemoveTimeout(int id)
{
   auto it = Timeouts().find(id);
   if (it == Timeouts().end())
      return;
   CFRunLoopTimerInvalidate(it->second->timer);
   CFRelease(it->second->timer);
   Timeouts().erase(it);
}

bool IsGuiThread()
{
   return pthread_equal(pthread_self(), gui_thread);
}

}  // namespace w32::loop
