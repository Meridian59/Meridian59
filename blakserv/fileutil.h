// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#ifndef _FILEUTIL_H
#define _FILEUTIL_H

// Fill in "files" with the names of all files matching the given pattern.
// extension should include the dot, e.g., ".txt"
// Return true on success.
bool FindMatchingFiles(const char *path, const char *extension, StringVector *files);

#endif
