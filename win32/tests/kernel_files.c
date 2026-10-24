/*
 * kernel_files.c:  CreateFile, WriteFile, GetFileSize, DeleteFile, FindFirstFile
 *   patterns, GetCurrentDirectory, file mappings, standard handles.
 */

// Before Windows' headers: macOS's unistd.h declares select with its fd_set,
// which winsock.h replaces when it comes first.
#include <unistd.h>

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "kernel_check.h"

#define DIR_ "kernel_files_tmp"

static void Write(const char *name, const char *text)
{
   HANDLE h = CreateFileA(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   CHECK(h != INVALID_HANDLE_VALUE);
   DWORD n = 0;
   CHECK(WriteFile(h, text, (DWORD) strlen(text), &n, NULL));
   CHECK_INT(n, strlen(text));
   CloseHandle(h);
}

// The names FindFirstFile(spec) finds, ","-joined, in order; "!<error>" if none.
static const char *FindAll(const char *spec)
{
   static char out[1024];
   WIN32_FIND_DATAA fd;
   HANDLE h = FindFirstFileA(spec, &fd);
   if (h == INVALID_HANDLE_VALUE)
   {
      sprintf(out, "!%lu", (unsigned long) GetLastError());
      return out;
   }
   out[0] = 0;
   do
   {
      if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
         continue;
      if (out[0])
         strcat(out, ",");
      strcat(out, fd.cFileName);
   } while (FindNextFileA(h, &fd));
   CHECK_INT(GetLastError(), ERROR_NO_MORE_FILES);
   CHECK(FindClose(h));
   return out;
}

int main(void)
{
   system("rm -rf " DIR_);
   CHECK(mkdir(DIR_) == 0);

   // CreateFile dispositions and errors.
   HANDLE h = CreateFileA(DIR_ "\\missing.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
   CHECK(h == INVALID_HANDLE_VALUE);
   CHECK_INT(GetLastError(), ERROR_FILE_NOT_FOUND);
   h = CreateFileA(DIR_ "\\nodir\\x.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
   CHECK(h == INVALID_HANDLE_VALUE);
   CHECK_INT(GetLastError(), ERROR_PATH_NOT_FOUND);
   h = CreateFileA(DIR_ "\\new.txt", GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
   CHECK(h != INVALID_HANDLE_VALUE);
   CloseHandle(h);
   h = CreateFileA(DIR_ "\\new.txt", GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
   CHECK(h == INVALID_HANDLE_VALUE);
   CHECK_INT(GetLastError(), ERROR_FILE_EXISTS);
   h = CreateFileA(DIR_ "\\new.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
   CHECK(h != INVALID_HANDLE_VALUE);
   CHECK_INT(GetLastError(), ERROR_ALREADY_EXISTS);
   CloseHandle(h);
   h = CreateFileA(DIR_ "\\other.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
   CHECK(h != INVALID_HANDLE_VALUE);
   CHECK_INT(GetLastError(), 0);
   CloseHandle(h);
   h = CreateFileA(DIR_, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
   CHECK(h == INVALID_HANDLE_VALUE);
   CHECK_INT(GetLastError(), ERROR_ACCESS_DENIED);

   // WriteFile, GetFileSize; names ignore case.
   Write(DIR_ "\\Data.BIN", "hello, world");
   h = CreateFileA(DIR_ "\\data.bin", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
   CHECK(h != INVALID_HANDLE_VALUE);
   DWORD high = 99;
   CHECK_INT(GetFileSize(h, &high), 12);
   CHECK_INT(high, 0);
   CHECK_INT(GetFileSize(h, NULL), 12);
   DWORD n = 5;
   CHECK(!WriteFile(h, "x", 1, &n, NULL));   // opened for reading
   CHECK_INT(n, 0);

   // Read-only mapping, as memmap.c.
   HANDLE m = CreateFileMappingA(h, NULL, PAGE_READONLY, 0, 12, NULL);
   CHECK(m != NULL);
   char *p = (char *) MapViewOfFile(m, FILE_MAP_READ, 0, 0, 0);
   CHECK(p != NULL);
   CHECK(p != NULL && memcmp(p, "hello, world", 12) == 0);
   CHECK(MapViewOfFile(m, FILE_MAP_WRITE, 0, 0, 0) == NULL);
   CHECK_INT(GetLastError(), ERROR_ACCESS_DENIED);
   CHECK(MapViewOfFile(m, FILE_MAP_READ, 0, 1, 0) == NULL);
   CHECK_INT(GetLastError(), ERROR_MAPPED_ALIGNMENT);
   CHECK(MapViewOfFile(m, FILE_MAP_READ, 0, 0, 13) == NULL);
   // The view outlives both handles.
   CHECK(CloseHandle(m));
   CHECK(CloseHandle(h));
   CHECK(p != NULL && p[11] == 'd');
   CHECK(UnmapViewOfFile(p));
   CHECK(!UnmapViewOfFile(p));
   CHECK(GetFileSize(h, NULL) == INVALID_FILE_SIZE);
   CHECK_INT(GetLastError(), ERROR_INVALID_HANDLE);
   // A mapping of an empty file fails.
   h = CreateFileA(DIR_ "\\new.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
   CHECK(CreateFileMappingA(h, NULL, PAGE_READONLY, 0, 0, NULL) == NULL);
   CHECK_INT(GetLastError(), ERROR_FILE_INVALID);
   CloseHandle(h);
   // Read-write mapping that grows the file, as util/memmap.c.
   h = CreateFileA(DIR_ "\\rw.bin", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
   m = CreateFileMappingA(h, NULL, PAGE_READWRITE, 0, 100000, NULL);
   CHECK(m != NULL);
   p = (char *) MapViewOfFile(m, FILE_MAP_WRITE, 0, 0, 0);
   CHECK(p != NULL);
   if (p != NULL)
   {
      memcpy(p, "abc", 3);
      p[99999] = 'z';
      CHECK(UnmapViewOfFile(p));
   }
   CHECK_INT(GetFileSize(h, NULL), 100000);
   CloseHandle(m);
   CloseHandle(h);
   FILE *f = fopen(DIR_ "\\rw.bin", "rb");
   char buf[4] = {0};
   CHECK(f != NULL && fread(buf, 1, 3, f) == 3);
   CHECK_STR(buf, "abc");
   if (f) fclose(f);
   // Memory-only mapping.
   m = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, NULL);
   p = (char *) MapViewOfFile(m, FILE_MAP_ALL_ACCESS, 0, 0, 0);
   CHECK(p != NULL && p[0] == 0);
   if (p) { p[4095] = 1; UnmapViewOfFile(p); }
   CloseHandle(m);

   // FindFirstFile patterns.
   CHECK(mkdir(DIR_ "/resource") == 0);
   CHECK(mkdir(DIR_ "/resource/subdir.rsc") == 0);
   Write(DIR_ "\\resource\\b.rsc", "1");
   Write(DIR_ "\\resource\\A.RSC", "22");
   Write(DIR_ "\\resource\\c.rscx", "3");
   Write(DIR_ "\\resource\\noext", "4");
   Write(DIR_ "\\resource\\d.bgf", "5");
   Write(DIR_ "\\resource\\x1.rsc", "6");
   Write(DIR_ "\\resource\\x12.rsc", "7");
   CHECK_STR(FindAll(DIR_ "\\resource\\*.rsc"), "A.RSC,b.rsc,subdir.rsc,x1.rsc,x12.rsc");
   CHECK_STR(FindAll(DIR_ "\\Resource\\*.RSC"), "A.RSC,b.rsc,subdir.rsc,x1.rsc,x12.rsc");
   CHECK_STR(FindAll(DIR_ "/resource/*.rsc"), "A.RSC,b.rsc,subdir.rsc,x1.rsc,x12.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\*.*"), "A.RSC,b.rsc,c.rscx,d.bgf,noext,subdir.rsc,x1.rsc,x12.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\*"), "A.RSC,b.rsc,c.rscx,d.bgf,noext,subdir.rsc,x1.rsc,x12.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\x?.rsc"), "x1.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\x??.rsc"), "x12.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\noext.*"), "noext");
   CHECK_STR(FindAll(DIR_ "\\resource\\b.rsc"), "b.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\B.RSC"), "b.rsc");
   CHECK_STR(FindAll(DIR_ "\\resource\\*.xyz"), "!2");
   CHECK_STR(FindAll(DIR_ "\\nowhere\\*.rsc"), "!3");
   CHECK_STR(FindAll(DIR_ "\\resource\\"), "!2");
   WIN32_FIND_DATAA fd;
   h = FindFirstFileA(DIR_ "\\resource\\a.rsc", &fd);
   CHECK(h != INVALID_HANDLE_VALUE);
   CHECK_STR(fd.cFileName, "A.RSC");
   CHECK_INT(fd.nFileSizeLow, 2);
   CHECK_INT(fd.nFileSizeHigh, 0);
   CHECK(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
   CHECK(fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
   CHECK(fd.ftLastWriteTime.dwHighDateTime > 0x01D00000);   // after 2014
   CHECK(!FindNextFileA(h, &fd));
   CHECK_INT(GetLastError(), ERROR_NO_MORE_FILES);
   CHECK(!CloseHandle(h));   // find handles close with FindClose only
   CHECK(FindClose(h));
   CHECK(!FindClose(h));
   h = FindFirstFileA(DIR_ "\\resource\\subdir.rsc", &fd);
   CHECK(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
   FindClose(h);
   // "." and ".." come first for "*".
   h = FindFirstFileA(DIR_ "\\resource\\*", &fd);
   CHECK_STR(fd.cFileName, ".");
   CHECK(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
   FindNextFileA(h, &fd);
   CHECK_STR(fd.cFileName, "..");
   FindClose(h);

   // DeleteFile.
   CHECK(DeleteFileA(DIR_ "\\RESOURCE\\B.rsc"));
   CHECK(!DeleteFileA(DIR_ "\\resource\\b.rsc"));
   CHECK_INT(GetLastError(), ERROR_FILE_NOT_FOUND);
   CHECK(!DeleteFileA(DIR_ "\\resource\\subdir.rsc"));
   CHECK_INT(GetLastError(), ERROR_ACCESS_DENIED);

   // GetCurrentDirectory: length, or the size needed.
   char cwd[MAX_PATH];
   UINT len = GetCurrentDirectoryA(MAX_PATH, cwd);
   CHECK(len > 0 && len == strlen(cwd));
   CHECK_INT(GetCurrentDirectoryA(0, NULL), len + 1);
   char tiny[2];
   CHECK_INT(GetCurrentDirectoryA(2, tiny), len + 1);
   CHECK_INT(GetCurrentDirectoryA(len, cwd), len + 1);
   CHECK_INT(GetCurrentDirectoryA(len + 1, cwd), len);

   // Standard handles, as client.c's debug console.
   HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
   CHECK(out != INVALID_HANDLE_VALUE && out != NULL);
   CHECK(GetStdHandle(STD_OUTPUT_HANDLE) == out);
   fflush(stdout);
   CHECK(WriteFile(out, "stdout via WriteFile\n", 21, &n, NULL));
   CHECK_INT(n, 21);
   CHECK(GetStdHandle(STD_ERROR_HANDLE) != out);
   CHECK(GetStdHandle(12345) == INVALID_HANDLE_VALUE);
   CHECK(CloseHandle(out));
   HANDLE out2 = GetStdHandle(STD_OUTPUT_HANDLE);
   CHECK(out2 != out);
   CHECK(WriteFile(out2, "still open\n", 11, &n, NULL));

   system("rm -rf " DIR_);
   return CheckReport("kernel_files");
}
