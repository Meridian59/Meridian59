/*
 * profile.cpp:  INI files (GetPrivateProfileString and friends), with Windows'
 *   semantics as Wine's tests pin them down: names compare ignoring case and
 *   surrounding space, NULL names enumerate, quotes around values are stripped,
 *   truncated lists return size - 2.  Edits change only the lines they touch:
 *   comments, blank lines, line endings (CRLF or LF) and unrelated lines stay.
 *   Parsed files are cached and re-read when their stat changes; every write
 *   goes to disk at once.  A file name without a directory is relative to the
 *   current directory (Windows looks in the Windows directory).
 */

#include "core/kernel/kernel.h"
#include "core/paths.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <string>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace w32 {

namespace {

enum class LineKind { Blank, Section, Key };

struct Line
{
   std::string text;   // without its line ending
   std::string eol;    // "\r\n", "\n", or "" (last line without one)
   LineKind kind = LineKind::Blank;
   std::string name;   // section or key name, trimmed
   std::string value;  // trimmed
   bool has_value = false;
};

struct Stamp
{
   bool exists = false;
   dev_t dev = 0;
   ino_t ino = 0;
   off_t size = 0;
   int64_t mtime_ns = 0;
   bool operator==(const Stamp &) const = default;
};

struct IniFile
{
   std::vector<Line> lines;
   std::string eol = "\r\n";   // for new lines: the file's own
   bool bom = false;            // starts with a UTF-8 byte order mark
   Stamp stamp;
};

std::mutex ini_lock;
std::map<std::string, IniFile> &Cache()
{
   static std::map<std::string, IniFile> cache;
   return cache;
}

bool IsSpace(char c)
{
   return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == 0x1a;
}

std::string Trim(const char *begin, const char *end)
{
   while (begin < end && IsSpace(*begin))
      begin++;
   while (end > begin && IsSpace(end[-1]))
      end--;
   return std::string(begin, end);
}

std::string Trim(const char *s)
{
   return Trim(s, s + strlen(s));
}

void Classify(Line &line)
{
   const char *begin = line.text.c_str(), *end = begin + line.text.size();
   while (begin < end && IsSpace(*begin))
      begin++;
   line.name.clear();
   line.value.clear();
   line.has_value = false;
   if (begin == end)
   {
      line.kind = LineKind::Blank;
      return;
   }
   if (*begin == '[')
   {
      const char *close = end - 1;
      while (close > begin && *close != ']')
         close--;
      if (close > begin)
      {
         line.kind = LineKind::Section;
         line.name = Trim(begin + 1, close);
         return;
      }
      // No ']': an ordinary line, as on Windows.
   }
   line.kind = LineKind::Key;
   const char *eq = static_cast<const char *>(memchr(begin, '=', end - begin));
   if (eq == nullptr)
   {
      line.name = Trim(begin, end);
      return;
   }
   line.name = Trim(begin, eq);
   line.value = Trim(eq + 1, end);
   line.has_value = true;
}

Stamp StatFile(const std::string &path)
{
   Stamp s;
   struct stat st;
   if (::stat(path.c_str(), &st) != 0)
      return s;
   s.exists = true;
   s.dev = st.st_dev;
   s.ino = st.st_ino;
   s.size = st.st_size;
#ifdef __APPLE__
   s.mtime_ns = (int64_t) st.st_mtimespec.tv_sec * 1000000000 + st.st_mtimespec.tv_nsec;
#else
   s.mtime_ns = (int64_t) st.st_mtim.tv_sec * 1000000000 + st.st_mtim.tv_nsec;
#endif
   return s;
}

void Parse(IniFile &ini, const std::string &data)
{
   ini.lines.clear();
   ini.eol = "\r\n";
   bool eol_known = false;
   // A UTF-8 byte order mark is not part of the first line's text.
   ini.bom = data.compare(0, 3, "\xEF\xBB\xBF") == 0;
   size_t pos = ini.bom ? 3 : 0;
   while (pos < data.size())
   {
      size_t nl = data.find('\n', pos);
      size_t end = nl == std::string::npos ? data.size() : nl;
      size_t text_end = end > pos && data[end - 1] == '\r' ? end - 1 : end;
      Line line;
      line.text = data.substr(pos, text_end - pos);
      if (nl != std::string::npos)
      {
         line.eol = data.substr(text_end, nl + 1 - text_end);
         if (!eol_known)
         {
            ini.eol = line.eol;
            eol_known = true;
         }
      }
      pos = nl == std::string::npos ? data.size() : nl + 1;
      Classify(line);
      ini.lines.push_back(std::move(line));
   }
}

// The parsed file for host path (empty when it doesn't exist).
IniFile &Load(const std::string &path)
{
   Stamp stamp = StatFile(path);
   auto it = Cache().find(path);
   if (it != Cache().end() && it->second.stamp == stamp)
      return it->second;
   IniFile &ini = Cache()[path];
   ini = IniFile();
   ini.stamp = stamp;
   if (!stamp.exists)
      return ini;
   std::string data;
   int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
   if (fd >= 0)
   {
      char buf[65536];
      ssize_t n;
      while ((n = ::read(fd, buf, sizeof(buf))) > 0 || (n < 0 && errno == EINTR))
         if (n > 0)
            data.append(buf, n);
      ::close(fd);
   }
   Parse(ini, data);
   return ini;
}

bool Save(const std::string &path, IniFile &ini)
{
   std::string data = ini.bom ? "\xEF\xBB\xBF" : "";
   for (size_t i = 0; i < ini.lines.size(); i++)
   {
      Line &line = ini.lines[i];
      if (line.eol.empty() && i + 1 < ini.lines.size())
         line.eol = ini.eol;
      data += line.text;
      data += line.eol;
   }
   int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
   bool ok = fd >= 0;
   int error = errno;
   for (size_t done = 0; ok && done < data.size();)
   {
      ssize_t n = ::write(fd, data.data() + done, data.size() - done);
      if (n < 0 && errno == EINTR)
         continue;
      if (n < 0)
      {
         error = errno;
         ok = false;
      }
      else
         done += n;
   }
   if (fd >= 0)
      ::close(fd);
   if (!ok)
   {
      Cache().erase(path);   // not what is on disk
      SetLastError(error == ENOENT ? ERROR_PATH_NOT_FOUND : ErrorFromErrno(error));
      return false;
   }
   ini.stamp = StatFile(path);
   return true;
}

std::string IniPath(LPCSTR file)
{
   return HostPath(file != nullptr ? file : "win.ini");
}

bool NameIs(const std::string &a, const std::string &b)
{
   return strcasecmp(a.c_str(), b.c_str()) == 0;
}

// Index of the first header of section, or -1.
long FindSection(const IniFile &ini, const std::string &section)
{
   for (size_t i = 0; i < ini.lines.size(); i++)
      if (ini.lines[i].kind == LineKind::Section && NameIs(ini.lines[i].name, section))
         return (long) i;
   return -1;
}

// One past the last line of the section whose header is at index.
size_t SectionEnd(const IniFile &ini, size_t header)
{
   size_t i = header + 1;
   while (i < ini.lines.size() && ini.lines[i].kind != LineKind::Section)
      i++;
   return i;
}

// Where a new line of the section goes: after its last non-blank line, so the
// blank lines before the next section stay there.
size_t SectionInsertPoint(const IniFile &ini, size_t header)
{
   size_t end = SectionEnd(ini, header);
   while (end > header + 1 && ini.lines[end - 1].kind == LineKind::Blank)
      end--;
   return end;
}

long FindKey(const IniFile &ini, size_t header, const std::string &key)
{
   size_t end = SectionEnd(ini, header);
   for (size_t i = header + 1; i < end; i++)
      if (ini.lines[i].kind == LineKind::Key && NameIs(ini.lines[i].name, key))
         return (long) i;
   return -1;
}

bool IsComment(const std::string &name) { return !name.empty() && name[0] == ';'; }

Line MakeLine(const std::string &text, const std::string &eol)
{
   Line line;
   line.text = text;
   line.eol = eol;
   Classify(line);
   return line;
}

// value into buffer (len chars with the NUL), without enclosing quotes when strip.
void CopyEntry(char *buffer, const char *value, size_t len, bool strip)
{
   if (len == 0)
      return;
   size_t n = strlen(value);
   if (strip && n > 1 && (value[0] == '"' || value[0] == '\'') && value[n - 1] == value[0])
   {
      value++;
      n -= 2;
   }
   if (n > len - 1)
      n = len - 1;
   memcpy(buffer, value, n);
   buffer[n] = 0;
}

// Section names, "a\0b\0\0"; size - 2 when they don't fit.
int SectionNames(const IniFile &ini, char *buffer, size_t len)
{
   if (len == 0)
      return 0;
   if (len == 1)
   {
      *buffer = 0;
      return 0;
   }
   size_t free = len - 1;
   char *p = buffer;
   for (const Line &line : ini.lines)
   {
      if (line.kind != LineKind::Section || line.name.empty())
         continue;
      size_t n = line.name.size() + 1;
      if (n >= free)
      {
         if (free > 0)
         {
            memcpy(p, line.name.data(), free - 1);
            p += free - 1;
            *p++ = 0;
         }
         *p = 0;
         return (int) len - 2;
      }
      memcpy(p, line.name.c_str(), n);
      p += n;
      free -= n;
   }
   *p = 0;
   return (int) (p - buffer);
}

// The section's keys ("a\0b\0\0") or lines ("a=1\0b\0\0", with values); size - 2
// when they don't fit.  Comments (";...") are left out.
int SectionEntries(const IniFile &ini, size_t header, char *buffer, size_t len, bool values)
{
   if (len < 2)
   {
      if (len == 1)
         *buffer = 0;
      return 0;
   }
   size_t oldlen = len;
   char *p = buffer;
   size_t end = SectionEnd(ini, header);
   for (size_t i = header + 1; i < end; i++)
   {
      const Line &line = ini.lines[i];
      if (line.kind != LineKind::Key || line.name.empty() || IsComment(line.name))
         continue;
      if (len <= 2)
         break;
      CopyEntry(p, line.name.c_str(), len - 1, false);
      size_t n = strlen(p) + 1;
      len -= n;
      p += n;
      if (len < 2)
         break;
      if (values && line.has_value)
      {
         p[-1] = '=';
         CopyEntry(p, line.value.c_str(), len - 1, false);
         n = strlen(p) + 1;
         len -= n;
         p += n;
      }
   }
   *p = 0;
   if (len <= 1)
   {
      p[-1] = 0;
      return (int) oldlen - 2;
   }
   if (p == buffer)
      p[1] = 0;   // an empty list is "\0\0" too
   return (int) (oldlen - len);
}

// Sets (or with value NULL deletes) key in section; with key NULL deletes the
// section.  append: add the key even if the section has one (section writes).
// True if the file changed.
bool SetString(IniFile &ini, const std::string &section, const char *key, const char *value, bool append)
{
   long header = FindSection(ini, section);
   if (key == nullptr)
   {
      if (header < 0)
         return false;
      ini.lines.erase(ini.lines.begin() + header, ini.lines.begin() + SectionEnd(ini, header));
      return true;
   }
   std::string k = Trim(key);
   if (value == nullptr)
   {
      if (header < 0)
         return false;
      long at = FindKey(ini, header, k);
      if (at < 0)
         return false;
      ini.lines.erase(ini.lines.begin() + at);
      return true;
   }
   while (IsSpace(*value))
      value++;
   if (header < 0)
   {
      if (!ini.lines.empty() && ini.lines.back().kind != LineKind::Blank)
         ini.lines.push_back(MakeLine("", ini.eol));   // a blank line between sections
      ini.lines.push_back(MakeLine("[" + section + "]", ini.eol));
      header = (long) ini.lines.size() - 1;
   }
   long at = append ? -1 : FindKey(ini, header, k);
   if (at >= 0)
   {
      Line &line = ini.lines[at];
      if (line.has_value && line.value == value)
         return false;
      line = MakeLine(line.name + "=" + value, line.eol);
      return true;
   }
   ini.lines.insert(ini.lines.begin() + SectionInsertPoint(ini, header), MakeLine(k + "=" + value, ini.eol));
   return true;
}

}  // namespace

}  // namespace w32

using namespace w32;

extern "C" {

INT WINAPI GetPrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR def, LPSTR buffer, UINT size,
                                    LPCSTR file)
{
   if (buffer == nullptr || size == 0)
      return 0;
   // Trailing spaces of the default don't count.
   std::string def_value = def != nullptr ? def : "";
   while (def_value.size() > 1 && def_value.back() == ' ')
      def_value.pop_back();

   std::lock_guard<std::mutex> lock(ini_lock);
   std::string path = IniPath(file);
   const IniFile &ini = Load(path);
   if (section == nullptr)
      return SectionNames(ini, buffer, size);

   std::string s = Trim(section);
   if (key != nullptr)
   {
      std::string k = Trim(key);
      const char *value = def_value.c_str();
      long header = k.empty() ? -1 : FindSection(ini, s);
      long at = header < 0 ? -1 : FindKey(ini, header, k);
      if (at >= 0 && ini.lines[at].has_value)
         value = ini.lines[at].value.c_str();
      CopyEntry(buffer, value, size, true);
      return (int) strlen(buffer);
   }

   // Key names; the default when there are none.
   long header = s.empty() ? -1 : FindSection(ini, s);
   if (header >= 0)
   {
      int n = SectionEntries(ini, header, buffer, size, false);
      if (buffer[0] != 0)
         return n;
   }
   else if (s.empty())
   {
      buffer[0] = 0;
      return 0;
   }
   CopyEntry(buffer, def_value.c_str(), size, true);
   return (int) strlen(buffer);
}

UINT WINAPI GetPrivateProfileIntA(LPCSTR section, LPCSTR key, INT def, LPCSTR file)
{
   char buf[30];
   if (GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), file) == 0 || buf[0] == 0)
      return (UINT) def;
   // As RtlCharToInteger with base 0: sign, then 0x/0o/0b, then digits up to the
   // first that isn't one; nothing valid is 0.
   const char *p = buf;
   while (*p != 0 && (unsigned char) *p <= ' ')
      p++;
   bool negative = false;
   if (*p == '+' || *p == '-')
      negative = *p++ == '-';
   unsigned base = 10;
   if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
      base = 16, p += 2;
   else if (p[0] == '0' && (p[1] == 'o' || p[1] == 'O'))
      base = 8, p += 2;
   else if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B'))
      base = 2, p += 2;
   UINT v = 0;
   for (;; p++)
   {
      unsigned d;
      if (*p >= '0' && *p <= '9')
         d = *p - '0';
      else if (*p >= 'a' && *p <= 'z')
         d = *p - 'a' + 10;
      else if (*p >= 'A' && *p <= 'Z')
         d = *p - 'A' + 10;
      else
         break;
      if (d >= base)
         break;
      v = v * base + d;
   }
   return negative ? 0u - v : v;
}

INT WINAPI GetPrivateProfileSectionA(LPCSTR section, LPSTR buffer, DWORD size, LPCSTR file)
{
   if (section == nullptr || buffer == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }
   if (size == 0)
      return 0;
   std::lock_guard<std::mutex> lock(ini_lock);
   std::string path = IniPath(file);
   const IniFile &ini = Load(path);
   long header = FindSection(ini, Trim(section));
   if (header < 0)
   {
      buffer[0] = 0;
      if (size > 1)
         buffer[1] = 0;
      return 0;
   }
   return SectionEntries(ini, header, buffer, size, true);
}

BOOL WINAPI WritePrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR value, LPCSTR file)
{
   std::lock_guard<std::mutex> lock(ini_lock);
   if (section == nullptr && key == nullptr && value == nullptr)
   {
      // Flush the cache: writes are never held back, so there is nothing to do
      // but forget it.  Windows returns FALSE here.
      Cache().erase(IniPath(file));
      return FALSE;
   }
   if (section == nullptr)
   {
      SetLastError(ERROR_FILE_NOT_FOUND);
      return FALSE;
   }
   std::string path = IniPath(file);
   IniFile &ini = Load(path);
   if (!SetString(ini, Trim(section), key, value, false))
      return TRUE;
   return Save(path, ini) ? TRUE : FALSE;
}

BOOL WINAPI WritePrivateProfileSectionA(LPCSTR section, LPCSTR entries, LPCSTR file)
{
   std::lock_guard<std::mutex> lock(ini_lock);
   if (section == nullptr && entries == nullptr)
   {
      Cache().erase(IniPath(file));
      return FALSE;
   }
   if (section == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   std::string path = IniPath(file);
   IniFile &ini = Load(path);
   std::string s = Trim(section);
   bool changed;
   if (entries == nullptr)
      changed = SetString(ini, s, nullptr, nullptr, false);
   else
   {
      // Replace the section's lines (keeping the blank lines that end it).
      changed = false;
      long header = FindSection(ini, s);
      if (header >= 0)
      {
         size_t begin = header + 1, end = SectionInsertPoint(ini, header);
         if (end > begin)
         {
            ini.lines.erase(ini.lines.begin() + begin, ini.lines.begin() + end);
            changed = true;
         }
      }
      // "key=value\0...\0\0"; entries without '=' are skipped.
      for (const char *p = entries; *p != 0; p += strlen(p) + 1)
      {
         const char *eq = strchr(p, '=');
         if (eq == nullptr)
            continue;
         std::string k(p, eq);
         changed = SetString(ini, s, k.c_str(), eq + 1, true) || changed;
      }
   }
   if (!changed)
      return TRUE;
   return Save(path, ini) ? TRUE : FALSE;
}

}  // extern "C"
