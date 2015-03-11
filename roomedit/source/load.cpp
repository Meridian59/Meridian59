/*
	FILE:         levels.cpp

	OVERVIEW
	========
	Source file for level loading/saving functions
*/

#include "common.h"
#pragma hdrstop

#include "levels.h"
#include "lprogdlg.h"
#include "gfx.h"	// ComputeAngle()
#include "objects.h"	// IsLineDefInside()
#include "names.h"		//  GetDoomLevelName()
#include "things.h"
#include "bsp.h"

/* the global data */
MDirPtr Level = NULL;			/* master dictionary entry for the level */

SHORT NumThings = 0;			/* number of things */
TPtr  Things;					/* things data */

SHORT NumLineDefs = 0;			/* number of line defs */
LDPtr LineDefs;					/* line defs data */

SHORT NumSideDefs = 0;			/* number of side defs */
SDPtr SideDefs;					/* side defs data */

SHORT NumVertexes = 0;			/* number of vertexes */
VPtr  Vertexes;					/* vertex data */

SHORT NumSectors = 0;			/* number of sectors */
SPtr  Sectors;					/* sectors data */

SHORT  NumSegs = 0;				/* number of segments */
SEPtr Segs = NULL;				/* list of segments */
SEPtr LastSeg = NULL;			/* last segment in the list */

SHORT NumSSectors = 0;			/* number of subsectors */
SSPtr SSectors = NULL;			/* list of subsectors */
SSPtr LastSSector = NULL;		/* last subsector in the list */

//static int node_num;            /* Used when numbering nodes for saving them out */
//static int wall_num;            /* Used when numbering walls for saving them out */
//static WORD num_walls;          /* Number of walls saved to file */

SHORT MapMaxX = -32767;			/* maximum X value of map */
SHORT MapMaxY = -32767;			/* maximum Y value of map */
SHORT MapMinX = 32767;			/* minimum X value of map */
SHORT MapMinY = 32767;			/* minimum Y value of map */
BOOL MadeChanges = FALSE;		/* made changes? */
BOOL MadeMapChanges = FALSE;	/* made changes that need rebuilding? */

// Constants for roo file
BYTE room_magic[] = { 0x52, 0x4F, 0x4F, 0xB1 };

static int room_version;   // Version of file we're reading

static FileSideDef *FileSideDefs;  // Array of sidedef info read in from file
static WORD NumFileSideDefs;

static Bool LoadRoom(int infile);
static BOOL ReadSlopeInfo(int infile, SlopeInfo *info);

/***************************************************************************/
/*
 * ReadLevelData: Read in the given room file, and extract Vertices, LineDefs,
 *   SideDefs, and Sectors.
 */
void ReadLevelData (char *levelname)
{
   int infile;

   infile = open(levelname, O_BINARY | O_RDONLY);
   if (infile < 0)
   {
      Notify("Error opening file %s", levelname);
		return;
   }
   if (LoadRoom(infile) == FALSE)
      Notify("Error reading file %s", levelname);

   close(infile);

   return;
}
/***************************************************************************/
/*
 * AddVertex:  Add given vertex if it doesn't already appear in Vertexes.
 *   Return # of vertex.
 */
SHORT AddVertex(SHORT x, SHORT y)
{
   SHORT i;

   for (i=0; i < NumVertexes; i++)
      if (Vertexes[i].x == x && Vertexes[i].y == y)
	 return i;

   Vertexes[NumVertexes].x = x;
   Vertexes[NumVertexes].y = y;

   NumVertexes++;

   if (x < MapMinX) MapMinX = x;
   if (x > MapMaxX) MapMaxX = x;
   if (y < MapMinY) MapMinY = y;
   if (y > MapMaxY) MapMaxY = y;

   return NumVertexes - 1;
}
/***************************************************************************/
/*
 * FindVertex:  Return the # of the vertex with the given coordinate, or -1 if none.
 */
SHORT FindVertex(SHORT x, SHORT y)
{
   SHORT i;

   for (i=0; i < NumVertexes; i++)
      if (Vertexes[i].x == x && Vertexes[i].y == y)
	 return i;
	return - 1;
}
/***************************************************************************/
/*
 * CopySideDefFromFile:  Copy info from FileSideDef to SideDef.
 */
void CopySideDefFromFile(SideDef *SD, FileSideDef *FSD)
{
   TextureInfo *info;

   SD->type1 = FSD->type_above;
   SD->type2 = FSD->type_below;
   SD->type3 = FSD->type_normal;
   
   SD->animate_speed = FSD->animate_speed;
   SD->user_id = FSD->id;

   // Fill in string names of textures
   info = FindTextureByNumber(SD->type1);
   if (info != NULL)
      strcpy(SD->tex1, info->Name);

   info = FindTextureByNumber(SD->type2);
   if (info != NULL)
      strcpy(SD->tex2, info->Name);

   info = FindTextureByNumber(SD->type3);
   if (info != NULL)
      strcpy(SD->tex3, info->Name);
}
/***************************************************************************/
/*
 * CombineFileFlags:  Given flags from + and - sidedefs, return linedef flags.
 */
int CombineFileFlags(int pos_flags, int neg_flags)
{
   int flags = 0;

   if (pos_flags & WF_BACKWARDS)
      flags |= BF_POS_BACKWARDS;
   if (pos_flags & WF_TRANSPARENT)
      flags |= BF_POS_TRANSPARENT;
   if (pos_flags & WF_PASSABLE)
      flags |= BF_POS_PASSABLE;
   if (pos_flags & WF_NOLOOKTHROUGH)
		flags |= BF_POS_NOLOOKTHROUGH;
   if (pos_flags & WF_ABOVE_BOTTOMUP)
      flags |= BF_POS_ABOVE_BUP;
   if (pos_flags & WF_BELOW_TOPDOWN)
      flags |= BF_POS_BELOW_TDOWN;
   if (pos_flags & WF_NORMAL_TOPDOWN)
      flags |= BF_POS_NORMAL_TDOWN;
   if (pos_flags & WF_NO_VTILE)
      flags |= BF_POS_NO_VTILE;

   if (neg_flags & WF_BACKWARDS)
      flags |= BF_NEG_BACKWARDS;
   if (neg_flags & WF_TRANSPARENT)
      flags |= BF_NEG_TRANSPARENT;
   if (neg_flags & WF_PASSABLE)
      flags |= BF_NEG_PASSABLE;
   if (neg_flags & WF_NOLOOKTHROUGH)
      flags |= BF_NEG_NOLOOKTHROUGH;
   if (neg_flags & WF_ABOVE_BOTTOMUP)
      flags |= BF_NEG_ABOVE_BUP;
   if (neg_flags & WF_BELOW_TOPDOWN)
      flags |= BF_NEG_BELOW_TDOWN;
   if (neg_flags & WF_NORMAL_TOPDOWN)
      flags |= BF_NEG_NORMAL_TDOWN;
   if (neg_flags & WF_NO_VTILE)
      flags |= BF_NEG_NO_VTILE;

   if ((pos_flags & WF_MAP_NEVER) || (neg_flags & WF_MAP_NEVER))
      flags |= BF_MAP_NEVER;
   if ((pos_flags & WF_MAP_ALWAYS) || (neg_flags & WF_MAP_ALWAYS))
      flags |= BF_MAP_ALWAYS;

   flags |= (WallScrollSpeed(pos_flags) << 20);
   flags |= (WallScrollDirection(pos_flags) << 22);
   flags |= (WallScrollSpeed(neg_flags) << 25);
   flags |= (WallScrollDirection(neg_flags) << 27);

   return flags;
}
/***************************************************************************/
/*
 * SetSideDefs:  Fix linedefs and sidedefs to reflect contents of file sidedefs
 */
void SetSideDefs(void)
{
	int i, file_sd;
   SideDef *SD;
   int flags1, flags2;

   for (i=0; i < NumLineDefs; i++)
   {
      flags1 = flags2 = 0;
      LineDef *LD = &LineDefs[i];

		file_sd = LD->file_sidedef1 - 1;
      if (file_sd == -1)
	 LD->sidedef1 = -1;
      else
      {
	 // Copy file sidedef info into sidedef and linedef
	 SD = &SideDefs[LD->sidedef1];

	 CopySideDefFromFile(SD, &FileSideDefs[file_sd]);
	 flags1 = FileSideDefs[file_sd].flags;
      }

      file_sd = LD->file_sidedef2 - 1;
      if (file_sd == -1)
	 LD->sidedef2 = -1;
      else
      {
	 // Copy file sidedef info into sidedef and linedef
	 SD = &SideDefs[LD->sidedef2];

	 CopySideDefFromFile(SD, &FileSideDefs[file_sd]);
	 flags2 = FileSideDefs[file_sd].flags;
      }

      LD->blak_flags = CombineFileFlags(flags1, flags2);
   }
}
/***************************************************************************/
/*
 * LoadRoom:  Load the room editor part of the room file, and set Vertices,
 *   Linedefs, etc.
 *   Return True on success.
 */
BOOL LoadRoom(int infile)
{
   int i, j, temp;
	BYTE byte;
   long node_pos, cwall_pos, rwall_pos, sector_pos, thing_pos, sidedef_pos;

   // Check magic number and version
   for (i = 0; i < 4; i++)
      if (read(infile, &byte, 1) != 1 || byte != room_magic[i])
	 return FALSE;

   if (read(infile, &room_version, 4) != 4 || room_version > ROO_VERSION)
		return FALSE;

	// Skip room security check
	if (read(infile, &temp, 4) != 4) return FALSE;

	// Seek to main section of the file
	if (read(infile, &temp, 4) != 4) return FALSE;
	lseek(infile, temp, SEEK_SET);

	// Skip width and height
	if (read(infile, &temp, 4) != 4) return FALSE;
	if (read(infile, &temp, 4) != 4) return FALSE;

	// Read subsection locations
	if (read(infile, &node_pos, 4) != 4) return FALSE;
	if (read(infile, &cwall_pos, 4) != 4) return FALSE;
	if (read(infile, &rwall_pos, 4) != 4) return FALSE;
	if (read(infile, &sidedef_pos, 4) != 4) return FALSE;
	if (read(infile, &sector_pos, 4) != 4) return FALSE;
	if (read(infile, &thing_pos, 4) != 4) return FALSE;

	NumThings = 0;
	NumVertexes = 0;
	NumLineDefs = 0;
	NumSideDefs = 0;
	NumSectors = 0;
	NumFileSideDefs = 0;

	MapMaxX = MAP_MIN_X;
	MapMaxY = MAP_MIN_Y;
	MapMinX = MAP_MAX_X;
	MapMinY = MAP_MAX_Y;

	// *** Read linedefs
	lseek(infile, rwall_pos, SEEK_SET);
	if (read(infile, &NumLineDefs, 2) != 2) return FALSE;
	if (NumLineDefs < 1)
	{
		ProgError("While loading roo file it specified %d Line Defs",(int)NumLineDefs);
	}
   LineDefs = (LDPtr)GetMemory (NumLineDefs * sizeof (LineDef));
   SideDefs = (SDPtr)GetMemory (2 * NumLineDefs * sizeof (SideDef));
   Vertexes = (VPtr)GetMemory (2 * NumLineDefs * sizeof (Vertex));

   NumSideDefs = 2 * NumLineDefs;

   for (i=0; i < NumLineDefs; i++)
   {
		SHORT x0, x1, y0, y1;
      SideDef *new_sd1, *new_sd2;

      // Read sidedef info
      if (read(infile, &LineDefs[i].file_sidedef1, 2) != 2) return FALSE;
      if (read(infile, &LineDefs[i].file_sidedef2, 2) != 2) return FALSE;

      // Add new sidedefs
      new_sd1 = &SideDefs[2*i];
      new_sd2 = &SideDefs[2*i + 1];
      LineDefs[i].sidedef1 = 2*i;
      LineDefs[i].sidedef2 = 2*i + 1;

      if (read(infile, &new_sd1->xoff, 2) != 2) return FALSE;
      if (read(infile, &new_sd2->xoff, 2) != 2) return FALSE;
      if (read(infile, &new_sd1->yoff, 2) != 2) return FALSE;
      if (read(infile, &new_sd2->yoff, 2) != 2) return FALSE;

      if (read(infile, &new_sd1->sector, 2) != 2) return FALSE;
      if (read(infile, &new_sd2->sector, 2) != 2) return FALSE;


      // Read endpoints
      if (read(infile, &temp, 4) != 4) return FALSE;
      x0 = temp;
      if (read(infile, &temp, 4) != 4) return FALSE;
      y0 = temp;
      if (read(infile, &temp, 4) != 4) return FALSE;
      x1 = temp;
      if (read(infile, &temp, 4) != 4) return FALSE;
      y1 = temp;
      LineDefs[i].start = AddVertex(x0, y0);
      LineDefs[i].end   = AddVertex(x1, y1);
   }   


   // *** Read file sidedefs
   lseek(infile, sidedef_pos, SEEK_SET);
   if (read(infile, &NumFileSideDefs, 2) != 2) return FALSE;
   if (NumFileSideDefs != 0)
      FileSideDefs = (FileSideDef *) GetMemory (NumFileSideDefs * sizeof (FileSideDef));
   for (i=0; i < NumFileSideDefs; i++)
   {
      FileSideDef *fsd = &FileSideDefs[i];

      if (read(infile, &fsd->id, 2) != 2) return FALSE;
      if (read(infile, &fsd->type_normal, 2) != 2) return FALSE;
      if (read(infile, &fsd->type_above, 2) != 2) return FALSE;
      if (read(infile, &fsd->type_below, 2) != 2) return FALSE;
      if (read(infile, &fsd->flags, 4) != 4) return FALSE;
      if (read(infile, &fsd->animate_speed, 1) != 1) return FALSE;
   }

   // Set editor sidedefs to reflect file sidedefs
   SetSideDefs();

   // *** Read sectors
   lseek(infile, sector_pos, SEEK_SET);
   if (read(infile, &NumSectors, 2) != 2) return FALSE;
   if (NumSectors != 0)
      Sectors = (SPtr)GetMemory (NumSectors * sizeof (Sector));
   for (i=0; i < NumSectors; i++)
   {
      WORD word;

      if (read(infile, &Sectors[i].user_id, 2) != 2) return FALSE;
      if (read(infile, &word, 2) != 2) return FALSE;
      Sectors[i].floor_type = word;
      if (read(infile, &word, 2) != 2) return FALSE;
      Sectors[i].ceiling_type = word;
      
      if (read(infile, &word, 2) != 2) return FALSE;
      Sectors[i].xoffset = word;
      if (read(infile, &word, 2) != 2) return FALSE;
      Sectors[i].yoffset = word;
      
      if (read(infile, &word, 2) != 2) return FALSE;
      Sectors[i].floorh = word;
      if (read(infile, &word, 2) != 2) return FALSE;
      Sectors[i].ceilh = word;

      if (read(infile, &Sectors[i].light, 1) != 1) return FALSE;
      if (read(infile, &Sectors[i].blak_flags, 4) != 4) return FALSE;

      // XXX Old version
      if (room_version >= 10)
	 if (read(infile, &Sectors[i].animate_speed, 1) != 1) return FALSE;

      // Load slope information, if appropriate
      for (j=0; j < 3; j++)
      {
	 Sectors[i].floor_slope.points[j].vertex = -1;
	 Sectors[i].floor_slope.points[j].z = -1;
	 Sectors[i].ceiling_slope.points[j].vertex = -1;
	 Sectors[i].ceiling_slope.points[j].z = -1;
      }
      if (Sectors[i].blak_flags & SF_SLOPED_FLOOR)
	 if (!ReadSlopeInfo(infile, &Sectors[i].floor_slope))
	    return FALSE;
      if (Sectors[i].blak_flags & SF_SLOPED_CEILING)
	 if (!ReadSlopeInfo(infile, &Sectors[i].ceiling_slope))
	    return FALSE;

      // Find string name of textures
      TextureInfo *info = FindTextureByNumber(Sectors[i].floor_type);
		if (info != NULL)
	 strcpy(Sectors[i].floort, info->Name);

		info = FindTextureByNumber(Sectors[i].ceiling_type);
		if (info != NULL)
	 strcpy(Sectors[i].ceilt, info->Name);
	}

	// *** Read things
	lseek(infile, thing_pos, SEEK_SET);
	if (read(infile, &NumThings, 2) != 2) return FALSE;

	if (NumThings > 2) // new format
	{
		if (NumThings != 0)
			Things = (TPtr)GetMemory (NumThings * sizeof(Thing));
		memset(Things,0,NumThings * sizeof(Thing));
		for (i=0; i < NumThings; i++)
		{
			char comments[64];

			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].type = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].angle = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].xpos = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].ypos = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].when = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].xExitPos = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].yExitPos = temp;
			if (read(infile, &temp, 4) != 4) return FALSE;
			Things[i].flags = temp;
			if (read(infile, comments, 64) != 64) return FALSE;
			strcpy(Things[i].comment,comments);
		}
		if (read(infile, &temp, 4) != 4) return FALSE;
		RoomID = temp;
	}
	else // with only two "things" then we are loading an old file
	{
		if (NumThings > 0)
		{
			int size = NumThings * sizeof(Thing);
			Things = (TPtr)GetMemory (size);
			memset(Things,0,size); // start with everything cleared
			for (i=0; i < NumThings; i++)
			{
				Things[i].type = kodPlayerBlocking;
				if (read(infile, &temp, 4) != 4) return FALSE;
				Things[i].xpos = temp;
				if (read(infile, &temp, 4) != 4) return FALSE;
				Things[i].ypos = temp;
			}
		}
		else
			MessageBox(NULL,"Warning!  There are no 'Things' defined in this ROO file.  Please add Player bounding boxes!!","Load ROO error",MB_OK);
		RoomID = 0;
	}

	// Fill in MapMinX, etc.
	FindRoomBoundary();

	SafeFree(FileSideDefs);
	LogMessage("Got %d lines, %d sectors, %d things\n", NumLineDefs, NumSectors, NumThings);
   
   return TRUE;
}


/*
   forget the level data
*/

void ForgetLevelData() /* SWAP! */
{
	/* forget the Things */
	NumThings = 0;
	if (Things)
		FreeMemory (Things);
	Things = NULL;

	/* forget the Vertices */
	NumVertexes = 0;
	if (Vertexes)
		FreeMemory (Vertexes);
	Vertexes = NULL;

	/* forget the LineDefs */
	NumLineDefs = 0;
	if (LineDefs)
		FreeMemory (LineDefs);
	LineDefs = NULL;

	/* forget the SideDefs */
	NumSideDefs = 0;
	if (SideDefs)
		FreeMemory (SideDefs);
	SideDefs = NULL;

	/* forget the Sectors */
	NumSectors = 0;
	if (Sectors)
		FreeMemory (Sectors);
	Sectors = NULL;

	// Don't do this
	//BUG: If we create a new level, ForgetLevelData is called, but
	//     we want the Level var. still to point to the level entry in the
	//     Master directory.
	//     Thanks to ??? for signaling the bug.
	// Level = NULL;
}

/***************************************************************************/
/*
 * ReadSlopeInfo:  Read a SlopeInfo structure from the given file.
 *   Return TRUE on success.
 */
BOOL ReadSlopeInfo(int infile, SlopeInfo *info)
{
   int x, y;
   int i;
   SHORT z;

   if (!read(infile, &info->plane.a, 4)) return FALSE;
   if (!read(infile, &info->plane.b, 4)) return FALSE;
   if (!read(infile, &info->plane.c, 4)) return FALSE;
   if (!read(infile, &info->plane.d, 4)) return FALSE;
   if (!read(infile, &info->x, 4)) return FALSE;
   if (!read(infile, &info->y, 4)) return FALSE;
   if (!read(infile, &info->angle, 4)) return FALSE;
   info->angle = info->angle * 360 / NUMDEGREES;

   for (i=0; i < 3; i++)
   {
		if (!read(infile, &x, 2)) return FALSE;
      if (!read(infile, &y, 2)) return FALSE;
      info->points[i].vertex = FindVertex(x, y);
      if (!read(infile, &z, 2)) return FALSE;
      info->points[i].z = z;      
   }

   return TRUE;
}
/***************************************************************************/
/*
 * FindRoomBoundary:  Fill in MapMinX, MapMinY, MapMaxX, and MapMaxY with coordinates
 *   of boundary vertices.
 */
void FindRoomBoundary(void)
{
	SHORT       n;

	MapMaxX = MAP_MIN_X;
	MapMaxY = MAP_MIN_Y;
	MapMinX = MAP_MAX_X;
	MapMinY = MAP_MAX_Y;
	for (n = 0; n < NumVertexes; n++)
	{
		Vertex ve = Vertexes[n];
		if (ve.x < MapMinX)   MapMinX = ve.x;
		if (ve.x > MapMaxX)   MapMaxX = ve.x;
		if (ve.y < MapMinY)   MapMinY = ve.y;
		if (ve.y > MapMaxY)   MapMaxY = ve.y;
	}
}

/* utility functions */

/***************************************************************************/
/*
 * GetRoomSubRect:  Compute and return the area of the room enclosed by the bounding box
 *   of the Things in the room.
 *   If at least 2 Things are in the room, compute the rectangle and return TRUE.
 *   Otherwise return FALSE.
 */
BOOL GetRoomSubRect(RECT *rect)
{
   if (NumThings < 2)
		return FALSE;

	rect->left   = Things[0].xpos;
	rect->right  = Things[0].xpos;
	rect->top    = Things[0].ypos;
	rect->bottom = Things[0].ypos;
	rect->left   = min(rect->left, Things[1].xpos);
	rect->right  = max(rect->right, Things[1].xpos);
	rect->top    = max(rect->top, Things[1].ypos);
	rect->bottom = min(rect->bottom, Things[1].ypos);

	return TRUE;
}
/***************************************************************************/
/*
 * GetServerCoords:  Given editor coordinates, set x and y to server coordinates.
 */
void GetServerCoords(int *x, int *y, int *xoffset, int *yoffset)
{
   RECT r;
   int old_x, old_y;

   old_x = *x;
   old_y = *y;

   if (GetRoomSubRect(&r) == FALSE)
   {
      r.left = MapMinX;
      r.top  = MapMaxY;
   }

   // Server coordinates start at (1, 1)
   *x = 1 + (old_x - r.left) / 64;
   *y = 1 + (r.top - old_y) / 64;

   *xoffset = (old_x - r.left) % 64;
   *yoffset = (r.top - old_y) % 64;
}

