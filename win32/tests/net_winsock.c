/*
 * net_winsock.c:  Winsock over a loopback TCP pair: WSAAsyncSelect's FD_CONNECT,
 *   FD_ACCEPT, FD_READ, FD_WRITE and FD_CLOSE with their re-enabling rules,
 *   connection refused, names and byte order.  The notifications go to two
 *   windows, which record them, through the layer's message queue.
 */

#include <deque>   // before windows.h, whose min/max macros break it

#include <windows.h>
#include <winsock.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#define WM_CLIENT (WM_USER + 1)
#define WM_SERVER (WM_USER + 2)

static HWND hClient, hServer;

struct Message
{
   HWND hwnd;
   UINT msg;
   WPARAM wParam;
   LPARAM lParam;
};
static std::deque<Message> queue;

static LRESULT CALLBACK Record(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (msg == WM_CLIENT || msg == WM_SERVER)
   {
      queue.push_back({ hwnd, msg, wParam, lParam });
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static int failures = 0;

#define CHECK(cond)                                                          \
   do {                                                                      \
      if (!(cond)) {                                                         \
         printf("FAIL %s:%d: %s (WSAGetLastError %d)\n", __FILE__, __LINE__, \
                #cond, WSAGetLastError());                                   \
         failures++;                                                         \
      }                                                                      \
   } while (0)

static double Now()
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void RunLoopOnce()
{
   MSG m;
   while (PeekMessageA(&m, NULL, 0, 0, PM_REMOVE))
      DispatchMessageA(&m);
}

// Runs the loop for up to seconds or until a message for (hwnd, s, event) is
// queued; removes and returns it.
static bool Wait(HWND hwnd, SOCKET s, int event, int *error, double seconds = 2.0)
{
   double end = Now() + seconds;
   while (true)
   {
      for (auto it = queue.begin(); it != queue.end(); ++it)
         if (it->hwnd == hwnd && it->wParam == (WPARAM) s && WSAGETSELECTEVENT(it->lParam) == event)
         {
            if (error != NULL)
               *error = WSAGETSELECTERROR(it->lParam);
            queue.erase(it);
            return true;
         }
      if (Now() > end)
         return false;
      RunLoopOnce();
      struct timespec ts = { 0, 1000000 };
      nanosleep(&ts, NULL);
   }
}

// Runs the loop for a while and counts the messages for (hwnd, s, event).
static int Count(HWND hwnd, SOCKET s, int event, double seconds = 0.2)
{
   double end = Now() + seconds;
   while (Now() < end)
   {
      RunLoopOnce();
      struct timespec ts = { 0, 1000000 };
      nanosleep(&ts, NULL);
   }
   int n = 0;
   for (auto it = queue.begin(); it != queue.end();)
      if (it->hwnd == hwnd && it->wParam == (WPARAM) s && WSAGETSELECTEVENT(it->lParam) == event)
      {
         n++;
         it = queue.erase(it);
      }
      else
         ++it;
   return n;
}

int main()
{
   WNDCLASSA wc = {};
   wc.lpfnWndProc = Record;
   wc.lpszClassName = "net_winsock";
   RegisterClassA(&wc);
   hClient = CreateWindowA("net_winsock", "client", WS_POPUP, 0, 0, 10, 10, NULL, NULL, NULL, NULL);
   hServer = CreateWindowA("net_winsock", "server", WS_POPUP, 0, 0, 10, 10, NULL, NULL, NULL, NULL);
   WSADATA data;
   CHECK(WSAStartup(MAKEWORD(1, 1), &data) == 0);
   CHECK(data.wVersion == MAKEWORD(1, 1));
   CHECK(WSAStartup(MAKEWORD(0, 9), &data) == WSAVERNOTSUPPORTED);

   // Byte order and addresses.
   CHECK(htons(0x1234) == 0x3412);
   CHECK(ntohl(htonl(0x01020304)) == 0x01020304);
   CHECK(inet_addr("127.0.0.1") == htonl(0x7f000001));
   CHECK(inet_addr("not.an.address") == INADDR_NONE);
   CHECK(inet_addr("") == INADDR_NONE);
   {
      IN_ADDR a;
      a.s_addr = htonl(0x0a000102);
      CHECK(strcmp(inet_ntoa(a), "10.0.1.2") == 0);
   }

   // Names.
   PHOSTENT phe = gethostbyname("localhost");
   CHECK(phe != NULL);
   if (phe != NULL)
   {
      CHECK(phe->h_addrtype == AF_INET && phe->h_length == 4);
      CHECK(phe->h_addr != NULL);
   }
   phe = gethostbyname("127.0.0.1");
   CHECK(phe != NULL && memcmp(phe->h_addr, "\x7f\0\0\x01", 4) == 0);
   CHECK(gethostbyname("no-such-host.invalid") == NULL);
   {
      int e = WSAGetLastError();
      CHECK(e == WSAHOST_NOT_FOUND || e == WSANO_DATA || e == WSATRY_AGAIN);
   }

   // Bad sockets fail quietly.
   CHECK(closesocket(INVALID_SOCKET) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK);
   CHECK(recv(12345, NULL, 0, 0) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK);
   CHECK(socket(AF_INET + 100, SOCK_STREAM, 0) == INVALID_SOCKET && WSAGetLastError() == WSAEAFNOSUPPORT);

   // A listening socket on a free port.
   SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
   CHECK(listener != INVALID_SOCKET);
   SOCKADDR_IN addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   addr.sin_port = 0;
   CHECK(bind(listener, (PSOCKADDR) &addr, sizeof(addr)) == 0);
   CHECK(listen(listener, 5) == 0);
   int len = sizeof(addr);
   CHECK(getsockname(listener, (PSOCKADDR) &addr, &len) == 0 && len == sizeof(addr));
   CHECK(addr.sin_family == AF_INET && addr.sin_port != 0);
   CHECK(WSAAsyncSelect(listener, hServer, WM_SERVER, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE) == 0);

   // The client, as com.c does it: select first, then connect.
   SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
   CHECK(client != INVALID_SOCKET);
   CHECK(WSAAsyncSelect(client, hClient, WM_CLIENT, FD_READ | FD_CLOSE | FD_CONNECT) == 0);
   int rc = connect(client, (PSOCKADDR) &addr, sizeof(addr));
   CHECK(rc == 0 || WSAGetLastError() == WSAEWOULDBLOCK);
   CHECK(connect(client, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR);

   int error = -1;
   CHECK(Wait(hClient, client, FD_CONNECT, &error));
   CHECK(error == 0);
   CHECK(Count(hClient, client, FD_CONNECT) == 0);   // once
   CHECK(Wait(hServer, listener, FD_ACCEPT, &error));
   CHECK(Count(hServer, listener, FD_ACCEPT) == 0);   // until accept

   SOCKADDR_IN peer;
   len = sizeof(peer);
   SOCKET server = accept(listener, (PSOCKADDR) &peer, &len);
   CHECK(server != INVALID_SOCKET);
   CHECK(peer.sin_family == AF_INET && peer.sin_addr.s_addr == inet_addr("127.0.0.1"));
   CHECK(accept(listener, NULL, NULL) == INVALID_SOCKET && WSAGetLastError() == WSAEWOULDBLOCK);
   // The accepted socket has the listener's selection: FD_WRITE once connected.
   CHECK(Wait(hServer, server, FD_WRITE, &error));
   CHECK(Count(hServer, server, FD_WRITE) == 0);

   // FD_READ comes once until recv; again after a recv that leaves data.
   CHECK(send(server, "hello world", 11, 0) == 11);
   CHECK(Wait(hClient, client, FD_READ, &error));
   CHECK(error == 0);
   CHECK(Count(hClient, client, FD_READ) == 0);
   char buf[64];
   CHECK(recv(client, buf, 5, 0) == 5 && memcmp(buf, "hello", 5) == 0);
   CHECK(Wait(hClient, client, FD_READ, &error));
   CHECK(recv(client, buf, sizeof(buf), 0) == 6 && memcmp(buf, " world", 6) == 0);
   CHECK(Count(hClient, client, FD_READ) == 0);
   CHECK(recv(client, buf, sizeof(buf), 0) == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK);

   // FD_WRITE again after a send would block, once there is room.
   {
      static char block[65536];
      int total = 0, n;
      // Full until the peer reads; where the loopback keeps moving data to the
      // peer's buffer (macOS), FD_WRITE says there is room again: fill again.
      int writes = 1;
      for (int round = 0; round < 50 && writes != 0; round++)
      {
         while ((n = send(server, block, sizeof(block), 0)) > 0)
            total += n;
         CHECK(n == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK);
         writes = Count(hServer, server, FD_WRITE, 0.05);
      }
      CHECK(writes == 0);
      // Drain the client; it gets FD_READ as it goes.
      int got = 0;
      double end = Now() + 5;
      while (got < total && Now() < end)
      {
         int r = recv(client, block, sizeof(block), 0);
         if (r > 0)
            got += r;
         else
            RunLoopOnce();
      }
      CHECK(got == total);
      CHECK(Wait(hServer, server, FD_WRITE, &error));
      Count(hClient, client, FD_READ, 0.05);
   }

   // The server closes with data still unread: FD_READ, then FD_CLOSE once the
   // data has been read.
   CHECK(send(server, "bye", 3, 0) == 3);
   CHECK(closesocket(server) == 0);
   CHECK(Wait(hClient, client, FD_READ, &error));
   CHECK(Count(hClient, client, FD_CLOSE, 0.1) == 0);
   CHECK(recv(client, buf, sizeof(buf), 0) == 3);
   CHECK(Wait(hClient, client, FD_CLOSE, &error));
   CHECK(error == 0);
   CHECK(Count(hClient, client, FD_CLOSE) == 0);
   CHECK(recv(client, buf, sizeof(buf), 0) == 0);

   // Cancelling the selection stops messages.
   CHECK(WSAAsyncSelect(client, hClient, 0, 0) == 0);
   CHECK(closesocket(client) == 0);
   CHECK(closesocket(client) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK);

   // Connection refused: FD_CONNECT with the error, and nothing else.
   CHECK(closesocket(listener) == 0);
   SOCKET refused = socket(AF_INET, SOCK_STREAM, 0);
   CHECK(WSAAsyncSelect(refused, hClient, WM_CLIENT, FD_READ | FD_CLOSE | FD_CONNECT) == 0);
   rc = connect(refused, (PSOCKADDR) &addr, sizeof(addr));
   CHECK(rc == SOCKET_ERROR);
   int e = WSAGetLastError();
   if (e == WSAEWOULDBLOCK)
   {
      CHECK(Wait(hClient, refused, FD_CONNECT, &error));
      CHECK(error == WSAECONNREFUSED);
      CHECK(Count(hClient, refused, FD_CLOSE) == 0);
      CHECK(Count(hClient, refused, FD_READ) == 0);
   }
   else
      CHECK(e == WSAECONNREFUSED);
   closesocket(refused);

   // A blocking connect to nowhere, without a selection.
   SOCKET blocking = socket(AF_INET, SOCK_STREAM, 0);
   CHECK(connect(blocking, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR && WSAGetLastError() == WSAECONNREFUSED);
   CHECK(connect(blocking, (PSOCKADDR) &addr, 4) == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT);
   closesocket(blocking);

   CHECK(WSACleanup() == 0);
   CHECK(WSACleanup() == SOCKET_ERROR && WSAGetLastError() == WSANOTINITIALISED);
   CHECK(socket(AF_INET, SOCK_STREAM, 0) == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED);
   CHECK(queue.empty());
   for (const Message &m : queue)
      printf("left over: hwnd %p wParam %lu event %d error %d\n", (void *) m.hwnd, (unsigned long) m.wParam,
             WSAGETSELECTEVENT(m.lParam), WSAGETSELECTERROR(m.lParam));

   printf("%s: %d failure(s)\n", failures ? "FAILED" : "PASSED", failures);
   return failures ? 1 : 0;
}
