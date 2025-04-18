// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * draw3d.c
 * Draw first-person perspective view of room 
 */

/* Things to remember:
 * East = 0 radians, south = pi/2
 * South is direction of positive y 
 */

#include "client.h"

HDC gBitsDC;
BYTE *gBits;                /* Actual pixels of offscreen bitmap */
static HBITMAP gBitmap;     /* bitmap */
static HBITMAP gOldBitmap;  /* Original 1x1 monochrome bitmap */

/* Bitmap for stretching image before copying to screen */
static HDC gBufferDC;          
static HBITMAP gBufferBitmap;  
BYTE *gBufferBits; 

//	Bitmap for MiniMap.
static HDC gMiniMapDC;
static HBITMAP gMiniMapBitmap;
static BYTE* gMiniMapBits;

ViewElement ViewElements[NUM_VIEW_ELEMENTS] = {
  { IDB_VIEWTREAT_TOPLEFT,		0,   0,    NULL,   0,   0,  },
  { IDB_VIEWTREAT_TOPRIGHT,		0,   0,    NULL,   0,   0,  },
  { IDB_VIEWTREAT_BOTTOMLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_VIEWTREAT_BOTTOMRIGHT,	0,   0,    NULL,   0,   0,  } ,
  { IDB_VIEWTREAT_TOPLEFT_HILIGHT,		0,   0,    NULL,   0,   0,  },
  { IDB_VIEWTREAT_TOPRIGHT_HILIGHT,		0,   0,    NULL,   0,   0,  },
  { IDB_VIEWTREAT_BOTTOMLEFT_HILIGHT,	0,   0,    NULL,   0,   0,  },
  { IDB_VIEWTREAT_BOTTOMRIGHT_HILIGHT,	0,   0,    NULL,   0,   0,  } 
};
// Indices of interface elements in "elements"
#define VIEW_ELEMENT_TOPLEFT		0
#define VIEW_ELEMENT_TOPRIGHT		1
#define VIEW_ELEMENT_BOTTOMLEFT		2
#define VIEW_ELEMENT_BOTTOMRIGHT	3
#define VIEW_ELEMENT_TOPLEFT_HILIGHT		4
#define VIEW_ELEMENT_TOPRIGHT_HILIGHT		5
#define VIEW_ELEMENT_BOTTOMLEFT_HILIGHT		6
#define VIEW_ELEMENT_BOTTOMRIGHT_HILIGHT	7


/* Drawing parameters passed in */
Draw3DParams *drawParams;

extern player_info player;
extern HPALETTE hPal;

extern font_3d					gFont;

extern int num_visible_objects;

// Main client windows current viewport area
extern int main_viewport_width;
extern int main_viewport_height;

extern d3d_driver_profile gD3DDriverProfile;

long horizon;                   /* row of horizon */

AREA area;                      /* size and position of view window */

Vector3D sun_vect = { 0, FINENESS, 0 }; // sun is towards positive y
long shade_amount = (FINENESS>>1)+(FINENESS>>3);  // set max shading range (FINENESS-shade_amount) to FINENESS

// Depths (FINENESS units) corresponding to various sector depth settings
int sector_depths[] = {0, FINENESS / 5, 2 * FINENESS / 5, 3 * FINENESS / 5};

/* Palettes for different light levels */
extern BYTE light_palettes[NUM_PALETTES][NUM_COLORS];

BYTE light_rows[MAXY/2+1];      // Strength of light as function of screen row

PDIB background;                      /* Pointer to background bitmap */

long getShadeAmount()
{
	return shade_amount;
}

const Vector3D& getSunVector()
{
	return sun_vect;
}

/* local function prototypes */
static void StretchImage(void);
static void Stretch(BYTE* src, BYTE* dest, int width, int height,
    int new_width, int new_height, int max_width);
/************************************************************************/
/* return TRUE iff successful */
Bool InitializeGraphics3D(void)
{
	int i;
	BITMAPINFOHEADER* ptr;

   gBitsDC = CreateMemBitmap(MAXX, MAXY, &gOldBitmap, &gBits);
   gBufferDC = CreateMemBitmap(2*MAXX, 2*MAXY, &gOldBitmap, &gBufferBits);
   gMiniMapDC = CreateMemBitmap(MINIMAP_MAX_AREA, MINIMAP_MAX_AREA, &gOldBitmap, &gMiniMapBits);

   SetBkMode(gBitsDC, TRANSPARENT);
   GraphicsResetFont();

//   if (gD3DEnabled)
//	D3DRenderFontInit(&gFont, GetFont(FONT_LABELS));

   BSPInitialize();

	//	Load view elements for corner treatment.
	for( i = 0; i < NUM_VIEW_ELEMENTS; i++ )
	{
		ptr = GetBitmapResource( hInst, ViewElements[i].id );
		if( ptr == NULL )
		{
			ViewElements[i].bits = NULL;
			continue;
		}
		ViewElements[i].width  = ptr->biWidth;
		ViewElements[i].height = ptr->biHeight;
		ViewElements[i].bits = ( (BYTE*)ptr ) + sizeof( BITMAPINFOHEADER ) + NUM_COLORS * sizeof( RGBQUAD );
	}

   return True;
}
/************************************************************************/
void CloseGraphics3D(void)
{
   HBITMAP gCurrentBitmap;
   
   gCurrentBitmap = (HBITMAP) SelectObject(gBitsDC, gOldBitmap);
   DeleteObject(gCurrentBitmap);
   DeleteDC(gBitsDC);

   gCurrentBitmap = (HBITMAP) SelectObject(gBufferDC, gOldBitmap);
   DeleteObject(gCurrentBitmap);
   DeleteDC(gBufferDC);

   gCurrentBitmap = (HBITMAP) SelectObject( gMiniMapDC, gOldBitmap );
   DeleteObject( gCurrentBitmap );
   DeleteDC( gMiniMapDC );

	// ( Don't need to free view elements as they are loaded from resource file. )
}

/************************************************************************/
/*
 * ViewElementsReposition:  Called from GraphicsAreaResize. Reposition ViewElements treatment dibs.
 */
void ViewElementsReposition( AREA* pAreaView )
{
	ViewElements[ VIEW_ELEMENT_TOPLEFT ]	.x = 0;
	ViewElements[ VIEW_ELEMENT_TOPLEFT ]	.y = 0;
	ViewElements[ VIEW_ELEMENT_TOPRIGHT ]	.x = pAreaView->cx - ViewElements[ VIEW_ELEMENT_TOPRIGHT ].width;
	ViewElements[ VIEW_ELEMENT_TOPRIGHT ]	.y = 0;
	ViewElements[ VIEW_ELEMENT_BOTTOMLEFT ]	.x = 0;
	ViewElements[ VIEW_ELEMENT_BOTTOMLEFT ]	.y = pAreaView->cy - ViewElements[ VIEW_ELEMENT_BOTTOMLEFT ].height;
	ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT ].x = pAreaView->cx - ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT ].width;
	ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT ].y = pAreaView->cy - ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT ].height;
	ViewElements[ VIEW_ELEMENT_TOPLEFT_HILIGHT ]	.x = 	ViewElements[ VIEW_ELEMENT_TOPLEFT ]	.x;
	ViewElements[ VIEW_ELEMENT_TOPLEFT_HILIGHT ]	.y = 	ViewElements[ VIEW_ELEMENT_TOPLEFT ]	.y;
	ViewElements[ VIEW_ELEMENT_TOPRIGHT_HILIGHT ]	.x = 	ViewElements[ VIEW_ELEMENT_TOPRIGHT ]	.x;
	ViewElements[ VIEW_ELEMENT_TOPRIGHT_HILIGHT ]	.y = 	ViewElements[ VIEW_ELEMENT_TOPRIGHT ]	.y;
	ViewElements[ VIEW_ELEMENT_BOTTOMLEFT_HILIGHT ]	.x = 	ViewElements[ VIEW_ELEMENT_BOTTOMLEFT ]	.x;
	ViewElements[ VIEW_ELEMENT_BOTTOMLEFT_HILIGHT ]	.y = 	ViewElements[ VIEW_ELEMENT_BOTTOMLEFT ]	.y;
	ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT_HILIGHT ].x = 	ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT ].x;
	ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT_HILIGHT ].y = 	ViewElements[ VIEW_ELEMENT_BOTTOMRIGHT ].y;
}

/************************************************************************/
/*
 * GraphicsResetFont:  Called when font changed.
 */
void GraphicsResetFont(void)
{
	HFONT	hFont;

	hFont = GetFont(FONT_LABELS);

   // Set up font for drawing names on graphics window
   SelectObject(gBitsDC, hFont);

   if (gD3DEnabled)
     D3DRenderFontInit(&gFont, hFont);
}

/************************************************************************/
void DrawPreOverlayEffects(room_type* room, Draw3DParams* params)
{
	PDIB pdibCeiling = NULL;

	// Only drawn in room, before overlays are drawn.

	pdibCeiling = GetPointCeilingTexture(params->viewer_x, params->viewer_y);

	// sand
	if (effects.sand)
	{
		SandDib(gBits, MAXX, MAXY, 200/*drops*/);
		RedrawAll();
	}

#if 0
	// rain
	if (effects.raining && !pdibCeiling)
	{
		RainDib(gBits, MAXX, MAXY, 100/*drops*/, params->viewer_angle/*myheading*/, 0/*windheading*/, 10/*windstrength*/, TRUE/*torch*/);
		RedrawAll();
	}

	// snow
	if (effects.snowing && !pdibCeiling)
	{
		SnowDib(gBits, MAXX, MAXY, 100/*drops*/, params->viewer_angle/*myheading*/, 0/*windheading*/, 10/*windstrength*/, TRUE/*torch*/);
		RedrawAll();
	}
#endif
}

/************************************************************************/
void DrawPostOverlayEffects(room_type* room, Draw3DParams* params)
{
   static DWORD timeLastFrame = 0;
   DWORD timeCurrent, timeDelta;
   int amount;

   timeCurrent = timeGetTime();
   timeDelta = timeCurrent - timeLastFrame;
   timeLastFrame = timeCurrent;

   // May be drawn over room or map.

   // Blurred Vision.
   if (effects.blur)
   {
      // Blur by 1-EFFECT_BLUR_AMPLITUDE pixels
      amount = (effects.blur / EFFECT_BLUR_RATE) % (2 * EFFECT_BLUR_AMPLITUDE);
      if (amount > EFFECT_BLUR_AMPLITUDE)
	 amount = 2 * EFFECT_BLUR_AMPLITUDE - amount;
      amount++;

      BlurDib(gBits, MAXX, MAXY, amount);
      RedrawAll();
   }

   // Wavering Vision.
   if (effects.waver)
   {
      static int offset = 0;
      offset++;
      WaverDib(gBits, MAXX, MAXY, offset);
      RedrawAll();
   }

   // Flash of XLAT.  Could be color, blindness, whatever.
   if (effects.flashxlat != XLAT_IDENTITY)
   {
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(effects.flashxlat));
      effects.duration -= (int)timeDelta;
      if (effects.duration <= 0)
      {
	 effects.flashxlat = XLAT_IDENTITY;
	 effects.duration = 0;
      }
   }

   if (effects.xlatOverride > 0)
   {
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(effects.xlatOverride));
      RedrawAll();
      return;
   }

   // Whiteout
   if (effects.whiteout > 500)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND100WHITE));
   else if (effects.whiteout > 250)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND90WHITE));
   else if (effects.whiteout > 0)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND80WHITE));
   
   // Pain (always drawn last).
   if (!config.pain)
      return;
   if (effects.pain > 2000)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND80RED));
   else if (effects.pain > 1000)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND70RED));
   else if (effects.pain > 500)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND60RED));
   else if (effects.pain > 400)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND50RED));
   else if (effects.pain > 300)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND40RED));
   else if (effects.pain > 200)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND30RED));
   else if (effects.pain > 100)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND20RED));
   else if (effects.pain)
      XlatDib(gBits, MAXX, MAXY, FindStandardXlat(XLAT_BLEND10RED));
}

/************************************************************************/
void DrawViewTreatment()
{
   if (gD3DDriverProfile.bSoftwareRenderer == TRUE)
   {
      //	Draw view elements (edge treatment): added by ajw.
      int i;
      int iOffset = 0;

      BYTE* pBitsTarget = gBufferBits;
      int iWidthTarget = MAXX*2;

      if (GetFocus() == hMain)
         iOffset = 4;

      for (i = iOffset; i < iOffset + (NUM_VIEW_ELEMENTS / 2); i++)
      {
         BitCopy(pBitsTarget, iWidthTarget, ViewElements[i].x, ViewElements[i].y, ViewElements[i].width, ViewElements[i].height,
            ViewElements[i].bits, 0, 0, DIBWIDTH(ViewElements[i].width), OBB_FLIP | OBB_TRANSPARENT);
         GdiFlush();
      }
   }

   //	Ensure that border, which covers up parts of view treatment, is drawn.
   DrawGridBorder();
   GdiFlush();
}

/************************************************************************/
void DrawRoom3D(room_type *room, Draw3DParams *params)
{
   long t1,t2,t3,t4,t5;
   static int count = 0;
   
   /* write stuff in static variables */
   drawParams = params;
   
   /* Size of offscreen bitmap */
   area.x = area.y = 0;
   area.cx = CLASSIC_WIDTH;
   area.cy = CLASSIC_HEIGHT;

   // Force size to be even
   area.cy = area.cy & ~1;  
   area.cx = area.cx & ~1;

   /* some precalculations */
   horizon = area.cy/2 + PlayerGetHeightOffset();
   num_visible_objects = 0;

   t1=timeGetTime();
   DrawBSP(room, params, area.cx, True);
   t2=timeGetTime();
   DrawPreOverlayEffects(room, params);
   if (!player.viewID)
      DrawPlayerOverlays();
   DrawPostOverlayEffects(room, params);
   t3=timeGetTime();
   StretchImage();   
   t4=timeGetTime();
   //	Draw corner treatment.
   DrawViewTreatment();
   //	Copy offscreen buffer to screen.
   if (!D3DRenderIsEnabled())
   {
		RecopyRoom3D( params->hdc, params->x, params->y, params->width, params->height, FALSE );
		GdiFlush();
   }
   t5=timeGetTime();
   
   count++;
   if (count > 500)
   {
      debug(("BSP draw %ldms, overlays %dms, stretch %ldms, treatment and copy %ldms\n", t2-t1, t3-t2, t4-t3, t5-t4));
      count = 0;
   }
}

void UpdateRoom3D(room_type *room, Draw3DParams *params)
{
   long t1,t2,t3;
   static int count = 0;

   // View area of the player (as per software renderer view/fov)
   area.x = area.y = 0;
   area.cx = CLASSIC_WIDTH;
   area.cy = CLASSIC_HEIGHT;

   // Force size to be even.
   area.cy = area.cy & ~1;
   area.cx = area.cx & ~1;

   // Horizon is used by drawbsp.c to determine which objects are visible.
   horizon = area.cy / 2 + PlayerGetHeightOffset();

   drawParams = params;
   num_visible_objects = 0;

   t1=timeGetTime();
   DrawBSP(room, params, area.cx, False);
   t2=timeGetTime();

   // Draw corner treatment.
   DrawViewTreatment();

   // Copy offscreen buffer to screen.
   if (!D3DRenderIsEnabled())
   {
		RecopyRoom3D( params->hdc, params->x, params->y, params->width, params->height, False );
		GdiFlush();
   }
   t3=timeGetTime();
   
   count++;
   if (count > 500)
   {
      debug(("BSP draw %ldms, treatment and copy %ldms\n", t2-t1, t3-t2));
      count = 0;
   }
}

/************************************************************************/
/*
 * DrawMap: Draw map in area described by params.
 */
void DrawMap( room_type *room, Draw3DParams *params, Bool bMiniMap )
{
   AREA area;
   HDC gDC;
   BYTE *bits;
   int width;
   long t1, t2, t3, t4, t5;
   static int count = 0;
   
   int num_visible_object_SavedForMiniMapHack;

   area.x = area.y = 0;
   area.cx = params->width & ~1;
   area.cy = params->height & ~1;

   if( !bMiniMap )
   {
		  gDC = gBufferDC;
		  bits = gBufferBits;
		  width = 2 * MAXX;
   }
   else
   {
		//	It seems num_visible_objects needs to be zero initially, and as a result of processing, is left at some
		//	actual value for number of objects in the room. Running this proc for the MiniMap is resulting in this
		//	var being left zero at the end. Unsure why. For now preserve the old value of num_visible_objects to restore
		//	after processing. ajw
		num_visible_object_SavedForMiniMapHack = num_visible_objects;
		gDC		= gMiniMapDC;
		bits	= gMiniMapBits;
		width	= MINIMAP_MAX_AREA;
   }

   num_visible_objects = 0;

   t1 = timeGetTime();
   // Trace BSP tree to see if more walls are visible
   DrawBSP(room, params, MAXX, False);

   t2 = timeGetTime();

   t3 = timeGetTime();
   MapDraw( gDC, bits, &area, room, width, bMiniMap );
   DrawPostOverlayEffects(room, params);
   t4 = timeGetTime();
   RecopyRoom3D( params->hdc, params->x, params->y, params->width, params->height, bMiniMap );
   t5 = timeGetTime();

   if( bMiniMap )
	   num_visible_objects = num_visible_object_SavedForMiniMapHack;

   if (count++ % 50 == 0)
      debug(("Map trace %ld, clear %ld, draw %ld, copy %ld\n", t2-t1, t3-t2, t4-t3, t5-t4));
}
/*****************************************************************************/
/*
 * SetLightingInfo:  set variables for directional shading.
 *   sun_x, sun_y       give the x and y position of the sun, x is in [0, NUMDEGREES)
 *                      and y is the height of the sun in pixels
 *                      (same units as background overlay).
 *   intensity          strength of shading, in [0, 64), 0 = no shading
 *   We convert this to:		        
 *	sun_vect	3D vector pointing towards sun, magnitude FINENESS
 *	shade_amount	range of shading affect darkest is (sector light)*(FINENESS-shade_amount)/FINENESS
 */
void SetLightingInfo(int sun_x, int sun_y, BYTE intensity) 
{
   sun_vect.x = COS(sun_x) >> (FIX_DECIMAL - LOG_FINENESS);
   sun_vect.y = SIN(sun_x) >> (FIX_DECIMAL - LOG_FINENESS);
   sun_vect.z = 0;  // Hey Andrew - You need to fix this!
   
	shade_amount = intensity * FINENESS / 64;
}
/************************************************************************/

/*
   row numbering systems (area.cy must be even):
   
   screen row numbering		ceiling row numbering		floor row numbering
   --------------------		---------------------		-------------------
   0				area.cy/2			- area.cy/2 + 1	top ceiling row
   1				area.cy/2 - 1			- area.cy/2 + 2
   .				.				.
   .				.				.
   .				.				.
   area.cy/2 - 2		2			        -1
   area.cy/2 - 1		1				0		bottom ceiling row
   area.cy/2		        0				1		top floor row
   area.cy/2 + 1		-1				2
   .				.				.
   .				.				.
   .				.				.
   area.cy - 2			- area.cy/2 + 2			area.cy/2 - 1
   area.cy - 1			- area.cy/2 + 1			area.cy/2	bottom floor row
   
   */


/************************************************************************/
/* 
 * StretchImage:  Double the size of image by copying the square
 * (0,0) -> (area.cx,area.cy) at gBits into the square 
 * (0,0) -> (2*area.cx,2*area.cy) at gBufferBits
 */
void StretchImage(void)
{
  Stretch(gBits, gBufferBits, area.cx, area.cy, main_viewport_width, main_viewport_height, MAXX);
}
/************************************************************************/
/************************************************************************/
/*
 * Stretch: Function to stretch a source image to a new size while maintaining the aspect ratio.
 * - src: Pointer to the source image data (bytes)
 * - dest: Pointer to the destination image buffer where the stretched image will be stored (bytes).
 * - width: Width of the source image in pixels.
 * - height: Height of the source image in pixels.
 * - new_width: Desired width of the stretched image in pixels.
 * - new_height: Desired height of the stretched image in pixels.
 * - row_stride: The row stride in bytes (used for row size calculations).
 */
void Stretch(BYTE* src, BYTE* dest, int width, int height, 
    int new_width, int new_height, int row_stride)
{
    // Calculate the scaling ratios
    int half_new_width = new_width / 2;
    int x_ratio = (width << 16) / half_new_width;
    int y_ratio = (height << 16) / new_height;

    for (int i = 0; i < new_height; i++)
    {
        // Calculate the y-coordinate in the source image
        int y = (i * y_ratio) >> 16;
        BYTE* s = src + y * row_stride; // Set source pointer

        // Calculate the destination pointers for the current and next rows
        BYTE* d = dest + i * row_stride * 2;
        BYTE* d2 = d + row_stride;

        for (int j = 0; j < half_new_width; j++)
        {
            // Calculate the x-coordinate in the source image
            int x = (j * x_ratio) >> 16;
            BYTE b = *(s + x); // Fetch the pixel from the source

            // Write the pixel to the destination, expanding both horizontally and vertically
            *(d + 2 * j) = b;
            *(d + 2 * j + 1) = b;
            *(d2 + 2 * j) = b;
            *(d2 + 2 * j + 1) = b;
        }
    }
}
/************************************************************************/
/*
 * RecopyRoom3D: Redraw portion of room in response to an expose event
 *   (just copy it from offscreen) 
 *   The viewing area should be of size (width, height).  
 *   The room will be copied to (x,y) of the given DC.  This area will
 *   be centered on the offscreen bitmap that contains the current
 *   view. 
 */
void RecopyRoom3D( HDC hdc, int x, int y, int width, int height, Bool bMiniMap )
{
   HDC gCopyDC;  // DC to copy from

   SelectPalette(hdc, hPal, FALSE);

   if( !bMiniMap )
   {
   width =  min(width, 2 * MAXX);
   height = min(height, 2 * MAXY);
   gCopyDC = gBufferDC;
   }
   else
   {
	   gCopyDC = gMiniMapDC;
   }

   BitBlt(hdc, x, y, width, height, gCopyDC, 0, 0, SRCCOPY);

   return;
}
/************************************************************************/
/*
 * Return floor(log base 2(x)).  Assumes that x > 0.
 */
int DiscreteLog(int x)
{
   int count = 0;
   while (x != 0)
   {
      x = x >> 1;
      count++;
   }
   return count - 1;
}
/************************************************************************/
/* 
 * FindOffsets:  Calculate dx and dy for a point at distance d along
 *   a line at angle theta.
 *   Modifies dx and dy.
 */
void FindOffsets(int d, long theta, int *dx, int *dy)
{
   *dx = (int) FixToLong(FixMul(LongToFix(d), COS(theta)));
   *dy = (int) FixToLong(FixMul(LongToFix(d), SIN(theta)));
}
/************************************************************************/
/*
 * EnterNewRoom3D:  Call whenever a new room is entered.  Performs precomputation
 *   to speed drawing.
 */
void EnterNewRoom3D(room_type *room)
{
   NewBackground3D(room->bkgnd);
   BSPEnterRoom(room);
}
/************************************************************************/
/*
 * NewBackground3D:  Call to change background bitmap of room.
 */
void NewBackground3D(ID bkgnd)
{
   /* Load background bitmap, if any */
   background = GetBackgroundPdib(bkgnd);
}
/************************************************************************/
/*
 * LightChanged3D:  Call when ambient or player light changes; recalculate palettes
 */
void LightChanged3D(BYTE viewer_light, BYTE ambient_light)
{
   int row;
   int temp_hv = FINENESS * VIEWER_DISTANCE;

   //   debug(("Viewer light = %d\n", (int) viewer_light));

   for (row = 1; row <= MAXY/2; row++)
   {
      int distance, x;
      /* Pick palette to use for rows based on distance & light level */
      distance = temp_hv / row;
      x = LIGHT_INDEX(distance, (int) viewer_light, (int) ambient_light);
      light_rows[row] = x;
   }
}
/************************************************************************/
/*
 * GetLightPalette:  Given a distance (in FINENESS units) and the light level at a point in 
 *   the room, return the correct palette.  lightOffset was added to have self lighted objects
 */
BYTE *GetLightPalette(int distance, BYTE sector_light, long scale, int lightOffset)
{
   int index, row;

   // Check for special palette effects
   if (effects.invert > 0)
      return light_palettes[PALETTE_INVERT];

   if (IsBlind())
      return light_palettes[0];

   // See if sector is affected by ambient light
   if (sector_light > 127)
   {
      row = ((FINENESS/2) << LOG_VIEWER_DISTANCE) / distance;
      if (row < 1)
	 row = 1;
      if (row > MAXY / 2)
	 row = MAXY / 2;

      index = ((int) light_rows[row] + (int) sector_light - LIGHT_NEUTRAL) * 
	 LIGHT_LEVELS / MAX_LIGHT;  // Scale from 0-255 to # of palettes

   }
   else index = LIGHT_INDEX(distance, (int) drawParams->viewer_light, 0) * LIGHT_LEVELS / MAX_LIGHT
      + (int) sector_light / 2;

   if ((scale != FINENESS) && (sector_light > 127))
       index = (scale * index)>>LOG_FINENESS;

   index += lightOffset;
   index = max(index, 0);
   index = min(index, LIGHT_LEVELS - 1);

   return light_palettes[index];
}

int GetLightPaletteIndex(int distance, BYTE sector_light, long scale, int lightOffset)
{
   int index, row;

   // Check for special palette effects
   if (effects.invert > 0)
      return PALETTE_INVERT;

   if (IsBlind())
      return 0;

   if (distance == 0)
	   distance = 1;

   // See if sector is affected by ambient light
   if (sector_light > 127)
   {
      row = ((FINENESS/2) << LOG_VIEWER_DISTANCE) / distance;
      if (row < 1)
	 row = 1;
      if (row > MAXY / 2)
	 row = MAXY / 2;

      index = ((int) light_rows[row] + (int) sector_light - LIGHT_NEUTRAL) * 
	 LIGHT_LEVELS / MAX_LIGHT;  // Scale from 0-255 to # of palettes

   }
   else index = LIGHT_INDEX(distance, (int) drawParams->viewer_light, 0) * LIGHT_LEVELS / MAX_LIGHT
      + (int) sector_light / 2;

   if ((scale != FINENESS) && (sector_light > 127))
       index = (scale * index)>>LOG_FINENESS;

   index += lightOffset;
   index = max(index, 0);
   index = min(index, LIGHT_LEVELS - 1);

   return index;
}

int GetFlicker(room_contents_node *r)
{
   return r->obj.lightAdjust;
}

void DrawMapAsView(room_type *room, Draw3DParams *params)
{
   AREA area;
   HDC gDC;
   BYTE *bits;
   int width;
   
   //int num_visible_object_SavedForMiniMapHack;

   area.x = area.y = 0;
   area.cx = params->width & ~1;
   area.cy = params->height & ~1;

   gDC = gBufferDC;
   bits = gBufferBits;
   width = 2 * MAXX;

   // Trace BSP tree to see if more walls are visible
   DrawBSP(room, params, MAXX, False);
   MapDraw(gDC, bits, &area, room, width, FALSE);
   DrawPostOverlayEffects(room, params); // In case player blind or drunk
   DrawViewTreatment();
   RecopyRoom3D( params->hdc, params->x, params->y, params->width, params->height,FALSE);
}

void DrawMiniMap(room_type *room, Draw3DParams *params)
{
   AREA area,areaMiniMap;
   int num_visible_object_SavedForMiniMapHack = num_visible_objects;

   CopyCurrentAreaMiniMap(&areaMiniMap);
   area = areaMiniMap;
   area.x = area.y = 0;
   MapDraw(gMiniMapDC, gMiniMapBits, &area, room, MINIMAP_MAX_AREA, TRUE);
   num_visible_objects = num_visible_object_SavedForMiniMapHack; // restore
   RecopyRoom3D(params->hdc, areaMiniMap.x, areaMiniMap.y, area.cx, area.cy,TRUE);
}

const Draw3DParams& getDrawParams()
{
	return *drawParams;
}

void setDrawParams(Draw3DParams* newDrawParams)
{
	drawParams = newDrawParams;
}
