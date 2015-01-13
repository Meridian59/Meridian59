// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawbmp.c:  Draw objects and their overlays, with many options.
 * 
 * Drawing is done to a single offscreen buffer, and then copied or stretched to the screen.
 */

#include "client.h"

#define OFFSCREEN_BITMAP_SIZE 256

/* Offscreen bitmap & DC for drawing */
static HDC      gOffscreenDC;
static HBITMAP  gOldOffscreenBitmap;
static BYTE *gOffscreenBits;   /* Pointer to pixels of offscreen bitmap */

/* Bitmap for drawing window background */
static RawBitmap bkgnd;

// Inner loops for drawing objects with special effects
extern DrawingLoop drawing_loops[];

extern HPALETTE hPal;
/* Palettes for different light levels */
extern BYTE light_palettes[NUM_PALETTES][NUM_COLORS];

/* local function prototypes */
static void DrawStretchedBitmap(PDIB pdib, RECT rect, int inc, BYTE translation, BYTE secondtranslation, int flags, BYTE drawingtype);
static Bool ComputeObjectBoundingBox(PDIB pdib, list_type overlays, Bool include_object, RECT *max_rect, int angle);
static void DrawOverlays(PDIB pdib_obj, RECT *obj_rect, list_type overlays, 
						 int inc, Bool underlays, BYTE secondtranslation, int flags, BYTE drawingtype, int angle);
static void OffscreenBitCopy(HDC hdc, int dest_x, int dest_y, int width, int height,
							 BYTE *bits, int source_x, int source_y, int source_width, int options);
/************************************************************************/
/*
* DrawBitmapInitialize:  Setup up offscreen buffer.
*/
void DrawBitmapInitialize(void)
{
	gOffscreenDC = CreateMemBitmap(OFFSCREEN_BITMAP_SIZE, OFFSCREEN_BITMAP_SIZE,
		&gOldOffscreenBitmap, &gOffscreenBits);
	if (gOffscreenDC == 0)
		debug(("DrawBitmapInitialize Couldn't create DC!\n"));
	
	CreateWindowBackground();
}
/************************************************************************/
/*
* DrawBitmapClose:  Free offscreen buffer.
*/
void DrawBitmapClose(void)
{
	HBITMAP gCurrentBitmap;
	
	gCurrentBitmap = (HBITMAP) SelectObject(gOffscreenDC, gOldOffscreenBitmap);
	DeleteObject(gCurrentBitmap);
	DeleteDC(gOffscreenDC);
}
/************************************************************************/
/*
* DrawStretchedObjectDefault: Call DrawStretchedObjectGroup with group 0.
*/
void DrawStretchedObjectDefault(HDC hdc, object_node *obj, AREA *area, HBRUSH brush)
{
	DrawObject(hdc, obj, 0, True, area, brush, 0, 0, 0, True);
}
/************************************************************************/
/*
* DrawStretchedOverlays:  Draw overlays of given object in given area.
*   (See DrawStretchedObjectGroup.)
*/
void DrawStretchedOverlays(HDC hdc, object_node *obj, AREA *area, HBRUSH brush)
{
	DrawObject(hdc, obj, 0, False, area, brush, 0, 0, 0, True);
}
/************************************************************************/
/*
* DrawStretchedOverlayRange:  Like DrawStretchedOverlays, but only include
*   overlays that attach to hotspots in the range [low, high].
*/
void DrawStretchedOverlayRange(HDC hdc, object_node *obj, AREA *area, HBRUSH brush, 
							   int low, int high)
{
	list_type new_overlays, l;
	list_type *old_overlays = obj->overlays;
	
	// Build up list of overlays with hotspots in range
	new_overlays = NULL;
	for (l = *old_overlays; l != NULL; l = l->next)
	{
		Overlay *overlay = (Overlay *) (l->data);
		int num = ABS(overlay->hotspot);
		
		if (num >= low && num <= high)
			new_overlays = list_add_item(new_overlays, l->data);
	}
	
	obj->overlays = &new_overlays;
	DrawStretchedOverlays(hdc, obj, area, brush);
	list_delete(new_overlays);
	obj->overlays = old_overlays;
}
/************************************************************************/
/*
* DrawStretchedObjectGroup: Draw given group # in DrawStretchedObject.
*/
void DrawStretchedObjectGroup(HDC hdc, object_node *obj, int group, AREA *area, HBRUSH brush)
{
	if (obj->flags & OF_PLAYER)
		DrawObject(hdc, obj, group, True, area, brush, 0, 0, 7*MAX_ANGLE/8, True);
	else
		DrawObject(hdc, obj, group, True, area, brush, 0, 0, 0, True);
}
/*
*/
void DrawObjectIcon(HDC hdc, ID icon, int group, Bool draw_obj, AREA *area, HBRUSH brush,
					int x, int y, Bool copy)
{
	int draw_size;
	int width, height, obj_shrink, temp, inc;
	RECT rect, obj_rect, max_rect;
	PDIB pdib;
	Bool has_overlay = False;
	
	if (0 == icon) // if we pass in a null pointer then abort early
		return;
	
	// If object larger than offscreen buffer, we'll have to stretch it down
	draw_size = min(max(OFFSCREEN_BITMAP_SIZE - x, OFFSCREEN_BITMAP_SIZE - y), 
		max(area->cx, area->cy));
	
	SelectPalette(hdc, hPal, FALSE);
	RealizePalette(hdc);
	
	pdib = GetObjectPdib(icon, 0, group);
	if (pdib == NULL)
		return;
	obj_shrink = DibShrinkFactor(pdib);
	
	/* Draw background */
	if (brush != NULL)
	{
		rect.left = x;
		rect.top = y;
		rect.bottom = draw_size;
		rect.right = draw_size;
		
		FillRect(gOffscreenDC, &rect, brush);
		
		/* Force background paint before foreground drawn */
		GdiFlush();
	}
	
	/* Find bounding rectangle of object and overlays */
	//has_overlay = ComputeObjectBoundingBox(pdib, *(obj->overlays), draw_obj, &max_rect);
	max_rect.left = max_rect.top = 0;
	if (draw_obj)
	{
		max_rect.right  = DibWidth(pdib);
		max_rect.bottom = DibHeight(pdib);
	}
	else
	{
		max_rect.right = max_rect.bottom = 0;
	}
	
	width  = max_rect.right - max_rect.left;
	height = max_rect.bottom - max_rect.top;
	
	/* Stretch object to just fit in square */
	inc = (max(width, height) << FIX_DECIMAL) / draw_size;
	
	if (draw_obj || has_overlay)
	{
		// Center object in drawing square
		temp = draw_size - (width << FIX_DECIMAL) / inc;
		obj_rect.left  = x + temp / 2 - DIVUP((max_rect.left << FIX_DECIMAL), inc);
		obj_rect.right = obj_rect.left + DIVUP((DibWidth(pdib) << FIX_DECIMAL), inc);
		
		temp = draw_size - (height << FIX_DECIMAL) / inc;
		obj_rect.top    = y + temp / 2 - DIVUP((max_rect.top << FIX_DECIMAL), inc);
		obj_rect.bottom = obj_rect.top + DIVUP((DibHeight(pdib) << FIX_DECIMAL), inc);
	}
	
	// Draw underlays
	//if (obj->overlays != NULL)
	//DrawOverlays(pdib, &obj_rect, *(obj->overlays), inc, True, obj->secondtranslation, obj->flags, obj->drawingtype);
	
	if (draw_obj)
		DrawStretchedBitmap(pdib, obj_rect, inc, 0, 0, 0, 0);
	
	// Draw overlays
	//if (obj->overlays != NULL)
	//DrawOverlays(pdib, &obj_rect, *(obj->overlays), inc, False, obj->secondtranslation, obj->flags, obj->drawingtype);
	
	if (!copy) 
		return;
	
	/* Copy result to target DC, stretching if necessary */
	if (draw_size > OFFSCREEN_BITMAP_SIZE)
		StretchBlt(hdc, area->x, area->y, area->cx, area->cy,
		gOffscreenDC, x, y, draw_size, draw_size, SRCCOPY);
	else BitBlt(hdc, area->x, area->y, draw_size, draw_size, gOffscreenDC, x, y, SRCCOPY);
}
/************************************************************************/
/*
* DrawObject: Draw the given object in given area on the given dc.  
*   The object is drawn transparently onto an area with background color given by brush.
*   If brush is NULL, doesn't erase background first (allows caller to put background there).
*   The object is drawn to an offscreen buffer, and stretched larger if necessary.
*   If draw_obj is False, draw only overlays, and not object itself.
*   The object is drawn at (x, y) on the offscreen bitmap.
*   If copy is False, don't copy to hdc; just leave on offscreen bitmap.
*/
void DrawObject(HDC hdc, object_node *obj, int group, Bool draw_obj, AREA *area, HBRUSH brush,
				int x, int y, int angle, Bool copy)
{
	int draw_size;
	int width, height, obj_shrink, temp, inc;
	RECT rect, obj_rect, max_rect;
	PDIB pdib;
	Bool has_overlay = False;
	
	if (NULL == obj) // if we pass in a null pointer then abort early
		return;
	
	// If object larger than offscreen buffer, we'll have to stretch it down
	draw_size = min(max(OFFSCREEN_BITMAP_SIZE - x, OFFSCREEN_BITMAP_SIZE - y), 
		max(area->cx, area->cy));
	
	SelectPalette(hdc, hPal, FALSE);
	RealizePalette(hdc);
	
	pdib = GetObjectPdib(obj->icon_res, angle, group);
	if (pdib == NULL)
		return;
	obj_shrink = DibShrinkFactor(pdib);
	
	/* Draw background */
	if (brush != NULL)
	{
		rect.left = x;
		rect.top = y;
		rect.bottom = draw_size;
		rect.right = draw_size;
		
		FillRect(gOffscreenDC, &rect, brush);
		
		/* Force background paint before foreground drawn */
		GdiFlush();
	}
	
	/* Find bounding rectangle of object and overlays */
	has_overlay = ComputeObjectBoundingBox(pdib, *(obj->overlays), draw_obj, &max_rect, angle);
	
	width  = max_rect.right - max_rect.left;
	height = max_rect.bottom - max_rect.top;
	
	/* Stretch object to just fit in square */
	inc = (max(width, height) << FIX_DECIMAL) / draw_size;
	
	if (draw_obj || has_overlay)
	{
		// Center object in drawing square
		temp = draw_size - (width << FIX_DECIMAL) / inc;
		obj_rect.left  = x + temp / 2 - DIVUP((max_rect.left << FIX_DECIMAL), inc);
		obj_rect.right = obj_rect.left + DIVUP((DibWidth(pdib) << FIX_DECIMAL), inc);
		
		temp = draw_size - (height << FIX_DECIMAL) / inc;
		obj_rect.top    = y + temp / 2 - DIVUP((max_rect.top << FIX_DECIMAL), inc);
		obj_rect.bottom = obj_rect.top + DIVUP((DibHeight(pdib) << FIX_DECIMAL), inc);
	}
	
	// Draw underlays
	if (obj->overlays != NULL)
		DrawOverlays(pdib, &obj_rect, *(obj->overlays), inc, True, obj->secondtranslation, obj->flags, obj->drawingtype, angle);
	
	if (draw_obj)
		DrawStretchedBitmap(pdib, obj_rect, inc, obj->translation, obj->secondtranslation, obj->flags, obj->drawingtype);
	
	// Draw overlays
	if (obj->overlays != NULL)
		DrawOverlays(pdib, &obj_rect, *(obj->overlays), inc, False, obj->secondtranslation, obj->flags, obj->drawingtype, angle);
	
	if (!copy) 
		return;
	
	/* Copy result to target DC, stretching if necessary */
	if (draw_size > OFFSCREEN_BITMAP_SIZE)
		StretchBlt(hdc, area->x, area->y, area->cx, area->cy,
		gOffscreenDC, x, y, draw_size, draw_size, SRCCOPY);
	else BitBlt(hdc, area->x, area->y, draw_size, draw_size, gOffscreenDC, x, y, SRCCOPY);
}
/************************************************************************/
/*
* DrawOverlays:  Draw overlays on object whose bitmap is pdib_obj.
*   obj_rect is the rectangle containing the object.
*   inc tells how far to step on pdib_obj per pixel of obj_rect (fixed point).
*   If underlays is True, draw only those overlays which should be drawn
*     before the object is drawn.
*   flags is a set of object flags used for OF_FLASHING etc. drawingtype is
*   an enum of types such as DRAWFX_INVISIBLE, etc.
*/
void DrawOverlays(PDIB pdib_obj, RECT *obj_rect, list_type overlays, 
				  int inc, Bool underlays, BYTE secondtranslation, int flags, BYTE drawingtype, int angle)
{
	RECT rect;
	list_type l;
	int shrink, obj_shrink, x, y;
	int pass;  // Which pass of overlays are we on; 0 = under; 1 = normal; 2 = over
	int depth; // Current overlay depth to match
	
	/* Draw overlays over main object */
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
			
			// If overlay doesn't have a hotspot, draw it as an overlay
			if (overlay->hotspot == 0 && depth != HOTSPOT_OVER)
				continue;
			
			pdib_ov = GetObjectPdib(overlay->icon_res, angle, overlay->animate.group);
			if (pdib_ov == NULL)
				continue;
			
			x = 0;
			y = 0;
			
			// See if overlay should be placed on hotspot
			if (overlay->hotspot != 0)
			{
				POINT hotspot;
				int retval;
				
				retval = FindHotspot(overlays, pdib_obj, pdib_ov, overlay->hotspot, angle, &hotspot);
				
				// If hotspot not found, or doesn't match underlays, skip
				if (retval == HOTSPOT_NONE)
				{
					char *str = LookupRsc( overlay->icon_res);
					
					debug(("Failed to find hotspot %d for bitmap %d (%s)\n",overlay->hotspot, overlay->icon_res,str));
					continue;
				}
				
				if (retval != depth)
					continue;
				
				x += hotspot.x;
				y += hotspot.y;
				// debug(("x = %d, y = %d, hotspot = (%d %d)\n", x, y, hotspot.x, hotspot.y));
			}
			
			shrink = DibShrinkFactor(pdib_ov);
			obj_shrink = DibShrinkFactor(pdib_obj);
			
			// Place overlay on object
			rect.left  = obj_rect->left + ((x << FIX_DECIMAL) / inc / OVERLAY_FACTOR);
			rect.right = rect.left + 
				DIVUP(((DibWidth(pdib_ov) * obj_shrink / shrink) << FIX_DECIMAL), inc);
			
			rect.top    = obj_rect->top + ((y << FIX_DECIMAL) / inc / OVERLAY_FACTOR);
			rect.bottom = rect.top + 
				DIVUP(((DibHeight(pdib_ov) * obj_shrink / shrink) << FIX_DECIMAL), inc);
			
			if (overlay->effect)
				DrawStretchedBitmap(pdib_ov, rect, inc * shrink / obj_shrink,
					overlay->translation, secondtranslation, flags, overlay->effect);
			else
				DrawStretchedBitmap(pdib_ov, rect, inc * shrink / obj_shrink,
					overlay->translation, secondtranslation, flags, drawingtype);
		}
	}
}
/************************************************************************/
/*
* DrawStretchedBitmap:  Draw given PDIB in given rectangle on offscreen bitmap.
*   inc tells how far to step on PDIB per pixel of gBits (fixed point).
*   translation gives the palette translation type.
*   flags is a set of object flags, used to specify special drawing effects
*/
void DrawStretchedBitmap(PDIB pdib, RECT rect, int inc, BYTE translation, BYTE secondtranslation, int flags, BYTE drawingtype)
{
	int bitmap_width;
	BYTE *obj_bits, *offscreen_ptr, *object_ptr, *end_ptr, index;
	int i, j, x, y;
	ObjectRowData d;
	
	// Bring rectangle into range to prevent crashes
	rect.left   = max(rect.left, 0);
	rect.right  = min(rect.right, OFFSCREEN_BITMAP_SIZE - 1);
	rect.top    = max(rect.top, 0);
	rect.bottom = min(rect.bottom, OFFSCREEN_BITMAP_SIZE - 1);
	
	bitmap_width = DibWidth(pdib);
	obj_bits = DibPtr(pdib);
	
	y = 0;
	d.flags = flags;
	d.drawingtype = drawingtype;
	d.minimapflags  = 0;
	d.namecolor = 0;
	d.objecttype = OT_NONE;
	d.moveontype = MOVEON_YES;
	d.translation = translation;
	d.secondtranslation = secondtranslation;
	// Dummy palette for use with special effects--draws at max brightness
	d.palette = light_palettes[LIGHT_LEVELS - 1];
	for (i = rect.top; i < rect.bottom; i++)
	{
		x = 0;
		object_ptr = obj_bits + (y >> FIX_DECIMAL) * bitmap_width;
		offscreen_ptr = gOffscreenBits + rect.left + i * OFFSCREEN_BITMAP_SIZE;
		end_ptr = offscreen_ptr + (rect.right - rect.left - 1);
		// NOTE: Could make effect cases faster by making separate loops without palette indirection
		
		// Handle common case of no effects specially here
		if (translation == 0 && drawingtype == 0)
		{
			for (j = rect.left; j < rect.right; j++)
			{
				index = *(object_ptr + (x >> FIX_DECIMAL));
				if (index != TRANSPARENT_INDEX)
					*offscreen_ptr = index;
				x += inc;
				offscreen_ptr++;
			}
		}
		else
		{
			// Call correct inner loop for effect
			DrawingLoop loop;
			BYTE effect;
			
			// Take effect from palette translation or object drawingtype
			effect = drawingtype;
			if (effect == 0)
				effect = DRAWFX_TRANSLATE;
			
			loop = drawing_loops[effect];
			
			if (loop == NULL)
			{
				//debug(("DrawStretchedBitmap got unknown effect index %d\n", drawingtype));
			}
			else
			{
				d.start_ptr = offscreen_ptr;
				d.end_ptr = end_ptr;
				d.row = i;
				d.obj_bits = object_ptr;
				d.x = x;
				d.xinc = inc;
				d.xsize = OFFSCREEN_BITMAP_SIZE;
				d.ysize = rect.bottom;
				(*loop)(&d);
			}
		}
		y += inc;
	}   
}
/************************************************************************/
/*
* ComputeObjectBoundingBox:  Find the smallest rectangle that encloses
*   the object and its overlay bitmaps at angle 0, and put it in rect.
*   pdib is the object's bitmap; overlays is the list of its overlays.
*   include_object is True iff object bitmap should be included in bounding box.
*   Returns True iff the object has any visible overlays at angle 0.
*/
Bool ComputeObjectBoundingBox(PDIB pdib, list_type overlays, Bool include_object, RECT *max_rect, int angle)
{
	list_type l;
	int obj_shrink, shrink, x, y;
	Bool has_overlay = False;
	
	max_rect->left = max_rect->top = 0;
	if (include_object)
	{
		max_rect->right  = DibWidth(pdib);
		max_rect->bottom = DibHeight(pdib);
	}
	else max_rect->right = max_rect->bottom = 0;
	
	obj_shrink = DibShrinkFactor(pdib);
	
	for (l = overlays; l != NULL; l = l->next)
	{
		Overlay *overlay = (Overlay *) (l->data);
		PDIB pdib_ov = GetObjectPdib(overlay->icon_res, angle, overlay->animate.group);
		if (pdib_ov == NULL)
			continue;
		shrink = DibShrinkFactor(pdib_ov);
		
		x = 0;
		y = 0;
		
		// See if overlay should be placed on hotspot
		if (overlay->hotspot != 0)
		{
			POINT hotspot;
			
			if (FindHotspot(overlays, pdib, pdib_ov, overlay->hotspot, angle, &hotspot) != HOTSPOT_NONE)
			{
				x += hotspot.x / OVERLAY_FACTOR;
				y += hotspot.y / OVERLAY_FACTOR;
			}
		}
		
		// If we're only drawing overlays and this is first one, set bounding box 
		if (!include_object && !has_overlay)
		{
			max_rect->left = x;
			max_rect->top  = y;
		}
		else
		{
			max_rect->left = min(max_rect->left, x);
			max_rect->top  = min(max_rect->top, y);
		}
		
		max_rect->right  = max(max_rect->right, x + DibWidth(pdib_ov) * obj_shrink / shrink);
		max_rect->bottom = max(max_rect->bottom, y + DibHeight(pdib_ov) * obj_shrink / shrink);
		
		has_overlay = True;
	}
	return has_overlay;
}
/************************************************************************/
/*
* CreateWindowBackground:  Create a bitmap for drawing the window
*   background.  It would be nice to do this with a Windows brush instead,
*   but these are limited to 8x8 pixels.
*/
void CreateWindowBackground(void)
{
	BITMAPINFOHEADER *ptr;
	
	ptr = (BITMAPINFOHEADER *) GetBitmapResource(hInst, IDB_BACKGROUND);
	if (ptr == NULL)
		debug(("Couldn't lock resource!\n"));      
	bkgnd.bits = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
	
	bkgnd.height = min(ptr->biHeight, OFFSCREEN_BITMAP_SIZE);
	bkgnd.width  = min(ptr->biWidth, OFFSCREEN_BITMAP_SIZE);
}
/************************************************************************/
/*
* DrawWindowBackgroundColor:
*   If index is >=0, fill rectangle with given index color from palette.
*   If index is -1, fill rectangle with window background bitmap.
*   The background bitmap is drawn with origin as if the rectangle were at
*   (xin, yin) in client window coordinates, so that different areas drawn
*   with the background bitmap can line up.
*
*   The background bitmap is given by bg; if it's NULL, the default window 
*   background is used.
*/
void DrawWindowBackgroundColor(RawBitmap *bg, HDC hdc, RECT *rect, int xin, int yin, int index)
{
	int width, height, x, y, xoffset, yoffset, i;
	
	if (bg == NULL)
		bg = &bkgnd;
	
	switch (GameGetState())
	{
	case GAME_NONE:
		FillRect(hdc, rect, GetBrush(COLOR_BGD));
		break;
		
	default:
		SelectPalette(hdc, hPal, FALSE);
		RealizePalette(hdc);
		
		if (index == -1)
			// Copy background bits to offscreen DC
			for (i=0; i < bg->height; i++)
				memcpy(gOffscreenBits + i * OFFSCREEN_BITMAP_SIZE, 
				bg->bits + i * bg->width, bg->width);
			else
				for (i=0; i < bg->height; i++)
					memset(gOffscreenBits + i * OFFSCREEN_BITMAP_SIZE, index, bg->width);
				
				
				// Do manual rectangle fill
				y = rect->top;
				yoffset = yin % bg->height;
				while (y < rect->bottom)
				{
					x = rect->left;
					xoffset = xin % bg->width;
					height = min(bg->height - yoffset, rect->bottom - y);
					while (x < rect->right)
					{
						width = min(bg->width - xoffset, rect->right - x);
						BitBlt(hdc, x, y, width, height, gOffscreenDC, xoffset, yoffset, SRCCOPY);
						x += bg->width - xoffset;
						xoffset = 0;
					}
					y += bg->width - yoffset;
					yoffset = 0;
				}
				break;
	}
}
/************************************************************************/
/*
* DrawWindowBackgroundMem:  Draw background of main window in a block of memory.
*   ptr is a pointer to the block of memory to draw in.
*   rect gives the rectangle to draw in.
*   width is the width of the block pointed to by ptr.
*
*   The background bitmap is drawn with origin as if the rectangle were at
*   (xin, yin) in client window coordinates, so that different areas drawn
*   with the background bitmap can line up.
*/
void DrawWindowBackgroundMem(RawBitmap *bg, BYTE *ptr, RECT *r, int block_width, int xin, int yin)
{
	int x, y, xoffset, yoffset, width, height, i;
	BYTE *gptr, *bkgnd_ptr;
	
	if (bg == NULL)
		bg = &bkgnd;
	
	// Do manual rectangle fill
	y = r->top;
	yoffset = yin % bg->height;
	while (y < r->bottom)
	{
		x = r->left;
		xoffset = xin % bg->width;
		height = min(bg->height - yoffset, r->bottom - y);
		while (x < r->right)
		{
			width = min(bg->width - xoffset, r->right - x);
			
			gptr = ptr + x + y * block_width;
			bkgnd_ptr = bg->bits + xoffset + yoffset * bg->width;
			for (i=0; i < height; i++)
			{
				memcpy(gptr + i * block_width, 
					bkgnd_ptr + i * bg->width, 
					width);
			}
			x += bg->width - xoffset;
			xoffset = 0;
		}
		y += bg->width - yoffset;
		yoffset = 0;
	}
}
/************************************************************************/
/*
* OffscreenWindowBackground:  Draw background of main window in offscreen bitmap.
*   The background bitmap is drawn with origin as if the rectangle were at
*   (xin, yin) in client window coordinates, so that different areas drawn
*   with the background bitmap can line up.
*   A rectangle of size (width, height) of the background is drawn.  width and height
*   must be at most OFFSCREEN_BITMAP_SIZE.
*/
void OffscreenWindowBackground(RawBitmap *bg, int xin, int yin, int width, int height)
{
	RECT r;
	
	r.left = r.top = 0;
	r.right = width;
	r.bottom = height;
	DrawWindowBackgroundMem(bg, gOffscreenBits, &r, OFFSCREEN_BITMAP_SIZE, xin, yin);
}
/************************************************************************/
/*
* OffscreenWindowColor:  Fill the offscreen area (0, 0) (width, height)
*   with the given palette index.
*   width and height must be at most OFFSCREEN_BITMAP_SIZE.
*/
void OffscreenWindowColor(int width, int height, int index)
{
	int i;
	BYTE *ptr;
	
	for (i=0; i < height; i++)
	{
		ptr = gOffscreenBits + i * OFFSCREEN_BITMAP_SIZE;
		memset(ptr, index, width);
	}
}
/************************************************************************/
/*
* OffscreenBitBlt:  Copy the bits from the rectangle (source_x, source_y, width, height)
*   to (dest_x, dest_y) on hdc.
*   bits points to a block of memory with width source_width.
*
*   options contains flags that influence how bitmap is drawn.
*
*   Requires that width and height are at most OFFSCREEN_BITMAP_SIZE.
*   
*   If options doesn't have OBB_COPY set, the bitmap is drawn at (dest_x, dest_y)
*   on the offscreen bitmap; otherwise it's drawn at (0, 0).
*/ 
void OffscreenBitBlt(HDC hdc, int dest_x, int dest_y, int width, int height,
					 BYTE *bits, int source_x, int source_y, int source_width, int options)
{
	OffscreenBitCopy(hdc, dest_x, dest_y, width, height, 
		bits, source_x, source_y, source_width, options);
	if (options & OBB_COPY)
		BitBlt(hdc, dest_x, dest_y, width, height, gOffscreenDC, 0, 0, SRCCOPY);
}
/************************************************************************/
/*
* OffscreenStretchBlt: Same as OffscreenBitBlt, but stretch result.
*/
void OffscreenStretchBlt(HDC hdc, int dest_x, int dest_y, int dest_width, int dest_height,
						 BYTE *bits, int source_x, int source_y, int source_width, int source_height,
						 int options)
{
	OffscreenBitCopy(hdc, dest_x, dest_y, source_width, source_height, 
		bits, source_x, source_y, source_width, options);
	StretchBlt(hdc, dest_x, dest_y, dest_width, dest_height, 
		gOffscreenDC, 0, 0, source_width, source_height, SRCCOPY);
}
/************************************************************************/
/*
* OffscreenBitCopy:  Perform real work of OffscreenBitBlt; copy from bits
*   to offscreen bitmap.
*/
void OffscreenBitCopy(HDC hdc, int dest_x, int dest_y, int width, int height,
					  BYTE *bits, int source_x, int source_y, int source_width, int options)
{
	int i, j, dest_xoffset, dest_yoffset;
	BYTE *src, *dest;
	int increment;
	
	if (options & OBB_COPY)
	{
		dest_xoffset = 0;
		dest_yoffset = 0;
	}
	else 
	{
		dest_xoffset = dest_x;
		dest_yoffset = dest_y;
	}
	
	if (options & OBB_FLIP)
	{
		dest = gOffscreenBits + (height - 1 + dest_yoffset) * OFFSCREEN_BITMAP_SIZE + dest_xoffset;
		increment = - OFFSCREEN_BITMAP_SIZE;
	}
	else
	{
		dest = gOffscreenBits + dest_yoffset * OFFSCREEN_BITMAP_SIZE + dest_xoffset;
		increment = OFFSCREEN_BITMAP_SIZE;
	}
	
	if (options & OBB_TRANSPARENT)
	{
		for (i=0; i < height; i++)
		{
			src = bits + i * source_width + source_x;
			
			if (options & OBB_FLIP)
				dest = gOffscreenBits + (height - 1 - i + dest_yoffset) * OFFSCREEN_BITMAP_SIZE + 
				dest_xoffset;
			else dest = gOffscreenBits + (i + dest_yoffset) * OFFSCREEN_BITMAP_SIZE + dest_xoffset;
			for (j=0; j < width; j++)
			{
				if (*src != TRANSPARENT_INDEX)
					*dest = *src;
				src++;
				dest++;
			}
		}
	}
	else
	{
		for (i=0; i < height; i++)
		{
			memcpy(dest, bits + i * source_width + source_x, width);
			dest += increment;
		}
	}
}
/************************************************************************/
/*
* OffscreenCopy:   Copy the (width, height) sized rectangle from (source_x, source_y) 
*   on the offscreen bitmap to (dest_x, dest_y) on hdc.
*/
void OffscreenCopy(HDC hdc, int dest_x, int dest_y, int width, int height, 
				   int source_x, int source_y)
{
	BitBlt(hdc, dest_x, dest_y, width, height, gOffscreenDC, source_x, source_y, SRCCOPY);
}

/************************************************************************/
/*
* BitCopy:   Copied from OffscreenBitCopy. Does same thing but puts writes results to specified bits pointer instead of gOffscreenBits. ajw
*				dest_bits is a pointer to the destination bits.
*				dest_bits_width is the width of the destination buffer.
*/
void BitCopy( BYTE* dest_bits, int dest_bits_width, int dest_x, int dest_y, int width, int height,
			 BYTE *bits, int source_x, int source_y, int source_width, int options)
{
	int i, j, dest_xoffset, dest_yoffset;
	BYTE *src, *dest;
	int increment;
	
	if (options & OBB_COPY)
	{
		dest_xoffset = 0;
		dest_yoffset = 0;
	}
	else 
	{
		dest_xoffset = dest_x;
		dest_yoffset = dest_y;
	}
	
	if (options & OBB_FLIP)
	{
		dest = dest_bits + (height - 1 + dest_yoffset) * dest_bits_width + dest_xoffset;
		increment = -dest_bits_width;
	}
	else
	{
		dest = dest_bits + dest_yoffset * dest_bits_width + dest_xoffset;
		increment = dest_bits_width;
	}
	
	if (options & OBB_TRANSPARENT)
	{
		for (i=0; i < height; i++)
		{
			src = bits + i * source_width + source_x;
			
			if (options & OBB_FLIP)
				dest = dest_bits + (height - 1 - i + dest_yoffset) * dest_bits_width + 
				dest_xoffset;
			else dest = dest_bits + (i + dest_yoffset) * dest_bits_width + dest_xoffset;
			for (j=0; j < width; j++)
			{
				if (*src != TRANSPARENT_INDEX)
					*dest = *src;
				src++;
				dest++;
			}
		}
	}
	else
	{
		for (i=0; i < height; i++)
		{
			memcpy(dest, bits + i * source_width + source_x, width);
			dest += increment;
		}
	}
}
