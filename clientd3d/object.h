// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * object.h:  Header file for object.c
 */

#ifndef _OBJECT_H
#define _OBJECT_H

typedef DWORD ID;  /* Server id #s */

/* Illegal object id #, used in return values */
#define INVALID_ID ((ID) -1)

/* Top 4 bits of object id can contain tag info for, example if object is a quantity
   (such as money).  Bottom 28 bits contain actual id # */
#define GetObjId(id)  ( (id) & 0x0fffffff)
#define GetObjTag(id) ( ((id) & 0xf0000000) >> 28)
#define IsNumberObj(id) (GetObjTag(id) == CLIENT_TAG_NUMBER)

#define MAX_CHARNAME 30   // Maximum length of character name
#define MAX_DESCRIPTION 1000  /* Maximum length of character description */

// Hold state of an object's animation
typedef struct {
   BYTE animation;   /* How to animate object's bitmap; constants in animate.h */
   int  period;      /* Milliseconds between change of bitmaps */
   int  tick;        /* Milliseconds remaining to next bitmap change for this object */
   WORD group;       /* Current bitmap group being used to display object */
   WORD group_low, group_high; /* Interval used to animate object */
   WORD group_final; /* Group to go to after single-shot animation is done */
} Animate;

// Hold state of object's motion
typedef struct {
   int       x, y, z;                      // Current position in room (FINENESS units)
   int       source_x, source_y, source_z; // Coordinates where travel should start (FINENESS units)
   int       dest_x, dest_y, dest_z;       // Coordinates where travel should stop (FINENESS units)
   int       v_z;                          // Vertical velocity (FINENESS units per second)
                                           // (Used for falling and climbing)
   float     progress;                     // 0 = source; 1 = destination
   float     increment;                    // Amount by which progress increments per millisecond
   float     speed;							// keep history of speed to allow for auto correction
   Animate   animate;                      // Object animation when moving
   list_type overlays;                     // Overlays (and animation) to use when moving
   Bool      move_animating;               // True when move animation is being displayed
   BYTE      translation;                  // Palette translation when object is moving
   BYTE	     effect;			   // Display effect when object is moving
} Motion;

typedef struct {
   ID      icon_res;             // Bitmap to display             
   char    hotspot;              // If nonzero, hotspot to place overlay on
   BYTE    translation;          // Palette translation information
   BYTE	   effect;		 // Display effect (like translucency)
   Animate animate;              // What group to use, and state of animation
} Overlay;

typedef struct
{
	u_short	flags;
	u_short	color;
	u_char	intensity;
} d_lighting;

typedef struct {
   ID           id;
   ID           icon_res;
   ID           name_res;
   DWORD        amount;              /* If top bits of id signify CLIENT_TAG_NUMBER, then
                                        this field gives amount of object */
   DWORD        temp_amount;         /* Scratch field used when user is selecting amount of object */
   int          flags;               // Boolean object flags.
   BYTE         drawingtype;        // Object flags for drawing effects (invisibility, lighting type etc.)
   int          minimapflags;        // Flags used to draw the right color/icon on the minimap.
   unsigned int namecolor;           // Player name color flags
   object_type  objecttype;          /* Enum of object type (i.e. outlaw, murderer, NPC) */
   moveon_type  moveontype;          // MoveOn type of the object
   BYTE         translation;         // Palette translation information
   Animate      *animate;            /* Pointer to current animation (normal or motion animation) */
   list_type    *overlays;           /* Pointer to current overlays (normal or motion animation) */
   Animate      normal_animate;      /* Holds normal (non-motion) animation info */
   list_type    normal_overlays;     /* Bitmaps to be overlaid on this object when drawn */
   BYTE         normal_translation;  // Palette translation when not moving
   BYTE         secondtranslation;   // Overriding, additional second translation.
   WORD         bounceTime;          //
   WORD         phaseTime;           //
   int          boundingHeight;      //
   int          boundingWidth;       //
   int          lightAdjust;         // For flicker and flash
   BYTE         effect;              // Display effect
   d_lighting   dLighting;           // new lighting flags for d3d client
} object_node;

typedef struct {
   object_node obj;
   long        angle;             /* Angle object is facing, in [0, NUMDEGREES) */
   Bool        moving;            /* True iff object in the middle of an interpolated move */
   Motion      motion;            /* Current state of object's motion (includes position) */
   int         distance;          /* Distance from player to object; may not be valid */
   Bool        visible;           // True when object is visible in current frame
   int			boundingHeightAdjust;	// adjustment in height from overlays
} room_contents_node;

M59EXPORT Bool CompareIdObject(void *idnum, void *obj);
Bool CompareId(void *id1, void *id2);
Bool CompareIdRoomObject(void *idnum, void *obj);
int  CompareRoomObjectDistance(void* p1, void* p2);
list_type    OverlayListDestroy(list_type overlays);
M59EXPORT object_node *ObjectGetBlank(void);
M59EXPORT object_node *ObjectCopy(object_node *obj);
M59EXPORT void         ObjectDestroy(object_node *obj);
M59EXPORT object_node *ObjectDestroyAndFree(object_node *obj);
M59EXPORT list_type    ObjectListDestroy(list_type obj_list);
M59EXPORT room_contents_node *RoomObjectDestroy(room_contents_node *r);
room_contents_node *RoomObjectDestroyAndFree(room_contents_node *r);
M59EXPORT list_type    RoomObjectListDestroy(list_type obj_list);
void         RoomObjectSetHeight(room_contents_node *r);
void         ObjectStopAnimation(object_node *obj);
void         RoomObjectSetAnimation(room_contents_node *r, Bool move);

#endif /* #ifndef _OBJECT_H */
