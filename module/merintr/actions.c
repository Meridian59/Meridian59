// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * actions.c:  Deal with "Actions" menu.  This is dynamically added to the menu bar
 *   when the user enters the game.
 */

#include "client.h"
#include "merintr.h"

static HMENU actions_menu;    // Actions submenu

typedef struct {
   int command;                // Windows menu command when item selected (0 if a separator item)
   int name;                   // ID of string to show for item
   char *text;                 // Text command to execute
} Action;

static Action actions[] = {
  { IDM_WHO,      IDS_MENU_WHO,      "who",     },
  { IDM_GROUP,    IDS_MENU_GROUP,    "group",   },
  { IDM_ALIAS,    IDS_MENU_ALIAS,    "alias",   },
  { IDM_VERBALIAS,IDS_MENU_CMDALIAS, "cmdalias",},
  { IDM_GUILD,    IDS_MENU_GUILD,    "guild",   },
  { 0,            0,              NULL,      },  // Separator
  { IDM_WAVE,     IDS_MENU_WAVE,     "wave",    },
  { IDM_POINT,    IDS_MENU_POINT,    "point",   },
  { IDM_DANCE,    IDS_MENU_DANCE,    "dance",   },
  { 0,            0,              NULL,      },  // Separator
  { IDM_HAPPY,    IDS_MENU_HAPPY,    "happy",   },
  { IDM_SAD,      IDS_MENU_SAD,      "sad",     },
  { IDM_NEUTRAL,  IDS_MENU_NEUTRAL,  "neutral", },
  { IDM_WRY,      IDS_MENU_WRY,      "wry",     },
};

static int num_actions = (sizeof(actions) / sizeof(Action));

// Need to know numeric positions of main menu top level things, silly as this may be
#define MENU_POSITION_ACTIONS 3

/********************************************************************/
/*
 * ActionsInit:  Initialize actions when game entered.
 */
void ActionsInit(void)
{
   int i;

   actions_menu = CreatePopupMenu();

   // Add "actions" menu item
   if (actions_menu == NULL)
     return;

   InsertMenu(cinfo->main_menu, MENU_POSITION_ACTIONS, MF_STRING | MF_POPUP | MF_BYPOSITION, 
	      (UINT) actions_menu, GetString(hInst, IDS_ACTIONS));
   DrawMenuBar(cinfo->hMain);

   // Add subitems
   for (i=0; i < num_actions; i++)
     {
       if (actions[i].command == 0)
          InsertMenu(actions_menu, i, MF_STRING | MF_SEPARATOR, 0, NULL);
       else InsertMenu(actions_menu, i, MF_STRING | MF_BYPOSITION, actions[i].command, 
                       GetString(hInst, actions[i].name));
     }
}
/********************************************************************/
/*
 * ActionsExit:  Free actions when game exited.
 */
void ActionsExit(void)
{
   // Remove "Actions" menu
   if (actions_menu != NULL)
   {
      RemoveMenu(cinfo->main_menu, MENU_POSITION_ACTIONS, MF_BYPOSITION);
      DrawMenuBar(cinfo->hMain);
   }

   DestroyMenu(actions_menu);
   actions_menu = NULL;
}
/********************************************************************/
/*
 * MenuActionChosen:  The spell with the given command id in the spell menu was chosen.
 */
void MenuActionChosen(int id)
{
  int i;

  for (i=0; i < num_actions; i++)
    {
      if (actions[i].command != id)
         continue;

      PerformAction(A_TEXTCOMMAND, actions[i].text);
      return;
    }
}
