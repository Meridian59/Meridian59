// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * object3d.c:  Draw objects in 3D view.
 *
 * During the walk of the BSP tree, when an object is encountered, the BSP code calls 
 * GetObjectSize to compute the total size of an object and its overlays.  This size information
 * is used to split the object among view cones.
 *
 * When an object is to be drawn, the BSP code calls DrawObject3D.  This procedure first computes
 * the screen rectangle bounding the object's main bitmap.  First the underlays are drawn, then
 * the object's main bitmap, and then the overlays.  As each object and its overlays are drawn,
 * we build up a bounding rectangle for the entire object.  This rectangle is used elsewhere
 * to determine which objects cover which areas on the screen.
 *
 * Each overlay and underlay can itself have overlays an underlays placed on it.  There are thus
 * seven possible levels of depth in an object bitmap (underlay on an underlay, underlay on
 * main bitmap, overlay on underlay, main bitmap, underlay on overlay, overlay on main bitmap,
 * overlay on overlay).
 *
 * To produce the "wading" effect, where an object is drawn extending under the floor, we keep
 * a bottom screen "cutoff" row, below which the object and its overlays are not drawn.  This
 * cutoff is kept separately from the object rectangle, because the size of the object rectangle
 * determines the scale of the object, which doesn't change for the wading effect.
 *
 * Various possible drawing effects are possible per object.  We define a table of inner loop
 * functions, indexed on the drawing effect.  Depending on an object's effect, the correct
 * inner loop is called to actually put the pixels on the screen.
 */

#include "client.h"

// Convert bitmap size to FINENESS units (BITMAP_WIDTH pixels = FINENESS world size)
#define BitmapToFineness(x) (FINENESS * (x) / BITMAP_WIDTH)

/* Drawing parameters passed in */
extern AREA area;                  /* size and position of view window */
extern HDC  gBitsDC;
extern BYTE *gBits;                /* Actual pixels of bitmap */
extern player_info player;
extern long viewer_height;      /* Viewer's height above floor, in fine coordinates */

extern long horizon;

extern Color base_palette[];         

static RECT orect;  /* Hold rectangle actually occupied by last object drawn */

/* Keep track of visible objects for client */
int num_visible_objects;
ObjectRange visible_objects[MAXOBJECTS];    /* Where objects are on screen */

// Inner loops for drawing objects with special effects
extern DrawingLoop drawing_loops[];

/* local function prototypes */
static void SortObjects(DrawnObject *objects, int *indices, int max_object);
static void ComputeObjectArea(PDIB pdib, int center, int distance, int height, AREA *a, int *cutoff);
static void ComputeOverlayArea(int distance, AREA *obj, int obj_stretch, AREA *ov, int ov_stretch);
int  FindHotspotPdib(PDIB pdib, char hotspot, POINT *point);
static Bool DrawObjectOverlays( DrawObjectInfo *dos, list_type overlays, PDIB pdib_obj,
			       AREA *obj_area, int angle, Bool underlays, Bool bTargetSelectEffect );
/************************************************************************/
/*
 * DrawObject3D:  Draw the single object described by the given DrawnObject structure.
 * Clip object drawing to cone (Not required, but helpful)
 */
BOOL DrawObject3D(DrawnObject *object, ViewCone *clip)
{
	long angle, center;
	PDIB pdib_obj;
	int  object_stretch;  // Factor to scale bitmap down when drawing it
	AREA object_area;     // Area in which to draw object
	DrawObjectInfo dos;   // Info for drawing object
	Bool visible, temp;
	int cutoff;           // Last row of object to be drawn on screen.

	Bool bShowTargetSelectEffect = FALSE;

	ZeroMemory(&dos,sizeof(dos));

	dos.obj = object->obj;
	/* get stuff computed in drawbsp.c */
	dos.distance = object->distance;
	angle = object->angle;
	center = object->center;

	/* Get object's bitmap */
	pdib_obj = GetObjectPdib(object->icon_res, angle, object->group);

	if (pdib_obj == NULL)
	    return FALSE;  /* Can't load bitmap */
	object_stretch = DibShrinkFactor(pdib_obj);

	cutoff = object->depth;
	ComputeObjectArea(pdib_obj, center, dos.distance, object->height, &object_area, &cutoff);

	// Set up object rectangle
	orect.left   = area.cx;
	orect.right  = 0;
	orect.top    = area.cy;
	orect.bottom = 0;

	// Draw underlays
	dos.light  = object->light;
	dos.draw   = object->draw;
	dos.cone   = clip;
	dos.flags = object->flags;
	dos.drawingtype  = object->drawingtype;
	dos.minimapflags  = object->minimapflags;
	dos.namecolor = object->namecolor;
	dos.objecttype = object->objecttype;
	dos.moveontype = object->moveontype;
	dos.translation = object->translation;
	dos.secondtranslation = object->secondtranslation;

   if( object->id != INVALID_ID && object->id == GetUserTargetID() )
	{
      bShowTargetSelectEffect = TRUE;
	}

	for(;;) 	//	Do twice if bShowTargetSelectEffect and there are overlays (so that inner halo lines get erased by 2nd pass).
	{
		dos.cutoff = cutoff;
		visible = False;
		if (object->overlays != NULL)
		{
			temp = DrawObjectOverlays( &dos, object->overlays, pdib_obj, &object_area, angle, True, bShowTargetSelectEffect );
			visible = visible || temp;
		}

		dos.pdib        = pdib_obj;
		dos.translation = object->translation;
		dos.effect      = 0;

		temp = DrawObjectBitmap( &dos, &object_area, bShowTargetSelectEffect );
		visible = visible || temp;

		// Draw overlays
		if (object->overlays != NULL)
		{
			temp = DrawObjectOverlays( &dos, object->overlays, pdib_obj, &object_area, angle, False, bShowTargetSelectEffect );
			visible = visible || temp;
		}
		if( object->overlays == NULL || !bShowTargetSelectEffect )
			break;
		bShowTargetSelectEffect = FALSE;
	}

	if (!visible)
		return FALSE;

	/* Save list of visible objects for client (but skip projectiles) */
	/* An object can be drawn in several pieces, so we need to assemble its rectangle */
	if (object->id != INVALID_ID)
	{
		ObjectRange *range = FindVisibleObjectById(object->id);

		if (range == NULL)
		{
			// Set up new visible object
			range = &visible_objects[num_visible_objects];
			range->id       = object->id;
			range->distance = dos.distance;
			range->left_col   = area.cx;
			range->right_col  = 0;
			range->top_row    = area.cy;
			range->bottom_row = 0;
			num_visible_objects++;
		}

		/* Record boundaries of drawing area */
		range->left_col   = min(range->left_col, orect.left);
		range->right_col  = max(range->right_col, orect.right);
		range->top_row    = min(range->top_row, orect.top);
		range->bottom_row = max(range->bottom_row, orect.bottom);
	}

	return TRUE;
}

/************************************************************************/
/*
 * DrawObjectOverlays:  Draw overlays on object whose bitmap is dos->pdib.
 *   angle is angle that object is being viewed at.
 *   If underlays is True, draw only those overlays which should be drawn
 *     before the object is drawn.
 *   Return True iff at least part of one overlay is visible.
 */
Bool DrawObjectOverlays( DrawObjectInfo *dos, list_type overlays, PDIB pdib_obj,
			AREA *obj_area, int angle, Bool underlays, Bool bTargetSelectEffect )
{
	list_type l;
	AREA overlay_area;
	int pass;  // Which pass of overlays are we on; 0 = under; 1 = normal; 2 = over
	int depth; // Current overlay depth to match
	Bool visible, temp;

	visible = False;
	for (pass = 0; pass < 3; pass++)
	{
		if (underlays)
			switch (pass)
			{
			case 0: depth = HOTSPOT_UNDERUNDER;  break;
			case 1: depth = HOTSPOT_UNDER;       break;
			case 2: depth = HOTSPOT_UNDEROVER;   break;
			}
		else
			switch (pass)
			{
			case 0: depth = HOTSPOT_OVERUNDER;  break;
			case 1: depth = HOTSPOT_OVER;       break;
			case 2: depth = HOTSPOT_OVEROVER;   break;
			}

		for (l = overlays; l != NULL; l = l->next)
		{
			Overlay *overlay = (Overlay *) (l->data);
			PDIB pdib_ov;

			pdib_ov = GetObjectPdib(overlay->icon_res, angle, overlay->animate.group);
			if (pdib_ov == NULL)
				continue;

			// debug(("checking icon %d, depth = %d\n", overlay->icon_res, depth));

			if (!FindOverlayArea(pdib_ov, angle, overlay->hotspot, pdib_obj, overlays, depth, &overlay_area))
				continue;

			ComputeOverlayArea(dos->distance, obj_area, DibShrinkFactor(pdib_obj), 
								&overlay_area, DibShrinkFactor(pdib_ov));
			dos->pdib        = pdib_ov;
			dos->translation = overlay->translation;
			dos->effect = overlay->effect;

			// debug(("drawing icon %d, depth = %d\n", overlay->icon_res, depth));
			temp = DrawObjectBitmap( dos, &overlay_area, bTargetSelectEffect );
			visible = visible || temp;
		}
	}
	return visible;
}
/************************************************************************/
/*
 * DrawObjectBitmap:  Draw given bitmap in given rectangle on screen.
 *   The rectangle is clipped to the screen.
 * Return True iff the object is at least partly visible.
 */
Bool DrawObjectBitmap( DrawObjectInfo *dos, AREA *obj_area, Bool bTargetSelectEffect )
{
   long x, xinc, y, yinc;
   long startx, endx, starty, endy, bitmap_width;
   BYTE *screen_ptr, *row_bits, *obj_bits, *palette, *end_screen_ptr;
   long col, row, rowTimesMAXX;
   long lefttop,righttop,leftbot,rightbot;
   ViewCone *c;
   BYTE effect;
   ObjectRowData d;

//	Bool	bClipHaloLeft;
//	Bool	bClipHaloRight;

   if (obj_area->cx == 0 || obj_area->cy == 0)
      return False;

   c = dos->cone;

   /* Clip object to sides of screen */
   startx = max(0, obj_area->x);
   endx = min(area.cx - 1, obj_area->x + obj_area->cx - 1);

   starty = max(0, obj_area->y);
   endy = min(area.cy - 1, obj_area->y + obj_area->cy - 1);

   // Cut off bottom of bitmap at ground level
   endy = min(endy, dos->cutoff);

   startx = max(startx, c->leftedge);
   endx = min(endx, c->rightedge);

   /* See if completely off screen */
   if (endx < 0 || endy < 0 || startx >= area.cx || starty >= area.cy)
      return False;

   if (!dos->draw)
      return True;

   obj_bits = DibPtr(dos->pdib);
   bitmap_width = DibWidth(dos->pdib);

   xinc = (bitmap_width << FIX_DECIMAL) / obj_area->cx;
   yinc = (DibHeight(dos->pdib) << FIX_DECIMAL) / obj_area->cy;

   lefttop = DIVUP(c->top_b * c->leftedge + c->top_d, c->top_a);
   righttop = DIVUP(c->top_b * c->rightedge + c->top_d, c->top_a);
   if (lefttop < righttop)
      starty = max(starty, lefttop);
   else
      starty = max(starty, righttop);

   leftbot = DIVDOWN(c->bot_b * c->leftedge + c->bot_d, c->bot_a);
   rightbot = DIVDOWN(c->bot_b * c->rightedge + c->bot_d, c->bot_a);
   if (leftbot > rightbot)
      endy = min(endy, leftbot);
   else
      endy = min(endy, rightbot);

   /* Save object drawing rectangle */
   //	(Enlarge boundaries encompassing all bitmaps in this object.)
   orect.left   = min(orect.left, startx);
   orect.right  = max(orect.right, endx);
   orect.top    = min(orect.top, starty);
   orect.bottom = max(orect.bottom, endy);

   y = (starty - obj_area->y) * yinc;

   /* Find palette to use, depending on distance */
   if (dos->flags & (OF_FLICKERING | OF_FLASHING))
   {
      palette = GetLightPalette(dos->distance, dos->light, FINENESS,GetFlicker(dos->obj));
   }
   else
   {
      palette = GetLightPalette(dos->distance, dos->light, FINENESS,0);
   }
   //	xxx testing
   //palette = GetLightPalette(dos->distance, rand() % 60 + 1, FINENESS);

   d.palette = palette;
   d.flags  = dos->flags;
   d.drawingtype = dos->drawingtype | dos->effect;
   d.minimapflags  = dos->minimapflags;
   d.namecolor = dos->namecolor;
   d.objecttype = dos->objecttype;
   d.moveontype = dos->moveontype;
   d.translation = dos->translation;
   d.secondtranslation = dos->secondtranslation;
   rowTimesMAXX = starty * MAXX;
   for (row = starty; row <= endy; row++, y += yinc, rowTimesMAXX += MAXX)
   {
      long mincol = startx;
      long maxcol = endx;

      if (c->top_b > 0)
      {
	 //x <= ay-d / b, b>0
	 col = (c->top_a * row - c->top_d) / c->top_b;
	 if (col < maxcol) 
	    maxcol = col;
//			else
//				bClipHaloLeft = TRUE;		//	Not sure why this works.
      }
      else if (c->top_b < 0)
      {
      //	x >= ay-d / b, b<0
	 col = (c->top_a * row - c->top_d + c->top_b + 1) / c->top_b;
	 if (col > mincol) 
	    mincol = col;
//			else	Never seems to happen
      }

      if (c->bot_b > 0)
      {
      //	x >= ay-d / b, b>0
	 col = (c->bot_a * row - c->bot_d + c->bot_b - 1) / c->bot_b;
	 if (col > mincol)
	    mincol = col;
//			else
//				bClipHaloRight = TRUE;		//	Not sure why this works.
      }
      else if (c->bot_b < 0)
      {
	 //	x <= ay-d / b, b<0
	 col = (c->bot_a * row - c->bot_d) / c->bot_b;
	 if (col < maxcol) 
	    maxcol = col;
//			else	Never seems to happen.
      }

      screen_ptr = gBits + mincol + rowTimesMAXX;
      end_screen_ptr = gBits + maxcol + rowTimesMAXX;

      row_bits = obj_bits + (y >> FIX_DECIMAL) * bitmap_width;
      x = (mincol - obj_area->x) * xinc;

      //xxx
      if( bTargetSelectEffect  )
      {
	 //	Draw "selected target" effect.
	 d.start_ptr	= screen_ptr;
	 d.end_ptr	= end_screen_ptr;
	 d.obj_bits	= row_bits;
	 d.x		= x;
	 d.xinc		= xinc;
	 DrawTargetHalo( &d, obj_bits, y, yinc, bitmap_width, (Bool)( row == starty ), (Bool)( row == endy ) );	//, bClipHaloLeft, bClipHaloRight );
      }


      // Handle common case of no effects specially here
      if (d.translation == 0 && d.drawingtype == 0)
      {	 // Draw normally
#if 1
	 while (screen_ptr <= end_screen_ptr)
	 {
	    /* Don't draw transparent pixels */
	    register BYTE color = *(row_bits + (x >> FIX_DECIMAL));
	    if (color != TRANSPARENT_INDEX)
	       *screen_ptr = palette[color];

	    /* Move to next column of screen */
	    screen_ptr++;
	    x += xinc;
	 }
#else
	 __asm 
	 {
	    push  ebp;
	    mov   ebp, palette;
	    xor   eax, eax;
	    mov	  ecx, end_screen_ptr;
	    mov   edi, screen_ptr;
	    sub	  ecx, edi;
	    jl	  END_TRANS_BLIT;
	    inc	  ecx;
	    mov	  ebx, x;
	    mov	  edx, row_bits;
	    cmp	  ecx, 4;
	    jl	  DO_REMAINDER;
DRAW_TRANSPARENT0:
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    jne	  SOLID0;
;DRAW_TRANSPARENT1:
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    jne	  SOLID1;
DRAW_TRANSPARENT2:
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    jne	  SOLID2;
DRAW_TRANSPARENT3:
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    jne	  SOLID3;

	    // Done our 4 pixels, so move to next group
END_TRANSPARENT:
	    add	  edi,4;
	    sub	  ecx,4;
	    jle	  END_TRANS_BLIT;
	    cmp	  ecx,4;
	    jge	  DRAW_TRANSPARENT0;
	    jmp	  DO_REMAINDER;

	    // This is for drawing solid bytes
SOLID0:
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi],al;	        // Save the byte on the screen
	    mov	  esi, ebx;		        // Get the next byte
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    je	  DRAW_TRANSPARENT2;
SOLID1:
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi+1],al;
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    je	  DRAW_TRANSPARENT3;
SOLID2:
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi+2],al;
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    je	  END_TRANSPARENT;
SOLID3:
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi+3],al;
	    add	  edi,4;
	    sub	  ecx,4;
	    jle	  END_TRANS_BLIT;
	    cmp	  ecx,4;
	    jge	  DRAW_TRANSPARENT0;
DO_REMAINDER:
	    dec	  ecx;
	    jl	  END_TRANS_BLIT;
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    je	  DO_REMAINDER1;
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi],al;
DO_REMAINDER1:
	    dec	  ecx;
	    jl	  END_TRANS_BLIT;
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    add	  ebx, xinc;
	    cmp	  al, TRANSPARENT_INDEX;
	    je	  DO_REMAINDER2;
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi+1],al;
DO_REMAINDER2:
	    dec	  ecx;
	    jl	  END_TRANS_BLIT;
	    mov	  esi, ebx;
	    shr	  esi, FIX_DECIMAL;
	    mov	  al, BYTE PTR [esi + edx];
	    cmp	  al, TRANSPARENT_INDEX;
	    je	  END_TRANS_BLIT;
	    mov   al, BYTE PTR [ebp + eax];
	    mov	  BYTE PTR [edi+2],al;
END_TRANS_BLIT:
	    pop   ebp;
	 }
#endif
      }
      else
      {
	 // Call correct inner loop for effect
	 DrawingLoop loop;

	 // Take effect from palette translation or object flags
	 effect = d.drawingtype;
	 if (effect == 0)
	    effect = DRAWFX_TRANSLATE;

	 loop = drawing_loops[effect];
	 if (loop == NULL)
	    debug(("DrawObjectBitmap got unknown effect index %d\n", d.drawingtype));
	 else
	 {
	    d.start_ptr = screen_ptr;
	    d.end_ptr = end_screen_ptr;
	    d.row = row;
	    d.obj_bits = row_bits;
	    d.x = x;
	    d.xinc = xinc;	 
	    d.xsize = MAXX;
	    d.ysize = area.cy;
	    (*loop)(&d);
	 }
      }
   }
   return True;
}
/************************************************************************/
/*
 * ComputeObjectArea:  Compute area taken up by object, and place in a.
 *   pdib is bitmap of object.
 *   This area is given in screen rows and columns.
 *   center is the center column of the object in pixels.
 *   distance is distance to object in FINENESS units.
 *   height is z coordinate of bottom of object in FINENESS units.
 *   On entry, cutoff contains the depth under the ground to draw the object, in FINENESS units.
 *   On exit, cutoff receives the lowest pixel row on the screen in which the object will be drawn
 *     (this might not be the last row in the area "a" if the bottom of the object projects
 *     below the ground, e.g. for an object standing in water).
 */
void ComputeObjectArea(PDIB pdib, int center, int distance, int height, AREA *a, int *cutoff)
{
   int bottom, xoffset, yoffset; 
   int stretch;  // Factor to scale bitmap down when drawing it

   stretch = DibShrinkFactor(pdib);

   /* Find screen coordinates of left end of object */
   a->cx = BitmapToFineness(DibWidth(pdib)) / stretch * VIEWER_DISTANCE / distance;

   xoffset = BitmapToFineness(DibXOffset(pdib)) / stretch * VIEWER_DISTANCE / distance;

   a->x = center - a->cx / 2 + xoffset;
   
   /* Find top and bottom of object on screen -- rest bottom of object on floor */
   bottom = horizon + ((viewer_height - height) << LOG_VIEWER_DISTANCE) / distance;
   a->cy = BitmapToFineness(DibHeight(pdib)) / stretch * VIEWER_DISTANCE / distance;

   yoffset = BitmapToFineness(DibYOffset(pdib)) / stretch * VIEWER_DISTANCE / distance;

   a->y = bottom - a->cy + yoffset + (*cutoff << LOG_VIEWER_DISTANCE) / distance;
   *cutoff = bottom;
}
/************************************************************************/
/*
 * ComputeOverlayArea:  Compute area taken up by overlay, and place in ov.
 *   This area is given in screen rows and columns.
 *   On input, ov should contain the offset of the overlay into the object
 *     in its x and y fields, and the overlay bitmap's size in its cx and cy fields.
 *     obj should contain the screen area of the object upon which to put overlay.
 *     obj_stretch has stretch factor of object; ov_stretch has factor for overlay.
 */
void ComputeOverlayArea(int distance, AREA *obj, int obj_stretch, AREA *ov, int ov_stretch)
{
   int numerator, denominator;

   /* First compute the screen size of the overlay */
   ov->cx = BitmapToFineness(ov->cx) / ov_stretch * VIEWER_DISTANCE / distance;
   ov->cy = BitmapToFineness(ov->cy) / ov_stretch * VIEWER_DISTANCE / distance;

   /* Now place the overlay on the object */
   // XXX Watch for overflow here
   numerator = FINENESS * VIEWER_DISTANCE / BITMAP_WIDTH;

   denominator = distance * obj_stretch * OVERLAY_FACTOR;
   
   ov->x = obj->x + ov->x * numerator / denominator;
   ov->y = obj->y + ov->y * numerator / denominator;
}
/************************************************************************/
/*
 * FindOverlayArea:  Fill in overlay_area with position and size of overlay bitmap.
 *   pdib_ov gives the bitmap of the overlay; angle is its display angle.
 *   hotspot is the hotspot # of the overlay.  
 *   pdib_obj is the bitmap of the underlying object.
 *   overlays lists all the overlays on the underlying object. 
 *   overlay_depth tells which depth of overlay to match (HOTSPOT_OVER, etc.).  If this
 *     is HOTSPOT_ANY, the depth is ignored; any hotspot with the correct number matches.
 *
 *   If this bitmap should be drawn now, fill in overlay_area and return True.
 *   Otherwise, return False.
 */
Bool FindOverlayArea(PDIB pdib_ov, int angle, char hotspot, PDIB pdib_obj, list_type overlays,
		     int overlay_depth, AREA *overlay_area)
{
   // If overlay doesn't have a hotspot, draw it as an overlay
   if (hotspot == 0 && overlay_depth != HOTSPOT_OVER)
      return False;

   // See if overlay should be placed on hotspot
   if (hotspot != 0)
   {
      POINT hotspot_pos;
      int retval;
      
      retval = FindHotspot(overlays, pdib_obj, pdib_ov, hotspot, angle, &hotspot_pos);

      // If hotspot not found, or doesn't match underlays, skip
      if (retval == HOTSPOT_NONE)
      {
	 // Don't complain, since having missing hotspots is sometimes useful
	 //	    debug(("Failed to find overlay %d for bitmap %d\n", 
	 //		    overlay->hotspot, overlay->icon_res));
	 return False;
      }
      
      if (overlay_depth != HOTSPOT_ANY && retval != overlay_depth)
	 return False;
      
      // Move overlay to hotspot
      overlay_area->x = hotspot_pos.x;
      overlay_area->y = hotspot_pos.y;
   }
   else
   {
      overlay_area->x  = 0;
      overlay_area->y  = 0;
   }   

   overlay_area->cx = DibWidth(pdib_ov);
   overlay_area->cy = DibHeight(pdib_ov);
   return True;
}
/************************************************************************/
/*
 * FindHotspot:  An overlay is to be drawn on an object, attached
 *   at the given hotspot number.  pdib is the bitmap of the object, and
 *   overlays gives the object's overlays. Object is being viewed from given angle.
 *   pdib_ov is overlay bitmap.
 *   If the object or one of its overlays has a hotspot with this number, 
 *   set the point to this hotspot.  Point is scaled up by OVERLAY_FACTOR to improve accuracy.
 *   If hotspot is an underlay in pdib, return HOTSPOT_UNDER.
 *   If hotspot is an overlay in pdib, return HOTSPOT_OVER.
 *   Otherwise, return HOTSPOT_NONE.
 */
int FindHotspot(list_type overlays, PDIB pdib, PDIB pdib_ov, char hotspot, int angle, POINT *point)
{
   int retval, retval2, obj_shrink;
   list_type l;
   PDIB pdib_ov2;

   // Check object
   retval = FindHotspotPdib(pdib, hotspot, point);
   if (retval != HOTSPOT_NONE)
   {
      point->x = (point->x + DibXOffset(pdib_ov)) * OVERLAY_FACTOR;
      point->y = (point->y + DibYOffset(pdib_ov)) * OVERLAY_FACTOR;
      return retval;
   }

   obj_shrink = DibShrinkFactor(pdib);

   // Check overlays
   for (l = overlays; l != NULL; l = l->next)
   {
      Overlay *overlay = (Overlay *) (l->data);
      
      pdib_ov2 = GetObjectPdib(overlay->icon_res, angle, overlay->animate.group);
      if (pdib_ov2 == NULL)
	 continue;

      if ((retval = FindHotspotPdib(pdib_ov2, hotspot, point)) != HOTSPOT_NONE)
      {
	 // Measure position relative to overlay's point of attachment
	 // If hotspot is over or under object, use this as return value
	 POINT p2;
	 retval2 = FindHotspotPdib(pdib, overlay->hotspot, &p2);
	 if (retval2 != HOTSPOT_NONE)
	 {

	    point->x = point->x * OVERLAY_FACTOR * obj_shrink / DibShrinkFactor(pdib_ov2) +
	       p2.x * OVERLAY_FACTOR;
	    point->y = point->y * OVERLAY_FACTOR * obj_shrink / DibShrinkFactor(pdib_ov2) + 
	       p2.y * OVERLAY_FACTOR;

	    // Include offset, in units of underlying overlay pixels.
	    // Also include offset of underlying overlay (first term).

	    point->x += DibXOffset(pdib_ov2) * OVERLAY_FACTOR + 
	       DibXOffset(pdib_ov) * OVERLAY_FACTOR *
		  DibShrinkFactor(pdib) / DibShrinkFactor(pdib_ov2);
	    point->y += DibYOffset(pdib_ov2) * OVERLAY_FACTOR + 
	       DibYOffset(pdib_ov) * OVERLAY_FACTOR * 
		  DibShrinkFactor(pdib) / DibShrinkFactor(pdib_ov2);

            if (retval == HOTSPOT_OVER)
	       if (retval2 == HOTSPOT_OVER)
		  return HOTSPOT_OVEROVER;
	       else return HOTSPOT_UNDEROVER;
	    else
	       if (retval2 == HOTSPOT_OVER)
		  return HOTSPOT_OVERUNDER;
	       else return HOTSPOT_UNDERUNDER;
	 }
	 
	 // If overlay had no hotspot attachment, assume overlay
	 return HOTSPOT_OVER;
      }
   }
   return HOTSPOT_NONE;
}
/************************************************************************/
/*
 * FindHotspotPdib:  Look for given hotspot number in pdib.
 *   If present, set point to the hotspot's location.
 *   If hotspot is an underlay in pdib, return HOTSPOT_UNDER.
 *   If hotspot is an overlay in pdib, return HOTSPOT_OVER.
 *   Otherwise, return HOTSPOT_NONE.
 */
int FindHotspotPdib(PDIB pdib, char hotspot, POINT *point)
{
   int i, num;
   POINT p;

   for (i=0; i < DibNumHotspots(pdib); i++)
   {
      num = DibHotspotNumber(pdib, i);
      if (ABS(num) == hotspot)
      {
	 p = DibHotspotIndex(pdib, i);
	 point->x = p.x;
	 point->y = p.y;
	 return (num > 0) ? HOTSPOT_OVER : HOTSPOT_UNDER;
      }
   }
   return HOTSPOT_NONE;
}

/*****************************************************************************/
/*
 * GetObjectSize: Figure out size of an object's bitmap and overlays,
 *   and set width and height to these values.
 *   Return True on success.
 */
Bool GetObjectSize(ID icon_res, int group, int angle, list_type overlays, int *width, int *height)
{
  PDIB pdib, pdib_ov;
  int min_x, max_x;
  int min_y, max_y;
  list_type l;
  
  pdib = GetObjectPdib(icon_res, angle, group);
  if (!pdib)
    return False;
  
  /* find width of primary bitmap */
  min_x = min_y = 0;
  max_x = BitmapToFineness(DibWidth(pdib)) / DibShrinkFactor(pdib);
  max_y = BitmapToFineness(DibHeight(pdib)) / DibShrinkFactor(pdib);
  
  /* find width of overlays */
  for(l = overlays; l != NULL; l = l->next)
  {
     Overlay *overlay = (Overlay *) (l->data);
     char hotspot = overlay->hotspot;
     AREA ov_area;
     
     pdib_ov = GetObjectPdib(overlay->icon_res, angle, overlay->animate.group);
     if (pdib_ov == NULL)
	continue;

     // Try to put overlay on hotspot to determine its location
     if (!FindOverlayArea(pdib_ov, angle, hotspot, pdib, overlays, HOTSPOT_ANY, &ov_area))
	continue;

     ov_area.cx = BitmapToFineness(ov_area.cx) / DibShrinkFactor(pdib_ov);
     ov_area.x  = BitmapToFineness(ov_area.x) / OVERLAY_FACTOR;

     ov_area.cy = BitmapToFineness(ov_area.cy) / DibShrinkFactor(pdib_ov);
     ov_area.y  = BitmapToFineness(ov_area.y) / OVERLAY_FACTOR;

     min_x = min(min_x, ov_area.x);
     max_x = max(max_x, ov_area.x + ov_area.cx);

     min_y = min(min_y, ov_area.y);
     max_y = max(max_y, ov_area.y + ov_area.cy);
  }
  *width  = max_x - min_x;
  *height = max_y - min_y;
  return True;
}

/************************************************************************/
/*
 * DrawObjectDecorations:  Given object has been completely drawn; draw
 *   any other things associated with the object.
 */
void DrawObjectDecorations(DrawnObject *object)
{
   ObjectRange *range;
   char *name;
   int namelen;
   int x, y;
   SIZE s;
   COLORREF fg_color, bg_color;
   BYTE *palette;
   Color color;

   room_contents_node *r;

   if (!config.draw_names || effects.blind)
      return;

   r = GetRoomObjectById(object->id);
   if (r == NULL)
      return;

   if (!(r->obj.flags & OF_PLAYER) || (r->obj.drawingtype == DRAWFX_INVISIBLE))
      return;

   // Draw player name
   range = FindVisibleObjectById(r->obj.id);
   if (range == NULL || range->distance > MAX_NAME_DISTANCE)
      return;
   
   name = LookupNameRsc(r->obj.name_res);
   namelen = strlen(name);

   // Center over object
   GetTextExtentPoint32(gBitsDC, name, strlen(name), &s);
   x = (range->left_col + range->right_col - s.cx) / 2;
   y = range->top_row - s.cy - 2;

   // Give a shadowed look to be visible on all color backgrounds
   fg_color = GetPlayerNameColor(&r->obj,name);
   bg_color = NAME_COLOR_NORMAL_BG;

   // Some names never grow darker, they use PALETTEINDEX().
   if (HIBYTE(HIWORD(fg_color)) == HIBYTE(HIWORD(PALETTEINDEX(0))))
   {
      //XXX: normally, SetTextColor() works with PALETTEINDEX() types fine,
      //     but not here for unknown reason
      //     so we convert to our base_palette[] PALETTERGB() type.
      //
      color = base_palette[LOBYTE(LOWORD(fg_color))];
      fg_color = PALETTERGB(color.red, color.green, color.blue);
   }
   else
   {
      // Draw name with color that fades with distance, just like object
      if (r->obj.flags & (OF_FLICKERING | OF_FLASHING))
      {
         palette = GetLightPalette(object->distance, object->light, FINENESS,GetFlicker(r));
      }
      else
      {
         palette = GetLightPalette(object->distance, object->light, FINENESS,0);
      }
      color = base_palette[palette[GetClosestPaletteIndex(fg_color)]];
      fg_color = PALETTERGB(color.red, color.green, color.blue);
   }

   SetTextColor(gBitsDC, bg_color);
   TextOut(gBitsDC, x + 1, y + 1, name, namelen);
   SetTextColor(gBitsDC, fg_color);
   TextOut(gBitsDC, x, y, name, namelen);

   GdiFlush();
}
