/*----------------------------------------------------------------------------*
 | This file is part of WinDEU, the port of DEU to Windows.                   |
 | WinDEU was created by the DEU team:                                        |
 |  Renaud Paquay, Raphael Quinet, Brendon Wyber and others...                |
 |                                                                            |
 | DEU is an open project: if you think that you can contribute, please join  |
 | the DEU team.  You will be credited for any code (or ideas) included in    |
 | the next version of the program.                                           |
 |                                                                            |
 | If you want to make any modifications and re-distribute them on your own,  |
 | you must follow the conditions of the WinDEU license. Read the file        |
 | LICENSE or README.TXT in the top directory.  If do not  have a copy of     |
 | these files, you can request them from any member of the DEU team, or by   |
 | mail: Raphael Quinet, Rue des Martyrs 9, B-4550 Nandrin (Belgium).         |
 |                                                                            |
 | This program comes with absolutely no warranty.  Use it at your own risks! |
 *----------------------------------------------------------------------------*

	Project WinDEU
	DEU team
	Jul-Dec 1994, Jan-Mar 1995

	FILE:         mapdc.cpp

	OVERVIEW
	========
	Source file for implementation of TMapDC (TDC)
*/

#include "common.h"
#pragma hdrstop

#include <math.h>
#include <limits.h>

#include "mapdc.h"

#ifndef __infocont_h
	#include "infocont.h"
#endif

#ifndef __editcli_h
	#include "editcli.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif
#include "bsp.h"


/*
	09-01-95: RP

	  New define option to be able to AVOID the use a the cache for pens.
	  It's for debug purpose (to know if a problem is bound to the use
	  of the cache or not) that I undefine USE_PEN_CACHE.
*/

#define USE_PEN_CACHE

/*
	Use Windows scaling, or use program computed scaling.
	Windows scaling should be faster than program computed, but it does
	not seem to be the case (Maybe under Win32 ?).

	I think it depends on the graphics driver too, so try this if you
	find the drawing too slow on your computer
*/

// #define WINDOWS_SCALING

/*
	Use program computed clipping, or Windows clipping.
	Same note as above
*/

// #define WINDOWS_CLIPPING


/*
	Test if a rect is visible on the map
*/

#define IS_VISIBLE_MAP_RECT(x0,y0,x1,y1)    		\
(                                       			\
  !(                                                \
	((x0) < VisMapMinX && (x1) < VisMapMinX) || 	\
	((x0) > VisMapMaxX && (x1) > VisMapMaxX) || 	\
	((y0) < VisMapMinY && (y1) < VisMapMinY) || 	\
	((y0) > VisMapMaxY && (y1) > VisMapMaxY)    	\
   )                                                \
)


//
// Drawing base colors for editor
//
const TColor Color16Tab[16] =
{
	RGB(0,0,0),			// BLACK
	RGB(0,0,160),		// BLUE
	RGB(0,160,0),   	// GREEN
	RGB(0,160,160), 	// CYAN
	RGB(160,0,0),   	// RED
	RGB(160,0,160),		// MAGENTA
	RGB(128,64,0),		// BROWN
	RGB(160,160,160),	// LIGHTGRAY
	RGB(100,100,100),	// DARKGRAY
	RGB(0,0,255),		// LIGHTBLUE
	RGB(0,255,0),		// LIGHTGREEN
	RGB(0,255,255),		// LIGHTCYAN
	RGB(255,0,0),		// LIGHTRED
	RGB(255,0,255),		// LIGHTMAGENTA
	RGB(255,255,0),   	// YELLOW
	RGB(255,255,255)   	// WHITE
};

const char *Color16NameTab[16] =
{
	"BLACK",
	"BLUE",
	"GREEN",
	"CYAN",
	"RED",
	"MAGENTA",
	"BROWN",
	"LIGHTGRAY",
	"DARKGRAY",
	"LIGHTBLUE",
	"LIGHTGREEN",
	"LIGHTCYAN",
	"LIGHTRED",
	"LIGHTMAGENTA",
	"YELLOW",
	"WHITE"
} ;



//////////////////////////////////////////////////////////
//
// TPenColor implementation
//
//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
// TPenColor16
// -----------
//
TPenColor16::TPenColor16 (int _color, int _width, UINT _style):
	TPen (TMapDC::GetColor16(_color), _width, _style)
{
	color = _color;
	width = _width;
	style = _style;
	HitCount = (ULONG)0;
//	TRACE ("TPenColor(" << Color16NameTab[color] << ", " <<
//		   width << ", " << style << "): constructor");
}


//////////////////////////////////////////////////////////
// TPenColor16
// -----------
//
TPenColor16::~TPenColor16()
{
//	TRACE ("TPenColor(" << Color16NameTab[color] << ", " <<
//		   width << ", " << style << "): destructor");
}



//////////////////////////////////////////////////////////
//
// TMapDC implementation
//
//////////////////////////////////////////////////////////


//
// Cache of pointers to last used pens
//
#ifdef USE_PEN_CACHE
TPenColor16 **TMapDC::LastPens = NULL;
#endif


#ifdef USE_PEN_CACHE
//////////////////////////////////////////////////////////
// TMapDC
// ------
//	Initialize GDI pen cache
void
TMapDC::InitCacheData()
{
	TRACE ("TMapDC::InitCacheData called");
	if ( LastPens == NULL )
	{
		LastPens = (TPenColor16 **)GetMemory (COLOR16_CACHE_SIZE * sizeof (TPenColor16 *));

		for (int i = 0 ; i < COLOR16_CACHE_SIZE ; i++)
		{
			LastPens[i] = 0;
		}

		atexit (CleanupCacheData);
	}
}
#endif

#ifdef USE_PEN_CACHE
//////////////////////////////////////////////////////////
// TMapDC
// ------
//	Cleanup GDI pen cache
void _USERENTRY
TMapDC::CleanupCacheData()
{
	TRACE ("TMapDC::CleanupCacheData called");
	if ( LastPens != NULL )
	{
		// NextPos = 0 ;
		for (int i = 0 ; i < COLOR16_CACHE_SIZE ; i++)
		{
			delete LastPens[i];
		}
		FreeMemory (LastPens);
	}
}
#endif


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  Returns the COLORREF equivalent of a predefined doom editor color
TColor TMapDC::GetColor16 (int color)
{
	assert (color >= 0 && color <= 15);

	return (Color16Tab[color]);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
// Returns a TColorPen correponding to color, width and style.
// This function uses a cache of most rencently used pens to avoid
// too many GDI API calls.
TPenColor16 *TMapDC::GetPenColor16(int color, int width, UINT style)
{
	// Pointer to the returned pen
	TPenColor16 *pen = NULL ;

#ifndef USE_PEN_CACHE
	pen = new TPenColor16 (color, width, style);
#else
	// Call counter to know least recently used pen in cache
	static ULONG CallCount = 0;

	//
	// Search for pen in cache
	//
	for (int i = 0 ; i < COLOR16_CACHE_SIZE ; i++)
	{
		// If new pen = a pen in the cache
		if ( LastPens[i]        != 0     &&
			 LastPens[i]->color == color &&
			 LastPens[i]->width == width &&
			 LastPens[i]->style == style)
		{
			pen = LastPens[i];
			LastPens[i]->HitCount = CallCount;
#if 0
			TRACE ("GetPenColor16() Cache hit: " << i << " (" <<
				   Color16NameTab[LastPens[i]->color] << ")");
#endif
			break ;
		}
	}

	// If pen not in cache
	if ( pen == NULL )
	{
		// Search an unused or the least recently used pen in the cache
		ULONG MinHitCount = ULONG_MAX;
		int PenPos = -1;
		for (int i = 0 ; i < COLOR16_CACHE_SIZE ; i++)
		{
			if ( LastPens[i] == NULL )
			{
				PenPos = i;
				break ;
			}

#if 0
			TRACE ("GetPenColor16()      Replace Candidate" <<
				   "i = " << i <<
				   ", Color = " << Color16NameTab[LastPens[i]->color] <<
				   ", HitCount = " << LastPens[i]->HitCount <<
				   ", MinHitCount = " << MinHitCount);
#endif

			if ( LastPens[i]->HitCount < MinHitCount )
			{
				MinHitCount = LastPens[i]->HitCount;
				PenPos = i;
			}
		}
		assert (PenPos != -1);

#if 0
		TRACE ("GetPenColor16() Cache replace: " <<
			   "PenPos = " << PenPos <<
			   ", Color = " << (LastPens[PenPos] != NULL ? Color16NameTab[LastPens[PenPos]->color] : "NULL") <<
			   ", MinHitCount = " << MinHitCount <<
			   ", CallCount = " << CallCount);
#endif

		// Destroy old pen in cache
		// NOTE:'delete' accepts 0 (and does nothing in that case)
		delete LastPens[PenPos];

		// Create a new pen in cache
		pen = LastPens[PenPos] = new TPenColor16 (color, width, style);
		pen->HitCount = CallCount;
	}

	// Return the pen
	CallCount++;
#endif // USE_PEN_CACHE
	return (pen);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//
TMapDC::TMapDC (TEditorClient *pEditor, TDC &dc):
	TDC ((HDC)dc)
{
	FromWindow = FALSE;
	Init (pEditor);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//
TMapDC::TMapDC (TEditorClient *pEditor):
	TDC (::GetDC(pEditor->Handle))
{
	FromWindow = TRUE;
	Init (pEditor);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//
TMapDC::~TMapDC()
{
	// Restore first pen of DC
	OrgPen = hOldPen;
	RestoreObjects();

	// If DC created with GetDC, call ReleseDC
	if ( FromWindow )
	{
		::ReleaseDC (pEditor->Handle, GetHDC());
		Handle = 0;		// DC destroyed
	}
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//
void
TMapDC::Init (TEditorClient *_pEditor)
{
#ifdef USE_PEN_CACHE
	if ( LastPens == NULL )		InitCacheData();
#endif

	// Pointer to editor client window
	pEditor = _pEditor;

	DoDelete = FALSE;
	CurPenColor = -1;
	CurPenWidth = 0;
	CurPenStyle = 0;

	hOldPen = 0;

	// Don't fill figures
	SelectStockObject (NULL_BRUSH);

	// Setup clipping region and Window and DC origins
	// SetupClipping();
	SetupOrg();
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//	Set pen color, width and style.
//	Call the GetPenColor16 to use the cache of most recently used pens.
void
TMapDC::SetNewPenColor16 (int color, int width, UINT style)
{
	int nwidth = width;
#ifdef WINDOWS_SCALING
	// Adjust logical pen width to screen width
	if (width > 1  &&  Scale < 1.0)
		nwidth = width * DIV_SCALE;
#endif

#ifdef USE_PEN_CACHE
	TPenColor16 *newPen = GetPenColor16 (color, nwidth, style);
	SelectObject (*newPen);
#else
	static TPenColor16 *newPen = 0;
	delete newPen;
	newPen = new TPenColor16 (color, nwidth, style);
	SelectObject (*newPen);
#endif

	// Save current pen definition
	CurPenColor = color;
	CurPenWidth = width;
	CurPenStyle = style;

	// Save first selected pen
	if ( hOldPen == 0 )
		hOldPen = (HPEN)(*newPen);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//
void
TMapDC::SetTextColor16 (int color)
{
	SetTextColor (GetColor16(color));
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//
void
TMapDC::SetBkColor16 (int color)
{
	SetBkColor (GetColor16(color));
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Exclude the current info. windows from current dc clipping region
void
TMapDC::SetupClipping()
{
	// Set clipping for client rect. of window
	TRect clientRect (0, 0, pEditor->ClientSize.cx, pEditor->ClientSize.cy);
	SelectClipRgn (TRegion (clientRect));

	if ( pEditor->InfoWinShown )
	{
		switch (pEditor->EditMode)
		{
		case OBJ_THINGS:
			ExcludeClipWindow (pEditor->pThingInfo);
			break;

		case OBJ_LINEDEFS:
			ExcludeClipWindow (pEditor->pLineDefInfo);
			ExcludeClipWindow (pEditor->pSideDef1Info);
			ExcludeClipWindow (pEditor->pSideDef2Info);
			break;

		case OBJ_VERTEXES:
			ExcludeClipWindow (pEditor->pVertexInfo);
			break;

		case OBJ_SECTORS:
			ExcludeClipWindow (pEditor->pSectorInfo);
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////
// TMapDC
// -------------
// Exclude a window from the DC clipping region
//
void
TMapDC::ExcludeClipWindow (TWindow *pWnd)
{
	TRect wRect;
	pWnd->GetWindowRect (wRect);

	TPoint TopLeft (wRect.left, wRect.top);
	TPoint BotRight (wRect.right, wRect.bottom);

	pEditor->ScreenToClient (TopLeft);
	pEditor->ScreenToClient (BotRight);

	ExcludeClipRect (TRect (TopLeft, BotRight));
}


/////////////////////////////////////////////////////////////////////
// TMapDC
// ------
//
void
TMapDC::SetupOrg()
{
#ifdef WINDOWS_SCALING
	// Set window and viewport scale and extension
	SetMapMode (MM_ANISOTROPIC);
	SetViewportOrg (TPoint(ScrCenterX, ScrCenterY));
	SetWindowOrg (TPoint(OrigX, OrigY)) ;
	if ( Scale > 1.0 )
	{
		// dc.SetWindowExt (TSize(ScrMaxX, ScrMaxY));
		// dc.SetViewportExt (TSize(ScrMaxX * MUL_SCALE, -ScrMaxY * MUL_SCALE));
		SetWindowExt (TSize(1, 1));
		SetViewportExt (TSize(MUL_SCALE, -MUL_SCALE));
	}
	else if (Scale <= 1.0 )
	{
		SetWindowExt (TSize(ScrMaxX * DIV_SCALE, ScrMaxY * DIV_SCALE));
		SetViewportExt (TSize(ScrMaxX, -ScrMaxY));
	}

	/*
	for (int x = MIN_MAP_X ; x <= MAX_MAP_X ; x+=1000)
		for (SHORT y = MIN_MAP_Y ; y <= MAX_MAP_Y ; y+=1000)
		{
			char msg[80];
			wsprintf (msg,"(%d:%d)", x/1000, y/1000);
			dc.TextOut (x, y, msg);
		}
	*/

#endif
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
// draw a line on the screen from map coords
void
TMapDC::DrawMapLine(SHORT mapXstart, SHORT mapYstart, SHORT mapXend, SHORT mapYend)
{
	// LogMessage ("DrawMapLine(%d,%d,%d,%d)\n", mapXstart, mapYstart, mapXend, mapYend);
#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(mapXstart, mapYstart, mapXend, mapYend) )
		return;
#endif

#ifdef WINDOWS_SCALING
	MoveTo (mapXstart, mapYstart) ;
	LineTo (mapXend, mapYend);
#else
	MoveTo (SCREENX(mapXstart), SCREENY(mapYstart)) ;
	LineTo (SCREENX(mapXend),   SCREENY(mapYend));
#endif

}


//////////////////////////////////////////////////////////
// TMapDC
// ------
// draw a rectangle on the screen from map coords
void
TMapDC::DrawMapRect(SHORT mapXstart, SHORT mapYstart, SHORT mapXend, SHORT mapYend)
{
	// LogMessage ("DrawMapRect(%d,%d,%d,%d)\n", mapXstart, mapYstart, mapXend, mapYend);
#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(mapXstart, mapYstart, mapXend, mapYend) )
		return;
#endif

#ifdef WINDOWS_SCALING
	Rectangle (mapXstart, mapYstart,
			   mapXend, mapYend);
#else
	Rectangle (SCREENX(mapXstart), SCREENY(mapYstart),
			   SCREENX(mapXend),   SCREENY(mapYend));
#endif
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
// draw a circle on the screen from map coords
void
TMapDC::DrawMapCircle(SHORT mapXcenter, SHORT mapYcenter, SHORT mapRadius)
{
	SHORT radius;
	SHORT x0 = mapXcenter - mapRadius;
	SHORT y0 = mapYcenter + mapRadius;
	SHORT x1 = mapXcenter + mapRadius;
	SHORT y1 = mapYcenter - mapRadius;

#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(x0,y0,x1,y1) )
		return;
#endif

	radius = mapRadius * ScaleNum / ScaleDen;

#ifndef WINDOWS_SCALING
	x0 = SCREENX(x0);
	y0 = SCREENY(y0);
	x1 = SCREENX(x1);
	y1 = SCREENY(y1);
#endif

	// LogMessage ("DrawMapCircle(%d,%d,%d,%d)\n", x0, y0, x1, y1);
	// SelectStockObject (NULL_BRUSH);

	// If radius is small, draw a rectangle instead of an ellipse
	if ( radius <= 3 )
	{
		TPoint points[5], *pp = points ;
#ifdef WINDOWS_SCALING
		pp->x = mapXcenter; 	pp->y = y0;				pp++;
		pp->x = x1; 			pp->y = mapYcenter;   	pp++;
		pp->x = mapXcenter;		pp->y = y1;            	pp++;
		pp->x = x0;				pp->y = mapYcenter;  	pp++;
		pp->x = mapXcenter; 	pp->y = y0;
#else
		pp->x = x0+radius; 	pp->y = y0;				pp++;
		pp->x = x1; 		pp->y = y0+radius;    	pp++;
		pp->x = x0+radius;	pp->y = y1;             pp++;
		pp->x = x0;			pp->y = y0+radius;  	pp++;
		pp->x = x0+radius; 	pp->y = y0;
#endif
		Polyline (points, 5);
	}
	else
	{
		Ellipse (x0, y0, x1+1, y1+1);
	}
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
// draw a vector (line with an ending arrow) on the screen
// from map coords
void
TMapDC::DrawMapVector(SHORT mapXstart, SHORT mapYstart, SHORT mapXend, SHORT mapYend)
{
#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(mapXstart, mapYstart, mapXend, mapYend) )
		return;
#endif

#ifdef WINDOWS_SCALING
	SHORT x0 = mapXstart;
	SHORT y0 = mapYstart;
	SHORT x1 = mapXend;
	SHORT y1 = mapYend;
#else
	SHORT x0 = SCREENX(mapXstart);
	SHORT y0 = SCREENY(mapYstart);
	SHORT x1 = SCREENX(mapXend);
	SHORT y1 = SCREENY(mapYend);
#endif
	SHORT dx = x0 - x1;
	SHORT dy = y0 - y1;

	// Draw the line part of the vector
	MoveTo (x0, y0);
	LineTo (x1, y1);

	// Don't draw arrow if Scale <= 1/6
	if ( (ScaleNum == 1) && (ScaleDen >= 6) )
		return;

	// Calc the length of the line
	SHORT r = (SHORT)hypot(dx, dy);
	// Don't calc if length == 0 (no orientation !)
	if ( r == 0 )
		return;

	// Draw the arrow part of the vector
	SHORT scrXoff = (SHORT) (dx * 8 * MUL_SCALE / r);
	SHORT scrYoff = (SHORT) (dy * 8 * MUL_SCALE / r);
	x0 = x1 + 2 * scrXoff;
	y0 = y1 + 2 * scrYoff;
	MoveTo (x0 - scrYoff, y0 + scrXoff);
	LineTo (x1          , y1);
	LineTo (x0 + scrYoff, y0 - scrXoff);
}



//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw an arrow on the screen from map coords and angle (0 - 65535)
//  TODO: Need to add clipping and scaling
void
TMapDC::DrawMapArrow(SHORT mapXstart, SHORT mapYstart, USHORT angle, USHORT len)
{
	// NOTE: 10430,...  = 65535 / (2 * PI)
	SHORT    mapXend   = mapXstart + (SHORT) (len * cos(angle / 10430.37835));
	SHORT    mapYend   = mapYstart + (SHORT) (len * sin(angle / 10430.37835));
	DrawMapVector (mapXstart, mapYstart, mapXend, mapYend);
}



//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a Thing
void TMapDC::DrawMapThing (int editmode, SHORT tnum)
{
	assert_tnum (tnum);
	Thing *pThing = &Things[tnum];

	SHORT xpos = pThing->xpos;
	SHORT ypos = pThing->ypos;
	SHORT scrRadius;
	SHORT mapRadius;

	// Select the radius
	if (editmode == OBJ_THINGS)
		mapRadius = GetKodObjectRadius(Things[tnum].type);
	else
		mapRadius = OBJSIZE;

	// Calc. bouding rectangle
	SHORT x0 = xpos - mapRadius;
	SHORT y0 = ypos + mapRadius;
	SHORT x1 = xpos + mapRadius;
	SHORT y1 = ypos - mapRadius;

#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(x0,y0,x1,y1) )
		return;
#endif

	// Convert map coords. to screen coords.
	scrRadius = (SHORT)(mapRadius * MUL_SCALE);

#ifndef WINDOWS_SCALING
	xpos = SCREENX(xpos);
	ypos = SCREENY(ypos);
	x0   = SCREENX(x0);
	y0   = SCREENY(y0);
	x1   = SCREENX(x1);
	y1   = SCREENY(y1);
#endif

	// Draw bouding circle of the thing
	if (editmode == OBJ_THINGS)
	{
		SetPenColor16 (GetKodObjectColor (Things[tnum].type));
		// If radius is small, draw a rectangle instead of an ellipse
		if ( scrRadius > 4 )
		{
			Ellipse (x0, y0, x1+1, y1+1);
		}
		else if ( scrRadius > 1 )
		{
			TPoint points[5], *pp = points ;
			pp->x = xpos; 		pp->y = y0;		pp++;
			pp->x = x1; 		pp->y = ypos;  	pp++;
			pp->x = xpos;		pp->y = y1;   	pp++;
			pp->x = x0;			pp->y = ypos;  	pp++;
			pp->x = xpos; 		pp->y = y0;
			Polyline (points, 5);
		}
	}
	// Set pen color to default (LIGHTGRAY)
	else
		SetPenColor16 (LIGHTGRAY);

	// Draw the cross
	MoveTo (xpos, y0) ;
	LineTo (xpos, y1) ;
	MoveTo (x0, ypos) ;
	LineTo (x1, ypos) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a LineDef
void TMapDC::DrawMapLineDef (int editmode, SHORT ldnum, BOOL DrawLen)
{
	assert_ldnum (ldnum);

	// Use interm. vars. to accelarate
	LineDef *pLD = &LineDefs[ldnum];
	SHORT start = pLD->start;
	SHORT end   = pLD->end;

	assert_vnum (start);
	assert_vnum (end);
	Vertex *pVStart = &Vertexes[start];
	Vertex *pVEnd   = &Vertexes[end];
	SHORT x0    = pVStart->x;
	SHORT y0    = pVStart->y;
	SHORT x1    = pVEnd->x;
	SHORT y1    = pVEnd->y;

#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(x0, y0, x1, y1) )
		return;
#endif

	// Set the LineDef color
	int color = GetLineDefColor(editmode, ldnum);
	SetPenColor16(color);

	// Draw the LineDef
	if ( editmode == OBJ_VERTEXES )
		DrawMapVector(x0, y0, x1, y1);
	else
		DrawMapLine(x0, y0, x1, y1);

	// Draw the Length
	if ( DrawLen )
		DrawLineDefLen(x0, y0, x1, y1, color);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a LineDef
void TMapDC::DrawMapLineDefLen (int editmode, SHORT ldnum)
{
	assert_ldnum (ldnum);

	// Use interm. vars. to accelarate
	LineDef *pLD = &LineDefs[ldnum];
	SHORT start = pLD->start;
	SHORT end   = pLD->end;

	assert_vnum (start);
	assert_vnum (end);
	Vertex *pVStart = &Vertexes[start];
	Vertex *pVEnd   = &Vertexes[end];
	SHORT x0    = pVStart->x;
	SHORT y0    = pVStart->y;
	SHORT x1    = pVEnd->x;
	SHORT y1    = pVEnd->y;

#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(x0,y0,x1,y1) )
		return;
#endif

    // Get the LineDef color
    int color = GetLineDefColor(editmode, ldnum);

	// Draw the Length
	DrawLineDefLen (x0, y0, x1, y1, color);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a LineDef
void TMapDC::DrawLineDefLen (SHORT x0, SHORT y0, SHORT x1, SHORT y1, int color)
{
	char Text[10];

	// How to display a Text using a XOR mode?
	//  1. Create a memory dc
	//  2. Calc the text size (GetTextExtent)
	//  3. Create a bitmap of this size
	//  4. Draw text in bitmap (TextOut) in the memory DC
	//  5. Draw bitmap (with text in it) on screen using XOR

	// Build the string of the line length
	sprintf (Text, "%d", ComputeDist (abs(x1 - x0), abs(y1 - y0)));

	// Get the text size
	TSize TextSize = GetTextExtent(Text, strlen(Text));

	// Calc. the text drawing rect
#ifdef WINDOWS_SCALING
	TRect TTextRect(TPoint ((x0 + x1) / 2, (y0 + y1) /2), TextSize));
#else
	TRect TTextRect(TPoint (SCREENX((x0 + x1) / 2), SCREENY((y0 + y1) /2)), TextSize);
#endif
	// Draw text in bitmap
	TMemoryDC MemDC;
	TBitmap TextBitmap (*this, TextSize.cx, TextSize.cy);
	MemDC.SelectObject (TextBitmap);
	MemDC.SetTextAlign (TA_LEFT);
	MemDC.SetBkMode (TRANSPARENT);
	MemDC.SetTextColor (GetColor16(BLUE));
	MemDC.SelectObject (TPen(GetColor16(color)));
	MemDC.SelectObject (TBrush(GetColor16(WHITE)));
	MemDC.Rectangle (TPoint(0, 0), TextSize);
	MemDC.TextOut (0, 0, Text);

	// Draw bitmap (with text in it) on screen using XOR
    // TTextRect.left -= TextSize.cx; // align to right
	BitBlt (TTextRect,
			MemDC,
			TPoint(0, 0),
			SRCINVERT);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  Return a LineDef color
int TMapDC::GetLineDefColor (int editmode, SHORT ldnum)
{
	assert_ldnum (ldnum);
	int color = 0;
	SHORT m;
	Sector *pSector;

	// Use interm. vars. to accelarate
	LineDef *pLD = &LineDefs[ldnum];

	// Select pen color
	switch (editmode)
	{
	case OBJ_THINGS:
	case OBJ_LINEDEFS:
	case OBJ_VERTEXES:
		if ((pLD->blak_flags & BF_POS_PASSABLE) && 
		    (pLD->blak_flags & BF_NEG_PASSABLE))
		   color = MAGENTA;
		else color = LIGHTGRAY;
		break;

	case OBJ_SECTORS:
		if ((m = pLD->sidedef1)      < 0 ||
			(m = SideDefs[m].sector) < 0 )
		{
			color = LIGHTRED;
		}
		else
		{
			pSector = &Sectors[m];

			if      (pSector->tag > 0)			color = LIGHTGREEN;
			else if (pSector->special > 0)   	color = LIGHTCYAN;
			else if (pLD->flags & 1)    		color = WHITE;
			else                                color = LIGHTGRAY;

			if ((m = pLD->sidedef2) >= 0)
			{
				m = SideDefs[m].sector;
				if (m < 0)						color = LIGHTRED;
				else
				{
					pSector = &Sectors[m];
					if (pSector->tag > 0)       	color = LIGHTGREEN;
					else if (pSector->special > 0)   color = LIGHTCYAN;
				}
			}
		}
		break;
	}

	return color;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a Vertex
void TMapDC::DrawMapVertex (int editmode, SHORT vnum)
{
	assert_vnum (vnum);
	if ( editmode != OBJ_VERTEXES )
		return;

	Vertex ve = Vertexes[vnum];
	SHORT x0 = ve.x - OBJSIZE;
	SHORT y0 = ve.y - OBJSIZE;
	SHORT x1 = ve.x + OBJSIZE;
	SHORT y1 = ve.y + OBJSIZE;

#ifndef WINDOWS_CLIPPING
	if ( ! IS_VISIBLE_MAP_RECT(x0, y0, x1, y1) )
		return;
#endif

	// Draw a point if cross is small
	SetPenColor16 (LIGHTGREEN);
	// if ( ((x1 - x0) * MUL_SCALE) <= 3 )
	// Draw point if Scale <= 1/6
	if ( (ScaleNum == 1) && (ScaleDen >= 6) )
	{
#ifdef WINDOWS_SCALING
		SHORT x = ve.x;
		SHORT y = ve.y;
#else
		SHORT x = SCREENX(ve.x);
		SHORT y = SCREENY(ve.y);
#endif
		MoveTo (x, y);
		LineTo (x + 1, y);

		// SetPixel (SCREENX(ve.x), SCREENY(ve.y), GetColor16(LIGHTGREEN));
		// SetPixel (ve.x, ve.y, GetColor16(LIGHTGREEN));
		// ::SetPixel (*this, SCREENX(ve.x), SCREENY(ve.y), Color16Tab[LIGHTGREEN]);
	}
	else
	{
#ifndef WINDOWS_SCALING
		x0 = SCREENX(x0);
		y0 = SCREENY(y0);
		x1 = SCREENX(x1);
		y1 = SCREENY(y1);
#endif
		MoveTo(x0, y0);
		LineTo(x1, y1);
		MoveTo(x1, y0);
		LineTo(x0, y1);
	}
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw (the LineDefs of) a sector
void TMapDC::DrawMapSector (int editmode, SHORT snum)
{
	assert_snum (snum);

	SHORT n;
	LineDef *pLD;
	for (n = 0, pLD = LineDefs;
		 n < NumLineDefs ;
		 n++, pLD++)
	{
		SHORT s1 = pLD->sidedef1;
		SHORT s2 = pLD->sidedef2;

		if ( (s1 >= 0 && SideDefs[s1].sector == snum) ||
			 (s2 >= 0 && SideDefs[s2].sector == snum) )
		{
			DrawMapLineDef (editmode, n);
		}
	}
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a list of Things
void TMapDC::DrawMapThingsList (int editmode, SelPtr list)
{
	for (SelPtr cur = list ; cur != NULL ; cur = cur->next)
		DrawMapThing (editmode, cur->objnum);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a list of LineDefs
void TMapDC::DrawMapLineDefsList (int editmode, SelPtr list, BOOL DrawLen)
{
	for (SelPtr cur = list ; cur != NULL ; cur = cur->next)
		DrawMapLineDef (editmode, cur->objnum, DrawLen);
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a list of Vertexes
void TMapDC::DrawMapVertexesList (int editmode, SelPtr list)
{
	if ( editmode == OBJ_VERTEXES )
		for (SelPtr cur = list ; cur != NULL ; cur = cur->next)
			DrawMapVertex (editmode, cur->objnum) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a list of Sectors
void TMapDC::DrawMapSectorsList (int editmode, SelPtr list)
{
	for (SelPtr cur = list ; cur != NULL ; cur = cur->next)
		DrawMapSector (editmode, cur->objnum) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a all Things
void TMapDC::DrawMapThings (int editmode)
{
	for (SHORT n = 0; n < NumThings; n++)
		DrawMapThing (editmode, n) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a all LineDefs
void TMapDC::DrawMapLineDefs (int editmode, BOOL DrawLen)
{
	for (SHORT n = 0; n < NumLineDefs; n++)
		DrawMapLineDef (editmode, n, DrawLen) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a all Vertexes
void TMapDC::DrawMapVertexes (int editmode)
{
	if ( editmode == OBJ_VERTEXES )
		for (SHORT n = 0; n < NumVertexes; n++)
			DrawMapVertex (editmode, n) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw a all Sectors
void TMapDC::DrawMapSectors (int editmode)
{
	for (SHORT n = 0; n < NumLineDefs; n++)
		DrawMapLineDef (editmode, n) ;
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  draw grid
void TMapDC::DrawMapGrid (SHORT grid)
{
	if ( grid > 0 )
	{
		SHORT n;
		SHORT mapx0 = (MAPX(0)       - grid) & ~(grid - 1);
		SHORT mapx1 = (MAPX(ScrMaxX) + grid) & ~(grid - 1);
		SHORT mapy0 = (MAPY(ScrMaxY) - grid) & ~(grid - 1);
		SHORT mapy1 = (MAPY(0)       + grid) & ~(grid - 1);

		SetPenColor16 (BLUE);
		for (n = mapx0; n <= mapx1; n += grid)
			DrawMapLine (n, mapy0, n, mapy1);
		for (n = mapy0; n <= mapy1; n += grid)
			DrawMapLine (mapx0, n, mapx1, n);
	}
}


//////////////////////////////////////////////////////////
// TMapDC
// ------
//  Draw the level map centered on (OrigX, OrigY), according to
//  editing mode.
//
void TMapDC::DrawMap (int editmode, SHORT grid, BOOL drawgrid)
{
	//
	// draw the grid
	//
	if ( drawgrid == TRUE )
		DrawMapGrid (grid);

	/* draw the linedefs to form the map */
	switch (editmode)
	{
		case OBJ_THINGS:
			DrawMapLineDefs (editmode);
			break;

		case OBJ_LINEDEFS:
			DrawMapLineDefs (editmode);
			break;

		case OBJ_VERTEXES:
			DrawMapLineDefs (editmode);
			DrawMapVertexes (editmode);
			break;

		case OBJ_SECTORS:
			DrawMapSectors (editmode);
			break;
	}

	// draw in the things
	DrawMapThings (editmode);
}

