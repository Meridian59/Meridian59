// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * makebgf.h:  Header file for makebgf.c
 */

#ifndef _MAKEBGF_H
#define _MAKEBGF_H

#include <windows.h>
#include <windowsx.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include "zlib.h"

#include "dibutil.h"

typedef unsigned char Bool;
enum {False = 0, True = 1};

#define MAX_BITMAPNAME 32

#define NUM_COLORS 256  // Require bitmaps to have exactly this many colors

typedef struct {
   int   num_indices;   // Number of bitmaps in this group
   int  *indices;       // Indices of bitmaps in this group.  Counts from 0.
} Group;

typedef struct {
   BYTE   num_hotspots;
   char  *numbers;
   POINT *positions;
} Hotspots;

typedef struct {
   char     name[MAX_BITMAPNAME];  // String name of bitmap
   int      num_bitmaps;   // Number of bitmaps
   PDIB     *bitmaps;      // Array of bitmap pointers
   POINT    *offsets;      // Array of offsets, one offset per bitmap
   Hotspots *hotspots;     // Array of hotspot locations, one set of locations per bitmap
   int       num_groups;   // Number of bitmap groups
   Group    *groups;       // Array of groups
} Bitmaps;

// Command line option parameters
typedef struct {
   int  shrink;
   Bool rotate;
   Bool compress;
} Options;

void Error(char *fmt,...);

#include "writebgf.h"

#endif /* #ifndef _MAKEBGF_H */
