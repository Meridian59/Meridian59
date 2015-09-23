// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bsp.h:  Header for bsp.c
 */

#ifndef _BSP_H
#define _BSP_H

/* Bit flags for wall characteristics */
#define WF_BACKWARDS      0x00000001      // Draw bitmap right/left reversed
#define WF_TRANSPARENT    0x00000002      // normal wall has some transparency
#define WF_PASSABLE       0x00000004      // wall can be walked through
#define WF_MAP_NEVER      0x00000008      // Don't show wall on map
#define WF_MAP_ALWAYS     0x00000010      // Always show wall on map
#define WF_NOLOOKTHROUGH  0x00000020      // bitmap can't be seen through even though it's transparent
#define WF_ABOVE_BOTTOMUP 0x00000040      // Draw upper texture bottom-up
#define WF_BELOW_TOPDOWN  0x00000080      // Draw lower texture top-down
#define WF_NORMAL_TOPDOWN 0x00000100      // Draw normal texture top-down
#define WF_NO_VTILE       0x00000200      // Don't tile texture vertically (must be transparent)
#define	WF_HAS_ANIMATED	  0x00000400      // has animated once and hence is dynamic geometry, required for new client

// Texture scrolling constants
#define SCROLL_NONE    0x00000000      // No texture scrolling   
#define SCROLL_SLOW    0x00000001      // Slow speed texture scrolling   
#define SCROLL_MEDIUM  0x00000002      // Medium speed texture scrolling   
#define SCROLL_FAST    0x00000003      // Fast speed texture scrolling   
#define SCROLL_N       0x00000000      // Texture scroll to N
#define SCROLL_NE      0x00000001      // Texture scroll to NE
#define SCROLL_E       0x00000002      // Texture scroll to E
#define SCROLL_SE      0x00000003      // Texture scroll to SE
#define SCROLL_S       0x00000004      // Texture scroll to S
#define SCROLL_SW      0x00000005      // Texture scroll to SW
#define SCROLL_W       0x00000006      // Texture scroll to W
#define SCROLL_NW      0x00000007      // Texture scroll to NW

#define WallScrollSpeed(flags) ((BYTE)(((flags) & 0x00000C00) >> 10))
#define WallScrollDirection(flags) ((BYTE)(((flags) & 0x00007000) >> 12))

/* Bit flags for sector characteristics */
#define SF_DEPTH0         0x00000000      // Sector has default (0) depth
#define SF_DEPTH1         0x00000001      // Sector has shallow depth
#define SF_DEPTH2         0x00000002      // Sector has deep depth
#define SF_DEPTH3         0x00000003      // Sector has very deep depth

#define SF_SCROLL_FLOOR   0x00000080      // Scroll floor texture
#define SF_SCROLL_CEILING 0x00000100      // Scroll ceiling textire

#define SectorDepth(flags) ((flags) & 0x00000003)  // Retrieve depth bits from sector flags
#define SectorScrollSpeed(flags) ((BYTE)(((flags) & 0x0000000C) >> 2))
#define SectorScrollDirection(flags) ((BYTE)(((flags) & 0x00000070) >> 4))

#define SF_FLICKER        0x00000200      // Flicker light in sector
#define SF_SLOPED_FLOOR   0x00000400      // Sector has sloped floor
#define SF_SLOPED_CEILING 0x00000800      // Sector has sloped ceiling
#define	SF_HAS_ANIMATED	  0x00001000      // has animated once and hence is dynamic geometry, required for new client

/* Bit flags for sloped surface characteristics */
#define SLF_DIRECTIONAL   0x0001
#define SLF_TOO_STEEP     0x0002

/* c constant threshold values corresponding to above flags */
#define DIRECTIONAL_THRESHOLD   1010   // steep enough to get directional lighting
#define TOO_STEEP_THRESHOLD     480    // too steep to walk on

/* for bowtie handling */
#define BT_BELOW_POS    1    // below wall is bowtie & positive sector is on top at endpoint 0
#define BT_BELOW_NEG    2    // below wall is bowtie & negative sector is on top at endpoint 0
#define BT_ABOVE_POS    4    // above wall is bowtie & positive sector is on top at endpoint 0
#define BT_ABOVE_NEG    8    // above wall is bowtie & negative sector is on top at endpoint 0

#define BT_BELOW_BOWTIE 3    // mask to test for bowtie on below wall
#define BT_ABOVE_BOWTIE 12   // mask to test for bowtie on above wall

#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define SGN(x) ((x) == 0 ? 0 : ((x) > 0 ? 1 : -1))
#define SGNDOUBLE(x) (((x) <= 0.001 && (x) >= -0.001) ? 0 : ((x) > 0.001 ? 1 : -1))

#pragma warning( disable : 4201 )		// nonstandard extension used : nameless struct/union (a union in this case )

// D3D types
typedef struct custom_xyz
{
   float	x, y, z;
} custom_xyz;

typedef struct custom_st
{
   float	s, t;
} custom_st;

typedef struct custom_bgra
{
   unsigned char	b, g, r, a;
} custom_bgra;

/* plane defined by ax + by + c = 0. (x and y are in fineness units.) */
typedef struct
{
   float a, b, c;
} Plane,Plane2D;

/* 3D plane defined by ax + by + cz + d */
typedef struct
{
    float a, b, c, d;
} Plane3D;

/* box defined by its top left and bottom right coordinates (in fineness) */
typedef struct
{
   float x0,y0,x1,y1;
} Box;

typedef struct
{
   float x,y;
} Pnt,Pnt2D,Vector2D;

typedef struct
{
   float x,y,z;
} Pnt3D,Vector3D;

typedef struct ObjectData
{
   long x0,y0,x1,y1;           /* position of object, in fineness coordinates */
   DrawnObject draw;           /* info used to draw object */
   BYTE  *ncones_ptr;          /* pointer to # of cones this object occupies */
   BYTE  ncones;               /* destination of ncones_ptr for first cone of obj */
   
   struct ObjectData *next;    /* next in list of objects in BSP leaf */
   struct BSPleaf *parent;     /* leaf containing object */
} ObjectData, *ObjectList;

typedef struct {
    Plane3D	plane;         /* plane equation of slope */
    Pnt3D	p0;	       /* texture origin */
    Pnt3D	p1;	       /* u axis end point */
    Pnt3D	p2;            /* v axis end point */
    Pnt3D	h;	       /* h, v, o : values for texturing surface from current viewpoint */
    Pnt3D	v;
    Pnt3D	o;
    FixedPoint  z0;	       /* distances for viewer light calculation */
    FixedPoint  z_du;
    FixedPoint  z_dv;
	long	texRot;			// new client needs the raw rotation angle
    short	flags;	       /* flags about properties of this surface */
    short       lightscale;    /* scaling factor for directional lighting */
} SlopeData;

typedef struct {
   long  tx,ty;                 /* coordinates of texture origin */
   PDIB  floor;                 /* pointer to floor texture */
   PDIB  ceiling;               /* pointer to ceiling texture */
   WORD  floor_type;            /* Grid bitmap # for floor */
   WORD  ceiling_type;          /* Grid bitmap # for ceiling */
   int   floor_height;          /* Height of floor relative to 0 = "normal" height */
   int   ceiling_height;        /* Height of ceiling relative to 0 = "normal" height */
   SlopeData *sloped_floor;     /* If floor is sloped, points to SlopeData. Null otherwise. */
   SlopeData *sloped_ceiling;   /* If ceiling is sloped, points to SlopeData. Null otherwise. */
   WORD  server_id;             /* Server id # of sector (0 if none) */
   BYTE  light;                 /* Light level of sector (0 = dark, 255 = bright) */
   int   flags;                 /* characteristics of sector */
   RoomAnimate *animate;        /* Animation structure for floor and ceiling */
} Sector;

typedef struct {
   WORD server_id;             /* Server id # of wall (0 if none) */

   WORD above_type;            /* Grid bitmap # for above wall */
   WORD below_type;            /* Grid bitmap # for below wall */
   WORD normal_type;           /* Grid bitmap # for normal wall */

   PDIB above_bmap;            /* pointer to above bitmap to tile wall with */
   PDIB below_bmap;            /* pointer to below to tile wall with on - side */
   PDIB normal_bmap;           /* pointer to normal to tile wall with on - side */

   int  flags;                 /* characteristics of wall (transparency, left/right swap) */
   RoomAnimate *animate;       /* Animation structure for wall */
} Sidedef;

#define MAX_NPTS 100

typedef struct WallData
{
   union {
      struct WallData *next;   /* next in list of polys coincident to separator plane */
      int next_num;            /* number of next wall (used during loading) */
   };

   WORD pos_xoffset;           /* X offset of + side bitmap */
   WORD neg_xoffset;           /* X offset of - side bitmap */
   WORD pos_yoffset;           /* Y offset of + side bitmap */
   WORD neg_yoffset;           /* Y offset of - side bitmap */

   Plane separator;
   
   float length;                 /* length of wall; 1 grid square = 64 */
   
   // Since I doubled the number of heights stored here, I'm changing
   // these heights to shorts to take up half the space. No room (except godroom)
   // is currently tall enough to require more storage than this.

   // 11-2003:  Changing these back to longs, because the signed/unsigned mismatches
   // are jacking everything up - MisterY

   // CA: these are now heights at x0,y0
   long  z0;         /* height of bottom of lower wall */
   long  z1;         /* height of top of lower wall / bottom of normal wall */
   long  z2;         /* height of top of normal wall / bottom of upper wall */
   long  z3;         /* height of top of upper wall */

   // CA: these are heights at x1,y1
   long  zz0;        /* height of bottom of lower wall */
   long  zz1;        /* height of top of lower wall / bottom of normal wall */
   long  zz2;        /* height of top of normal wall / bottom of upper wall */
   long  zz3;        /* height of top of upper wall */

   // 2-2004:  Some bowties are actually calculated incorrectly (in fact, for some it's impossible
   // to calculate them correctly), which is bad news for the old renderer and even
   // worse news for the new renderer.  To fix this in the new renderer, we now cache the height
   // values for both pos sidedef and neg sidedef in the wall structure, and at each bowtie
   // generate two walls:  pos wall uses height values from the neg sector, and neg wall uses
   // the height values from the pos sector.  In the case of a bowtie, the above height values
   // are assumed to be positive sector, and the values below are negative sector.
   // As cosmic retribution for the old client code necessitating this hack, God killed a kitten
   // today - MisterY

   long  z0Neg;         /* height of bottom of lower wall */
   long  z1Neg;         /* height of top of lower wall / bottom of normal wall */
   long  zz0Neg;        /* height of bottom of lower wall */
   long  zz1Neg;        /* height of top of lower wall / bottom of normal wall */

   float x0, y0, x1, y1;         /* coordinates of wall start and end */
   /* (x0,y0) and (x1,y1) must satisfy separator plane equation */
   /* positive side of wall must be on right when going from 0 to 1 */

   Bool seen;                  /* True iff part of this wall has been drawn */
   Bool drawbelow;     // True if D3D renderer should draw this wall.
   Bool drawabove;     // True if D3D renderer should draw this wall.
   Bool drawnormal;    // True if D3D renderer should draw this wall.

   // for bowtie handling
   BYTE bowtie_bits;           /* flags set indicating a bowtie & it's orientation */

   long lightscale;            /* scalar value based on angle to light source */

   union {
      Sector *pos_sector;      /* Adjoining sector on + side (NULL if none) */
      WORD    pos_sector_num;  /* Sector number (used during loading) */
   };

   union {
      Sector *neg_sector;      /* Adjoining sector on - side (NULL if none) */
      WORD    neg_sector_num;  /* Sector number (used during loading) */
   };

   union {
      Sidedef *pos_sidedef;     /* Sidedef on + side (NULL if none) */
      WORD     pos_sidedef_num; /* Sidedef number (used during loading) */
   };

   union {
      Sidedef *neg_sidedef;     /* Sidedef on - side (NULL if none) */
      WORD     neg_sidedef_num; /* Sidedef number (used during loading) */
   };

   // The following data structres are used by D3D to calculate and store
   // information about the walls, to prevent having to recalculate multiple
   // times a frame.
   custom_xyz pos_normal_xyz[4];
   custom_xyz pos_below_xyz[4];
   custom_xyz pos_above_xyz[4];

   custom_st pos_normal_stBase[4];
   custom_st pos_below_stBase[4];
   custom_st pos_above_stBase[4];

   custom_bgra pos_normal_bgra[4];
   custom_bgra pos_below_bgra[4];
   custom_bgra pos_above_bgra[4];

   unsigned int pos_normal_d3dFlags;
   unsigned int pos_below_d3dFlags;
   unsigned int pos_above_d3dFlags;

   custom_xyz neg_normal_xyz[4];
   custom_xyz neg_below_xyz[4];
   custom_xyz neg_above_xyz[4];

   custom_st neg_normal_stBase[4];
   custom_st neg_below_stBase[4];
   custom_st neg_above_stBase[4];

   custom_bgra neg_normal_bgra[4];
   custom_bgra neg_below_bgra[4];
   custom_bgra neg_above_bgra[4];

   unsigned int neg_normal_d3dFlags;
   unsigned int neg_below_d3dFlags;
   unsigned int neg_above_d3dFlags;
} WallData, *WallList, *WallDataList;

typedef struct
{
   Plane separator;               /* plane that separates space */
   union {
      WallList walls_in_plane;    /* any walls that are conincident to separator plane */
      WORD wall_num;              /* number of first wall in list (used during loading) */
   };
   union {
      struct BSPnode *pos_side;   /* stuff on ax + by + c > 0 side */
      WORD pos_num;               /* number of node on + side (used during loading) */
   };
   union {
      struct BSPnode *neg_side;   /* stuff on ax + by + c < 0 side */
      WORD neg_num;               /* number of node on + side (used during loading) */
   };
} BSPinternal;

typedef struct {
   int npts;                   /* # of points in polygon */
   Pnt p[MAX_NPTS + 1];          /* points of polygon (clockwise ordered, looking down on floor) */
   /*    invariant: p[npts] == p[0] */
} Poly;

typedef struct BSPleaf
{
   Poly poly;                  /* Polygon of floor area */
   ObjectList objects;         /* objects within this leaf */

   union {
      Sector *sector;      /* Sector to which leaf belongs (cannot be NULL) */
      WORD    sector_num;  /* Sector number (used during loading) */
   };
} BSPleaf;

typedef enum
{
   BSPinternaltype = 1,
   BSPleaftype = 2,
} BSPnodetype;

typedef struct BSPnode
{
   BSPnodetype type;           /* type of BSP node */
   Box bbox;                   /* bounding box for this node */
   
   union
   {
      BSPinternal internal;
      BSPleaf leaf;
   } u;

   // The following data structres are used by D3D to calculate and store
   // information about the ceiling and floor, to prevent having to
   // recalculate multiple times a frame.
   custom_xyz ceiling_xyz[MAX_NPTS];
   custom_st ceiling_stBase[MAX_NPTS];
   custom_bgra ceiling_bgra[MAX_NPTS];
   PDIB ceiling_pDib;
   custom_xyz floor_xyz[MAX_NPTS];
   custom_st floor_stBase[MAX_NPTS];
   custom_bgra floor_bgra[MAX_NPTS];
   PDIB floor_pDib;

   Bool drawfloor;   // True if D3D renderer should draw this floor.
   Bool drawceiling; // True if D3D renderer should draw this ceiling.

} BSPnode, *BSPTree;

#pragma warning( default : 4201 )		// nonstandard extension used : nameless struct/union (a union in this case )

#endif  /* #ifndef _BSP_H */
