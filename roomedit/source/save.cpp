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

static int min_player_distance = (FINENESS / 4);  // Minimum distance player is allowed to get to wall
static int min_monster_distance = (FINENESS / 16);  // Minimum distance monster is allowed to get to wall

static int NumNodes;            /* # of BSP nodes */
static int NumFileSideDefs;     /* # of sidedefs in save file */
static int node_num;            /* Used when numbering nodes for saving them out */
static WORD wall_num;           /* Used when numbering walls for saving them out */
static WORD num_walls;          /* Number of walls saved to file */
static int security;            /* Room security checksum, built up as we save file */

FileSideDef *FileSideDefs;  /* List of file sidedefs to save */

// Constants for roo file
static BYTE room_magic[] = { 0x52, 0x4F, 0x4F, 0xB1 };

static void SaveBoundingBox(FILE *file, Box *box);
static void FileBackpatch(FILE *file, int pos, int data);
static WallDataList AddWallsFromLinedef(WallDataList walls, LineDef *l, int num);
static Bool BackupFile(char *fname);
static void SetWallHeights(WallData *wall);
static void FreeFileSideDefs(void);
static void WriteSlopeInfo(FILE *file, SlopeInfo *info, int floor);

//
// Modeless Dialog Box for showing progress while saving level data
//
static TLevelProgressDialog *pProgressDialog = NULL;

void ShowProgress (int objtype)
{
	assert (pProgressDialog != NULL);

	pProgressDialog->ShowNodesProgress (objtype);
}


/***************************************************************************/
/*
 * NumberNodes:  Fill in num fields of nodes in tree, so that the
 *   number of a node gives the order in which it appears in the file.
 *   Also number walls in the order they appear in the file.
 */
void NumberNodes(BSPTree tree)
{
   WallData *wall;

   if (tree == NULL)
      return;

   tree->num = node_num++;

   if (tree->type == BSPinternaltype)
   {
      // Number parent, then left child, then right child
      
      BSPinternal *inode = &tree->u.internal;
      
      for (wall = inode->walls_in_plane; wall != NULL; wall = wall->next)
	 wall->num = wall_num++;
      
      NumberNodes(inode->pos_side);
      NumberNodes(inode->neg_side);   
   }
}
/***************************************************************************/
/*
 * SaveNodes:  Write out BSP nodes to file.
 */
void SaveNodes(FILE *file, BSPTree tree)
{
   int temp, i;
   WallData *wall;
   BYTE byte;
   BSPinternal *inode;
   BSPleaf *leaf;
   WORD word;

   if (tree == NULL)
      return;

   switch (tree->type)
   {
   case BSPinternaltype:
      inode = &tree->u.internal;

      // Type of node
      byte = 1;
      WriteBytes(file, &byte, 1);

      SaveBoundingBox(file, &tree->bbox);

      // Write parent, then left child, then right child
      
      // Plane of node
      WriteBytes(file, &inode->separator.a, 4);
      WriteBytes(file, &inode->separator.b, 4);
      WriteBytes(file, &inode->separator.c, 4);

      security += inode->separator.a + inode->separator.b + inode->separator.c;
      
      // Node numbers of children
      word = 0;
      if (inode->pos_side != NULL)
	 word = inode->pos_side->num;
      WriteBytes(file, &word, 2);

      word = 0;
      if (inode->neg_side != NULL)
	 word = inode->neg_side->num;
      WriteBytes(file, &word, 2);
      
      // Number of first wall in list
      word = 0;
      wall = inode->walls_in_plane;
      if (wall != NULL)
	 word = wall->num;
      WriteBytes(file, &word, 2);
      security += word;
      
      SaveNodes(file, inode->pos_side);
      SaveNodes(file, inode->neg_side);   
      break;
      
   case BSPleaftype:
      leaf = &tree->u.leaf;

      // Type of node
      byte = 2;
      WriteBytes(file, &byte, 1);

      SaveBoundingBox(file, &tree->bbox);

      word = leaf->sector + 1;  // 0 = no sector
      WriteBytes(file, &word, 2);
      
      short num_points = leaf->poly.npts;
      WriteBytes(file, &num_points, 2);

      for (i=0; i < num_points; i++)
      {
	 temp = leaf->poly.p[i].x;
	 WriteBytes(file, &temp, 4);
	 security += temp;
	 temp = leaf->poly.p[i].y;
	 WriteBytes(file, &temp, 4);
	 security += temp;
      }
      break;
   }
}
/***************************************************************************/
/*
 * SaveBoundingBox:  Write bounding box to file.
 */
void SaveBoundingBox(FILE *file, Box *box)
{
   WriteBytes(file, &box->x0, 4);
   WriteBytes(file, &box->y0, 4);
   WriteBytes(file, &box->x1, 4);
   WriteBytes(file, &box->y1, 4);
}
/***************************************************************************/
/*
 * SaveClientWalls:  Write out list of client walls in tree to file.
 */
void SaveClientWalls(FILE *file, BSPTree tree)
{
   if (tree == NULL)
      return;
   if (tree->type != BSPinternaltype)
      return;
   
   BSPinternal *inode = &tree->u.internal;
   WallData *wall, *next_wall;
   WORD word;
   
   for (wall = inode->walls_in_plane; wall != NULL; wall = wall->next)
   {
      num_walls++;

      // Next wall num in list
      next_wall = wall->next;
      word = 0;
      if (next_wall != NULL)
	 word = next_wall->num;
      WriteBytes(file, &word, 2);

      // Sidedef numbers
      WriteBytes(file, &wall->pos_sidedef, 2);
      WriteBytes(file, &wall->neg_sidedef, 2);
      security += wall->pos_sidedef + wall->neg_sidedef;

      // Start and end of wall
      WriteBytes(file, &wall->x0, 4);
      WriteBytes(file, &wall->y0, 4);
      WriteBytes(file, &wall->x1, 4);
      WriteBytes(file, &wall->y1, 4);
      security += wall->x0 + wall->y0 + wall->x1 + wall->y1;

      // Length of wall
      word = wall->length;
      WriteBytes(file, &word, 2);

      // Texture offsets
      word = wall->pos_xoffset;
      WriteBytes(file, &word, 2);
      word = wall->neg_xoffset;
      WriteBytes(file, &word, 2);      
      word = wall->pos_yoffset;
      WriteBytes(file, &word, 2);
      word = wall->neg_yoffset;
      WriteBytes(file, &word, 2);

      // Adjacent sector numbers (0 = none, 1 = first one in file)
      word = wall->pos_sector + 1;
      WriteBytes(file, &word, 2);
      security += word;
      word = wall->neg_sector + 1;
      WriteBytes(file, &word, 2);
      security += word;
   }
   
   SaveClientWalls(file, inode->pos_side);
   SaveClientWalls(file, inode->neg_side);   
}
/***************************************************************************/
/*
 * SaveSideDefs:  Write out list of sidedefs to file
 */
void SaveSideDefs(FILE *file)
{
   FileSideDef *ptr;

   for (ptr = FileSideDefs; ptr != NULL; ptr = ptr->next)
   {
      WriteBytes(file, &ptr->id, 2);
      WriteBytes(file, &ptr->type_normal, 2);
      WriteBytes(file, &ptr->type_above, 2);
      WriteBytes(file, &ptr->type_below, 2);
      WriteBytes(file, &ptr->flags, 4);
      WriteBytes(file, &ptr->animate_speed, 1);
      security += ptr->id + ptr->type_normal + ptr->type_above + ptr->type_below +
	 ptr->flags;
   }
}
/***************************************************************************/
/*
 * SaveSectors:  Write out list of sectors to client part of file
 */
void SaveSectors(FILE *file)
{
   int i;
   WORD word;
   BYTE byte;

   for (i=0; i < NumSectors; i++)
   {
      Sector CurSector = Sectors[i];

      // Sector user-id
      word = CurSector.user_id;
      WriteBytes(file, &word, 2);
      security += word;

      word = CurSector.floor_type;
      WriteBytes(file, &word, 2);
      security += word;
      word = CurSector.ceiling_type;
      WriteBytes(file, &word, 2);
      security += word;
      word = CurSector.xoffset;
      WriteBytes(file, &word, 2);
      word = CurSector.yoffset;
      WriteBytes(file, &word, 2);
      word = CurSector.floorh;
      WriteBytes(file, &word, 2);
      security += word;
      word = CurSector.ceilh;
      WriteBytes(file, &word, 2);
      security += word;
      byte = CurSector.light;
      WriteBytes(file, &byte, 1);
      security += byte;
      WriteBytes(file, &CurSector.blak_flags, 4);
      security += CurSector.blak_flags;
      WriteBytes(file, &CurSector.animate_speed, 1);

      // Write slope information, if appropriate
      if (CurSector.blak_flags & SF_SLOPED_FLOOR)
	 WriteSlopeInfo(file, &CurSector.floor_slope, 1);
      if (CurSector.blak_flags & SF_SLOPED_CEILING)
	 WriteSlopeInfo(file, &CurSector.ceiling_slope, 0);
   }
}
/***************************************************************************/
/*
 * SaveServerInfo: Write out server grid info to given file.
 *   width and height are in client (FINENESS) units.
 */
void SaveServerInfo(FILE *file, BSPTree tree, int width, int height)
{
   BYTE b;
   int rows, cols, i, j, flags;

   rows = height / FINENESS;
   cols = width / FINENESS;

   WriteBytes(file, &rows, 4);
   WriteBytes(file, &cols, 4);

   // Write inter-square movement grid for player
   for (i=0; i < rows; i++)
   {
     for (j=0; j < cols; j++)
     {
       b = ComputeMoveFlags(tree, i, j, rows, cols, min_player_distance);
       WriteBytes(file, &b, 1);
       LogMessage("%2x ", b);
     }
     LogMessage("\n");
     pProgressDialog->ShowRejectProgress(i * 100 / rows);
   }

   // Write square flags
   for (i=0; i < rows; i++)
   {
     for (j=0; j < cols; j++)
     {
       b = ComputeSquareFlags(tree, i, j, rows, cols);
       WriteBytes(file, &b, 1);
//	 LogMessage("%2x ", b);
     }
//      LogMessage("\n");
     pProgressDialog->ShowBlockmapProgress(i * 100 / rows);
   }

   // Write inter-square movement grid for monsters
   for (i=0; i < rows; i++)
   {
     for (j=0; j < cols; j++)
     {
       b = ComputeMoveFlags(tree, i, j, rows, cols, min_monster_distance);
       WriteBytes(file, &b, 1);
       LogMessage("%2x ", b);
     }
     LogMessage("\n");
     pProgressDialog->ShowRejectProgress(i * 100 / rows);
   }
   
   /*********** NEW HIGH RESOLUTION GRID ********************/
   
   rows = height / FINENESSHIGHRESGRID;
   cols = width / FINENESSHIGHRESGRID;

   WriteBytes(file, &rows, 4);
   WriteBytes(file, &cols, 4);

   // Write inter-square movement grid for player
   for (i=0; i < rows; i++)
   {
     for (j=0; j < cols; j++)
     {
       flags = ComputeHighResSquareFlags(tree, i, j, rows, cols, min_player_distance);
       WriteBytes(file, &flags, 4);
       //LogMessage("%2x ", b);
     }
     //LogMessage("\n");
     pProgressDialog->ShowRejectProgress(i * 100 / rows);
   }
}
/***************************************************************************/
/*
 * SaveRoomeditWalls:  Write out wall info needed by room editor when file is reloaded.
 */
void SaveRoomeditWalls(FILE *file)
{
   int i;
   SideDef SD1, SD2;
   WORD word;
   Vertex VStart, VEnd;

   // Write out walls
   for (i=0; i < NumLineDefs; i++)
   {
      LineDef CurLD = LineDefs[i];
      if (CurLD.sidedef1 >= 0)
        SD1 = SideDefs[CurLD.sidedef1];
      else
        CurLD.file_sidedef1 = 0;

      if (CurLD.sidedef2 >= 0)
        SD2 = SideDefs[CurLD.sidedef2];
      else
        CurLD.file_sidedef2 = 0;

      // Write sidedef numbers
      WriteBytes(file, &CurLD.file_sidedef1, 2);
      WriteBytes(file, &CurLD.file_sidedef2, 2);

      // Write texture offsets
      word = 0;
      if (CurLD.sidedef1 >= 0)
	 word = SD1.xoff;
      WriteBytes(file, &word, 2);

      word = 0;
      if (CurLD.sidedef2 >= 0)
	 word = SD2.xoff;
      WriteBytes(file, &word, 2);

      word = 0;
      if (CurLD.sidedef1 >= 0)
	 word = SD1.yoff;
      WriteBytes(file, &word, 2);

      word = 0;
      if (CurLD.sidedef2 >= 0)
	 word = SD2.yoff;
      WriteBytes(file, &word, 2);

      // Write sectors
      word = -1;
      if (CurLD.sidedef1 >= 0)
	 word = SD1.sector;
      WriteBytes(file, &word, 2);

      word = -1;
      if (CurLD.sidedef2 >= 0)
	 word = SD2.sector;
      WriteBytes(file, &word, 2);

      VStart = Vertexes[CurLD.start];
		VEnd   = Vertexes[CurLD.end];
		WriteBytes(file, &VStart.x, 4);
		WriteBytes(file, &VStart.y, 4);
		WriteBytes(file, &VEnd.x, 4);
		WriteBytes(file, &VEnd.y, 4);
	}
}
/***************************************************************************/
void SaveThings(FILE *file)
{
	int i, temp;

	// Write out things
	for (i=0; i < NumThings; i++)
	{
		char comments[64];
		Thing CurThing = Things[i];

		if (NumThings <= 2)
		{
			temp = CurThing.xpos;
			WriteBytes(file, &temp, 4);

			temp = CurThing.ypos;
			WriteBytes(file, &temp, 4);
		}
		else // new save
		{
			temp = CurThing.type;
			WriteBytes(file, &temp, 4);

			temp = CurThing.angle;
			WriteBytes(file, &temp, 4);

			temp = CurThing.xpos;
			WriteBytes(file, &temp, 4);

			temp = CurThing.ypos;
			WriteBytes(file, &temp, 4);

			temp = CurThing.when;
			WriteBytes(file, &temp, 4);

			temp = CurThing.xExitPos;
			WriteBytes(file, &temp, 4);

			temp = CurThing.yExitPos;
			WriteBytes(file, &temp, 4);

			temp = CurThing.flags;
			WriteBytes(file, &temp, 4);

			strncpy(comments,CurThing.comment,63);
			comments[63] = '\0';
			WriteBytes(file, comments, 64);
		}
	}
	temp = RoomID;
	WriteBytes(file, &temp, 4);
}

static void AppendFile(LPCSTR filename, LPCSTR line)
{
	FILE *file = fopen(filename,"at"); // append text file
	if (file)
	{
		fputs(line,file);
		fputs("\n",file);
		fclose(file);
	}
}

#define KOD_ANGLE 4096

static void SaveKodFiles(LPCSTR rooFilename)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char file[_MAX_FNAME];
	char ext[_MAX_EXT];
	char itemName[_MAX_PATH];
	char exitName[_MAX_PATH];
	char monsterName[_MAX_PATH];
	char batchName[_MAX_PATH];
	char buffer[512];
	int i;

	_splitpath(rooFilename,drive,dir,file,ext);
	strcpy(itemName,drive);
	strcat(itemName,dir);
	strcat(itemName,file);
	strcpy(exitName,itemName);
	strcpy(monsterName,itemName);
	//strcpy(batchName,"\\Meridian58\\croot\\blakrun\\server\\");
	strcpy(batchName,ServerDir);
	strcat(batchName,file);
	strcat(batchName,".txt");
	strcat(itemName,".ktm");
	strcat(exitName,".kxt");
	strcat(monsterName,".kmn");
	unlink(itemName);
	unlink(exitName);
	unlink(monsterName);
	unlink(batchName);

	AppendFile(itemName,"% Item KOD Generation file");
	AppendFile(monsterName,"% Monster Generator KOD Generation file");
	AppendFile(monsterName,"\n\tplGenerators = $;\n");
	AppendFile(exitName,"% Exit KOD generation file");
	AppendFile(exitName,"\n\tplExits = $;\n");

	wsprintf(buffer,"send object 0 CleanOutRoom rid int %s",
		GetKodRoomNameByRoomID(RoomID));
	AppendFile(batchName,buffer);
	for (i=0; i < NumThings; i++)
	{
		char direction[64];
		int x,y,xOffset,yOffset;
		Thing CurThing = Things[i];

		x = CurThing.xpos;
		y = CurThing.ypos;
		GetServerCoords(&x,&y,&xOffset,&yOffset);
		switch (CurThing.type)
		{
		case kodPlayerBlocking:
			break;
		case kodExit:
			switch(CurThing.angle)
			{
			default:
			case   0: strcpy(direction,"ROTATE_NONE"); break;
			case  45: strcpy(direction,"ROTATE_315"); break;
			case  90: strcpy(direction,"ROTATE_270"); break;
			case 135: strcpy(direction,"ROTATE_225"); break;
			case 180: strcpy(direction,"ROTATE_180"); break;
			case 225: strcpy(direction,"ROTATE_135"); break;
			case 270: strcpy(direction,"ROTATE_90"); break;
			case 315: strcpy(direction,"ROTATE_45"); break;
			}
			wsprintf(buffer,"\tplExits = Cons([%d,%d,%s,%d,%d,%s],plExits);",
				y,x,GetKodRoomNameByRoomID(CurThing.when),
				(int)CurThing.xExitPos,(int)CurThing.yExitPos,direction);
			AppendFile(exitName,buffer);
			break;
		case kodMonsterGenerator:
			wsprintf(buffer,"\tplGenerators = Cons([%d,%d],plGenerators);",y,x);
			AppendFile(monsterName,buffer);
			break;
		case kodEntrance:
			Things[i].xExitPos = xOffset;
			Things[i].yExitPos = yOffset;
			break;
		case kodExtraPreObject2:
		case kodExtraPreObject3:
		case kodExtraPreObject4:
			break;
		case kodObject:
		default:
			if (CurThing.type >= kodObject)
			{
				switch(CurThing.angle)
				{
				default:
				case   0: strcpy(direction,"ANGLE_EAST"); break;
				case  45: strcpy(direction,"ANGLE_NORTH_EAST"); break;
				case  90: strcpy(direction,"ANGLE_NORTH"); break;
				case 135: strcpy(direction,"ANGLE_NORTH_WEST"); break;
				case 180: strcpy(direction,"ANGLE_WEST"); break;
				case 225: strcpy(direction,"ANGLE_SOUTH_WEST"); break;
				case 270: strcpy(direction,"ANGLE_SOUTH"); break;
				case 315: strcpy(direction,"ANGLE_SOUTH_EAST"); break;
				}
				if (CurThing.flags & THING_FLAG_DONTGENERATE)
				{
					wsprintf(buffer,"%% **ASIF**\n%% %s\n%%Send(self,@NewHold,#what=XXX,#new_row=%d,#new_col=%d,#fine_row=%d,#fine_col=%d,#new_angle=%s);",
					CurThing.comment,y,x,yOffset,xOffset,direction,CurThing.comment);
				}
				else if (GetNumKodTypes(CurThing.type) > 0)
				{
					wsprintf(buffer,"\tSend(self,@NewHold,#what=Create(&%s,#type=%s),"
						"#new_row=%d,#new_col=%d,#fine_row=%d,#fine_col=%d,#new_angle=%s);",
						GetKodObjectName(CurThing.type),GetKodTypeName(CurThing.type,CurThing.when),
						y,x,yOffset,xOffset,direction);
				}
				else
				{
					wsprintf(buffer,"\tSend(self,@NewHold,#what=Create(&%s),"
						"#new_row=%d,#new_col=%d,#fine_row=%d,#fine_col=%d,#new_angle=%s);",
						GetKodObjectName(CurThing.type),y,x,yOffset,xOffset,direction);
				}
				AppendFile(itemName,buffer);
				int angle; // server doesn't parse expressions, so we have to send #'s to the batch
				switch(CurThing.angle)
				{
				default:
				case   0: angle = 0; break; //strcpy(direction,"ANGLE_EAST"); break;
				case  45: angle = 7*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_NORTH_EAST"); break;
				case  90: angle = 6*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_NORTH"); break;
				case 135: angle = 5*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_NORTH_WEST"); break;
				case 180: angle = 4*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_WEST"); break;
				case 225: angle = 3*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_SOUTH_WEST"); break;
				case 270: angle = 2*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_SOUTH"); break;
				case 315: angle = 1*KOD_ANGLE/8; break; //strcpy(direction,"ANGLE_SOUTH_EAST"); break;
				}
				if (GetNumKodTypes(CurThing.type) > 0)
				{
					wsprintf(buffer,"Send object 0 PutInRoom rid int %s classtype class %s "
						"row int %d col int %d fine_row int %d fine_col int %d angle int %d type int %s",
						GetKodRoomNameByRoomID(RoomID),GetKodObjectName(CurThing.type),
						y,x,yOffset,xOffset,angle,GetKodTypeName(CurThing.type,CurThing.when));
				}
				else
				{
					wsprintf(buffer,"Send object 0 PutInRoom rid int %s classtype class %s "
						"row int %d col int %d fine_row int %d fine_col int %d angle int %d",
						GetKodRoomNameByRoomID(RoomID),GetKodObjectName(CurThing.type),
						y,x,yOffset,xOffset,angle);
				}
				AppendFile(batchName,buffer);
			}
		}
	}
}

/***************************************************************************/
/*
 * SaveLevelData:  Save current level to given file.
 */
void SaveLevelData (char *outfile)
{
	FILE       *file;
	int         i, width, height, true_minx, true_miny, true_maxx, true_maxy;
	SHORT       n;
	BSPTree     tree;
	int         temp;
	long        main_pos, server_pos, security_pos;
	long        node_pos, cwall_pos, rwall_pos, sector_pos, thing_pos, sidedef_pos;
	WallDataList walls;
	RECT        room_subrect;

	// Backup existent file, if any
	if (BackupFile(outfile) == False)
		return;

	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();

	ClearLog();
	WorkMessage ("Saving data to \"%s\"...", outfile);
	LogMessage ("Saving data to \"%s\"...\n", outfile);

	// Init. to NULL to destory it safely at the end of this function
	pProgressDialog = NULL;

	//
	// open the file
	//
	if ((file = fopen (outfile, "wb")) == NULL)
		ProgError ("Unable to open file \"%s\"", outfile);

	security = 0;

	for (i=0; i < 4; i++)
		WriteBytes(file, &room_magic[i], 1);
   
	temp = ROO_VERSION;
	WriteBytes(file, &temp, 4);
   security += ROO_VERSION;

	// Remember position in file for backpatching, and leave space in file
   security_pos = FileCurPos(file);
	WriteBytes(file, &temp, 4);
	main_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);
   server_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);

	// *** Save main section

   // Backpatch in position of client information
   FileBackpatch(file, main_pos, FileCurPos(file));

   // update MapMinX, MapMinY, MapMaxX, MapMaxY
   FindRoomBoundary();

	// See if we should set apparent room size smaller than actual room size
   if (GetRoomSubRect(&room_subrect) == TRUE)
   {
      true_minx = (MapMinX - room_subrect.left) * BLAK_FACTOR;
		true_miny = (room_subrect.top  - MapMaxY) * BLAK_FACTOR;
      true_maxx = (MapMaxX - room_subrect.left) * BLAK_FACTOR;
      true_maxy = (room_subrect.top - MapMinY) * BLAK_FACTOR;

      MapMinX = room_subrect.left;
      MapMinY = room_subrect.bottom;
      MapMaxX = room_subrect.right;
      MapMaxY = room_subrect.top;
   }
   else
	{
      true_minx = 0 * BLAK_FACTOR;
      true_miny = 0 * BLAK_FACTOR;
      true_maxx = (MapMaxX - MapMinX) * BLAK_FACTOR;
      true_maxy = (MapMaxY - MapMinY) * BLAK_FACTOR;
   }

	// Write out room width and height
	width = (MapMaxX - MapMinX) * BLAK_FACTOR;
   WriteBytes(file, &width, 4);
   height = (MapMaxY - MapMinY) * BLAK_FACTOR;
	WriteBytes(file, &height, 4);

   // Save file positions of subsections
   node_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);
   cwall_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);
	rwall_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);
   sidedef_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);
	sector_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);
   thing_pos = FileCurPos(file);
   WriteBytes(file, &temp, 4);

   // Create progress dialog box (modeless)
   TFrameWindow *MainWindow = ((TApplication *)::Module)->GetMainWindow();
   pProgressDialog = new TLevelProgressDialog(MainWindow);
   pProgressDialog->Create();
	// Diable main window
   MainWindow->EnableWindow(FALSE);
   
   // Build the wall list
	walls = NULL;
	NumFileSideDefs = 0;
	FileSideDefs = NULL;
   for (n = 0; n < NumLineDefs; n++)
   {
      LineDef *CurLD = &LineDefs[n];
      assert_vnum (CurLD->start);
      assert_vnum (CurLD->end);
      assert_vsdnum (CurLD->sidedef1);
		assert_vsdnum (CurLD->sidedef2);

      walls = AddWallsFromLinedef(walls, CurLD, n);
   }
   
   ShowProgress (OBJ_VERTEXES);
   ShowProgress (OBJ_SIDEDEFS);

   LogMessage(": Starting Nodes builder...\n");
   // LogMessage("\tNumber of Vertices: %d\n", NumVertexes);
	// LogMessage("\tNumber of Segs:     %d\n", NumSegs);
   // Notify ("DEBUG: CreateNodes");

	tree = BSPBuildTree(walls, true_minx, true_miny, true_maxx, true_maxy);

	NumNodes = BSPGetNumNodes();
   NumSegs = BSPGetNumWalls();

	ShowProgress (OBJ_SSECTORS);
	LogMessage(": Nodes created OK.\n");
	LogMessage("\tNumber of Nodes:    %d\n", NumNodes);
	LogMessage("\tNumber of Vertices: %d\n", NumVertexes);
	LogMessage("\tNumber of LineDefs: %d\n", NumLineDefs);
   LogMessage("\tNumber of SideDefs: %d\n", NumSideDefs);
   LogMessage("\tNumber of Segs:     %d\n", NumSegs);
   LogMessage("\tNumber of FileSideDefs: %d\n", NumFileSideDefs);
   
   // Number everything in the order they're going to be written to file
	node_num = 1;   // Start counting nodes from 1
   wall_num = 1;   // Start counting walls from 1
   NumberNodes(tree);

	// Output tree nodes
   FileBackpatch(file, node_pos, FileCurPos(file));
   WriteBytes(file, &NumNodes, 2);
   SaveNodes(file, tree);

   // Output client walls
   FileBackpatch(file, cwall_pos, FileCurPos(file));
   WriteBytes(file, &NumSegs, 2);
   num_walls = 0;
   SaveClientWalls(file, tree);

   // Output roomeditor walls
   FileBackpatch(file, rwall_pos, FileCurPos(file));
   WriteBytes(file, &NumLineDefs, 2);
	SaveRoomeditWalls(file);

   // Output sidedefs
	FileBackpatch(file, sidedef_pos, FileCurPos(file));
	WriteBytes(file, &NumFileSideDefs, 2);
	SaveSideDefs(file);

	// Output sectors
	FileBackpatch(file, sector_pos, FileCurPos(file));
   WriteBytes(file, &NumSectors, 2);
   SaveSectors(file);

   // Output things
   FileBackpatch(file, thing_pos, FileCurPos(file));
	WriteBytes(file, &NumThings, 2);
	SaveThings(file);

	// *** Save server section

	// Backpatch in position of server information
	FileBackpatch(file, server_pos, FileCurPos(file));

   // Output inter-square movement grid
   SaveServerInfo(file, tree, width, height);

   // Backpatch in room security check
	security ^= 0x89ab786c;
   dprintf("Room security = %d\n", security);
   FileBackpatch(file, security_pos, security);
   
   // close the file
	fclose (file);

	NumSegs = 0;
	NumSSectors = 0;
   
	BSPFreeTree(tree);
   FreeFileSideDefs();
   
   // the file is now up to date
   //
   MadeChanges = FALSE;
   MadeMapChanges = FALSE;
   
	// Destroy progress dialog box
   if ( pProgressDialog != NULL )
   {
      // Re-enable main window before closing modeless dialog box
		// to avoid Windows giving focus to another application.
      TFrameWindow *MainWindow = ((TApplication *)::Module)->GetMainWindow();
      MainWindow->EnableWindow(TRUE);
      MainWindow->ShowWindow(SW_SHOW);
      // Close dialog box.
      pProgressDialog->CloseWindow();
      delete pProgressDialog;
   }
   
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();

	CloseLog();

  // Removed 7/25/04 ARK
//	SaveKodFiles(outfile);
}
/***************************************************************************/
/*
 * ComputeSlopeInfo:  With the given SlopeInfo structure, assuming that the
 *   vertex and z information is set, fill in the rest of the structure
 *   (i.e. compute the plane equation).
 */
void ComputeSlopeInfo(SlopeInfo *info, int floor)
{
	int i;
	Point3D p[3];
	double u[3], v[3], uv[3], ucrossv;

	// Convert to client coordinates
	for (i=0; i < 3; i++)
	{
		assert_vnum(info->points[i].vertex);
		p[i].x = (Vertexes[info->points[i].vertex].x - MapMinX) * BLAK_FACTOR;
		p[i].y = (MapMaxY - Vertexes[info->points[i].vertex].y) * BLAK_FACTOR;
		p[i].z = info->points[i].z * BLAK_FACTOR;
      dprintf("vertex %d (%d):  %d, %d, %d\n", i, info->points[i].vertex, p[i].x, p[i].y, p[i].z);
   }

   // Take texture origin to be first vertex listed
   info->x = p[0].x;
   info->y = p[0].y;

   // Compute plane equation
   u[0] = p[1].x - p[0].x;
   u[1] = p[1].y - p[0].y;
   u[2] = p[1].z - p[0].z;
   v[0] = p[2].x - p[0].x;
   v[1] = p[2].y - p[0].y;
   v[2] = p[2].z - p[0].z;
   uv[0] = u[2] * v[1] - u[1] * v[2];
   uv[1] = u[0] * v[2] - u[2] * v[0];
   uv[2] = u[1] * v[0] - u[0] * v[1];
   ucrossv = sqrt(uv[0] * uv[0] + uv[1] * uv[1] + uv[2] * uv[2]);

   assert(ucrossv != 0.0);

   info->plane.a = (int) (uv[0] * FINENESS / ucrossv);
   info->plane.b = (int) (uv[1] * FINENESS / ucrossv);
   info->plane.c = (int) (uv[2] * FINENESS / ucrossv);
   info->plane.d = (int) - (info->plane.a * p[0].x + info->plane.b * p[0].y +
			  info->plane.c * p[0].z);

   if (floor != 0) { //floor
       if (info->plane.c < 0) { // normals of floors must point up
	   info->plane.a = -info->plane.a;
	   info->plane.b = -info->plane.b;
	   info->plane.c = -info->plane.c;
	   info->plane.d = -info->plane.d;
       }
   }
   else { // ceiling
       if (info->plane.c > 0) { // normals of ceilings must point down
	   info->plane.a = -info->plane.a;
	   info->plane.b = -info->plane.b;
	   info->plane.c = -info->plane.c;
	   info->plane.d = -info->plane.d;
       }
   }
   dprintf("plane equation: a = %d, b = %d, c = %d, d = %d\n", info->plane.a, info->plane.b, info->plane.c, info->plane.d);
}
/***************************************************************************/
/*
 * WriteSlopeInfo:  Write the given SlopeInfo structure to the given file.
 */
void WriteSlopeInfo(FILE *file, SlopeInfo *info, int floor)
{
   int i, angle;
   SHORT x, y, z, v;

   ComputeSlopeInfo(info, floor); // compute slope needs to know whether its
                                  // a floor or a ceiling
   
   WriteBytes(file, &info->plane.a, 4);
   WriteBytes(file, &info->plane.b, 4);
   WriteBytes(file, &info->plane.c, 4);
   WriteBytes(file, &info->plane.d, 4);
   WriteBytes(file, &info->x, 4);
   WriteBytes(file, &info->y, 4);
   angle = info->angle * NUMDEGREES / 360;
   WriteBytes(file, &angle, 4);
   for (i=0; i < 3; i++)
   {
      v = info->points[i].vertex;
      x = Vertexes[v].x;
      WriteBytes(file, &x, 2);
      y = Vertexes[v].y;
      WriteBytes(file, &y, 2);
      z = info->points[i].z;
      WriteBytes(file, &z, 2);
   }
}
/***************************************************************************/
/*
 * SidedefsEquivalent:  Return TRUE iff file sidedef s1 is equivalent to sidedef s.
 *   flags gives flags for s.
 */
BOOL SidedefsEquivalent(FileSideDef *s1, SideDef *s, int flags)
{
   return (s1->id == s->user_id &&
	   s1->type_above == s->type1 &&
	   s1->type_below == s->type2 &&
	   s1->type_normal == s->type3 &&
	   s1->flags == flags &&
	   s1->animate_speed == s->animate_speed);
}
/***************************************************************************/
/*
 * FindSidedef:  Given a linedef and one of its associated sidedefs, return the number of a 
 *  matching file sidedef, adding a new one if necessary.
 *  positive is true if s is + sidedef of l.
 */
int FindSidedef(LineDef *l, SideDef *s, Bool positive)
{
   int num, flags;
   FileSideDef *ptr, *last;
   FileSideDef *newdef;

   flags = 0;
   if ((positive && (l->blak_flags & BF_POS_BACKWARDS)) ||
       (!positive && (l->blak_flags & BF_NEG_BACKWARDS)))
      flags |= WF_BACKWARDS;
   if ((positive && (l->blak_flags & BF_POS_TRANSPARENT)) ||
       (!positive && (l->blak_flags & BF_NEG_TRANSPARENT)))
      flags |= WF_TRANSPARENT;
   if ((positive && (l->blak_flags & BF_POS_PASSABLE)) ||
       (!positive && (l->blak_flags & BF_NEG_PASSABLE)))
      flags |= WF_PASSABLE;
   if ((positive && (l->blak_flags & BF_POS_NOLOOKTHROUGH)) ||
       (!positive && (l->blak_flags & BF_NEG_NOLOOKTHROUGH)))
      flags |= WF_NOLOOKTHROUGH;
   if ((positive && (l->blak_flags & BF_POS_ABOVE_BUP)) ||
       (!positive && (l->blak_flags & BF_NEG_ABOVE_BUP)))
      flags |= WF_ABOVE_BOTTOMUP;
   if ((positive && (l->blak_flags & BF_POS_BELOW_TDOWN)) ||
       (!positive && (l->blak_flags & BF_NEG_BELOW_TDOWN)))
      flags |= WF_BELOW_TOPDOWN;
   if ((positive && (l->blak_flags & BF_POS_NORMAL_TDOWN)) ||
       (!positive && (l->blak_flags & BF_NEG_NORMAL_TDOWN)))
      flags |= WF_NORMAL_TOPDOWN;
   if ((positive && (l->blak_flags & BF_POS_NO_VTILE)) ||
       (!positive && (l->blak_flags & BF_NEG_NO_VTILE)))
      flags |= WF_NO_VTILE;

   if (l->blak_flags & BF_MAP_NEVER)
      flags |= WF_MAP_NEVER;
   if (l->blak_flags & BF_MAP_ALWAYS)
      flags |= WF_MAP_ALWAYS;

   if (positive)
   {
      flags |= (WallScrollPosSpeed(l->blak_flags) << 10);
      flags |= (WallScrollPosDirection(l->blak_flags) << 12);
   }
   else
   {
      flags |= (WallScrollNegSpeed(l->blak_flags) << 10);
      flags |= (WallScrollNegDirection(l->blak_flags) << 12);
   }

   num = 1;
   last = NULL;
   for (ptr = FileSideDefs; ptr != NULL; ptr = ptr->next)
   {
      if (SidedefsEquivalent(ptr, s, flags))
	 return num;
      last = ptr;
      num++;
   }
   newdef = (FileSideDef *) SafeMalloc(sizeof(FileSideDef));
   newdef->next = NULL;
   newdef->id = s->user_id;

   newdef->type_above = s->type1;
   newdef->type_below = s->type2;
   newdef->type_normal = s->type3;
   newdef->animate_speed = s->animate_speed;
   newdef->flags = flags;

   // Add new file sidedef to list
   if (last == NULL)
      FileSideDefs = newdef;
   else last->next = newdef;

   NumFileSideDefs++;
   return num;
}
/***************************************************************************/
/*
 * AddWallsFromLinedef:  Add walls in given linedef to "walls", and return new
 *   wall list.
 *   num gives # of linedef (used for debugging)
 */
WallDataList AddWallsFromLinedef(WallDataList walls, LineDef *l, int num)
{
   Vertex VStart = Vertexes[l->start];
   Vertex VEnd   = Vertexes[l->end];
   WallData *new_wall;
   SideDef *SD1, *SD2;
   WORD sd_num;

   if ((VStart.x == VEnd.x) && (VStart.y == VEnd.y))
   {
      LogError(">>> Wall %d has same start/end coordinates\n",num);
      return walls;
   }
   
   
   new_wall = BSPGetNewWall();
   new_wall->next = walls;
   walls = new_wall;
   
   // Origin of room is (0, 0)
   // Flip vertically, since client uses different coordinate system
   new_wall->x0 = ((VStart.x - MapMinX)) * BLAK_FACTOR;
   new_wall->y0 = (MapMaxY - VStart.y) * BLAK_FACTOR;
   new_wall->x1 = ((VEnd.x - MapMinX)) * BLAK_FACTOR;
   new_wall->y1 = (MapMaxY - VEnd.y) * BLAK_FACTOR;
   
   new_wall->pos_sidedef = 0;
   new_wall->neg_sidedef = 0;
   new_wall->pos_sector = -1;
   new_wall->neg_sector = -1;
   new_wall->flags = l->blak_flags;
   new_wall->length = ComputeDist(VEnd.x - VStart.x, VEnd.y - VStart.y);
   new_wall->linedef_num = num;
   
	//SD1 = SD2 = NULL;
   if (l->sidedef1 >= 0)
   {
      SD1 = &SideDefs[l->sidedef1];
      
      new_wall->pos_type    = SD1->type3;
      new_wall->pos_sector  = SD1->sector;
      new_wall->pos_xoffset = SD1->xoff;
      new_wall->pos_yoffset = SD1->yoff;
      sd_num = FindSidedef(l, SD1, True);
      new_wall->pos_sidedef = sd_num;
      l->file_sidedef1 = sd_num;
   }
   
   if (l->sidedef2 >= 0)
   {
      SD2 = &SideDefs[l->sidedef2];
      
      new_wall->neg_type    = SD2->type3;
      new_wall->neg_sector  = SD2->sector;
      new_wall->neg_xoffset = SD2->xoff;
      new_wall->neg_yoffset = SD2->yoff;
      sd_num = FindSidedef(l, SD2, False);
      new_wall->neg_sidedef = sd_num;
      l->file_sidedef2 = sd_num;
   }
   SetWallHeights(new_wall);
   return walls;
}
/***************************************************************************/
/*
 * FileBackpatch:  Write "data" to the 4 bytes starting at position "pos" in the file.
 *   This function leaves the file pointer at the same place as when the function was called.
 */
void FileBackpatch(FILE *file, int pos, int data)
{
   int cur_pos;

   cur_pos = FileCurPos(file);
   FileGoto(file, pos);
   WriteBytes(file, &data, 4);
   FileGoto(file, cur_pos);
}
/***************************************************************************/
/*
 * BackupFile:  Copy fname to a backup name if it already exists.
 *   Return True iff successful.
 */
Bool BackupFile(char *fname)
{
   struct stat s;
   char bname[MAX_PATH];

   // Do nothing if no file exists
   if (stat(fname, &s) == -1)
      return True;

   strcpy(bname, fname);
   strcat(bname, "~");

   if (CopyFile(fname, bname, FALSE) == FALSE)
   {
		if (MessageBox(NULL, "Can't make backup room file.\nSave anyway?", "Warning",
			  MB_YESNO | MB_DEFBUTTON2) == IDNO)
	 return False;
   }
   return True;
}
/***************************************************************************/
/*
 * SetWallHeights:  Fill in wall heights from adjacent sector heights.
 */
void SetWallHeights(WallData *wall)
{
   Sector *S1 = NULL, *S2 = NULL;

   if (wall->pos_sector != -1)
      S1 = &Sectors[wall->pos_sector];

   if (wall->neg_sector != -1)
      S2 = &Sectors[wall->neg_sector];

   // If no sectors, give defaults
   if (S1 == NULL && S2 == NULL)
   {
      wall->z0 = wall->z1 = 0;
      wall->z2 = wall->z3 = 64;
      return;
   }

   if (S1 == NULL)
   {
		wall->z0 = wall->z1 = S2->floorh;
      wall->z2 = wall->z3 = S2->ceilh;
      return;
   }

   if (S2 == NULL)
   {
      wall->z0 = wall->z1 = S1->floorh;
      wall->z2 = wall->z3 = S1->ceilh;
      return;
   }

   wall->z0 = min(S1->floorh, S2->floorh);
   wall->z1 = max(S1->floorh, S2->floorh);
   wall->z2 = min(S1->ceilh, S2->ceilh);
   wall->z3 = max(S1->ceilh, S2->ceilh);
}
/***************************************************************************/
/*
 * FreeFileSideDefs:  Free memory for list of file sidedefs.
 */
void FreeFileSideDefs(void)
{
   FileSideDef *next, *ptr;

   ptr = FileSideDefs;
   while (ptr != NULL)
   {
      next = ptr->next;
		SafeFree(ptr);
      ptr = next;
   }
   FileSideDefs = NULL;
}
