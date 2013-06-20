// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * archive.h:  Header for archive.c
 */

#ifndef _ARCHIVE_H
#define _ARCHIVE_H

void Dearchive(char *dest_path, char *filename);
//bool UnarchiveProgressCallback(const char *filename, ExtractionStatus status);

#endif _ARCHIVE_H
