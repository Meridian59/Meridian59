/*
 * eventloop.h:  The one event loop, the backend's (GLib on Linux, the run loop on
 *   macOS).  Native input, file descriptors (sockets), timeouts and wakeups from
 *   other threads all arrive here; GetMessage/PeekMessage run it.
 */

#ifndef W32_CORE_EVENTLOOP_H
#define W32_CORE_EVENTLOOP_H

#include <functional>

namespace w32::loop {

// Runs ready native events and callbacks; with wait, first blocks until there is
// one (or Wake is called).  GUI thread only.
void Run(bool wait);

// Makes a Run(true) on the GUI thread return soon.  Any thread.
void Wake();

// Calls ready(readable, writable) on the GUI thread while fd is readable or
// writable, for the directions asked for.  Returns an id for Unwatch.
int  WatchFd(int fd, bool read, bool write, std::function<void(bool readable, bool writable)> ready);
void UnwatchFd(int id);

// Calls fired() on the GUI thread every ms milliseconds until RemoveTimeout.
int  AddTimeout(unsigned ms, std::function<void()> fired);
void RemoveTimeout(int id);

// True on the thread that runs the loop (the one that made the first window).
bool IsGuiThread();

}  // namespace w32::loop

#endif
