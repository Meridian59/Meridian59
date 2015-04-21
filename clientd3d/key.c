// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * key.c:  Deal with user key presses.
 *
 * The main issues are key tables and timing.
 *
 * 1) Key tables
 * 
 * We translate each keypress into an "action", which is a description of what the user
 * wants to do, such as "move left" or "say something".  An action corresponds to one
 * or more keys along with the state of the modifier keys (control, shift, and alt).
 * The TranslateKey function translates a key and modifier state into an action.
 * It takes a key table as an argument; this table contains (key + modifers, action) pairs.
 *
 * The mapping of keys to actions depends on the game state.  We keep a list of key tables;
 * each table is active in only a single game state.  Modules add key tables by calling
 * KeyAddTable.
 *
 * Modules that handle certain keys can also call TranslateKey with a key table containing
 * the keys they want to handle, and the actions those keys correspond to.  Typically,
 * the module will also handle this action.
 *
 * 2) Timing
 *
 * We poll the keyboard whenever the main window's message loop is empty.  However, many
 * actions (such as toggling map mode) should not be repeated if a key is held down, while
 * others (such as moving and turning) should be.  The former actions are referred to
 * as "norepeat" actions; we keep the time the last norepeat action was performed in a global
 * variable.  If another norepeat action is attempted before KEY_NOREPEAT_INTERVAL, it is
 * discarded.
 *
 * The move and turn actions must be limited in frequency, since the rate of user motion
 * should be the same for all players.  However, we allow smaller moves and turns to
 * happen more often when animation is on, to produce smooth motion.
 */

#include "client.h"

#define NUM_KEYS      256  /* Number of possible key codes */
#define NUM_REAL_KEYS 128  /* Top of range of useful keys */

#define MAX_ACTIONS   10   /* Max # of actions to do on a single poll of keyboard */

// Minimum # of milliseconds between non-repeat actions
#define KEY_NOREPEAT_INTERVAL 400

// Last time we performed a non-repeat action
static DWORD last_norepeat_time;

typedef struct {
   int      state;   // Game state in which this key table applies
   KeyTable table;
} KeyTableListEntry;

static list_type key_tables;  // List of KeyTableListEntries

#define IsDown(k) ((k) & 0x80)    // True when given key status indicates key is down

static Bool CompareKeyTableListEntries(void *p1, void *p2);
static void KeyInitHack(void);

extern keymap	gCustomKeys[];
extern keymap	gQuickChatTable[];
/************************************************************************/
/*
 * KeyInit:
 */
void KeyInit(void)
{
   KeyInitHack();
}
/************************************************************************/
/*
 * KeyClose:  Free memory for key table list.
 */
void KeyClose(void)
{
   key_tables = list_destroy(key_tables);
}
/************************************************************************/
/*
 * KeyAddTable:  Add the given key table to handle keypresses in the given
 *   game state.
 */
void KeyAddTable(int game_state, KeyTable table)
{
   KeyTableListEntry *entry;

   entry = (KeyTableListEntry *) SafeMalloc(sizeof (KeyTableListEntry));
   entry->state = game_state;
   entry->table = table;

   key_tables = list_add_item(key_tables, entry);
}
/************************************************************************/
/*
 * KeyRemoveTable:  Remove the given key table.
 */
void KeyRemoveTable(int game_state, KeyTable table)
{
   KeyTableListEntry entry;

   entry.state = game_state;
   entry.table = table;

   key_tables = list_delete_item(key_tables, table, CompareKeyTableListEntries);
}
/************************************************************************/
Bool CompareKeyTableListEntries(void *p1, void *p2)
{
   KeyTableListEntry *e1 = (KeyTableListEntry *) p1;
   KeyTableListEntry *e2 = (KeyTableListEntry *) p2;
   return (e1->state == e2->state && e1->table == e2->table);
}

/************************************************************************/
/* 
 * TranslateKey: Translate and return the command constant for the given
 *   virtual key.  Perform translation using given keymap table.
 *   Return A_NOACTION if no command exists.  Otherwise, return action and set data 
 *   to point to data field for keypress.
 */
int TranslateKey(UINT vk_key, KeyTable table, void **data)
{
   int index;
   WORD table_flags, flags = 0;

   if (table == NULL)
   {
      debug(("TranslateKey got NULL key table\n"));
      return A_NOACTION;
   }

   /* Find out what modifier keys are pressed */
	if (GetKeyState(VK_CONTROL) < 0)
		flags |= KEY_CTL;
	    
   if (GetKeyState(VK_SHIFT) < 0)
      flags |= KEY_SHIFT;

   if (GetKeyState(VK_MENU) < 0)
      flags |= KEY_ALT;

   index = 0;
   while (table[index].vk_code != 0)
   {
      if (table[index].vk_code == vk_key)
      {
	 table_flags = table[index].flags;

	 if (table_flags == KEY_ANY || 
	     (table_flags & (KEY_SHIFT | KEY_ALT | KEY_CTL)) == flags || 
	     ((table_flags & KEY_NONE) && flags == 0) )
	 {
	    *data = table[index].data;
	    return table[index].command;
	 }
      }
      index++;
   }
   return A_NOACTION;
}
/************************************************************************/
/*
 * HandleKeys:  Check state of keyboard and perform actions that correspond
 *   to keys that are down.
 */
void HandleKeys(Bool poll)
{
   Bool norepeat;
   BYTE keys[NUM_KEYS];
   int action, i, j;
   DWORD now;
   int actions[MAX_ACTIONS];  // Holds actions we've already done
   int num_actions;           // Number of valid entries in actions
   Bool moved, turned, already_done;
   KeyTable table;
   void *action_data;
   list_type l;

   /* If main window no longer has the focus, don't read keyboard */
   if (GetFocus() != hMain)
      return;

   now = timeGetTime();
   num_actions = 0;

   GetKeyboardState(keys);

   // Go through all key tables that match the current state
   for (l = key_tables; l != NULL; l = l->next)
   {
      KeyTableListEntry *entry;

      entry = (KeyTableListEntry *) (l->data);

      if (entry->state != GameGetState())
	 continue;
      
      table = entry->table;

      /* Perform action for ALL keys that are down; this allows multiple arrow keys to work */
      /* First, mark all actions that correspond to keys that are down */
      norepeat = False;
      moved = turned = False;
      for (i = 1; i < NUM_KEYS; i++)
      {
	 if (!IsDown(keys[i]))
	    continue;
	 
	 action = TranslateKey(i, table, &action_data);
	 if (action == A_NOACTION)
	    continue;

   // Don't constantly toggle tab or mouselook if key held down.
   if ((IsTabAction(action) || IsMouseLookAction(action)) && poll)
      continue;

	 // Perform action at most once
	 already_done = False;
	 for (j=0; j < num_actions; j++)
	    if (actions[j] == action)
	       already_done = True;
	 
	 if (already_done || num_actions == MAX_ACTIONS)
	    continue;
	 
	 actions[num_actions++] = action;
	 
	 /* Only repeat for moving actions and a few others */
	 if (!RepeatAction(action))
	    if (now - last_norepeat_time >= KEY_NOREPEAT_INTERVAL)
	       norepeat = True;
	    else continue;    // Action retried too soon
	 
	if (IsMoveAction(action))
		if (moved)
			continue;
		else
		{
			int	tempKey = i;
			int	tempAction;

			moved = True;

			for (; tempKey < NUM_KEYS; tempKey++)
			{
				if (!IsDown(keys[tempKey]))
					continue;

				tempAction = TranslateKey(tempKey, table, &action_data);

				if (IsMoveAction(tempAction))
				{
					switch (action)
					{
						case A_FORWARD:
							switch(tempAction)
							{
								case A_SLIDELEFT:
									action = A_SLIDELEFTFORWARD;
									tempKey = NUM_KEYS;
								break;

								case A_SLIDERIGHT:
									action = A_SLIDERIGHTFORWARD;
									tempKey = NUM_KEYS;
								break;
							}
						case A_FORWARDFAST:
							switch(tempAction)
							{
								case A_SLIDELEFTFAST:
									action = A_SLIDELEFTFORWARDFAST;
									tempKey = NUM_KEYS;
								break;

								case A_SLIDERIGHTFAST:
									action = A_SLIDERIGHTFORWARDFAST;
									tempKey = NUM_KEYS;
								break;
							}
						break;

						case A_BACKWARD:
							switch(tempAction)
							{
								case A_SLIDELEFT:
									action = A_SLIDELEFTBACKWARD;
									tempKey = NUM_KEYS;
								break;

								case A_SLIDERIGHT:
									action = A_SLIDERIGHTBACKWARD;
									tempKey = NUM_KEYS;
								break;
							}
						case A_BACKWARDFAST:
							switch(tempAction)
							{
								case A_SLIDELEFTFAST:
									action = A_SLIDELEFTBACKWARDFAST;
									tempKey = NUM_KEYS;
								break;

								case A_SLIDERIGHTFAST:
									action = A_SLIDERIGHTBACKWARDFAST;
									tempKey = NUM_KEYS;
								break;
							}
						break;

						case A_SLIDELEFT:
							switch(tempAction)
							{
								case A_FORWARD:
									action = A_SLIDELEFTFORWARD;
									tempKey = NUM_KEYS;
								break;

								case A_BACKWARD:
									action = A_SLIDELEFTBACKWARD;
									tempKey = NUM_KEYS;
								break;
							}
						break;

						case A_SLIDELEFTFAST:
							switch(tempAction)
							{
								case A_FORWARDFAST:
									action = A_SLIDELEFTFORWARDFAST;
									tempKey = NUM_KEYS;
								break;

								case A_BACKWARDFAST:
									action = A_SLIDELEFTBACKWARDFAST;
									tempKey = NUM_KEYS;
								break;
							}
						break;

						case A_SLIDERIGHT:
							switch(tempAction)
							{
								case A_FORWARD:
									action = A_SLIDERIGHTFORWARD;
									tempKey = NUM_KEYS;
								break;

								case A_BACKWARD:
									action = A_SLIDERIGHTBACKWARD;
									tempKey = NUM_KEYS;
								break;
							}
						break;

						case A_SLIDERIGHTFAST:
							switch(tempAction)
							{
								case A_FORWARDFAST:
									action = A_SLIDERIGHTFORWARDFAST;
									tempKey = NUM_KEYS;
								break;

								case A_BACKWARDFAST:
									action = A_SLIDERIGHTBACKWARDFAST;
									tempKey = NUM_KEYS;
								break;
							}
						break;
					}
				}
			}
		}
	 if (IsTurnAction(action))
	    if (turned)
	       continue;
	    else turned = True;
	 
	 // If we're doing a non-repeat action, remember the time so that we don't repeat it
	 if (norepeat)
	    last_norepeat_time = now;
	 
	 PerformAction(action, action_data);
      }
   }
}
/***********************************************************************/
/*
 * KeySetLastNorepeatTime:  Set last time a non-repeat action occurred to the
 *   current time.
 */
void KeySetLastNorepeatTime(void)
{
   last_norepeat_time = timeGetTime();
}
/***********************************************************************/
/*
 * KeysDown:  Return # of keys that are down.  Since some keys (like ALT)
 *   actually correspond to more than one key value, we check only key
 *   values in [1, 127].
 */
int KeysDown(void)
{
   int count = 0, i;
   BYTE keys[NUM_KEYS];

   GetKeyboardState(keys);

   for (i = 1; i < NUM_REAL_KEYS; i++)
      if (IsDown(keys[i]))
	 count++;

   return count;
}
/****************************************************************************/
/*
 * KeyUpHack:  Due to an apparent bug in Windows, when a key is pressed, NUM LOCK
 *   is toggled, and then the key is released, the key up code is not always the
 *   same as the key down code.  Here we synthesize an up message for the original key;
 *   without this, the key would remain down, causing the player to spin out of control.
 *
 *   vk is the key that was released.
 */
void KeyUpHack(int vk)
{
   int key = 0;
   BYTE keys[NUM_KEYS];

   switch (vk)
   {
   case VK_NUMPAD1:   key = VK_END;        break;
   case VK_NUMPAD2:   key = VK_DOWN;       break;
   case VK_NUMPAD3:   key = VK_NEXT;       break;
   case VK_NUMPAD4:   key = VK_LEFT;       break;
   case VK_NUMPAD5:   key = VK_CLEAR;      break;
   case VK_NUMPAD6:   key = VK_RIGHT;      break;
   case VK_NUMPAD7:   key = VK_HOME;       break;
   case VK_NUMPAD8:   key = VK_UP;         break;
   case VK_NUMPAD9:   key = VK_PRIOR;      break;
   case VK_END:       key = VK_NUMPAD1;    break;
   case VK_DOWN:      key = VK_NUMPAD2;    break;
   case VK_NEXT:      key = VK_NUMPAD3;    break;
   case VK_LEFT:      key = VK_NUMPAD4;    break;
   case VK_CLEAR:     key = VK_NUMPAD5;    break;
   case VK_RIGHT:     key = VK_NUMPAD6;    break;
   case VK_HOME:      key = VK_NUMPAD7;    break;
   case VK_UP:        key = VK_NUMPAD8;    break;
   case VK_PRIOR:     key = VK_NUMPAD9;    break;
   }
   if (key == 0)
      return;

   GetKeyboardState(keys);
   keys[key] = 0;
   SetKeyboardState(keys);
}
/****************************************************************************/
/*
 * KeyInitHack:  Set number pad keys to be up, otherwise we can get the num lock
 *   bug described above.
 */
void KeyInitHack(void)
{
   BYTE keys[NUM_KEYS];

   GetKeyboardState(keys);
   keys[VK_END] = 0;
   keys[VK_DOWN] = 0;
   keys[VK_NEXT] = 0;
   keys[VK_LEFT] = 0;
   keys[VK_CLEAR] = 0;
   keys[VK_RIGHT] = 0;
   keys[VK_HOME] = 0;
   keys[VK_UP] = 0;
   keys[VK_PRIOR] = 0;
   keys[VK_NUMPAD1] = 0;
   keys[VK_NUMPAD2] = 0;
   keys[VK_NUMPAD3] = 0;
   keys[VK_NUMPAD4] = 0;
   keys[VK_NUMPAD5] = 0;
   keys[VK_NUMPAD6] = 0;
   keys[VK_NUMPAD7] = 0;
   keys[VK_NUMPAD8] = 0;
   keys[VK_NUMPAD9] = 0;
   SetKeyboardState(keys);
}
