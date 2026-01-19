// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#ifndef _FILEUTIL_H
#define _FILEUTIL_H

/**
 * RAII wrapper for FILE* that automatically closes the file when it goes out of scope.
 * 
 * This class ensures files are properly closed even if an error occurs or an early return
 * happens, eliminating the need for manual fclose() calls..
 */
class FileGuard
{
   FILE *file;

public:
   FileGuard(FILE *f) : file(f) {}

   // Destructor automatically closes the file if it's still open
   ~FileGuard()
   {
      if (file) fclose(file);
   }

   // Prevent copying
   FileGuard(const FileGuard&) = delete;
   FileGuard& operator=(const FileGuard&) = delete;

   // Allow access to underlying file
   FILE* get() const { return file; }
   FILE* operator->() const { return file; }
   operator FILE*() const { return file; }

   // Allow explicit release
   FILE* release() { FILE *f = file; file = nullptr; return f; }
};

// Fill in "files" with the names of all files matching the given pattern.
// extension should include the dot, e.g., ".txt"
// Return true on success.
bool FindMatchingFiles(const char *path, const char *extension, StringVector *files);

bool BlakMoveFile(const char *source, const char *dest);

#endif
