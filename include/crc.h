// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#ifndef _CRC_H
#define _CRC_H

unsigned int CRC32(const char *ptr, int len);
unsigned int CRC32Incremental(unsigned int crc, const char *ptr, int len);

#endif
