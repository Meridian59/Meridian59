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

	FILE:         gfx.cpp

	OVERVIEW
	========
	Source file for graphical mathematics functions.

*/

#include "common.h"
#pragma hdrstop

#ifndef __gfx_h
	#include "gfx.h"
#endif

#ifndef __objects_h
	#include "objects.h"	// InsertObject()
#endif

#ifndef OWL_COLOR_H
	#include <owl\color.h>
#endif

#include <dos.h>

/* the global variables */
SHORT OrigX;			/* the X origin (map coord) */
SHORT OrigY;			/* the Y origin (map coord) */
float Scale;		/* the scale value */
float ScaleMin;		/* the minimum scale value */
float ScaleMax;		/* the maximum scale value */
SHORT ScaleNum;       /* the numerator of Scale */
SHORT ScaleDen;       /* the denominator of Scale */
SHORT PointerX;		/* X position of pointer (screen coord) */
SHORT PointerY;		/* Y position of pointer (screen coord) */
SHORT ScrMaxX;		/* maximum X screen coord */
SHORT ScrMaxY;		/* maximum Y screen coord */
SHORT ScrCenterX;		/* X coord of screen center */
SHORT ScrCenterY;		/* Y coord of screen center */
SHORT VisMapMinX;		/* Minimum visible X map coord */
SHORT VisMapMaxX;		/* Maximum visible X map coord */
SHORT VisMapMinY;		/* Minimum visible Y map coord */
SHORT VisMapMaxY;		/* Maximum visible Y map coord */


/*
	Adjust ScaleNum and ScaleDen from Scale
*/
void AdjustScale ()
{
	// Maximum scale
	if ( Scale > ScaleMax )
		Scale = ScaleMax;

	// Increment Scale until >= ScaleMin
	while ( Scale < ScaleMin )
	{
		assert (Scale < 1.0);

		Scale = 1.0f / ((1.0f / Scale) - 1.0f);
	}

	// Set ScaleNum and ScaleDen
	if ( Scale >= 1.0 )
	{
		ScaleNum = (SHORT)Scale;
		ScaleDen = 1;
	}
	else
	{
		ScaleNum = 1;
		ScaleDen = (SHORT)(1.0/Scale + 0.5);
	}

#if 0
	// Adjust Visible map limits
	VisMapMinX = MAPX(0);
	VisMapMaxX = MAPX(ScrMaxX);
	VisMapMinY = MAPY(ScrMaxY);		// Vertical axe is reversed!
	VisMapMaxY = MAPY(0); 			// Vertical axe is reversed!
#endif
}


/*
	Calc minimum Scale according to current screen size
*/
void SetScaleMin ()
{
	float MinScaleX = (float)ScrMaxX / (float)MAP_X_SIZE;
	float MinScaleY = (float)ScrMaxY / (float)MAP_Y_SIZE;

	// Minimum scale is maximum of x and y minimum scales
	ScaleMin = MinScaleX > MinScaleY ? MinScaleX : MinScaleY;
	AdjustScale ();
}


/*
	Set the Scale and adjust ScaleNum and ScaleDen
*/
void SetScale (float _Scale)
{
	Scale = _Scale;
	SetScaleMin();
}


/*
	Decrement Scale and adjust ScaleNum and ScaleDen
*/
void DecScale()
{
	if (Scale <= 1.0)
		Scale = 1.0f / ((1.0f / Scale) + 1.0f);
	else
		Scale = Scale - 1.0f;

	// Check scale is not too small
	AdjustScale();
}

/*
	Increment Scale and adjust ScaleNum and ScaleDen
*/
void IncScale()
{
	if ( Scale < 1.0)
		Scale = 1.0f / ((1.0f / Scale) - 1.0f);
	else
		Scale = Scale + 1.0f;

	// Check Scale is not too big
	AdjustScale();
}


/*
   translate (dx, dy) into an integer angle value (0-65535)
*/

USHORT ComputeAngle (SHORT dx, SHORT dy)
{
   return (USHORT) (atan2( (double) dy, (double) dx) * 10430.37835 + 0.5);
   /* Yes, I know this function could be in another file, but */
   /* this is the only source file that includes <math.h>...  */
}



/*
   compute the distance from (0, 0) to (dx, dy)
*/

USHORT ComputeDist (SHORT dx, SHORT dy)
{
   return (USHORT) round((hypot( (double) dx, (double) dy)));
   /* Yes, I know this function could be in another file, but */
   /* this is the only source file that includes <math.h>...  */
}

/*
   compute the distance from (0, 0) to (dx, dy)
*/

double ComputeDistDouble(double dx, double dy)
{
   return hypot(dx, dy);
}


/*
   insert the vertices of a new polygon
*/

void InsertPolygonVertices (SHORT centerx, SHORT centery, SHORT sides, SHORT radius)
{
   double theta = PIX2 / (double)sides;
   double tangetial_factor = tan(theta);
   double radial_factor = cos(theta);
   double x = radius;
   double y = 0;

   for (SHORT n = 0; n < sides; n++)
   {
      InsertObject(OBJ_VERTEXES, -1, centerx + (SHORT)round(x), centery + (SHORT)round(y));
      double tx = -y;
      double ty = x;
      x += tx * tangetial_factor;
      y += ty * tangetial_factor;
      x *= radial_factor;
      y *= radial_factor;
   }
}

/*
   move (x, y) to a new position: rotate and scale around (0, 0)
*/

void RotateAndScaleCoords (SHORT *x, SHORT *y, double angle, double scale)
{
   double r, theta;

   r = hypot( (double) *x, (double) *y);
   theta = atan2( (double) *y, (double) *x);
   *x = (SHORT) round(r * scale * cos( theta + angle));
   *y = (SHORT) round(r * scale * sin( theta + angle));
   /* Yes, I know... etc. */
}

/* end of file */
