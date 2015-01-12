// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * object3d.h:  Header file for object3d.c
 */

#ifndef _OBJECT3D_H
#define _OBJECT3D_H

// Return values for FindHotspot
enum {
   HOTSPOT_ANY        = -1,  // Ignore over/under; match any hotspot
   HOTSPOT_NONE       = 0,
   HOTSPOT_OVER       = 1,   // Overlay on an object
   HOTSPOT_UNDER      = 2,   // Underlay on an object
   HOTSPOT_OVERUNDER  = 3,   // Underlay on an overlay
   HOTSPOT_OVEROVER   = 4,   // Overlay on an overlay
   HOTSPOT_UNDERUNDER = 5,   // Underlay on an underlay
   HOTSPOT_UNDEROVER  = 6,   // Over on an underlay
};

// Factor by which overlay offsets are magnified to give extra resolution
#define OVERLAY_FACTOR 64

#define MAX_NAME_DISTANCE (15 * FINENESS)  // Max distance for which we display obj name

/* Where an object was drawn on screen; useful to know this for coincidence testing later */
typedef struct {
   ID   id;                    /* ID of object */
   long left_col, right_col;
   long top_row, bottom_row;
   int  distance;              /* Distance between object and viewer, in FINENESS units */
} ObjectRange;

// Options to DrawObjectBitmap
typedef struct {
   PDIB         pdib;              // Object bitmap
   int          distance;          // Distance to object in FINENESS units
   BYTE         light;             // Light level to draw object
   Bool         draw;              // True if object should actually be drawn (False = just compute location)
   ViewCone     *cone;             // Cone in which to draw object
   int          flags;             // Boolean object flags.
   BYTE         drawingtype;      // Object flags for drawing effects (invisibility, lighting type etc.)
   int          minimapflags;      // Flag field for minimap dot drawing
   unsigned int namecolor;         /* Player name color flags */
   object_type  objecttype;        /* Enum of object type (i.e. outlaw, murderer, NPC) */
   moveon_type  moveontype;        // MoveOn type of the object
   int          cutoff;            // Last screen row in which to draw object (to cut off at ground level)
   BYTE         translation;       // Color translation type, 0 = none
   BYTE         secondtranslation; // Overriding second translation for all overlays.
   BYTE         effect;
   room_contents_node *obj;         // Pointer to room_contents_node for object
} DrawObjectInfo;

#define NAME_COLOR_NORMAL_BG   PALETTERGB(0, 0, 0)

BOOL DrawObject3D(DrawnObject *object, ViewCone *clip);
Bool DrawObjectBitmap( DrawObjectInfo *dos, AREA *area, Bool bTargetSelectEffect );
int  FindHotspot(list_type overlays, PDIB pdib, PDIB pdib_ov, char hotspot, 
		 int angle, POINT *point);
Bool GetObjectSize(ID icon_res, int group, int angle, list_type overlays, int *width, int *height);
void DrawObjectDecorations(DrawnObject *object);
Bool FindOverlayArea(PDIB pdib_ov, int angle, char hotspot, PDIB pdib_obj, 
		     list_type overlays, int overlay_depth, AREA *overlay_area);

#endif /* #ifndef _OBJECT3D_H */
