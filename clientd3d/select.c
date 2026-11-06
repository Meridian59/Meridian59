// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * select.c:  Game mode where user selects an object by clicking on it with the mouse.
 */

#include "client.h"

static SelectCallback callback = NULL;   /* Function to call when object selected */

/************************************************************************/
/*
 * SetSelectCallback:  Store callback function to be called when user
 *   clicks on an object.  Function is called with this object as parameter.
 */
void SetSelectCallback(SelectCallback fn)
{
   callback = fn;
}
/************************************************************************/
/*
 * GetMouseSquareObjects:  Return objects under (x, y), nearest first.
 */
static list_type GetMouseSquareObjects(int x, int y)
{
   list_type square_list, spell_list, l;
   room_contents_node *r;

   square_list = GetObjects3D(x, y, 0, 0, OF_NOEXAMINE);

   // Add spell-selectable objects to the list even if OF_NOEXAMINE
   spell_list = GetObjects3D(x, y, 0, OF_SPELLSELECT, 0);

   for (l = spell_list; l != NULL; l = l->next)
   {
      r = (room_contents_node *) (l->data);
      if (r->obj.flags & OF_NOEXAMINE)
         square_list = list_add_sorted_item(square_list, r, CompareRoomObjectDistance);
   }

   list_delete(spell_list);

   return square_list;
}
/************************************************************************/
/*
 * UserSelect:  User selected object to apply.  If target is 0, object is below
 *   current position of mouse pointer; otherwise target gives target object id.
 *   Get object that's there, if any, and call callback on that object.
 */
void UserSelect(ID target)
{
   int x, y;
   list_type square_list, sel_list;

   if (target == 0)
   {
      /* Find out which row of the room the user clicked on, if any */
      if (!MouseToRoom(&x, &y))
	 return;
      
      /* Switch focus to main window, in case it wasn't there */
      SetFocus(hMain);
      
      /* See if user clicked on an object--even if not, go back to normal mode */
      square_list = GetMouseSquareObjects(x, y);
      
      GameSetState(GAME_PLAY);
      
      if (square_list == NULL)
	 return;
      
      /* If more than one object in square, ask user which one */
      sel_list = DisplayLookList(hMain, GetString(hInst, IDS_GETTARGET), square_list, 
				 LD_SINGLEAUTO);
      if (sel_list == NULL)
      {
	 list_delete(square_list);
	 return;
      }

      target = ((object_node *) (sel_list->data))->id;
      list_destroy(sel_list);
      list_delete(square_list);
   }
   else GameSetState(GAME_PLAY);

   /* Call callback function */
   if (callback != NULL)
      (*callback)(target);
}
/************************************************************************/
/*
 * SelectedObject:  User clicked on object in inventory; call callback 
 */
void SelectedObject(ID object2)
{
   if (callback != NULL)
      (*callback)(object2);
   GameSetState(GAME_PLAY);
}
