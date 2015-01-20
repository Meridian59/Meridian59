// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildinv.c:  Handle guild invitation dialog.
 */

#include "client.h"
#include "merintr.h"

/*****************************************************************************/
/*
 * GuildInviteDialogProc:  Dialog procedure for guild invitation dialog.
 */
BOOL CALLBACK GuildInviteDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
  HWND hList;
  list_type l;
  int index;
  ID player_id;

  switch (message)
  {
  case WM_INITDIALOG:
     hList = GetDlgItem(hDlg, IDC_INROOMLIST);
     SetWindowFont(hList, GetFont(FONT_LIST), FALSE);

     // Add people in room to other list box (skip self and invisible people) 
     for (l = cinfo->current_room->contents; l != NULL; l = l->next)
     {
	room_contents_node *r = (room_contents_node *) (l->data);

	if ((r->obj.flags & OF_PLAYER) && r->obj.id != cinfo->player->id &&
	   r->obj.drawingtype != DRAWFX_INVISIBLE)
	{
	   index = ListBox_AddString(hList, LookupNameRsc(r->obj.name_res));
	   ListBox_SetItemData(hList, index, r->obj.id);
	}
     }
     
     CenterWindow(hDlg, GetParent(hDlg));
     return TRUE;
     
  case WM_COMMAND:
     switch(GET_WM_COMMAND_ID(wParam, lParam))
     {
     case IDC_INVITE:
	// Get currently selected user (if any), and send invite command
	hList = GetDlgItem(hDlg, IDC_INROOMLIST);
	index = ListBox_GetCurSel(hList);
	if (index == LB_ERR)
	   break;
	
	player_id = ListBox_GetItemData(hList, index);
	RequestInvite(player_id);
	return TRUE;
     }
  }
  return FALSE;
}

