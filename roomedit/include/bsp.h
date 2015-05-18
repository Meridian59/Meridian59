/*
 * bsp.h:  Header for bsp.cpp
 */

#ifndef _BSP_H
#define _BSP_H

#define FINENESS 1024
#define FINENESSHIGHRESGRID 256
#define NUMDEGREES 4096

#define BLAK_FACTOR 16  // Multiply by this to convert roomeditor coordinates to client coordinates

// Convert client coordinates to roomeditor coordinates
#define FinenessClientToKod(x) ((x) / BLAK_FACTOR)

/* Bit flags for linedef characteristics in editor */
/* Important:  if you add +/- flags here, you must add the appropriate line to
 * bspmake.cpp/BSPFlipWall.
 */
#define BF_POS_BACKWARDS     0x00000001      // Draw + side bitmap right/left reversed
#define BF_NEG_BACKWARDS     0x00000002      // Draw - side bitmap right/left reversed
#define BF_POS_TRANSPARENT   0x00000004      // + side bitmap has some transparency
#define BF_NEG_TRANSPARENT   0x00000008      // - side bitmap has some transparency
#define BF_POS_PASSABLE      0x00000010      // + side bitmap can be walked through
#define BF_NEG_PASSABLE      0x00000020      // - side bitmap can be walked through
#define BF_MAP_NEVER         0x00000040      // Don't show wall on map
#define BF_MAP_ALWAYS        0x00000080      // Always show wall on map

#define BF_POS_NOLOOKTHROUGH 0x00000400      // + side bitmap can't be seen through even though it's transparent
#define BF_NEG_NOLOOKTHROUGH 0x00000800      // - side bitmap can't be seen through even though it's transparent

#define BF_POS_ABOVE_BUP     0x00001000      // + side above texture bottom up
#define BF_NEG_ABOVE_BUP     0x00002000      // - side above texture bottom up
#define BF_POS_BELOW_TDOWN   0x00004000      // + side below texture top down
#define BF_NEG_BELOW_TDOWN   0x00008000      // - side below texture top down
#define BF_POS_NORMAL_TDOWN  0x00010000      // + side normal texture top down
#define BF_NEG_NORMAL_TDOWN  0x00020000      // - side normal texture top down
#define BF_POS_NO_VTILE      0x00040000      // + side no vertical tile
#define BF_NEG_NO_VTILE      0x00080000      // - side no vertical tile

// scrolling texture flags come next

#define WallScrollPosSpeed(flags) ((BYTE)(((flags) & 0x00300000) >> 20))
#define WallScrollPosDirection(flags) ((BYTE)(((flags) & 0x01C00000) >> 22))
#define WallScrollNegSpeed(flags) ((BYTE)(((flags) & 0x06000000) >> 25))
#define WallScrollNegDirection(flags) ((BYTE)(((flags) & 0x38000000) >> 27))

/* Bit flags for sidedef characteristics in roo file */
#define WF_BACKWARDS     0x00000001      // Draw bitmap right/left reversed
#define WF_TRANSPARENT   0x00000002      // normal wall has some transparency
#define WF_PASSABLE      0x00000004      // wall can be walked through
#define WF_MAP_NEVER     0x00000008      // Don't show wall on map
#define WF_MAP_ALWAYS    0x00000010      // Always show wall on map
#define WF_NOLOOKTHROUGH 0x00000020      // bitmap can't be seen through even though it's transparent
#define WF_ABOVE_BOTTOMUP 0x00000040      // Draw upper texture bottom-up
#define WF_BELOW_TOPDOWN  0x00000080      // Draw lower texture top-down
#define WF_NORMAL_TOPDOWN 0x00000100      // Draw normal texture top-down
#define WF_NO_VTILE       0x00000200      // Don't tile texture vertically (must be transparent)

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

#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define SGN(x) ((x) == 0 ? 0 : ((x) > 0 ? 1 : -1))
#define SGNDOUBLE(x) (((x) <= 0.001 && (x) >= -0.001) ? 0 : ((x) > 0.001 ? 1 : -1))

/* plane defined by ax + by + c = 0. (x and y are in fineness units.) */
typedef struct
{
   double a, b, c;
} Plane;

/* box defined by its top left and bottom right coordinates (in fineness) */
typedef struct
{
   double x0,y0,x1,y1;
} Box;

typedef struct
{
   double x,y;
} Pnt;

typedef struct WallData
{
   union {
      struct WallData *next;   /* next in list of polys coincident to separator plane */
      int next_num;            /* number of next wall (used during loading) */
   };

   int num;                    /* Ordinal # of this wall (1 = first wall) */
   
   int pos_type;               /* bitmap to tile wall with on positive side */
   int neg_type;               /* bitmap to tile wall with on negative side */

   int flags;                  /* characteristics of wall (transparency, left/right swap) */
   int pos_xoffset;            /* X offset of + side bitmap */
   int neg_xoffset;            /* X offset of - side bitmap */
   int pos_yoffset;            /* Y offset of + side bitmap */
   int neg_yoffset;            /* Y offset of - side bitmap */

   int pos_sector;             /* Sector # on + side */
   int neg_sector;             /* Sector # on - side */

   double x0, y0, x1, y1;      /* coordinates of wall start and end */

   double length;              /* length of wall; 1 grid square = 64 */
   int z0;                     /* height of bottom of lower wall */
   int z1;                     /* height of top of lower wall / bottom of normal wall */
   int z2;                     /* height of top of normal wall / bottom of upper wall */
   int z3;                     /* height of top of upper wall */

   WORD server_id;             /* User-id of wall */

   WORD pos_sidedef;           /* Sidedef # for + side of wall */
   WORD neg_sidedef;           /* Sidedef # for - side of wall */

   WORD linedef_num;           /* linedef # this wall came from; used for debugging */

   /* (x0,y0) and (x1,y1) must satisfy separator plane equation */
   /* positive side of wall must be on right when going from 0 to 1 */
} WallData, *WallList, *WallDataList;

typedef struct
{
   Plane separator;            /* plane that separates space */
   union {
      WallList walls_in_plane;    /* any walls that are conincident to separator plane */
      int wall_num;               /* number of first wall in list (used during loading) */
   };
   union {
      struct BSPnode *pos_side;   /* stuff on ax + by + c > 0 side */
      int pos_num;                /* number of node on + side (used during loading) */
   };
   union {
      struct BSPnode *neg_side;   /* stuff on ax + by + c < 0 side */
      int neg_num;                /* number of node on + side (used during loading) */
   };
} BSPinternal;

#define MAX_NPTS 100

typedef struct {
   int npts;                  /* # of points in polygon */
   Pnt p[MAX_NPTS+1];          /* points of polygon (clockwise ordered, looking down on floor) */
   /*    invariant: p[npts] == p[0] */
} Poly;

typedef struct BSPleaf
{
   long tx,ty;                 /* coordinates of texture origin */
   Poly poly;                  /* Polygon of floor area */
   int  floor_type;            /* Resource # of floor texture */
   int  ceil_type;             /* Resource # of ceiling texture */
   int  floor_height;          /* Height of floor relative to 0 = "normal" height */
   int  ceiling_height;        /* Height of ceiling relative to 0 = "normal" height */
   BYTE light;                 /* Light level in leaf */
   WORD server_id;             /* User-id of wall */
   int  sector;                /* Sector # of which leaf is a part */
} BSPleaf;

typedef enum
{
   BSPinternaltype = 1,
   BSPleaftype
} BSPnodetype;

typedef struct BSPnode
{
   BSPnodetype type;           /* type of BSP node */
   Box bbox;                   /* bounding box for this node */
   int  num;                   /* Ordinal # of this node (1 = first node) */
   
   union
   {
      BSPinternal internal;
      BSPleaf leaf;
   } u;
} BSPnode, *BSPTree;

int BSPGetNumNodes(void);
int BSPGetNumWalls(void);

BSPnode *BSPBuildTree(WallData *wall_list, int min_x, int min_y, int max_x, int max_y);
WallData *BSPGetNewWall(void);
BSPTree BSPFreeTree(BSPTree tree);
void BSPDumpTree(BSPnode *tree, int level);
BSPTree BSPRooFileLoad(char *fname);
void BSPTreeFree(void);
BYTE ComputeMoveFlags(BSPTree tree, int row, int col, int rows, int cols,
                      int min_distance);
BYTE ComputeSquareFlags(BSPTree tree, int row, int col, int rows, int cols);
int ComputeHighResSquareFlags(BSPTree tree, int row, int col, int rows, int cols, int min_distance);

#endif  /* #ifndef _BSP_H */
