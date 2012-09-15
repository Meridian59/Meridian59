// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * toolbar.c:  Deal with button bar, which is used to provide mouse shortcuts
 *   to common game commands.
 *
 * The bitmap for a toolbar button should contain two images side by side.
 * The bitmap on the left is used when the button is not pressed; the bitmap on the
 * right is used when the button is pressed.  
 *
 * Each button may optionally display a string in a tooltip.  
 *
 * Modules can add buttons to the toolbar by calling ToolbarAddButton.
 * 
 * A button can be a two state toggle by specifying an action to perform when the button
 * is released.
 */

#include "client.h"

#define MAX_BUTTONS 30

static Button buttons[MAX_BUTTONS];
static int    num_buttons;           // Number of buttons currently created
static int    num_separators;        // Number of separators currently created

static WNDPROC lpfnDefButtonProc; // Default toolbar window procedure

extern HPALETTE hPal;

/* local function prototypes */
static void ToolbarSetButtonVisibility(HWND hButton);
static void ToolbarButtonPressed(Button *b);
static Button *ToolbarFindButtonByHandle(HWND hwnd);
static Button *ToolbarFindButtonByAction(int action, void *action_data);
static long CALLBACK ToolbarButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
/****************************************************************************/
/*
 * ToolbarCreate:  Create toolbar.
 */
void ToolbarCreate(void)
{
   num_buttons = 0;
   num_separators = 0;
}
/****************************************************************************/
/*
 * ToolbarDestroy:  Destroy toolbar.
 */
void ToolbarDestroy(void)
{
   int i;

   for (i = 0; i < num_buttons; i++)
      DestroyWindow(buttons[i].hwnd);
}

/****************************************************************************/
/*
 * ToolbarAddButton:  Add a button to the toolbar.
 *   s describes the properties of the button.
 *   Return True iff button created successfully.
 */
Bool __cdecl ToolbarAddButton(AddButton *s)
{
   HWND hwnd;
   int x, y;
   BITMAPINFOHEADER *ptr;
   BYTE *bits;
   
   if (num_buttons >= MAX_BUTTONS)
   {
      debug(("ToolbarAddButton: Too many buttons!\n"));
      return False;
   }

   x = TOOLBAR_X + num_buttons * (TOOLBAR_BUTTON_LEFT_SPACING + TOOLBAR_BUTTON_WIDTH);	//	+ num_separators * TOOLBAR_SEPARATOR_WIDTH;
   y = TOOLBAR_Y + TOOLBAR_BUTTON_TOP_SPACING;

   // If id = 0, create separator
   if (s->bitmap_id == 0)
   {
      num_separators++;
      return True;
   }

   hwnd = CreateWindow("button", "", WS_CHILD | BS_OWNERDRAW, 
		       x, y, TOOLBAR_BUTTON_WIDTH, TOOLBAR_BUTTON_HEIGHT,
		       hMain, (HMENU) IDC_TOOLBUTTON, hInst, NULL);

   if (hwnd == NULL)
   {
      debug(("ToolbarAddButton failed to create button window\n"));
      return False;
   }

   ptr = GetBitmapResource(s->hModule, s->bitmap_id);
   if (ptr == NULL)
      return False;

   bits = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);

   if (ptr->biWidth != 2 * TOOLBAR_BUTTON_WIDTH || ptr->biHeight != TOOLBAR_BUTTON_HEIGHT)
   {
      debug(("ToolbarAddButton found button bitmap %d with bad size\n", s->bitmap_id));
      return False;
   }

   lpfnDefButtonProc = SubclassWindow(hwnd, ToolbarButtonProc);

   // Set up tooltip for button, if desired
   if (s->name != 0)
      TooltipAddWindow(hwnd, s->hModule, s->name);

   ToolbarSetButtonVisibility(hwnd);
   
   buttons[num_buttons].index        = num_buttons;
   buttons[num_buttons].hwnd         = hwnd;
   buttons[num_buttons].action       = s->action;
   buttons[num_buttons].action_data  = s->data;
   buttons[num_buttons].action2      = s->action2;
   buttons[num_buttons].action_data2 = s->data2;
   buttons[num_buttons].bits         = bits;
   buttons[num_buttons].x            = x;
   buttons[num_buttons].pressed      = False;
   num_buttons++;

   Lagbox_Reposition();

   return True;
}
/****************************************************************************/
/*
 * ToolbarResize:  Hide buttons that extend past the end of the toolbar area.
 */
void ToolbarResize(int xsize, int ysize, AREA view)
{
   int i;

   for (i=0; i < num_buttons; i++)
      ToolbarSetButtonVisibility(buttons[i].hwnd);
}
/****************************************************************************/
void ToolbarGetUnionRect(RECT* prcRect)
{
	int i;
	if (!prcRect)
		return;

	prcRect->left = 0;
	prcRect->top = 0;
	prcRect->right = 0;
	prcRect->bottom = 0;
	for (i = 0; i < num_buttons; i++)
	{
		RECT rcButton;
		GetWindowRect(buttons[i].hwnd, &rcButton);
		ScreenToClient(hMain, (POINT*)&rcButton);
		ScreenToClient(hMain, (POINT*)(&rcButton)+1);
		if (IsWindowVisible(buttons[i].hwnd))
			UnionRect(prcRect, prcRect, &rcButton);
	}
}
/****************************************************************************/
/*
 * ToolbarSetButtonVisibility:  Show or hide given toolbar button window, 
 *   depending on its position and the configuration setting for the toolbar.
 */
void ToolbarSetButtonVisibility(HWND hButton)
{
   RECT r;
   POINT p;
   AREA view;

   if (!config.toolbar)
   {
      ShowWindow(hButton, SW_HIDE);
      return;
   }

   GetWindowRect(hButton, &r);
   CopyCurrentView(&view);
   p.x = r.left;
   p.y = r.top;
   ScreenToClient(hMain, &p);
   if (p.x + r.right - r.left > view.x + view.cx * 3 / 4)
      ShowWindow(hButton, SW_HIDE);
   else ShowWindow(hButton, SW_NORMAL);
}
/****************************************************************************/
/*
 * ToolbarCommand:  A WM_COMMAND message has been received for one of the 
 *   toolbar buttons.
 */
void ToolbarCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   Button *b;

   switch (codeNotify)
   {
   case BN_CLICKED:
      SetFocus(hMain);      // Don't leave keyboard focus on button

      // Find button structure based on window handle
      b = ToolbarFindButtonByHandle(hwndCtl);
      if (b != NULL)
	 ToolbarButtonPressed(b);

      break;
   }
}
/****************************************************************************/
/*
 * ToolbarButtonPressed:  Toolbar button with given Button structure was pressed.
 */
void ToolbarButtonPressed(Button *b)
{
   int action;
   void *data;

   /* See if module wants to handle this event */
   if (ModuleEvent(EVENT_TOOLBUTTON, b) == False)
      return;

   action = b->action;
   data = b->action_data;

   // If button is a toggle, set state to toggled if pressing in; perform up action if pushing out
   if (b->action2 != A_NOACTION)
   {
     //     b->pressed = !b->pressed;
     if (b->pressed)
       Button_SetState(b->hwnd, TRUE);
     else 
       {
	 action = b->action2;
	 data   = b->action_data2;
       }
   }

   PerformAction(action, data);
}
/****************************************************************************/
/*
 * ToolbarSetButtonState:  Given an action, find the corresponding toolbar button
 *   and set its state (True = pressed in, False = out).  This is only useful
 *   for toggle buttons.
 *   Returns True iff button state set.
 */
Bool ToolbarSetButtonState(int action, void *action_data, Bool state)
{
  Button *b = ToolbarFindButtonByAction(action, action_data);
  
  if (b == NULL)
    return False;

  Button_SetState(b->hwnd, state);
  // Redraw if necessary
  if (state != b->pressed)
    InvalidateRect(b->hwnd, NULL, TRUE);
  
  b->pressed = state;

	return TRUE;
}

/****************************************************************************/
/*
 * ToolbarFindButtonByHandle:  Return Button structure for toolbar button
 *   with given handle, or NULL if none found.
 */
Button *ToolbarFindButtonByHandle(HWND hwnd)
{
   int i;
   
   for (i=0; i < num_buttons; i++)
      if (buttons[i].hwnd == hwnd)
	 return &buttons[i];
   return NULL;
}
/****************************************************************************/
/*
 * ToolbarFindButtonByAction:  Return Button structure for toolbar button
 *   with given action, or NULL if none found.
 */
Button *ToolbarFindButtonByAction(int action, void *action_data)
{
   int i;
   
   for (i=0; i < num_buttons; i++)
      if (buttons[i].action == action && buttons[i].action_data == action_data)
	 return &buttons[i];
   return NULL;
}
/****************************************************************************/
/*
 * ToolbarDrawButton: Handle WM_DRAWITEM messages for toolbar buttons.
 *   Return True iff message handled.
 */
Bool ToolbarDrawButton(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   Button *b;
   int x;

   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      // Draw bitmap, if button has one
      b = ToolbarFindButtonByHandle(lpdis->hwndItem);
      if (b == NULL || b->bits == NULL)
      {
	 FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
      }
      else
      {
	 // Pick image to draw, depending on whether button is pressed
	 if (lpdis->itemState & ODS_SELECTED || b->pressed)
	    x = TOOLBAR_BUTTON_WIDTH;
	 else x = 0;

	 SelectPalette(lpdis->hDC, hPal, FALSE);

	 OffscreenWindowBackground(NULL, b->x, TOOLBAR_Y, TOOLBAR_BUTTON_WIDTH, TOOLBAR_BUTTON_HEIGHT);

	 OffscreenBitBlt(lpdis->hDC, 0, 0, TOOLBAR_BUTTON_WIDTH, TOOLBAR_BUTTON_HEIGHT,
			 b->bits, x, 0, 2 * TOOLBAR_BUTTON_WIDTH, 
			 OBB_FLIP | OBB_COPY | OBB_TRANSPARENT);
      }
      return True;
   }
   return False;
}
/****************************************************************************/
/*
 * ToolbarButtonProc:  Subclassed window procedure for toolbar buttons
 */
long CALLBACK ToolbarButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   MSG msg;
   
   msg.hwnd = hwnd;
   msg.message = message;
   msg.wParam = wParam;
   msg.lParam = lParam;

   TooltipForwardMessage(&msg);

   switch (message)
   {
   case WM_LBUTTONUP:
      // If user moves pointer off button and then releases, focus is left on button.
      // So move focus back to main window if button not highlighted.
      if (!(Button_GetState(hwnd) & 0x0004))
	 SetFocus(hMain);
      else
      {
	// If releasing a toggle button, don't pop button out.
	Button *b = ToolbarFindButtonByHandle(hwnd);
	if (b != NULL && b->action2 != A_NOACTION)
	    b->pressed = !b->pressed;
      }
      break;

   case WM_LBUTTONDBLCLK:
     message = WM_LBUTTONDOWN;
     break;
   }
   return CallWindowProc(lpfnDefButtonProc, hwnd, message, wParam, lParam);
}
