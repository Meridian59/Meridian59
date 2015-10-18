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

	FILE:         objects.h
*/
#ifndef __objects_h
#define __objects_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef __levels_h
	#include "levels.h"		// TextureInfo
#endif

/* object types */
#define OBJ_THINGS      1
#define OBJ_LINEDEFS        2
#define OBJ_SIDEDEFS        3
#define OBJ_VERTEXES        4
#define OBJ_SEGS        5
#define OBJ_SSECTORS        6
#define OBJ_NODES       7
#define OBJ_SECTORS     8
#define OBJ_REJECT      9
#define OBJ_BLOCKMAP        10

// Object type used to copy the linedefs of sectors with no sidedef data.
// Done because copying linedefs now copies the sidedefs also
// (this was broken previously).
#define OBJ_LINEDEFSNOSIDEDEFS 11


/*
   the selection list is used when more than one object is selected
*/

typedef struct SelectionList *SelPtr;
struct SelectionList
{
   SelPtr next;         /* next in list */
   int objnum;          /* object number */
};


/* from objects.cpp */
void HighlightSelection (TMapDC &dc, int objtype, SelPtr list);
BOOL IsSelected (SelPtr list, SHORT objnum);
void SelectObject (SelPtr *list, SHORT objnum);
void UnSelectObject (SelPtr *list, SHORT objnum);
void ForgetSelection (SelPtr *list);
SHORT GetMaxObjectNum (int objtype);
SHORT GetCurObject (int objtype, SHORT x0, SHORT y0, SHORT x1, SHORT y1);
SelPtr SelectObjectsInBox (int objtype, SHORT x0, SHORT y0, SHORT x1, SHORT y1);
void HighlightObject(TMapDC &dc, int objtype, SHORT objnum, int color);
void DeleteObject (int objtype, SHORT objnum);
void DeleteObjects (int objtype, SelPtr *list);
void InsertObject (int objtype, SHORT copyfrom, SHORT xpos, SHORT ypos);
BOOL IsLineDefInside (SHORT ldnum, SHORT x0, SHORT y0, SHORT x1, SHORT y1);
SHORT GetOppositeSector (SHORT ld1, BOOL firstside);
void CopyObjects (int objtype, SelPtr obj);
BOOL MoveObjectsToCoords (int objtype, SelPtr obj, SHORT newx, SHORT newy, SHORT grid);
void GetObjectCoords (int objtype, SHORT objnum, SHORT *xpos, SHORT *ypos);
void RotateAndScaleObjects (int objtype, SelPtr obj, double angle, double scale);
SHORT FindFreeTag();
void FlipLineDefs (SelPtr obj, BOOL swapvertices);
void DeleteVerticesJoinLineDefs (SelPtr obj);
void MergeVertices (SelPtr *list);
BOOL AutoMergeVertices (SelPtr *list);
void SplitLineDefs (SelPtr obj);
void SplitSector (SHORT vertex1, SHORT vertex2);
void SplitLineDefsAndSector (SHORT linedef1, SHORT linedef2);
void MergeSectors (SelPtr *slist);
void DeleteLineDefsJoinSectors (SelPtr *ldlist);
void MakeDoorFromSector (SHORT sector);
void MakeLiftFromSector (SHORT sector);
SHORT GetTextureRefHeight (SHORT sidedef);
void AlignTexturesY (SelPtr *sdlist);
void AlignTexturesX (SelPtr *sdlist, SHORT type_sd, SHORT type_tex, SHORT type_off, int wall_part);
void DistributeSectorFloors (SelPtr obj);
void DistributeSectorCeilings (SelPtr obj);

void GoToObject (int objtype, SHORT objnum);

// originaly form editobj.cpp
void InsertRectangle (SHORT xpos, SHORT ypos, SHORT width, SHORT height);
void InsertPolygon (SHORT xpos, SHORT ypos, SHORT nsides, SHORT radius);
void InsertCircle(SHORT xpos, SHORT ypos, SHORT radius);
void InsertTorch(SHORT xpos, SHORT ypos, SHORT torchAngle);
void PutPointOnLineDef(LineDef *ld, SHORT *xpos, SHORT *ypos);
void CheckAndMergeVertex(SHORT vertexNum, SHORT *xpos, SHORT *ypos);
BOOL CheckFailed (BOOL fatal, char *format, ...);
BOOL IsTextureNameInList (char *name, TextureInfo **list, SHORT numelems);
void Statistics ();
BOOL CheckSectors ();
BOOL CheckCrossReferences ();
BOOL CheckTextures ();
BOOL CheckTextureNames ();


#endif
