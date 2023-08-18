// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * gameuser.c:  Handle user's game commands
 */

#include "client.h"

#define ATTACK_DELAY 1000  // Minimum number of milliseconds between user attacks

extern player_info player;
extern room_type current_room;

static SendOfferDialogStruct info; /* Stuff to send to offer dialog */

static ID object1;  /* object to be applied; see UserApply */
static DWORD last_attack_time;   // time of last attack action

static ID idTarget = INVALID_ID;		//	Target object, or INVALID_ID if no target set.

static void ApplyCallback(ID obj2);
static void SetDescParamsByRoomObject(room_contents_node *r, HWND hwnd);

extern BOOL		gbMouselook;
extern RECT		gD3DRect;
/************************************************************************/
/*
 * UserInventoryList:  Bring up a dialog for user to select multiple objects
 *   from his inventory.  hwnd is parent for dialog.
 */
list_type UserInventoryList(HWND hwnd, char *title)
{
   return DisplayLookList(hwnd, title, player.inventory, LD_MULTIPLESEL | LD_AMOUNTS);
}
/************************************************************************/
void UserAttack(int action)
{
   room_contents_node *target_obj;
   list_type object_list, sel_list;
   int x, y; 
   DWORD now;

   /* Find out where in the the room the user clicked on, if anywhere */
   if (!MouseToRoom(&x, &y))
      return;

   // Make sure that user doesn't attack too often
   now = timeGetTime();
   if (now - last_attack_time < ATTACK_DELAY)
      return;
   last_attack_time = now;

   /* Get objects at mouse position */
   object_list = GetObjects3D(x, y, 0, OF_ATTACKABLE, 0);
   if (object_list == NULL)
      return;

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_ATTACK), object_list, LD_SINGLEAUTO);
   
   if (sel_list != NULL)
   {
      MoveUpdatePosition();   // Send our exact position, so that we try to attack in range

      target_obj = (room_contents_node *) (sel_list->data);

	  if (config.bAttackOnTarget)
			RequestAttack(ATTACK_NORMAL, target_obj->obj.id);

		//	Set specifically attacked target as the user's "selected target".
		SetUserTargetID(target_obj->obj.id);
   }
   ObjectListDestroy(sel_list);
   list_delete(object_list);
}
/************************************************************************/
void UserAttackClosest(int action)
{
   room_contents_node *target_obj;
   list_type object_list;
   DWORD now;

   // Make sure that user doesn't attack too often
   now = timeGetTime();
   if (now - last_attack_time < ATTACK_DELAY)
      return;
   last_attack_time = now;

	//	If user has selected target, and target is seen, use it as the target of this attack.
	if (idTarget != INVALID_ID)
	{
		if (FindVisibleObjectById(idTarget))
			RequestAttack(ATTACK_NORMAL, idTarget);
		else
         if (idTarget == player.id)
            GameMessage(GetString(hInst, IDS_CANTATTACKSELF));
         else
			   GameMessage(GetString(hInst, IDS_TARGETNOTVISIBLEFORATTACK));
		return;
	}
	
   object_list = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK, 
			      CLOSE_DISTANCE, OF_ATTACKABLE, 0);
   if (object_list == NULL)
      return;

   target_obj = (room_contents_node *) (object_list->data);
   RequestAttack(ATTACK_NORMAL, target_obj->obj.id);
   list_delete(object_list);
}
/************************************************************************/
void UserPickup(void)
{
   list_type square_list, sel_list, l;

   if ((square_list = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
				   CLOSE_DISTANCE, OF_GETTABLE, 0)) == NULL)
       return;

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_GET), 
			      square_list, LD_MULTIPLESEL | LD_SINGLEAUTO);
   
   for (l = sel_list; l != NULL; l = l->next)
      RequestPickup(((room_contents_node *) (l->data))->obj.id);

   ObjectListDestroy(sel_list);
   list_delete(square_list);
}
/************************************************************************/
/*
 * GotObjectContents:  Display contents of object, and let user pick up objects.
 */
void GotObjectContents(ID object_id, list_type contents)
{
   list_type sel_list, l;
   room_contents_node *r;

   r = GetRoomObjectById(object_id);

   if (contents == NULL)
   {
      if (r != NULL)
	 GameMessagePrintf(GetString(hInst, IDS_EMPTY), LookupNameRsc(r->obj.name_res));
      return;
   }

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_GET), contents, LD_MULTIPLESEL);   

   for (l = sel_list; l != NULL; l = l->next)
      RequestPickup(((room_contents_node *) (l->data))->obj.id);

   ObjectListDestroy(sel_list);
}
/************************************************************************/
/*
 * UserLook:  Called when the user asks to look at an object; pick from
 *   among visible objects.
 */
void UserLook(void)
{
   list_type square_list, sel_list;
   room_contents_node *r;

   if ((square_list = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
				   0, 0, OF_NOEXAMINE | OF_INVISIBLE)) == NULL)
      return;
   
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_LOOK), square_list, LD_SINGLEAUTO);
   
   /* You may only look at one object at a time, so list has just 1 element */
   if (sel_list != NULL)
   {
      r = GetRoomObjectById(((object_node *) (sel_list->data))->id);
      if (r != NULL)
      {
	 RequestLook(r->obj.id);	
	 SetDescParamsByRoomObject(r, hMain);
      }
   }

   ObjectListDestroy(sel_list);
   list_delete(square_list);
}
/************************************************************************/
/*
 * UserLookInside:  User wants to look inside object under mouse cursor.
 */
void UserLookInside(void)
{
   int x, y;
   room_contents_node *r;

   /* Find out where in the the room the user clicked on, if anywhere */
   if (!MouseToRoom(&x, &y))
      return;

   r = GetObjectByPosition(x, y, CLOSE_DISTANCE, OF_CONTAINER, 0);
   if (r == NULL)
      return;
   RequestObjectContents(r->obj.id);
}
/************************************************************************/
/*
 * UserLookMouseSquare:  User wants to look at objects under mouse cursor.
 */
void UserLookMouseSquare(void)
{
   int x, y;
   list_type objects, sel_list;
   room_contents_node *r;

	/* Find out where in the the room the user clicked on, if anywhere */
	if (!MouseToRoom(&x, &y))
	{
		if (MouseToMiniMap(&x, &y))
			MapAnnotationClick(x, y);
		return;
	}
//   if (MapVisible())
//   {
//      MapAnnotationClick(x, y);
//      return;
//   }

   objects = GetObjects3D(x, y, 0, 0, OF_INVISIBLE);
   if (objects == NULL)
      return;

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_LOOK), objects, LD_SINGLEAUTO);
   
   /* You may only look at one object at a time, so list has just 1 element */
   if (sel_list != NULL)
   {
      r = GetRoomObjectById(((object_node *) (sel_list->data))->id);
      if (r != NULL)
      {
	 RequestLook(r->obj.id);
	 SetDescParamsByRoomObject(r, hMain);
      }
      else
      {
	 RequestLook(((object_node*)sel_list->data)->id);
	 SetDescParams(hMain, DESC_NONE);
      }
   }

   ObjectListDestroy(sel_list);
   list_delete(objects);   
}
/************************************************************************/
/*
 * UserActivate:  Called when the user asks to activate an object; pick from
 *   among visible objects.
 */
void UserActivate(void)
{
   list_type square_list, sel_list;
   object_node *obj;

   if ((square_list = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
				   CLOSE_DISTANCE, OF_ACTIVATABLE | OF_CONTAINER, OF_PLAYER)) == NULL)
      return;
   
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_ACTIVATE), square_list, LD_SINGLEAUTO);
   
   /* You may only look at one object at a time, so list has just 1 element */
   if (sel_list != NULL)
   {
      obj =  (object_node *) (sel_list->data);
      if (obj->flags & OF_CONTAINER)
	 RequestObjectContents(obj->id);
      else RequestActivate(obj->id);
   }

   ObjectListDestroy(sel_list);
   list_delete(square_list);
}
/************************************************************************/
/*
 * UserActivateMouse:  Activate object under mouse cursor.
 */
void UserActivateMouse(void)
{
   int x, y;
   list_type objects, sel_list;
   object_node *obj;

   /* Find out where in the the room the user clicked on, if anywhere */
   if (!MouseToRoom(&x, &y))
      return;

   objects = GetObjects3D(x, y, CLOSE_DISTANCE, OF_ACTIVATABLE | OF_CONTAINER, OF_PLAYER);
   if (objects == NULL)
      return;

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_ACTIVATE), objects, LD_SINGLEAUTO);
   
   /* You may only look at one object at a time, so list has just 1 element */
   if (sel_list != NULL)
   {
      obj =  (object_node *) (sel_list->data);
      if (obj->flags & OF_CONTAINER)
	 RequestObjectContents(obj->id);
      else RequestActivate(obj->id);
   }

   ObjectListDestroy(sel_list);
   list_delete(objects);   
}
/************************************************************************/
/*
 * UserDrop: Called when user wants to drop an item.
 */
void UserDrop(void)
{
   list_type sel_list, l;
   
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_DROP), 
			      player.inventory, LD_MULTIPLESEL | LD_AMOUNTS | LD_SINGLEAUTO);

   for (l = sel_list; l != NULL; l = l->next)
      RequestDrop((object_node *) (l->data));

   ObjectListDestroy(sel_list);
}
/************************************************************************/
/*
 * UserPut: Called when user wants to put an object inside another.
 */
void UserPut(void)
{
   list_type square_list, sel_list, l, container_list;
   ID container_id;

   /* If there are no containers around, don't ask for object to put */
   square_list = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
			      CLOSE_DISTANCE, OF_CONTAINER, 0);
   if (square_list == NULL)
      return;
   
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_PUT1), 
			      player.inventory, LD_MULTIPLESEL | LD_AMOUNTS | LD_SINGLEAUTO);
   if (sel_list == NULL)
      return;

   container_list = DisplayLookList(hMain, GetString(hInst, IDS_PUT2), square_list, LD_SINGLEAUTO);
   list_delete(square_list);

   if (container_list == NULL)
   {
      ObjectListDestroy(sel_list);
      return;
   }

   container_id = ((room_contents_node *) (container_list->data))->obj.id; 
   for (l = sel_list; l != NULL; l = l->next)
      RequestPut((object_node *) (l->data), container_id);
   
   ObjectListDestroy(sel_list);
   ObjectListDestroy(container_list);
}
/************************************************************************/
/*
 * UserUse: Called when user wants to use an object.
 */
void UserUse(void)
{
   list_type sel_list;
   
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_USE), player.inventory, LD_SINGLEAUTO);

   /* You may only use at one object at a time, so list has just 1 element */
   if (sel_list != NULL)
      RequestUse(((object_node *) (sel_list->data))->id);

   ObjectListDestroy(sel_list);
}
/************************************************************************/
/*
 * UserUnuse: Called when user wants to stop using an object.
 */
void UserUnuse(void)
{
   list_type sel_list, use_list;
   
   use_list = UseListGetObjects(player.inventory);

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_UNUSE), use_list, LD_SINGLEAUTO);

   /* You may only unuse at one object at a time, so list has just 1 element */
   if (sel_list != NULL)
      RequestUnuse(((object_node *) (sel_list->data))->id);

   list_delete(use_list);
   ObjectListDestroy(sel_list);
}
/************************************************************************/
/*
 * UserToggleMusic: Called when user turns music on or off.
 *   music_on is True iff user just turned music on.
 */
void UserToggleMusic(Bool music_on)
{
   if (music_on)
      MusicStart();
   else MusicAbort();
}
/************************************************************************/
/*
 * UserQuit:  Ask user if he wants to quit game.  If so, send quit message
 *   to server.
 */
void UserQuit(void)
{
      RequestQuit();   
}
/************************************************************************/
/*
 * UserMakeOffer: Called when user wants to make an offer.
 */
void UserMakeOffer(void)
{
   list_type sel_list, items, recipients;
   ID recipient;

   /* Only allow 1 offer at a time */
   if (OfferInProgress())
   {
      RaiseOfferWindow();
      return;
   }

   /* If you have nothing to offer, forget it */
   if (player.inventory == NULL)
      return;
      
   if (OfferInProgress())
      return;

   recipients = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
			     CLOSE_DISTANCE, OF_OFFERABLE, OF_INVISIBLE);

   if (recipients == NULL)
   {
      GameMessage(GetString(hInst, IDS_NOOFFERERS));
      return;
   }

   /* Bring up look list to allow user to choose recipient of offer */
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_OFFERTO), recipients, LD_SINGLEAUTO);
   
   if (sel_list == NULL)
   {
      list_delete(recipients);
      return;
   }

   recipient = ((object_node *) (sel_list->data))->id;
   info.receiver_name = ((object_node *) (sel_list->data))->name_res;

   list_delete(recipients);
   ObjectListDestroy(sel_list);
   
   /* Now get items user wants to offer */
   items = DisplayLookList(hMain, GetString(hInst, IDS_OFFERITEMS), player.inventory, 
			   LD_MULTIPLESEL | LD_AMOUNTS);
   if (items == NULL)
      return;

   /* Send offer to server */
   RequestOffer(recipient, items);
   ObjectListDestroy(items);
}
/************************************************************************/
/*
 * UserBuy: Called when user wants to buy something.
 */
void UserBuy(void)
{
   list_type sel_list, sellers;
   ID seller;

   // Get object to buy from
   sellers = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
			  CLOSE_DISTANCE, OF_BUYABLE, 0);

   if (sellers == NULL)
   {
      GameMessage(GetString(hInst, IDS_NOSELLERS));
      return;
   }

   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_BUYFROM), sellers, LD_SINGLEAUTO);
   
   if (sel_list != NULL)
   {
      seller = ((object_node *) (sel_list->data))->id;
      RequestBuy(seller);
   }

   list_delete(sellers);
}
/************************************************************************/
/*
 * UserDeposit: Called when user wants to deposit something.
 */
void UserDeposit(void)
{
   list_type sel_list, items, recipients;
   ID recipient;

   /* Only allow 1 offer at a time */
   if (OfferInProgress())
   {
      RaiseOfferWindow();
      return;
   }

   /* If you have nothing to offer, forget it */
   if (player.inventory == NULL)
      return;
      
   if (OfferInProgress())
      return;

   recipients = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
			     CLOSE_DISTANCE, OF_OFFERABLE, 0);

   if (recipients == NULL)
   {
      GameMessage(GetString(hInst, IDS_NOOFFERERS));
      return;
   }

   /* Bring up look list to allow user to choose recipient of offer */
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_OFFERTO), recipients, LD_SINGLEAUTO);
   
   if (sel_list == NULL)
   {
      list_delete(recipients);
      return;
   }

   recipient = ((object_node *) (sel_list->data))->id;
   info.receiver_name = ((object_node *) (sel_list->data))->name_res;

   list_delete(recipients);
   ObjectListDestroy(sel_list);
   
   /* Now get items user wants to offer */
   items = DisplayLookList(hMain, GetString(hInst, IDS_DEPOSIT_ITEMS), player.inventory, 
			   LD_MULTIPLESEL | LD_AMOUNTS);
   if (items == NULL)
      return;

   /* Send offer to server */
   RequestDeposit(recipient, items);
   ObjectListDestroy(items);
}
/************************************************************************/
/*
 * UserWithdraw: Called when user wants to withdraw something.
 */
void UserWithdraw(void)
{
   list_type banker_list, bankers;
   ID banker;

   // Get object to buy from
   bankers = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK,
			  CLOSE_DISTANCE, OF_BUYABLE, 0);

   if (bankers == NULL)
   {
      GameMessage(GetString(hInst, IDS_NOSELLERS)); // No Bankers!
      return;
   }

   banker_list = DisplayLookList(hMain, GetString(hInst, IDS_BUYFROM), bankers, LD_SINGLEAUTO);
   
   if (banker_list != NULL)
   {
      banker = ((object_node *) (banker_list->data))->id;
      RequestWithdrawal(banker);
   }

   list_delete(bankers);
}
/************************************************************************/
void Offered(list_type items)
{
   info.items = items;

   /* Bring up offer dialog */
   CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OFFERSEND), 
		     NULL, SendOfferDialogProc, (LPARAM) &info);
}
/************************************************************************/
/*
 * UserApply: Called when user wants to use one object on another.
 */
void UserApply(void)
{
   list_type sel_list;
   
   sel_list = DisplayLookList(hMain, GetString(hInst, IDS_APPLY), player.inventory, 
			      LD_SINGLEAUTO);

   /* You may only use at one object at a time, so list has just 1 element */
   if (sel_list == NULL)
      return;

   StartApply(((object_node *) (sel_list->data))->id);
   
   ObjectListDestroy(sel_list);
}
/************************************************************************/
/*
 * StartApply:  Begin an apply action by storing first selected object,
 *   registering the ApplyCallback function and going into select state.
 *   This procedure is used when the user selects apply both from the main
 *   window and from the inventory.
 */
void StartApply(ID id)
{
   object1 = id;
   
   GameSetState(GAME_SELECT);
   
   /* Set callback for when user selects target object */
   SetSelectCallback(ApplyCallback);
}
/************************************************************************/
void ApplyCallback(ID object2)
{
   RequestApply(object1, object2);
}
/************************************************************************/
/*
 * SetDescParamsByRoomObject:  Set parameters for description dialog, based
 *   on given room object's flags.  hwnd is the parent window for the 
 *   description dialog.
 */
void SetDescParamsByRoomObject(room_contents_node *r, HWND hwnd)
{
   int params;

   params = DESC_NONE;
   if (r->distance <= CLOSE_DISTANCE)
   {
      if (r->obj.flags & OF_CONTAINER)
	 params |= DESC_INSIDE;
      
      if (r->obj.flags & OF_ACTIVATABLE && !(r->obj.flags & OF_PLAYER))
	 params |= DESC_ACTIVATE;

      if (r->obj.flags & OF_GETTABLE)
      {
	 params |= DESC_GET;
	 if (!(r->obj.flags & OF_ACTIVATABLE))
	    params |= DESC_USE;	 
      }
   }
   SetDescParams(hwnd, params);
}

/************************************************************************/
/*
 * UserTargetNextOrPrevious:  Called when the user asks to target "next" object. Sets idTarget.
 *								ajw
 */
void UserTargetNextOrPrevious(Bool bTargetNext)
{
	list_type	object_list;
	int			iListIndex = 0;
	int			iCurrentTargetIndex = -1;
	room_contents_node* rcnObject = NULL;

	//	Get list of visible objects.
	//	xxx use something like OF_ATTACKABLE?
	object_list = GetObjects3D(NO_COORD_CHECK, NO_COORD_CHECK, 0, OF_ATTACKABLE, OF_INVISIBLE);
	
	//	There doesn't seem to be a "list_find_item()"-like proc that returns position of an item in a list,
	//	so I'll just iterate through the list myself.

	if (!object_list)
	{
		SetUserTargetID(INVALID_ID);
	}
	else
	{
		if (idTarget != INVALID_ID)
		{
			//	Try to find current target in list. If target not found, iCurrentTargetIndex remains -1.
			while (rcnObject = (room_contents_node*)list_nth_item(object_list, iListIndex))
			{
				if (GetObjId(idTarget) == GetObjId(rcnObject->obj.id)) //xxx why GetObjId
				{
					iCurrentTargetIndex = iListIndex;
					break;
				}
				iListIndex++;
			}
		}
		if (bTargetNext)
		{
			if (iCurrentTargetIndex == -1)
				//	Select first item in list.
				SetUserTargetID(((room_contents_node*)list_first_item(object_list))->obj.id);
			else
			{
				//	If advancing past the last item in list, select first item.
				if (iCurrentTargetIndex + 1 == list_length(object_list))
					SetUserTargetID(((room_contents_node*)list_first_item(object_list))->obj.id);
				else
					SetUserTargetID(((room_contents_node*)list_nth_item(object_list, iCurrentTargetIndex + 1))->obj.id);
			}
		}
		else	//	target previous
		{
			if (iCurrentTargetIndex == -1)
				//	Select last item in list.
				SetUserTargetID(((room_contents_node*)list_last_item(object_list))->obj.id);
			else
			{
				//	If reversing past the first item in list, select last item.
				if (iCurrentTargetIndex == 0)
					SetUserTargetID(((room_contents_node*)list_last_item(object_list))->obj.id);
				else
					SetUserTargetID(((room_contents_node*)list_nth_item(object_list, iCurrentTargetIndex - 1))->obj.id);
			}
		}
		list_delete(object_list);
	}
}

/************************************************************************/
/*
 * SetUserTargetID:  Sets current target object id, which should be INVALID_ID if no target set.
 *						ajw
 */
void SetUserTargetID(ID idTargetNew)
{
	extern Bool map;

	if (map)
		idTargetNew = INVALID_ID;

	if (idTarget != idTargetNew)
	{
		idTarget = idTargetNew;
		RedrawAll();
	}
}

/************************************************************************/
/*
 * GetUserTargetID:  Returns current target object id, which is INVALID_ID if no target set.
 *						ajw
 */
ID GetUserTargetID()
{
	return idTarget;
}

void UserMouselookToggle(void)
{
	if (FALSE == gbMouselook)
	{
		RECT		rect;
//		WINDOWINFO	windowInfo;
		POINT	pt, center;
//		int		x, y;

//		GetWindowInfo(hMain, &windowInfo);
		GetClientRect(hMain, &rect);
		pt.x = rect.left;
		pt.y = rect.top;
		ClientToScreen(hMain, &pt);

		center.x = (gD3DRect.right - gD3DRect.left) / 2;
		center.y = (gD3DRect.bottom - gD3DRect.top) / 2;

		center.x += gD3DRect.left + pt.x;
		center.y += gD3DRect.top + pt.y;

//		x = (gD3DRect.right - gD3DRect.left) / 2;
//		y = (gD3DRect.bottom - gD3DRect.top) / 2;

//		x += gD3DRect.left + windowInfo.rcClient.left;
//		y += gD3DRect.top + windowInfo.rcClient.top;

		gbMouselook = TRUE;
//		GetClientRect(hMain, &rect);
//		GetCursorPos(&pt);
//		SetCursorPos(rect.right / 2, rect.bottom / 2);
		SetCursorPos(center.x, center.y);
		while (ShowCursor(FALSE) >= 0)
			ShowCursor(FALSE);
	}
	else
	{
		gbMouselook = FALSE;
		while (ShowCursor(TRUE) < 0)
			ShowCursor(TRUE);
	}
}

Bool UserMouselookIsEnabled(void)
{
	return gbMouselook;
}

void UserTargetSelf()
{
	SetUserTargetID(player.id);
}
