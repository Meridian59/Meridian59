// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawint.c:  Draw flashy interface elements:
 *   - A scroll around the graphics and text areas.
 *   - A small border around the inventory area.
 *   - A small border around the statistics area.
 *   - Borders around the graph bars.
 *
 * When the window is resized, the positions of the scroll and inventory border are recalculated.
 * The graph bar elements are used around multiple windows; when the window is resized, some
 * elements' positions are recomputed relative to an imaginary window at (0, 0), while other
 * elements are moved each time they are drawn (based on the position of the graph bar they are
 * drawn around).
 */

#include "client.h"
#include "merintr.h"

extern int player_enchant_x;           // X position of left side of first player enchantment
extern int player_enchant_y;           // X position of left side of first player enchantment
extern int player_enchant_right;       // Right side of player enchantment area
extern int player_enchant_bottom;       // bottom side of player enchantment area
extern int room_enchant_x;             // X position of right side of first room enchantment
extern int room_enchant_y;             // Y position of top of first room enchantment
extern int room_enchant_left;          // Left side of room enchantment area
extern int stat_x;           // x position of left of main stats
extern int stat_bar_x;       // x position of left stats bars
extern int stat_width;       // Width of graph bars 

typedef struct {
  int id;          // Windows resource ID of bitmap
  int width, height;
  BYTE *bits;      // Pointer to bits of bitmap, NULL if none
  int x, y;        // Position on main window to draw elements
} InterfaceElement;

//#define NUM_ELEMENTS 50
//#define NUM_AUTO_ELEMENTS 48        // Elements drawn automatically 
#define NUM_ELEMENTS 58
#define NUM_AUTO_ELEMENTS 56        // Elements drawn automatically 

static InterfaceElement elements[NUM_ELEMENTS] = {
  // Main graphics view items
  { IDB_ULTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_ULLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_URTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_URRIGHT,    0,   0,    NULL,   0,   0,  },
  { IDB_LLBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_LLLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_LRBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_LRRIGHT,    0,   0,    NULL,   0,   0,  },

  // Inventory items
  { IDB_IULTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_IULLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_IURTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_IURRIGHT,    0,   0,    NULL,   0,   0,  },
  { IDB_ILLBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_ILLLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_ILRBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_ILRRIGHT,    0,   0,    NULL,   0,   0,  },

  // Stats items
  { IDB_SULTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_SULLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_SURTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_SURRIGHT,    0,   0,    NULL,   0,   0,  },
  { IDB_SLLBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_SLLLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_SLRBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_SLRRIGHT,    0,   0,    NULL,   0,   0,  },

  // Edge treatment - ajw
  { IDB_EULTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_EULLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_EURTOP,      0,   0,    NULL,   0,   0,  },
  { IDB_EURRIGHT,    0,   0,    NULL,   0,   0,  },
  { IDB_ELLBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_ELLLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_ELRBOTTOM,   0,   0,    NULL,   0,   0,  },
  { IDB_ELRRIGHT,    0,   0,    NULL,   0,   0,  },

  // Map treatment - ajw
  { IDB_MULTOP,		0,   0,    NULL,   0,   0,  },
  { IDB_MULLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_MURTOP,		0,   0,    NULL,   0,   0,  },
  { IDB_MURRIGHT,	0,   0,    NULL,   0,   0,  },
  { IDB_MLLBOTTOM,	0,   0,    NULL,   0,   0,  },
  { IDB_MLLLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_MLRBOTTOM,	0,   0,    NULL,   0,   0,  },
  { IDB_MLRRIGHT,	0,   0,    NULL,   0,   0,  },

  // EditBox treatment - ajw (not all bmps are used)
  { IDB_BULTOP,		0,   0,    NULL,   0,   0,  },
  { IDB_BULLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_BURTOP,		0,   0,    NULL,   0,   0,  },
  { IDB_BURRIGHT,	0,   0,    NULL,   0,   0,  },
  { IDB_BLLBOTTOM,	0,   0,    NULL,   0,   0,  },
  { IDB_BLLLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_BLRBOTTOM,	0,   0,    NULL,   0,   0,  },
  { IDB_BLRRIGHT,	0,   0,    NULL,   0,   0,  },

  // personal enchantment treatment - mistery
  { IDB_MULTOP,		0,   0,    NULL,   0,   0,  },
  { IDB_MULLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_MURTOP,		0,   0,    NULL,   0,   0,  },
  { IDB_MURRIGHT,	0,   0,    NULL,   0,   0,  },
  { IDB_MLLBOTTOM,	0,   0,    NULL,   0,   0,  },
  { IDB_MLLLEFT,	0,   0,    NULL,   0,   0,  },
  { IDB_MLRBOTTOM,	0,   0,    NULL,   0,   0,  },
  { IDB_MLRRIGHT,	0,   0,    NULL,   0,   0,  },

  // Graph bar items
  { IDB_BARLEFT,     0,   0,    NULL,   0,   0,  },
  { IDB_BARRIGHT,    0,   0,    NULL,   0,   0,  },
};

// Indices of interface elements in "elements"
#define ELEMENT_ULTOP    0
#define ELEMENT_ULLEFT   1
#define ELEMENT_URTOP    2
#define ELEMENT_URRIGHT  3
#define ELEMENT_LLBOTTOM 4
#define ELEMENT_LLLEFT   5
#define ELEMENT_LRBOTTOM 6
#define ELEMENT_LRRIGHT  7

#define ELEMENT_IULTOP    8
#define ELEMENT_IULLEFT   9
#define ELEMENT_IURTOP    10
#define ELEMENT_IURRIGHT  11
#define ELEMENT_ILLBOTTOM 12
#define ELEMENT_ILLLEFT   13
#define ELEMENT_ILRBOTTOM 14
#define ELEMENT_ILRRIGHT  15

#define ELEMENT_SULTOP    16
#define ELEMENT_SULLEFT   17
#define ELEMENT_SURTOP    18
#define ELEMENT_SURRIGHT  19
#define ELEMENT_SLLBOTTOM 20
#define ELEMENT_SLLLEFT   21
#define ELEMENT_SLRBOTTOM 22
#define ELEMENT_SLRRIGHT  23

#define ELEMENT_EULTOP		24
#define ELEMENT_EULLEFT		25
#define ELEMENT_EURTOP		26
#define ELEMENT_EURRIGHT	27
#define ELEMENT_ELLBOTTOM	28
#define ELEMENT_ELLLEFT		29
#define ELEMENT_ELRBOTTOM	30
#define ELEMENT_ELRRIGHT	31

#define ELEMENT_MULTOP		32
#define ELEMENT_MULLEFT		33
#define ELEMENT_MURTOP		34
#define ELEMENT_MURRIGHT	35
#define ELEMENT_MLLBOTTOM	36
#define ELEMENT_MLLLEFT		37
#define ELEMENT_MLRBOTTOM	38
#define ELEMENT_MLRRIGHT	39

#define ELEMENT_BULTOP		40
#define ELEMENT_BULLEFT		41
#define ELEMENT_BURTOP		42
#define ELEMENT_BURRIGHT	43
#define ELEMENT_BLLBOTTOM	44
#define ELEMENT_BLLLEFT		45
#define ELEMENT_BLRBOTTOM	46
#define ELEMENT_BLRRIGHT	47

#define ELEMENT_PEULTOP		48
#define ELEMENT_PEULLEFT	49
#define ELEMENT_PEURTOP		50
#define ELEMENT_PEURRIGHT	51
#define ELEMENT_PELLBOTTOM	52
#define ELEMENT_PELLLEFT	53
#define ELEMENT_PELRBOTTOM	54
#define ELEMENT_PELRRIGHT	55

#define ELEMENT_BARLEFT   56
#define ELEMENT_BARRIGHT  57

typedef struct {
  InterfaceElement element;
  Bool vertical;              // True = repeat vertically, False = repeat horizontally
  int end;                    // X or Y coordinate to stop repeating element
} InterfaceRepeater;

//#define NUM_REPEATERS 26
//#define NUM_AUTO_REPEATERS 24  // Repeaters drawn automatically
#define NUM_REPEATERS 30
#define NUM_AUTO_REPEATERS 28  // Repeaters drawn automatically

// Elements that are drawn repeatedly along a line
static InterfaceRepeater repeaters[NUM_REPEATERS] = {
  // Main graphics view items
  { IDB_TOP,        0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_BOTTOM,     0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_LEFT,       0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_RIGHT,      0,   0,    NULL,   0,   0,  True,  0, },

  // Inventory items
  { IDB_ITOP,       0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_IBOTTOM,    0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_ILEFT,      0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_IRIGHT,     0,   0,    NULL,   0,   0,  True,  0, },

  // Stats items
  { IDB_STOP,       0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_SBOTTOM,    0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_SLEFT,      0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_SRIGHT,     0,   0,    NULL,   0,   0,  True,  0, },

  // Edge treatment - ajw
  { IDB_EUREPEAT,    0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_EBREPEAT,    0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_ELREPEAT,    0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_ERREPEAT,    0,   0,    NULL,   0,   0,  True,  0, },

  // Map treatment - ajw
  { IDB_MUREPEAT,   0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_MLREPEAT,   0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_MRREPEAT,   0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_MBREPEAT,   0,   0,    NULL,   0,   0,  False, 0, },

  // EditBox treatment - ajw
  { IDB_BUREPEAT,   0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_BBREPEAT,   0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_BLREPEAT,   0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_BRREPEAT,   0,   0,    NULL,   0,   0,  True,  0, },

  // personal enchantment treatment - mistery
  { IDB_MUREPEAT,   0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_MLREPEAT,   0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_MRREPEAT,   0,   0,    NULL,   0,   0,  True,  0, },
  { IDB_MBREPEAT,   0,   0,    NULL,   0,   0,  False, 0, },

  // Graph bar items
  { IDB_BARTOP,     0,   0,    NULL,   0,   0,  False, 0, },
  { IDB_BARBOTTOM,  0,   0,    NULL,   0,   0,  False, 0, },
};

// Indices of interface elements in "repeaters"
#define ELEMENT_TOP      0
#define ELEMENT_BOTTOM   1
#define ELEMENT_LEFT     2
#define ELEMENT_RIGHT    3

#define ELEMENT_ITOP      4
#define ELEMENT_IBOTTOM   5
#define ELEMENT_ILEFT     6
#define ELEMENT_IRIGHT    7

#define ELEMENT_STOP      8
#define ELEMENT_SBOTTOM   9
#define ELEMENT_SLEFT     10
#define ELEMENT_SRIGHT    11

#define ELEMENT_ETOP      12
#define ELEMENT_EBOTTOM   13
#define ELEMENT_ELEFT     14
#define ELEMENT_ERIGHT    15

#define ELEMENT_MTOP      16
#define ELEMENT_MLEFT     17
#define ELEMENT_MRIGHT    18
#define ELEMENT_MBOTTOM   19

#define ELEMENT_BTOP      20
#define ELEMENT_BBOTTOM   21
#define ELEMENT_BLEFT     22
#define ELEMENT_BRIGHT    23

#define ELEMENT_PETOP      24
#define ELEMENT_PELEFT     25
#define ELEMENT_PERIGHT    26
#define ELEMENT_PEBOTTOM   27

#define ELEMENT_BARTOP    28
#define ELEMENT_BARBOTTOM 29

static void InterfaceLoadElement(InterfaceElement *element);
/****************************************************************************/
/*
 * InterfaceDrawInit:  Load interface bitmaps.
 */
void InterfaceDrawInit(void)
{
  int i;

  for (i=0; i < NUM_ELEMENTS; i++)
    InterfaceLoadElement(&elements[i]);

  for (i=0; i < NUM_REPEATERS; i++)
    InterfaceLoadElement(&repeaters[i].element);

  // Set some bar element positions, based on bar at (0, 0)
  elements[ELEMENT_BARLEFT].x = -elements[ELEMENT_BARLEFT].width;
  elements[ELEMENT_BARLEFT].y = -repeaters[ELEMENT_BARTOP].element.height;
  
  elements[ELEMENT_BARRIGHT].y = -repeaters[ELEMENT_BARTOP].element.height;

  repeaters[ELEMENT_BARTOP].element.x = 0;
  repeaters[ELEMENT_BARTOP].element.y = -repeaters[ELEMENT_BARTOP].element.height;

  repeaters[ELEMENT_BARBOTTOM].element.x = 0;
}
/****************************************************************************/
/*
 * InterfaceDrawExit:  Free interface bitmaps.
 */
void InterfaceDrawExit(void)
{
  // Don't need to free anything; these are loaded from resource file
}
/****************************************************************************/
/*
 * InterfaceLoadElement:  Initialize given InterfaceElements structure by
 *   getting pointer to bitmap from resource file.
 */
void InterfaceLoadElement(InterfaceElement *element)
{
   BITMAPINFOHEADER *ptr;
   
   ptr = GetBitmapResource(hInst, element->id);
   if (ptr == NULL)
   {
     element->bits = NULL;
     return;
   }
   
   element->width  = ptr->biWidth;
   element->height = ptr->biHeight;
   element->bits = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
}
/****************************************************************************/
/*
 * InterfaceDrawResize:  Called when main window resized; recompute positions 
 *   of interface elements.
 */
void InterfaceDrawResize(int xsize, int ysize, AREA *view)
{
  AREA inv, stat, areaMiniMap, areaEditBox, areaPersonalEnch;
  int i, x, y, width, height, end;
  HDC hdc;

  InventoryGetArea(&inv);
  StatsGetArea(&stat);

  areaPersonalEnch.x = player_enchant_x - 1;
  areaPersonalEnch.cx = USERAREA_WIDTH + stat_width + (stat_bar_x - stat_x) + MAPTREAT_WIDTH + 1;
  areaPersonalEnch.y = TOP_BORDER + EDGETREAT_HEIGHT + ENCHANT_BORDER - 1;
  areaPersonalEnch.cy = (player_enchant_bottom - player_enchant_y) + USERAREA_HEIGHT +
	  ENCHANT_BORDER + MAPTREAT_HEIGHT;

   CopyCurrentAreaMiniMap( &areaMiniMap );
  
   EditBoxGetArea( &areaEditBox );
   areaEditBox.cy += GetTextInputHeight() - 2;

  for (i=0; i < NUM_ELEMENTS; i++)
  {
    width  = elements[i].width;
    height = elements[i].height;
    switch (i)
    {
    case ELEMENT_ULTOP:
		x = view->x - elements[ELEMENT_ULLEFT].width;
		y = view->y - height;
      break;
    case ELEMENT_ULLEFT:
		x = view->x - width;
		y = view->y;
      break;
    case ELEMENT_URTOP:
		x = view->x + view->cx - width + elements[ELEMENT_URRIGHT].width;
		y = view->y - height;
      break;
    case ELEMENT_URRIGHT:
		x = view->x + view->cx;
		y = view->y;
      break;
    case ELEMENT_LLLEFT:
		x = view->x - width;
		y = view->y + view->cy - height;
      break;
    case ELEMENT_LLBOTTOM:
		x = view->x - elements[ELEMENT_LLLEFT].width;
		y = view->y + view->cy;
      break;
    case ELEMENT_LRBOTTOM:
		x = view->x + view->cx - width + elements[ELEMENT_URRIGHT].width;
		y = view->y + view->cy;
      break;
    case ELEMENT_LRRIGHT:
		x = view->x + view->cx;
		y = view->y + view->cy - height;
      break;

    case ELEMENT_IULTOP:
      x = inv.x;
      y = inv.y - height;
      break;
    case ELEMENT_IULLEFT:
      x = inv.x - width;
      y = inv.y - elements[ELEMENT_IULTOP].height;
      break;
    case ELEMENT_IURTOP:
      x = inv.x + inv.cx - width;
      y = inv.y - height;
      break;
    case ELEMENT_IURRIGHT:
      x = inv.x + inv.cx;
      y = inv.y - elements[ELEMENT_IURTOP].height;
      break;
    case ELEMENT_ILLLEFT:
      x = inv.x - width;
      y = inv.y + inv.cy - height + elements[ELEMENT_ILLBOTTOM].height;
      break;
    case ELEMENT_ILLBOTTOM:
      x = inv.x;
      y = inv.y + inv.cy;
      break;
    case ELEMENT_ILRBOTTOM:
      x = inv.x + inv.cx - width;
      y = inv.y + inv.cy;
      break;
    case ELEMENT_ILRRIGHT:
      x = inv.x + inv.cx;
      y = inv.y + inv.cy - height + elements[ELEMENT_ILRBOTTOM].height;
      break;

    case ELEMENT_SULTOP:
      x = stat.x - elements[ELEMENT_SULLEFT].width;
      y = stat.y - height;
      break;
    case ELEMENT_SULLEFT:
      x = stat.x - width;
      y = stat.y;
      break;
    case ELEMENT_SURTOP:
      x = stat.x + stat.cx - width + elements[ELEMENT_SURRIGHT].width;
      y = stat.y - height;
      break;
    case ELEMENT_SURRIGHT:
      x = stat.x + stat.cx;
      y = stat.y;
      break;
    case ELEMENT_SLLLEFT:
      x = stat.x - width;
      y = stat.y + stat.cy - height;
      break;
    case ELEMENT_SLLBOTTOM:
      x = stat.x - elements[ELEMENT_SLLLEFT].width;
      y = stat.y + stat.cy;
      break;
    case ELEMENT_SLRBOTTOM:
      x = stat.x + stat.cx - width + elements[ELEMENT_SLRRIGHT].width;
      y = stat.y + stat.cy;
      break;
    case ELEMENT_SLRRIGHT:
      x = stat.x + stat.cx;
      y = stat.y + stat.cy - height;
      break;

	case ELEMENT_EULTOP:
		x = 0;
		y = 0;
		break;
	case ELEMENT_EULLEFT:
		x = 0;
		y = elements[ ELEMENT_EULTOP ].height;
		break;
	case ELEMENT_EURTOP:
		x = xsize - width;
		y = 0;
		break;
	case ELEMENT_EURRIGHT:
		x = xsize - width;
		y = elements[ ELEMENT_EURTOP ].height;
		break;
	case ELEMENT_ELLBOTTOM:
		x = 0;
		y = ysize - height;
		break;
	case ELEMENT_ELLLEFT:
		x = 0;
		y = ysize - elements[ ELEMENT_ELLBOTTOM ].height - height;
		break;
	case ELEMENT_ELRBOTTOM:
		x = xsize - width;
		y = ysize - height;
		break;
	case ELEMENT_ELRRIGHT:
		x = xsize - width;
		y = ysize - elements[ ELEMENT_ELRBOTTOM ].height - height;
		break;

	case ELEMENT_MULTOP:
		x = areaMiniMap.x;
		y = areaMiniMap.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_MULLEFT:
		//	Map upper left corner, left side bitmap. Corner bitmaps extend vertically on the map instead of horizontally.
		x = areaMiniMap.x - MAPTREAT_WIDTH;
		y = areaMiniMap.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_MURTOP:
		x = areaMiniMap.x + areaMiniMap.cx - width;
		y = areaMiniMap.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_MURRIGHT:
		x = areaMiniMap.x + areaMiniMap.cx;
		y = areaMiniMap.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_MLLBOTTOM:
		x = areaMiniMap.x;
		y = areaMiniMap.y + areaMiniMap.cy + MAPTREAT_HEIGHT - height;
		break;
	case ELEMENT_MLLLEFT:
		x = areaMiniMap.x - MAPTREAT_WIDTH;
		y = areaMiniMap.y + areaMiniMap.cy + MAPTREAT_HEIGHT - height;
		break;
	case ELEMENT_MLRBOTTOM:
		x = areaMiniMap.x + areaMiniMap.cx - width;
		y = areaMiniMap.y + areaMiniMap.cy + MAPTREAT_HEIGHT - height;
		break;
	case ELEMENT_MLRRIGHT:
		x = areaMiniMap.x + areaMiniMap.cx;
		y = areaMiniMap.y + areaMiniMap.cy + MAPTREAT_HEIGHT - height;
		break;

		//	EditBox treatment. Alignment is like inventory treatment - "side" bitmaps extend into corners.
		//	Bottom is now stripped off.
    case ELEMENT_BULTOP:
      x = areaEditBox.x;
      y = areaEditBox.y - height;
      break;
    case ELEMENT_BULLEFT:
      x = areaEditBox.x - width;
      y = areaEditBox.y - elements[ELEMENT_BULTOP].height;
      break;
    case ELEMENT_BURTOP:
      x = areaEditBox.x + areaEditBox.cx - width;
      y = areaEditBox.y - height;
      break;
    case ELEMENT_BURRIGHT:
      x = areaEditBox.x + areaEditBox.cx;
      y = areaEditBox.y - elements[ELEMENT_BURTOP].height;
      break;
    case ELEMENT_BLLLEFT:
      x = areaEditBox.x - width;
      y = areaEditBox.y + areaEditBox.cy - height;		// + elements[ELEMENT_BLLBOTTOM].height;
      break;
    case ELEMENT_BLLBOTTOM:
      x = areaEditBox.x;
      y = areaEditBox.y + areaEditBox.cy;
      break;
    case ELEMENT_BLRBOTTOM:
      x = areaEditBox.x + areaEditBox.cx - width;
      y = areaEditBox.y + areaEditBox.cy;
      break;
    case ELEMENT_BLRRIGHT:
      x = areaEditBox.x + areaEditBox.cx;
      y = areaEditBox.y + areaEditBox.cy - height;		// + elements[ELEMENT_BLRBOTTOM].height;
      break;

  // new personal enchantment stuff
	  case ELEMENT_PEULTOP:
		x = areaPersonalEnch.x;
		y = areaPersonalEnch.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_PEULLEFT:
		//	Map upper left corner, left side bitmap. Corner bitmaps extend vertically on the map instead of horizontally.
		x = areaPersonalEnch.x - MAPTREAT_WIDTH;
		y = areaPersonalEnch.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_PEURTOP:
		x = areaPersonalEnch.x + areaPersonalEnch.cx - width;
		y = areaPersonalEnch.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_PEURRIGHT:
		x = areaPersonalEnch.x + areaPersonalEnch.cx;
		y = areaPersonalEnch.y - MAPTREAT_HEIGHT;
		break;
	case ELEMENT_PELLBOTTOM:
		x = areaPersonalEnch.x;
		y = areaPersonalEnch.y + areaPersonalEnch.cy + MAPTREAT_HEIGHT - height;
		break;
	case ELEMENT_PELLLEFT:
		x = areaPersonalEnch.x - MAPTREAT_WIDTH;
		y = areaPersonalEnch.y + areaPersonalEnch.cy + MAPTREAT_HEIGHT - height;
		break;
	case ELEMENT_PELRBOTTOM:
		x = areaPersonalEnch.x + areaPersonalEnch.cx - width;
		y = areaPersonalEnch.y + areaPersonalEnch.cy + MAPTREAT_HEIGHT - height;
		break;
	case ELEMENT_PELRRIGHT:
		x = areaPersonalEnch.x + areaPersonalEnch.cx;
		y = areaPersonalEnch.y + areaPersonalEnch.cy + MAPTREAT_HEIGHT - height;
		break;

    default:
      continue;
    }
    
    elements[i].x = x;
    elements[i].y = y;
  }
  
  for (i=0; i < NUM_REPEATERS; i++)
  {
    width  = repeaters[i].element.width;
    height = repeaters[i].element.height;
    switch (i)
    {
    case ELEMENT_ITOP:
      x = elements[ELEMENT_IULTOP].x + elements[ELEMENT_IULTOP].width;
      y = inv.y - height;
      end = elements[ELEMENT_IURTOP].x;
      break;
    case ELEMENT_IBOTTOM:
      x = elements[ELEMENT_ILLBOTTOM].x + elements[ELEMENT_ILLBOTTOM].width;
      y = inv.y + inv.cy;
      end = elements[ELEMENT_ILRBOTTOM].x;
      break;
    case ELEMENT_ILEFT:
      x = inv.x - width;
      y = elements[ELEMENT_IULLEFT].y + elements[ELEMENT_IULLEFT].height;
      end = elements[ELEMENT_ILLLEFT].y;
      break;
    case ELEMENT_IRIGHT:
      x = inv.x + inv.cx;
      y = elements[ELEMENT_IURRIGHT].y + elements[ELEMENT_IURRIGHT].height;
      end = elements[ELEMENT_ILRRIGHT].y;
      break;

    case ELEMENT_STOP:
      x = elements[ELEMENT_SULTOP].x + elements[ELEMENT_SULTOP].width;
      y = stat.y - height;
      end = elements[ELEMENT_SURTOP].x;
      break;
    case ELEMENT_SBOTTOM:
      x = elements[ELEMENT_SLLBOTTOM].x + elements[ELEMENT_SLLBOTTOM].width;
      y = stat.y + stat.cy;
      end = elements[ELEMENT_SLRBOTTOM].x;
      break;
    case ELEMENT_SLEFT:
      x = stat.x - width;
      y = elements[ELEMENT_SULLEFT].y + elements[ELEMENT_SULLEFT].height;
      end = elements[ELEMENT_SLLLEFT].y;
      break;
    case ELEMENT_SRIGHT:
      x = stat.x + stat.cx;
      y = elements[ELEMENT_SURRIGHT].y + elements[ELEMENT_SURRIGHT].height;
      end = elements[ELEMENT_SLRRIGHT].y;
      break;

	case ELEMENT_ETOP:
		x = elements[ ELEMENT_EULTOP ].width;
		y = 0;
		end = elements[ ELEMENT_EURTOP ].x;
		break;
	case ELEMENT_EBOTTOM:
		x = elements[ ELEMENT_ELLBOTTOM ].width;
		y = ysize - height;
		end = elements[ ELEMENT_ELRBOTTOM ].x;
		break;
	case ELEMENT_ELEFT:
		x = 0;
		y = elements[ ELEMENT_EULTOP ].height + elements[ ELEMENT_EULLEFT ].height;
		end = elements[ ELEMENT_ELLLEFT ].y;
		break;
	case ELEMENT_ERIGHT:
		x = xsize - width;
		y = elements[ ELEMENT_EURTOP ].height + elements[ ELEMENT_EURRIGHT ].height;
		end = elements[ ELEMENT_ELRRIGHT ].y;
		break;

	case ELEMENT_MTOP:
		x = elements[ ELEMENT_MULTOP ].x + elements[ ELEMENT_MULTOP ].width;
		y = elements[ ELEMENT_MULTOP ].y;
		end = elements[ ELEMENT_MURTOP ].x;
		break;
	case ELEMENT_MBOTTOM:
		x = elements[ ELEMENT_MLLBOTTOM ].x + elements[ ELEMENT_MLLBOTTOM ].width;
		y = elements[ ELEMENT_MLLBOTTOM ].y;
		end = elements[ ELEMENT_MLRBOTTOM ].x;
		break;
	case ELEMENT_MLEFT:
		x = elements[ ELEMENT_MULLEFT ].x;
		y = elements[ ELEMENT_MULLEFT ].y + elements[ ELEMENT_MULLEFT ].height;
		end = elements[ ELEMENT_MLLLEFT ].y;
		break;
	case ELEMENT_MRIGHT:
		x = elements[ ELEMENT_MURRIGHT ].x + elements[ ELEMENT_MURRIGHT ].width - width;
		y = elements[ ELEMENT_MURRIGHT ].y + elements[ ELEMENT_MURRIGHT ].height;
		end = elements[ ELEMENT_MLRRIGHT ].y;
		break;

    case ELEMENT_BTOP:
      x = elements[ELEMENT_BULTOP].x + elements[ELEMENT_BULTOP].width;
      y = areaEditBox.y - height;
      end = elements[ELEMENT_BURTOP].x;
      break;
    case ELEMENT_BBOTTOM:
      x = elements[ELEMENT_BLLBOTTOM].x + elements[ELEMENT_BLLBOTTOM].width;
      y = areaEditBox.y + areaEditBox.cy;
      end = elements[ELEMENT_BLRBOTTOM].x;
      break;
    case ELEMENT_BLEFT:
      x = areaEditBox.x - width;
      y = elements[ELEMENT_BULLEFT].y + elements[ELEMENT_BULLEFT].height;
      end = elements[ELEMENT_BLLLEFT].y;
      break;
    case ELEMENT_BRIGHT:
      x = areaEditBox.x + areaEditBox.cx;
      y = elements[ELEMENT_BURRIGHT].y + elements[ELEMENT_BURRIGHT].height;
      end = elements[ELEMENT_BLRRIGHT].y;
      break;

	  // new personal enchantment stuff
	case ELEMENT_PETOP:
		x = elements[ ELEMENT_PEULTOP ].x + elements[ ELEMENT_PEULTOP ].width;
		y = elements[ ELEMENT_PEULTOP ].y;
		end = elements[ ELEMENT_PEURTOP ].x;
		break;
	case ELEMENT_PEBOTTOM:
		x = elements[ ELEMENT_PELLBOTTOM ].x + elements[ ELEMENT_PELLBOTTOM ].width;
		y = elements[ ELEMENT_PELLBOTTOM ].y;
		end = elements[ ELEMENT_PELRBOTTOM ].x;
		break;
	case ELEMENT_PELEFT:
		x = elements[ ELEMENT_PEULLEFT ].x;
		y = elements[ ELEMENT_PEULLEFT ].y + elements[ ELEMENT_PEULLEFT ].height;
		end = elements[ ELEMENT_PELLLEFT ].y;
		break;
	case ELEMENT_PERIGHT:
		x = elements[ ELEMENT_PEURRIGHT ].x + elements[ ELEMENT_PEURRIGHT ].width - width;
		y = elements[ ELEMENT_PEURRIGHT ].y + elements[ ELEMENT_PEURRIGHT ].height;
		end = elements[ ELEMENT_PELRRIGHT ].y;
		break;

    default:
      continue;
    }    

    repeaters[i].element.x = x;
    repeaters[i].element.y = y;
    repeaters[i].end = end;
  }

  hdc = GetDC(cinfo->hMain);
  SelectPalette(hdc, cinfo->hPal, FALSE);
  InterfaceDrawElements(hdc);
  ReleaseDC(cinfo->hMain, hdc);
}
/****************************************************************************/
/*
 * InterfaceDrawElements:  Draw spiffy interface elements.
 */
void InterfaceDrawElements(HDC hdc)
{
  int i, x, y;
  InterfaceElement *e;
  Bool vertical;

  for (i=0; i < NUM_AUTO_ELEMENTS; i++)
  {
	  //	Temp. disable xxx
/*	  if( i >= ELEMENT_ULTOP && i <= ELEMENT_LRRIGHT )
		  continue;
	  if( i >= ELEMENT_IULTOP && i <= ELEMENT_ILRRIGHT )
		  continue;
	  if( i == ELEMENT_BLLBOTTOM || i == ELEMENT_BLRBOTTOM )
		  continue;*/

    OffscreenWindowBackground(NULL, elements[i].x, elements[i].y, elements[i].width, elements[i].height);
    
    OffscreenBitBlt(hdc, elements[i].x, elements[i].y, elements[i].width, elements[i].height,
		    elements[i].bits, 0, 0, DIBWIDTH(elements[i].width), 
		    OBB_COPY | OBB_FLIP | OBB_TRANSPARENT);
    GdiFlush();
  }

  for (i=0; i < NUM_AUTO_REPEATERS; i++)
  {
	  //	Temp. disable xxx
	if( i >= ELEMENT_ITOP && i <= ELEMENT_IRIGHT )
		continue;
	  //	disable xxx
	if( i >= ELEMENT_TOP && i <= ELEMENT_RIGHT )
		continue;
	if( i == ELEMENT_BBOTTOM )
		continue;

	e = &repeaters[i].element;
    x = e->x;
    y = e->y;
    vertical = repeaters[i].vertical;
        
    while (1)
    {
      if ((vertical && y > repeaters[i].end) || (!vertical && x > repeaters[i].end))
	break;
      
      OffscreenWindowBackground(NULL, x, y, e->width, e->height);
      OffscreenBitBlt(hdc, x, y, e->width, e->height, e->bits, 0, 0, DIBWIDTH(e->width), 
		      OBB_COPY | OBB_FLIP | OBB_TRANSPARENT);
      if (vertical)
	y += e->height;
      else x += e->width;

      GdiFlush();
    }
  }
}
/****************************************************************************/
/*
 * InterfaceDrawBarBorder:  Draw border around a graph bar; the bar occupies
 *   the area "a" on hdc.
 */
void InterfaceDrawBarBorder( RawBitmap* prawbmpBackground, HDC hdc, AREA *a )
{
  int i, x, y;
  InterfaceElement *e;
  Bool vertical;

  elements[ELEMENT_BARRIGHT].x = a->cx;

  for (i = ELEMENT_BARLEFT; i <= ELEMENT_BARRIGHT; i++)
  {
    OffscreenWindowBackground(prawbmpBackground, a->x + elements[i].x, a->y + elements[i].y, 
			      elements[i].width, elements[i].height);
    
    OffscreenBitBlt(hdc, a->x + elements[i].x, a->y + elements[i].y, 
		    elements[i].width, elements[i].height,
		    elements[i].bits, 0, 0, DIBWIDTH(elements[i].width), 
		    OBB_COPY | OBB_FLIP | OBB_TRANSPARENT);
    GdiFlush();
  }

  repeaters[ELEMENT_BARBOTTOM].element.y = a->cy;

  for (i = ELEMENT_BARTOP; i <= ELEMENT_BARBOTTOM; i++)
  {
    e = &repeaters[i].element;
    x = e->x + a->x;
    y = e->y + a->y;
    vertical = repeaters[i].vertical;

    while (1)
    {
      if ((vertical && y >= a->y + a->cy) || (!vertical && x >= a->x + a->cx))
	break;

      OffscreenWindowBackground(prawbmpBackground, x, y, e->width, e->height);
      OffscreenBitBlt(hdc, x, y, e->width, e->height, e->bits, 0, 0, DIBWIDTH(e->width), 
		      OBB_COPY | OBB_FLIP | OBB_TRANSPARENT);
      if (vertical)
	y += e->height;
      else x += e->width;

      GdiFlush();
    }
  }
}

