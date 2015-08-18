// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * files.c
 *
 */

#include "blakserv.h"

bool FindMatchingFiles(const char *path, std::vector<std::string> *files)
{
 #ifdef BLAK_PLATFORM_WINDOWS
   
	HANDLE hFindFile;
	WIN32_FIND_DATA search_data;

   files->clear();
   hFindFile = FindFirstFile(path, &search_data);
	if (hFindFile == INVALID_HANDLE_VALUE)
      return false;
   
   do
   {
      files->push_back(search_data.cFileName);
   } while (FindNextFile(hFindFile,&search_data));
   FindClose(hFindFile);

   return true;
   
 #elif BLAK_PLATFORM_LINUX

   struct dirent *entry;
   DIR *dir = opendir(path); // XXX Need to parse out path name
   if (dir == NULL)
      return false;

   while (entry = readdir(dir))
   {
      std::string filename = entry->d_name;
      if (filename != "." && filename != "..")
         files->push_back(filename);
   }
   
   closedir(dir);

   // XXX Need to pattern match
   
   return true;
   
 #else
   #error No platform implementation of FindMatchingFiles
 #endif
}

bool BlakMoveFile(const char *source, const char *dest)
{
 #ifdef BLAK_PLATFORM_WINDOWS
   
   if (!CopyFile(source,dest,FALSE))
   {
      eprintf("BlakMoveFile error moving %s to %s (%s)\n",source,dest,GetLastErrorStr());
      return false;
   }
   if (!DeleteFile(source))
   {
      eprintf("BlakMoveFile error deleting %s (%s)\n",source,GetLastErrorStr());
      return false;
   }
   return true;
   
 #elif BLAK_PLATFORM_LINUX

   // Doesn't work across filesystems, but probably fine for our purposes.
   return 0 == rename(source, dest);
   
 #else
   #error No platform implementation of BlakMoveFile
 #endif
}
