// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// bbgun.h:  Main include file for BBGun

#ifndef _BBGUN_H
#define _BBGUN_H

#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <memory.h>
#include <stdio.h>
#include <mmsystem.h>
#include <fcntl.h>
#include <io.h>

#include "dibutil.h"
#include "palette.h"
#include "draw.h"
#include "bmparea.h"
#include "bbgfile.h"

#include "resource.h"

#define NUM_COLORS 256       // # of colors in bitmaps
#define LIGHT_LEVELS 256      // # of lighting levels (not used yet)


#define MAX_BITMAPS 150
#define MAX_BBGS 3
#define MAX_GROUPS 25
#define MAX_INDICES 12
#define MAX_HOTSPOTS 15
#define MAX_COMMENT 4096

#define ZOOM_INITIAL 1
#define ZOOM_MAX 10

typedef struct {
   int Lap;   // Neg Underlap, Pos Overlap
   int X;
   int Y;
} Spot;

typedef struct {
   int    XOffset;  // X offset to use when displaying this bitmap over another one
   int    YOffset;  // Y offset to use when displaying this bitmap over another one
   BYTE   NumHotspots;   // # of hotspots in bitmap
   Spot   Hotspots[MAX_HOTSPOTS]; // Hotspot positions; tell where overlay bitmaps should be placed
   char   Title[50];
   PDIB   pdib;     // Pointer to bitmap info
} Bitmap;

typedef struct {
   int Members;
   int Indices[MAX_INDICES];
} Group;


typedef struct {
   char   FullPath[MAX_PATH];
   char   Title[50];
   int    NumBitmaps;
   int    NumGroups;
   Bitmap Bitmaps[MAX_BITMAPS];
   Group  Groups[MAX_GROUPS];
   BYTE   Shrink;   // Divide by this to get actual size of object
   int    CurrentBitmap;
   int    CurrentHotspot;
   int    CurrentGroup;
   char   Comment[MAX_COMMENT];
   BOOL   changed;  // TRUE when some part of BBG has been changed
} BBGdata;

typedef struct {
   BOOL  dual_mode;            // Is dual bitmap mode on?
   BOOL  show_hotspots;        // Show any hotspots?
   BOOL  all_hotspots;         // Show all hotspots?
   BOOL  transparent;          // Draw bitmaps transparently?
} Config;

typedef struct
{
   int BBG;
   int HotNum;
   int Lap;
} GraphSpot;

extern LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);

extern void UpdateHotspot(void);
extern void UpdateOffset(void);
extern void UpdateOffset2(void);
extern int  GetPrimaryBBG(void);
extern int  ListBoxRemoveCurrentItem(HWND hList);

// From bbgun.c
void FillHotlist(void);
void GroupInsertBitmap(int bitmap_index);
void FillBitmapList(void);
void FillBitmapList2(void);
void FillGroupList(void);
void FillGroupiesList(void);
void GroupAddBitmap(void);
void GroupRemoveBitmap(void);
void GroupCreate(void);
void GroupRemove(void);
void CreateBBG(void);
void BitmapLoad(void);
void BitmapUnload(void);
void CreateHotspot(void);
void RemoveHotspot(void);
void DrawIt(void);

extern Config config;
extern HINSTANCE hInst;
extern HWND   hMain;
extern BBGdata BBGs[MAX_BBGS];

extern GraphSpot WanderOne;
extern GraphSpot Anchor;

extern char *szAppName;

#endif // _BBGUN_H
