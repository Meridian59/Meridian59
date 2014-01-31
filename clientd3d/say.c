// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * say.c:  Deal with functions to allow user to say something.
 *
 * We also maintain a list of currently logged in users here, in current_users.
 * This is a list of objects, where only the id and name fields are valid.
 * The server keeps this list updated as players log in and out.
 */

#include "client.h"

#define MAX_SPACES 10   // Max # of consecutive spaces you're allowed to say
#define MAX_CODERUN 4   // Max # of consecutive color codes you're allowed to say
#define MAX_CODES  20   // Max # of color codes you're allowed to say through a message

HWND hWhoDlg = NULL;

/* Users who are currently logged on -- list of objects */
list_type current_users = NULL;  

/*****************************************************************************/
/*
* SetCurrentUsers:  Set list of current users to given list.
*/
void SetCurrentUsers(list_type users)
{
	/* Destroy old list, and if dialog is up, reset user list box */
	ObjectListDestroy(current_users);
	current_users = users;
}
/*****************************************************************************/
/*
* AddCurrentUser:  Add user to list of current users
*/
void AddCurrentUser(object_node *user)
{
	// If already there, ignore
	if (list_find_item(current_users, (void *) user->id, CompareIdObject) != NULL)
	{
		ObjectDestroyAndFree(user);
		return;
	}
	
	current_users = list_add_item(current_users, user);
	UpdateWho(user, TRUE);
}
/*****************************************************************************/
/*
* RemoveCurrentUser:  Remove user with given id from list of current users.
*/
void RemoveCurrentUser(ID user_id)
{
	object_node *user;
	
	user = (object_node *) list_find_item(current_users, (void *) user_id, CompareIdObject);
	if (user == NULL)
	{
		debug(("Tried to remove nonexistent user %d from user list", user_id));
		return;
	}
	UpdateWho(user, FALSE);
	current_users = list_delete_item(current_users, (void *) user_id, CompareIdObject);
	SafeFree(user);
}
/*****************************************************************************/
/*
* FreeCurrentUsers:  Free list of current users
*/
void FreeCurrentUsers(void)
{
	ObjectListDestroy(current_users);
	current_users = NULL;
}

/************************************************************************/
/*
* UserWho:  Display a list of currently logged on users.
*/
void UserWho(void)
{
   /* If dialog is not open, create it; else raise it to top*/
   if (hWhoDlg == NULL)
   {
      hWhoDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_WHO), hMain,
      WhoDialogProc, (LPARAM) current_users);
   }
   else
   {
      ShowWindow(hWhoDlg, SW_SHOWNORMAL);
      SetFocus(hWhoDlg);
   }
}

/************************************************************************/
/*
* FilterSayMessage:
* Remove newlines and blank chars from said text to avoid spoofs.
* Remove redundant or excessive color codes.
* Limit the said text to a reasonable length.
* Returns FALSE only if the message is not to be sent at all
* (usually because it's listed as profanity).
*/
BOOL FilterSayMessage(char *message)
{
	char* s;
	char* d;
	int spaces;
	int coderun;
	int codes;
	
	if (!message)
		return FALSE;
	
	if (config.antiprofane && ContainsProfaneTerms(message))
	{
		char message[256];
		LoadString(hInst, IDS_PROFANITYWARNING, message, sizeof(message));
		MessageBox(hMain, message, szAppName, MB_OK | MB_ICONHAND);
		//
		// Kind of a misnomer: message says your text has been blocked,
		// but it requires the caller of FilterSayMessage() to actually
		// do the blocking when we return FALSE.
		//
		return FALSE;
	}
	
	// Remove all control characters.
	s = d = message;
	while (*s)
	{
		if (*s & ~0x1F)
			*d++ = *s;
		s++;
	}
	*d = '\0';
	
	// Remove large sequences of blanks and color codes.
	s = d = message;
	spaces = 0;
	coderun = 0;
	codes = 0;
	while (*s)
	{
		// Count consecutive spaces or color codes.
		if (*s == ' ' || *s == '~' || *s == '`')
		{
			spaces++;
			if (*s != ' ')
			{
				coderun++;
				codes++;
			}
		}
		else
		{
			spaces = 0;
			coderun = 0;
		}
		
		// If we haven't hit the limit,
		if ((spaces < MAX_SPACES) &&
			(coderun < MAX_CODERUN) &&
			(!(*s == '~' || *s == '`') || codes < MAX_CODES))
		{
			// Copy the single character or two-character color code.
			if (*s == '~' || *s == '`')
				*d++ = *s++;
			*d++ = *s++;
		}
		else
		{
			// Skip the single character or two-character color code.
			if (*s == '~' || *s == '`')
				s++;
			s++;
		}
	}
	*d = '\0';
	
	return TRUE;
}

/************************************************************************/
/*
* WhoClose:  Close the Who dialog box
*/
void WhoClose()
{
   if (hWhoDlg != NULL)
      {
      DestroyWindow(hWhoDlg);
      hWhoDlg = NULL;
      }
}