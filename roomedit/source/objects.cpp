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

 	FILE:         objects.cpp

	OVERVIEW
	========
	Source file for object handling routines.
*/

#include "common.h"
#pragma hdrstop

#ifndef OWL_INPUTDIA_H
	#include <owl\inputdia.h>
#endif

#ifndef __objects_h
	#include "objects.h"
#endif

#ifndef __levels_h
	#include "levels.h"	// GetWallTextureSize
#endif

#ifndef __mapdc_h
	#include "mapdc.h"
#endif

#ifndef __workdlg_h
	#include "workdlg.h"
#endif

#include "bsp.h"
//#include "entrance.h"

/*
   highlight the selected objects
*/

void HighlightSelection (TMapDC &dc, int objtype, SelPtr list)
{
	for (SelPtr cur = list; cur; cur = cur->next)
		HighlightObject (dc, objtype, cur->objnum, GREEN);
}



/*
   test if an object is in the selection list
*/

BOOL IsSelected (SelPtr list, SHORT objnum)
{
	for (SelPtr cur = list; cur; cur = cur->next)
		if (cur->objnum == objnum)
			return TRUE;
	return FALSE;
}



/*
   add an object to the selection list
*/

void SelectObject (SelPtr *list, SHORT objnum)
{
	assert (objnum >= 0);

	SelPtr cur;
	cur = (SelPtr) GetMemory (sizeof(SelectionList));
	cur->next = *list;
	cur->objnum = objnum;
	*list = cur;
}



/*
   remove an object from the selection list
*/

void UnSelectObject (SelPtr *list, SHORT objnum)
{
   // XXX Assertion kept going off for some reason; hopefully this is ok ARK
   if (objnum < 0)
      return;
//	assert (objnum >= 0);

	SelPtr cur, prev;
	prev = NULL;
	cur = *list;
	while (cur)
	{
		if (cur->objnum == objnum)
		{
			if (prev)
				prev->next = cur->next;
			else
				*list = cur->next;
			FreeMemory (cur);
			if (prev)
				cur = prev->next;
			else
				cur = NULL;
		}
		else
		{
			prev = cur;
			cur = cur->next;
		}
	}
}



/*
   forget the selection list
*/

void ForgetSelection (SelPtr *list)
{
	SelPtr cur, prev;

	cur = *list;
	while (cur)
	{
		prev = cur;
		cur = cur->next;
		FreeMemory (prev);
	}
	*list = NULL;
}



/*
   get the number of objets of a given type minus one
*/
SHORT GetMaxObjectNum (int objtype)
{
	switch (objtype)
	{
	case OBJ_THINGS:
		return NumThings - 1;
	case OBJ_LINEDEFS:
		return NumLineDefs - 1;
	case OBJ_SIDEDEFS:
		return NumSideDefs - 1;
	case OBJ_VERTEXES:
		return NumVertexes - 1;
	case OBJ_SEGS:
		return NumSegs - 1;
	case OBJ_SSECTORS:
		return NumSSectors - 1;
	case OBJ_SECTORS:
		return NumSectors - 1;
	}
	return -1;
}


/*
   check if there is something of interest inside the given box
*/

SHORT GetCurObject (int objtype, SHORT x0, SHORT y0, SHORT x1, SHORT y1)
{
	SHORT n, m, cur, curx;
	SHORT lx0, ly0, lx1, ly1;
	SHORT midx, midy;
	ULONG dist, r, mindist;
	long  x, y;
	SHORT xc, yc;

	cur = -1;
	if (x1 < x0)
	{
		n = x0;		x0 = x1;	x1 = n;
	}
	if (y1 < y0)
	{
		n = y0;	  	y0 = y1;  	y1 = n;
	}

	switch (objtype)
	{
	case OBJ_THINGS:
		mindist = ULONG_MAX;
		xc = (x0 + x1) / 2;
		yc = (y0 + y1) / 2;
		for (n = 0; n < NumThings; n++)
		{
			x = xc - Things[ n].xpos;
			y = yc - Things[ n].ypos;
			r = GetThingRadius (Things[ n].type) + 2;
			// Keep a reasonnable radius size
			if ( r * MUL_SCALE < 4 )		r = 4 * DIV_SCALE;
			dist = x * x + y * y;
			r = r * r;
			if ( dist <= r && dist <= mindist)
			{
				mindist = dist;
				cur = n;
				// break;
			}
		}
		break;

	case OBJ_VERTEXES:
		for (n = 0; n < NumVertexes; n++)
			if (Vertexes[ n].x >= x0 && Vertexes[ n].x <= x1 &&
				Vertexes[ n].y >= y0 && Vertexes[ n].y <= y1)
			{
				cur = n;
				break;
			}
		break;

	case OBJ_LINEDEFS:
		for (n = 0; n < NumLineDefs; n++)
		{
			if (IsLineDefInside( n, x0, y0, x1, y1))
			{
				cur = n;
				break;
			}
		}
		break;

	case OBJ_SECTORS:
		/* hack, hack...  I look for the first LineDef crossing an
		   horizontal half-line drawn from the cursor */
		curx = MapMaxX + 1;
		cur = -1;
		midx = (x0 + x1) / 2;
		midy = (y0 + y1) / 2;
		for (n = 0; n < NumLineDefs; n++)
		{
			if ((Vertexes[ LineDefs[ n].start].y > midy) !=
				(Vertexes[ LineDefs[ n].end  ].y > midy))
			{
				lx0 = Vertexes[ LineDefs[ n].start].x;
				ly0 = Vertexes[ LineDefs[ n].start].y;
				lx1 = Vertexes[ LineDefs[ n].end].x;
				ly1 = Vertexes[ LineDefs[ n].end].y;
				m = lx0 + (SHORT) ((long) (midy - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
				if (m >= midx && m < curx)
				{
					curx = m;
					cur = n;
				}
			}
		}

		/* now look if this LineDef has a SideDef bound to one sector */
		if (cur >= 0)
		{
			if (Vertexes[ LineDefs[ cur].start].y > Vertexes[ LineDefs[ cur].end].y)
				cur = LineDefs[ cur].sidedef1;
			else
				cur = LineDefs[ cur].sidedef2;
			if (cur >= 0)
			{
				cur = SideDefs[ cur].sector;
			}
			else
				cur = -1;
		}
		else
			cur = -1;
		break;
	}
	return cur;
}



/*
   select all objects inside a given box
*/

SelPtr SelectObjectsInBox (int objtype, SHORT x0, SHORT y0, SHORT x1, SHORT y1)
{
	SHORT n, m;
	SelPtr list;

	list = NULL;
	if (x1 < x0)
	{
		n = x0;		x0 = x1;	x1 = n;
	}
	if (y1 < y0)
	{
		n = y0;     y0 = y1;    y1 = n;
	}

	switch (objtype)
	{
	case OBJ_THINGS:
		for (n = 0; n < NumThings; n++)
		{
			SHORT xpos = Things[ n].xpos;
			SHORT ypos = Things[ n].ypos;
			if (xpos >= x0 && xpos <= x1 && ypos >= y0 && ypos <= y1)
			{
				SelectObject (&list, n);
			}
		}
		break;

	case OBJ_VERTEXES:
		for (n = 0; n < NumVertexes; n++)
		{
			SHORT x = Vertexes[ n].x;
			SHORT y = Vertexes[ n].y;
			if (x >= x0 && x <= x1 && y >= y0 && y <= y1)
			{
				SelectObject( &list, n);
			}
		}
		break;

	case OBJ_LINEDEFS:
		for (n = 0; n < NumLineDefs; n++)
		{
			SHORT x, y;
			/* the two ends of the line must be in the box */
			m = LineDefs[n].start;
			assert (m >= 0 && m < NumVertexes);
			x = Vertexes[ m].x;
			y = Vertexes[ m].y;
			if (x < x0  ||  x > x1  ||  y < y0  ||  y > y1)
			{
				continue;
			}
			m = LineDefs[n].end;
			assert (m >= 0 && m < NumVertexes);
			x = Vertexes[ m].x;
			y = Vertexes[ m].y;
			if ( x < x0  ||  x > x1  ||  y < y0  ||  y > y1)
			{
				continue;
			}
			SelectObject (&list, n);
		}
		break;

	case OBJ_SECTORS:
		/* hack: select all sectors... */
		for (n = 0; n < NumSectors; n++)
			SelectObject( &list, n);
		/* ... then remove the unwanted ones from the list */
		for (n = 0; n < NumLineDefs; n++)
		{
			SHORT x, y;
			m = LineDefs[ n].start;
			assert (m >= 0 && m < NumVertexes);
			x = Vertexes[ m].x;
			y = Vertexes[ m].y;
			if ( x < x0  ||  x > x1  ||  y < y0  ||  y > y1 )
			{
				// Unselect both SideDef if they exist
				m = LineDefs[ n].sidedef1;
				assert (m < NumSideDefs);
				if (m >= 0 && SideDefs[ m].sector >= 0)
					UnSelectObject( &list, SideDefs[ m].sector);
				m = LineDefs[ n].sidedef2;
				assert (m < NumSideDefs);
				if (m >= 0 && SideDefs[ m].sector >= 0)
					UnSelectObject( &list, SideDefs[ m].sector);
				continue;
			}

			m = LineDefs[ n].end;
			assert (m >= 0 && m < NumVertexes);
			x = Vertexes[ m].x;
			y = Vertexes[ m].y;
			if ( x < x0  ||  x > x1  ||  y < y0  ||  y > y1 )
			{
				// Unselect both SideDef if they exist
				m = LineDefs[ n].sidedef1;
				assert (m < NumSideDefs);
				if (m >= 0 && SideDefs[ m].sector >= 0)
					UnSelectObject( &list, SideDefs[ m].sector);
				m = LineDefs[ n].sidedef2;
				assert (m < NumSideDefs);
				if (m >= 0 && SideDefs[ m].sector >= 0)
					UnSelectObject( &list, SideDefs[ m].sector);
				continue;
			}
		}
		break;
	}

   return list;
}



/*
   highlight the selected object
*/

void HighlightObject(TMapDC &dc, int objtype, SHORT objnum, int color)
{
	SHORT n, m;
	SHORT x, y;
	SHORT x1, y1;
	SHORT start, end;

	if ( objnum == -1 )
		return;
		
	// use XOR mode : drawing any line twice erases it
	int OldROP2 = dc.SetROP2 (R2_XORPEN);

	switch ( objtype)
	{
	case OBJ_THINGS:
		assert (objnum >= 0 && objnum < NumThings);

		m = (GetKodObjectRadius (Things[objnum].type) * 3) / 2;
		x = Things[objnum].xpos;
		y = Things[objnum].ypos;

		dc.SetPenColor16(color);
		dc.DrawMapRect (x - m, y - m, x + m, y + m);
		dc.DrawMapArrow (x, y, Things[objnum].angle * 182, m * 10 / 8);
		break;

	case OBJ_LINEDEFS:
		assert (objnum >= 0 && objnum < NumLineDefs);

		start = LineDefs[ objnum].start;
		end   = LineDefs[ objnum].end;
		assert (start >= 0 && start < NumVertexes);
		assert (end   >= 0 && end   < NumVertexes);

		x = Vertexes[start].x;
		y = Vertexes[start].y;
		x1 = Vertexes[end].x;
		y1 = Vertexes[end].y;

		dc.SetPenColor16 (color);
		n = (x + x1) / 2;
		m = (y + y1) / 2;
		dc.DrawMapLine (n, m, n + (y1 - y) / 3, m + (x - x1) / 3);

		// Draw the vector
		// Set pen width to 3 (screen size, not map size!)
		dc.SetPenColor16 (color, 3);
		dc.DrawMapVector(x, y, x1, y1);

		// Draw sector attached (widh same tag number) to this LineDef
		if (color != LIGHTRED && LineDefs[ objnum].tag > 0)
		{
			for (m = 0; m < NumSectors; m++)
				if (Sectors[ m].tag == LineDefs[ objnum].tag)
					HighlightObject(dc, OBJ_SECTORS, m, LIGHTRED);
		}
		break;

        case OBJ_VERTEXES:
		// XXX Tends to go off after vertices merged ARK
#if 1		
		if (objnum < 0 || objnum >= NumVertexes)
		{
		   dprintf("objectso.cpp bjnum = %d, NumV = %d\n", objnum, NumVertexes);
		   break;
		}
#else
		assert (objnum >= 0 && objnum < NumVertexes);
#endif
		x = Vertexes[ objnum].x;
		y = Vertexes[ objnum].y;

		dc.SetPenColor16 (color);
		dc.DrawMapRect (x - OBJSIZE * 2, y - OBJSIZE * 2,
						x + OBJSIZE * 2, y + OBJSIZE * 2);
		break;

	case OBJ_SECTORS:
		assert (objnum >= 0 && objnum < NumSectors);

		// Set pen width to 3 (screen size, not map size!)
		dc.SetPenColor16 (color, 3);

		// Draw all LineDefs of this sector
		for (n = 0; n < NumLineDefs; n++)
		{
			SHORT sd1 = LineDefs[ n].sidedef1;
			SHORT sd2 = LineDefs[ n].sidedef2;

			if ( (sd1 >= 0 && SideDefs[sd1].sector == objnum) ||
				 (sd2 >= 0 && SideDefs[sd2].sector == objnum) )
			{
				start = LineDefs[n].start;
				end   = LineDefs[n].end;
				x     = Vertexes[start].x;
				y     = Vertexes[start].y;
				x1    = Vertexes[end].x;
				y1    = Vertexes[end].y;

				dc.DrawMapLine (x, y, x1, y1);
			}
		}

		// Draw LineDef attached (with same tag number) to this sector
		if (color != LIGHTRED && Sectors[ objnum].tag > 0)
		{
			for (m = 0; m < NumLineDefs; m++)
				if (LineDefs[ m].tag == Sectors[ objnum].tag)
					HighlightObject(dc, OBJ_LINEDEFS, m, LIGHTRED);
		}
		break;
	}

	dc.SetROP2 (OldROP2);
}



/*
   delete an object
*/

void DeleteObject (int objtype, SHORT objnum)
{
	SelPtr list;

	list = NULL;
	SelectObject (&list, objnum);
	DeleteObjects (objtype, &list);
}



/*
   delete a group of objects (*recursive*)
*/

void DeleteObjects (int objtype, SelPtr *list)
{
	SHORT n, objnum, i;
	SelPtr cur;

	MadeChanges = TRUE;
	switch (objtype)
	{
	case OBJ_THINGS:
		while (*list)
		{
			objnum = (*list)->objnum;
			if (kodEntrance == Things[objnum].type)
			{
//				DeleteEntrance(Things[objnum].when,Things[objnum].id);
			}
			/* delete the Thing */
			NumThings--;
			if (NumThings > 0)
			{
				for (n = objnum; n < NumThings; n++)
					Things[ n] = Things[ n + 1];
				Things = (TPtr)ResizeMemory (Things, NumThings * sizeof(Thing));
			}
			else
			{
				FreeMemory (Things);
				Things = NULL;
			}

			// Update objnum of selection list
			for (cur = (*list)->next; cur; cur = cur->next)
				if (cur->objnum > objnum)
					cur->objnum--;
			UnSelectObject( list, objnum);
		}
		break;

	case OBJ_VERTEXES:
		while (*list)
		{
			objnum = (*list)->objnum;
			// delete the LineDefs bound to this Vertex and
			// change the references
			for (n = 0; n < NumLineDefs; n++)
			{
				if (LineDefs[ n].start == objnum || LineDefs[ n].end == objnum)
					DeleteObject( OBJ_LINEDEFS, n--);
				else
				{
					if (LineDefs[ n].start >= objnum)
						LineDefs[ n].start--;
					if (LineDefs[ n].end >= objnum)
						LineDefs[ n].end--;

				}
			}

			// Change references in sectors for slopes

			for (n = 0; n < NumSectors; n++)
			   for (i=0; i < 3; i++)
			   {
			      // If the slope uses this vertex, remove slope. 
			      // Otherwise, just renumber if necessary
			      if (Sectors[n].blak_flags & SF_SLOPED_FLOOR)
			      {
				 if (Sectors[n].floor_slope.points[i].vertex == objnum)
				    Sectors[n].blak_flags &= ~SF_SLOPED_FLOOR;
				 if (Sectors[n].floor_slope.points[i].vertex > objnum)
				    Sectors[n].floor_slope.points[i].vertex--;
			      }
			      if (Sectors[n].blak_flags & SF_SLOPED_CEILING)
			      {
				 if (Sectors[n].ceiling_slope.points[i].vertex == objnum)
				    Sectors[n].blak_flags &= ~SF_SLOPED_CEILING;
				 if (Sectors[n].ceiling_slope.points[i].vertex > objnum)
				    Sectors[n].ceiling_slope.points[i].vertex--;
			      }
			   }
			
			// delete the Vertex
			NumVertexes--;
			if (NumVertexes > 0)
			{
				for (n = objnum; n < NumVertexes; n++)
					Vertexes[ n] = Vertexes[ n + 1];
				Vertexes = (VPtr)ResizeMemory (Vertexes, NumVertexes * sizeof(Vertex));
			}
			else
			{
				FreeMemory( Vertexes);
				Vertexes = NULL;
			}

			// Update objnum of selection list
			for (cur = (*list)->next; cur; cur = cur->next)
				if (cur->objnum > objnum)
					cur->objnum--;
			UnSelectObject( list, objnum);
		}
		break;

	case OBJ_LINEDEFS:
		while (*list)
		{
			objnum = (*list)->objnum;
			// delete the two SideDefs bound to this LineDef
			if (LineDefs[ objnum].sidedef1 >= 0)
				DeleteObject( OBJ_SIDEDEFS, LineDefs[ objnum].sidedef1);

			if (LineDefs[ objnum].sidedef2 >= 0)
				DeleteObject( OBJ_SIDEDEFS, LineDefs[ objnum].sidedef2);

			// delete the LineDef
			NumLineDefs--;
			if (NumLineDefs > 0)
			{
				for (n = objnum; n < NumLineDefs; n++)
					LineDefs[ n] = LineDefs[ n + 1];
				LineDefs = (LDPtr)ResizeMemory (LineDefs, NumLineDefs * sizeof(LineDef));
			}
			else
			{
				FreeMemory( LineDefs);
				LineDefs = NULL;
			}

			// Update objnum of selection list
			for (cur = (*list)->next; cur; cur = cur->next)
				if (cur->objnum > objnum)
					cur->objnum--;
			UnSelectObject( list, objnum);
		}
		break;

   case OBJ_SIDEDEFS:
		while (*list)
		{
			objnum = (*list)->objnum;
			// change the LineDefs references
			for (n = 0; n < NumLineDefs; n++)
			{
				if (LineDefs[ n].sidedef1 == objnum)
					LineDefs[ n].sidedef1 = -1;
				else if (LineDefs[ n].sidedef1 >= objnum)
					LineDefs[ n].sidedef1--;

				if (LineDefs[ n].sidedef2 == objnum)
					LineDefs[ n].sidedef2 = -1;
				else if (LineDefs[ n].sidedef2 >= objnum)
					LineDefs[ n].sidedef2--;
			}

			// delete the SideDef
			NumSideDefs--;
			if (NumSideDefs > 0)
			{
				for (n = objnum; n < NumSideDefs; n++)
					SideDefs[ n] = SideDefs[ n + 1];
				SideDefs = (SDPtr)ResizeMemory (SideDefs, NumSideDefs * sizeof(SideDef));
			}
			else
			{
				FreeMemory( SideDefs);
				SideDefs = NULL;
			}

			// Update objnum of selection list
			for (cur = (*list)->next; cur; cur = cur->next)
				if (cur->objnum > objnum)
					cur->objnum--;
			UnSelectObject( list, objnum);
		}
		MadeMapChanges = TRUE;
		break;

   case OBJ_SECTORS:
		while (*list)
		{
			objnum = (*list)->objnum;
			// delete the SideDefs bound to this Sector and
			// change the references

			for (n = 0; n < NumSideDefs; n++)
				if (SideDefs[ n].sector == objnum)
					DeleteObject( OBJ_SIDEDEFS, n--);
				else if (SideDefs[ n].sector >= objnum)
					SideDefs[ n].sector--;

			// delete the Sector
			NumSectors--;
			if (NumSectors > 0)
			{
				for (n = objnum; n < NumSectors; n++)
					Sectors[ n] = Sectors[ n + 1];
				Sectors = (SPtr)ResizeMemory (Sectors, NumSectors * sizeof (Sector));
			}
			else
			{
				FreeMemory( Sectors);
				Sectors = NULL;
			}
			// Update objnum of selection list
			for (cur = (*list)->next; cur; cur = cur->next)
				if (cur->objnum > objnum)
					cur->objnum--;
			UnSelectObject( list, objnum);
		}
		break;

	default:
		Beep();
	}
}



/*
   insert a new object
*/

void InsertObject (int objtype, SHORT copyfrom, SHORT xpos, SHORT ypos)
{
	SHORT last;
	Thing NewThing;
	LineDef NewLineDef;
	SideDef NewSideDef;
	Vertex NewVertex;
	Sector NewSector;
	int i;

	MadeChanges = TRUE;
	switch (objtype)
	{
	case OBJ_THINGS:
		last = NumThings++;
		if (last > 0)
			Things = (TPtr)ResizeMemory (Things, NumThings * sizeof (Thing));
		else
			Things = (TPtr)GetMemory (sizeof(Thing));

		// Init new thing
		memset (&NewThing, 0, sizeof (Thing));

		NewThing.xpos = xpos;
		NewThing.ypos = ypos;
		if (copyfrom >= 0)
		{
			Thing TCopy = Things[copyfrom];

			NewThing.type		= TCopy.type;
			NewThing.angle		= TCopy.angle;
			NewThing.when		= TCopy.when;
			NewThing.id			= TCopy.id;
			NewThing.xExitPos = TCopy.xExitPos;
			NewThing.yExitPos = TCopy.yExitPos;
			NewThing.flags		= TCopy.flags;
			if (NewThing.type == kodEntrance)
			{
				int x = NewThing.xpos;
				int y = NewThing.ypos;
				int col,row;
				
            GetServerCoords(&x,&y,&col,&row);
				NewThing.xExitPos = col;
				NewThing.yExitPos = row;
				NewThing.id = RoomID;
			}
		}
		else
		{
			NewThing.type  = kodObject;
		}
		Things[last] = NewThing;
		break;

	case OBJ_VERTEXES:
		last = NumVertexes++;
		if (last > 0)
			Vertexes = (VPtr)ResizeMemory (Vertexes, NumVertexes * sizeof(Vertex));
		else
			Vertexes = (VPtr)GetMemory (sizeof(Vertex));
		memset (&NewVertex, 0, sizeof (Vertex));

		/* kluge: the Nodes builder will put -2 in copyfrom */
		if (copyfrom == -2)
		{
			NewVertex.x = xpos;
			NewVertex.y = ypos;
		}
		else
		{
         // If this vertex is copied from another one, the old vertex needs
         // to keep track of the new one in case it was part of a slope.
         // The new slope (if this is part of a copied sector) will have to
         // get the new vertex from the old one.
         if (copyfrom >= 0)
            Vertexes[copyfrom].copiedTo = last;

         // These lines seems to just put the vertex in the wrong position,
         // commented out for now and replaced with exact x/y pos.
         //NewVertex.x = xpos & ~7;
			//NewVertex.y = ypos & ~7;
         NewVertex.x = xpos;
         NewVertex.y = ypos;
			if (NewVertex.x < MapMinX)	  MapMinX = NewVertex.x;
			if (NewVertex.x > MapMaxX)    MapMaxX = NewVertex.x;
			if (NewVertex.y < MapMinY)    MapMinY = NewVertex.y;
			if (NewVertex.y > MapMaxY)    MapMaxY = NewVertex.y;
			MadeMapChanges = TRUE;
		}

      NewVertex.copiedTo = -1;
		Vertexes[last] = NewVertex;
		break;

   case OBJ_LINEDEFS:
		last = NumLineDefs++;
		if (last > 0)
			LineDefs = (LDPtr)ResizeMemory (LineDefs, NumLineDefs * sizeof(LineDef));
		else
			LineDefs = (LDPtr)GetMemory (sizeof (LineDef));
		memset (&NewLineDef, 0, sizeof (LineDef));

		if (copyfrom >= 0)
		{
			LineDef LdCopy = LineDefs[copyfrom];

			NewLineDef.start = LdCopy.start;
			NewLineDef.end   = LdCopy.end;
			NewLineDef.flags = LdCopy.flags;
			NewLineDef.type  = LdCopy.type;
			NewLineDef.tag   = LdCopy.tag;
			NewLineDef.blak_flags   = LdCopy.blak_flags;
		}
		else
		{
			NewLineDef.start = 0;
			NewLineDef.end   = NumVertexes - 1;
			NewLineDef.flags = 1;
			NewLineDef.type  = 0;
			NewLineDef.tag   = 0;
		}
		NewLineDef.sidedef1 = -1;
		NewLineDef.sidedef2 = -1;

		LineDefs[last] = NewLineDef;
		break;

   case OBJ_SIDEDEFS:
		/* SideDefs are added from the LineDefs menu, so "copyfrom" should always be -1.  But I test it anyway. */
		last = NumSideDefs++;
		if (last > 0)
			SideDefs = (SDPtr)ResizeMemory (SideDefs, NumSideDefs * sizeof(SideDef));
		else
			SideDefs = (SDPtr)GetMemory (sizeof(SideDef));

		// Init new sidedef
		memset (&NewSideDef, 0, sizeof (SideDef));
		if (copyfrom >= 0)
		{
			SideDef SDCopy = SideDefs [copyfrom];

			NewSideDef.xoff = SDCopy.xoff;
			NewSideDef.yoff = SDCopy.yoff;
			strncpy (NewSideDef.tex1, SDCopy.tex1, MAX_BITMAPNAME);
			strncpy (NewSideDef.tex2, SDCopy.tex2, MAX_BITMAPNAME);
			strncpy (NewSideDef.tex3, SDCopy.tex3, MAX_BITMAPNAME);
			NewSideDef.sector = SDCopy.sector;
			NewSideDef.type1 = SDCopy.type1;
			NewSideDef.type2 = SDCopy.type2;
			NewSideDef.type3 = SDCopy.type3;
			NewSideDef.animate_speed = SDCopy.animate_speed;
			NewSideDef.user_id   = SDCopy.user_id;
		}
		else
		{
			NewSideDef.xoff = 0;
			NewSideDef.yoff = 0;
			strcpy (NewSideDef.tex1, DefaultUpperTexture);
			strcpy (NewSideDef.tex2, DefaultLowerTexture);
			strcpy (NewSideDef.tex3, DefaultWallTexture);
			NewSideDef.sector = NumSectors - 1;
			NewSideDef.type1 = TextureToNumber(NewSideDef.tex1);
			NewSideDef.type2 = TextureToNumber(NewSideDef.tex2);
			NewSideDef.type3 = TextureToNumber(NewSideDef.tex3);
			NewSideDef.animate_speed = 0;
		}
		SideDefs[last] = NewSideDef;
		MadeMapChanges = TRUE;
		break;

	case OBJ_SECTORS:
		last = NumSectors++;
		if (last > 0)
			Sectors = (SPtr)ResizeMemory (Sectors, NumSectors * sizeof (Sector));
		else
			Sectors = (SPtr)GetMemory (sizeof(Sector));
		memset (&NewSector, 0, sizeof (Sector));

		if (copyfrom >= 0)
		{
			Sector SCopy = Sectors[ copyfrom];

			NewSector.floorh = SCopy.floorh;
			NewSector.ceilh = SCopy.ceilh;
			strncpy (NewSector.floort, SCopy.floort, MAX_BITMAPNAME);
			strncpy (NewSector.ceilt, SCopy.ceilt, MAX_BITMAPNAME);
			NewSector.light = SCopy.light;
			NewSector.special = SCopy.special;
			NewSector.tag = SCopy.tag;
			NewSector.floor_type = SCopy.floor_type;
			NewSector.ceiling_type = SCopy.ceiling_type;
			NewSector.xoffset = SCopy.xoffset;
			NewSector.yoffset = SCopy.yoffset;
			NewSector.blak_flags = SCopy.blak_flags;
			NewSector.animate_speed = SCopy.animate_speed;

         // Copy the slope data.
         NewSector.ceiling_slope = SCopy.ceiling_slope;
         NewSector.floor_slope = SCopy.floor_slope;
         // Put proper vertexes in slopes, using the numbers of the new
         // vertexes stored in the copied old ones.
         for (int i = 0; i < 3; i++)
         {
            if (SCopy.blak_flags & SF_SLOPED_FLOOR)
               if (Vertexes[NewSector.floor_slope.points[i].vertex].copiedTo >= 0)
                  NewSector.floor_slope.points[i].vertex = Vertexes[NewSector.floor_slope.points[i].vertex].copiedTo;
            if (SCopy.blak_flags & SF_SLOPED_CEILING)
               if (Vertexes[NewSector.ceiling_slope.points[i].vertex].copiedTo >= 0)
                  NewSector.ceiling_slope.points[i].vertex = Vertexes[NewSector.ceiling_slope.points[i].vertex].copiedTo;
         }
		}
		else
		{
			NewSector.floorh = DefaultFloorHeight;
			NewSector.ceilh = DefaultCeilingHeight;
			strncpy (NewSector.floort, DefaultFloorTexture, MAX_BITMAPNAME);
			strncpy (NewSector.ceilt, DefaultCeilingTexture, MAX_BITMAPNAME);
			NewSector.light = DefaultLightLevel;
			NewSector.special = 0;
			NewSector.tag = 0;
			NewSector.floor_type = TextureToNumber(NewSector.floort);
			NewSector.ceiling_type = TextureToNumber(NewSector.ceilt);
			NewSector.xoffset = 0;
			NewSector.yoffset = 0;
			NewSector.blak_flags = 0;
			NewSector.animate_speed = 0;
			for (i=0; i < 3; i++)
			{
			   NewSector.floor_slope.points[i].vertex = -1;
			   NewSector.floor_slope.points[i].z = -1;
			   NewSector.ceiling_slope.points[i].vertex = -1;
			   NewSector.ceiling_slope.points[i].z = -1;
			}
		}
      // Don't need to do this anymore, slope data gets copied.
		// Set slope information to OFF
		//NewSector.blak_flags &= ~SF_SLOPED_FLOOR;
		//NewSector.blak_flags &= ~SF_SLOPED_CEILING;
		Sectors[last] = NewSector;
		break;

	default:
		Beep();
	}
}

/*
   check if a (part of a) LineDef is inside a given block
*/

BOOL IsLineDefInside (SHORT ldnum, SHORT x0, SHORT y0, SHORT x1, SHORT y1)
{
	LineDef *pLD = &LineDefs[ldnum];
	SHORT start = pLD->start;
	SHORT end   = pLD->end;

	Vertex *pVStart = &Vertexes[start];
	Vertex *pVEnd   = &Vertexes[end];
	SHORT lx0 = pVStart->x;
	SHORT ly0 = pVStart->y;
	SHORT lx1 = pVEnd->x;
	SHORT ly1 = pVEnd->y;
	SHORT i;

	// The LineDef start is entirely inside the square ?
	if (lx0 >= x0 && lx0 <= x1 && ly0 >= y0 && ly0 <= y1)
		return TRUE;

	// The LineDef end is entirely inside the square ?
	if (lx1 >= x0 && lx1 <= x1 && ly1 >= y0 && ly1 <= y1)
		return TRUE;


	// The LineDef crosses the y0 side (left) ?
	if ((ly0 > y0) != (ly1 > y0))
	{
		i = lx0 + (SHORT) ( (long) (y0 - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
		if (i >= x0 && i <= x1)
			return TRUE;
	}

	// The LineDef crosses the y1 side (right) ?
	if ((ly0 > y1) != (ly1 > y1))
	{
		i = lx0 + (SHORT) ( (long) (y1 - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
		if (i >= x0 && i <= x1)
			return TRUE;
	}

	// The LineDef crosses the x0 side (down) ?
	if ((lx0 > x0) != (lx1 > x0))
	{
		i = ly0 + (SHORT) ( (long) (x0 - lx0) * (long) (ly1 - ly0) / (long) (lx1 - lx0));
		if (i >= y0 && i <= y1)
			return TRUE;
	}

	// The LineDef crosses the x1 side (up) ?
	if ((lx0 > x1) != (lx1 > x1))
	{
		i = ly0 + (SHORT) ( (long) (x1 - lx0) * (long) (ly1 - ly0) / (long) (lx1 - lx0));
		if (i >= y0 && i <= y1)
			return TRUE;
	}

	return FALSE;
}



/*
   get the Sector number of the SideDef opposite to this LineDef
   (returns -1 if it cannot be found)
*/

SHORT GetOppositeSector (SHORT ld1, BOOL firstside)
{
	SHORT x0, y0, dx0, dy0;
	SHORT x1, y1, dx1, dy1;
	SHORT x2, y2, dx2, dy2;
	SHORT ld2, dist;
	SHORT bestld, bestdist, bestmdist;

	/* get the coords for this LineDef */
	LineDef *pLineDef1 = &LineDefs[ld1];
	Vertex v1s = Vertexes[pLineDef1->start];
	Vertex v1e = Vertexes[pLineDef1->end];

	x0  = v1s.x;
	y0  = v1s.y;
	dx0 = v1e.x - x0;
	dy0 = v1e.y - y0;

	/* find the normal vector for this LineDef */
	x1  = (dx0 + x0 + x0) / 2;
	y1  = (dy0 + y0 + y0) / 2;
	if (firstside == TRUE)
	{
		dx1 = dy0;
		dy1 = -dx0;
	}
	else
	{
		dx1 = -dy0;
		dy1 = dx0;
	}

	bestld = -1;
	// use a parallel to an axis instead of the normal vector (faster method)
	if ( abs(dy1) > abs(dx1) )
	{
		if (dy1 > 0)
		{
			// get the nearest LineDef in that direction (increasing Y's: North)
			bestdist = 32767;
			bestmdist = 32767;
			for (ld2 = 0; ld2 < NumLineDefs; ld2++)
			{
				LineDef *pLineDef2 = &LineDefs[ld2];
				Vertex v2s = Vertexes[pLineDef2->start];
				Vertex v2e = Vertexes[pLineDef2->end];
				if (ld2 != ld1 && ((v2s.x > x1) != (v2e.x > x1)))
				{
					x2  = v2s.x;
					y2  = v2s.y;
					dx2 = v2e.x - x2;
					dy2 = v2e.y - y2;
					dist = y2 + (SHORT) (((LONG) (x1 - x2) * dy2) / dx2);
					if (dist > y1 &&
						(dist < bestdist ||
						 (dist == bestdist && (y2 + dy2 / 2) < bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = y2 + dy2 / 2;
					}
				}
			}
		}
		else
		{
			/* get the nearest LineDef in that direction (decreasing Y's: South) */
			bestdist = -32767;
			bestmdist = -32767;
			for (ld2 = 0; ld2 < NumLineDefs; ld2++)
			{
				LineDef *pLineDef2 = &LineDefs[ld2];
				Vertex v2s = Vertexes[pLineDef2->start];
				Vertex v2e = Vertexes[pLineDef2->end];
				if (ld2 != ld1 && ((v2s.x > x1) != (v2e.x > x1)))
				{
					x2  = v2s.x;
					y2  = v2s.y;
					dx2 = v2e.x - x2;
					dy2 = v2e.y - y2;
					dist = y2 + (SHORT) (((LONG) (x1 - x2) *dy2) / dx2);
					if (dist < y1 &&
						(dist > bestdist ||
						 (dist == bestdist && (y2 + dy2 / 2) > bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = y2 + dy2 / 2;
					}
				}
			}
		}
	}
	else
	{
		if (dx1 > 0)
		{
			/* get the nearest LineDef in that direction (increasing X's: East) */
			bestdist = 32767;
			bestmdist = 32767;
			for (ld2 = 0; ld2 < NumLineDefs; ld2++)
			{
				LineDef *pLineDef2 = &LineDefs[ld2];
				Vertex v2s = Vertexes[pLineDef2->start];
				Vertex v2e = Vertexes[pLineDef2->end];
				if (ld2 != ld1 && ((v2s.y > y1) != (v2e.y > y1)))
				{
					x2  = v2s.x;
					y2  = v2s.y;
					dx2 = v2e.x - x2;
					dy2 = v2e.y - y2;
					dist = x2 + (SHORT) (((LONG) (y1 - y2) * dx2) / dy2);
					if (dist > x1 &&
						(dist < bestdist ||
						 (dist == bestdist && (x2 + dx2 / 2) < bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = x2 + dx2 / 2;
					}
				}
			}
		}
		else
		{
			/* get the nearest LineDef in that direction (decreasing X's: West) */
			bestdist = -32767;
			bestmdist = -32767;
			for (ld2 = 0; ld2 < NumLineDefs; ld2++)
			{
				LineDef *pLineDef2 = &LineDefs[ld2];
				Vertex v2s = Vertexes[pLineDef2->start];
				Vertex v2e = Vertexes[pLineDef2->end];
				if (ld2 != ld1 && ((v2s.y > y1) != (v2e.y > y1)))
				{
					x2  = v2s.x;
					y2  = v2s.y;
					dx2 = v2e.x - x2;
					dy2 = v2e.y - y2;
					dist = x2 + (SHORT) (((LONG) (y1 - y2) * dx2) / dy2);
					if (dist < x1 &&
						(dist > bestdist ||
						 (dist == bestdist && (x2 + dx2 / 2) > bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = x2 + dx2 / 2;
					}
				}
			}
		}
	}

	/* no intersection: the LineDef was pointing outwards! */
	if (bestld < 0)
		return -1;

	/* now look if this LineDef has a SideDef bound to one sector */
	pLineDef1 = &LineDefs[bestld];
	v1s = Vertexes[pLineDef1->start];
	v1e = Vertexes[pLineDef1->end];

	if (abs( dy1) > abs( dx1))
	{

		if ( (v1s.x < v1e.x) == (dy1 > 0) )
			x0 = pLineDef1->sidedef1;
		else
			x0 = pLineDef1->sidedef2;
	}
	else
	{
		if ( (v1s.y < v1e.y) != (dx1 > 0) )
			x0 = pLineDef1->sidedef1;
		else
			x0 = pLineDef1->sidedef2;
	}

	/* there is no SideDef on this side of the LineDef! */
	if (x0 < 0)
		return -1;

	/* OK, we got it -- return the Sector number */
	return SideDefs[x0].sector;
}



/*
   copy a group of objects to a new position
   The selection is modified to contain the new created objects.
*/

void CopyObjects (int objtype, SelPtr obj)
{
	SHORT      n;
	SelPtr     cur;
	SelPtr     list1, list2;
	SelPtr     ref1, ref2;

	if (obj == NULL)
		return;

	/* copy the object(s) */
	switch (objtype)
	{
	case OBJ_THINGS:
		for (cur = obj; cur; cur = cur->next)
		{
			Thing *pThing = &Things[cur->objnum];
			InsertObject (OBJ_THINGS, cur->objnum, pThing->xpos, pThing->ypos);
			cur->objnum = NumThings - 1;
		}
		MadeChanges = TRUE;
		break;

	case OBJ_VERTEXES:
		for (cur = obj; cur; cur = cur->next)
		{
			Vertex *pVertex = &Vertexes[cur->objnum];
			InsertObject (OBJ_VERTEXES, cur->objnum, pVertex->x-30, pVertex->y-30);
			cur->objnum = NumVertexes - 1;
		}
		MadeChanges = TRUE;
		MadeMapChanges = TRUE;
		break;

	case OBJ_LINEDEFS:
		list1 = NULL;
		list2 = NULL;
		/* create the LineDefs */
		for (cur = obj; cur; cur = cur->next)
		{
         LineDef *pLineDef = &LineDefs[cur->objnum];
         SHORT sd1 = pLineDef->sidedef1;
         SHORT sd2 = pLineDef->sidedef2;

         InsertObject (OBJ_LINEDEFS, cur->objnum, 0, 0);
			cur->objnum = NumLineDefs - 1;
         /* Create the SideDefs info */
         LineDef *pLineDef1 = &LineDefs[cur->objnum];
         if (sd1 >= 0)
         {
            InsertObject(OBJ_SIDEDEFS, sd1, 0, 0);
            sd1 = NumSideDefs - 1;
            pLineDef1->sidedef1 = sd1;
         }
         if (sd2 >= 0)
         {
            InsertObject(OBJ_SIDEDEFS, sd2, 0, 0);
            sd2 = NumSideDefs - 1;
            pLineDef1->sidedef2 = sd2;
         }

         if (!IsSelected( list1, pLineDef->start))
			{
				SelectObject(&list1, pLineDef->start);
				SelectObject(&list2, pLineDef->start);
			}
			if (!IsSelected (list1, pLineDef->end))
			{
				SelectObject (&list1, pLineDef->end);
				SelectObject (&list2, pLineDef->end);
			}
		}
		/* create the Vertices */
		CopyObjects (OBJ_VERTEXES, list2);

		/* update the references to the Vertexes */
		for (ref1 = list1,      ref2 = list2;
			 ref1 != NULL &&    ref2 != NULL;
			 ref1 = ref1->next, ref2 = ref2->next)
		{
			for (cur = obj; cur; cur = cur->next)
			{
				LineDef *pLineDef = &LineDefs[cur->objnum];

				if (ref1->objnum == pLineDef->start)
					pLineDef->start = ref2->objnum;

				if (ref1->objnum == pLineDef->end)
					pLineDef->end = ref2->objnum;
			}
		}
		ForgetSelection (&list1);
		ForgetSelection (&list2);
		break;

   // Copies linedefs and vertexes but not sidedefs.
   case OBJ_LINEDEFSNOSIDEDEFS:
      list1 = NULL;
      list2 = NULL;
      /* create the LineDefs */
      for (cur = obj; cur; cur = cur->next)
      {
         LineDef *pLineDef = &LineDefs[cur->objnum];
         InsertObject(OBJ_LINEDEFS, cur->objnum, 0, 0);
         cur->objnum = NumLineDefs - 1;

         if (!IsSelected(list1, pLineDef->start))
         {
            SelectObject(&list1, pLineDef->start);
            SelectObject(&list2, pLineDef->start);
         }
         if (!IsSelected(list1, pLineDef->end))
         {
            SelectObject(&list1, pLineDef->end);
            SelectObject(&list2, pLineDef->end);
         }
      }
      /* create the Vertices */
      CopyObjects(OBJ_VERTEXES, list2);

      /* update the references to the Vertexes */
      for (ref1 = list1, ref2 = list2;
         ref1 != NULL &&    ref2 != NULL;
         ref1 = ref1->next, ref2 = ref2->next)
      {
         for (cur = obj; cur; cur = cur->next)
         {
            LineDef *pLineDef = &LineDefs[cur->objnum];

            if (ref1->objnum == pLineDef->start)
               pLineDef->start = ref2->objnum;

            if (ref1->objnum == pLineDef->end)
               pLineDef->end = ref2->objnum;
         }
      }
      ForgetSelection(&list1);
      ForgetSelection(&list2);
      break;

	case OBJ_SECTORS:
		list1 = NULL;
		list2 = NULL;

		/* create the LineDefs (and Vertices) */
		for (cur = obj; cur; cur = cur->next)
		{
			for (n = 0; n < NumLineDefs; n++)
			{
				LineDef *pLineDef = &LineDefs[n];
				SHORT sd1 = pLineDef->sidedef1;
				SHORT sd2 = pLineDef->sidedef2;
				if ( ((sd1 >= 0 && SideDefs[sd1].sector == cur->objnum) ||
					  (sd2 >= 0 && SideDefs[sd2].sector == cur->objnum)  )
					&& ! IsSelected (list1, n))
				{
					SelectObject (&list1, n);
					SelectObject (&list2, n);
				}
			}
		}

      // Create the LineDefs and vertices. Calling CopyObjects with
      // this object type copies the linedefs and vertex positions but
      // doesn't copy the sidedef info (we add that here).
      CopyObjects(OBJ_LINEDEFSNOSIDEDEFS, list2);

		 //Create the SideDefs info */
		for (ref1 = list1,      ref2 = list2;
			 ref1 != NULL &&    ref2 != NULL;
			 ref1 = ref1->next, ref2 = ref2->next)
		{
			//BUG(NO) Don't need to update later since insert SideDefs only
			LineDef *pLineDef1 = &LineDefs[ref1->objnum];
			LineDef *pLineDef2 = &LineDefs[ref2->objnum];
			SHORT sd1 = pLineDef1->sidedef1;
			SHORT sd2 = pLineDef1->sidedef2;

			if ( sd1 >= 0)
			{
				InsertObject (OBJ_SIDEDEFS, sd1, 0, 0);
				sd1 = NumSideDefs - 1;
				pLineDef2->sidedef1 = sd1;
			}
			if ( sd2 >= 0)
			{
				InsertObject (OBJ_SIDEDEFS, sd2, 0, 0);
				sd2 = NumSideDefs - 1;
				pLineDef2->sidedef2 = sd2;
			}
			ref1->objnum = sd1;
			ref2->objnum = sd2;
		}

		/* create the Sectors */
		for (cur = obj; cur; cur = cur->next)
		{
			InsertObject (OBJ_SECTORS, cur->objnum, 0, 0);

			for (ref1 = list1,      ref2 = list2;
				 ref1 != NULL    && ref2 != NULL;
				 ref1 = ref1->next, ref2 = ref2->next)
			{
				SHORT sd1 = ref1->objnum;
				SHORT sd2 = ref2->objnum;
				SideDef *pSideDef1;
				SideDef *pSideDef2;
				if (sd1 >= 0 )		pSideDef1 = &SideDefs[sd1];
				if (sd2 >= 0 )		pSideDef2 = &SideDefs[sd2];

				if ( sd1 >= 0  &&  pSideDef1->sector == cur->objnum)
					pSideDef1->sector = NumSectors - 1;

				if ( sd2 >= 0  &&  pSideDef2->sector == cur->objnum)
					pSideDef2->sector = NumSectors - 1;
			}
			cur->objnum = NumSectors - 1;
		}
		ForgetSelection (&list1);
		ForgetSelection (&list2);
		break;
	}
}

/*
   move a group of objects to a new position
   (must be called with obj = NULL before moving the objects)
*/

BOOL MoveObjectsToCoords (int objtype, SelPtr obj, SHORT newx, SHORT newy,
						  SHORT grid)
{
	SHORT        n;
	SHORT        dx, dy;
	SelPtr       cur, vertices;
	static SHORT refx, refy; /* previous position */

	// Snap to grid
	if (grid > 0)
	{
		newx = (newx + grid / 2) & ~(grid - 1);
		newy = (newy + grid / 2) & ~(grid - 1);
	}

	/* only update the reference point? */
	if (obj == NULL)
	{
		refx = newx;
		refy = newy;
		return TRUE;
	}

	/* compute the displacement */
	dx = newx - refx;
	dy = newy - refy;
	/* nothing to do? */
	if (dx == 0 && dy == 0)
		return FALSE;

	/* move the object(s) */
	switch (objtype)
	{
	case OBJ_THINGS:
		for (cur = obj; cur; cur = cur->next)
		{
			assert (cur->objnum >= 0  &&  cur->objnum < NumThings);
			Thing *pThing = &Things[cur->objnum];
			pThing->xpos += dx;
			pThing->ypos += dy;
			if (pThing->type == kodEntrance)
			{
//				pThing->id = SetEntrance(RoomID, pThing->id, pThing->xpos, pThing->ypos, pThing->angle, pThing->comment);
			}
		}
		refx = newx;
		refy = newy;
		MadeChanges = TRUE;
		break;

	case OBJ_VERTEXES:
		for (cur = obj; cur; cur = cur->next)
		{
			assert_vnum(cur->objnum);
			Vertex *pVertex = &Vertexes[cur->objnum];

			pVertex->x += dx;
			pVertex->y += dy;
		}
		refx = newx;
		refy = newy;
		MadeChanges = TRUE;
		MadeMapChanges = TRUE;
		break;

	case OBJ_LINEDEFS:
		vertices = NULL;
		for (cur = obj; cur; cur = cur->next)
		{
			assert_ldnum(cur->objnum);
			LineDef *pLineDef = &LineDefs[cur->objnum];
			SHORT start = pLineDef->start;
			SHORT end   = pLineDef->end;

			if ( !IsSelected (vertices, start) )
				SelectObject (&vertices, start);

			if ( !IsSelected (vertices, end) )
				SelectObject (&vertices, end);
		}
		MoveObjectsToCoords (OBJ_VERTEXES, vertices, newx, newy, grid);
		ForgetSelection (&vertices);
		break;

	case OBJ_SECTORS:
		vertices = NULL;
		for (cur = obj; cur; cur = cur->next)
		{
			assert_snum (cur->objnum);
			for (n = 0; n < NumLineDefs; n++)
			{
				LineDef *pLineDef = &LineDefs[n];
				SHORT sd1   = pLineDef->sidedef1;
				SHORT sd2   = pLineDef->sidedef2;
				SHORT start = pLineDef->start;
				SHORT end   = pLineDef->end;

				if ( (sd1 >= 0 && SideDefs[sd1].sector == cur->objnum) ||
					 (sd2 >= 0 && SideDefs[sd2].sector == cur->objnum) )
				{
					if ( !IsSelected (vertices, start) )
						SelectObject (&vertices, start);

					if ( !IsSelected (vertices, end) )
						SelectObject (&vertices, end);
				}
			}
		}
		MoveObjectsToCoords (OBJ_VERTEXES, vertices, newx, newy, grid);
		ForgetSelection (&vertices);
		break;
	}
	return TRUE;
}



/*
   get the coordinates (approx.) of an object
*/

void GetObjectCoords (int objtype, SHORT objnum, SHORT *xpos, SHORT *ypos)
{
	SHORT         n;
	LONG          accx, accy, num;
	Thing   *pThing;
	Vertex  *pVertex;
	LineDef *pLineDef;
	Vertex  *pv1;
	Vertex  *pv2;

	switch (objtype)
	{
	case OBJ_THINGS:
		assert (objnum >= 0  &&  objnum < NumThings);
		pThing = &Things[objnum];

		*xpos = pThing->xpos;
		*ypos = pThing->ypos;
		break;

	case OBJ_VERTEXES:
		assert_vnum(objnum);
		pVertex = &Vertexes[objnum];

		*xpos = pVertex->x;
		*ypos = pVertex->y;
		break;

	case OBJ_LINEDEFS:
		assert_ldnum(objnum);
		pLineDef = &LineDefs[objnum];
		pv1 = &Vertexes[pLineDef->start];
		pv2 = &Vertexes[pLineDef->end];

		*xpos = (pv1->x + pv2->x) / 2;
		*ypos = (pv1->y + pv2->y) / 2;
		break;

	case OBJ_SIDEDEFS:
		assert (objnum >= 0  &&  objnum < NumSideDefs);
		for (n = 0; n < NumLineDefs; n++)
		{
			pLineDef = &LineDefs[n];
			if ( pLineDef->sidedef1 == objnum || pLineDef->sidedef2 == objnum)
			{
				pv1 = &Vertexes[pLineDef->start];
				pv2 = &Vertexes[pLineDef->end];

				*xpos = (pv1->x + pv2->x) / 2;
				*ypos = (pv1->y + pv2->y) / 2;
				return;
			}
		}
		*xpos = (MapMinX + MapMaxX) / 2;
		*ypos = (MapMinY + MapMaxY) / 2;
		break;

	case OBJ_SECTORS:
		assert (objnum >= 0  &&  objnum < NumSectors);
		accx = 0L;
		accy = 0L;
		num = 0L;
		for (n = 0; n < NumLineDefs; n++)
		{
			pLineDef = &LineDefs[n];
			SHORT sd1 = pLineDef->sidedef1;
			SHORT sd2 = pLineDef->sidedef2;

			if ( (sd1 >= 0 && SideDefs[sd1].sector == objnum) ||
				 (sd2 >= 0 && SideDefs[sd2].sector == objnum) )
			{
				pv1 = &Vertexes[pLineDef->start];
				pv2 = &Vertexes[pLineDef->end];

				/* if the Sector is closed, all Vertices will be counted twice */
				accx += (LONG) pv1->x;
				accy += (LONG) pv1->y;
				num++;
				accx += (LONG) pv2->x;
				accy += (LONG) pv2->y;
				num++;
			}
		}
		if (num > 0)
		{
			*xpos = (SHORT) ((accx + num / 2L) / num);
			*ypos = (SHORT) ((accy + num / 2L) / num);
		}
		else
		{
			*xpos = (MapMinX + MapMaxX) / 2;
			*ypos = (MapMinY + MapMaxY) / 2;
		}
		break;
	}
}



/*
   rotate and scale a group of objects around the center of gravity
*/

void RotateAndScaleObjects (int objtype, SelPtr obj, double angle, double scale)
{
	SHORT  n, m;
	SHORT  dx, dy;
	SHORT  centerx, centery;
	LONG   accx, accy, num;
	SelPtr cur, vertices;

	if (obj == NULL)
		return;

	switch (objtype)
	{
	case OBJ_THINGS:
		accx = 0L;
		accy = 0L;
		num = 0L;
		for (cur = obj; cur; cur = cur->next)
		{
			assert_tnum(cur->objnum);
			Thing *pThing = &Things[cur->objnum];

			accx += (LONG) pThing->xpos;
			accy += (LONG) pThing->ypos;
			num++;
		}
		centerx = (SHORT) ((accx + num / 2L) / num);
		centery = (SHORT) ((accy + num / 2L) / num);

		for (cur = obj; cur; cur = cur->next)
		{
			assert_tnum(cur->objnum);
			Thing *pThing = &Things[cur->objnum];

			dx = pThing->xpos - centerx;
			dy = pThing->ypos - centery;
			RotateAndScaleCoords (&dx, &dy, angle, scale);
			pThing->xpos = centerx + dx;
			pThing->ypos = centery + dy;
		}
		MadeChanges = TRUE;
		break;

	  case OBJ_VERTEXES:
		accx = 0L;
		accy = 0L;
		num = 0L;
		for (cur = obj; cur; cur = cur->next)
		{
			accx += (LONG) Vertexes[ cur->objnum].x;
			accy += (LONG) Vertexes[ cur->objnum].y;
			num++;
		}
		centerx = (SHORT) ((accx + num / 2L) / num);
		centery = (SHORT) ((accy + num / 2L) / num);

		for (cur = obj; cur; cur = cur->next)
		{
			dx = Vertexes[ cur->objnum].x - centerx;
			dy = Vertexes[ cur->objnum].y - centery;
			RotateAndScaleCoords( &dx, &dy, angle, scale);
			Vertexes[ cur->objnum].x = (centerx + dx + 4) & ~7;
			Vertexes[ cur->objnum].y = (centery + dy + 4) & ~7;
		}
		MadeChanges = TRUE;
		MadeMapChanges = TRUE;
		break;

	case OBJ_LINEDEFS:
		vertices = NULL;
		for (cur = obj; cur; cur = cur->next)
		{
			SHORT start = LineDefs[cur->objnum].start;
			SHORT end   = LineDefs[cur->objnum].end;

			if (!IsSelected (vertices, start))
				SelectObject (&vertices, start);

			if (!IsSelected (vertices, end))
				SelectObject (&vertices, end);
		}

		RotateAndScaleObjects (OBJ_VERTEXES, vertices, angle, scale);
		ForgetSelection (&vertices);
		break;

	case OBJ_SECTORS:
		vertices = NULL;
		for (cur = obj; cur; cur = cur->next)
		{
			for (n = 0; n < NumLineDefs; n++)
			if (((m = LineDefs[ n].sidedef1) >= 0 && SideDefs[ m].sector == cur->objnum)
				 || ((m = LineDefs[ n].sidedef2) >= 0 && SideDefs[ m].sector == cur->objnum))
			{
				if (!IsSelected (vertices, LineDefs[ n].start))
					SelectObject (&vertices, LineDefs[ n].start);

				if (!IsSelected (vertices, LineDefs[ n].end))
					SelectObject (&vertices, LineDefs[ n].end);
			}
		}
		RotateAndScaleObjects (OBJ_VERTEXES, vertices, angle, scale);
		ForgetSelection (&vertices);
		break;
	}
}



/*
   find a free tag number
*/

SHORT FindFreeTag()
{
	SHORT tag, n;
	BOOL  ok;

	tag = 1;
	ok = FALSE;
	while (! ok)
	{
		ok = TRUE;
		if (ok)
		   for (n = 0; n < NumSectors; n++)
		      if (Sectors[ n].user_id == tag)
		      {
			 ok = FALSE;
			 break;
		      }
		tag++;
	}
	return tag - 1;
}



/*
   flip one or several LineDefs
*/

void FlipLineDefs (SelPtr obj, BOOL swapvertices)
{
	SelPtr cur;
	SHORT  tmp;

	for (cur = obj; cur; cur = cur->next)
	{
		LineDef *pLineDef = &LineDefs[cur->objnum];
		if (swapvertices)
		{
			/* swap starting and ending Vertices */
			tmp             = pLineDef->end;
			pLineDef->end   = pLineDef->start;
			pLineDef->start = tmp;
		}
		/* swap first and second SideDefs */
		tmp                = pLineDef->sidedef1;
		pLineDef->sidedef1 = pLineDef->sidedef2;
		pLineDef->sidedef2 = tmp;
	}
	MadeChanges = TRUE;
	MadeMapChanges = TRUE;
}



/*
   delete a Vertex and join the two LineDefs
*/

void DeleteVerticesJoinLineDefs (SelPtr obj)
{
	SHORT  lstart, lend, l;
	SelPtr cur;

	while (obj != NULL)
	{
		cur = obj;
		obj = obj->next;
		lstart = -1;
		lend = -1;
		for (l = 0; l < NumLineDefs; l++)
		{
			if (LineDefs[ l].start == cur->objnum)
			{
				if (lstart == -1)
					lstart = l;
				else
					lstart = -2;
			}
			if (LineDefs[ l].end == cur->objnum)
			{
				if (lend == -1)
					lend = l;
				else
					lend = -2;
			}
		}
		if (lstart < 0 || lend < 0)
		{
			Beep();
			Notify ("Cannot delete Vertex #%d and join the LineDefs. "
					"The Vertex must be the start of one LineDef and the "
					"end of another one", cur->objnum);
			continue;
		}
		LineDefs[ lend].end = LineDefs[ lstart].end;
		DeleteObject( OBJ_LINEDEFS, lstart);
		DeleteObject( OBJ_VERTEXES, cur->objnum);
		MadeChanges = TRUE;
		MadeMapChanges = TRUE;
	}
}



/*
   merge several vertices into one
*/

void MergeVertices (SelPtr *list)
{
	SHORT v, l;

	v = (*list)->objnum;
	UnSelectObject (list, v);

	if (*list == NULL)
	{
		Beep();
		Notify ("You must select at least two vertices !");
		return;
	}

	// change the LineDefs starts & ends
	for (l = 0; l < NumLineDefs; l++)
	{
		LineDef *pLineDef = &LineDefs[l];
		SHORT start = pLineDef->start;
		SHORT end   = pLineDef->end;

		if (IsSelected (*list, start) )
		{
			// don't change a LineDef that has both ends on the same spot
			if ( !IsSelected (*list, end) && end != v)
				start = v;
		}
		else if ( IsSelected (*list, end))
		{
			// idem
			if (start != v)
				end = v;
		}

		// Copy back start end end
		pLineDef->start = start;
		pLineDef->end   = end;
	}

	// delete the Vertices (and some LineDefs too)
	DeleteObjects (OBJ_VERTEXES, list);
	MadeChanges = TRUE;
	MadeMapChanges = TRUE;
}



/*
   check if some vertices should be merged into one
*/

BOOL AutoMergeVertices (SelPtr *list)
{
	SelPtr ref, cur;
	BOOL   confirmed, redraw;
	BOOL   flipped, mergedone, isldend;
	SHORT  v, refv;
	SHORT  ld, sd;
	SHORT  oldnumld;

	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();

	confirmed = FALSE;
	redraw = FALSE;
	mergedone = FALSE;
	isldend = FALSE;

	//
	// first, check if two (or more) Vertices should be merged
	//
	WorkMessage ("AutoMerge: Merging Vertices ...");
	ref = *list;
	while (ref)
	{
		refv = ref->objnum;
		assert_vnum(refv);
		ref = ref->next;
		// check if there is a Vertex at the same position (same X and Y)
		for (v = 0; v < NumVertexes; v++)
		{
			Vertex *pv2 = &Vertexes[v];
			//BUG put this line here since the Vertexes ptr may have
			//    changed (when deleting somme vertices)
			Vertex *pv1 = &Vertexes[refv];

			// RP: Vertex position comparaison is more approximative
			//     (box of 7*7 instead of pure equality)
			if (v != refv &&
				(pv1->x >= pv2->x - 3) && (pv1->x <= pv2->x + 3) &&
				(pv1->y >= pv2->y - 3) && (pv1->y <= pv2->y + 3) )
			{
				redraw = TRUE;
				if ( confirmed ||
					 Expert    ||
					 Confirm ("Some Vertices occupy the same position.\n"
							  "Do you want to merge them into one ?") )
				{
					// don't ask for confirmation twice
					confirmed = TRUE;

					// merge the two vertices
					mergedone = TRUE;
					cur = NULL;
					SelectObject (&cur, refv);
					SelectObject (&cur, v);
					MergeVertices (&cur);

					// not useful but safer...
					// update the references in the selection list
					for (cur = *list; cur; cur = cur->next)
						if (cur->objnum > refv)
							cur->objnum = cur->objnum - 1;
					if (v > refv)
						v--;
					// the old Vertex has been deleted
					UnSelectObject (list, refv);
					// select the new Vertex instead
					if (!IsSelected (*list, v))
						SelectObject (list, v);
					break;
				}
				else
					goto AutoMergeEnd;
			}
		}
	}
	confirmed = FALSE;

	//
	// now, check if one or more Vertices are on a LineDef
	//
	WorkMessage ("AutoMerge: Spliting LineDefs ...");
	ref = *list;
	while (ref)
	{
		refv = ref->objnum;
		assert_vnum(refv);
		Vertex  *pCurV  = &Vertexes[refv];
		ref = ref->next;
		oldnumld = NumLineDefs;
		// check if this Vertex is on a LineDef
		for (ld = 0; ld < oldnumld; ld++)
		{
			LineDef *pCurLD = &LineDefs[ld];
			LineDef *pNewLD;

			if (pCurLD->start == refv || pCurLD->end == refv)
			{
				// one Vertex had a LineDef bound to it -- check it later
				isldend = TRUE;
			}
			else if (IsLineDefInside (ld, pCurV->x - 3, pCurV->y - 3,
										  pCurV->x + 3, pCurV->y + 3))
			{
				redraw = TRUE;
				if ( confirmed ||
					 Expert    ||
					 Confirm ("Some Vertices are on a LineDef. Do you want to "
							  "split the LineDef there ?") )
				{
					// don't ask for confirmation twice
					confirmed = TRUE;
					// split the LineDef
					mergedone = TRUE;
					InsertObject (OBJ_LINEDEFS, ld, 0, 0);
					//BUG: We've used InsertOjbect, so this assert may not be true
					// (because or Realloc in InsertObject)
					//assert(pCurLD == &LineDefs[ld]);
					pCurLD = &LineDefs[ld];
					pNewLD = &LineDefs[NumLineDefs - 1];

					pCurLD->end = refv;
					pNewLD->start = refv;
					sd = pCurLD->sidedef1;
					if (sd >= 0)
					{
						InsertObject(OBJ_SIDEDEFS, sd, 0, 0);
						pNewLD->sidedef1 = NumSideDefs - 1;
					}
					sd = pCurLD->sidedef2;
					if (sd >= 0)
					{
						InsertObject (OBJ_SIDEDEFS, sd, 0, 0);
						pNewLD->sidedef2 = NumSideDefs - 1;
					}
					MadeChanges = TRUE;
					MadeMapChanges = TRUE;
				}
				else
					goto AutoMergeEnd;
			}
		}
	}

	//
	// don't continue if this isn't necessary
	//
	if (isldend == FALSE || mergedone == FALSE)
		goto AutoMergeEnd;

	confirmed = FALSE;

	//
	// finally, test if two LineDefs are between the same pair of Vertices
	//
	WorkMessage ("AutoMerge: Merging LineDefs ...");
	for (v = 0; v < NumLineDefs - 1; v++)
	{

		for (ld = v + 1; ld < NumLineDefs; ld++)
		{
			LineDef *pLineDefLD = &LineDefs[ld];
			//BUG put this line here since the LineDefs ptr may have
			//    changed (when deleting somme LineDefs)
			LineDef *pLineDefV = &LineDefs[v];

			if ( (pLineDefV->start == pLineDefLD->start &&
				  pLineDefV->end   == pLineDefLD->end  ) ||
				 (pLineDefV->start == pLineDefLD->end   &&
				  pLineDefV->end   == pLineDefLD->start) )
			{
				redraw = TRUE;
				if ( confirmed ||
					 Expert    ||
					 Confirm ("Some LineDefs are superimposed. Do you want to "
							  "merge them into one ?") )
				{
					// don't ask for confirmation twice
					confirmed = TRUE;
					// test if the LineDefs have the same orientation
					flipped = (pLineDefV->start == pLineDefLD->end);

					// merge the two LineDefs
					if (pLineDefV->sidedef1 < 0)
					{
						if (flipped)
						{
							pLineDefV->sidedef1  = pLineDefLD->sidedef2;
							pLineDefLD->sidedef2 = -1;
						}
						else
						{
							pLineDefV->sidedef1  = pLineDefLD->sidedef1;
							pLineDefLD->sidedef1 = -1;
						}
					}
					if (pLineDefV->sidedef2 < 0)
					{
						if (flipped)
						{
							pLineDefV->sidedef2  = pLineDefLD->sidedef1;
							pLineDefLD->sidedef1 = -1;
						}
						else
						{
							pLineDefV->sidedef2  = pLineDefLD->sidedef2;
							pLineDefLD->sidedef2 = -1;
						}
					}
					if (pLineDefV->sidedef1 >= 0 &&
						pLineDefV->sidedef2 >= 0 &&
						(pLineDefV->flags & 0x04) == 0)
					{
						pLineDefV->flags |= 0x04;
					}
					// Delete LineDef 'ld'
					DeleteObject (OBJ_LINEDEFS, ld);
				}
			}
		}
	}

AutoMergeEnd:
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();
	return redraw;
}



/*
   split one or more LineDefs in two, adding new Vertices in the middle
*/

void SplitLineDefs (SelPtr obj)
{
	SelPtr cur;
	SHORT  vstart, vend, sd;

	// ObjectsNeeded( OBJ_LINEDEFS, 0);
	for (cur = obj; cur; cur = cur->next)
	{
		vstart = LineDefs[cur->objnum].start;
		vend   = LineDefs[cur->objnum].end;
		InsertObject(OBJ_VERTEXES, -1, (Vertexes[ vstart].x + Vertexes[ vend].x) / 2, (Vertexes[ vstart].y + Vertexes[ vend].y) / 2);
		InsertObject(OBJ_LINEDEFS, cur->objnum, 0, 0);
		LineDefs[cur->objnum].end = NumVertexes - 1;
		LineDefs[NumLineDefs - 1].start = NumVertexes - 1;
		sd = LineDefs[ cur->objnum].sidedef1;
		if (sd >= 0)
		{
			InsertObject( OBJ_SIDEDEFS, sd, 0, 0);
			// ObjectsNeeded( OBJ_LINEDEFS, 0);
			LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs - 1;
		}
		sd = LineDefs[ cur->objnum].sidedef2;
		if (sd >= 0)
		{
			InsertObject( OBJ_SIDEDEFS, sd, 0, 0);
			// ObjectsNeeded( OBJ_LINEDEFS, 0);
			LineDefs[ NumLineDefs - 1].sidedef2 = NumSideDefs - 1;
		}
	}
	MadeChanges = TRUE;
	MadeMapChanges = TRUE;
}



/*
   split a Sector in two, adding a new LineDef between the two Vertices
*/

void SplitSector (SHORT vertex1, SHORT vertex2)
{
   SelPtr llist;
   SHORT    curv, s, l, sd;
   char   msg1[ 80], msg2[ 80];

   /* check if there is a Sector between the two Vertices (in the middle) */
   s = GetCurObject( OBJ_SECTORS, Vertexes[ vertex1].x, Vertexes[ vertex1].y, Vertexes[ vertex2].x, Vertexes[ vertex2].y);
   if (s < 0)
   {
	  Beep();
	  Notify ("There is no Sector between Vertex #%d and Vertex #%d", vertex1, vertex2);
	  return;
   }
   /* check if there is a closed path from vertex1 to vertex2, along the edge of the Sector s */
   // ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
   llist = NULL;
   curv = vertex1;
   while (curv != vertex2)
   {
	  for (l = 0; l < NumLineDefs; l++)
	  {
	 sd = LineDefs[ l].sidedef1;
	 if (sd >= 0 && SideDefs[ sd].sector == s && LineDefs[ l].start == curv)
	 {
		curv = LineDefs[ l].end;
		SelectObject( &llist, l);
		break;
	 }
	 sd = LineDefs[ l].sidedef2;
	 if (sd >= 0 && SideDefs[ sd].sector == s && LineDefs[ l].end == curv)
	 {
		curv = LineDefs[ l].start;
		SelectObject( &llist, l);
		break;
	 }
	  }
	  if (l >= NumLineDefs)
	  {
	 Beep();
	 wsprintf( msg1, "Cannot find a closed path from Vertex #%d to Vertex #%d", vertex1, vertex2);
	 if (curv == vertex1)
		wsprintf( msg2, "There is no SideDef starting from Vertex #%d on Sector #%d", vertex1, s);
	 else
		wsprintf( msg2, "Check if Sector #%d is closed (cannot go past Vertex #%d)", s, curv);
	 Notify("%s.\n %s", msg1, msg2);
	 ForgetSelection( &llist);
	 return;
	  }
	  if (curv == vertex1)
	  {
	 Beep();
	 Notify("Vertex #%d is not on the same Sector (#%d) as Vertex #%d",
			vertex2, s, vertex1);
	 ForgetSelection( &llist);
	 return;
	  }
   }
   /* now, the list of LineDefs for the new Sector is in llist */

   /* add the new Sector, LineDef and SideDefs */
   InsertObject( OBJ_SECTORS, s, 0, 0);
   InsertObject( OBJ_LINEDEFS, -1, 0, 0);
   LineDefs[ NumLineDefs - 1].start = vertex1;
   LineDefs[ NumLineDefs - 1].end = vertex2;
   LineDefs[ NumLineDefs - 1].flags = 4;
   InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
   SideDefs[ NumSideDefs - 1].sector = s;
   strncpy( SideDefs[ NumSideDefs - 1].tex3, "-", MAX_BITMAPNAME);
   InsertObject( OBJ_SIDEDEFS, -1, 0, 0);
   strncpy( SideDefs[ NumSideDefs - 1].tex3, "-", MAX_BITMAPNAME);
   // ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
   LineDefs[ NumLineDefs - 1].sidedef1 = NumSideDefs - 2;
   LineDefs[ NumLineDefs - 1].sidedef2 = NumSideDefs - 1;

   /* bind all LineDefs in llist to the new Sector */
   while (llist)
   {
	  sd = LineDefs[ llist->objnum].sidedef1;
	  if (sd < 0 || SideDefs[ sd].sector != s)
	 sd = LineDefs[ llist->objnum].sidedef2;
	  SideDefs[ sd].sector = NumSectors - 1;
	  UnSelectObject( &llist, llist->objnum);
   }

   /* second check... uselful for Sectors within Sectors */
   // ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
   for (l = 0; l < NumLineDefs; l++)
   {
	  sd = LineDefs[ l].sidedef1;
	  if (sd >= 0 && SideDefs[ sd].sector == s)
	  {
		 curv = GetOppositeSector( l, TRUE);
		 // ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		 if (curv == NumSectors - 1)
			SideDefs[ sd].sector = NumSectors - 1;
	  }
	  sd = LineDefs[ l].sidedef2;
	  if (sd >= 0 && SideDefs[ sd].sector == s)
	  {
		 curv = GetOppositeSector( l, FALSE);
		 // ObjectsNeeded( OBJ_LINEDEFS, OBJ_SIDEDEFS, 0);
		 if (curv == NumSectors - 1)
			SideDefs[ sd].sector = NumSectors - 1;
	  }
   }

   MadeChanges = TRUE;
   MadeMapChanges = TRUE;
}



/*
   split two LineDefs, then split the Sector and add a new LineDef between the new Vertices
*/

void SplitLineDefsAndSector (SHORT linedef1, SHORT linedef2)
{
   SelPtr llist;
   SHORT    s1, s2, s3, s4;

   /* check if the two LineDefs are adjacent to the same Sector */
   // ObjectsNeeded( OBJ_LINEDEFS, 0);
   s1 = LineDefs[ linedef1].sidedef1;
   s2 = LineDefs[ linedef1].sidedef2;
   s3 = LineDefs[ linedef2].sidedef1;
   s4 = LineDefs[ linedef2].sidedef2;
   // ObjectsNeeded( OBJ_SIDEDEFS, 0);
   if (s1 >= 0)
	  s1 = SideDefs[ s1].sector;
   if (s2 >= 0)
	  s2 = SideDefs[ s2].sector;
   if (s3 >= 0)
	  s3 = SideDefs[ s3].sector;
   if (s4 >= 0)
	  s4 = SideDefs[ s4].sector;
   if ((s1 < 0 || (s1 != s3 && s1 != s4)) && (s2 < 0 || (s2 != s3 && s2 != s4)))
   {
	  Beep();
	  Notify("LineDefs #%d and #%d are not adjacent to the same Sector",
			 linedef1, linedef2);
	  return;
   }
   /* split the two LineDefs and create two new Vertices */
   llist = NULL;
   SelectObject( &llist, linedef1);
   SelectObject( &llist, linedef2);
   SplitLineDefs( llist);
   ForgetSelection( &llist);
   /* split the Sector and create a LineDef between the two Vertices */
   SplitSector( NumVertexes - 1, NumVertexes - 2);
}



/*
   merge two or more Sectors into one
*/

void MergeSectors (SelPtr *slist)
{
  SelPtr cur;
  SHORT    n, olds, news;

  /* save the first Sector number */
  news = (*slist)->objnum;
  UnSelectObject( slist, news);
  // ObjectsNeeded( OBJ_SIDEDEFS, 0);

  /* change all SideDefs references to the other Sectors */
  for (cur = *slist; cur; cur = cur->next)
  {
	 olds = cur->objnum;
	 for (n = 0; n < NumSideDefs; n++)
	 {
	if (SideDefs[ n].sector == olds)
	   SideDefs[ n].sector = news;
	 }
  }

  /* delete the Sectors */
  DeleteObjects( OBJ_SECTORS, slist);

  /* the returned list contains only the first Sector */
  SelectObject( slist, news);
}



/*
   delete one or several two-sided LineDefs and join the two Sectors
*/

void DeleteLineDefsJoinSectors (SelPtr *ldlist)
{
   SelPtr cur, slist;
   SHORT  sd1, sd2;
   SHORT  s1, s2;

   /* first, do the tests for all LineDefs */
   for (cur = *ldlist; cur; cur = cur->next)
   {
	  // ObjectsNeeded( OBJ_LINEDEFS, 0);
	  sd1 = LineDefs[ cur->objnum].sidedef1;
	  sd2 = LineDefs[ cur->objnum].sidedef2;
	  if (sd1 < 0 || sd2 < 0)
	  {
		 Beep();
		 Notify("ERROR: LineDef #%d has only one side", cur->objnum);
		 return;
	  }
	  // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	  s1 = SideDefs[ sd1].sector;
	  s2 = SideDefs[ sd2].sector;
	  if (s1 < 0 || s2 < 0)
	  {
		 Beep();
		 Notify("ERROR: LineDef #%d has two sides, but one side is not bound "
				"to any Sector", cur->objnum);
		 return;
	  }
   }

   /* then join the Sectors and delete the LineDefs */
   for (cur = *ldlist; cur; cur = cur->next)
   {
	  // ObjectsNeeded( OBJ_LINEDEFS, 0);
	  sd1 = LineDefs[ cur->objnum].sidedef1;
	  sd2 = LineDefs[ cur->objnum].sidedef2;
	  // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	  s1 = SideDefs[ sd1].sector;
	  s2 = SideDefs[ sd2].sector;
	  slist = NULL;
	  SelectObject( &slist, s2);
	  SelectObject( &slist, s1);
	  MergeSectors( &slist);
	  ForgetSelection( &slist);
   }
   DeleteObjects( OBJ_LINEDEFS, ldlist);
}




/*
   turn a Sector into a door: change the LineDefs and SideDefs
*/

void MakeDoorFromSector (SHORT sector)
{
   SHORT  sd1, sd2;
   SHORT  n, s;
   SelPtr ldok, ldflip, ld1s;

   ldok = NULL;
   ldflip = NULL;
   ld1s = NULL;
   s = 0;
   /* build lists of LineDefs that border the Sector */
   for (n = 0; n < NumLineDefs; n++)
   {
	  // ObjectsNeeded( OBJ_LINEDEFS, 0);
	  sd1 = LineDefs[ n].sidedef1;
	  sd2 = LineDefs[ n].sidedef2;
	  if (sd1 >= 0 && sd2 >= 0)
	  {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (SideDefs[ sd2].sector == sector)
	 {
		SelectObject( &ldok, n); /* already ok */
		s++;
	 }
	 if (SideDefs[ sd1].sector == sector)
	 {
		SelectObject( &ldflip, n); /* must be flipped */
		s++;
	 }
	  }
	  else if (sd1 >= 0 && sd2 < 0)
	  {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (SideDefs[ sd1].sector == sector)
		SelectObject( &ld1s, n); /* wall (one-sided) */
	  }
   }
   /* a normal door has two sides... */
   if (s < 2)
   {
	  Beep();
	  Notify("The door must be connected to two other Sectors.");
	  ForgetSelection( &ldok);
	  ForgetSelection( &ldflip);
	  ForgetSelection( &ld1s);
	  return;
   }
   if ((s > 2) &&
		!(Expert || Confirm ("The door will have more than two sides.\n"
							 "Do you still want to create it?") ))
   {
	  ForgetSelection( &ldok);
	  ForgetSelection( &ldflip);
	  ForgetSelection( &ld1s);
	  return;
   }
   /* flip the LineDefs that have the wrong orientation */
   if (ldflip != NULL)
	  FlipLineDefs( ldflip, TRUE);
   /* merge the two selection lists */
   while (ldflip != NULL)
   {
	  if (!IsSelected( ldok, ldflip->objnum))
	 SelectObject( &ldok, ldflip->objnum);
	  UnSelectObject( &ldflip, ldflip->objnum);
   }
   /* change the LineDefs and SideDefs */
   while (ldok != NULL)
   {
	  /* give the "normal door" type and flags to the LineDef */
	  // ObjectsNeeded( OBJ_LINEDEFS, 0);
	  n = ldok->objnum;
	  LineDefs[ n].type = 1;
	  LineDefs[ n].flags = 0x04;
	  sd1 = LineDefs[ n].sidedef1; /* outside */
	  sd2 = LineDefs[ n].sidedef2; /* inside */
	  /* adjust the textures for the SideDefs */
	  // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	  if (strncmp( SideDefs[ sd1].tex3, "-", MAX_BITMAPNAME))
	  {
	 if (!strncmp( SideDefs[ sd1].tex1, "-", MAX_BITMAPNAME))
		strncpy( SideDefs[ sd1].tex1, SideDefs[ sd1].tex3, MAX_BITMAPNAME);
	 strncpy( SideDefs[ sd1].tex3, "-", 8);
	  }
	  if (!strncmp( SideDefs[ sd1].tex1, "-", MAX_BITMAPNAME))
	 strncpy( SideDefs[ sd1].tex1, "BIGDOOR2", MAX_BITMAPNAME);
	  strncpy( SideDefs[ sd2].tex3, "-", MAX_BITMAPNAME);
	  UnSelectObject( &ldok, n);
   }
   while (ld1s != NULL)
   {
	  /* give the "door side" flags to the LineDef */
	  // ObjectsNeeded( OBJ_LINEDEFS, 0);
	  n = ld1s->objnum;
	  LineDefs[ n].flags = 0x11;
	  sd1 = LineDefs[ n].sidedef1;
	  /* adjust the textures for the SideDef */
	  // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	  if (!strncmp( SideDefs[ sd1].tex3, "-", MAX_BITMAPNAME))
	 strncpy( SideDefs[ sd1].tex3, "DOORTRAK", MAX_BITMAPNAME);
	  strncpy( SideDefs[ sd1].tex1, "-", MAX_BITMAPNAME);
	  strncpy( SideDefs[ sd1].tex2, "-", MAX_BITMAPNAME);
	  UnSelectObject( &ld1s, n);
   }
   /* adjust the ceiling height */
   // ObjectsNeeded( OBJ_SECTORS, 0);
   Sectors[ sector].ceilh = Sectors[ sector].floorh;
}



/*
   turn a Sector into a lift: change the LineDefs and SideDefs
*/

void MakeLiftFromSector (SHORT sector)
{
   SHORT  sd1, sd2;
   SHORT  n, s, tag;
   SelPtr ldok, ldflip, ld1s;
   SelPtr sect, curs;
   SHORT  minh, maxh;

   ldok = NULL;
   ldflip = NULL;
   ld1s = NULL;
   sect = NULL;
   /* build lists of LineDefs that border the Sector */
   for (n = 0; n < NumLineDefs; n++)
   {
      // ObjectsNeeded( OBJ_LINEDEFS, 0);
	  sd1 = LineDefs[ n].sidedef1;
	  sd2 = LineDefs[ n].sidedef2;
      if (sd1 >= 0 && sd2 >= 0)
      {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (SideDefs[ sd2].sector == sector)
	 {
		SelectObject( &ldok, n); /* already ok */
	    s = SideDefs[ sd1].sector;
	    if (s != sector && !IsSelected( sect, s))
	       SelectObject( &sect, s);
	 }
	 if (SideDefs[ sd1].sector == sector)
	 {
	    SelectObject( &ldflip, n); /* will be flipped */
	    s = SideDefs[ sd2].sector;
		if (s != sector && !IsSelected( sect, s))
	       SelectObject( &sect, s);
	 }
	  }
      else if (sd1 >= 0 && sd2 < 0)
      {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (SideDefs[ sd1].sector == sector)
	    SelectObject( &ld1s, n); /* wall (one-sided) */
      }
   }
   /* there must be a way to go on the lift... */
   if (sect == NULL)
   {
	  Beep();
	  Notify("The lift must be connected to at least one other Sector.");
      ForgetSelection( &ldok);
      ForgetSelection( &ldflip);
      ForgetSelection( &ld1s);
      return;
   }
   /* flip the LineDefs that have the wrong orientation */
   if (ldflip != NULL)
	  FlipLineDefs( ldflip, TRUE);
   /* merge the two selection lists */
   while (ldflip != NULL)
   {
      if (!IsSelected( ldok, ldflip->objnum))
	 SelectObject( &ldok, ldflip->objnum);
      UnSelectObject( &ldflip, ldflip->objnum);
   }

   /* find a free tag number */
   tag = FindFreeTag();

   /* find the minimum and maximum altitudes */
   // ObjectsNeeded( OBJ_SECTORS, 0);
   minh = 32767;
   maxh = -32767;
   for (curs = sect; curs; curs = curs->next)
   {
	  if (Sectors[ curs->objnum].floorh < minh)
	 minh = Sectors[ curs->objnum].floorh;
      if (Sectors[ curs->objnum].floorh > maxh)
	 maxh = Sectors[ curs->objnum].floorh;
   }
   ForgetSelection( &sect);

   /* change the lift's floor height if necessary */
   if (Sectors[ sector].floorh < maxh)
      Sectors[ sector].floorh = maxh;

   /* change the lift's ceiling height if necessary */
   if (Sectors[ sector].ceilh < maxh + 56)
      Sectors[ sector].ceilh = maxh + 56;

   /* assign the new tag number to the lift */
   Sectors[ sector].tag = tag;

   /* change the LineDefs and SideDefs */
   while (ldok != NULL)
   {
      /* give the "lower lift" type and flags to the LineDef */
      // ObjectsNeeded( OBJ_LINEDEFS, 0);
      n = ldok->objnum;
      LineDefs[ n].type = 62; /* lower lift (switch) */
      LineDefs[ n].flags = 0x04;
      LineDefs[ n].tag = tag;
      sd1 = LineDefs[ n].sidedef1; /* outside */
      sd2 = LineDefs[ n].sidedef2; /* inside */
      /* adjust the textures for the SideDef visible from the outside */
	  // ObjectsNeeded( OBJ_SIDEDEFS, 0);
      if (strncmp( SideDefs[ sd1].tex3, "-", MAX_BITMAPNAME))
      {
	 if (!strncmp( SideDefs[ sd1].tex2, "-", MAX_BITMAPNAME))
		strncpy( SideDefs[ sd1].tex2, SideDefs[ sd1].tex3, MAX_BITMAPNAME);
	 strncpy( SideDefs[ sd1].tex3, "-", 8);
      }
      if (!strncmp( SideDefs[ sd1].tex2, "-", MAX_BITMAPNAME))
	 strncpy( SideDefs[ sd1].tex2, "SHAWN2", MAX_BITMAPNAME);
      /* adjust the textures for the SideDef visible from the lift */
      strncpy( SideDefs[ sd2].tex3, "-", MAX_BITMAPNAME);
      s = SideDefs[ sd1].sector;
	  // ObjectsNeeded( OBJ_SECTORS, 0);
      if (Sectors[ s].floorh > minh)
      {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (strncmp( SideDefs[ sd2].tex3, "-", MAX_BITMAPNAME))
	 {
	    if (!strncmp( SideDefs[ sd2].tex2, "-", MAX_BITMAPNAME))
		   strncpy( SideDefs[ sd2].tex2, SideDefs[ sd1].tex3, MAX_BITMAPNAME);
	    strncpy( SideDefs[ sd2].tex3, "-", MAX_BITMAPNAME);
	 }
	 if (!strncmp( SideDefs[ sd2].tex2, "-", MAX_BITMAPNAME))
	    strncpy( SideDefs[ sd2].tex2, "SHAWN2", MAX_BITMAPNAME);
      }
      else
      {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 strncpy( SideDefs[ sd2].tex2, "-", MAX_BITMAPNAME);
      }
      strncpy( SideDefs[ sd2].tex3, "-", MAX_BITMAPNAME);
      // ObjectsNeeded( OBJ_SECTORS, 0);
	  /* if the ceiling of the Sector is lower than that of the lift */
      if (Sectors[ s].ceilh < Sectors[ sector].ceilh)
	  {
	 // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	 if (strncmp( SideDefs[ sd2].tex1, "-", MAX_BITMAPNAME))
	    strncpy( SideDefs[ sd2].tex1, DefaultUpperTexture, MAX_BITMAPNAME);
	  }
      // ObjectsNeeded( OBJ_SECTORS, 0);
      /* if the floor of the Sector is above the lift */
      if (Sectors[ s].floorh >= Sectors[ sector].floorh)
      {
	 // ObjectsNeeded( OBJ_LINEDEFS, 0);
	 LineDefs[ n].type = 88; /* lower lift (walk through) */
	 /* flip it, just for fun */
	 curs = NULL;
	 SelectObject( &curs, n);
	 FlipLineDefs( curs, TRUE);
	 ForgetSelection( &curs);
      }
      /* done with this LineDef */
      UnSelectObject( &ldok, n);
   }

   while (ld1s != NULL)
   {
	  /* these are the lift walls (one-sided) */
      // ObjectsNeeded( OBJ_LINEDEFS, 0);
      n = ld1s->objnum;
      LineDefs[ n].flags = 0x01;
      sd1 = LineDefs[ n].sidedef1;
      /* adjust the textures for the SideDef */
      // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	  if (!strncmp( SideDefs[ sd1].tex3, "-", MAX_BITMAPNAME))
	 strncpy( SideDefs[ sd1].tex3, DefaultWallTexture, MAX_BITMAPNAME);
      strncpy( SideDefs[ sd1].tex1, "-", MAX_BITMAPNAME);
      strncpy( SideDefs[ sd1].tex2, "-", MAX_BITMAPNAME);
      UnSelectObject( &ld1s, n);
   }
}



/*
   get the absolute height from which the textures are drawn
*/

SHORT GetTextureRefHeight (SHORT sidedef)
{
   SHORT l, sector;
   SHORT otherside;

   /* find the SideDef on the other side of the LineDef, if any */
   // ObjectsNeeded( OBJ_LINEDEFS, 0);
   for (l = 0; l < NumLineDefs; l++)
   {
	  if (LineDefs[ l].sidedef1 == sidedef)
	  {
	 otherside = LineDefs[ l].sidedef2;
	 break;
	  }
	  if (LineDefs[ l].sidedef2 == sidedef)
	  {
	 otherside = LineDefs[ l].sidedef1;
	 break;
	  }
   }
   /* get the Sector number */
   // ObjectsNeeded( OBJ_SIDEDEFS, 0);
   sector = SideDefs[ sidedef].sector;
   /* if the upper texture is displayed, then the reference is taken from the other Sector */
   if (otherside >= 0)
   {
	  l = SideDefs[ otherside].sector;
	  if (l > 0)
	  {
	 // ObjectsNeeded( OBJ_SECTORS, 0);
	 if (Sectors[ l].ceilh < Sectors[ sector].ceilh && Sectors[ l].ceilh > Sectors[ sector].floorh)
		sector = l;
	  }
   }
   /* return the altitude of the ceiling */
   // ObjectsNeeded( OBJ_SECTORS, 0);
   if (sector >= 0)
	  return Sectors[ sector].ceilh; /* textures are drawn from the ceiling down */
   else
	  return 0; /* yuck! */
}



/*
   Align all textures for the given SideDefs

   Note from RQ:
	  This function should be improved!
	  But what should be improved first is the way the SideDefs are selected.
	  It is stupid to change both sides of a wall when only one side needs
	  to be changed.  But with the current selection method, there is no
	  way to select only one side of a two-sided wall.
*/

void AlignTexturesY (SelPtr *sdlist)
{
   SHORT h, refh;

   if (*sdlist == NULL)
	  return;

   /* get the reference height from the first SideDef */
   refh = GetTextureRefHeight( (*sdlist)->objnum);
   // ObjectsNeeded( OBJ_SIDEDEFS, 0);
   SideDefs[ (*sdlist)->objnum].yoff = 0;
   UnSelectObject( sdlist, (*sdlist)->objnum);

   /* adjust Y offset in all other SideDefs */
   while (*sdlist != NULL)
   {
	  h = GetTextureRefHeight( (*sdlist)->objnum);
	  // ObjectsNeeded( OBJ_SIDEDEFS, 0);
	  SideDefs[ (*sdlist)->objnum].yoff = (refh - h) % 128;
	  UnSelectObject( sdlist, (*sdlist)->objnum);
   }
   MadeChanges = TRUE;
}



/*
	Function is to align all highlighted textures in the X-axis

	Note from RJH:
		LineDefs highlighted are read off in reverse order of highlighting.
		The '*sdlist' is in the reverse order of the above mentioned LineDefs
		i.e. the first LineDef SideDefs you highlighted will be processed first.

	Note from RQ:
		See also the note for the previous function.

	Note from RJH:
		For the menu for aligning textures 'X' NOW operates upon the fact that
		ALL the SIDEDEFS from the selected LINEDEFS are in the *SDLIST, 2nd
		SideDef is first, 1st SideDef is 2nd). Aligning textures X now does
		SIDEDEF 1's and SIDEDEF 2's.  If the selection process is changed,
		the following needs to be altered radically.
*/

// wall_part: 1 = upper, 2 = lower, 3 = normal

void AlignTexturesX (SelPtr *sdlist, SHORT type_sd, SHORT type_tex, SHORT type_off, int wall_part)
{
	// Check parameters
	assert_bound (type_sd, 1, 2);
	assert_bound (type_tex, 0, 1);
	assert_bound (type_off, 0, 1);
	assert_bound (wall_part, 1, 3);

	SHORT  ldef;			/* linedef number */
	SHORT  sd1;         	/* current SideDef in *sdlist */
	SHORT  vert1, vert2;	/* vertex 1 and 2 for the linedef under scrutiny */
	SHORT  xoffset;			/* xoffset accumulator */
	static SHORT useroffset = 0;      /* user input offset for first input */
	SHORT  texlength;       /* the length of texture to format to */
	SHORT  length;			/* length of linedef under scrutiny */
	SHORT  dummy;			/* holds useless data */

	vert1   = -1;
	vert2   = -1;		/* first time round the while loop the -1 value is needed */
	texlength  = 0;
	xoffset    = 0;

	ldef = 0;
	sd1 = (*sdlist) ->objnum;

	/* throw out all 2nd SideDefs until a 1st is found */
	if(type_sd == 1)
	{
		while((*sdlist)!=NULL && LineDefs[ldef].sidedef1!=sd1 && ldef<=NumLineDefs)
		{
			ldef++;
			if(LineDefs[ldef].sidedef2 == sd1)
			{
				UnSelectObject(sdlist, (*sdlist)->objnum);
				//RP moved from two lines after (needed because
				//   "sd1 = (*sdlist) ->objnum" may cause a GPF if
				//   (*sdlist) == NULL !!!
				if((*sdlist)==NULL)
				{
					Notify ("No first SideDef found!");
					return;
				}
				sd1 = (*sdlist) ->objnum;
				ldef = 0;
			}
		}
	}

	/* throw out all 1st SideDefs untill a 2nd is found */
	if(type_sd == 2)
	{
		while(LineDefs[ldef].sidedef2!=sd1 && ldef<=NumLineDefs)
		{
			ldef++;
			if(LineDefs[ldef].sidedef1 == sd1)
			{
				UnSelectObject(sdlist, (*sdlist)->objnum);
				//RP moved from two lines after (needed because
				//   "sd1 = (*sdlist) ->objnum" may cause a GPF if
				//   (*sdlist) == NULL !!!
				if((*sdlist) == NULL)
				{
					Notify ("No second SideDef found!");
					return;
				}
				sd1 = (*sdlist) ->objnum;
				ldef = 0;
			}
		}
	}

	/* get texture number of the SideDef in the *sdlist) */
	// Which texture to use depends on wall_part
	int texnum;
	switch (wall_part)
	{
	case 1: texnum = SideDefs[(*sdlist)->objnum].type1; break;
	case 2: texnum = SideDefs[(*sdlist)->objnum].type2; break;
	case 3: texnum = SideDefs[(*sdlist)->objnum].type3; break;
	}

	/* test if there is a texture there */
	if (texnum == 0)
	{
	   Beep();
	   Notify("No texture for SideDef #%d.", (*sdlist)->objnum);
	   return;
	}

	GetWallTextureSize (&texlength, &dummy, texnum); /* clunky, but it works */

	/* get initial offset to use (if requrired) */
	if (type_off == 1)
	{
		char *Title = "Offset alignment";
		char Prompt[80];
		char Buf[6];	// Init to 0 and Keep between calls
		int BufSize = 6;
		TRangeValidator *pValid = new TRangeValidator (0, texlength);
		TWindow *pWnd = ((TApplication *)::Module)->GetMainWindow();
		wsprintf (Prompt, "Enter initial offset between 0 and %d:", texlength);
		wsprintf (Buf, "%d", useroffset);

		if ( TInputDialog (pWnd, Title, Prompt,
						   Buf, BufSize, 0, pValid).Execute() != IDOK )
		{
			return;
		}

		useroffset = (SHORT)atoi(Buf);
	}

	/* main processing loop */
	while (*sdlist != NULL)
	{
		ldef = 0;
		sd1 = (*sdlist)->objnum;

		if(type_sd == 1) /* throw out all 2nd SideDefs untill a 1st is found */
		{
			while(LineDefs[ldef].sidedef1!=sd1 && ldef<=NumLineDefs)
			{
				ldef++;
				if(LineDefs[ldef].sidedef2 == sd1)
				{
					UnSelectObject(sdlist, (*sdlist)->objnum);
					sd1 = (*sdlist) ->objnum;
					ldef = 0;
					if((*sdlist) == NULL)
						return;
				}
			}
		}

		if(type_sd == 2) /* throw out all 1st SideDefs untill a 2nd is found */
		{
			while(LineDefs[ldef].sidedef2!=sd1 && ldef<=NumLineDefs)
			{
				ldef++;
				if(LineDefs[ldef].sidedef1 == sd1)
				{
					UnSelectObject(sdlist, (*sdlist)->objnum);
					sd1 = (*sdlist) ->objnum;
					ldef = 0;
					if((*sdlist) == NULL)
						return;
				}
			}
		}

		if(type_tex == 1) /* do we test for same textures for the SideDef in question?? */
		{
			// ObjectsNeeded( OBJ_SIDEDEFS, 0);
			if (SideDefs[ (*sdlist)->objnum].type3 != texnum)
			{
			   Beep();
			   Notify("No texture for SideDef #%d.", (*sdlist)->objnum);
			   return;
			}
		}

		sd1 = (*sdlist)->objnum;
		ldef = 0;
		/* find out which LineDef holds that SideDef */
		if(type_sd == 1)
		{
			while (LineDefs[ ldef].sidedef1 != sd1 && ldef < NumLineDefs)
				ldef++;
		}
		else
		{
			while (LineDefs[ ldef].sidedef2 != sd1 && ldef < NumLineDefs)
				ldef++;
		}
		vert1 = LineDefs[ldef].start;

		/* test for linedef highlight continuity */
		if (vert1 != vert2 && vert2 != -1)
		{
			Beep();
			/*
			Notify("SideDef #%d is not contiguous with the previous SideDef, "
				   "please reselect.", (*sdlist)->objnum);
			*/
			Notify("LineDef #%d is not contiguous with the previous LineDef, "
				   "please reselect.", ldef);
			return;
		}

		/* is this the first time round here */
		if (vert1 != vert2)
		{
			/* do we have an initial offset ? */
			if (type_off == 1)
			{
				SideDefs[sd1].xoff = useroffset;
				xoffset = useroffset;
			}
			else
				SideDefs[sd1].xoff = 0;
		}
		else		/* put new xoffset into the SideDef */
			SideDefs[sd1].xoff = xoffset;

		/* calculate length of LineDef */
		vert2 = LineDefs[ldef].end;
		length = ComputeDist (Vertexes[vert2].x - Vertexes[vert1].x,
							  Vertexes[vert2].y - Vertexes[vert1].y);

		xoffset += length;
		/* remove multiples of texlength from xoffset */
		xoffset = xoffset % texlength;
		/* move to next object in selected list */
		UnSelectObject (sdlist, (*sdlist)->objnum);
	}
	MadeChanges = TRUE;
}



/*
   Distribute sector floor heights
*/

void DistributeSectorFloors (SelPtr obj)
{
   SelPtr cur;
   SHORT  n, num, floor1h, floor2h;


   num = 0;
   for (cur = obj; cur->next; cur = cur->next)
	  num++;

   floor1h = Sectors[ obj->objnum].floorh;
   floor2h = Sectors[ cur->objnum].floorh;

   n = 0;
   for (cur = obj; cur; cur = cur->next)
   {
	  Sectors[ cur->objnum].floorh = floor1h + n * (floor2h - floor1h) / num;
	  n++;
   }
   MadeChanges = TRUE;
}



/*
   Distribute sector ceiling heights
*/

void DistributeSectorCeilings (SelPtr obj)
{
   SelPtr cur;
   SHORT  n, num, ceil1h, ceil2h;

   num = 0;
   for (cur = obj; cur->next; cur = cur->next)
	  num++;

   ceil1h = Sectors[ obj->objnum].ceilh;
   ceil2h = Sectors[ cur->objnum].ceilh;

   n = 0;
   for (cur = obj; cur; cur = cur->next)
   {
	  Sectors[ cur->objnum].ceilh = ceil1h + n * (ceil2h - ceil1h) / num;
	  n++;
   }
   MadeChanges = TRUE;
}


/*
  center the map around the object and zoom in if necessary
*/
void GoToObject( int objtype, SHORT objnum)
{
	SHORT   xpos, ypos;
	SHORT   xpos2, ypos2;
	SHORT   n;
	SHORT   sd1, sd2;
	float oldscale;

	GetObjectCoords (objtype, objnum, &xpos, &ypos);

	// Set OrigX and OrigX to xpos and ypos
	OrigX = xpos;
	OrigY = ypos;
	oldscale = Scale;

	/* zoom in until the object can be selected */
	// RP: Don't do it, since we dit not move mouse pointer
	//     in window center (moving the mouse pointer is not 'politicaly'
	//     correct in Windows)
	/*
	while (Scale < ScaleMax &&
		   GetCurObject( objtype,
						 MAPX( PointerX - 4), MAPY( PointerY - 4),
						 MAPX( PointerX + 4), MAPY( PointerY + 4)) != objnum)
	{
		IncScale();
	}
	*/

	/* Special case for Sectors: if several Sectors are one inside another, then    */
	/* zooming in on the center won't help.  So I choose a LineDef that borders the */
	/* Sector, move a few pixels towards the inside of the Sector, then zoom in.    */
	if ( objtype == OBJ_SECTORS &&
		 GetCurObject (OBJ_SECTORS, OrigX, OrigY, OrigX, OrigY) != objnum )
	{
		/* restore the Scale */
		Scale = oldscale;
		for (n = 0; n < NumLineDefs; n++)
		{
			//   ObjectsNeeded( OBJ_LINEDEFS, 0);
			sd1 = LineDefs[ n].sidedef1;
			sd2 = LineDefs[ n].sidedef2;
			//  ObjectsNeeded( OBJ_SIDEDEFS, 0);
			if (sd1 >= 0 && SideDefs[ sd1].sector == objnum)
				break;
			if (sd2 >= 0 && SideDefs[ sd2].sector == objnum)
				break;
		}
		if (n < NumLineDefs)
		{
			GetObjectCoords( OBJ_LINEDEFS, n, &xpos2, &ypos2);
			n = ComputeDist( abs( xpos - xpos2), abs( ypos - ypos2)) / 7;
			if (n <= 1)
				n = 2;
			xpos = xpos2 + (xpos - xpos2) / n;
			ypos = ypos2 + (ypos - ypos2) / n;
			OrigX = xpos;
			OrigY = ypos;

			/* zoom in until the sector can be selected */
			// RP: Don't do it, since we dit not move mouse pointer
			//     in window center (moving the mouse pointer is not 'politicaly'
			//     correct in Windows
			/*
			while (Scale < ScaleMax &&
				   GetCurObject( OBJ_SECTORS,
								 OrigX, OrigY, OrigX, OrigY) != objnum)
			{
				IncScale();
			}
			*/
		}
	}
}



/*
   insert a rectangle object at given position
*/

void InsertRectangle (SHORT xpos, SHORT ypos, SHORT width, SHORT height)
{
	SHORT sector;

	// are we inside a Sector?
	sector = GetCurObject (OBJ_SECTORS, xpos, ypos, xpos, ypos);

	// Setup width and position of rectangle
	if (width < 8)		width = 8;
	if (height < 8)		height = 8;
	xpos -= width / 2;
	ypos -= height / 2;

	// Insert the vertices
	InsertObject (OBJ_VERTEXES, -1, xpos,         ypos);
	InsertObject (OBJ_VERTEXES, -1, xpos + width, ypos);
	InsertObject (OBJ_VERTEXES, -1, xpos + width, ypos + height);
	InsertObject (OBJ_VERTEXES, -1, xpos,         ypos + height);

	// Insert a new sector
	if (sector < 0)
		InsertObject (OBJ_SECTORS, -1, 0, 0);

	// Insert the LineDefs and SideDefs 1
	for (SHORT n = 0; n < 4; n++)
	{
		InsertObject (OBJ_LINEDEFS, -1, 0, 0);
		InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
		LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs-1;
		if (sector >= 0)
			SideDefs[NumSideDefs - 1].sector = sector;
	}

	// Setup start and end vertices of LineDefs
	if (sector >= 0)
	{
		LineDef *pLD = &LineDefs[NumLineDefs - 4];
		pLD->start = NumVertexes - 4;
		pLD->end   = NumVertexes - 3;		  pLD++;
		pLD->start = NumVertexes - 3;
		pLD->end   = NumVertexes - 2;         pLD++;
		pLD->start = NumVertexes - 2;
		pLD->end   = NumVertexes - 1;         pLD++;
		pLD->start = NumVertexes - 1;
		pLD->end   = NumVertexes - 4;
	}
	else
	{
		LineDef *pLD = &LineDefs[NumLineDefs - 4];

		pLD->start = NumVertexes - 1;
		pLD->end   = NumVertexes - 2;		  pLD++;
		pLD->start = NumVertexes - 2;
		pLD->end   = NumVertexes - 3;		  pLD++;
		pLD->start = NumVertexes - 3;
		pLD->end   = NumVertexes - 4;		  pLD++;
		pLD->start = NumVertexes - 4;
		pLD->end   = NumVertexes - 1;
	}
}


/*
   insert a rectangle object at given position
*/

void InsertPolygon (SHORT xpos, SHORT ypos, SHORT nsides, SHORT radius)
{
	SHORT sector;
	SHORT n;

	// are we inside a Sector?
	sector = GetCurObject (OBJ_SECTORS, xpos, ypos, xpos, ypos);

	if (nsides < 3)		nsides = 3;
	if (radius < 8)		radius = 8;

	// Insert vertices (and new sector)
	InsertPolygonVertices (xpos, ypos, nsides, radius);
	if (sector < 0)
		InsertObject (OBJ_SECTORS, -1, 0, 0);

	// Insert the LineDefs
	for (n = 0; n < nsides; n++)
	{
		// Insert LineDef
		InsertObject (OBJ_LINEDEFS, -1, 0, 0);

		// Insert SideDef1
		InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
		LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs-1;
		if (sector >= 0)
			SideDefs[NumSideDefs - 1].sector = sector;
	}

	// If inside a sector, join LineDef clockwise
	if (sector >= 0)
	{
		LineDef *pLineDef = &LineDefs[NumLineDefs-1];

		// Close polygon with last LineDef
		pLineDef->start = NumVertexes - 1;
		pLineDef->end   = NumVertexes - nsides;

		// Build polygon with LineDefs
		for (n = 2; n <= nsides; n++)
		{
			pLineDef = &LineDefs[NumLineDefs-n];

			pLineDef->start = NumVertexes - n;
			pLineDef->end   = NumVertexes - n + 1;
		}
	}
	// If outside a sector, join LineDef anti-clockwise
	else
	{
		LineDef *pLineDef = &LineDefs[NumLineDefs-1];

		pLineDef->start = NumVertexes - nsides;
		pLineDef->end   = NumVertexes - 1;

		for (n = 2; n <= nsides; n++)
		{
			pLineDef = &LineDefs[NumLineDefs-n];

			pLineDef->start = NumVertexes - n + 1;
			pLineDef->end   = NumVertexes - n;
		}
	}
}

// Inserts a circle of a given radius at the mouse cursor.
void InsertCircle(SHORT xpos, SHORT ypos, SHORT radius)
{
   SHORT sector;
   SHORT n;

   if (radius < 8)
      radius = 8;

   SHORT nsides = (SHORT)(9.8 * sqrt((double)radius));

   // are we inside a Sector?
   sector = GetCurObject(OBJ_SECTORS, xpos, ypos, xpos, ypos);

   // Insert vertices (and new sector)
   InsertPolygonVertices(xpos, ypos, nsides, radius);
   if (sector < 0)
      InsertObject(OBJ_SECTORS, -1, 0, 0);

   // Insert the LineDefs
   for (n = 0; n < nsides; n++)
   {
      // Insert LineDef
      InsertObject(OBJ_LINEDEFS, -1, 0, 0);

      // Insert SideDef1
      InsertObject(OBJ_SIDEDEFS, -1, 0, 0);
      LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs - 1;
      if (sector >= 0)
         SideDefs[NumSideDefs - 1].sector = sector;
   }

   // If inside a sector, join LineDef clockwise
   if (sector >= 0)
   {
      LineDef *pLineDef = &LineDefs[NumLineDefs - 1];

      // Close polygon with last LineDef
      pLineDef->start = NumVertexes - 1;
      pLineDef->end = NumVertexes - nsides;

      // Build polygon with LineDefs
      for (n = 2; n <= nsides; n++)
      {
         pLineDef = &LineDefs[NumLineDefs - n];

         pLineDef->start = NumVertexes - n;
         pLineDef->end = NumVertexes - n + 1;
      }
   }
   // If outside a sector, join LineDef anti-clockwise
   else
   {
      LineDef *pLineDef = &LineDefs[NumLineDefs - 1];

      pLineDef->start = NumVertexes - nsides;
      pLineDef->end = NumVertexes - 1;

      for (n = 2; n <= nsides; n++)
      {
         pLineDef = &LineDefs[NumLineDefs - n];

         pLineDef->start = NumVertexes - n + 1;
         pLineDef->end = NumVertexes - n;
      }
   }
}

// Makes a torch at the given position.
void InsertTorch(SHORT xpos, SHORT ypos, SHORT torchAngle)
{
   SHORT sector;

   // Get the sector, if invalid we won't place the torch.
   sector = GetCurObject(OBJ_SECTORS, xpos, ypos, xpos, ypos);
   if (sector < 0)
      return;

   // Vertex positions for the rest of the torch.
   SHORT secondVX, secondVY, thirdVX, thirdVY, fourthVX, fourthVY, fifthVX, fifthVY;

   // Texture names for torch.
   char tx1[33] = "Torch Attaches to wall";
   char tx2[33] = "Torch Cross [The other part =)]";

   // Place first vertex.
   InsertObject(OBJ_VERTEXES, -1, xpos, ypos);
   // This vertex can possibly be merged into the linedef it is placed near, so check for that.
   // CheckVertexMerge will merge the vertex into a linedef if possible, and if successful we
   // will branch the torch outwards from that linedef.
   CheckAndMergeVertex(NumVertexes - 1, &xpos, &ypos);

   // Currently we only place torches on the cardinal directions. Any other input
   // gets changed to the nearest cardinal.
   torchAngle /= 90;
   if (torchAngle == 1)
   {
      secondVY = thirdVY = ypos;
      secondVX = fourthVX = fifthVX = xpos + 12;
      thirdVX = xpos + 18;
      fourthVY = ypos + 8;
      fifthVY = ypos - 8;
   }
   else if (torchAngle == 2)
   {
      secondVX = thirdVX = xpos;
      secondVY = fourthVY = fifthVY = ypos - 12;
      thirdVY = ypos - 18;
      fourthVX = xpos + 8;
      fifthVX = xpos - 8;
   }
   else if (torchAngle == 3)
   {
      secondVY = thirdVY = ypos;
      secondVX = fourthVX = fifthVX = xpos - 12;
      thirdVX = xpos - 18;
      fourthVY = ypos - 8;
      fifthVY = ypos + 8;
   }
   else
   {
      secondVX = thirdVX = xpos;
      secondVY = fourthVY = fifthVY = ypos + 12;
      thirdVY = ypos + 18;
      fourthVX = xpos - 8;
      fifthVX = xpos + 8;
   }

   // Second and third vertexes (the long part of the torch).
   InsertObject(OBJ_VERTEXES, -1, secondVX, secondVY);
   InsertObject(OBJ_VERTEXES, -1, thirdVX, thirdVY);

   // Make the linedefs and sidedefs for the long arm.
   for (int i = 2; i < 4; i++)
   {
      InsertObject(OBJ_LINEDEFS, -1, 0, 0);
      LineDefs[NumLineDefs - 1].start = NumVertexes - i;
      LineDefs[NumLineDefs - 1].end = NumVertexes - i + 1;

      // First sidedef
      InsertObject(OBJ_SIDEDEFS, -1, 0, 0);
      LineDefs[NumLineDefs - 1].sidedef1 = NumSideDefs - 1;
      LineDefs[NumLineDefs - 1].blak_flags = 786558;
      SideDefs[NumSideDefs - 1].sector = sector;
      SideDefs[NumSideDefs - 1].animate_speed = 90;
      strcpy(SideDefs[NumSideDefs - 1].tex3, tx1);

      // Top part has to be offset.
      if (i == 2)
      {
         SideDefs[NumSideDefs - 1].xoff = 12;
         SideDefs[NumSideDefs - 1].type3 = 8886;
      }
      else
         SideDefs[NumSideDefs - 1].type3 = 8886;

      // Second sidedef
      InsertObject(OBJ_SIDEDEFS, -1, 0, 0);
      LineDefs[NumLineDefs - 1].sidedef2 = NumSideDefs - 1;
      SideDefs[NumSideDefs - 1].sector = sector;
      SideDefs[NumSideDefs - 1].animate_speed = 90;
      strcpy(SideDefs[NumSideDefs - 1].tex3, tx1);

      // Top part has to be offset.
      if (i == 2)
      {
         SideDefs[NumSideDefs - 1].xoff = 12;
         SideDefs[NumSideDefs - 1].type3 = 8886;
      }
      else
         SideDefs[NumSideDefs - 1].type3 = 8886;
   }

   // Vertexes and linedefs for the short arm.
   InsertObject(OBJ_VERTEXES, -1, fourthVX, fourthVY);
   InsertObject(OBJ_LINEDEFS, -1, 0, 0);
   LineDefs[NumLineDefs - 1].start = NumVertexes - 3;
   LineDefs[NumLineDefs - 1].end = NumVertexes - 1;
   InsertObject(OBJ_VERTEXES, -1, fifthVX, fifthVY);
   InsertObject(OBJ_LINEDEFS, -1, 0, 0);
   LineDefs[NumLineDefs - 1].start = NumVertexes - 1;
   LineDefs[NumLineDefs - 1].end =  NumVertexes - 4;

   // Create the sidedefs for the short arm.
   for (int i = 1; i < 3; i++)
   {
      // First sidedef
      InsertObject(OBJ_SIDEDEFS, -1, 0, 0);
      LineDefs[NumLineDefs - i].sidedef1 = NumSideDefs - 1;
      LineDefs[NumLineDefs - i].blak_flags = 786558;
      SideDefs[NumSideDefs - 1].sector = sector;
      SideDefs[NumSideDefs - 1].animate_speed = 90;
      strcpy(SideDefs[NumSideDefs - 1].tex3, tx2);

      // Second arm has to be offset.
      if (i == 2)
      {
         SideDefs[NumSideDefs - 1].type3 = 8887;
         SideDefs[NumSideDefs - 1].xoff = 8;
      }
      else
      {
         SideDefs[NumSideDefs - 1].type3 = 8887;
      }
      // Second sidedef
      InsertObject(OBJ_SIDEDEFS, -1, 0, 0);
      LineDefs[NumLineDefs - i].sidedef2 = NumSideDefs - 1;
      SideDefs[NumSideDefs - 1].sector = sector;
      SideDefs[NumSideDefs - 1].animate_speed = 90;
      strcpy(SideDefs[NumSideDefs - 1].tex3, tx2);

      // Second arm has to be offset.
      if (i == 2)
      {
         SideDefs[NumSideDefs - 1].type3 = 8886;
         SideDefs[NumSideDefs - 1].xoff = 8;
      }
      else
      {
         SideDefs[NumSideDefs - 1].type3 = 8887;
      }
   }

   int x, y, xoffset, yoffset;
   x = Vertexes[NumVertexes - 4].x;
   y = Vertexes[NumVertexes - 4].y;
   GetServerCoords(&x, &y, &xoffset, &yoffset);
   Notify("Position for dynamic light is row: %03d, col: %03d, finerow: %02d, finecol: %02d.", y, x, yoffset, xoffset);
}

/*
 * CheckAndMergeVertex: takes a vertex number, and checks if the vertex can be
 *                      merged into a linedef. If possible, merges it.
 */
void CheckAndMergeVertex(SHORT vertexNum, SHORT *xpos, SHORT *ypos)
{
   SHORT sd, ld;
   
   // How close to the linedef do we check?
   SHORT distance = 4;
   SelPtr vertex = NULL;

   for (ld = 0; ld < NumLineDefs; ld++)
   {
      LineDef *pCurLD = &LineDefs[ld];
      LineDef *pNewLD;

      if (IsLineDefInside(ld, Vertexes[vertexNum].x - distance, Vertexes[vertexNum].y - distance,
         Vertexes[vertexNum].x + distance, Vertexes[vertexNum].y + distance))
      {
         // Select the vertex, necessary if we want to move it.
         SelectObject(&vertex, vertexNum);
         // Initialise MoveObjectsToCoords with the old coordinates.
         MoveObjectsToCoords(OBJ_VERTEXES, NULL, *xpos, *ypos, FALSE);
         // Get the closest point to the vertex on this linedef,
         // modifying xpos and ypos to be on the line.
         PutPointOnLineDef(pCurLD, xpos, ypos);
         // See if we need to move the vertex onto the linedef.
         MoveObjectsToCoords(OBJ_VERTEXES, vertex, *xpos, *ypos, FALSE);
         // Unselect the vertex.
         ForgetSelection(&vertex);

         InsertObject(OBJ_LINEDEFS, ld, 0, 0);
         pCurLD = &LineDefs[ld];
         pNewLD = &LineDefs[NumLineDefs - 1];

         pCurLD->end = vertexNum;
         pNewLD->start = vertexNum;
         sd = pCurLD->sidedef1;
          if (sd >= 0)
          {
             InsertObject(OBJ_SIDEDEFS, sd, 0, 0);
             pNewLD->sidedef1 = NumSideDefs - 1;
          }
          sd = pCurLD->sidedef2;
          if (sd >= 0)
          {
             InsertObject(OBJ_SIDEDEFS, sd, 0, 0);
             pNewLD->sidedef2 = NumSideDefs - 1;
          }
          MadeChanges = TRUE;
          MadeMapChanges = TRUE;
          break;
          //return True;
      }
   }
   //return False;
}

/*
 * PutPointOnLineDef: modifies xpos and ypos to be on the linedef.
 */
void PutPointOnLineDef(LineDef *ld, SHORT *xpos, SHORT *ypos)
{
   SHORT v1, v2;
   v1 = ld->start;
   v2 = ld->end;

   float A = (float)*xpos - Vertexes[v1].x;
   float B = (float)*ypos - Vertexes[v1].y;
   float C = (float)Vertexes[v2].x - Vertexes[v1].x;
   float D = (float)Vertexes[v2].y - Vertexes[v1].y;
   float dot = A * C + B * D;
   float len_sq = C * C + D * D;
   float t = dot / len_sq;
   if (t < 0.0f)
      t = 0.0f;
   else if (t > 1.0f)
      t = 1.0f;
   *xpos = (SHORT)roundf(Vertexes[v1].x + C * t);
   *ypos = (SHORT)roundf(Vertexes[v1].y + D * t);
}

/*
 *CheckFailed: display a message, then ask if the check should continue
 *             returns TRUE if user want to stop checking.
 */
BOOL CheckFailed (BOOL fatal, char *format, ...)
{
	int rc;
	char msg[256];
	int len = 0;
	va_list args;
	UINT MBStyle;

	va_start (args, format);
	len = vsprintf (&msg[len], format, args);
	va_end (args);

	if ( fatal )
	{
		len += wsprintf (&msg[len], "\n\nDOOM will crash if you play "
									"with this level !");
		MBStyle = MB_OK;
	}
	else
	{
		len += wsprintf (&msg[len], "\n\nDo you want to continue the "
									"verification process ?");
		MBStyle = MB_YESNO;
	}

	// Display dialog box
#if 0
	rc = ::MessageBox (((TApplication *)::Module)->GetMainWindow()->GetActiveWindow(),
					   msg,
					   "Verification failed",
					   MBStyle | MB_ICONEXCLAMATION);
#endif

#if 1
	rc = ::MessageBox (0,
					   msg,
					   "Q would not approve...",
					   MBStyle | MB_ICONEXCLAMATION | MB_TASKMODAL);
#endif

	return (rc == IDNO);
}


/*
   check if all sectors are closed
*/

BOOL CheckSectors ()
{
	BOOL rc = FALSE;
	SHORT  s, n, sd;
	BYTE   *ends = NULL;

	// Set WAIT cursor and save status bar message
	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();
	TWorkDialog *pWorkDlg = new TWorkDialog;
	pWorkDlg->Create();
	LogMessage ("\nVerifying Sectors...\n");

	// TODO: CheckingObjects( -1, -1);
	ends = (BYTE *)GetMemory (NumVertexes * sizeof(BYTE));

	//
	// Check that each sector is closed
	//
	pWorkDlg->SetRange(0, NumSectors-1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Checking closed Sectors...");

	for (s = 0; s < NumSectors; s++)
	{
		if ( s % 16 == 0 || s == NumSectors-1)
		{
			pWorkDlg->SetValue(s);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckSectorsEnd;
			// WorkMessage ("Checking closed Sectors %d/%d...",
			// 			 s, NumSectors-1);
		}
		/* clear the "ends" array */
		for (n = 0; n < NumVertexes; n++)
			ends[n] = 0;

		/* for each SideDef bound to the Sector, store a "1" in the "ends" */
		/* array for its starting Vertex, and a "2" for its ending Vertex  */
		for (n = 0; n < NumLineDefs; n++)
		{
			LineDef CurLD = LineDefs[n];

			sd = CurLD.sidedef1;
			if (sd >= 0 && SideDefs[sd].sector == s)
			{
				ends[CurLD.start] |= 1;
				ends[CurLD.end]   |= 2;
			}

			sd = CurLD.sidedef2;
			if (sd >= 0 && SideDefs[ sd].sector == s)
			{
				ends[CurLD.end]   |= 1;
				ends[CurLD.start] |= 2;
			}
		}

		/* every entry in the "ends" array should be "0" or "3" */
		for (n = 0; n < NumVertexes; n++)
		{
			if (ends[n] == 1)
			{
				if (CheckFailed (FALSE, "Sector #%d is not closed! There is "
										"no SideDef ending at Vertex #%d", s, n))
				{
					GoToObject (OBJ_VERTEXES, n);
					rc = TRUE;
					goto CheckSectorsEnd;
				}
			}

			if (ends[n] == 2)
			{
				if (CheckFailed (FALSE, "Sector #%d is not closed ! There is "
										"no SideDef starting at Vertex #%d", s, n))
				{
					GoToObject (OBJ_VERTEXES, n);
					rc = TRUE;
					goto CheckSectorsEnd;
				}
			}
		}
	}
	FreeMemory (ends);

	/*
	  Note from RQ:
		This is a very simple idea, but it works!  The first test (above)
		checks that all Sectors are closed.  But if a closed set of LineDefs
		is moved out of a Sector and has all its "external" SideDefs pointing
		to that Sector instead of the new one, then we need a second test.
		That's why I check if the SideDefs facing each other are bound to
		the same Sector.

	  Other note from RQ:
		Nowadays, what makes the power of a good editor is its automatic tests.
		So, if you are writing another Doom editor, you will probably want
		to do the same kind of tests in your program.  Fine, but if you use
		these ideas, don't forget to credit DEU...  Just a reminder... :-)
	*/

	//
	// now check if all SideDefs are facing a SideDef with the same
	// Sector number
	//
	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Checking facing SideDefs...");

	for (n = 0; n < NumLineDefs; n++)
	{
		LineDef CurLD = LineDefs[n];

		if ( n % 8 == 0  ||  n == NumLineDefs-1)
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckSectorsEnd;
			// WorkMessage ("Checking facing SideDefs %d/%d...",
			//			 n, NumLineDefs-1);
		}

		// Two SideDefs
		for (int i = 0 ; i < 2 ; i++)
		{
			BOOL FirstSide;
			if ( i == 0 )
			{
				sd = CurLD.sidedef1;
				FirstSide = TRUE;
			}
			else
			{
				sd = CurLD.sidedef2;
				FirstSide = FALSE;
			}

			if (sd >= 0)
			{
				s = GetOppositeSector (n, FirstSide);
				if (s < 0 || SideDefs[sd].sector != s)
				{
					BOOL endCheck;

					if (s < 0)
					{
						endCheck = CheckFailed (FALSE,
										"Sector #%d is not closed ! Check "
										"LineDef #%d (%s SideDef: #%d)",
										SideDefs[ sd].sector, n,
										i == 0 ? "first" : "second", sd);
					}
					else
					{
						endCheck = CheckFailed (FALSE,
										"Sectors #%d and #%d are not closed ! "
										"Check LineDef #%d (%s SideDef: "
										"#%d) and the one facing it.",
										SideDefs[ sd].sector, s, n,
										i == 0 ? "first" : "second", sd);
					}

					if ( endCheck )
					{
						GoToObject (OBJ_LINEDEFS, n);
						rc = TRUE;
						goto CheckSectorsEnd;
					}
				}
			}
		}
	}

CheckSectorsEnd:
	if ( ends != NULL )
		FreeMemory(ends);
	delete pWorkDlg;
	RESTORE_WORK_MSG();
	UNSELECT_WAIT_CURSOR();
	return (rc);
}


/*
   display number of objects, etc.
*/
/* RP: Replaced by a dialog box */
#if 0
void Statistics ()
{
	char msg[1024];
	int len = 0;

	len += wsprintf (&msg[len], "Statistics:\n\n");
	len += wsprintf (&msg[len], "Number of Things:\t\t%4d (%3luKb)\n",
					 NumThings,
					 ((ULONG) NumThings * sizeof(Thing) + 512L) / 1024L);
	len += wsprintf (&msg[len], "Number of Vertices:\t%5d (%3luKb)\n",
					 NumVertexes,
					 ((ULONG) NumVertexes * sizeof(Vertex) + 512L) / 1024L);
	len += wsprintf (&msg[len],  "Number of LineDefs:\t%5d (%3luKb)\n",
					 NumLineDefs,
					 ((ULONG) NumLineDefs * sizeof(LineDef) + 512L) / 1024L);
	len += wsprintf (&msg[len], "Number of SideDefs:\t%5d (%3luKb)\n",
					 NumSideDefs,
					 ((ULONG) NumSideDefs * sizeof(SideDef) + 512L) / 1024L);
	len += wsprintf (&msg[len], "Number of Sectors:\t%5d (%3luKb)",
					 NumSectors,
					 ((ULONG) NumSectors * sizeof(Sector) + 512L) / 1024L);

	::MessageBox (0,
				  msg,
				  "Statistics of level objects",
				  MB_OK | MB_TASKMODAL);
}
#endif


/*
   check cross-references and delete unused objects
*/

BOOL CheckCrossReferences ()
{
	BOOL rc = FALSE;
	SHORT  n, m;
	SelPtr cur;

	// Select WAIT cursor and save status bar message
	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();
	TWorkDialog *pWorkDlg = new TWorkDialog;
	pWorkDlg->Create();

	LogMessage( "\nVerifying cross-references...\n");

	//
	// Check SideDefs and Vertices of LineDefs
	//
	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for invalid LineDefs...");

	for (n = 0; n < NumLineDefs; n++)
	{
		if ( n % 16 == 0 || n == NumLineDefs-1)
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckCrossEnd;

			// WorkMessage ("Looking for invalid LineDefs %d/%d...",
			//			 n, NumLineDefs-1);
		}
		LineDef CurLD = LineDefs[n];

		/* check for missing first SideDefs */
		if (CurLD.sidedef1 < 0)
		{
			CheckFailed (TRUE, "ERROR: LineDef #%d has no first SideDef!", n);
			GoToObject (OBJ_LINEDEFS, n);
			rc = TRUE;
			goto CheckCrossEnd;
		}

		/* check for SideDefs used twice in the same LineDef */
		if (CurLD.sidedef1 == CurLD.sidedef2)
		{
			CheckFailed (TRUE, "ERROR: LineDef #%d uses the same SideDef twice "
							   "(#%d)", n, CurLD.sidedef1);
			GoToObject( OBJ_LINEDEFS, n);
			rc = TRUE;
			goto CheckCrossEnd;
		}

		/* check for Vertices used twice in the same LineDef */
		if (CurLD.start == CurLD.end)
		{
			CheckFailed (TRUE, "ERROR: LineDef #%d uses the same Vertex twice "
							   "(#%d)", n, CurLD.start);
			GoToObject( OBJ_LINEDEFS, n);
			rc = TRUE;
			goto CheckCrossEnd;
		}
	}

	//
	// check if there aren't two LineDefs between the same Vertices
	//
	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for redundant LineDefs...");

	cur = NULL;
	for (n = NumLineDefs - 1; n >= 1; n--)
	{
		LineDef *pLineDefn = &LineDefs[n];
		SHORT nstart = pLineDefn->start;
		SHORT nend   = pLineDefn->end;
		if ( n % 16 == 0 || n == 1)
		{
			pWorkDlg->SetValue(NumLineDefs - n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckCrossEnd;

			// WorkMessage ("Looking for redundant LineDefs %d...", n);
		}
		for (m = n - 1; m >= 0; m--)
		{
			LineDef *pLineDefm = &LineDefs[m];
			SHORT mstart = pLineDefm->start;
			SHORT mend   = pLineDefm->end;
			if ( (nstart == mstart && nend == mend  ) ||
				 (nstart == mend   && nend == mstart) )
			{
				SelectObject( &cur, n);
				break;
			}
		}
	}

	if (cur != NULL &&
		(Expert ||
		 Confirm ("There are multiple LineDefs between the same Vertices.\n"
				  "Do you want to delete the redundant LineDefs ?") ) )
	{
		// Set MadeChanges to TRUE
		DeleteObjects (OBJ_LINEDEFS, &cur);
		rc = TRUE;
	}
	ForgetSelection (&cur);

	//
	// check for invalid flags in the LineDefs
	//
	//	WorkMessage ("Looking for invalid LineDef flags...");
	pWorkDlg->SetRange(0, 3);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for invalid LineDef flags...");

	// These flags are no longer used ARK
#if 0
	// Impassible flag
	assert (cur == NULL);
	for (n = 0; n < NumLineDefs; n++)
	{
		LineDef *pLineDef = &LineDefs[n];
		if ( (pLineDef->flags & 0x01) == 0  &&  pLineDef->sidedef2 < 0)
		{
			SelectObject (&cur, n);
		}
	}
	pWorkDlg->SetValue(1);

	if (cur != NULL &&
		(Expert ||
		 Confirm ("Some LineDefs have only one side but their 'Impassible' "
				  "flag is not set.\nDo you want to set their flag ?") ) )
	{
		while (cur)
		{
			LineDefs[cur->objnum].flags |= 0x01;
			MadeChanges = TRUE;
			UnSelectObject (&cur, cur->objnum);
		}
		rc = TRUE;
	}
	ForgetSelection( &cur);


	// Two-sided redundant flag
	assert (cur == NULL);
	for (n = 0; n < NumLineDefs; n++)
	{
		LineDef *pLineDef = &LineDefs[n];
		if ( (pLineDef->flags & 0x04) != 0  &&  pLineDef->sidedef2 < 0)
		{
			SelectObject (&cur, n);
		}
	}
	pWorkDlg->SetValue(2);

	if (cur != NULL &&
		(Expert ||
		 Confirm ("Some LineDefs have only one side but their 'Two-sided' bit "
				  "is set.\nDo you want to clear their flag ?") ) )
	{
		while (cur)
		{
			LineDefs[cur->objnum].flags &= ~0x04;
			MadeChanges = TRUE;
			UnSelectObject( &cur, cur->objnum);
		}
		rc = TRUE;
	}
	ForgetSelection( &cur);

	// Two-sided missing flag
	assert (cur == NULL);
	for (n = 0; n < NumLineDefs; n++)
	{
		LineDef *pLineDef = &LineDefs[n];
		if ( (pLineDef->flags & 0x04) == 0  &&  pLineDef->sidedef2 >= 0)
		{
			SelectObject (&cur, n);
		}
	}
	pWorkDlg->SetValue(3);

	if (cur != NULL &&
		(Expert ||
		 Confirm ("Some LineDefs have two sides but their 'Two-sided' bit is "
				  "not set.\nDo you want to set their flag ?") ) )
	{
		while (cur)
		{
			LineDefs[cur->objnum].flags |= 0x04;
			MadeChanges = TRUE;
			UnSelectObject (&cur, cur->objnum);
		}
		rc = TRUE;
	}
	ForgetSelection (&cur);
#endif

	//
	//  Check for unused vertices
	//

	/* select all Vertices */
	assert (cur == NULL);
	for (n = 0; n < NumVertexes; n++)
		SelectObject (&cur, n);

	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for unused Vertices...");

	/* unselect Vertices used in a LineDef */
	for (n = 0; n < NumLineDefs &&  cur != NULL ; n++)
	{
		LineDef *pLineDef = &LineDefs[n];
		if ( n % 16 == 0 || n == NumLineDefs - 1 )
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckCrossEnd;

			// WorkMessage ("Looking for unused Vertices %d/%d...",
			//			 n, NumLineDefs-1);
		}

		m = pLineDef->start;
		if (m >= 0)			UnSelectObject (&cur, m);
		m = pLineDef->end;
		if (m >= 0)         UnSelectObject (&cur, m);
	}

	/* check if there are any Vertices left */
	if (cur != NULL &&
		(Expert ||
		 Confirm ("Some Vertices are not bound to any LineDef.\n"
				  "Do you want to delete these unused Vertices ?") ) )
	{
		// Set MadeChanges to TRUE
		DeleteObjects (OBJ_VERTEXES, &cur);
		rc = TRUE;
	}
	ForgetSelection (&cur);

	//
	// Check for unused SideDefs
	//

	/* select all SideDefs */
	assert (cur == NULL);
	for (n = 0; n < NumSideDefs; n++)
		SelectObject (&cur, n);

	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for unused SideDefs...");

	/* unselect SideDefs bound to a LineDef */
	for (n = 0; n < NumLineDefs && cur != NULL; n++)
	{
		LineDef *pLineDef = &LineDefs[n];
		if ( n % 16 == 0 || n == NumLineDefs-1 )
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckCrossEnd;

			// WorkMessage ("Looking for unused SideDefs %d/%d...",
			//			 n, NumLineDefs-1);
		}

		m = pLineDef->sidedef1;
		if (m >= 0)		UnSelectObject (&cur, m);
		m = pLineDef->sidedef2;
		if (m >= 0)		UnSelectObject (&cur, m);
	}

	/* check if there are any SideDefs left */
	if (cur != NULL &&
		(Expert ||
		 Confirm ("Some SideDefs are not bound to any LineDef.\n"
				  "Do you want to delete these unused SideDefs ?") ) )
	{
		// Set MadeChanges to TRUE
		DeleteObjects( OBJ_SIDEDEFS, &cur);
		rc = TRUE;
	}
	ForgetSelection (&cur);

	//
	// Check for unused Sectors
	//

	/* select all Sectors */
	assert (cur == NULL);
	for (n = 0; n < NumSectors; n++)
		SelectObject (&cur, n);

	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for unused Sectors...");

	/* unselect Sectors bound to a SideDef */
	for (n = 0; n < NumLineDefs && cur != NULL; n++)
	{
		if ( n % 16 == 0 || n == NumLineDefs-1 )
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckCrossEnd;

			// WorkMessage ("Looking for unused Sectors %d/%d...",
			//			 n, NumLineDefs-1);
		}

		LineDef *pLineDef = &LineDefs[n];
		SHORT sector;
		m = pLineDef->sidedef1;
		if (m >= 0 && (sector = SideDefs[m].sector) >= 0)
			UnSelectObject (&cur, sector);

		m = pLineDef->sidedef2;
		if (m >= 0 && (sector = SideDefs[m].sector) >= 0)
			UnSelectObject (&cur, sector);
	}

	/* check if there are any Sectors left */
	if (cur != NULL &&
		(Expert ||
		 Confirm ("Some Sectors are not bound to any SideDef.\n"
				  "Do you want to delete these unused Sectors ?") ) )
	{
		// Set MadeChanges to TRUE
		DeleteObjects (OBJ_SECTORS, &cur);
		rc = TRUE;
	}
	ForgetSelection (&cur);

	// Restore cursor and status bar
CheckCrossEnd:
	delete pWorkDlg;
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();
	return (rc);
}



/*
   check for missing textures
   Return TRUE if need to repaint window
*/

BOOL CheckTextures ()
{
	BOOL          rc = FALSE;
	SHORT         n;
	char texname[MAX_BITMAPNAME+1];
	texname[MAX_BITMAPNAME] = '\0';

	// Select WAIT cursor and save status bar message
	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();
	TWorkDialog *pWorkDlg = new TWorkDialog;
	pWorkDlg->Create();
	LogMessage( "\nVerifying textures...\n");

	pWorkDlg->SetRange(0, NumSectors-1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Checking texture and height of Sectors...");
	for (n = 0; n < NumSectors; n++)
	{
		Sector *pSector = &Sectors[n];

		if ( n % 16 == 0 || n == NumSectors-1)
		{
			pWorkDlg->SetValue(n);
			//WorkMessage ("Checking texture and height of Sectors %d/%d...",
			//			 n, NumSectors-1);
		}
		// Check ceiling texture
		strncpy (texname, pSector->ceilt, MAX_BITMAPNAME);
		if (texname[0] == '-' && texname[1] == '\0')
		{
			CheckFailed (TRUE, "Error: Sector #%d has no ceiling texture.\n"
							   "You probaly used a brain-damaged editor to "
							   "do that...", n);
			GoToObject (OBJ_SECTORS, n);
			rc = TRUE;
			goto CheckTexturesEnd;
		}

		// Check floor texture
		strncpy (texname, pSector->ceilt, MAX_BITMAPNAME);
		if (texname[0] == '-' && texname[1] == '\0')
		{
			CheckFailed (TRUE, "Error: Sector #%d has no floor texture.\n"
							   "You probaly used a brain-damaged editor "
							   "to do that...", n);
			GoToObject (OBJ_SECTORS, n);
			rc = TRUE;
			goto CheckTexturesEnd;
		}

		// Check floor and ceiling heights
		if (pSector->ceilh < pSector->floorh)
		{
			CheckFailed (TRUE, "Error: Sector #%d has its ceiling lower than "
							   "its floor.\nThe textures will never be "
							   "displayed if you cannot go there", n);
			GoToObject( OBJ_SECTORS, n);
			rc = TRUE;
			goto CheckTexturesEnd;
		}

		// Check floor and ceiling difference in heights
		if (pSector->ceilh - pSector->floorh > 4096)
		{
			CheckFailed (TRUE, "Error: Sector #%d has its ceiling too high.\n"
							   "The maximum difference allowed is 1023 "
							   "(ceiling height - floor height)", n);
			GoToObject (OBJ_SECTORS, n);
			rc = TRUE;
			goto CheckTexturesEnd;
		}
	}


	pWorkDlg->SetRange(0, NumLineDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Checking textures of LineDefs & SideDefs...");
	for (n = 0; n < NumLineDefs; n++)
	{
		if ( n % 16 == 0 || n == NumLineDefs-1)
		{
			pWorkDlg->SetValue(n);
			// WorkMessage ("Checking textures of LineDefs & SideDefs %d/%d...",
			//			 n, NumLineDefs-1);
		}
		SHORT   sd1, sd2;
		SHORT   s1, s2;
		SideDef *pSideDef1;
		SideDef *pSideDef2;
		Sector  *pSector1;
		Sector  *pSector2;
		LineDef *pLineDef = &LineDefs[n];
		sd1 = pLineDef->sidedef1;
		sd2 = pLineDef->sidedef2;

		if (sd1 >= 0)
		{
			pSideDef1 = &SideDefs[sd1];
			s1 = pSideDef1->sector;
			if ( s1 >= 0 )
				pSector1 = &Sectors[s1];
		}
		else
			s1 = -1;

		if (sd2 >= 0)
		{
			pSideDef2 = &SideDefs[sd2];
			s2 = pSideDef2->sector;
			if ( s2 >= 0 )
				pSector2 = &Sectors[s2];
		}
		else
			s2 = -1;

		// Looking for missing normal texture in first SideDef
		if (s1 >= 0 && s2 < 0)
		{
			if (pSideDef1->tex3[0] == '-' && pSideDef1->tex3[1] == '\0')
			{
				if (CheckFailed (FALSE, "Error in one-sided Linedef #%d: "
										"SideDef #%d has no normal texture.\n"
										"Do you want to set the texture to "
										"\"%s\" and continue ?", n, sd1,
										DefaultWallTexture) )
				{
					GoToObject (OBJ_LINEDEFS, n);
					rc = TRUE;
					goto CheckTexturesEnd;
				}
				strncpy (pSideDef1->tex3, DefaultWallTexture, MAX_BITMAPNAME);
				MadeChanges = TRUE;
				rc = TRUE;
			}
		}

		// Looking for missing upper texture in first SideDef
		if (s1 >= 0 && s2 >= 0 && pSector1->ceilh > pSector2->ceilh)
		{
			if (pSideDef1->tex1[0] == '-'  &&  pSideDef1->tex1[1] == '\0'
				&& (strncmp(pSector1->ceilt, "F_SKY1", MAX_BITMAPNAME) != 0 ||
					strncmp(pSector2->ceilt, "F_SKY1", MAX_BITMAPNAME) != 0 ) )
			{
				if (CheckFailed (FALSE, "Error in first SideDef of Linedef "
										"#%d: SideDef #%d has no upper "
										"texture.\nDo you want to set the "
										"texture to \"%s\" and continue ?", n,
										sd1, DefaultUpperTexture) )
				{
					GoToObject (OBJ_LINEDEFS, n);
					rc = TRUE;
					goto CheckTexturesEnd;
				}
				strncpy (pSideDef1->tex1, DefaultUpperTexture, MAX_BITMAPNAME);
				MadeChanges = TRUE;
				rc = TRUE;
			}
		}

		// Looking for missing lower texture in first SideDef
		if (s1 >= 0 && s2 >= 0 && pSector1->floorh < pSector2->floorh)
		{
			if (pSideDef1->tex2[0] == '-' && pSideDef1->tex2[1] == '\0')
			{
				if (CheckFailed (FALSE, "Error in first SideDef of Linedef "
										"#%d: SideDef #%d has no lower "
										"texture.\nDo you want to set the "
										"texture to \"%s\" and continue ?", n,
										sd1, DefaultLowerTexture))
				{
					GoToObject( OBJ_LINEDEFS, n);
					rc = TRUE;
					goto CheckTexturesEnd;
				}
				strncpy (pSideDef1->tex2, DefaultLowerTexture, MAX_BITMAPNAME);
				MadeChanges = TRUE;
				rc = TRUE;
			}
		}

		// Looking for missing upper texture in second SideDef
		if (s1 >= 0 && s2 >= 0 && pSector2->ceilh > pSector1->ceilh)
		{
			if (pSideDef2->tex1[0] == '-' && pSideDef2->tex1[1] == '\0'
				&& (strncmp (pSector1->ceilt, "F_SKY1", MAX_BITMAPNAME) ||
					strncmp (pSector2->ceilt, "F_SKY1", MAX_BITMAPNAME) ) )
			{
				if ( CheckFailed (FALSE, "Error in second SideDef of Linedef "
										 "#%d: SideDef #%d has no upper "
										 "texture.\nDo you want to set the "
										 "texture to \"%s\" and continue ?", n,
										 sd2, DefaultUpperTexture))
				{
					GoToObject (OBJ_LINEDEFS, n);
					rc = TRUE;
					goto CheckTexturesEnd;
				}
				strncpy (pSideDef2->tex1, DefaultUpperTexture, MAX_BITMAPNAME);
				MadeChanges = TRUE;
				rc = TRUE;
			}
		}

		// Looking for missing lower texture in second SideDef
		if (s1 >= 0 && s2 >= 0 && pSector2->floorh < pSector1->floorh)
		{
			if (pSideDef2->tex2[0] == '-' && pSideDef2->tex2[1] == '\0')
			{
				if (CheckFailed (FALSE, "Error in second SideDef of Linedef "
										"#%d: SideDef #%d has no lower "
										"texture.\nDo you want to set the "
										"texture to \"%s\" and continue?", n,
										sd2, DefaultLowerTexture))
				{
					GoToObject (OBJ_LINEDEFS, n);
					rc = TRUE;
					goto CheckTexturesEnd;
				}
				strncpy (pSideDef2->tex2, DefaultLowerTexture, MAX_BITMAPNAME);
				MadeChanges = TRUE;
				rc = TRUE;
			}
		}
	} // End For

	// Restore cursor and status bar
CheckTexturesEnd:
	delete pWorkDlg;
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();
	return (rc);
}



/*
   check if a texture name matches one of the elements of a list (or if it's blank)
*/

BOOL IsTextureNameInList (char *name, TextureInfo **list, SHORT numelems)
{
   SHORT n;

   if (name[0] == 0)
      return TRUE;

   for (n = 0; n < numelems; n++)
      if (strnicmp (name, list[n]->Name, MAX_BITMAPNAME) == 0)
	 return TRUE;
   return FALSE;
}



/*
   check for invalid texture names
   Return TRUE if need to repaint window
*/

BOOL CheckTextureNames ()
{
	BOOL rc = FALSE;
	SHORT  n;
	char texname[MAX_BITMAPNAME+1];
	texname[MAX_BITMAPNAME] = '\0';

	// Select WAIT cursor and save status bar message
	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();
	TWorkDialog *pWorkDlg = new TWorkDialog;
	pWorkDlg->Create();

	LogMessage ("\nVerifying texture names...\n");

	/*
		TODO: What is this? PROTECTION (REMOVED!)
	if (FindMasterDir (MasterDir, "F2_START") == NULL)
		NumThings--;
	*/
	pWorkDlg->SetRange(0, NumSectors-1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for invalid Sector texture names...");

	for (n = 0; n < NumSectors; n++)
	{
		if ( n % 16 == 0 || n == NumSectors-1)
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckTexturesNamesEnd;

			// WorkMessage ("Looking for invalid Sector texture names %d/%d...",
			//			 n, NumSectors-1);
		}
		Sector *pSector = &Sectors[n];

		strncpy (texname, pSector->ceilt, MAX_BITMAPNAME);
		if (! IsTextureNameInList (texname, FTexture, NumFTexture))
		{
			if ( CheckFailed (FALSE, "Invalid ceiling texture in Sector #%d.\n"
									 "The name \"%s\" is not a floor/ceiling "
									 "texture", n, texname))
			{
				GoToObject (OBJ_SECTORS, n);
				rc = TRUE;
				goto CheckTexturesNamesEnd;
			}
		}

		strncpy (texname, pSector->floort, MAX_BITMAPNAME);
		if (! IsTextureNameInList (texname, FTexture, NumFTexture))
		{
			if (CheckFailed (FALSE, "Invalid floor texture in Sector #%d.\n"
									"The name \"%s\" is not a floor/ceiling"
									"texture", n, texname))
			{
				GoToObject (OBJ_SECTORS, n);
				rc = TRUE;
				goto CheckTexturesNamesEnd;
			}
		}
	} // End for


	pWorkDlg->SetRange(0, NumSideDefs - 1);
	pWorkDlg->SetValue(0);
	pWorkDlg->SetWorkText ("Looking for invalid SideDef texture names...");

	for (n = 0; n < NumSideDefs; n++)
	{
		SideDef *pSideDef = &SideDefs[n];

		if ( n % 16 == 0 || n == NumSideDefs-1)
		{
			pWorkDlg->SetValue(n);
			COOPERATE();
			if ( pWorkDlg->Cancel )
				goto CheckTexturesNamesEnd;

			// WorkMessage ("Looking for invalid SideDef texture names %d/%d...",
			//			 n, NumSideDefs-1);
		}

		strncpy (texname, pSideDef->tex1, MAX_BITMAPNAME);
		if (! IsTextureNameInList (texname, WTexture, NumWTexture))
		{
			if (CheckFailed (FALSE, "Invalid upper texture in SideDef #%d.\n"
									"The name \"%s\" is not a wall texture",
									n, texname))
			{
				GoToObject (OBJ_SIDEDEFS, n);
				rc = TRUE;
				goto CheckTexturesNamesEnd;
			}
		}

		strncpy (texname, pSideDef->tex2, MAX_BITMAPNAME);
		if (! IsTextureNameInList (texname, WTexture, NumWTexture))
		{
			if (CheckFailed (FALSE, "Invalid lower texture in SideDef #%d.\n"
									"The name \"%s\" is not a wall texture",
									n, texname))
			{
				GoToObject (OBJ_SIDEDEFS, n);
				rc = TRUE;
				goto CheckTexturesNamesEnd;
			}
		}


		strncpy (texname, pSideDef->tex3, MAX_BITMAPNAME);
		if (! IsTextureNameInList (texname, WTexture, NumWTexture))
		{
			if (CheckFailed (FALSE, "Invalid normal texture in SideDef #%d.\n"
									"The name \"%s\" is not a wall texture",
									n, texname))
			{
				GoToObject (OBJ_SIDEDEFS, n);
				rc = TRUE;
				goto CheckTexturesNamesEnd;
			}
		}
	} // End for

	// Restore cursor and status bar
CheckTexturesNamesEnd:
	delete pWorkDlg;
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();
	return (rc);
}



/* end of file */
