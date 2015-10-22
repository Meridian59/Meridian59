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

	FILE:         gfx.h
*/
#ifndef __gfx_h
#define __gfx_h

#ifndef __common_h
	#include "common.h"
#endif

#include <math.h>

/* Defines the extremal maps dimensions. Recommended values are
   -10000 to +10000 for both X and Y. It's dangerous to choose
   greater values. These values are used for setting the scroll bars
   range of the editor window. */
#if 0
// Experimenting ARK
#define MAP_MIN_X   0
#define MAP_MAX_X    14000
#define MAP_X_SIZE  (MAP_MAX_X - MAP_MIN_X)
#define MAP_MIN_Y   0
#define MAP_MAX_Y    14000
#define MAP_Y_SIZE  (MAP_MAX_Y - MAP_MIN_Y)
#endif
#if 1
#define MAP_MIN_X   -14000
#define MAP_MAX_X    14000
#define MAP_X_SIZE  (MAP_MAX_X - MAP_MIN_X)
#define MAP_MIN_Y   -14000
#define MAP_MAX_Y    14000
#define MAP_Y_SIZE  (MAP_MAX_Y - MAP_MIN_Y)
#define PIX2 6.283185307179586476925286766559
#endif

/*
   Two macros used to replace the use of the Scale var. for
   integer calculations.
   EX: mapRadius = scrRadius * DIV_SCALE
	   scrRadius = mapRadius * MUL_SCALE
*/
#define DIV_SCALE   (long)ScaleDen / ScaleNum
#define MUL_SCALE   (long)ScaleNum / ScaleDen

/* convert screen coordinates to map coordinates */
#define MAPX(x)         (OrigX + (SHORT)(((x) - ScrCenterX) * (LONG)DIV_SCALE))
#define MAPY(y)         (OrigY + (SHORT)((ScrCenterY - (y)) * (LONG)DIV_SCALE))

/* convert map coordinates to screen coordinates */
#define SCREENX(x)      (ScrCenterX + (SHORT)(((x) - OrigX) * (LONG)MUL_SCALE))
#define SCREENY(y)      (ScrCenterY + (SHORT)((OrigY - (y)) * (LONG)MUL_SCALE))


/* half the size of an object (Thing or Vertex) in map coords */
#define OBJSIZE         7


/*

	Colors used in the edtior

*/
#define BLACK       0
#define BLUE        1
#define GREEN       2
#define CYAN        3
#define RED         4
#define MAGENTA     5
#define BROWN       6
#define LIGHTGRAY   7
#define DARKGRAY    8
#define LIGHTBLUE   9
#define LIGHTGREEN  10
#define LIGHTCYAN   11
#define LIGHTRED    12
#define LIGHTMAGENTA    13
#define YELLOW      14
#define WHITE       15



/* from gfx.cpp */
extern float Scale;     /* scale to draw map */
extern float ScaleMin;  /* the minimum scale value */
extern float ScaleMax;  /* the maximum scale value */
extern SHORT ScaleNum;  /* Numerator of Scale */
extern SHORT ScaleDen;  /* Denominator of Scale */
extern SHORT OrigX;     /* the X map origin */
extern SHORT OrigY;     /* the Y map origin */
extern SHORT PointerX;      /* X position of pointer */
extern SHORT PointerY;      /* Y position of pointer */
extern SHORT ScrMaxX;       /* maximum X screen coord */
extern SHORT ScrMaxY;       /* maximum Y screen coord */
extern SHORT ScrCenterX;    /* X coord of screen center */
extern SHORT ScrCenterY;    /* Y coord of screen center */
extern SHORT VisMapMinX;        /* Minimum visible X map coord */
extern SHORT VisMapMaxX;        /* Maximum visible X map coord */
extern SHORT VisMapMinY;        /* Minimum visible Y map coord */
extern SHORT VisMapMaxY;        /* Maximum visible Y map coord */


/* from gfx.cpp */
void SetScale (float);
void IncScale ();
void DecScale ();
void AdjustScale ();
void SetScaleMin ();

USHORT ComputeAngle (SHORT, SHORT);
USHORT ComputeDist (SHORT, SHORT);
double ComputeDistDouble (double, double);
void InsertPolygonVertices (SHORT, SHORT, SHORT, SHORT);
void RotateAndScaleCoords (SHORT *, SHORT *, double, double);


#endif
