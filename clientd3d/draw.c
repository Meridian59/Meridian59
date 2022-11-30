// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * draw.c:  Draw rooms and objects.
 * 
 * This file currently contains a hodegepodge of utility functions for drawing both the 3D
 * view, and parts of the interface.
 */

#include "client.h"

int border_index;  // Palette index for border of graphics area

#define MINIMAP_REDRAW_EVERY_XFRAME 1 // 1 is every frame, 2 every other frame, etc

extern HPALETTE hPal;
extern room_type current_room;
extern player_info player;

/************************************************************************/
/*
 * DrawRoom:  Draw the given room on the given DC, by first drawing
 *   it all offscreen, then copying.
 *   map is True if we should draw a map of the room over the normal 3D view.
 *   Requires that room is valid.
 */
void DrawRoom(HDC hdc, int x, int y, room_type *room, Bool map)
{
   Draw3DParams params;
   AREA view;
   static int miniDrawCount = MINIMAP_REDRAW_EVERY_XFRAME;
   room_contents_node *viewObject = NULL;

   if (player.x < 0 || player.x > room->width || player.y < 0 || player.y > room->height)
   {
      debug(("DrawRoom tried to draw player outside room, at x = %d, y = %d\n", 
	     player.x, player.y));
      return;
   }

   CopyCurrentView(&view);
   memset(&params,0,sizeof(params));

   if (player.viewID)
      viewObject = GetRoomObjectById(player.viewID);
   if (NULL != viewObject)
   {
      params.player_id = player.viewID;
      params.viewer_angle = viewObject->angle;
      params.viewer_light = player.viewLight; // What do we do about light?
      params.viewer_x = viewObject->motion.x + effects.view_dx;
      params.viewer_y = viewObject->motion.y + effects.view_dy;
      params.viewer_height = player.viewHeight;
   }
   else
   {
      params.player_id = player.id;
      params.viewer_angle = player.angle;
      params.viewer_light = player.light;
      params.viewer_x = player.x + effects.view_dx;
      params.viewer_y = player.y + effects.view_dy;
      params.viewer_height = PlayerGetHeight() + effects.view_dz;
   }
   params.hdc = hdc;
   params.x = x;
   params.y = y;
   params.width = view.cx;
   params.height = view.cy;

   // Draw the current view.
   //
   // TODO(akirmse): Apparently map mode was never implemented in the 3D renderer,
   // which means that the "map" command does nothing (other than
   // switch rendering modes, oddly).
   if (D3DRenderIsEnabled())
	   D3DRenderBegin(room, &params);
   else
   {
     if (map)
       DrawMapAsView(room, &params);
     else
       DrawRoom3D(room, &params);
   }

   // Draw the mini-map if it's old enough.
   //
   miniDrawCount--;
   if (miniDrawCount <= 0)
   {
      DrawMiniMap(room, &params);
      miniDrawCount = MINIMAP_REDRAW_EVERY_XFRAME;
   }

//   DrawBuffer(742,view.cy/params.stretchfactor,gscreen_ptr);
   SetFrameDrawn(TRUE);
} 

/************************************************************************/
/*
 * GetObjectPdib: Return bitmap for given object at given viewing
 * angle.  Angle is in the range [0, NUMDEGREES). 
 */
PDIB GetObjectPdib(ID id, long angle, int group)
{
   object_bitmap_type bmp;
   int num, interval, index;

   bmp = GetObjectBitmap(id);
   if (bmp == NULL)
      return NULL;

//   debug(("Drawing %ld at angle %d, group %d", id, angle, group));

   if (group < 0 || group >= BitmapsNumGroups(bmp->bmaps))
   {
//      debug(("Out of range group #%d, max is %d for icon %d\n", group, 
//	     BitmapsNumGroups(bmp->bmaps) - 1, id));
      return NULL;
   }

   /* Wrap however many bitmaps are present around 360 degrees */
   /* We center the range of each bitmap on 360 degrees/n, where n is the
    * number of bitmaps.  Thus, with two bitmaps, one is visible from
    * 270 to 90 degrees, and the other from 90 to 270. */
   /* +1 is to get around roundoff error */
   interval = NUMDEGREES / BitmapsInGroup(bmp->bmaps, group) + 1;
   
   angle = (angle + interval / 2) % NUMDEGREES;
   num = angle / interval;

//   debug((" using bitmap %d (angle = %d)\n", num, angle));

   // Index of -1 specifies that there is no bitmap; check for this case
   index = BitmapsGetIndex(bmp->bmaps, group, num);

   if (index == BITMAP_MISSING)
   {
//      debug(("Missing bitmap for object %d group %d #%d\n", id, group, num));
      return NULL;
   }
   
   return BitmapsGetPdibByIndex(bmp->bmaps, index);
}
/************************************************************************/
/*
 * GetGridPdib:  Return a pointer to the pdib containing the given grid's 
 *   current bitmap, or NULL if none found.
 */
PDIB GetGridPdib(WORD grid_id)
{
   grid_bitmap_type grid;
   
   if (grid_id == 0)
      return NULL;

   grid = GetGridBitmap(grid_id);

   if (grid == NULL)
      return NULL;

   return grid->bmaps.pdibs[0];
}
/************************************************************************/
/*
 * GetBackgroundPdib:  Get the pdib containing the given resource's bitmap.
 */
PDIB GetBackgroundPdib(DWORD rsc)
{
   object_bitmap_type s = GetBackgroundBitmap(rsc);

   if (s == NULL)
      return NULL;

   return s->bmaps.pdibs[0];
}

/************************************************************************/
/*
 * DrawBorder:  Draw border around the given area with given palette index color.
 *   If index is -1, redraw window background under area. Pass pExclude NULL if corners of border are not obscured.
 */
void DrawBorder( AREA *area, int index, DRAWBORDEREXCLUDE* pExclude )
{
   HDC hdc;

   hdc = GetDC(hMain);

   DrawWindowBackgroundBorder( NULL, hdc, area, HIGHLIGHT_THICKNESS, area->x, area->y, index, pExclude );

   ReleaseDC(hMain, hdc);
}
/************************************************************************/
/*
 * DrawWindowBackgroundBorder:  Draw a rectangle of thickness pixels around the
 *   outside of the given area.  xin and yin give the offset into the background
 *   texture (see DrawWindowBackgroundColor).
 *   index gives palette index color to draw (-1 for window background).
 */
void DrawWindowBackgroundBorder(RawBitmap *bg, 
				HDC hdc, AREA *area, int thickness, int xin, int yin, int index, DRAWBORDEREXCLUDE* pExclude )
{
   RECT r;
   // Redraw each of the four sides of the rectangle

   // Top side
   r.top    = area->y - thickness;
   r.bottom = r.top + thickness;
   if( !pExclude )
   {
		r.left   = area->x - thickness;
		r.right  = area->x + area->cx + thickness;
   }
   else
   {
		r.left   = area->x + pExclude->iTopEdge_Left;
		r.right  = area->x + area->cx - pExclude->iTopEdge_Right;
   }
   DrawWindowBackgroundColor(bg, hdc, &r, xin + r.left - area->x, yin + r.top - area->y, index);    
   
   // Left side
   if( !pExclude )
   {
		r.top    = area->y - thickness;
		r.bottom = area->y + area->cy + thickness;
   }
   else
   {
		r.top    = area->y + pExclude->iLeftEdge_Top;
		r.bottom = area->y + area->cy - pExclude->iLeftEdge_Bottom;
   }
   r.left   = area->x - thickness;
   r.right  = r.left + thickness;
   DrawWindowBackgroundColor(bg, hdc, &r, xin + r.left - area->x, yin + r.top - area->y, index);    
   
   // Bottom side
   r.top    = area->y + area->cy;
   r.bottom = r.top + thickness;      
   if( !pExclude )
   {
		r.left   = area->x - thickness;
		r.right  = area->x + area->cx + thickness;
   }
   else
   {
		r.left   = area->x + pExclude->iBottomEdge_Left;
		r.right  = area->x + area->cx - pExclude->iBottomEdge_Right;
   }
   DrawWindowBackgroundColor(bg, hdc, &r, xin + r.left - area->x, yin + r.top - area->y, index);    
   
   // Right side
   if( !pExclude )
   {
		r.top    = area->y - thickness;
		r.bottom = area->y + area->cy + thickness;      
   }
   else
   {
		r.top    = area->y + pExclude->iRightEdge_Top;
		r.bottom = area->y + area->cy - pExclude->iRightEdge_Bottom;
   }
   r.left   = area->x + area->cx;
   r.right  = r.left + thickness;
   DrawWindowBackgroundColor(bg, hdc, &r, xin + r.left - area->x, yin + r.top - area->y, index);
}
/************************************************************************/
void DrawChangeColor(void)
{
}
/************************************************************************/
/*
 * CreateMemBitmap:  Create a bitmap, select it into a new memory DC,
 *   and return the DC.  Also set the palette of the bitmap.
 *   gOldBitmap is set to the bitmap originally in the DC.
 *   gBits is set to point to the bits of the DC.
 *   Returns NULL on failure.
 */
HDC CreateMemBitmap(int width, int height, HBITMAP *gOldBitmap, BYTE **gBits)
{
   HDC gDC;
   HBITMAP gBitmap;
   struct {
      BITMAPINFOHEADER header;
      RGBQUAD ColorTable[NUM_COLORS];
   } info;

   gDC = CreateCompatibleDC(NULL);
   if (gDC == 0)
   {
      debug(("CreateMemBitmap Couldn't create DC!\n"));
      return NULL;
   }

   /* Always top down; easier and we don't really care about speed here */
   memset(&info, 0, sizeof(BITMAPINFOHEADER));
   info.header.biSize = sizeof(BITMAPINFOHEADER);
   info.header.biPlanes = 1;
   info.header.biBitCount = 8;
   info.header.biCompression = BI_RGB;
   info.header.biHeight = - height; 
   info.header.biWidth = width;
   gBitmap = CreateDIBSection(gDC, (BITMAPINFO *) &info, DIB_RGB_COLORS,
                              (void **) gBits, NULL, 0);
   if (gBitmap == 0)
   {
      debug(("CreateMemBitmap couldn't create bitmap!\n"));
      DeleteDC(gDC);
      return NULL;
   }
   *gOldBitmap = (HBITMAP) SelectObject(gDC, gBitmap);
   SetDIBPalette(gDC);
   return gDC;
}
/************************************************************************/
Bool DrawInitialize(void)
{
   if (InitializeGraphics3D() == False)
      return False;

   CacheInitialize();
   DrawBitmapInitialize();

   DrawChangeColor();

   MapInitialize();

   // Compute palette index for border of graphics area
//   border_index = GetClosestPaletteIndex(RGB(188, 152, 108));
   border_index = GetClosestPaletteIndex(RGB(103, 103, 103));

   return True;
}
/************************************************************************/
void DrawClose(void)
{
   FreeBitmaps();

   DrawBitmapClose();

   CloseGraphics3D();
   MapClose();
}

void DrawTransparentBytes(BYTE *dest, const BYTE *src, int count)
{
   __asm
   {
      mov   ecx, count;
      cmp   ecx, 0;
      je    END_DRAW_TRANSPARENT_BYTES;
      mov   ebx, TRANSPARENT_INDEX;
      mov   esi, src;
      mov   edi, dest;
      cmp   ecx, 4;
      jl    DO_LESS_THAN_4;
TRANSPARENT0:
      mov   al, BYTE PTR [esi];
      cmp   al, bl;
      jne   SOLID0;
TRANSPARENT1:
      mov   al, BYTE PTR [esi+1];
      cmp   al, bl;
      jne   SOLID1;
TRANSPARENT2:
      mov   al, BYTE PTR [esi+2];
      cmp   al, bl;
      jne   SOLID2;
TRANSPARENT3:
      mov   al, BYTE PTR [esi+3];
      cmp   al, bl;
      jne   SOLID3;
      jmp   DONE4;
SOLID0:
      mov   BYTE PTR [edi],al;
      mov   al, BYTE PTR [esi+1];
      cmp   al, bl;
      je    TRANSPARENT1;
SOLID1:
      mov   BYTE PTR [edi+1],al;
      mov   al, BYTE PTR [esi+2];
      cmp   al, bl;
      je    TRANSPARENT2;
SOLID2:
      mov   BYTE PTR [edi+2],al;
      mov   al, BYTE PTR [esi+3];
      cmp   al, bl;
      je    TRANSPARENT3;
SOLID3:
      mov   BYTE PTR [edi+3],al;
DONE4:
      add   esi,4;
      add   edi,4;
      sub   ecx,4;
      jz    END_DRAW_TRANSPARENT_BYTES;
      cmp   ecx,4;
      jge   TRANSPARENT0;
DO_LESS_THAN_4:
      mov   al, BYTE PTR [esi];
      cmp   al, bl;
      je    CHECK1;
      mov   BYTE PTR [edi],al;
      dec   ecx;
      jz    END_DRAW_TRANSPARENT_BYTES;
CHECK1:
      mov   al, BYTE PTR [esi+1];
      cmp   al, bl;
      je    CHECK2;
      mov   BYTE PTR [edi+1],al;
      dec   ecx;
      jz    END_DRAW_TRANSPARENT_BYTES;
CHECK2:
      mov   al, BYTE PTR [esi+2];
      cmp   al, bl;
      je    END_DRAW_TRANSPARENT_BYTES;
      mov   BYTE PTR [edi+2],al;
END_DRAW_TRANSPARENT_BYTES:
   }
}
