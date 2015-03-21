// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statbtn.c:  Handle display of the statistic group buttons on the main window.
 *
 * Each stat button is owner drawn; its bitmap consists of the unpushed and pushed states
 * next to each other in a single bitmap. 
 */

#include "client.h"
#include "merintr.h"

typedef struct {
  int idLeft;           // Windows resource id of bitmaps used for left, middle, and right pieces of button.
  int idMid;			//
  int idRight;			//
  HWND hwnd;            // Window containing button
  char *bitsLeft;       // Pointer to bits of bitmap
  char *bitsMid;        // Pointer to bits of bitmap
  char *bitsRight;      // Pointer to bits of bitmap
  int  width, height;   // Full size of button.
  int  iWidthLeft;		//	Width of left bitmap used for button.
  int  iWidthMid;		//	Width of repeating center piece bitmap used for button.
  int  iWidthRight;		//	Width of right bitmap used for button.
  int  x, y;            // Location of button relative to top left of stat area
  int  name;            // Resource identifier for tooltip name string (0 if none)
} StatButton;

#define NUM_BUTTONS 4

static StatButton buttons[NUM_BUTTONS] = {
  { IDB_SBUTTON1_LEFT, IDB_SBUTTON1_MID, IDB_SBUTTON1_RIGHT, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, IDS_TT_STATS },
  { IDB_SBUTTON2_LEFT, IDB_SBUTTON2_MID, IDB_SBUTTON2_RIGHT, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, IDS_TT_SPELLS },
  { IDB_SBUTTON3_LEFT, IDB_SBUTTON3_MID, IDB_SBUTTON3_RIGHT, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, IDS_TT_SKILLS },
  { IDB_SBUTTON4_LEFT, IDB_SBUTTON4_MID, IDB_SBUTTON4_RIGHT, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, IDS_TT_INVENTORY },
};

/* Keys to override default actions */
static keymap stats_key_table[] = {
{ VK_TAB,         KEY_NONE,                         A_TABFWD,    (void *) IDC_STATS },
{ VK_TAB,         KEY_SHIFT,                        A_TABBACK,   (void *) IDC_STATS },
{ VK_ESCAPE,      KEY_ANY,                          A_GOTOMAIN },
{ VK_SINGLEQUOTE, KEY_ANY,                          A_GOTOSAY },
{ VK_RIGHT,       KEY_ANY,                          A_NEXT },
{ VK_LEFT,        KEY_ANY,                          A_PREV },
{ 0, 0, 0},   // Must end table this way
};

extern HWND hStats;

static WNDPROC lpfnDefButtonProc; // Default stats group window procedure
static int max_width, max_height; // Maximum sizes of a button

static int button_border = 0;         // Height of area reserved for buttons

/* local function prototypes */
static void StatsCreateButtons(void);
static long CALLBACK StatButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void StatsMoveButtonFocus(HWND button, Bool forward);
/************************************************************************/
/*
 * StatButtonsCreate:  Load bitmaps for stat buttons.
 */
void StatButtonsCreate(void)
{
	int i;
	BITMAPINFOHEADER *ptr;

	for (i=0; i < NUM_BUTTONS; i++)
	{
		ptr = GetBitmapResource(hInst, buttons[i].idLeft);
		if (ptr == NULL)
		{
			buttons[i].bitsLeft = NULL;
			continue;
		}
		buttons[i].height = ptr->biHeight;
		buttons[i].bitsLeft = (char *) ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
		buttons[i].iWidthLeft	= ptr->biWidth / 2;		//	Bitmap has up and down images in it, so divide by 2.

		ptr = GetBitmapResource(hInst, buttons[i].idMid);
		if (ptr == NULL)
		{
			buttons[i].bitsMid = NULL;
			continue;
		}
		buttons[i].bitsMid =  (char *) ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
		buttons[i].iWidthMid	= ptr->biWidth / 2;		//	Bitmap has up and down images in it, so divide by 2.

		ptr = GetBitmapResource(hInst, buttons[i].idRight);
		if (ptr == NULL)
		{
			buttons[i].bitsRight = NULL;
			continue;
		}
		buttons[i].bitsRight	= (char *) ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
		buttons[i].iWidthRight	= ptr->biWidth / 2;		//	Bitmap has up and down images in it, so divide by 2.
	}
}
/************************************************************************/
/*
 * StatButtonsDestroy:  Free bitmaps for stat buttons.
 */
void StatButtonsDestroy(void)
{
}
/************************************************************************/
/*
 * StatsSetButtons:  Set and create buttons for stat groups.
 */
void StatsSetButtons(int num_groups)
{
   if (num_groups == NUM_BUTTONS + 1)
     StatsCreateButtons();
   else debug(("StatsSetButtons got %d buttons; expecting %d\n", num_groups, NUM_BUTTONS + 1));
}
/************************************************************************/
/*
 * StatsCreateButtons:  Create buttons for the different possible stat groups.
 */
void StatsCreateButtons(void)
{
   int i, max_height = 0;
   StatButton *button;

   // Get rid of existent button windows
   for (i = 0; i < NUM_BUTTONS; i++)
   {
      button = &buttons[i];
      if (button->hwnd != NULL)
	DestroyWindow(button->hwnd);

      button->hwnd = CreateWindow("button", NULL, WS_CHILD | BS_OWNERDRAW, 
				  0, 0, 0, 0,
				  hStats, (HMENU) IDC_STATBUTTON, hInst, NULL);
      lpfnDefButtonProc = SubclassWindow(button->hwnd, StatButtonProc);
      max_height = max(max_height, button->height);
		if( button->name != 0 )
			TooltipAddWindow( button->hwnd, hInst, button->name );
   }

   StatsMoveButtons();
   button_border = max_height + STATUS_BUTTON_SPACING;
}
/************************************************************************/
/*
 * StatsDestroyButtons:  Destroy the stats group buttons.
 */
void StatsDestroyButtons(void)
{
   int i;

   for (i = 0; i < NUM_BUTTONS; i++)
     if (buttons[i].hwnd != NULL)
      DestroyWindow(buttons[i].hwnd);
}
/************************************************************************/
/*
 * StatsMoveButtons:  Resize and move buttons, for when buttons are created or the
 *   main window is resized.
 */
void StatsMoveButtons(void)
{
	int x, y, i;
	AREA stats_area;

	StatsGetArea(&stats_area);

	//	Buttons are now stretched to fit width of area.
	for( i = 0; i < NUM_BUTTONS; i++ )
	{
		StatButton* button = NULL;
		//	ajw Hard-coded changed order of buttons appearance.
		switch( i )
		{
		case 0:
			button = &buttons[3];		//	Inventory
			break;
		case 1:
			button = &buttons[1];		//	Spells
			break;
		case 2:
			button = &buttons[2];		//	Skills
			break;
		case 3:
			button = &buttons[0];		//	Stats
			break;
		}
		if( button->hwnd == NULL )
			continue;

//		x = ( stats_area.cx / ( NUM_BUTTONS * 2 ) ) * ( 2 * i + 1 ) - (int)( button->width / 2 );
//		y = 0;

		x = ( stats_area.cx / NUM_BUTTONS ) * i;
		y = 0;
		if( i < NUM_BUTTONS - 1 )
			button->width = ( stats_area.cx / NUM_BUTTONS );
		else
			//	Undo possible roundoff error by stretching final button.
			button->width = stats_area.cx - (int)( stats_area.cx / NUM_BUTTONS ) * ( NUM_BUTTONS - 1 );

		MoveWindow( button->hwnd, x, y, button->width, button->height, TRUE );
		ShowWindow( button->hwnd, SW_SHOWNORMAL );
		button->x = x;
		button->y = y;
	}
/*
   // Compute width of each button; all same size, but must fit in stats area
   width = -STATUS_BUTTON_SPACING;
   for (i=0; i < NUM_BUTTONS; i++)
     width += buttons[i].width + STATUS_BUTTON_SPACING;

   x = (stats_area.cx - width) / 2;
   y = 0;
   for (i=0; i < NUM_BUTTONS; i++)
   {
      StatButton *button;
	  
	//	ajw Hard-coded changed order of buttons appearance.
	switch( i )
	{
	case 0:
	     button = &buttons[3];		//	Inventory
	     break;
	case 1:
	     button = &buttons[1];		//	Spells
	     break;
	case 2:
	     button = &buttons[2];		//	Skills
	     break;
	case 3:
	     button = &buttons[0];		//	Stats
	     break;
	}

      if (button->hwnd == NULL)
	continue;
      
      MoveWindow(button->hwnd, x, y, button->width, button->height, TRUE);
      ShowWindow(button->hwnd, SW_SHOWNORMAL);
      button->x = x;
      button->y = y;

      x += button->width + STATUS_BUTTON_SPACING;
   }
*/
}
/************************************************************************/
/*
 * StatsSetButtonFocus:  Set focus to button for given group, if possible.
 */
void StatsSetButtonFocus(int group)
{
   // Skip main group
   group--;

   // Set focus to button of current group (starts counting from 0)
   if (group <= NUM_BUTTONS)
     SetFocus(buttons[group - 1].hwnd);
}
/************************************************************************/
/*
 * StatsGetButtonBorder:  Return height of stats area reserved for buttons
 */
int StatsGetButtonBorder(void)
{
   return button_border;
}

/****************************************************************************/
/*
 * StatButtonProc:  Subclassed window procedure for stat group buttons
 */
long CALLBACK StatButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	msg.hwnd = hwnd;
	msg.message = message;
	msg.wParam = wParam;
	msg.lParam = lParam;
	TooltipForwardMessage(&msg);

   switch (message)
   {
   case WM_ERASEBKGND:
     return 1;

   case WM_KEYDOWN:
      if (HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, StatInputKey) == True)
      	 return 0;
      break;

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      StatsDrawBorder();
      break;
   }
   return CallWindowProc(lpfnDefButtonProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * StatInputKey:  User pressed a key on stat group button or list box.
 *   Return True iff key should NOT be passed on to Windows for default processing.
 */
Bool StatInputKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags)
{
   Bool held_down = (flags & 0x4000) ? True : False;  /* Is key being held down? */
   int action;
   void *action_data;

   UserDidSomething();
   
   /* See if stat button handles this key */
   action = TranslateKey(key, stats_key_table, &action_data); 
   if (action == A_NOACTION)
      return False;

   switch(action)
   {
   case A_TABFWD:
      TextInputSetFocus(True);
      break;
   case A_TABBACK:
		SetFocus(cinfo->hMain);	//	ajw
		//InventorySetFocus(False);
		break;
   case A_NEXT:
      StatsMoveButtonFocus(hwnd, True);
      break;
   case A_PREV:
      StatsMoveButtonFocus(hwnd, False);
      break;

   default:
      if (!held_down)
	 PerformAction(action, action_data);
      break;
   }
   return True;
}
/************************************************************************/
/*
 * StatButtonDrawItem:  Draw stat button for given DRAWITEMSTRUCT structure.
 */
Bool StatButtonDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
	int group, xSrc;
	int xDest;
	StatButton *button;
	Bool bPressed = False;

	switch (lpdis->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		// Draw bitmap, if button has one
		group = StatsFindGroupByHwnd(lpdis->hwndItem) - 1;
		if( group == GROUP_NONE || buttons[group].bitsLeft == NULL )
		{
			FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
		}
		else
		{
			button = &buttons[group];
			// Pick image to draw, depending on whether button is pressed
			if( lpdis->itemState & ODS_SELECTED || StatsGetCurrentGroup() == group + 2 )
				bPressed = True;

			SelectPalette(lpdis->hDC, cinfo->hPal, FALSE);
//			OffscreenWindowBackground( NULL, button->x, button->y, button->width, button->height );

			//	Left piece.
			if( bPressed )
				xSrc = button->iWidthLeft;
			else xSrc = 0;
			OffscreenBitBlt(lpdis->hDC, 0, 0, button->iWidthLeft, button->height,
                         (BYTE *) button->bitsLeft, xSrc, 0, 2 * button->iWidthLeft,
                         OBB_FLIP | OBB_COPY | OBB_TRANSPARENT );
			//	Middle repeating pieces.
			if( bPressed )
				xSrc = button->iWidthMid;
			xDest = button->iWidthLeft;
			while( xDest < button->width - button->iWidthRight )
			{
				OffscreenBitBlt(lpdis->hDC, xDest, 0, button->iWidthMid, button->height,
                            (BYTE *) button->bitsMid, xSrc, 0, 2 * button->iWidthMid,
                            OBB_FLIP | OBB_COPY | OBB_TRANSPARENT );
				xDest += button->iWidthMid;
			}
			//	Right piece.
			if( bPressed )
				xSrc = button->iWidthRight;
			xDest = button->width - button->iWidthRight;
			OffscreenBitBlt(lpdis->hDC, xDest, 0, button->iWidthRight, button->height,
                         (BYTE *) button->bitsRight, xSrc, 0, 2 * button->iWidthRight,
                         OBB_FLIP | OBB_COPY | OBB_TRANSPARENT );
		}
		return True;
	}
	return False;
}
/************************************************************************/
/*
 * StatButtonCommand:  User pressed stat button
 */
void StatButtonCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	int group;
	list_type stat_list;

	group = StatsFindGroupByHwnd(hwndCtl) + 1;  // Skip main stat group
	if (group == GROUP_NONE)
		return;

	if (group != StatsGetCurrentGroup())
	{
		//	ajw Changes to make Inventory act somewhat like one of the stats groups.
		if( StatsGetCurrentGroup() == STATS_INVENTORY )
		{
			//	Inventory must be going away.
			ShowInventory( False );
//			StatsShowGroup( True );
		}
		if( group == STATS_INVENTORY )
		{
			//	The hacks continue... Force previously toggled non-inventory button to repaint and show new unpressed state.
			InvalidateRect( buttons[ StatsGetCurrentGroup()-2 ].hwnd, NULL, FALSE );

			StatsShowGroup( False );
			ShowInventory( True );
			DisplayInventoryAsStatGroup( (BYTE)STATS_INVENTORY );
		}
		else
		{
			// Check stat cache; if group not present, ask server
			if (StatCacheGetEntry(group, &stat_list) == True)
				DisplayStatGroup((BYTE) group, stat_list);
			else
			{
				debug(("Requesting group %d\n", group));
				RequestStats(group);
			}
		}
	}
	if( StatsGetCurrentGroup() == STATS_INVENTORY )
		InventorySetFocus( True );		//	Force focus to leave stats group window after button within it was pushed.
	//	[ xxx Still broken - click on Inv when it is already selected, but drag off of button and release. ]
}

/************************************************************************/
/*
 * StatsMoveButtonFocus:  Focus is currently on given button; move focus forward
 *   or backward, depending on "forward".
 */
void StatsMoveButtonFocus(HWND button, Bool forward)
{
   int num = StatsFindGroupByHwnd(button);
   int dx = forward ? +1 : -1;

   if (num == GROUP_NONE)
   {
      debug(("StatsMoveButtonFocus got bogus button window\n"));
      return;
   }
   num = (num - 1 + dx) % NUM_BUTTONS;
   if (num < 0)
      num += NUM_BUTTONS;
   
   if (num < NUM_BUTTONS)
      SetFocus(buttons[num].hwnd);
}
/************************************************************************/
/*
 * StatsFindGroupByHwnd:  Return group # of stat with given button, or GROUP_NONE if none.
 */
int StatsFindGroupByHwnd(HWND hwnd)
{
   int i, num = STATS_MAIN;

   for (i = 0; i < NUM_BUTTONS; i++)
      if (buttons[i].hwnd == hwnd)
	 return i + 1;

   return GROUP_NONE;
}
