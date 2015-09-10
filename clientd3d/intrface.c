// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * intrface.c:  Deal with interface objects other than main board area.
 *
 * Functions in this file mostly dispatch events to all of the interface components.
 * The PerformAction function dispatches user actions.
 */

#include "client.h"

#define CURSOR_JUMP 10         /* # of pixels to move cursor per user keystroke */

/* Tabbing order of controls on main window */
static int tab_order[] = {
IDC_MAIN, 0, IDC_TEXTINPUT,
};
static int NUMTABS = (sizeof(tab_order) / sizeof(int));
    
static Bool interface_created = False;  /* True iff interface has been created */

extern font_3d				gFont;
extern d3d_driver_profile	gD3DDriverProfile;

/************************************************************************/
void InterfaceInitialize(HWND hParent)
{
   RECT r;

   /* Create all child windows--order is important to get positions & sizes right! */
   GraphicsAreaCreate(hParent);
   TextInputCreate(hParent);
   EditBoxCreate(hParent);
   ToolbarCreate();
   TooltipCreate();
   Lagbox_Create();

   MapAnnotationsInitialize();
   
   interface_created = True;

   /* Send resize message so that subwindows can size themselves */
   GetClientRect(hMain,&r);
   InterfaceResize(r.right,r.bottom);

   SetFocus(hMain);  /* Put focus on grid area */
}
/************************************************************************/
void InterfaceClose(void)
{           
   if (!interface_created)
      return;

   /* Close offer dialogs, if they're up */
   OfferClose();

   /* Close modal dialogs */
   AbortLookList();
   AbortGameDialogs();
   AbortPasswordDialog();
   AbortPreferencesDialog();
   AbortAboutDialog();

   MusicStop();
   SoundStopAll();

   GraphicsAreaDestroy();
   TextInputDestroy();
   EditBoxDestroy();
   ToolbarDestroy();
   TooltipDestroy();
   Lagbox_Destroy();
   
   //InvalidateRect(hMain, NULL, TRUE);
   interface_created = False;
}
/************************************************************************/
/*
 * InterfaceRedraw: Called when interface needs to be redrawn.
 *   hdc is DC from PAINTSTRUCT in expose event message.
 */
void InterfaceRedraw(HDC hdc)
{
   if (!interface_created)
      return;

   /* Draw border around appropriate area */
   EditBoxDrawBorder();
   DrawGridBorder();

   GraphicsAreaRedraw(hdc);
   
   // Allow modules to redraw
   ModuleEvent(EVENT_REDRAW, hdc);
}
/************************************************************************/
/* 
 * InterfaceResize:  Called when main window is resized, to xsize by ysize.
 *   Resize all subwindows.
 */
void InterfaceResize(int xsize, int ysize)
{
   AREA view;

   debug(("InterfaceResize to %i, %i.\n", xsize, ysize ));

   GraphicsAreaResize(xsize, ysize);

   CopyCurrentView(&view);

   TextInputResize(xsize, ysize, view);
   EditBoxResize(xsize, ysize, view);
   ToolbarResize(xsize, ysize, view);
   Lagbox_Reposition();

   RedrawAll();

   // Allow modules to resize their components
   ModuleEvent(EVENT_RESIZE, xsize, ysize, &view);
   InvalidateRect(hMain,NULL,TRUE);
}
/************************************************************************/
/* 
 * InterfaceGetMaxSize:  Fill s with maximum allowed main window size.
 */
void InterfaceGetMaxSize(SIZE *s)
{
   int factor = config.large_area ? 2 : 1;

   s->cx = MAXX * factor + INVENTORY_MAX_WIDTH + LEFT_BORDER * 3 
      + 2 * GetSystemMetrics(SM_CXFRAME);
   s->cy = GetSystemMetrics(SM_CYSCREEN) + 2 * GetSystemMetrics(SM_CYFRAME);
}


/************************************************************************/
/* 
 * GameChangeFont:  Redraw interface after user has changed a font.
 */
void GameChangeFont(void)
{
   RECT r;

   /* If interface hasn't been created, do nothing */
   if (!interface_created)
      return;
   
   /* Just redraw everything; we don't know what is using default font,
    * for example.
    */
   InvalidateRect(hMain, NULL, TRUE);
   TextInputResetFont();
   EditBoxResetFont();
   OfferChangeFonts();
   GraphicsResetFont();

   /* Send a resize so that controls resize themselves */
   GetClientRect(hMain,&r);
   InterfaceResize(r.right,r.bottom);
   RedrawAll();
}
/************************************************************************/
/* 
 * GameChangeColor:  Redraw interface after user has changed a color.
 */
void GameChangeColor(void)
{
   RECT r;

   /* If interface hasn't been created, do nothing */
   if (!interface_created)
      return;

   /* Just redraw everything */
   InvalidateRect(hMain, NULL, TRUE);
   
   EditBoxChangeColor();
   DrawChangeColor();
   OfferChangeColor();

   // Force a total redraw
   GetClientRect(hMain,&r);
   InterfaceResize(r.right,r.bottom);
}
/************************************************************************/
/*
 * MainTab:  Switch focus when user presses TAB on main window.
 *   ctrl is the id of the control which currently has the focus.
 *   Forward should be True if TAB pressed, False if Shift+TAB pressed.
 */
void MainTab(int ctrl, Bool forward)
{
   int i, next_index = -1, direction;

   /* From edit box, go to main window */
   if (ctrl == IDC_MAINTEXT)
   {
      SetFocus(hMain);
      return;
   }

   /* If Shift+TAB pressed, go backwards; ( NUMTABS - 1 = -1 (mod NUMTABS) ) */
   direction = forward ? 1 : (NUMTABS - 1);
   
   for (i=0; i < NUMTABS; i++)
      if (ctrl == tab_order[i])
	 next_index = (i + direction) % NUMTABS;
       
   /* If illegal control passed in, ignore */
   if (next_index == -1)
      return;
   
   switch (tab_order[next_index])
   {
   case IDC_MAIN:
      SetFocus(hMain);
      DrawGridBorder();
      break;

   case IDC_MAINTEXT:
      EditBoxSetFocus(forward);
      break;

   case IDC_TEXTINPUT:
      TextInputSetFocus(forward);
      break;
   }
}
/************************************************************************/
/*
 * PerformAction:  Dispatch on game action.
 */
void PerformAction(int action, void *action_data)
{
   // See if a module wants to handle this action
   if (ModuleEvent(EVENT_USERACTION, action, action_data) == False)
      return;

   if (1)
   {
		switch (action)
		{
			case A_FORWARD:
			case A_BACKWARD:
			case A_SLIDELEFT:
			case A_SLIDERIGHT:
			case A_FORWARDFAST:
			case A_BACKWARDFAST:
			case A_SLIDELEFTFAST:
			case A_SLIDERIGHTFAST:
			case A_SLIDELEFTFORWARD:
			case A_SLIDELEFTBACKWARD:
			case A_SLIDELEFTFORWARDFAST:
			case A_SLIDELEFTBACKWARDFAST:
			case A_SLIDERIGHTFORWARD:
			case A_SLIDERIGHTBACKWARD:
			case A_SLIDERIGHTFORWARDFAST:
			case A_SLIDERIGHTBACKWARDFAST:
				UserMovePlayer(action);
			break;

			case A_TURNLEFT:
			case A_TURNFASTLEFT:
			case A_TURNRIGHT:
			case A_TURNFASTRIGHT:
				UserTurnPlayer(action);
			break;

			case A_FORWARDTURNLEFT:
			case A_FORWARDTURNFASTLEFT:
				UserMovePlayer(A_FORWARDFAST);
				UserMovePlayer(A_SLIDELEFT);
			break;

			case A_FORWARDTURNRIGHT:
			case A_FORWARDTURNFASTRIGHT:
				UserMovePlayer(A_FORWARDFAST);
				UserMovePlayer(A_SLIDERIGHT);
			break;

			case A_BACKWARDTURNLEFT:
			case A_BACKWARDTURNFASTLEFT:
				UserMovePlayer(A_BACKWARDFAST);
				UserMovePlayer(A_SLIDELEFT);
			break;

			case A_BACKWARDTURNRIGHT:
			case A_BACKWARDTURNFASTRIGHT:
				UserMovePlayer(A_BACKWARDFAST);
				UserMovePlayer(A_SLIDERIGHT);
			break;
		}
   }

   switch (action)
   {
   case A_TABFWD:
      MainTab((int) action_data, True);
      break;
   case A_TABBACK:
      MainTab((int) action_data, False);
      break;

/*   case A_FORWARD:
   case A_BACKWARD:
   case A_SLIDELEFT:
   case A_SLIDERIGHT:
   case A_FORWARDFAST:
   case A_BACKWARDFAST:
      UserMovePlayer(action);
      break;

   case A_TURNLEFT:
   case A_TURNRIGHT:
   case A_TURNFASTLEFT:
   case A_TURNFASTRIGHT:
      UserTurnPlayer(action);
      break;

   case A_FORWARDTURNLEFT:
      UserMovePlayer(A_FORWARD);
      UserTurnPlayer(A_TURNLEFT);
      break;
   case A_FORWARDTURNRIGHT:
      UserMovePlayer(A_FORWARD);
      UserTurnPlayer(A_TURNRIGHT);
      break;
   case A_BACKWARDTURNLEFT:
      UserMovePlayer(A_BACKWARD);
      UserTurnPlayer(A_TURNLEFT);
      break;
   case A_BACKWARDTURNRIGHT:
      UserMovePlayer(A_BACKWARD);
      UserTurnPlayer(A_TURNRIGHT);
      break;
   case A_FORWARDTURNFASTLEFT:
      UserMovePlayer(A_FORWARD);
      UserTurnPlayer(A_TURNFASTLEFT);
      break;
   case A_FORWARDTURNFASTRIGHT:
      UserMovePlayer(A_FORWARD);
      UserTurnPlayer(A_TURNFASTRIGHT);
      break;
   case A_BACKWARDTURNFASTLEFT:
      UserMovePlayer(A_BACKWARD);
      UserTurnPlayer(A_TURNFASTLEFT);
      break;
   case A_BACKWARDTURNFASTRIGHT:
      UserMovePlayer(A_BACKWARD);
      UserTurnPlayer(A_TURNFASTRIGHT);
      break;*/

   case A_ATTACK:
      if ((GetPlayerInfo()->viewID == 0) || (GetPlayerInfo()->viewID == GetPlayerInfo()->id))
	 UserAttack(action);
	  // need to force userarea to redraw whenever target changes
	  ModuleEvent(EVENT_USERACTION, action, action_data);
      break;
   case A_ATTACKCLOSEST:
      if ((GetPlayerInfo()->viewID == 0) || (GetPlayerInfo()->viewID == GetPlayerInfo()->id))
         UserAttackClosest(action);
      break;

   case A_ENDSELECT:  /* Exit GAME_SELECT state, go back to GAME_PLAY state */
   case A_CURSOR_ESC:
	   if (UserMouselookIsEnabled())
	   {
			while (ShowCursor(FALSE) >= 0)
				ShowCursor(FALSE);
	   }
      if (GAME_PLAY == GameGetState())
	 UserMoveEsc();
      else
	 GameSetState(GAME_PLAY);
      break;

   case A_CURSORUP : 
      UserMoveCursor(A_CURSORUP, -CURSOR_JUMP, 0);
      break;
   case A_CURSORDOWN :
      UserMoveCursor(A_CURSORDOWN, CURSOR_JUMP, 0);
      break;
   case A_CURSORLEFT :
      UserMoveCursor(A_CURSORLEFT, 0, -CURSOR_JUMP);
      break;
   case A_CURSORRIGHT :
      UserMoveCursor(A_CURSORRIGHT, 0, CURSOR_JUMP);
      break;
   case A_CURSORUPLEFT : 
      UserMoveCursor(A_CURSORUPLEFT, -CURSOR_JUMP, -CURSOR_JUMP);
      break;
   case A_CURSORUPRIGHT :
      UserMoveCursor(A_CURSORUPRIGHT, -CURSOR_JUMP, CURSOR_JUMP);
      break;
   case A_CURSORDOWNLEFT :
      UserMoveCursor(A_CURSORDOWNLEFT, CURSOR_JUMP, -CURSOR_JUMP);
      break;
   case A_CURSORDOWNRIGHT :
      UserMoveCursor(A_CURSORDOWNRIGHT, CURSOR_JUMP, CURSOR_JUMP);
      break;

   case A_LOOK:
      UserLook();
      break;

   case A_LOOKINSIDE:
      UserLookInside();
      break;

   case A_LOOKMOUSE:
      UserLookMouseSquare();
      break;

   case A_PICKUP:
      UserPickup();
      break;

   case A_DROP:
      UserDrop();
      break;

   case A_PUT:
      UserPut();
      break;

   case A_APPLY:
      UserApply();
      break;

   case A_UNUSE:
      UserUnuse();
      break;

   case A_OFFER:
      UserMakeOffer();
      break;

   case A_GO:
      MoveUpdatePosition();   // Send our exact position, so that we try to go in right spot
      RequestGo();
      break;
      
   case A_TARGETCLEAR:
      if ((GetPlayerInfo()->viewID == 0) || (GetPlayerInfo()->viewID == GetPlayerInfo()->id))
	 SetUserTargetID( INVALID_ID );
      else
	 UserMoveEsc();
	  // need to force userarea to redraw whenever target changes
	  ModuleEvent(EVENT_USERACTION, action, action_data);
      // Fall through...
   case A_GOTOMAIN:  /* User pressed Escape; move focus to main menu */
      SetFocus(hMain);
      break;

   case A_GOTOSAY:   /* User pressed quote; move focus to text input box */
      TextInputSetFocus(True);  // Pretend that moving forward in window list; no harm here
      break;

   case A_SELECT:
	   if (UserMouselookIsEnabled())
	   {
			while (ShowCursor(FALSE) >= 0)
				ShowCursor(FALSE);
	   }
      UserSelect((ID) action_data);
      break;

   case A_BUY:
      UserBuy();
      break;

   case A_USE:
      UserUse();
      break;

   case A_STARTDRAG:
      UserStartDrag();
      break;

   case A_USERACTION:
      RequestAction((int) action_data);
      break;

   case A_MOUSEMOVE:
      UserMouseMove();
      break;

   case A_MAP:
     GraphicsToggleMap();

	   if (gD3DDriverProfile.bSoftwareRenderer == FALSE)
			D3DRenderEnableToggle();
      break;

   case A_LOOKUP:
      PlayerChangeHeight(+1);
      break;

   case A_LOOKDOWN:
      PlayerChangeHeight(-1);
      break;

   case A_LOOKSTRAIGHT:
      PlayerResetHeight();
      break;

   case A_QUIT:
      UserQuit();
      break;

   case A_HELP:
      StartHelp();
      break;

   case A_MAPZOOMIN:
      MapZoom(+1);
      break;

   case A_MAPZOOMOUT:
      MapZoom(-1);
      break;

   case A_TEXTINSERT:
      TextInputSetText((char *) action_data, True);
      break;

   case A_TEXTCOMMAND:
      TextInputSetText((char *) action_data, False);
      ParseGotText((char *) action_data);
      break;

   case A_WHO:
      UserWho();
      break;

   case A_FLIP:
      UserFlipPlayer();
      break;

   case A_ACTIVATEMOUSE:
      UserActivateMouse();
      break;

   case A_ACTIVATE:
      UserActivate();
      break;

   case A_CHANGEPASSWORD:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_PASSWORD), hMain, PasswordDialogProc);
      break;

   case A_TARGETPREVIOUS:
      UserTargetNextOrPrevious( False );
	  // need to force userarea to redraw whenever target changes
	  ModuleEvent(EVENT_USERACTION, action, action_data);
      break;

   case A_TARGETNEXT:
      UserTargetNextOrPrevious( True );
	  // need to force userarea to redraw whenever target changes
	  ModuleEvent(EVENT_USERACTION, action, action_data);
      break;

	case A_TARGETSELF:
		UserTargetSelf();
		// need to force userarea to redraw whenever target changes
	  ModuleEvent(EVENT_USERACTION, action, action_data);
	break;

	case A_MOUSELOOK:
		UserMouselookToggle();
		break;

#if 1
   case A_DEPOSIT:
      UserDeposit();
      break;

   case A_WITHDRAW:
      UserWithdraw();
      break;
#endif
   }
}

