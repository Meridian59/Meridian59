// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roomanim.h:  Header file for roomanim.c
 */

#ifndef _ROOMANIM_H
#define _ROOMANIM_H

// Number of milliseconds per pixel scrolled for various scrolling texture speeds
#define SCROLL_SLOW_PERIOD       12
#define SCROLL_MEDIUM_PERIOD     6
#define SCROLL_FAST_PERIOD       2

#define SCROLL_WALL_SLOW_PERIOD     96
#define SCROLL_WALL_MEDIUM_PERIOD   32
#define SCROLL_WALL_FAST_PERIOD      8

// Constant to override a BP_SECTOR_CHANGE variable.
#define CHANGE_OVERRIDE 0x00000004

// Number of milliseconds between flickering a light
#define FLICKER_PERIOD             100

// For ANIMATE_ROOM_BITMAP animations
typedef struct {
   Animate a;             // Normal bitmap animation info
   BYTE    action;        // Action associated with animation (codes in proto.h)
} RoomBitmap;

// For ANIMATE_FLOOR_LIFT and ANIMATE_CEILING_LIFT animations
typedef struct {
   int       source_z, dest_z;    // Height where lift starts and stops (FINENESS units)
   int       z;                   // Current height of lift (FINENESS units)
   float     progress;            // 0 = source; 1 = destination
   float     increment;           // Amount by which progress increments per millisecond
} RoomLift;

// For ANIMATE_SCROLL animations
typedef struct {
   int  period;               // Milliseconds between 1-pixel scrolls
   int  tick;                 // Milliseconds remaining to next scroll for this texture
   BYTE direction;            // Direction of texture scrolling
   int  xoffset;              // Amount to shift texture in x direction 
   int  yoffset;              // Amount to shift texture in y direction
} RoomScroll;

// For ANIMATE_FLICKER animations
typedef struct {
   int  period;               // Milliseconds between flickers
   int  tick;                 // Milliseconds remaining to next flicker for this sector
   BYTE light;                // New, flickered sector light level
   BYTE original_light;       // Light level of sector as set in room file
   WORD server_id;            // User id # of sector; used to synchronize flickering
                              // for adjacent sectors
} RoomFlicker;

// Hold state of a room bitmap's animation
typedef struct {
   BYTE animation;   /* How to animate bitmap; constants in animate.h */
   
   union {
      RoomLift    lift;     // Used when animation = ANIMATE_FLOOR_LIFT or ANIMATE_CEILING_LIFT
      RoomBitmap  bitmap;   // Used when animation = ANIMATE_ROOM_BITMAP
      RoomScroll  scroll;   // Used when animation = ANIMATE_SCROLL
      RoomFlicker flicker;  // Used when animation = ANIMATE_FLICKER
   } u;
} RoomAnimate;

void MoveSector(BYTE type, WORD sector_num, WORD height, BYTE speed);
void WallChange(WORD wall_num, Animate *a, BYTE action);
void SectorAnimate(WORD sector_num, Animate *a, BYTE action);
void SectorChange(WORD sector_num, BYTE depth, BYTE scroll);
void TextureChange(WORD id_num, WORD texture_num, BYTE flags);
void SectorFlickerChange(WORD sector_num, BYTE type);

#endif /* #ifndef _ROOMANIM_H */
