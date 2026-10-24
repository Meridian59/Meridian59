/*
 * kernel_profile.c:  INI file semantics (GetPrivateProfileString and friends).
 */

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "kernel_check.h"

#define INI "kernel_profile_tmp.ini"

static void Put(const char *text)
{
   FILE *f = fopen(INI, "wb");
   fputs(text, f);
   fclose(f);
}

static const char *Contents(void)
{
   static char buf[4096];
   FILE *f = fopen(INI, "rb");
   if (f == NULL)
      return "<none>";
   size_t n = fread(buf, 1, sizeof(buf) - 1, f);
   buf[n] = 0;
   fclose(f);
   return buf;
}

// A "a\0b\0\0" list as "a|b|".
static const char *List(const char *buf)
{
   static char out[4096];
   out[0] = 0;
   for (const char *p = buf; *p; p += strlen(p) + 1)
   {
      strcat(out, p);
      strcat(out, "|");
   }
   return out;
}

static const char *Get(const char *section, const char *key, const char *def)
{
   static char buf[256];
   memset(buf, 'X', sizeof(buf));
   GetPrivateProfileStringA(section, key, def, buf, sizeof(buf), INI);
   return buf;
}

int main(void)
{
   char buf[256];
   int ret;
   remove(INI);

   // No file: defaults, with trailing spaces trimmed.
   CHECK_STR(Get("s", "k", "def"), "def");
   CHECK_STR(Get("s", "k", "def   "), "def");
   CHECK_STR(Get("s", "k", " "), " ");
   CHECK_STR(Get("s", "k", NULL), "");
   CHECK_STR(Get("s", "k", "\"q\""), "q");
   CHECK_INT(GetPrivateProfileStringA("s", "k", "default", buf, sizeof(buf), INI), 7);
   CHECK_INT(GetPrivateProfileStringA("s", "k", "default", buf, 4, INI), 3);
   CHECK_STR(buf, "def");
   CHECK_INT(GetPrivateProfileStringA("s", "k", "default", buf, 0, INI), 0);
   CHECK_INT(GetPrivateProfileIntA("s", "k", 17, INI), 17);
   ret = GetPrivateProfileStringA(NULL, NULL, "", buf, sizeof(buf), INI);
   CHECK_INT(ret, 0);
   CHECK_STR(Contents(), "<none>");
   // Deleting from a missing file doesn't make one.
   CHECK(WritePrivateProfileStringA("s", "k", NULL, INI));
   CHECK(WritePrivateProfileStringA("s", NULL, NULL, INI));
   CHECK_STR(Contents(), "<none>");

   // Writes make the file and section.
   CHECK(WritePrivateProfileStringA("Sec", "key", "val", INI));
   CHECK_STR(Contents(), "[Sec]\r\nkey=val\r\n");
   CHECK_STR(Get("SEC", "KEY", ""), "val");
   CHECK_STR(Get(" sec ", " key ", ""), "val");
   CHECK(WritePrivateProfileStringA("Sec", "two", "  2", INI));
   CHECK(WritePrivateProfileStringA("sec", "KEY", "new", INI));
   CHECK(WritePrivateProfileStringA("Other", "x", "y", INI));
   CHECK_STR(Contents(), "[Sec]\r\nkey=new\r\ntwo=2\r\n\r\n[Other]\r\nx=y\r\n");
   CHECK(!WritePrivateProfileStringA(NULL, "a", "b", INI));
   CHECK(!WritePrivateProfileStringA(NULL, NULL, NULL, INI));   // flush: FALSE
   CHECK(!WritePrivateProfileStringA("s", "k", "v", "kernel_no_such_dir\\x.ini"));
   CHECK_INT(GetLastError(), ERROR_PATH_NOT_FOUND);

   // Parsing.
   Put("leading=ignored\r\n"
       "[s]\r\n"
       "  name1 =  val1  \r\n"
       "name2=\"quoted\"\r\n"
       "name3\r\n"
       "name4='x'\r\n"
       "name5=\"half\r\n"
       ";comment=1\r\n"
       "empty=\r\n"
       "\r\n"
       "[ s2 ]\r\n"
       "k=v ; not a comment\r\n"
       "[s]\r\n"
       "name1=second\r\n"
       "dup=1\r\n");
   CHECK_STR(Get("s", "name1", ""), "val1");
   CHECK_STR(Get("s", " name1 ", ""), "val1");
   CHECK_STR(Get("s", "name2", ""), "quoted");
   CHECK_STR(Get("s", "name3", "dflt"), "dflt");
   CHECK_STR(Get("s", "name4", ""), "x");
   CHECK_STR(Get("s", "name5", ""), "\"half");
   CHECK_STR(Get("s", "empty", "dflt"), "");
   CHECK_STR(Get("s", "dup", "none"), "none");     // only the first [s] counts
   CHECK_STR(Get("s2", "k", ""), "v ; not a comment");
   CHECK_STR(Get("S2", "K", ""), "v ; not a comment");
   CHECK_STR(Get("s", "", "dflt"), "dflt");
   CHECK_STR(Get("", "leading", "dflt"), "dflt");
   CHECK_STR(Get("nosuch", "k", "dflt"), "dflt");
   CHECK_INT(GetPrivateProfileStringA("s", "name1", "", buf, 3, INI), 2);
   CHECK_STR(buf, "va");
   CHECK_INT(GetPrivateProfileStringA("s", "name1", "", buf, 5, INI), 4);
   CHECK_STR(buf, "val1");
   CHECK_INT(GetPrivateProfileStringA("s", "name2", "", buf, 4, INI), 3);
   CHECK_STR(buf, "quo");

   // Section names.
   ret = GetPrivateProfileStringA(NULL, "ignored", "", buf, sizeof(buf), INI);
   CHECK_INT(ret, 7);
   CHECK(memcmp(buf, "s\0s2\0s\0\0", 8) == 0);
   memset(buf, 'X', sizeof(buf));
   ret = GetPrivateProfileStringA(NULL, NULL, "", buf, 4, INI);
   CHECK_INT(ret, 2);
   CHECK(memcmp(buf, "s\0\0\0", 4) == 0);
   memset(buf, 'X', sizeof(buf));
   ret = GetPrivateProfileStringA(NULL, NULL, "", buf, 5, INI);
   CHECK_INT(ret, 3);
   CHECK(memcmp(buf, "s\0s\0\0", 5) == 0);

   // Key names (comments left out).
   memset(buf, 'X', sizeof(buf));
   ret = GetPrivateProfileStringA("s", NULL, "", buf, sizeof(buf), INI);
   CHECK_STR(List(buf), "name1|name2|name3|name4|name5|empty|");
   CHECK_INT(ret, 36);
   CHECK(buf[36] == 0 && buf[35] == 0);
   memset(buf, 'X', sizeof(buf));
   ret = GetPrivateProfileStringA("s", NULL, "", buf, 10, INI);
   CHECK_INT(ret, 8);
   CHECK(memcmp(buf, "name1\0na\0\0", 10) == 0);
   CHECK_STR(Get("nosuch", NULL, "dflt"), "dflt");
   Put("[empty]\r\n[full]\r\na=1\r\n");
   CHECK_STR(Get("empty", NULL, "dflt"), "dflt");
   CHECK_INT(GetPrivateProfileStringA("full", NULL, "", buf, sizeof(buf), INI), 2);
   CHECK(memcmp(buf, "a\0\0", 3) == 0);

   // GetPrivateProfileSection.
   Put("[s]\r\n"
       "name1=val1\r\n"
       "name2=\r\n"
       "name3\r\n"
       ";comment\r\n"
       "name4 = \"val4\" \r\n"
       "[t]\r\n");
   memset(buf, 'X', sizeof(buf));
   ret = GetPrivateProfileSectionA("s", buf, sizeof(buf), INI);
   CHECK_STR(List(buf), "name1=val1|name2=|name3|name4=\"val4\"|");
   CHECK_INT(ret, 37);
   memset(buf, 'X', sizeof(buf));
   ret = GetPrivateProfileSectionA("S", buf, 12, INI);
   CHECK_INT(ret, 10);
   CHECK(buf[10] == 0 && buf[11] == 0);
   memset(buf, 'X', sizeof(buf));
   CHECK_INT(GetPrivateProfileSectionA("t", buf, sizeof(buf), INI), 0);
   CHECK(buf[0] == 0);
   CHECK_INT(GetPrivateProfileSectionA("none", buf, sizeof(buf), INI), 0);
   CHECK(buf[0] == 0 && buf[1] == 0);
   CHECK_INT(GetPrivateProfileSectionA(NULL, buf, sizeof(buf), INI), 0);

   // alias.c's loop: grow until the result is less than size - 2.
   {
      char text[8192] = "[CommandAliases]\r\n";
      for (int i = 0; i < 200; i++)
         sprintf(text + strlen(text), "verb%d=command number %d\r\n", i, i);
      Put(text);
      int allocated = 1024, returned = 0;
      char *section = NULL;
      for (;;)
      {
         section = (char *) malloc(allocated);
         returned = GetPrivateProfileSectionA("commandaliases", section, allocated, INI);
         if (returned && returned < allocated - 2)
            break;
         free(section);
         allocated = allocated * 4 / 3;
      }
      int count = 0;
      for (char *p = section; *p; p += strlen(p) + 1)
         count++;
      CHECK_INT(count, 200);
      CHECK(strcmp(section, "verb0=command number 0") == 0);
      free(section);
   }

   // GetPrivateProfileInt.
   Put("[n]\r\na=42\r\nb= -7\r\nc=0x1F\r\nd=12abc\r\ne=abc\r\nf=\r\ng=\"5\"\r\nh=+3\r\n");
   CHECK_INT(GetPrivateProfileIntA("n", "a", 1, INI), 42);
   CHECK_INT((INT) GetPrivateProfileIntA("n", "b", 1, INI), -7);
   CHECK_INT(GetPrivateProfileIntA("n", "c", 1, INI), 31);
   CHECK_INT(GetPrivateProfileIntA("n", "d", 1, INI), 12);
   CHECK_INT(GetPrivateProfileIntA("n", "e", 1, INI), 0);
   CHECK_INT(GetPrivateProfileIntA("n", "f", 9, INI), 9);
   CHECK_INT(GetPrivateProfileIntA("n", "g", 1, INI), 5);
   CHECK_INT(GetPrivateProfileIntA("n", "h", 1, INI), 3);
   CHECK_INT(GetPrivateProfileIntA("n", "missing", 77, INI), 77);

   // Edits keep everything else: comments, blank lines, odd lines, CRLF.
   Put("; header comment\r\n"
       "[a]\r\n"
       "x=1\r\n"
       "; about y\r\n"
       "Y = 2\r\n"
       "\r\n"
       "[b]\r\n"
       "z=3\r\n");
   CHECK(WritePrivateProfileStringA("a", "y", "20", INI));
   CHECK_STR(Contents(), "; header comment\r\n[a]\r\nx=1\r\n; about y\r\nY=20\r\n\r\n[b]\r\nz=3\r\n");
   CHECK(WritePrivateProfileStringA("a", "new", "n", INI));
   CHECK_STR(Contents(), "; header comment\r\n[a]\r\nx=1\r\n; about y\r\nY=20\r\nnew=n\r\n\r\n[b]\r\nz=3\r\n");
   CHECK(WritePrivateProfileStringA("b", "w", "4", INI));
   CHECK_STR(Contents(), "; header comment\r\n[a]\r\nx=1\r\n; about y\r\nY=20\r\nnew=n\r\n\r\n[b]\r\nz=3\r\nw=4\r\n");
   CHECK(WritePrivateProfileStringA("a", "X", NULL, INI));
   CHECK_STR(Contents(), "; header comment\r\n[a]\r\n; about y\r\nY=20\r\nnew=n\r\n\r\n[b]\r\nz=3\r\nw=4\r\n");
   CHECK(WritePrivateProfileStringA("a", "nothere", NULL, INI));
   CHECK(WritePrivateProfileStringA("A", NULL, NULL, INI));
   CHECK_STR(Contents(), "; header comment\r\n[b]\r\nz=3\r\nw=4\r\n");
   CHECK_STR(Get("a", "y", "gone"), "gone");
   CHECK_STR(Get("b", "w", ""), "4");
   // Same value: nothing written (and still TRUE).
   CHECK(WritePrivateProfileStringA("b", "w", "4", INI));

   // LF files stay LF; a last line without a line ending gets one.
   Put("[u]\nk=v\n[w]\nq=1");
   CHECK(WritePrivateProfileStringA("u", "k2", "v2", INI));
   CHECK(WritePrivateProfileStringA("w", "r", "2", INI));
   CHECK_STR(Contents(), "[u]\nk=v\nk2=v2\n[w]\nq=1\nr=2\n");
   CHECK_STR(Get("w", "q", ""), "1");

   // WritePrivateProfileSection.
   Put("[CommandAliases]\r\nold=1\r\n;c\r\n\r\n[keep]\r\nk=v\r\n");
   CHECK(WritePrivateProfileSectionA("commandaliases", "\0\0\0", INI));   // as alias.c
   CHECK_STR(Contents(), "[CommandAliases]\r\n\r\n[keep]\r\nk=v\r\n");
   CHECK(WritePrivateProfileStringA("CommandAliases", "g", "go", INI));
   CHECK_STR(Contents(), "[CommandAliases]\r\ng=go\r\n\r\n[keep]\r\nk=v\r\n");
   CHECK(WritePrivateProfileSectionA("CommandAliases", "a=1\0b = 2\0junk\0\0", INI));
   CHECK_STR(Contents(), "[CommandAliases]\r\na=1\r\nb=2\r\n\r\n[keep]\r\nk=v\r\n");
   CHECK(WritePrivateProfileSectionA("fresh", "x=9\0\0", INI));
   CHECK_STR(Get("fresh", "x", ""), "9");
   CHECK(WritePrivateProfileSectionA("fresh", NULL, INI));
   CHECK_STR(Contents(), "[CommandAliases]\r\na=1\r\nb=2\r\n\r\n[keep]\r\nk=v\r\n\r\n");
   CHECK(!WritePrivateProfileSectionA(NULL, NULL, INI));

   // Changes made by others show (the cache follows the file).
   Put("[c]\r\nv=1\r\n");
   CHECK_STR(Get("c", "v", ""), "1");
   Sleep(20);
   Put("[c]\r\nv=22\r\n");
   CHECK_STR(Get("c", "v", ""), "22");
   Sleep(20);
   Put("[c]\r\nv=33\r\n");
   CHECK_STR(Get("c", "v", ""), "33");

   // Windows path separators and case in the file name.
   CHECK(WritePrivateProfileStringA("c", "w", "1", ".\\KERNEL_PROFILE_TMP.INI"));
   CHECK_STR(Get("c", "w", ""), "1");

   // Many reads and writes, as preferences.c.
   remove(INI);
   for (int i = 0; i < 100; i++)
   {
      char key[32], value[32];
      sprintf(key, "key%d", i);
      sprintf(value, "%d", i * 3);
      WritePrivateProfileStringA("config", key, value, INI);
   }
   int ok = 1;
   for (int i = 0; i < 100; i++)
   {
      char key[32];
      sprintf(key, "key%d", i);
      ok &= GetPrivateProfileIntA("config", key, -1, INI) == (UINT) (i * 3);
   }
   CHECK(ok);

   remove(INI);
   return CheckReport("kernel_profile");
}
