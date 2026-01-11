// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#include "blakserv.h"

namespace fs = std::filesystem;

bool FindMatchingFiles(const char *path, const char *extension, StringVector *files)
{
  files->clear();
  
  if (!fs::is_directory(path))
  {
    eprintf("Tried to enumerate contents of bad directory %s\n", path);
    return false;
  }

  try {
    for (const auto& entry : fs::directory_iterator(path))
    {
      if (entry.is_regular_file() && entry.path().extension() == extension)
      {
        files->push_back(entry.path().filename().string());
      }
    }
  } catch (const fs::filesystem_error& e) {
    eprintf("Error enumerating contents of directory %s: %s\n", path, e.what());
    return false;
  }
    
  return true;
}

bool BlakMoveFile(const char *source, const char *dest)
{
  try {
    fs::rename(source, dest);
  } catch (const fs::filesystem_error& e) {
    eprintf("Error moving file %s: %s\n", source, e.what());
    return false;
  }
  
  return true;
}
