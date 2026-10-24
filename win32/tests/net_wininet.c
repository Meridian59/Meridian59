/*
 * net_wininet.c:  WinINet against a small HTTP server on loopback (run on a
 *   thread with blocking Winsock): a large GET read the way transfer.c reads it,
 *   a POST the way web.c sends it, redirects, 404, header queries, refused
 *   connections and closing a parent handle.
 *
 *   Set M59_NET_TEST_HTTPS=host/path to also fetch https://host/path.
 */

#include <string>
#include <thread>

#include <windows.h>
#include <winsock.h>
#include <wininet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GetLastError is the kernel part's; WSAGetLastError reads the same value, as
// on Windows.
static DWORD LastError()
{
   return (DWORD) WSAGetLastError();
}

static int failures = 0;

#define CHECK(cond)                                                                                \
   do {                                                                                            \
      if (!(cond)) {                                                                               \
         printf("FAIL %s:%d: %s (last error %lu)\n", __FILE__, __LINE__, #cond,                  \
                (unsigned long) LastError());                                                   \
         failures++;                                                                               \
      }                                                                                            \
   } while (0)

static const int kBigSize = 3 * 1024 * 1024 + 123;

static char Pattern(int i)
{
   return (char) ((i * 7 + i / 251) & 0xff);
}

static bool SendAll(SOCKET s, const char *data, size_t len)
{
   while (len > 0)
   {
      int n = send(s, data, (int) (len > 65536 ? 65536 : len), 0);
      if (n <= 0)
         return false;
      data += n;
      len -= n;
   }
   return true;
}

static void Respond(SOCKET s, const char *status, const std::string &headers, const std::string &body)
{
   std::string head = std::string("HTTP/1.1 ") + status + "\r\nConnection: close\r\nContent-Length: " +
                      std::to_string(body.size()) + "\r\n" + headers + "\r\n";
   SendAll(s, head.data(), head.size());
   SendAll(s, body.data(), body.size());
}

static void Serve(SOCKET client)
{
   std::string request;
   char buf[4096];
   size_t header_end;
   while ((header_end = request.find("\r\n\r\n")) == std::string::npos)
   {
      int n = recv(client, buf, sizeof(buf), 0);
      if (n <= 0)
         return;
      request.append(buf, n);
   }
   std::string head = request.substr(0, header_end + 2);
   std::string body = request.substr(header_end + 4);
   size_t cl = head.find("Content-Length: ");
   size_t want = cl == std::string::npos ? 0 : strtoul(head.c_str() + cl + 16, NULL, 10);
   while (body.size() < want)
   {
      int n = recv(client, buf, sizeof(buf), 0);
      if (n <= 0)
         return;
      body.append(buf, n);
   }

   std::string line = head.substr(0, head.find("\r\n"));
   if (line == "GET /big.bin HTTP/1.1")
   {
      std::string data(kBigSize, 0);
      for (int i = 0; i < kBigSize; i++)
         data[i] = Pattern(i);
      Respond(client, "200 OK", "Content-Type: application/octet-stream\r\nX-Twice: one\r\nX-Twice: two\r\n", data);
   }
   else if (line == "POST /echo HTTP/1.1")
      Respond(client, "200 OK", "Content-Type: text/plain\r\n", head + "\r\n" + body);
   else if (line == "GET /moved HTTP/1.1")
      Respond(client, "302 Found", "Location: /small%20file.txt\r\n", "moved");
   else if (line == "GET /small%20file.txt HTTP/1.1")
      Respond(client, "200 OK", "", "small");
   else
      Respond(client, "404 Not Found", "", "nope");
}

static void ServerLoop(SOCKET listener)
{
   while (true)
   {
      SOCKET c = accept(listener, NULL, NULL);
      if (c == INVALID_SOCKET)
         return;   // closed: done
      Serve(c);
      shutdown(c, 1);
      closesocket(c);
   }
}

static std::string ReadAll(HINTERNET h, DWORD chunk)
{
   std::string out;
   char *buf = (char *) malloc(chunk);
   DWORD n = 0;
   while (InternetReadFile(h, buf, chunk, &n) && n > 0)
   {
      out.append(buf, n);
      n = 0;
   }
   free(buf);
   return out;
}

int main()
{
   WSADATA wsa;
   WSAStartup(MAKEWORD(1, 1), &wsa);
   SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   CHECK(bind(listener, (PSOCKADDR) &addr, sizeof(addr)) == 0);
   CHECK(listen(listener, 5) == 0);
   int len = sizeof(addr);
   getsockname(listener, (PSOCKADDR) &addr, &len);
   INTERNET_PORT port = ntohs(addr.sin_port);
   std::thread server(ServerLoop, listener);

   HINTERNET session = InternetOpenA("m59 test", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
   CHECK(session != NULL);
   HINTERNET conn = InternetConnectA(session, "127.0.0.1", port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
   CHECK(conn != NULL);
   CHECK(HttpOpenRequestA(session, NULL, "/", NULL, NULL, NULL, 0, 0) == NULL &&
         LastError() == ERROR_INTERNET_INCORRECT_HANDLE_TYPE);

   // A large file, as transfer.c gets it.
   {
      const char *types[2] = { "application/x-zip-compressed" };
      HINTERNET req = HttpOpenRequestA(conn, NULL, "/big.bin", NULL, NULL, types, INTERNET_FLAG_NO_UI, 0);
      CHECK(req != NULL);
      DWORD n;
      char tmp[16];
      CHECK(!InternetReadFile(req, tmp, sizeof(tmp), &n) && LastError() == ERROR_INTERNET_INCORRECT_HANDLE_STATE);
      CHECK(HttpSendRequestA(req, NULL, 0, NULL, 0));

      DWORD size = 0, size_len = sizeof(size), index = 0;
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &size, &size_len, &index));
      CHECK(size == (DWORD) kBigSize && size_len == sizeof(DWORD) && index == 1);
      DWORD status = 0, status_len = sizeof(status);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &status_len, NULL));
      CHECK(status == 200);

      char text[64];
      DWORD text_len = sizeof(text);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_STATUS_TEXT, text, &text_len, NULL));
      CHECK(strcmp(text, "OK") == 0 && text_len == 2);
      text_len = 3;
      CHECK(!HttpQueryInfoA(req, HTTP_QUERY_CONTENT_TYPE, text, &text_len, NULL));
      CHECK(LastError() == ERROR_INSUFFICIENT_BUFFER && text_len == strlen("application/octet-stream") + 1);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_CONTENT_TYPE, text, &text_len, NULL));
      CHECK(strcmp(text, "application/octet-stream") == 0);

      // Repeated headers by index.
      index = 0;
      strcpy(text, "x-twice");
      text_len = sizeof(text);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_CUSTOM, text, &text_len, &index) && strcmp(text, "one") == 0);
      strcpy(text, "X-Twice");
      text_len = sizeof(text);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_CUSTOM, text, &text_len, &index) && strcmp(text, "two") == 0);
      strcpy(text, "X-Twice");
      text_len = sizeof(text);
      CHECK(!HttpQueryInfoA(req, HTTP_QUERY_CUSTOM, text, &text_len, &index) &&
            LastError() == ERROR_HTTP_HEADER_NOT_FOUND && index == 2);
      text_len = sizeof(text);
      CHECK(!HttpQueryInfoA(req, HTTP_QUERY_LOCATION, text, &text_len, NULL) && LastError() == ERROR_HTTP_HEADER_NOT_FOUND);

      char raw[1024];
      DWORD raw_len = sizeof(raw);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_RAW_HEADERS_CRLF, raw, &raw_len, NULL));
      CHECK(strncmp(raw, "HTTP/1.1 200 OK\r\n", 17) == 0 && strstr(raw, "\r\nContent-Length: ") != NULL);
      CHECK(raw_len == strlen(raw) && strcmp(raw + raw_len - 4, "\r\n\r\n") == 0);

      std::string body = ReadAll(req, 4096);
      CHECK(body.size() == (size_t) kBigSize);
      bool same = body.size() == (size_t) kBigSize;
      for (int i = 0; same && i < kBigSize; i++)
         same = body[i] == Pattern(i);
      CHECK(same);
      CHECK(InternetReadFile(req, tmp, sizeof(tmp), &n) && n == 0);
      CHECK(InternetCloseHandle(req));
      CHECK(!InternetCloseHandle(req) && LastError() == ERROR_INVALID_HANDLE);
   }

   // A POST, as web.c sends it (headers separated by "\n").
   {
      HINTERNET req = HttpOpenRequestA(conn, "POST", "/echo", 0, 0, 0, INTERNET_FLAG_RELOAD, 0);
      CHECK(req != NULL);
      const char *headers = "Content-Type: application/x-www-form-urlencoded\nClient-Version: 50.53\n";
      const char *post = "user=a%20b&pass=c";
      CHECK(HttpSendRequestA(req, headers, (DWORD) strlen(headers), (LPVOID) post, (DWORD) strlen(post)));
      char buf[1025];
      DWORD n = 0;
      std::string echo;
      while (InternetReadFile(req, buf, sizeof(buf) - 1, &n) && n)
      {
         buf[n] = 0;
         echo += buf;
         n = 0;
      }
      CHECK(echo.find("POST /echo HTTP/1.1\r\n") == 0);
      CHECK(echo.find("\r\nContent-Type: application/x-www-form-urlencoded\r\n") != std::string::npos);
      CHECK(echo.find("\r\nClient-Version: 50.53\r\n") != std::string::npos);
      CHECK(echo.find("\r\nUser-Agent: m59 test\r\n") != std::string::npos);
      CHECK(echo.find("Expect:") == std::string::npos);
      CHECK(echo.find("\r\nContent-Length: 17\r\n") != std::string::npos);
      CHECK(echo.size() > strlen(post) && echo.compare(echo.size() - strlen(post), std::string::npos, post) == 0);
      InternetCloseHandle(req);
   }

   // Redirects are followed; the headers are the final response's.
   {
      HINTERNET req = HttpOpenRequestA(conn, "GET", "moved", NULL, NULL, NULL, 0, 0);
      CHECK(HttpSendRequestA(req, NULL, 0, NULL, 0));
      DWORD status = 0, status_len = sizeof(status);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &status_len, NULL));
      CHECK(status == 200);
      CHECK(ReadAll(req, 2) == "small");
      InternetCloseHandle(req);

      req = HttpOpenRequestA(conn, "GET", "/moved", NULL, NULL, NULL, INTERNET_FLAG_NO_AUTO_REDIRECT, 0);
      CHECK(HttpSendRequestA(req, NULL, 0, NULL, 0));
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &status_len, NULL));
      CHECK(status == 302);
      char loc[64];
      DWORD loc_len = sizeof(loc);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_LOCATION, loc, &loc_len, NULL) && strcmp(loc, "/small%20file.txt") == 0);
      InternetCloseHandle(req);
   }

   // A 404 is a successful request.
   {
      HINTERNET req = HttpOpenRequestA(conn, NULL, "/missing", NULL, NULL, NULL, 0, 0);
      CHECK(HttpSendRequestA(req, NULL, 0, NULL, 0));
      char code[8];
      DWORD code_len = sizeof(code);
      CHECK(HttpQueryInfoA(req, HTTP_QUERY_STATUS_CODE, code, &code_len, NULL) && strcmp(code, "404") == 0);
      CHECK(ReadAll(req, 100) == "nope");
      // Closing the session closes what was made from it.
      CHECK(InternetCloseHandle(session));
      CHECK(!InternetCloseHandle(req) && LastError() == ERROR_INVALID_HANDLE);
      CHECK(!InternetCloseHandle(conn) && LastError() == ERROR_INVALID_HANDLE);
   }

   // Nothing listening.
   closesocket(listener);
   server.join();
   {
      HINTERNET s = InternetOpenA("m59 test", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_RELOAD);
      HINTERNET c = InternetConnectA(s, "127.0.0.1", port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
      HINTERNET req = HttpOpenRequestA(c, NULL, "/", NULL, NULL, NULL, 0, 0);
      CHECK(!HttpSendRequestA(req, NULL, 0, NULL, 0) && LastError() == ERROR_INTERNET_CANNOT_CONNECT);
      c = InternetConnectA(s, "no-such-host.invalid", 0, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
      req = HttpOpenRequestA(c, NULL, "/", NULL, NULL, NULL, 0, 0);
      CHECK(!HttpSendRequestA(req, NULL, 0, NULL, 0) && LastError() == ERROR_INTERNET_NAME_NOT_RESOLVED);
      InternetCloseHandle(s);
   }

   if (const char *target = getenv("M59_NET_TEST_HTTPS"))
   {
      std::string host(target), path = "/";
      size_t slash = host.find('/');
      if (slash != std::string::npos)
      {
         path = host.substr(slash);
         host.resize(slash);
      }
      HINTERNET s = InternetOpenA("Meridian59 Signup Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
      HINTERNET c = InternetConnectA(s, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
      HINTERNET req = HttpOpenRequestA(c, "GET", path.c_str(), 0, 0, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
      CHECK(HttpSendRequestA(req, NULL, 0, NULL, 0));
      DWORD status = 0, status_len = sizeof(status);
      HttpQueryInfoA(req, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &status_len, NULL);
      std::string body = ReadAll(req, 1024);
      printf("https://%s%s: status %lu, %zu bytes\n", host.c_str(), path.c_str(), (unsigned long) status, body.size());
      CHECK(status != 0 && !body.empty());
      InternetCloseHandle(s);
   }

   WSACleanup();
   printf("%s: %d failure(s)\n", failures ? "FAILED" : "PASSED", failures);
   return failures ? 1 : 0;
}
