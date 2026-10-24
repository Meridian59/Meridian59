/*
 * winsock.cpp:  Winsock 1.1 on BSD sockets.  The client's calls arrive under m59_
 *   names (include/crt/winsock_redirect.h) with Wine's structs; SOCKET is the file
 *   descriptor.  WSAAsyncSelect watches the descriptor on the event loop and posts
 *   FD_* messages with Windows' re-enabling rules.
 */

// Wine's winsock.h under WS_ names, so the C library's socket headers fit beside it.
#define USE_WS_PREFIX
#include "core/win32.h"
#include "core/net/net.h"
#include "core/eventloop.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace w32::net {

namespace {
thread_local DWORD last_error;
}

void SetError(DWORD error)
{
   if (&SetLastError != nullptr)
      SetLastError(error);
   else
      last_error = error;
}

DWORD GetError()
{
   return &GetLastError != nullptr ? GetLastError() : last_error;
}

}  // namespace w32::net

namespace {

using w32::net::SetError;

int WsaError(int e)
{
   switch (e)
   {
   case 0:               return 0;
   case EINTR:           return WSAEINTR;
   case EBADF:           return WSAENOTSOCK;
   case EACCES:          return WSAEACCES;
   case EPERM:           return WSAEACCES;
   case EFAULT:          return WSAEFAULT;
   case EINVAL:          return WSAEINVAL;
   case EMFILE:          return WSAEMFILE;
   case ENFILE:          return WSAEMFILE;
#if EWOULDBLOCK != EAGAIN
   case EWOULDBLOCK:
#endif
   case EAGAIN:          return WSAEWOULDBLOCK;
   case EINPROGRESS:     return WSAEWOULDBLOCK;   // non-blocking connect
   case EALREADY:        return WSAEALREADY;
   case ENOTSOCK:        return WSAENOTSOCK;
   case EDESTADDRREQ:    return WSAEDESTADDRREQ;
   case EMSGSIZE:        return WSAEMSGSIZE;
   case EPROTOTYPE:      return WSAEPROTOTYPE;
   case ENOPROTOOPT:     return WSAENOPROTOOPT;
   case EPROTONOSUPPORT: return WSAEPROTONOSUPPORT;
   case ESOCKTNOSUPPORT: return WSAESOCKTNOSUPPORT;
#if EOPNOTSUPP != ENOTSUP
   case ENOTSUP:
#endif
   case EOPNOTSUPP:      return WSAEOPNOTSUPP;
   case EPFNOSUPPORT:    return WSAEPFNOSUPPORT;
   case EAFNOSUPPORT:    return WSAEAFNOSUPPORT;
   case EADDRINUSE:      return WSAEADDRINUSE;
   case EADDRNOTAVAIL:   return WSAEADDRNOTAVAIL;
   case ENETDOWN:        return WSAENETDOWN;
   case ENETUNREACH:     return WSAENETUNREACH;
   case ENETRESET:       return WSAENETRESET;
   case ECONNABORTED:    return WSAECONNABORTED;
   case ECONNRESET:      return WSAECONNRESET;
   case EPIPE:           return WSAECONNRESET;
   case ENOBUFS:         return WSAENOBUFS;
   case ENOMEM:          return WSAENOBUFS;
   case EISCONN:         return WSAEISCONN;
   case ENOTCONN:        return WSAENOTCONN;
   case ESHUTDOWN:       return WSAESHUTDOWN;
   case ETOOMANYREFS:    return WSAETOOMANYREFS;
   case ETIMEDOUT:       return WSAETIMEDOUT;
   case ECONNREFUSED:    return WSAECONNREFUSED;
   case ELOOP:           return WSAELOOP;
   case ENAMETOOLONG:    return WSAENAMETOOLONG;
   case EHOSTDOWN:       return WSAEHOSTDOWN;
   case EHOSTUNREACH:    return WSAEHOSTUNREACH;
   default:              return WSAEINVAL;
   }
}

int Fail(int wsa_error)
{
   SetError(wsa_error);
   return SOCKET_ERROR;
}

int FailErrno()
{
   return Fail(WsaError(errno));
}

enum class State { Idle, Connecting, Connected, Listening, Failed };

struct Socket
{
   int fd;
   State state = State::Idle;
   bool shut_send = false;
   // WSAAsyncSelect: where to post, which events, and which are enabled now.
   HWND hwnd = nullptr;
   UINT msg = 0;
   LONG events = 0;
   bool read_enabled = false, write_enabled = false, accept_enabled = false;
   bool close_reported = false;
   int watch = 0;
   bool watch_read = false, watch_write = false;
};

struct Post
{
   HWND hwnd;
   UINT msg;
   SOCKET s;
   int event, error;
};

std::mutex mutex;
std::map<int, Socket *> sockets;
int startups = 0;

Socket *Find(SOCKET s)
{
   if (s > (SOCKET) INT32_MAX)
      return nullptr;
   auto it = sockets.find((int) s);
   return it == sockets.end() ? nullptr : it->second;
}

void Deliver(const std::vector<Post> &posts)
{
   if (&PostMessageA == nullptr)
      return;
   for (const Post &p : posts)
      PostMessageA(p.hwnd, p.msg, (WPARAM) p.s, (LPARAM) WSAMAKESELECTREPLY(p.event, p.error));
}

void Queue(std::vector<Post> &posts, Socket *s, int event, int error)
{
   if (s->events & event)
      posts.push_back({ s->hwnd, s->msg, (SOCKET) s->fd, event, error });
}

void OnReady(int fd, bool readable, bool writable);

// Watches the descriptor for exactly what the enabled events need; GLib's fd
// sources are level-triggered, so a direction is watched only while its event
// may be posted.
void Rearm(Socket *s)
{
   bool want_read = false, want_write = false;
   if (s->events != 0)
      switch (s->state)
      {
      case State::Connecting:
         want_write = true;
         break;
      case State::Connected:
         want_read = (s->events & (FD_READ | FD_CLOSE)) && s->read_enabled && !s->close_reported;
         want_write = (s->events & FD_WRITE) && s->write_enabled;
         break;
      case State::Listening:
         want_read = (s->events & FD_ACCEPT) && s->accept_enabled;
         break;
      default:
         break;
      }
   if (s->watch != 0 && want_read == s->watch_read && want_write == s->watch_write)
      return;
   if (s->watch != 0)
      w32::loop::UnwatchFd(s->watch);
   s->watch = 0;
   s->watch_read = want_read;
   s->watch_write = want_write;
   if (want_read || want_write)
   {
      int fd = s->fd;
      s->watch = w32::loop::WatchFd(fd, want_read, want_write,
                                    [fd](bool r, bool w) { OnReady(fd, r, w); });
   }
}

// A connect in progress finished (error 0) or failed.
void Connected(Socket *s, int error, std::vector<Post> &posts)
{
   if (error != 0)
   {
      // Windows reports only FD_CONNECT with the error; the socket is dead.
      s->state = State::Failed;
      Queue(posts, s, FD_CONNECT, WsaError(error));
      return;
   }
   s->state = State::Connected;
   s->read_enabled = true;
   Queue(posts, s, FD_CONNECT, 0);
   // FD_WRITE follows a new connection.
   s->write_enabled = true;
}

void OnReady(int fd, bool readable, bool writable)
{
   std::vector<Post> posts;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find((SOCKET) fd);
      if (s == nullptr || s->events == 0)
         return;
      switch (s->state)
      {
      case State::Connecting:
      {
         int error = 0;
         socklen_t len = sizeof(error);
         if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) != 0)
            error = errno;
         Connected(s, error, posts);
         break;
      }
      case State::Listening:
         if (readable && s->accept_enabled)
         {
            s->accept_enabled = false;   // until accept
            Queue(posts, s, FD_ACCEPT, 0);
         }
         break;
      case State::Connected:
         if (readable && s->read_enabled && !s->close_reported)
         {
            // Data, or the end of the stream: FD_CLOSE comes only once the data
            // before it has been read, as on Windows.
            char c;
            ssize_t n = ::recv(fd, &c, 1, MSG_PEEK | MSG_DONTWAIT);
            if (n > 0)
            {
               s->read_enabled = false;   // until recv
               Queue(posts, s, FD_READ, 0);
            }
            else if (n == 0)
            {
               s->close_reported = true;
               Queue(posts, s, FD_CLOSE, 0);
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            {
               s->close_reported = true;
               Queue(posts, s, FD_CLOSE, WsaError(errno));
            }
         }
         if (writable && s->write_enabled)
         {
            s->write_enabled = false;   // until a send would block
            Queue(posts, s, FD_WRITE, 0);
         }
         break;
      default:
         break;
      }
      Rearm(s);
   }
   Deliver(posts);
}

bool SetNonBlocking(int fd, bool on)
{
   int flags = ::fcntl(fd, F_GETFL);
   if (flags < 0)
      return false;
   flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
   return ::fcntl(fd, F_SETFL, flags) == 0;
}

bool IsNonBlocking(int fd)
{
   int flags = ::fcntl(fd, F_GETFL);
   return flags >= 0 && (flags & O_NONBLOCK) != 0;
}

void Configure(int fd)
{
   ::fcntl(fd, F_SETFD, FD_CLOEXEC);
#ifdef SO_NOSIGPIPE
   int one = 1;
   ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
}

#ifdef MSG_NOSIGNAL
constexpr int kNoSignal = MSG_NOSIGNAL;
#else
constexpr int kNoSignal = 0;   // SO_NOSIGPIPE instead
#endif

// Wine's sockaddr_in (sin_family a short, no sin_len) to the platform's.
bool ToNative(const WS_sockaddr *name, int namelen, sockaddr_in &out, int &error)
{
   if (name == nullptr || namelen < (int) sizeof(WS_sockaddr_in))
   {
      error = WSAEFAULT;
      return false;
   }
   const auto *in = reinterpret_cast<const WS_sockaddr_in *>(name);
   if (in->sin_family != WS_AF_INET)
   {
      error = WSAEAFNOSUPPORT;
      return false;
   }
   std::memset(&out, 0, sizeof(out));
#ifdef __APPLE__
   out.sin_len = sizeof(out);
#endif
   out.sin_family = AF_INET;
   out.sin_port = in->sin_port;
   std::memcpy(&out.sin_addr, &in->sin_addr, 4);
   return true;
}

// The platform's address to Wine's, in a buffer of *namelen bytes.
bool FromNative(const sockaddr_storage &ss, WS_sockaddr *name, int *namelen, int &error)
{
   if (name == nullptr || namelen == nullptr || *namelen < (int) sizeof(WS_sockaddr_in))
   {
      error = WSAEFAULT;
      return false;
   }
   if (ss.ss_family != AF_INET)
   {
      error = WSAEAFNOSUPPORT;
      return false;
   }
   const auto &in = reinterpret_cast<const sockaddr_in &>(ss);
   WS_sockaddr_in out;
   std::memset(&out, 0, sizeof(out));
   out.sin_family = WS_AF_INET;
   out.sin_port = in.sin_port;
   std::memcpy(&out.sin_addr, &in.sin_addr, 4);
   std::memcpy(name, &out, sizeof(out));
   *namelen = sizeof(out);
   return true;
}

// Socket options: Windows' level and name to the platform's.
bool NativeOption(int level, int optname, int &nlevel, int &nname)
{
   if (level == WS_SOL_SOCKET)
   {
      nlevel = SOL_SOCKET;
      switch (optname)
      {
      case WS_SO_REUSEADDR: nname = SO_REUSEADDR; return true;
      case WS_SO_KEEPALIVE: nname = SO_KEEPALIVE; return true;
      case WS_SO_BROADCAST: nname = SO_BROADCAST; return true;
      case WS_SO_LINGER:    nname = SO_LINGER;    return true;
      case WS_SO_OOBINLINE: nname = SO_OOBINLINE; return true;
      case WS_SO_SNDBUF:    nname = SO_SNDBUF;    return true;
      case WS_SO_RCVBUF:    nname = SO_RCVBUF;    return true;
      case WS_SO_SNDTIMEO:  nname = SO_SNDTIMEO;  return true;
      case WS_SO_RCVTIMEO:  nname = SO_RCVTIMEO;  return true;
      case WS_SO_ERROR:     nname = SO_ERROR;     return true;
      case WS_SO_TYPE:      nname = SO_TYPE;      return true;
      case WS_SO_ACCEPTCONN: nname = SO_ACCEPTCONN; return true;
      }
      return false;
   }
   if (level == WS_IPPROTO_TCP && optname == WS_TCP_NODELAY)
   {
      nlevel = IPPROTO_TCP;
      nname = TCP_NODELAY;
      return true;
   }
   return false;
}

struct HostStorage
{
   WS_hostent host;
   std::string name;
   std::vector<in_addr> addrs;
   std::vector<char *> addr_list;
   char *aliases[1] = { nullptr };
};

thread_local HostStorage host_storage;

}  // namespace

/* WSA functions *************************************************************/

int WINAPI WSAStartup(WORD version, LPWSADATA data)
{
   BYTE major = LOBYTE(version), minor = HIBYTE(version);
   if (major < 1 || (major == 1 && minor < 1))
      return WSAVERNOTSUPPORTED;
   if (data == nullptr)
      return WSAEFAULT;
   std::memset(data, 0, sizeof(*data));
   data->wVersion = (major > 2 || (major == 2 && minor > 2)) ? MAKEWORD(2, 2) : version;
   data->wHighVersion = MAKEWORD(2, 2);
   std::strcpy(data->szDescription, "WinSock 2.0");
   std::strcpy(data->szSystemStatus, "Running");
   std::lock_guard<std::mutex> lock(mutex);
   startups++;
   return 0;
}

int WINAPI WSACleanup(void)
{
   std::vector<int> fds;
   {
      std::lock_guard<std::mutex> lock(mutex);
      if (startups == 0)
         return Fail(WSANOTINITIALISED);
      if (--startups > 0)
         return 0;
      // The last cleanup closes what is still open, as closesocket would.
      for (auto &entry : sockets)
      {
         if (entry.second->watch != 0)
            w32::loop::UnwatchFd(entry.second->watch);
         fds.push_back(entry.first);
         delete entry.second;
      }
      sockets.clear();
   }
   for (int fd : fds)
      ::close(fd);
   return 0;
}

int WINAPI WSAGetLastError(void)
{
   return (int) w32::net::GetError();
}

void WINAPI WSASetLastError(int error)
{
   SetError((DWORD) error);
}

int WINAPI WSAAsyncSelect(SOCKET sock, HWND hwnd, WS_u_int msg, LONG events)
{
   std::vector<Post> posts;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      if (!SetNonBlocking(s->fd, true))
         return FailErrno();
      s->hwnd = hwnd;
      s->msg = msg;
      s->events = events;
      // A new selection reports conditions that hold now (data waiting, room
      // to send, a connection made or closed) once more.
      s->read_enabled = true;
      s->accept_enabled = true;
      s->close_reported = false;
      s->write_enabled = s->state == State::Connected;
      if (events != 0 && s->state == State::Connected)
         Queue(posts, s, FD_CONNECT, 0);
      Rearm(s);
   }
   Deliver(posts);
   return 0;
}

/* Sockets *******************************************************************/

extern "C" W32_EXPORT SOCKET WINAPI m59_socket(int af, int type, int protocol)
{
   {
      std::lock_guard<std::mutex> lock(mutex);
      if (startups == 0)
      {
         SetError(WSANOTINITIALISED);
         return INVALID_SOCKET;
      }
   }
   if (af != WS_AF_INET)
   {
      SetError(WSAEAFNOSUPPORT);
      return INVALID_SOCKET;
   }
   int ntype;
   switch (type)
   {
   case WS_SOCK_STREAM: ntype = SOCK_STREAM; break;
   case WS_SOCK_DGRAM:  ntype = SOCK_DGRAM;  break;
   case WS_SOCK_RAW:    ntype = SOCK_RAW;    break;
   default:
      SetError(WSAESOCKTNOSUPPORT);
      return INVALID_SOCKET;
   }
   int nprotocol;
   switch (protocol)
   {
   case 0:               nprotocol = 0;           break;
   case WS_IPPROTO_TCP:  nprotocol = IPPROTO_TCP; break;
   case WS_IPPROTO_UDP:  nprotocol = IPPROTO_UDP; break;
   case WS_IPPROTO_ICMP: nprotocol = IPPROTO_ICMP; break;
   default:
      SetError(WSAEPROTONOSUPPORT);
      return INVALID_SOCKET;
   }
   int fd = ::socket(AF_INET, ntype, nprotocol);
   if (fd < 0)
   {
      SetError(WsaError(errno));
      return INVALID_SOCKET;
   }
   Configure(fd);
   std::lock_guard<std::mutex> lock(mutex);
   Socket *s = new Socket;
   s->fd = fd;
   sockets[fd] = s;
   return (SOCKET) fd;
}

extern "C" W32_EXPORT int WINAPI m59_closesocket(SOCKET sock)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      if (s->watch != 0)
         w32::loop::UnwatchFd(s->watch);
      fd = s->fd;
      // On Windows this ends an accept blocked on another thread; close alone
      // does not.
      if (s->state == State::Listening)
         ::shutdown(fd, SHUT_RDWR);
      sockets.erase(fd);
      delete s;
   }
   ::close(fd);
   return 0;
}

extern "C" W32_EXPORT int WINAPI m59_connect(SOCKET sock, const WS_sockaddr *name, int namelen)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      if (s->state == State::Connecting)
         return Fail(WSAEALREADY);
      if (s->state == State::Connected)
         return Fail(WSAEISCONN);
      fd = s->fd;
   }
   sockaddr_in addr;
   int error;
   if (!ToNative(name, namelen, addr, error))
      return Fail(error);

   int result = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
   int err = result == 0 ? 0 : errno;

   std::vector<Post> posts;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      if (result == 0)
      {
         // Done at once; still reported by FD_CONNECT and FD_WRITE.
         s->state = State::Idle;
         Connected(s, 0, posts);
      }
      else if (err == EINPROGRESS)
         s->state = State::Connecting;
      Rearm(s);
   }
   Deliver(posts);
   if (result == 0)
      return 0;
   return Fail(WsaError(err));
}

extern "C" W32_EXPORT int WINAPI m59_bind(SOCKET sock, const WS_sockaddr *name, int namelen)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      fd = s->fd;
   }
   sockaddr_in addr;
   int error;
   if (!ToNative(name, namelen, addr, error))
      return Fail(error);
   if (::bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0)
      return FailErrno();
   return 0;
}

extern "C" W32_EXPORT int WINAPI m59_listen(SOCKET sock, int backlog)
{
   std::lock_guard<std::mutex> lock(mutex);
   Socket *s = Find(sock);
   if (s == nullptr)
      return Fail(WSAENOTSOCK);
   if (::listen(s->fd, backlog) != 0)
      return FailErrno();
   s->state = State::Listening;
   s->accept_enabled = true;
   Rearm(s);
   return 0;
}

extern "C" W32_EXPORT SOCKET WINAPI m59_accept(SOCKET sock, WS_sockaddr *addr, int *addrlen)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
      {
         SetError(WSAENOTSOCK);
         return INVALID_SOCKET;
      }
      if (s->state != State::Listening)
      {
         SetError(WSAEINVAL);
         return INVALID_SOCKET;
      }
      fd = s->fd;
   }
   if (addr != nullptr && (addrlen == nullptr || *addrlen < (int) sizeof(WS_sockaddr_in)))
   {
      SetError(WSAEFAULT);
      return INVALID_SOCKET;
   }

   sockaddr_storage ss;
   socklen_t len = sizeof(ss);
   int nfd = ::accept(fd, reinterpret_cast<sockaddr *>(&ss), &len);
   int err = errno;

   std::lock_guard<std::mutex> lock(mutex);
   Socket *s = Find(sock);
   if (s != nullptr && s->events != 0)
   {
      s->accept_enabled = true;   // accept re-enables FD_ACCEPT
      Rearm(s);
   }
   if (nfd < 0)
   {
      SetError(WsaError(err));
      return INVALID_SOCKET;
   }
   Configure(nfd);
   if (addr != nullptr)
   {
      int error;
      FromNative(ss, addr, addrlen, error);
   }
   // The new socket has the listening socket's selection (and so is
   // non-blocking if that one is).
   Socket *n = new Socket;
   n->fd = nfd;
   n->state = State::Connected;
   if (s != nullptr)
   {
      if (IsNonBlocking(s->fd))
         SetNonBlocking(nfd, true);
      n->hwnd = s->hwnd;
      n->msg = s->msg;
      n->events = s->events;
   }
   n->read_enabled = true;
   n->write_enabled = true;
   sockets[nfd] = n;
   Rearm(n);
   return (SOCKET) nfd;
}

extern "C" W32_EXPORT int WINAPI m59_recv(SOCKET sock, char *buf, int len, int flags)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      if (s->state == State::Listening)
         return Fail(WSAENOTCONN);
      fd = s->fd;
   }
   int nflags = 0;
   if (flags & WS_MSG_OOB)
      nflags |= MSG_OOB;
   if (flags & WS_MSG_PEEK)
      nflags |= MSG_PEEK;
   if (len < 0)
      return Fail(WSAEFAULT);

   ssize_t n = ::recv(fd, buf, (size_t) len, nflags);
   int err = errno;

   {
      // recv re-enables FD_READ (and the FD_CLOSE behind the data).
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s != nullptr && s->events != 0)
      {
         s->read_enabled = true;
         Rearm(s);
      }
   }
   if (n < 0)
      return Fail(WsaError(err));
   return (int) n;
}

extern "C" W32_EXPORT int WINAPI m59_send(SOCKET sock, const char *buf, int len, int flags)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      if (s->shut_send)
         return Fail(WSAESHUTDOWN);
      fd = s->fd;
   }
   int nflags = kNoSignal;
   if (flags & WS_MSG_OOB)
      nflags |= MSG_OOB;
   if (flags & WS_MSG_DONTROUTE)
      nflags |= MSG_DONTROUTE;
   if (len < 0)
      return Fail(WSAEFAULT);

   ssize_t n = ::send(fd, buf, (size_t) len, nflags);
   int err = errno;

   if (n < 0 && (err == EAGAIN || err == EWOULDBLOCK))
   {
      // A send that would block re-enables FD_WRITE.
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s != nullptr && s->events != 0)
      {
         s->write_enabled = true;
         Rearm(s);
      }
   }
   if (n < 0)
      return Fail(WsaError(err));
   return (int) n;
}

extern "C" W32_EXPORT int WINAPI m59_shutdown(SOCKET sock, int how)
{
   std::lock_guard<std::mutex> lock(mutex);
   Socket *s = Find(sock);
   if (s == nullptr)
      return Fail(WSAENOTSOCK);
   int nhow;
   switch (how)
   {
   case 0: nhow = SHUT_RD; break;
   case 1: nhow = SHUT_WR; break;
   case 2: nhow = SHUT_RDWR; break;
   default: return Fail(WSAEINVAL);
   }
   if (::shutdown(s->fd, nhow) != 0)
      return FailErrno();
   if (how != 0)
      s->shut_send = true;
   return 0;
}

static int SockName(SOCKET sock, WS_sockaddr *name, int *namelen, bool peer)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      fd = s->fd;
   }
   sockaddr_storage ss;
   socklen_t len = sizeof(ss);
   int result = peer ? ::getpeername(fd, reinterpret_cast<sockaddr *>(&ss), &len)
                     : ::getsockname(fd, reinterpret_cast<sockaddr *>(&ss), &len);
   if (result != 0)
      return FailErrno();
   int error;
   if (!FromNative(ss, name, namelen, error))
      return Fail(error);
   return 0;
}

extern "C" W32_EXPORT int WINAPI m59_getsockname(SOCKET sock, WS_sockaddr *name, int *namelen)
{
   return SockName(sock, name, namelen, false);
}

extern "C" W32_EXPORT int WINAPI m59_getpeername(SOCKET sock, WS_sockaddr *name, int *namelen)
{
   return SockName(sock, name, namelen, true);
}

extern "C" W32_EXPORT int WINAPI m59_ioctlsocket(SOCKET sock, LONG cmd, ULONG *argp)
{
   std::lock_guard<std::mutex> lock(mutex);
   Socket *s = Find(sock);
   if (s == nullptr)
      return Fail(WSAENOTSOCK);
   if (argp == nullptr)
      return Fail(WSAEFAULT);
   switch ((ULONG) cmd)
   {
   case (ULONG) WS_FIONBIO:
      // A selected socket stays non-blocking (MSDN).
      if (*argp == 0 && s->events != 0)
         return Fail(WSAEINVAL);
      if (!SetNonBlocking(s->fd, *argp != 0))
         return FailErrno();
      return 0;
   case (ULONG) WS_FIONREAD:
   {
      int avail = 0;
      if (::ioctl(s->fd, FIONREAD, &avail) != 0)
         return FailErrno();
      *argp = (ULONG) avail;
      return 0;
   }
   default:
      return Fail(WSAEINVAL);
   }
}

extern "C" W32_EXPORT int WINAPI m59_setsockopt(SOCKET sock, int level, int optname, const char *optval, int optlen)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      fd = s->fd;
   }
   int nlevel, nname;
   if (!NativeOption(level, optname, nlevel, nname) || nname == SO_ERROR || nname == SO_TYPE)
      return Fail(WSAENOPROTOOPT);
   if (optval == nullptr || optlen < 1)
      return Fail(WSAEFAULT);
   int result;
   if (nname == SO_LINGER)
   {
      if (optlen < (int) sizeof(WS_linger))
         return Fail(WSAEFAULT);
      WS_linger wl;
      std::memcpy(&wl, optval, sizeof(wl));
      linger l = { wl.l_onoff, wl.l_linger };
      result = ::setsockopt(fd, nlevel, nname, &l, sizeof(l));
   }
   else if (nname == SO_SNDTIMEO || nname == SO_RCVTIMEO)
   {
      // Windows: a DWORD of milliseconds.
      if (optlen < (int) sizeof(DWORD))
         return Fail(WSAEFAULT);
      DWORD ms;
      std::memcpy(&ms, optval, sizeof(ms));
      timeval tv = { (time_t) (ms / 1000), (suseconds_t) ((ms % 1000) * 1000) };
      result = ::setsockopt(fd, nlevel, nname, &tv, sizeof(tv));
   }
   else
   {
      // BOOL and int options; Windows takes a one-byte BOOL too.
      int value = 0;
      if (optlen >= (int) sizeof(int))
         std::memcpy(&value, optval, sizeof(int));
      else
         value = (unsigned char) optval[0];
      result = ::setsockopt(fd, nlevel, nname, &value, sizeof(value));
   }
   return result == 0 ? 0 : FailErrno();
}

extern "C" W32_EXPORT int WINAPI m59_getsockopt(SOCKET sock, int level, int optname, char *optval, int *optlen)
{
   int fd;
   {
      std::lock_guard<std::mutex> lock(mutex);
      Socket *s = Find(sock);
      if (s == nullptr)
         return Fail(WSAENOTSOCK);
      fd = s->fd;
   }
   int nlevel, nname;
   if (!NativeOption(level, optname, nlevel, nname))
      return Fail(WSAENOPROTOOPT);
   if (optval == nullptr || optlen == nullptr)
      return Fail(WSAEFAULT);
   if (nname == SO_LINGER)
   {
      if (*optlen < (int) sizeof(WS_linger))
         return Fail(WSAEFAULT);
      linger l;
      socklen_t len = sizeof(l);
      if (::getsockopt(fd, nlevel, nname, &l, &len) != 0)
         return FailErrno();
      WS_linger wl = { (WS_u_short) l.l_onoff, (WS_u_short) l.l_linger };
      std::memcpy(optval, &wl, sizeof(wl));
      *optlen = sizeof(wl);
      return 0;
   }
   if (*optlen < (int) sizeof(int))
      return Fail(WSAEFAULT);
   int value;
   if (nname == SO_SNDTIMEO || nname == SO_RCVTIMEO)
   {
      timeval tv;
      socklen_t len = sizeof(tv);
      if (::getsockopt(fd, nlevel, nname, &tv, &len) != 0)
         return FailErrno();
      value = (int) (tv.tv_sec * 1000 + tv.tv_usec / 1000);
   }
   else
   {
      socklen_t len = sizeof(value);
      if (::getsockopt(fd, nlevel, nname, &value, &len) != 0)
         return FailErrno();
      if (nname == SO_ERROR)
         value = WsaError(value);
      else if (nname == SO_TYPE)
         value = value == SOCK_STREAM ? WS_SOCK_STREAM : value == SOCK_DGRAM ? WS_SOCK_DGRAM : WS_SOCK_RAW;
   }
   std::memcpy(optval, &value, sizeof(value));
   *optlen = sizeof(value);
   return 0;
}

/* Names and byte order ******************************************************/

extern "C" W32_EXPORT WS_hostent *WINAPI m59_gethostbyname(const char *name)
{
   {
      std::lock_guard<std::mutex> lock(mutex);
      if (startups == 0)
      {
         SetError(WSANOTINITIALISED);
         return nullptr;
      }
   }
   char local[256];
   if (name == nullptr || *name == 0)
   {
      // Windows: the local host.
      if (::gethostname(local, sizeof(local)) != 0)
      {
         SetError(WSANO_RECOVERY);
         return nullptr;
      }
      local[sizeof(local) - 1] = 0;
      name = local;
   }

   addrinfo hints;
   std::memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   addrinfo *list = nullptr;
   int rc = ::getaddrinfo(name, nullptr, &hints, &list);
   if (rc != 0)
   {
      switch (rc)
      {
      case EAI_AGAIN:  SetError(WSATRY_AGAIN); break;
      case EAI_FAIL:   SetError(WSANO_RECOVERY); break;
#if defined(EAI_NODATA) && EAI_NODATA != EAI_NONAME
      case EAI_NODATA: SetError(WSANO_DATA); break;
#endif
#ifdef EAI_ADDRFAMILY
      case EAI_ADDRFAMILY: SetError(WSANO_DATA); break;
#endif
      case EAI_MEMORY: SetError(WSAENOBUFS); break;
      default:         SetError(WSAHOST_NOT_FOUND); break;
      }
      return nullptr;
   }

   // One hostent per thread, as Windows has.
   HostStorage &hs = host_storage;
   hs.name = list->ai_canonname != nullptr ? list->ai_canonname : name;
   hs.addrs.clear();
   for (addrinfo *ai = list; ai != nullptr; ai = ai->ai_next)
      if (ai->ai_family == AF_INET)
      {
         in_addr a = reinterpret_cast<sockaddr_in *>(ai->ai_addr)->sin_addr;
         bool seen = false;
         for (const in_addr &b : hs.addrs)
            seen |= b.s_addr == a.s_addr;
         if (!seen)
            hs.addrs.push_back(a);
      }
   ::freeaddrinfo(list);
   if (hs.addrs.empty())
   {
      SetError(WSANO_DATA);
      return nullptr;
   }
   hs.addr_list.clear();
   for (in_addr &a : hs.addrs)
      hs.addr_list.push_back(reinterpret_cast<char *>(&a));
   hs.addr_list.push_back(nullptr);
   hs.host.h_name = hs.name.data();
   hs.host.h_aliases = hs.aliases;
   hs.host.h_addrtype = WS_AF_INET;
   hs.host.h_length = 4;
   hs.host.h_addr_list = hs.addr_list.data();
   return &hs.host;
}

extern "C" W32_EXPORT ULONG WINAPI m59_inet_addr(const char *cp)
{
   if (cp == nullptr || *cp == 0)
      return WS_INADDR_NONE;
   in_addr a;
   if (::inet_aton(cp, &a) == 0)
      return WS_INADDR_NONE;
   return (ULONG) a.s_addr;
}

extern "C" W32_EXPORT char *WINAPI m59_inet_ntoa(WS_in_addr in)
{
   thread_local char buf[16];
   in_addr a;
   std::memcpy(&a, &in, 4);
   if (::inet_ntop(AF_INET, &a, buf, sizeof(buf)) == nullptr)
      return nullptr;
   return buf;
}

// Byte swaps: the hosts are little-endian (x86-64, ARM64), network order big.
// (macOS has these only as macros, which the Windows headers' declarations need gone.)
extern "C" W32_EXPORT ULONG WINAPI m59_htonl(ULONG v) { return __builtin_bswap32((uint32_t) v); }
extern "C" W32_EXPORT ULONG WINAPI m59_ntohl(ULONG v) { return __builtin_bswap32((uint32_t) v); }
extern "C" W32_EXPORT WS_u_short WINAPI m59_htons(WS_u_short v) { return __builtin_bswap16(v); }
extern "C" W32_EXPORT WS_u_short WINAPI m59_ntohs(WS_u_short v) { return __builtin_bswap16(v); }
