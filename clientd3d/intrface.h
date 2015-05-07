// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * intrface.h:  Header file for intrface.c
 */

#ifndef _INTRFACE_H
#define _INTRFACE_H

#define TEXT_AREA_MIN_HEIGHT 96

/* Grid view area */
#define GRID_TOP_BORDER  5
#define HIGHLIGHT_THICKNESS 2

//	Distance from bottom of view to top of editbox.
#define EDITAREA_TOP_GAP 15

/* Area left free on the sides of the screen */
//#define BOTTOM_BORDER (11 + HIGHLIGHT_THICKNESS)
#define BOTTOM_BORDER  0
#define TOP_BORDER     5
#define RIGHT_BORDER   5
#define LEFT_BORDER    8
//	ajw
#define EDGETREAT_WIDTH		16
#define EDGETREAT_HEIGHT	16
#define MAPTREAT_WIDTH		5
#define MAPTREAT_HEIGHT		5

//	ajw Extra vertical clearance for Inventory - height of stats groups buttons plus spacing.
#define GROUPBUTTONS_HEIGHT 20

/* Space between toolbar and main graphics window */
#define GRAPHICS_TOP_HEIGHT 24

/* Inventory area */
//#define INVENTORY_MIN_WIDTH (170 + LEFT_BORDER)
#define INVENTORY_MIN_WIDTH (185 + LEFT_BORDER + 67)
#define INVENTORY_MAX_WIDTH (250 + LEFT_BORDER)

//	MiniMap area.
#define MINIMAP_MAX_WIDTH	( INVENTORY_MAX_WIDTH + 3 ) & ~3
#define MINIMAP_MAX_HEIGHT	MINIMAP_MAX_WIDTH

//	How much of the minimap/stats box area goes to the minimap.
#define PROPORTION_MINIMAP		.4

//	ajw Extra space between minimap and stats area.
#define MAP_STATS_GAP_HEIGHT	4
//	Extra space below stats area.
#define STATS_BOTTOM_GAP_HEIGHT	2

/* List of actions that can be taken by user */
enum { A_NOACTION = 0, 
	  A_PICKUP, A_QUIT, A_DROP, A_PUT, A_SAY, A_TABFWD, A_TABBACK, 
	  A_USE, A_UNUSE, A_LOOK, A_LOOKMOUSE, A_OFFER, A_ATTACK,
	  A_GOTOMAIN, A_GO, A_YELL, A_BUY, 
	  A_DEPOSIT, A_WITHDRAW,
	  A_APPLY, A_SELECT, A_ENDSELECT,
	  A_CURSORLEFT, A_CURSORRIGHT, A_CURSORUP, A_CURSORDOWN,
	  A_CURSORUPLEFT, A_CURSORUPRIGHT, A_CURSORDOWNRIGHT, A_CURSORDOWNLEFT,
	  A_FORWARD, A_BACKWARD, A_SLIDELEFT, A_SLIDERIGHT,
	  A_SLIDELEFTFORWARD, A_SLIDELEFTBACKWARD,
	  A_SLIDERIGHTFORWARD, A_SLIDERIGHTBACKWARD,
	  A_FORWARDFAST, A_BACKWARDFAST,
	  A_SLIDELEFTFAST, A_SLIDERIGHTFAST,
	  A_SLIDELEFTFORWARDFAST, A_SLIDELEFTBACKWARDFAST,
	  A_SLIDERIGHTFORWARDFAST, A_SLIDERIGHTBACKWARDFAST,
	  A_TURNLEFT, A_TURNRIGHT,
	  A_FORWARDTURNLEFT, A_FORWARDTURNRIGHT, A_BACKWARDTURNLEFT, A_BACKWARDTURNRIGHT, 
	  A_FORWARDTURNFASTLEFT, A_FORWARDTURNFASTRIGHT, 
	  A_BACKWARDTURNFASTLEFT, A_BACKWARDTURNFASTRIGHT, 
	  A_TURNFASTLEFT, A_TURNFASTRIGHT,
	  A_QUITPROGRAM, A_MOUSELOOK,
	  A_MAPZOOMIN, A_MAPZOOMOUT, A_TOGGLEUSE, 
	  A_MOUSEMOVE, A_MAP, A_LOOKUP, A_LOOKDOWN, A_LOOKSTRAIGHT, 
	  A_GOTOSAY, A_NEXT, A_PREV, A_STARTDRAG, A_ENDDRAG, 
	  A_WHO, A_ATTACKCLOSEST, A_TEXTCOMMAND, A_HELP, A_FLIP,
	  A_TEXTINSERT, A_USERACTION, A_LOOKINSIDE, A_ACTIVATE,
	  A_ACTIVATEMOUSE, A_CHANGEPASSWORD, 
	  A_TARGETPREVIOUS, A_TARGETNEXT, A_TARGETCLEAR, A_TARGETSELF,
	  A_CURSOR_ESC,
    };

/* Modules should use action codes starting at A_MODULE to avoid conflicts with client */
#define A_MODULE  1000


#define IsMoveAction(action)   ((( (action) >= A_FORWARD &&          \
				  (action) <= A_SLIDERIGHTBACKWARDFAST))  \
				|| ((action) == A_MOUSEMOVE))
#define IsTurnAction(action)   ((( (action) >= A_TURNLEFT && (action) <= A_TURNFASTRIGHT)) \
				|| ((action) == A_MOUSEMOVE))
#define IsMoveFastAction(action)   ((( (action) >= A_FORWARDFAST &&          \
				  (action) <= A_SLIDERIGHTBACKWARDFAST))  \
				|| ((action) == A_MOUSEMOVE))
#define IsTurnFastAction(action)   ((( (action) >= A_FORWARDTURNFASTLEFT && (action) <= A_TURNFASTRIGHT)) \
				|| ((action) == A_MOUSEMOVE))
#define IsCursorAction(action) ( (action) >= A_CURSORLEFT && (action) <= A_CURSORDOWNLEFT)
#define IsAttackAction(action) ( (action) == A_ATTACK || (action) == A_ATTACKCLOSEST)
#define IsViewAction(action)   ( (action) == A_LOOKUP || (action) == A_LOOKDOWN)
#define IsMapMoveAction(action)      ( (action) == A_MAPZOOMIN || (action) == A_MAPZOOMOUT)
#define IsTabAction(action) ( (action) == A_TABFWD || (action) == A_TABBACK)
#define IsMouseLookAction(action) ( (action) == A_MOUSELOOK)
// True iff action should cause auto-repeat
#define RepeatAction(a) (IsMoveAction(a) || IsTurnAction(a) || IsCursorAction(a) || \
			 IsViewAction(a) || IsMapMoveAction(a) || IsTabAction(a) || IsMouseLookAction(a))

#ifdef __cplusplus
extern "C" {
#endif

void InterfaceInitialize(HWND hParent);
void InterfaceResize(int xsize, int ysize);
void InterfaceClose(void);
void InterfaceRedraw(HDC hdc);
void InterfaceGetMaxSize(SIZE *s);

void GameChangeFont(void);
void GameChangeColor(void);

void MainTab(int ctrl, Bool forward);

M59EXPORT void PerformAction(int action, void *action_data);

#ifdef __cplusplus
};
#endif

#endif /* #ifndef _INTRFACE_H */
