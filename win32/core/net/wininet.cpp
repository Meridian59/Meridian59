/*
 * wininet.cpp:  WinINet's synchronous HTTP(S) API on libcurl.  A request runs on
 *   a curl multi handle that HttpSendRequest drives until the response headers
 *   are in and InternetReadFile drives as the body is read, so large downloads
 *   stream (the body buffer is bounded by pausing the transfer).
 */

// Wine's winsock.h under WS_ names: curl.h brings in the C library's sockets.
#define USE_WS_PREFIX
#include "core/win32.h"
#include "core/handles.h"
#include "core/net/net.h"

#include <wininet.h>

#include <curl/curl.h>

#include <algorithm>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>
#include <strings.h>

namespace {

using w32::net::SetError;

constexpr size_t kMaxBuffered = 1 << 20;   // body bytes held before pausing

struct Internet
{
   enum Kind { Session, Connection, Request } kind;
   HINTERNET handle = nullptr;
   Internet *parent = nullptr;
   std::vector<HINTERNET> children;

   // Session
   std::string agent, proxy, proxy_bypass;
   DWORD access = INTERNET_OPEN_TYPE_PRECONFIG;

   // Connection
   std::string server, user, password;
   INTERNET_PORT port = 0;
   DWORD service = INTERNET_SERVICE_HTTP;

   // Request
   std::string verb, object, version, referrer;
   std::vector<std::string> accept;
   DWORD flags = 0;
   std::vector<std::string> request_headers;   // as sent, "Name: value"
   std::string body;                           // the request's optional data

   CURL *easy = nullptr;
   CURLM *multi = nullptr;
   curl_slist *header_list = nullptr;
   bool sent = false, done = false, paused = false;
   CURLcode result = CURLE_OK;
   std::string status_line;                    // of the last response
   std::vector<std::string> response_headers;  // its header lines
   std::string data;                           // body received, not yet read
   size_t data_pos = 0;

   ~Internet() { Reset(); }

   void Reset()
   {
      if (multi != nullptr && easy != nullptr)
         curl_multi_remove_handle(multi, easy);
      if (easy != nullptr)
         curl_easy_cleanup(easy);
      if (multi != nullptr)
         curl_multi_cleanup(multi);
      if (header_list != nullptr)
         curl_slist_free_all(header_list);
      easy = nullptr;
      multi = nullptr;
      header_list = nullptr;
      sent = done = paused = false;
      result = CURLE_OK;
      status_line.clear();
      response_headers.clear();
      data.clear();
      data_pos = 0;
   }

   size_t Buffered() const { return data.size() - data_pos; }
};

w32::HandleTable<Internet, HINTERNET> &Table()
{
   static w32::HandleTable<Internet, HINTERNET> table(w32::kInternetHandleBase);
   return table;
}

// Guards the parent/child links (handles are opened and closed on any thread).
std::recursive_mutex tree_mutex;

std::string Str(LPCSTR s)
{
   return s != nullptr ? std::string(s) : std::string();
}

Internet *Get(HINTERNET h, Internet::Kind kind)
{
   Internet *object = Table().Get(h);
   if (object == nullptr)
   {
      SetError(ERROR_INVALID_HANDLE);
      return nullptr;
   }
   if (object->kind != kind)
   {
      SetError(ERROR_INTERNET_INCORRECT_HANDLE_TYPE);
      return nullptr;
   }
   return object;
}

HINTERNET Add(Internet *object, Internet *parent)
{
   std::lock_guard<std::recursive_mutex> lock(tree_mutex);
   object->parent = parent;
   object->handle = Table().Add(object);
   if (parent != nullptr)
      parent->children.push_back(object->handle);
   return object->handle;
}

// Closing a handle closes the handles made from it, as WinINet does.
bool Close(HINTERNET h)
{
   std::lock_guard<std::recursive_mutex> lock(tree_mutex);
   Internet *object = Table().Remove(h);
   if (object == nullptr)
      return false;
   std::vector<HINTERNET> children;
   children.swap(object->children);
   for (HINTERNET child : children)
      Close(child);
   if (object->parent != nullptr)
   {
      auto &siblings = object->parent->children;
      siblings.erase(std::remove(siblings.begin(), siblings.end(), h), siblings.end());
   }
   delete object;
   return true;
}

DWORD InternetError(CURLcode code)
{
   switch (code)
   {
   case CURLE_OK:                    return ERROR_SUCCESS;
   case CURLE_UNSUPPORTED_PROTOCOL:  return ERROR_INTERNET_UNRECOGNIZED_SCHEME;
   case CURLE_URL_MALFORMAT:         return ERROR_INTERNET_INVALID_URL;
   case CURLE_COULDNT_RESOLVE_PROXY:
   case CURLE_COULDNT_RESOLVE_HOST:  return ERROR_INTERNET_NAME_NOT_RESOLVED;
   case CURLE_COULDNT_CONNECT:       return ERROR_INTERNET_CANNOT_CONNECT;
   case CURLE_OPERATION_TIMEDOUT:    return ERROR_INTERNET_TIMEOUT;
   case CURLE_TOO_MANY_REDIRECTS:    return ERROR_HTTP_REDIRECT_FAILED;
   case CURLE_GOT_NOTHING:
   case CURLE_WEIRD_SERVER_REPLY:    return ERROR_HTTP_INVALID_SERVER_RESPONSE;
   case CURLE_SEND_ERROR:
   case CURLE_RECV_ERROR:
   case CURLE_PARTIAL_FILE:          return ERROR_INTERNET_CONNECTION_RESET;
   case CURLE_SSL_CONNECT_ERROR:     return ERROR_INTERNET_SECURITY_CHANNEL_ERROR;
   case CURLE_PEER_FAILED_VERIFICATION:
   case CURLE_SSL_CACERT_BADFILE:
   case CURLE_SSL_CERTPROBLEM:       return ERROR_INTERNET_SEC_CERT_ERRORS;
   case CURLE_ABORTED_BY_CALLBACK:   return ERROR_INTERNET_OPERATION_CANCELLED;
   case CURLE_OUT_OF_MEMORY:         return ERROR_NOT_ENOUGH_MEMORY;
   default:                          return ERROR_INTERNET_INTERNAL_ERROR;
   }
}

size_t OnHeader(char *buffer, size_t size, size_t count, void *user)
{
   auto *r = static_cast<Internet *>(user);
   size_t n = size * count;
   std::string line(buffer, n);
   while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
      line.pop_back();
   if (line.compare(0, 5, "HTTP/") == 0)
   {
      // A new response (after a 1xx or a redirect): its headers replace the last.
      r->status_line = line;
      r->response_headers.clear();
   }
   else if (!line.empty())
      r->response_headers.push_back(line);
   return n;
}

size_t OnData(char *buffer, size_t size, size_t count, void *user)
{
   auto *r = static_cast<Internet *>(user);
   if (r->Buffered() >= kMaxBuffered)
   {
      r->paused = true;
      return CURL_WRITEFUNC_PAUSE;   // curl delivers this data again on resume
   }
   if (r->data_pos > 0 && r->data_pos == r->data.size())
   {
      r->data.clear();
      r->data_pos = 0;
   }
   r->data.append(buffer, size * count);
   return size * count;
}

// Runs the transfer until ready() or it ends.
template <class F> void Pump(Internet *r, F ready)
{
   while (!r->done && !ready())
   {
      if (r->paused && r->Buffered() < kMaxBuffered)
      {
         r->paused = false;
         curl_easy_pause(r->easy, CURLPAUSE_CONT);
         if (ready())
            break;
      }
      int running = 0;
      curl_multi_perform(r->multi, &running);
      int left;
      while (CURLMsg *m = curl_multi_info_read(r->multi, &left))
         if (m->msg == CURLMSG_DONE)
         {
            r->done = true;
            r->result = m->data.result;
         }
      if (r->done || ready())
         break;
      if (r->paused)
         break;   // nothing to wait for until the reader takes data
      curl_multi_poll(r->multi, nullptr, 0, 1000, nullptr);
   }
}

bool HasHeader(const std::vector<std::string> &headers, const char *name)
{
   size_t len = std::strlen(name);
   for (const std::string &h : headers)
      if (h.size() > len && h[len] == ':' && ::strncasecmp(h.c_str(), name, len) == 0)
         return true;
   return false;
}

// WinINet sends the object name with spaces and non-ASCII bytes escaped.
std::string EscapePath(const std::string &object)
{
   static const char hex[] = "0123456789ABCDEF";
   std::string out;
   if (object.empty() || object[0] != '/')
      out += '/';
   for (unsigned char c : object)
      if (c <= ' ' || c >= 0x7f)
      {
         out += '%';
         out += hex[c >> 4];
         out += hex[c & 15];
      }
      else
         out += (char) c;
   return out;
}

// The value of header name's index-th occurrence in lines, or false.
bool FindHeader(const std::vector<std::string> &lines, const std::string &name, DWORD index, std::string &value)
{
   DWORD seen = 0;
   for (const std::string &h : lines)
   {
      size_t colon = h.find(':');
      if (colon == std::string::npos || colon != name.size() ||
          ::strncasecmp(h.c_str(), name.c_str(), colon) != 0)
         continue;
      if (seen++ < index)
         continue;
      size_t start = h.find_first_not_of(" \t", colon + 1);
      value = start == std::string::npos ? std::string() : h.substr(start);
      return true;
   }
   return false;
}

const char *HeaderName(DWORD query)
{
   switch (query)
   {
   case HTTP_QUERY_MIME_VERSION:              return "Mime-Version";
   case HTTP_QUERY_CONTENT_TYPE:              return "Content-Type";
   case HTTP_QUERY_CONTENT_TRANSFER_ENCODING: return "Content-Transfer-Encoding";
   case HTTP_QUERY_CONTENT_ID:                return "Content-ID";
   case HTTP_QUERY_CONTENT_DESCRIPTION:       return "Content-Description";
   case HTTP_QUERY_CONTENT_LENGTH:            return "Content-Length";
   case HTTP_QUERY_CONTENT_LANGUAGE:          return "Content-Language";
   case HTTP_QUERY_ALLOW:                     return "Allow";
   case HTTP_QUERY_PUBLIC:                    return "Public";
   case HTTP_QUERY_DATE:                      return "Date";
   case HTTP_QUERY_EXPIRES:                   return "Expires";
   case HTTP_QUERY_LAST_MODIFIED:             return "Last-Modified";
   case HTTP_QUERY_MESSAGE_ID:                return "Message-ID";
   case HTTP_QUERY_URI:                       return "URI";
   case HTTP_QUERY_DERIVED_FROM:              return "Derived-From";
   case HTTP_QUERY_COST:                      return "Cost";
   case HTTP_QUERY_LINK:                      return "Link";
   case HTTP_QUERY_PRAGMA:                    return "Pragma";
   case HTTP_QUERY_CONNECTION:                return "Connection";
   case HTTP_QUERY_ACCEPT:                    return "Accept";
   case HTTP_QUERY_ACCEPT_CHARSET:            return "Accept-Charset";
   case HTTP_QUERY_ACCEPT_ENCODING:           return "Accept-Encoding";
   case HTTP_QUERY_ACCEPT_LANGUAGE:           return "Accept-Language";
   case HTTP_QUERY_AUTHORIZATION:             return "Authorization";
   case HTTP_QUERY_CONTENT_ENCODING:          return "Content-Encoding";
   case HTTP_QUERY_FORWARDED:                 return "Forwarded";
   case HTTP_QUERY_FROM:                      return "From";
   case HTTP_QUERY_IF_MODIFIED_SINCE:         return "If-Modified-Since";
   case HTTP_QUERY_LOCATION:                  return "Location";
   case HTTP_QUERY_ORIG_URI:                  return "Orig-Uri";
   case HTTP_QUERY_REFERER:                   return "Referer";
   case HTTP_QUERY_RETRY_AFTER:               return "Retry-After";
   case HTTP_QUERY_SERVER:                    return "Server";
   case HTTP_QUERY_TITLE:                     return "Title";
   case HTTP_QUERY_USER_AGENT:                return "User-Agent";
   case HTTP_QUERY_WWW_AUTHENTICATE:          return "WWW-Authenticate";
   case HTTP_QUERY_PROXY_AUTHENTICATE:        return "Proxy-Authenticate";
   case HTTP_QUERY_ACCEPT_RANGES:             return "Accept-Ranges";
   case HTTP_QUERY_SET_COOKIE:                return "Set-Cookie";
   case HTTP_QUERY_COOKIE:                    return "Cookie";
   case HTTP_QUERY_REFRESH:                   return "Refresh";
   case HTTP_QUERY_CONTENT_DISPOSITION:       return "Content-Disposition";
   case HTTP_QUERY_AGE:                       return "Age";
   case HTTP_QUERY_CACHE_CONTROL:             return "Cache-Control";
   case HTTP_QUERY_CONTENT_BASE:              return "Content-Base";
   case HTTP_QUERY_CONTENT_LOCATION:          return "Content-Location";
   case HTTP_QUERY_CONTENT_MD5:               return "Content-MD5";
   case HTTP_QUERY_CONTENT_RANGE:             return "Content-Range";
   case HTTP_QUERY_ETAG:                      return "ETag";
   case HTTP_QUERY_HOST:                      return "Host";
   case HTTP_QUERY_IF_MATCH:                  return "If-Match";
   case HTTP_QUERY_IF_NONE_MATCH:             return "If-None-Match";
   case HTTP_QUERY_IF_RANGE:                  return "If-Range";
   case HTTP_QUERY_IF_UNMODIFIED_SINCE:       return "If-Unmodified-Since";
   case HTTP_QUERY_MAX_FORWARDS:              return "Max-Forwards";
   case HTTP_QUERY_PROXY_AUTHORIZATION:       return "Proxy-Authorization";
   case HTTP_QUERY_RANGE:                     return "Range";
   case HTTP_QUERY_TRANSFER_ENCODING:         return "Transfer-Encoding";
   case HTTP_QUERY_UPGRADE:                   return "Upgrade";
   case HTTP_QUERY_VARY:                      return "Vary";
   case HTTP_QUERY_VIA:                       return "Via";
   case HTTP_QUERY_WARNING:                   return "Warning";
   case HTTP_QUERY_EXPECT:                    return "Expect";
   case HTTP_QUERY_PROXY_CONNECTION:          return "Proxy-Connection";
   case HTTP_QUERY_UNLESS_MODIFIED_SINCE:     return "Unless-Modified-Since";
   default:                                   return nullptr;
   }
}

void InitCurl()
{
   static std::once_flag once;
   std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

// Sets up r's transfer and runs it until the response headers are in.
bool Send(Internet *r)
{
   Internet *conn = r->parent;
   Internet *session = conn->parent;
   bool secure = (r->flags & INTERNET_FLAG_SECURE) != 0;
   INTERNET_PORT port = conn->port != INTERNET_INVALID_PORT_NUMBER
                           ? conn->port
                           : (secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
   std::string host = conn->server;
   if (host.find(':') != std::string::npos && host[0] != '[')
      host = "[" + host + "]";
   std::string url = std::string(secure ? "https://" : "http://") + host + ":" + std::to_string(port) +
                     EscapePath(r->object);

   r->easy = curl_easy_init();
   r->multi = curl_multi_init();
   if (r->easy == nullptr || r->multi == nullptr)
   {
      SetError(ERROR_NOT_ENOUGH_MEMORY);
      return false;
   }
   CURL *e = r->easy;
   curl_easy_setopt(e, CURLOPT_URL, url.c_str());
   curl_easy_setopt(e, CURLOPT_NOSIGNAL, 1L);
   curl_easy_setopt(e, CURLOPT_HTTP_VERSION,
                    r->version == "HTTP/1.0" ? (long) CURL_HTTP_VERSION_1_0 : (long) CURL_HTTP_VERSION_1_1);
   curl_easy_setopt(e, CURLOPT_FOLLOWLOCATION, (r->flags & INTERNET_FLAG_NO_AUTO_REDIRECT) ? 0L : 1L);
   curl_easy_setopt(e, CURLOPT_MAXREDIRS, 30L);
   curl_easy_setopt(e, CURLOPT_CONNECTTIMEOUT, 60L);
   // WinINet's receive timeout: give up when nothing arrives for a minute.
   curl_easy_setopt(e, CURLOPT_LOW_SPEED_LIMIT, 1L);
   curl_easy_setopt(e, CURLOPT_LOW_SPEED_TIME, 60L);
   if (!session->agent.empty())
      curl_easy_setopt(e, CURLOPT_USERAGENT, session->agent.c_str());
   if (session->access == INTERNET_OPEN_TYPE_DIRECT)
      curl_easy_setopt(e, CURLOPT_NOPROXY, "*");
   else if (session->access == INTERNET_OPEN_TYPE_PROXY && !session->proxy.empty())
   {
      curl_easy_setopt(e, CURLOPT_PROXY, session->proxy.c_str());
      if (!session->proxy_bypass.empty())
         curl_easy_setopt(e, CURLOPT_NOPROXY, session->proxy_bypass.c_str());
   }
   if (!conn->user.empty())
   {
      curl_easy_setopt(e, CURLOPT_USERNAME, conn->user.c_str());
      curl_easy_setopt(e, CURLOPT_PASSWORD, conn->password.c_str());
   }
   if (!r->referrer.empty())
      curl_easy_setopt(e, CURLOPT_REFERER, r->referrer.c_str());
   if (r->flags & INTERNET_FLAG_IGNORE_CERT_CN_INVALID)
      curl_easy_setopt(e, CURLOPT_SSL_VERIFYHOST, 0L);

   // Only the headers WinINet sends: none of curl's defaults it lacks.
   for (const std::string &h : r->request_headers)
      r->header_list = curl_slist_append(r->header_list, h.c_str());
   if (!HasHeader(r->request_headers, "Accept"))
   {
      std::string accept = "Accept:";
      for (size_t i = 0; i < r->accept.size(); i++)
         accept += (i == 0 ? " " : ", ") + r->accept[i];
      r->header_list = curl_slist_append(r->header_list, accept.c_str());
   }
   if (!HasHeader(r->request_headers, "Expect"))
      r->header_list = curl_slist_append(r->header_list, "Expect:");
   if (!HasHeader(r->request_headers, "Content-Type"))
      r->header_list = curl_slist_append(r->header_list, "Content-Type:");
   curl_easy_setopt(e, CURLOPT_HTTPHEADER, r->header_list);

   std::string verb = r->verb.empty() ? "GET" : r->verb;
   if (::strcasecmp(verb.c_str(), "HEAD") == 0)
      curl_easy_setopt(e, CURLOPT_NOBODY, 1L);
   else if (::strcasecmp(verb.c_str(), "GET") == 0 && r->body.empty())
      curl_easy_setopt(e, CURLOPT_HTTPGET, 1L);
   else
   {
      if (::strcasecmp(verb.c_str(), "POST") == 0 || !r->body.empty())
      {
         curl_easy_setopt(e, CURLOPT_POSTFIELDS, r->body.data());
         curl_easy_setopt(e, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t) r->body.size());
      }
      if (::strcasecmp(verb.c_str(), "POST") != 0)
         curl_easy_setopt(e, CURLOPT_CUSTOMREQUEST, verb.c_str());
   }

   curl_easy_setopt(e, CURLOPT_HEADERFUNCTION, OnHeader);
   curl_easy_setopt(e, CURLOPT_HEADERDATA, r);
   curl_easy_setopt(e, CURLOPT_WRITEFUNCTION, OnData);
   curl_easy_setopt(e, CURLOPT_WRITEDATA, r);
   curl_multi_add_handle(r->multi, e);
   r->sent = true;

   // The headers are complete once the body starts (curl passes on only the
   // final response's body) or the transfer ends.
   Pump(r, [r] { return r->Buffered() > 0 || r->paused; });
   if (r->done && r->result != CURLE_OK && r->status_line.empty())
   {
      DWORD error = InternetError(r->result);
      r->Reset();
      SetError(error);
      return false;
   }
   return true;
}

// Copies value to a string query's buffer, WinINet's way.
BOOL ReturnString(const std::string &value, LPVOID buffer, LPDWORD length)
{
   if (buffer == nullptr || *length < value.size() + 1)
   {
      *length = (DWORD) value.size() + 1;
      SetError(ERROR_INSUFFICIENT_BUFFER);
      return FALSE;
   }
   std::memcpy(buffer, value.c_str(), value.size() + 1);
   *length = (DWORD) value.size();
   return TRUE;
}

}  // namespace

HINTERNET WINAPI InternetOpenA(LPCSTR agent, DWORD access, LPCSTR proxy, LPCSTR proxy_bypass, DWORD flags)
{
   InitCurl();
   auto *s = new Internet;
   s->kind = Internet::Session;
   s->agent = Str(agent);
   s->access = access;
   s->proxy = Str(proxy);
   s->proxy_bypass = Str(proxy_bypass);
   s->flags = flags;
   return Add(s, nullptr);
}

HINTERNET WINAPI InternetConnectA(HINTERNET session, LPCSTR server, INTERNET_PORT port, LPCSTR user,
                                  LPCSTR password, DWORD service, DWORD flags, DWORD_PTR)
{
   std::lock_guard<std::recursive_mutex> lock(tree_mutex);
   Internet *s = Get(session, Internet::Session);
   if (s == nullptr)
      return nullptr;
   if (server == nullptr || *server == 0)
   {
      SetError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }
   if (service != INTERNET_SERVICE_HTTP)
   {
      SetError(ERROR_INTERNET_INVALID_OPERATION);   // FTP and gopher are not here
      return nullptr;
   }
   auto *c = new Internet;
   c->kind = Internet::Connection;
   c->server = server;
   c->port = port;
   c->user = Str(user);
   c->password = Str(password);
   c->service = service;
   c->flags = flags;
   return Add(c, s);
}

HINTERNET WINAPI HttpOpenRequestA(HINTERNET connection, LPCSTR verb, LPCSTR object, LPCSTR version,
                                  LPCSTR referrer, LPCSTR *accept_types, DWORD flags, DWORD_PTR)
{
   std::lock_guard<std::recursive_mutex> lock(tree_mutex);
   Internet *c = Get(connection, Internet::Connection);
   if (c == nullptr)
      return nullptr;
   auto *r = new Internet;
   r->kind = Internet::Request;
   r->verb = verb != nullptr && *verb ? verb : "GET";
   r->object = Str(object);
   r->version = Str(version);
   r->referrer = Str(referrer);
   if (accept_types != nullptr)
      for (LPCSTR *t = accept_types; *t != nullptr; t++)
         if (**t)
            r->accept.push_back(*t);
   r->flags = flags;
   return Add(r, c);
}

BOOL WINAPI HttpSendRequestA(HINTERNET request, LPCSTR headers, DWORD headers_length, LPVOID optional,
                             DWORD optional_length)
{
   Internet *r = Get(request, Internet::Request);
   if (r == nullptr)
      return FALSE;
   // Sending again starts the request over.
   r->Reset();
   r->request_headers.clear();
   if (headers != nullptr)
   {
      std::string all = headers_length == (DWORD) -1 ? std::string(headers) : std::string(headers, headers_length);
      size_t start = 0;
      while (start < all.size())
      {
         size_t end = all.find('\n', start);
         if (end == std::string::npos)
            end = all.size();
         std::string line = all.substr(start, end - start);
         while (!line.empty() && (line.back() == '\r' || line.back() == 0))
            line.pop_back();
         if (!line.empty())
            r->request_headers.push_back(line);
         start = end + 1;
      }
   }
   r->body.assign(static_cast<const char *>(optional), optional != nullptr ? optional_length : 0);
   return Send(r) ? TRUE : FALSE;
}

BOOL WINAPI InternetReadFile(HINTERNET file, LPVOID buffer, DWORD size, LPDWORD read)
{
   Internet *r = Get(file, Internet::Request);
   if (r == nullptr)
      return FALSE;
   if (read == nullptr || (buffer == nullptr && size > 0))
   {
      SetError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   *read = 0;
   if (!r->sent)
   {
      SetError(ERROR_INTERNET_INCORRECT_HANDLE_STATE);
      return FALSE;
   }
   // A synchronous read fills the buffer, or returns what is left at the end.
   auto *out = static_cast<char *>(buffer);
   DWORD copied = 0;
   while (copied < size)
   {
      size_t n = std::min<size_t>(r->Buffered(), size - copied);
      std::memcpy(out + copied, r->data.data() + r->data_pos, n);
      r->data_pos += n;
      copied += (DWORD) n;
      if (r->data_pos == r->data.size())
      {
         r->data.clear();
         r->data_pos = 0;
      }
      if (copied == size || r->done)
         break;
      Pump(r, [r] { return r->Buffered() > 0; });
      if (r->Buffered() == 0 && r->done)
         break;
   }
   *read = copied;
   if (copied == 0 && r->done && r->result != CURLE_OK)
   {
      SetError(InternetError(r->result));
      return FALSE;
   }
   return TRUE;
}

BOOL WINAPI HttpQueryInfoA(HINTERNET request, DWORD level, LPVOID buffer, LPDWORD length, LPDWORD index)
{
   Internet *r = Get(request, Internet::Request);
   if (r == nullptr)
      return FALSE;
   if (length == nullptr)
   {
      SetError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   DWORD query = level & HTTP_QUERY_HEADER_MASK;
   bool want_request = (level & HTTP_QUERY_FLAG_REQUEST_HEADERS) != 0;
   if (!want_request && r->status_line.empty())
   {
      SetError(ERROR_INTERNET_INCORRECT_HANDLE_STATE);
      return FALSE;
   }
   const std::vector<std::string> &lines = want_request ? r->request_headers : r->response_headers;

   // "HTTP/1.1 200 OK"
   std::string version, code, text;
   {
      const std::string &s = r->status_line;
      size_t sp1 = s.find(' ');
      version = s.substr(0, sp1);
      if (sp1 != std::string::npos)
      {
         size_t sp2 = s.find(' ', sp1 + 1);
         code = s.substr(sp1 + 1, sp2 == std::string::npos ? std::string::npos : sp2 - sp1 - 1);
         if (sp2 != std::string::npos)
            text = s.substr(sp2 + 1);
      }
   }

   std::string value;
   bool header = false;   // a header line (index applies), not the status line
   switch (query)
   {
   case HTTP_QUERY_STATUS_CODE:
      value = code;
      break;
   case HTTP_QUERY_STATUS_TEXT:
      value = text;
      break;
   case HTTP_QUERY_VERSION:
      value = version;
      break;
   case HTTP_QUERY_REQUEST_METHOD:
      value = r->verb;
      break;
   case HTTP_QUERY_RAW_HEADERS:
   case HTTP_QUERY_RAW_HEADERS_CRLF:
   {
      bool crlf = query == HTTP_QUERY_RAW_HEADERS_CRLF;
      std::string sep = crlf ? std::string("\r\n") : std::string(1, '\0');
      if (!want_request)
         value = r->status_line + sep;
      for (const std::string &h : lines)
         value += h + sep;
      value += sep;
      if (!crlf)
      {
         // NUL-separated lines; the trailing NUL ReturnString adds ends the list.
         value.pop_back();
      }
      break;
   }
   default:
   {
      std::string name;
      if (query == HTTP_QUERY_CUSTOM)
      {
         if (buffer == nullptr)
         {
            SetError(ERROR_INVALID_PARAMETER);
            return FALSE;
         }
         name = static_cast<const char *>(buffer);
      }
      else if (const char *n = HeaderName(query))
         name = n;
      else
      {
         SetError(ERROR_HTTP_INVALID_QUERY_REQUEST);
         return FALSE;
      }
      if (!FindHeader(lines, name, index != nullptr ? *index : 0, value))
      {
         SetError(ERROR_HTTP_HEADER_NOT_FOUND);
         return FALSE;
      }
      header = true;
      break;
   }
   }

   if (level & HTTP_QUERY_FLAG_SYSTEMTIME)
   {
      SetError(ERROR_HTTP_INVALID_QUERY_REQUEST);
      return FALSE;
   }
   if (level & HTTP_QUERY_FLAG_NUMBER)
   {
      if (buffer == nullptr || *length < sizeof(DWORD))
      {
         *length = sizeof(DWORD);
         SetError(ERROR_INSUFFICIENT_BUFFER);
         return FALSE;
      }
      DWORD number = (DWORD) std::strtoul(value.c_str(), nullptr, 10);
      std::memcpy(buffer, &number, sizeof(number));
      *length = sizeof(DWORD);
   }
   else if (!ReturnString(value, buffer, length))
      return FALSE;
   if (header && index != nullptr)
      (*index)++;
   return TRUE;
}

BOOL WINAPI InternetCloseHandle(HINTERNET handle)
{
   if (!Close(handle))
   {
      SetError(ERROR_INVALID_HANDLE);
      return FALSE;
   }
   return TRUE;
}
