// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * files.h
 *
 */

#ifndef _FILES_H
#define _FILES_H

#include <string>
#include <vector>

// Fill in "files" with the names of all files matching the given pattern.
// Return true on success.
typedef std::vector<std::string> StringVector;
bool FindMatchingFiles(const char *path, StringVector *files);

bool BlakMoveFile(const char *source, const char *dest);

#endif
