// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * parse.c:  Parse commands typed by the user.
 */

#include "client.h"

/************************************************************************/
/*
 * ParseGotText:  User typed given string.  Pass it on to modules.
 */
BOOL ParseGotText(char *str)
{
   char text[MAXSAY + 1];
   BOOL bInvalid;

   // Copy string to buffer, in case it gets modified
   strncpy(text, str, MAXSAY);
   text[MAXSAY] = 0;

   bInvalid = ModuleEvent(EVENT_TEXTCOMMAND, text);
   if (bInvalid)
      GameMessage(GetString(hInst, IDS_BADCOMMAND));

   return !bInvalid;
}
/************************************************************************/
/*
 * ParseCommand:  Parse given string according to given 
 *   command table and call the appropriate function.
 *   Return True iff a command was matched.
 */
Bool ParseCommand(char *str, TypedCommand *commands)
{
   char *ptr1, *ptr2;
   int max_index;  // Index of best matching command
   int index, match, max_match;
   Bool tied;      // True if a different index matches as well as best_index

   // Skip initial spaces
   while (*str == ' ')
      str++;

   max_match = 0;
   index = 0;
   while (commands[index].name != NULL)
   {
      ptr1 = str;
      ptr2 = commands[index].name;
      match = 0;

      while (*ptr1 != 0 && *ptr2 != 0)
      {
         // Stop comparing when we reach end of typed word
         if (*ptr1 == ' ' && *ptr2 != ' ')
            break;

         if (toupper(*ptr1) != toupper(*ptr2))
         {
            match = 0;
               break;
         }
         match++;
         ptr1++;
         ptr2++;
      }

      if (*ptr2 == 0)
      {
         // Check for exact match
         if (*ptr1 == 0 || *ptr1 == ' ')
         {
            max_match = 1;
            max_index = index;
            break;
         }
         else
         {
            // Don't match if extra characters typed
            index++;
            continue;
         }
      }

      if (match > max_match)
      {
         max_match = match;
         max_index = index;
      }
      else if (match == max_match)
         tied = True;
      index++;
   }

   if (max_match == 0)
      return False;

   // Allow "tied" to be True; first command listed will be chosen

   // Skip command
   ptr1 = str;
   while (*ptr1 != 0 && *ptr1 != ' ')
      ptr1++;

   // Remove intervening spaces and call associated procedure
   while (*ptr1 == ' ')
      ptr1++;
   (*commands[max_index].proc)(ptr1);

   return True;
}
