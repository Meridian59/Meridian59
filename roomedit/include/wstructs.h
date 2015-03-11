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

	FILE:         wstructs.h
*/
#ifndef __wstructs_h
#define __wstructs_h

#ifndef __common_h
	#include "common.h"
#endif

#include "dibutil.h"


/*
   this data structure contains the information about the THINGS

*/
#define THING_FLAG_DONTGENERATE	0x0001
#define THING_FLAG_HASCOMMENT		0x0002

struct Thing
{
	SHORT type;      /* thing type */       /* size =  8 */
	SHORT xpos;      /* x position */		/* size =  2 */
	SHORT ypos;      /* y position */       /* size =  4 */
	SHORT angle;     /* facing angle */     /* size =  6 */
	SHORT when;      /* appears when? */    /* size = 10 */
	SHORT xExitPos;
	SHORT yExitPos;
	SHORT flags;
	SHORT id;
	char comment[64];
};
typedef struct Thing *TPtr;



/*
   this data structure contains the information about the LINEDEFS
*/
struct LineDef
{
   SHORT start;     /* from this vertex ... */
   SHORT end;       /* ... to this vertex */
   SHORT flags;     /* see NAMES.C for more info */
   SHORT type;      /* see NAMES.C for more info */
   SHORT tag;       /* crossing this linedef activates the sector with the same tag */
   SHORT sidedef1;  /* sidedef */
   SHORT sidedef2;  /* only if this line adjoins 2 sectors */
   int   blak_flags; /* Blakston flags for wall ARK */

   WORD  file_sidedef1;  // Used during saving ARK
   WORD  file_sidedef2;  // Used during saving ARK
};
typedef struct LineDef *LDPtr;



/*
   this data structure contains the information about the SIDEDEFS
*/
struct SideDef
{
   SHORT xoff;                          /* X offset for texture */
   SHORT yoff;                          /* Y offset for texture */
   char tex1[MAX_BITMAPNAME + 1];  	/* texture name for the part above */
   char tex2[MAX_BITMAPNAME + 1];  	/* texture name for the part below */
   char tex3[MAX_BITMAPNAME + 1];  	/* texture name for the regular part */
   int  type1;                          /* Bitmap # for the part above ARK */
   int  type2;                          /* Bitmap # for the part below ARK */
   int  type3;                          /* Bitmap # for the regular part ARK */
   SHORT sector;                        /* adjacent sector */
   BYTE animate_speed;                  /* speed of animation (tenths of frame per second) ARK */
   WORD user_id;                        /* User-id of sidedef ARK */
};
typedef struct SideDef *SDPtr;

/*
   this data structure contains the information about the VERTEXES
*/
struct Vertex
{
   SHORT x;         /* X coordinate */
   SHORT y;         /* Y coordinate */
};
typedef struct Vertex *VPtr;


/*
   this data structure contains the information about the SEGS
*/
typedef struct Seg *SEPtr;
struct Seg
{
   SEPtr next;      /* next Seg in list */
   SHORT start;     /* from this vertex ... */
   SHORT end;       /* ... to this vertex */
   USHORT angle;    /* angle (0 = east, 16384 = north, ...) */
   SHORT linedef;   /* linedef that this seg goes along*/
   SHORT flip;      /* true if not the same direction as linedef */
   USHORT dist;     /* distance from starting point */
};




/*
   this data structure contains the information about the SSECTORS
*/
typedef struct SSector *SSPtr;
struct SSector
{
   SSPtr next;	    /* next Sub-Sector in list */
   SHORT num;       /* number of Segs in this Sub-Sector */
   SHORT first;     /* first Seg */
};



/*
   this data structure contains the information about the NODES
*/
typedef struct Node *NPtr;
struct Node
{
   SHORT x, y;                      // starting point
   SHORT dx, dy;                    // offset to ending point
   SHORT miny1, maxy1, minx1, maxx1;// bounding rectangle 1
   SHORT miny2, maxy2, minx2, maxx2;// bounding rectangle 2
   SHORT child1, child2;            // Node or SSector (if high bit is set)
   NPtr node1, node2;               // pointer if the child is a Node
   SHORT num;                       // number given to this Node
};



/* 3D plane defined by ax + by + cz + d */
typedef struct {
   FixedPoint a, b, c, d;
} Plane3D;

typedef struct {
   int x, y, z;
} Point3D;

typedef struct {
   SHORT vertex;                 // Vertex index
   int   z;                      // z-coordinate at vertex
} VertexHeight;

typedef struct {
   Plane3D plane;                // Plane equation
   int     x, y;                 // Texture origin
   int     angle;                // Direction of positive u axis (same scale as client angle)
   VertexHeight points[3];       // User specified points indices defining plane
} SlopeInfo;

/*
   this data structure contains the information about the SECTORS
*/
struct Sector
{
   int   floorh;                 /* floor height */
   int   ceilh;                  /* ceiling height */
   char  floort[MAX_BITMAPNAME + 1]; /* floor texture */
   char  ceilt[MAX_BITMAPNAME + 1];  /* ceiling texture */
   SHORT light;                  /* light level (0-255) */
   SHORT special;                /* special behaviour (0 = normal, 9 = secret, ...) */
   SHORT tag;                    /* sector activated by a linedef with the same tag */
   int   floor_type;             /* Bitmap # for floor ARK */
   int   ceiling_type;           /* Bitmap # for ceiling ARK */
   int   xoffset;                /* x offset of texture ARK */
   int   yoffset;                /* y offset of texture ARK */
   WORD  user_id;                /* User-id of sector ARK */
   int   blak_flags;             /* Blakston flags for sector ARK */
   BYTE  animate_speed;          /* speed of animation (tenths of frame per second) */

   SlopeInfo floor_slope;        // Info on sloped floor (if sloped floor blak_flag set)
   SlopeInfo ceiling_slope;      // Info on sloped ceiling (if sloped ceiling blak_flag set)
};
typedef struct Sector *SPtr;


// Sidedef info saved to file
typedef struct _FileSideDef{
   struct _FileSideDef *next;
   WORD  id;
   WORD  type_above;
   WORD  type_normal;
   WORD  type_below;
   int   flags;
   BYTE  animate_speed;
} FileSideDef;



#endif
/* end of file */
