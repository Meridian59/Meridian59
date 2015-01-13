// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * objdraw.c:  Contains inner loops for drawing objects, based on special effect types.
 *   Each loop draws one row of an object on the screen.
 *
 * The inner loop functions are entered into a table that is indexed by the effect type,
 * which is sent from the server.
 */

#include "client.h"

#define BLACK_INDEX 0       // Index of black in the palette

static void DrawObjectTranslucent(ObjectRowData *d);
static void DrawObjectBlack(ObjectRowData *d);
static void DrawObjectSilhouette(ObjectRowData *d);
static void DrawObjectInvisible(ObjectRowData *d);
static void DrawObjectSimpleTranslate(ObjectRowData *d);
static void DrawObjectDoubleTranslate(ObjectRowData *d);
static void DrawObjectDitherTranslate(ObjectRowData *d);
static void DrawObjectSecondTranslate(ObjectRowData *d);
static void DrawObjectDither50Invisible(ObjectRowData *d);
static void DrawObjectDitherGrey(ObjectRowData *d);

static xlat* pHaloXlat = NULL;

DrawingLoop drawing_loops[NUM_DRAW_EFFECTS] = {
	NULL,                     // No effect
		DrawObjectTranslucent,    // 25% translucency
		DrawObjectTranslucent,    // 50% translucency
		DrawObjectTranslucent,    // 75% translucency
		DrawObjectBlack,          // solid black
		DrawObjectInvisible,      // invisibility effect
		DrawObjectSimpleTranslate,// palette translation
		DrawObjectDither50Invisible, // 50% dither invisible
		DrawObjectDitherTranslate, // 50% dither between two translates
		DrawObjectDoubleTranslate, // translate once, then twice
		DrawObjectSecondTranslate, // ignore first translate, use second translate
		DrawObjectDitherGrey,      // 50% dither invis, 50% greyscale
		DrawObjectSilhouette,     // solid color index
};

/************************************************************************/
#define INVISIBLE_MAGNITUDE 1   // Size of distortion for invisibility effect
void DrawObjectInvisible(ObjectRowData *d)
{
	BYTE *copy_ptr;
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *row_bits;
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	copy_ptr = start;

	if (rand() % 8 == 0)
	{
		if (d->ysize > 2 * INVISIBLE_MAGNITUDE)
			if (d->row <= INVISIBLE_MAGNITUDE)
				copy_ptr += INVISIBLE_MAGNITUDE * d->xsize;
			else copy_ptr -= INVISIBLE_MAGNITUDE * d->xsize;
			while (start <= end)
			{
				/* Don't draw transparent pixels */
				index = *(row_bits + (x >> FIX_DECIMAL));
				if (index != TRANSPARENT_INDEX)
					*start = *(copy_ptr - INVISIBLE_MAGNITUDE);
				
				/* Move to next column of screen */
				start++;
				copy_ptr++;
				x += xinc;
			}
	}
	else
	{
		if (d->ysize > 2 * INVISIBLE_MAGNITUDE)
			if (d->row + INVISIBLE_MAGNITUDE >= d->ysize)
				copy_ptr -= INVISIBLE_MAGNITUDE * d->xsize;
			else copy_ptr += INVISIBLE_MAGNITUDE * d->xsize;
			while (start <= end)
			{
				/* Don't draw transparent pixels */
				index = *(row_bits + (x >> FIX_DECIMAL));
				if (index != TRANSPARENT_INDEX)
					*start = *(copy_ptr + INVISIBLE_MAGNITUDE);
				
				/* Move to next column of screen */
				start++;
				copy_ptr++;
				x += xinc;
			}
	}
}
/************************************************************************/
void DrawObjectTranslucent(ObjectRowData *d)
{
	extern bixlat _blend25;
	extern bixlat _blend50;
	extern bixlat _blend75;

	bixlat* pBiXlat = &_blend75;
	BYTE index;
	BYTE effect;
	register BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	palette = d->palette;
	effect = d->drawingtype;

	switch( effect ) {
		case DRAWFX_TRANSLUCENT25:
			pBiXlat = &_blend25;
			break;
		case DRAWFX_TRANSLUCENT50:
			pBiXlat = &_blend50;
			break;
		default:
			break;
	}

//charlie: i`m using the next line for reference.
//#define fastBIXLAT(index1,index2,ptable)  (ptable->entry[((index1)<<8)|(index2)])

	while (start <= end)
	{
		/* Don't draw transparent pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX) {
			//*start = fastBIXLAT(palette[index], *start, pBiXlat);
			*start = (pBiXlat->entry[(palette[index]<<8)|(*start)]);
		}
			
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}
/************************************************************************/
void DrawObjectDither50Invisible(ObjectRowData *d)
{
	xlat *pXlat = FindStandardXlat(d->translation);
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	if (pXlat == NULL)
	{
		//debug(("DrawObjectDither50Invisible got unknown translation type %d\n", d->translation));
		return;
	}
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	palette = d->palette;
	
	while (start <= end)
	{
		/* Don't draw transparent pixels or half of remaining pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			if ((d->row ^ (int)start) & 1)
				*start = palette[fastXLAT(index, pXlat)];
			
			/* Move to next column of screen */
			start++;
			x += xinc;
	}
}
/************************************************************************/
void DrawObjectDitherGrey(ObjectRowData *d)
{
	xlat *pXlat = FindStandardXlat(XLAT_FILTERWHITE90);
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	if (pXlat == NULL)
	{
		//debug(("DrawObjectDitherGrey got unknown translation type %d\n", XLAT_FILTERWHITE90));
		return;
	}
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	palette = d->palette;
	
	while (start <= end)
	{
		/* Don't draw transparent pixels or half of remaining pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			if ((d->row ^ (int)start) & 1)
				*start = palette[fastXLAT(index, pXlat)];
			
			/* Move to next column of screen */
			start++;
			x += xinc;
	}
}
/************************************************************************/
void DrawObjectSilhouette(ObjectRowData *d)
{
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	start = d->start_ptr;
	end = d->end_ptr;
	row_bits = d->obj_bits;
	x = d->x;
	xinc = d->xinc;
	palette = d->palette;
	
	while (start <= end)
	{
		/* Don't draw transparent pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			*start = palette[d->secondtranslation];
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}
/************************************************************************/
void DrawObjectBlack(ObjectRowData *d)
{
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *row_bits;
	
	start = d->start_ptr;
	end = d->end_ptr;
	row_bits = d->obj_bits;
	x = d->x;
	xinc = d->xinc;
	while (start <= end)
	{
		/* Don't draw transparent pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			*start = BLACK_INDEX;
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}
/************************************************************************/
void DrawObjectSimpleTranslate(ObjectRowData *d)
{
	xlat *pXlat = FindStandardXlat(d->translation);
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	if (pXlat == NULL)
	{
		//debug(("DrawObjectSimpleTranslate got unknown translation type %d\n", d->translation));
		return;
	}
	
	start = d->start_ptr;
	end = d->end_ptr;
	row_bits = d->obj_bits;
	x = d->x;
	xinc = d->xinc;
	palette = d->palette;
	while (start <= end)
	{
		/* Don't draw transparent pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			*start = palette[fastXLAT(index, pXlat)];
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}
/************************************************************************/
void DrawObjectDoubleTranslate(ObjectRowData *d)
{
	xlat *pXlat;
	xlat *pXlat2;
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	if (d->secondtranslation == 0 || d->secondtranslation == d->translation)
	{
		DrawObjectSimpleTranslate(d);
		return;
	}
	
	pXlat = FindStandardXlat(d->translation);
	pXlat2 = FindStandardXlat(d->secondtranslation);
	if (pXlat == NULL || pXlat2 == NULL)
	{
		//debug(("DrawObjectDoubleTranslate got unknown translation type\n"));
		return;
	}
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	palette = d->palette;
	
	while (start <= end)
	{
		/* Don't draw transparent pixels, but go through two translates plus light-palette */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			*start = palette[fastXLAT(fastXLAT(index, pXlat), pXlat2)];
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}
/************************************************************************/
void DrawObjectSecondTranslate(ObjectRowData *d)
{
	xlat *pXlat2;
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	if (d->secondtranslation == 0)
	{
		DrawObjectSimpleTranslate(d);
		return;
	}
	
	pXlat2 = FindStandardXlat(d->secondtranslation);
	if (pXlat2 == NULL)
	{
		//debug(("DrawObjectDoubleTranslate got unknown translation type\n"));
		return;
	}
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	palette = d->palette;
	
	while (start <= end)
	{
		/* Don't draw transparent pixels, but go through two translates plus light-palette */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			*start = palette[fastXLAT(index, pXlat2)];
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}
/************************************************************************/
void DrawObjectDitherTranslate(ObjectRowData *d)
{
	xlat *pXlat;
	xlat *pXlat2;
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	if (d->secondtranslation == 0 || d->secondtranslation == d->translation)
	{
		DrawObjectSimpleTranslate(d);
		return;
	}
	
	pXlat = FindStandardXlat(d->translation);
	pXlat2 = FindStandardXlat(d->secondtranslation);
	if (pXlat == NULL || pXlat2 == NULL)
	{
		//debug(("DrawObjectDitherTranslate got unknown translation type %d\n", d->translation));
		return;
	}
	
	start = d->start_ptr;
	end = d->end_ptr;
	x = d->x;
	xinc = d->xinc;
	row_bits = d->obj_bits;
	palette = d->palette;
	
	while (start <= end)
	{
		/* Don't draw transparent pixels, but dither between translates plus light-palette */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
		{
			if ((d->row ^ (int)start) & 1)
				*start = palette[fastXLAT(index, pXlat)];
			else
				*start = palette[fastXLAT(index, pXlat2)];
		}
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
}

/************************************************************************/
void DrawTargetHalo( ObjectRowData* d, BYTE* obj_bits, long y, long yinc, long bitmap_width, Bool bTopRow, Bool bBottomRow )	//, Bool bClipHaloLeft, Bool bClipHaloRight )
{
	//	ajw
	//	Draw target selection effect around object.
	//	This occurs before object is drawn, so only stuff around the edges will show.
	
	xlat* pXlat = NULL;
	xlat* pXlatxxx = NULL;
	BYTE index;
	BYTE index_next, index_prev, index_above, index_below;
	BYTE* start, * end;
	BYTE* nontrans_start = NULL;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	//#define HALO_WIDTH	1
	
	row_bits	= d->obj_bits;
	xinc		= d->xinc;
	palette		= d->palette;
	
	pXlatxxx = FindStandardXlat( XLAT_BLEND50BLUE );
	switch( config.halocolor )
	{
	case 0:
		pXlat = FindStandardXlat( XLAT_BLEND75RED );
		break;
	case 1:
		pXlat = FindStandardXlat( XLAT_BLEND75BLUE );
		break;
	case 2:
	default:
		pXlat = FindStandardXlat( XLAT_BLEND75GREEN );
		break;
	}
	
	//	Hard-coded 1 pixel width halo; start and end points not included...
	x		= d->x;
	start	= d->start_ptr;
	end		= d->end_ptr;
	while( start <= end )
	{
		index = *(row_bits + (x >> FIX_DECIMAL));
		if( index == TRANSPARENT_INDEX )
		{
			if( start != end )
				index_next = *( row_bits + ( ( x + xinc ) >> FIX_DECIMAL ) );
			else
				index_next = TRANSPARENT_INDEX;
			if( start != d->start_ptr )
				index_prev = *( row_bits + ( ( x - xinc ) >> FIX_DECIMAL ) );
			else
				index_prev = TRANSPARENT_INDEX;
			if( !bTopRow )
				//				index_above = *( obj_bits + ( y >> FIX_DECIMAL ) * bitmap_width + ( x >> FIX_DECIMAL ) );
				index_above = *( obj_bits + ( ( y - yinc ) >> FIX_DECIMAL ) * bitmap_width + ( x >> FIX_DECIMAL ) );
			//				index_above = *( row_bits + ( x >> FIX_DECIMAL ) - lWidthBitmap );
			else
				index_above = TRANSPARENT_INDEX;
			if( !bBottomRow )
				//				index_below = *( obj_bits + ( y >> FIX_DECIMAL ) * bitmap_width + ( x >> FIX_DECIMAL ) );
				index_below = *( obj_bits + ( ( y + yinc ) >> FIX_DECIMAL ) * bitmap_width + ( x >> FIX_DECIMAL ) );
			//				index_below = *( row_bits + ( x >> FIX_DECIMAL ) + lWidthBitmap );
			else
				index_below = TRANSPARENT_INDEX;
			if( index_next != TRANSPARENT_INDEX || index_prev != TRANSPARENT_INDEX || index_above != TRANSPARENT_INDEX || index_below != TRANSPARENT_INDEX )
				*start = palette[ fastXLAT( *start, pXlat ) ];
		}
		start++;
		x += xinc;
	}
	
	/* outer edges only...
	//	Left side...
	start	= d->start_ptr;
	end		= d->end_ptr;
	x		= d->x;
	//	Find first non-transparent pixel.
	while( start <= end )
	{
	index = *(row_bits + (x >> FIX_DECIMAL));
	if( index != TRANSPARENT_INDEX )
	{
	nontrans_start = start;
	break;
	}
	start++;
	x += xinc;
	}
	if( !nontrans_start )
	//	No non-transparent pixels on line.
	return;
	
	  if( nontrans_start != d->start_ptr ) // && bClipHaloLeft,...
	  {
	  start = nontrans_start - HALO_WIDTH;
	  //	x = d->x + ( ( nontrans_start - start - HALO_WIDTH ) * xinc );
	  while( start < nontrans_start )
	  {
	  //			if( bClipHaloLeft )		//xxx
	  //				*start = palette[ fastXLAT( *start, pXlatxxx ) ];
	  //			else
	  *start = palette[ fastXLAT( *start, pXlat ) ];
	  //				*start = fastXLAT( *start, pXlat );
	  //				*start = 192;
	  start++;
	  //		x += xinc;
	  }
	  }
	  
		//	Right side.
		start	= d->end_ptr;
		end		= d->start_ptr;
		x		= d->x + xinc * ( start - end );
		//	Find last non-transparent pixel.
		while( start > end )
		{
		index = *(row_bits + (x >> FIX_DECIMAL));
		if( index != TRANSPARENT_INDEX )
		{
		nontrans_start = start;
		break;
		}
		start--;
		x -= xinc;
		}
		
		  if( nontrans_start != d->end_ptr )
		  {
		  start = nontrans_start + 1;
		  end = nontrans_start + 1 + HALO_WIDTH;
		  while( start < end )
		  {
		  //			if( bClipHaloRight )		//xxx
		  //				*start = palette[ fastXLAT( *start, pXlatxxx ) ];
		  //			else
		  *start = palette[ fastXLAT( *start, pXlat ) ];
		  //				*start = 192;
		  start++;
		  }
		  }
	*/
}

/************************************************************************/
void DrawTargetEffect( ObjectRowData* d )		//	ajw
{
	//	For showing user's selected target.
	//	Draws overs previously drawn image, acting on pixels already drawn.
	xlat* pXlat = FindStandardXlat( XLAT_BLEND25YELLOW );
	BYTE index;
	BYTE *start, *end;
	int x, xinc;
	BYTE *palette, *row_bits;
	
	start = d->start_ptr;
	end = d->end_ptr;
	row_bits = d->obj_bits;
	x = d->x;
	xinc = d->xinc;
	palette = d->palette;
	while (start <= end)
	{
		/* Don't draw transparent pixels */
		index = *(row_bits + (x >> FIX_DECIMAL));
		if (index != TRANSPARENT_INDEX)
			*start = palette[ fastXLAT( *start, pXlat ) ];
		
		/* Move to next column of screen */
		start++;
		x += xinc;
	}
	
}
